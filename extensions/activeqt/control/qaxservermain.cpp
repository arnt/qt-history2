/****************************************************************************
** $Id: $
**
** Implementation of win32 ActiveX server startup routines
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qmetaobject.h>
#include <qsettings.h>
#include <qmap.h>
#include <qapplication.h>
#include <qfile.h>
#include <private/qucom_p.h>
#include <qdir.h>

#include <qt_windows.h>

/*
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
*/
#define _ATL_APARTMENT_THREADED
#define STRICT

#include "qaxserverbase.h"
#include "qaxbindable.h"

#ifdef DEBUG
const DWORD dwTimeOut = 1000;
const DWORD dwPause = 500;
#else
const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 1000; // time to wait for threads to finish up
#endif

#ifdef Q_OS_TEMP
EXTERN_C int __cdecl main( int, char ** );
#else
EXTERN_C int main( int, char ** );
#endif
CExeModule _Module;

extern bool qax_ownQApp;

extern HHOOK hhook;
LRESULT CALLBACK FilterProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if ( qApp )
	qApp->sendPostedEvents();

    return CallNextHookEx( hhook, nCode, wParam, lParam );
}


// COM Factory class, mapping COM requests to ActiveQt requests.
// One instance of this class for each ActiveX the server can provide.
class QClassFactory : public IClassFactory
{
public:
    QClassFactory( CLSID clsid )
	: ref( 0 )
    {
	// COM only knows the CLSID, but QAxFactory is class name based...
	QStringList keys = _Module.factory()->featureList();
	for ( QStringList::Iterator  key = keys.begin(); key != keys.end(); ++key ) {
	    if ( _Module.factory()->classID( *key ) == clsid ) {
		className = *key;
		break;
	    }
	}
    }

    // IUnknown
    unsigned long WINAPI AddRef()
    {
	return ++ref;
    }
    unsigned long WINAPI Release()
    {
	if ( !--ref ) {
	    delete this;
	    return 0;
	}
	return ref;
    }
    HRESULT WINAPI QueryInterface( REFIID iid, LPVOID *iface )
    {
	*iface = 0;
	if ( iid == IID_IUnknown )
	    *iface = (IUnknown*)this;
	else if ( iid == IID_IClassFactory )
	    *iface = (IClassFactory*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    // IClassFactory
    HRESULT WINAPI CreateInstance( IUnknown *pUnkOuter, REFIID iid, void **ppObject )
    {
	// Make sure a QApplication instance is present (inprocess case)
	if ( !qApp ) {
	    qax_ownQApp = TRUE;
	    int argc = 0;
	    (void)new QApplication( argc, 0 );
	    hhook = SetWindowsHookEx( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
	}

	// Create the ActiveX wrapper
	QAxServerBase *activeqt = new QAxServerBase( className );
	return activeqt->QueryInterface( iid, ppObject );
    }
    HRESULT WINAPI LockServer( BOOL fLock )
    {
	if ( fLock )
	    _Module.Lock();
	else
	    _Module.Unlock();

	return S_OK;
    }

protected:
    unsigned long ref;
    QString className;
};

// Passed to CreateThread to monitor the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if ( !l ) {
        bActivity = true;
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
    }
    return l;
}

//Monitors the shutdown event
void CExeModule::MonitorShutdown()
{
    while (1) {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!bActivity && m_nLockCnt == 0) { // if no activity let's really bail
            break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    PostQuitMessage( 0 );
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

extern QUnknownInterface *ucm_instantiate();

QAxFactoryInterface *CExeModule::factory()
{
    if ( !_factory ) {
	QInterfacePtr<QUnknownInterface> unknown = ucm_instantiate();
	if ( unknown )
	    unknown->queryInterface( IID_QAxFactory, (QUnknownInterface**)&_factory );
    }
    return _factory;
}

// dummy function used in object map.
// We have our own factory to do that
static HRESULT WINAPI CreateInstance( void *pUnkOuter, REFIID iid, void **ppUnk )
{
    HRESULT nRes = E_OUTOFMEMORY;
    return nRes;
}

// Create a QClassFactory object for class \a iid
HRESULT WINAPI GetClassObject( void *pv, REFIID iid, void **ppUnk )
{
    HRESULT nRes = E_OUTOFMEMORY;
    QClassFactory *factory = new QClassFactory( iid );
    if ( factory )
	nRes = factory->QueryInterface( IID_IClassFactory, ppUnk );
    return nRes;
}

char module_filename[MAX_PATH];


// (Un)Register the ActiveX server in the registry.
// The QAxFactory implementation provides the information.
HRESULT WINAPI UpdateRegistry(BOOL bRegister)
{
    QString file = QString::fromLocal8Bit(module_filename );
    QString path = file.left( file.findRev( "\\" )+1 );
    QString module = file.right( file.length() - path.length() );
    module = module.left( module.findRev( "." ) );

    const QString appId = _Module.factory()->appID().toString().upper();
    const QString libId = _Module.factory()->typeLibID().toString().upper();

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );

    if ( bRegister ) {
	settings.writeEntry( "/AppID/" + appId + "/.", module );
	settings.writeEntry( "/AppID/" + module + ".EXE/AppID", appId );

	settings.writeEntry( "/TypeLib/" + libId + "/1.0/0/win32/.", file );
	settings.writeEntry( "/TypeLib/" + libId + "/1.0/FLAGS/.", "0" );
	settings.writeEntry( "/TypeLib/" + libId + "/1.0/HELPDIR/.", path );
	settings.writeEntry( "/TypeLib/" + libId + "/1.0/.", module + " 1.0 Type Library" );

	QStringList keys = _Module.factory()->featureList();
	for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	    const QString classId = _Module.factory()->classID(*key).toString().upper();
	    const QString eventId = _Module.factory()->eventsID(*key).toString().upper();
	    const QString ifaceId = _Module.factory()->interfaceID(*key).toString().upper();
	    const QString className = *key;
	    
	    settings.writeEntry( "/" + module + "." + className + ".1/.", className + " Class" );
	    settings.writeEntry( "/" + module + "." + className + ".1/CLSID/.", classId );
	    settings.writeEntry( "/" + module + "." + className + ".1/Insertable/.", QString::null );
	    
	    settings.writeEntry( "/" + module + "." + className + "/.", className + " Class" );
	    settings.writeEntry( "/" + module + "." + className + "/CLSID/.", classId );
	    settings.writeEntry( "/" + module + "." + className + "/CurVer/.", module + "." + className + ".1" );
	    
	    settings.writeEntry( "/CLSID/" + classId + "/.", className + " Class" );
	    settings.writeEntry( "/CLSID/" + classId + "/AppID", appId );
	    settings.writeEntry( "/CLSID/" + classId + "/Control/.", QString::null );
	    settings.writeEntry( "/CLSID/" + classId + "/Insertable/.", QString::null );
	    if ( file.right( 3 ).lower() == "dll" )
		settings.writeEntry( "/CLSID/" + classId + "/InProcServer32/.", file );
	    else
		settings.writeEntry( "/CLSID/" + classId + "/LocalServer32/.", file + " -activex" );
	    settings.writeEntry( "/CLSID/" + classId + "/MiscStatus/.", "0" );
	    settings.writeEntry( "/CLSID/" + classId + "/MiscStatus/1/.", "131473" );
	    settings.writeEntry( "/CLSID/" + classId + "/Programmable/.", QString::null );
	    settings.writeEntry( "/CLSID/" + classId + "/ToolboxBitmap32/.", file + ", 101" );
	    settings.writeEntry( "/CLSID/" + classId + "/TypeLib/.", libId );
	    settings.writeEntry( "/CLSID/" + classId + "/Version/.", "1.0" );
	    settings.writeEntry( "/CLSID/" + classId + "/VersionIndependentProgID/.", module + "." + className );
	    settings.writeEntry( "/CLSID/" + classId + "/ProgID/.", module + "." + className + ".1" );
	    
	    settings.writeEntry( "/Interface/" + ifaceId + "/.", "I" + className );
	    settings.writeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid/.", "{00020424-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid32/.", "{00020424-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + ifaceId + "/TypeLib/.", libId );
	    settings.writeEntry( "/Interface/" + ifaceId + "/TypeLib/Version", "1.0" );
	    
	    settings.writeEntry( "/Interface/" + eventId + "/.", "I" + className + "Events" );
	    settings.writeEntry( "/Interface/" + eventId + "/ProxyStubClsid/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + eventId + "/ProxyStubClsid32/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + eventId + "/TypeLib/.", libId );
	    settings.writeEntry( "/Interface/" + eventId + "/TypeLib/Version", "1.0" );
	    
	    _Module.factory()->registerClass( *key, &settings );

	}
    } else {
	QStringList keys = _Module.factory()->featureList();
	for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	    const QString classId = _Module.factory()->classID(*key).toString().upper();
	    const QString eventId = _Module.factory()->eventsID(*key).toString().upper();
	    const QString ifaceId = _Module.factory()->interfaceID(*key).toString().upper();
	    const QString className = *key;

	    _Module.factory()->unregisterClass( *key, &settings );
	    
	    settings.removeEntry( "/" + module + "." + className + ".1/CLSID/." );
	    settings.removeEntry( "/" + module + "." + className + ".1/Insertable/." );
	    settings.removeEntry( "/" + module + "." + className + ".1/." );
	    
	    settings.removeEntry( "/" + module + "." + className + "/CLSID/." );
	    settings.removeEntry( "/" + module + "." + className + "/CurVer/." );
	    settings.removeEntry( "/" + module + "." + className + "/." );

	    settings.removeEntry( "/CLSID/" + classId + "/AppID" );
	    settings.removeEntry( "/CLSID/" + classId + "/Control/." );
	    settings.removeEntry( "/CLSID/" + classId + "/Insertable/." );
	    settings.removeEntry( "/CLSID/" + classId + "/InProcServer32/." );
	    settings.removeEntry( "/CLSID/" + classId + "/LocalServer32/." );
	    settings.removeEntry( "/CLSID/" + classId + "/MiscStatus/1/." );
	    settings.removeEntry( "/CLSID/" + classId + "/MiscStatus/." );	    
	    settings.removeEntry( "/CLSID/" + classId + "/Programmable/." );
	    settings.removeEntry( "/CLSID/" + classId + "/ToolboxBitmap32/." );
	    settings.removeEntry( "/CLSID/" + classId + "/TypeLib/." );
	    settings.removeEntry( "/CLSID/" + classId + "/Version/." );
	    settings.removeEntry( "/CLSID/" + classId + "/VersionIndependentProgID/." );
	    settings.removeEntry( "/CLSID/" + classId + "/ProgID/." );
	    settings.removeEntry( "/CLSID/" + classId + "/." );
	    
	    settings.removeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid/." );
	    settings.removeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid32/." );
	    settings.removeEntry( "/Interface/" + ifaceId + "/TypeLib/Version" );
	    settings.removeEntry( "/Interface/" + ifaceId + "/TypeLib/." );
	    settings.removeEntry( "/Interface/" + ifaceId + "/." );
	    
	    settings.removeEntry( "/Interface/" + eventId + "/ProxyStubClsid/." );
	    settings.removeEntry( "/Interface/" + eventId + "/ProxyStubClsid32/." );
	    settings.removeEntry( "/Interface/" + eventId + "/TypeLib/Version" );
	    settings.removeEntry( "/Interface/" + eventId + "/TypeLib/." );
	    settings.removeEntry( "/Interface/" + eventId + "/." );
	}
	settings.removeEntry( "/AppID/" + module + ".EXE/AppID" );
	settings.removeEntry( "/AppID/" + appId + "/." );
    
	settings.removeEntry( "/TypeLib/" + libId + "/1.0/0/win32/." );
	settings.removeEntry( "/TypeLib/" + libId + "/1.0/0/." );
	settings.removeEntry( "/TypeLib/" + libId + "/1.0/FLAGS/." );
	settings.removeEntry( "/TypeLib/" + libId + "/1.0/HELPDIR/." );
	settings.removeEntry( "/TypeLib/" + libId + "/1.0/." );
    }
   
    return S_OK;
}

static QStrList *enums = 0;

static const char* const type_map[][2] =
{
    // QVariant/Qt Value data types
    { "int",		"int" },
    { "uint",		"int" },
    { "bool",		"bool" },
    { "QString",	"BSTR" },
    { "double",		"double" },
    { "QCString",	"BSTR" },
    { "QColor",		"OLE_COLOR" },
    { "QDate",		"DATE" },
    { "QTime",		"DATE" },
    { "QDateTime",	"DATE" },
    { "QFont",		"IFontDisp*" },
    // And we support COM data types
    { "BOOL",		"BOOL" },
    { "BSTR",		"BSTR" },
    { "OLE_COLOR",	"OLE_COLOR" },
    { "DATE",		"DATE" },
    { 0,		0 }
};

static QString convertTypes( const QString &qtype, bool *ok )
{
    *ok = FALSE;

    int i = 0;
    while ( type_map[i][0] ) {
	if ( qtype == type_map[i][0] && type_map[i][1] ) {
	    *ok = TRUE;
	    return type_map[i][1];	    
	}
	++i;
    }
    if ( enums && enums->contains( qtype ) ) {
	*ok = TRUE;
	return "Enum" + qtype;
    }
    return qtype;
}

static const char* const keyword_map[][2] =
{
    { "aggregatable",	"aggregating"	    },
    { "allocate",	"alloc"		    },
    { "appobject",	"appObject"	    },
    { "arrays",		"array"		    },
    { "async",		"asynchronous"	    },
    { "bindable",	"binding"	    },
    { "Boolean",	"boolean"	    },
    { "broadcast",	"broadCast"	    },
    { "callback",	"callBack"	    },
    { "code",		"code_"		    },
    { "control",	"ctrl"		    },
    { "custom"		"custom_"	    },
    { "decode",		"deCode"	    },
    { "default",	"defaulted"	    },
    { "defaultbind",	"defaultBind"	    },
    { "defaultvalue",	"defaultValue"	    },
    { "dual"		"dual_"		    },
    { "encode"		"enCode"	    },
    { "endpoint",	"endPoint"	    },
    { "entry",		"entry_"	    },
    { "helpcontext",	"helpContext"	    },
    { "helpfile",	"helpFile"	    },
    { "helpstring",	"helpString"	    },
    { "hidden",		"isHidden"	    },
    { "id",		"ID"		    },
    { "ignore",		"ignore_"	    },
    { "local",		"local_"	    },
    { "message",	"message_"	    },
    { "notify",		"notify_"	    },
    { "object",		"object_"	    },
    { "optimize",	"optimize_"	    },
    { "optional",	"optional_"	    },
    { "out",		"out_"		    },
    { "pipe",		"pipe_"		    },
    { "proxy",		"proxy_"	    },
    { "ptr",		"pointer"	    },
    { "range",		"range_"	    },
    { "readonly",	"readOnly"	    },
    { "shape",		"shape_"	    },
    { "small",		"small_"	    },
    { "source",		"source_"	    },
    { "string",		"string_"	    },
    { "uuid",		"uuid_"		    },
    { 0,		0		    }
};

static QString replaceKeyword( const QString &name )
{
    int i = 0;
    while ( keyword_map[i][0] ) {
	if ( name == keyword_map[i][0] && keyword_map[i][1] )
	    return keyword_map[i][1];
	++i;
    }
    return name;
}

static QMap<QString, int> *mapping = 0;

static QString renameOverloads( const QString &name )
{
    QString newName = name;

    if ( !mapping )
	mapping = new QMap<QString, int>();

    int n = (*mapping)[name];
    if ( n ) {
	int n = (*mapping)[name];
	newName = name + "_" + QString::number(n);
	(*mapping)[name] = n+1;
    } else {
	(*mapping)[name] = 1;
    }

    return newName;
}

// filter out some properties
static const char* const ignore_props[] =
{
    "name",
    "isTopLevel",
    "isDialog",
    "isModal",
    "isPopup",
    "isDesktop",
    "geometry",
    "pos",
    "frameSize",
    "size",
    "rect",
    "childrenRect",
    "childrenRegion",
    "minimumSize",
    "maximumSize",
    "sizeIncrement",
    "baseSize",
    "ownPalette",
    "ownFont",
    "ownCursor",
    "visibleRect",
    "isActiveWindow",
    "underMouse",
    "visible",
    "hidden",
    "minimized",
    "focus",
    "focusEnabled",
    "customWhatsThis",
    0
};

// filter out some slots
static const char* const ignore_slots[] =
{
    "deleteLater",
    "setMouseTracking",
    "update",
    "repaint",
    "iconify",
    "showMinimized",
    "showMaximized",
    "showFullScreen",
    "showNormal",
    "polish",
    "constPolish",
    "stackUnder",
    0
};

static bool ignore( const char *test, const char *const *table )
{
    int i = 0;
    while ( table[i] ) {
	if ( !strcmp( test, table[i] ) )
	    return TRUE;
	++i;
    }
    return FALSE;
}


#define STRIPCB(x) x = x.mid( 1, x.length()-2 )

HRESULT DumpIDL( const QString &outfile, const QString &ver )
{
    QTextStream out;
    QString outpath = outfile.left( outfile.findRev( "\\" ) );
    QDir dir;
    dir.mkdir( outpath, FALSE );
    QFile file( outfile );
    if ( !file.open( IO_WriteOnly ) )
	qFatal( "Couldn't open %s for writing", outfile.latin1() );
    out.setDevice( &file );

    QString version = ver;
    while ( version.contains( '.' ) > 1 ) {
	int lastdot = version.findRev( '.' );
	version = version.left( lastdot ) + version.right( version.length() - lastdot - 1 );
    }

    QString filebase = module_filename;
    filebase = filebase.left( filebase.findRev( "." ) );
    
    out << "/****************************************************************************" << endl;
    out << "** Interface definition generated from '" << module_filename << "'" << endl;
    out << "**" << endl;
    out << "** Created:  " << QDateTime::currentDateTime().toString() << endl;
    out << "**" << endl;
    out << "** WARNING! All changes made in this file will be lost!" << endl;
    out << "****************************************************************************/" << endl << endl;


    out << "import \"oaidl.idl\";" << endl;
    out << "import \"ocidl.idl\";" << endl;
    out << "#include \"olectl.h\"" << endl << endl << endl;

    // dummy application to create widgets
    int argc;
    QApplication app( argc, 0 );

    QString appID = _Module.factory()->appID().toString().upper();
    STRIPCB(appID);
    QString typeLibID = _Module.factory()->typeLibID().toString().upper();
    STRIPCB(typeLibID);
    QString typelib = filebase.right( filebase.length() - filebase.findRev( "\\" )-1 );

    out << "[" << endl;
    out << "\tuuid(" << typeLibID << ")," << endl;
    out << "\tversion(" << version << ")," << endl;
    out << "\thelpstring(\"" << typelib << " " << version << " Type Library\")" << endl;
    out << "]" << endl;
    out << "library " << typelib << "Lib" << endl;
    out << "{" << endl;
    out << "\timportlib(\"stdole32.tlb\");" << endl;
    out << "\timportlib(\"stdole2.tlb\");" << endl << endl;


    QStringList keys = _Module.factory()->featureList();
    QStringList::Iterator key;
    for ( key = keys.begin(); key != keys.end(); ++key ) {
	delete mapping;
	mapping = 0;
	int id = 1;
	int i;

	QString className = *key;
	QWidget *w = _Module.factory()->create( className );
	QMetaObject *mo = w ? w->metaObject() : 0;
	if ( !mo )
	    return E_FAIL;

	QString topclass = _Module.factory()->exposeToSuperClass( className );

	QMetaObject *pmo = mo;
	do {
	    pmo = pmo->superClass();
	} while ( pmo && topclass != pmo->className() );

	int slotoff = pmo ? pmo->slotOffset() : mo->slotOffset();
	int propoff = pmo ? pmo->propertyOffset() : mo->propertyOffset();
	int signaloff = pmo ? pmo->signalOffset() : mo->signalOffset();

	QAxBindable *bind = (QAxBindable*)w->qt_cast( "QAxBindable" );
	bool isBindable =  bind != 0;
	bool hasStockEvents = FALSE;
	if ( bind )
	    hasStockEvents = bind->hasStockEvents();

	QString classID = _Module.factory()->classID( className ).toString().upper();
	STRIPCB(classID);
	QString interfaceID = _Module.factory()->interfaceID( className ).toString().upper();
	STRIPCB(interfaceID);
	QString eventsID = _Module.factory()->eventsID( className ).toString().upper();
	STRIPCB(eventsID);

#if QT_VERSION >= 0x030100
	QStrList enumerators = mo->enumeratorNames( TRUE );
	for ( i = 0; i < enumerators.count(); ++i ) {
	    if ( !enums )
		enums = new QStrList;

	    const char *enumerator = enumerators.at(i);
	    if ( !enumerator )
		continue;
	    const QMetaEnum *mEnum = mo->enumerator( enumerator, TRUE );
	    if ( !mEnum )
		continue;

	    if ( enums->contains( enumerator ) )
		continue;

	    enums->append( enumerator );
	    
	    out << "\ttypedef enum " << enumerator << " {" << endl;

	    for ( uint j = 0; j < mEnum->count; ++j ) {
		QString key = mEnum->items[j].key;
		key = key.leftJustify( 20 );
		out << "\t\t" << key << "\t= ";
		if ( mEnum->set )
		    out << "0x" << QString::number( mEnum->items[j].value, 16 ).rightJustify( 8, '0' );
		else
		    out << mEnum->items[j].value;
		if ( j < mEnum->count-1 )
		    out << ", ";
		out << endl;
	    }
	    out << "\t} Enum" << enumerator << ";" << endl << endl;
	}
#endif

	out << endl;
	out << "\t[" << endl;
	out << "\t\tuuid(" << interfaceID << ")," << endl;
	out << "\t\thelpstring(\"" << className << " Interface\")," << endl;
	out << "\t]" << endl;
	out << "\tdispinterface I" << className  << endl;
	out << "\t{" << endl;

	out << "\tproperties:" << endl;
	out << "\tmethods:" << endl;
	for ( i = slotoff; i < mo->numSlots( TRUE ); ++i ) {
	    const QMetaData *slotdata = mo->slot( i, TRUE );
	    if ( !slotdata || slotdata->access != QMetaData::Public )
		continue;

	    bool ok = TRUE;
	    if ( ignore( slotdata->method->name, ignore_slots ) )
		continue;

	    QString slotName = renameOverloads( replaceKeyword( slotdata->method->name ) );
	    QString slot = slotName ;
	    slot += "(";
	    for ( int p = 0; p < slotdata->method->count && ok; ++p ) {
		slot += convertTypes( slotdata->method->parameters[p].type->desc(), &ok );
		if ( slotdata->method->parameters[p].name )
		    slot += " _" + replaceKeyword( slotdata->method->parameters[p].name );
		else
		    slot += " p" + QString::number(p);
		if ( p+1 < slotdata->method->count )
		    slot += ", ";
	    }
	    slot += ")";
	    
	    if ( !ok )
		out << "\t/****** Slot parameter uses unsupported datatype" << endl;

	    out << "\t\t[id(" << id << ")] ";
	    out << "HRESULT " << slot << ";" << endl;
	    
	    if ( !ok )
		out << "\t******/" << endl;
	    ++id;
	}
	
	for ( i = propoff; i < mo->numProperties( TRUE ); ++i ) {
	    const QMetaProperty *property = mo->property( i, TRUE );
	    if ( !property || property->testFlags( QMetaProperty::Override ) )
		continue;
	    if ( ignore( property->name(), ignore_props ) )
		continue;

	    bool read = TRUE;
	    bool write = property->writable();
	    bool designable = property->designable( w );
	    bool scriptable = isBindable ? property->scriptable( w ) : FALSE;
	    bool ok = TRUE;
#if QT_VERSION >= 0x030100
	    QString type = convertTypes( property->type(), &ok );
#else
	    QString type = convertTypes( property->isEnumType() ? "int" : property->type(), &ok );
#endif
	    QString name = replaceKeyword( property->name() );

	    if ( !ok )
		out << "\t/****** Property is of unsupported datatype" << endl;

	    if ( read ) {
		out << "\t\t[id(" << id << "), propget";
		if ( scriptable )
		    out << ", bindable";
		if ( !designable )
		    out << ", nonbrowsable";
		if ( isBindable )
		    out << ", requestedit";
		out << "] " << type << " " << name << "();" << endl;
	    }
	    if ( write ) {
		out << "\t\t[id(" << id << "), propput";
		if ( scriptable )
		    out << ", bindable";
		if ( !designable )
		    out << ", nonbrowsable";
		if ( isBindable )
		    out << ", requestedit";
		out << "] HRESULT " << name << "(" << type << " newVal);" << endl;
	    }

	    if ( !ok )
		out << "\t******/" << endl;
	    ++id;
	}
	out << "\t};" << endl << endl;

	delete mapping;
	mapping = 0;
	id = 1;

	out << "\t[" << endl;
	out << "\t\tuuid(" << eventsID << ")," << endl;
	out << "\t\thelpstring(\"" << className << " Events Interface\")" << endl;
	out << "\t]" << endl;
	out << "\tdispinterface I" << className << "Events" << endl;
	out << "\t{" << endl;
	out << "\tproperties:" << endl;
	out << "\tmethods:" << endl;

	if ( hasStockEvents ) {
	    out << "\t/****** Stock events ******/" << endl;
	    out << "\t\t[id(DISPID_CLICK)] void Click();" << endl;
	    out << "\t\t[id(DISPID_DBLCLICK)] void DblClick();" << endl;
	    out << "\t\t[id(DISPID_KEYDOWN)] void KeyDown(short* KeyCode, short Shift);" << endl;
	    out << "\t\t[id(DISPID_KEYPRESS)] void KeyPress(short* KeyAscii);" << endl;
	    out << "\t\t[id(DISPID_KEYUP)] void KeyUp(short* KeyCode, short Shift);" << endl;
	    out << "\t\t[id(DISPID_MOUSEDOWN)] void MouseDown(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl;
	    out << "\t\t[id(DISPID_MOUSEMOVE)] void MouseMove(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl;
	    out << "\t\t[id(DISPID_MOUSEUP)] void MouseUp(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl << endl;
	}

	for ( i = signaloff; i < mo->numSignals( TRUE ); ++i ) {
	    const QMetaData *signaldata = mo->signal( i, TRUE );
	    if ( !signaldata )
		continue;

	    bool ok = TRUE;
	    QString signal = renameOverloads( replaceKeyword( signaldata->method->name ) );
	    signal += "(";
	    for ( int p = 0; p < signaldata->method->count && ok; ++p ) {
		signal += convertTypes( signaldata->method->parameters[p].type->desc(), &ok );
		if ( signaldata->method->parameters[p].name ) 
		    signal += " _" + replaceKeyword( signaldata->method->parameters[p].name );
		else
		    signal += " p" + QString::number(p);
		if ( p+1 < signaldata->method->count )
		    signal += ", ";
	    }
	    signal += ")";

	    if ( !ok )
		out << "\t/****** Signal parameter uses unsupported datatype" << endl;

	    out << "\t\t[id(" << id << ")] void " << signal << ";" << endl;

	    if ( !ok )
		out << "\t******/" << endl;
	    ++id;
	}
	out << "\t};" << endl << endl;
	
	out << "\t[" << endl;
	out << "\t\tuuid(" << classID << ")," << endl;
	out << "\t\thelpstring(\"" << className << " Class\")" << endl;
	out << "\t]" << endl;
	out << "\tcoclass " << className << endl;
	out << "\t{" << endl;
	out << "\t\t[default] interface I" << className << ";" << endl;
	out << "\t\t[default, source] dispinterface I" << className << "Events;" << endl;
	out << "\t};" << endl;

	delete w;
    }

    out << "};" << endl;

    delete mapping;
    mapping = 0;
    delete enums;
    enums = 0;

    return S_OK;
}

QInterfacePtr<QAxFactoryInterface> CExeModule::_factory = 0;
bool is_server = FALSE;

/////////////////////////////////////////////////////////////////////////////
//
#if defined( Q_OS_TEMP )
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
#endif

extern "C" int WINAPI WinMain(HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GetModuleFileNameA( 0, module_filename, MAX_PATH-1 );

    lpCmdLine = GetCommandLineA(); //this line necessary for _ATL_MIN_CRT
    QString cmdLine = QString::fromLatin1( lpCmdLine );

    QStringList cmds = QStringList::split( " ", cmdLine );
    int nRet = 0;
    bool bRun = TRUE;
    bool bRunMain = TRUE;
    for ( QStringList::Iterator it = cmds.begin(); it != cmds.end(); ++it ) {
	QString cmd = (*it).lower();
	if ( cmd == "-activex" || cmd == "/activex" ) {
	    bRunMain = FALSE;
	} else if ( cmd == "-unregserver" || cmd == "/unregserver" ) {
	    qWarning( "Unregistering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(FALSE);
            bRun = FALSE;
	    break;
	} else if ( cmd == "-regserver" || cmd == "/regserver" ) {
	    qWarning( "Registering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(TRUE);
            bRun = FALSE;
            break;
	} else if ( cmd == "-dumpidl" || cmd == "/dumpidl" ) {
	    ++it;
	    if ( it != cmds.end() ) {
		QString outfile = *it;
		++it;
		QString version;
		if ( it != cmds.end() && ( *it == "-version" || *it == "/version" ) ) {
		    ++it;
		    if ( it != cmds.end() )
			version = *it;
		    else
			version = "1.0";
		}

		nRet = DumpIDL( outfile, version );
	    } else {
		qWarning( "Wrong commandline syntax: <app> -dumpidl <idl file> [-version <x.y.z>]" );
	    }
	    bRun = FALSE;
	    break;
	}
    }

    if (bRun) {
	int argc;
	char* cmdp = 0;
	cmdp = new char[ cmdLine.length() + 1 ];
	qstrcpy( cmdp, cmdLine.latin1() );

	QMemArray<pchar> argv( 8 );
	qWinMain( hInstance, hPrevInstance, cmdp, nShowCmd, argc, argv );
	if ( bRunMain ) {
	    nRet = main( argc, argv.data() );
	} else {
	    HRESULT hRes = CoInitialize(NULL);

	    _ASSERTE(SUCCEEDED(hRes));
	    QStringList keys = _Module.factory()->featureList();
	    if ( !keys.count() )
		return nRet;
	    _ATL_OBJMAP_ENTRY *ObjectMap = new _ATL_OBJMAP_ENTRY[ keys.count() + 1 ];
	    memset( ObjectMap+keys.count(), 0, sizeof(_ATL_OBJMAP_ENTRY) );
	    int object = 0;
	    for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
		GUID *clsid = new GUID;
		*clsid = _Module.factory()->classID( *key );
		ObjectMap[object].pclsid = clsid;
		ObjectMap[object].pfnUpdateRegistry = UpdateRegistry;
		ObjectMap[object].pfnGetClassObject = GetClassObject;
		ObjectMap[object].pfnCreateInstance = CreateInstance;
		ObjectMap[object].pCF = NULL;
		ObjectMap[object].dwRegister = 0;
		ObjectMap[object].pfnGetObjectDescription = QAxServerBase::GetObjectDescription;
		ObjectMap[object].pfnGetCategoryMap = QAxServerBase::GetCategoryMap;
		ObjectMap[object].pfnObjectMain = QAxServerBase::ObjectMain;
		++object;
	    }

	    const IID TypeLib = _Module.factory()->typeLibID();
	    _Module.Init(ObjectMap, hInstance, &TypeLib );
	    _Module.dwThreadID = GetCurrentThreadId();

	    _Module.StartMonitor();

	    _ATL_OBJMAP_ENTRY* pEntry = _Module.m_pObjMap;
	    while ( pEntry->pclsid )
	    {
		IUnknown* p = 0;
		HRESULT hRes = GetClassObject( pEntry->pfnCreateInstance, *pEntry->pclsid, (void**) &p );
		if (SUCCEEDED(hRes))
		    hRes = CoRegisterClassObject( *pEntry->pclsid, p, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &pEntry->dwRegister );
		if ( p )
		    p->Release();

		++pEntry;
	    }

	    is_server = TRUE;
	    nRet = main( argc, argv.data() );

	    pEntry = _Module.m_pObjMap;
	    while ( pEntry->pclsid )
	    {
		    pEntry->RevokeClassObject();
		    ++pEntry;
	    }
	    Sleep(dwPause); //wait for any threads to finish

	    _Module.Term();
	    if ( QAxServerBase::typeInfoHolderList ) {
		QPtrListIterator<CComTypeInfoHolder> it( *QAxServerBase::typeInfoHolderList );
		while ( it.current() ) {
		    CComTypeInfoHolder *pth = it.current();
		    delete (GUID*)pth->m_pguid;
		    pth->m_pguid = 0;
		    delete (GUID*)pth->m_plibid;
		    pth->m_plibid = 0;
		    ++it;
		}
	    }
	    delete QAxServerBase::typeInfoHolderList;
	    QAxServerBase::typeInfoHolderList = 0;

	    CoUninitialize();

	    object = 0;
	    while ( ObjectMap[object].pclsid ) {
		delete (GUID*)ObjectMap[object].pclsid;
		object++;
	    }
	    delete[] ObjectMap;
	}
	delete[] cmdp;
    }

    return nRet;
}

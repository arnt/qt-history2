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

#include "qaxbindable.h"
#include "qaxfactory.h"
#include "../shared/types.h"

#ifdef QT_DEBUG
const DWORD dwTimeOut = 1000;
const DWORD dwPause = 500;
#else
const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 1000; // time to wait for threads to finish up
#endif

// Some global variables to store module information
bool qAxIsServer = FALSE;
HANDLE qAxInstance = 0;
ITypeLib *qAxTypeLibrary = 0;
char qAxModuleFilename[MAX_PATH];

// The QAxFactory instance
static QAxFactoryInterface* _factory;

// Some local variables to handle module lifetime
static bool qAxActivity = FALSE;
static long qAxModuleRef = 0;
static HANDLE hEventShutdown;
static CRITICAL_SECTION qAxModuleSection;
static DWORD dwThreadID;

void qAxInit()
{
    InitializeCriticalSection( &qAxModuleSection );

    QString libFile( qAxModuleFilename );
    BSTR oleLibFile = QStringToBSTR( libFile );
    LoadTypeLibEx( oleLibFile, REGKIND_NONE, &qAxTypeLibrary );
    SysFreeString( oleLibFile );
}

void qAxCleanup()
{
    if ( _factory ) {
	_factory->release();
	_factory = 0;
    }

    if ( qAxTypeLibrary ) {
	qAxTypeLibrary->Release();
	qAxTypeLibrary = 0;
    }

    DeleteCriticalSection( &qAxModuleSection );
}

unsigned long qAxLock()
{
    EnterCriticalSection( &qAxModuleSection );
    unsigned long ref = ++qAxModuleRef;
    LeaveCriticalSection( &qAxModuleSection );
    return ref;
}

unsigned long qAxUnlock()
{
    EnterCriticalSection( &qAxModuleSection );
    unsigned long ref = --qAxModuleRef;
    LeaveCriticalSection( &qAxModuleSection );

    if ( !ref ) {
        qAxActivity = TRUE;
        if ( hEventShutdown )
	    SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
    }
    return ref;
}

unsigned long qAxLockCount()
{
    return qAxModuleRef;
}

// Monitors the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    while (1) {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do {
            qAxActivity = FALSE;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while ( dwWait == WAIT_OBJECT_0 );
        // timed out
        if ( !qAxActivity && !qAxModuleRef ) // if no activity let's really bail
            break;
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    PostQuitMessage( 0 );

    return 0;
}

// Starts the monitoring thread
static bool StartMonitor()
{
    dwThreadID = GetCurrentThreadId();
    hEventShutdown = CreateEventA( 0, FALSE, FALSE, 0 );
    if ( hEventShutdown == 0 )
        return FALSE;
    DWORD dwThreadID;
    HANDLE h = CreateThread( 0, 0, MonitorProc, 0, 0, &dwThreadID );
    return (h != NULL);
}

extern QUnknownInterface *ucm_instantiate();
extern HRESULT __stdcall GetClassObject( void *pv, const GUID &iid, void **ppUnk );

QAxFactoryInterface *qAxFactory()
{
    if ( !_factory ) {
	QUnknownInterface *unknown = ucm_instantiate();
	if ( unknown ) {
	    unknown->queryInterface( IID_QAxFactory, (QUnknownInterface**)&_factory );
	    unknown->release();
	}
    }
    return _factory;
}



// (Un)Register the ActiveX server in the registry.
// The QAxFactory implementation provides the information.
HRESULT WINAPI UpdateRegistry(BOOL bRegister)
{
    QString file = QString::fromLocal8Bit( qAxModuleFilename );
    QString path = file.left( file.findRev( "\\" )+1 );
    QString module = file.right( file.length() - path.length() );
    module = module.left( module.findRev( "." ) );

    const QString appId = qAxFactory()->appID().toString().upper();
    const QString libId = qAxFactory()->typeLibID().toString().upper();

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );

    if ( bRegister ) {
	if ( file.right( 3 ).lower() == "exe" ) {
	    settings.writeEntry( "/AppID/" + appId + "/.", module );
	    settings.writeEntry( "/AppID/" + module + ".EXE/AppID", appId );
	}

	settings.writeEntry( "/TypeLib/" + libId + "/1.0/0/win32/.", file );
	settings.writeEntry( "/TypeLib/" + libId + "/1.0/FLAGS/.", "0" );
	settings.writeEntry( "/TypeLib/" + libId + "/1.0/HELPDIR/.", path );
	settings.writeEntry( "/TypeLib/" + libId + "/1.0/.", module + " 1.0 Type Library" );

	QStringList keys = qAxFactory()->featureList();
	for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	    const QString className = *key;
	    const QMetaObject *mo = QMetaObject::metaObject( className.latin1() );

	    const QString classId = qAxFactory()->classID(className).toString().upper();
	    const QString eventId = qAxFactory()->eventsID(className).toString().upper();
	    const QString ifaceId = qAxFactory()->interfaceID(className).toString().upper();
	    QString classVersion = mo ? QString(mo->classInfo( "VERSION" )) : QString::null;
	    if ( classVersion.isNull() )
		classVersion = "1.0";
	    const QString classMajorVersion = classVersion.left( classVersion.find(".") );

	    settings.writeEntry( "/" + module + "." + className + "." + classMajorVersion + "/.", className + " Class" );
	    settings.writeEntry( "/" + module + "." + className + "." + classMajorVersion + "/CLSID/.", classId );
	    settings.writeEntry( "/" + module + "." + className + "." + classMajorVersion + "/Insertable/.", QString::null );

	    settings.writeEntry( "/" + module + "." + className + "/.", className + " Class" );
	    settings.writeEntry( "/" + module + "." + className + "/CLSID/.", classId );
	    settings.writeEntry( "/" + module + "." + className + "/CurVer/.", module + "." + className + "." + classMajorVersion );

	    settings.writeEntry( "/CLSID/" + classId + "/.", className + " Class" );
	    if ( file.right( 3 ).lower() == "exe" )
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
	    settings.writeEntry( "/CLSID/" + classId + "/Version/.", classVersion );
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

	    qAxFactory()->registerClass( className, &settings );
	}
    } else {
	QStringList keys = qAxFactory()->featureList();
	for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	    const QString className = *key;
	    const QMetaObject *mo = QMetaObject::metaObject( className.latin1() );

	    const QString classId = qAxFactory()->classID(className).toString().upper();
	    const QString eventId = qAxFactory()->eventsID(className).toString().upper();
	    const QString ifaceId = qAxFactory()->interfaceID(className).toString().upper();
	    QString classVersion = mo ? QString(mo->classInfo( "VERSION" )) : QString::null;
	    if ( classVersion.isNull() )
		classVersion = "1.0";
	    const QString classMajorVersion = classVersion.left( classVersion.find(".") );

	    qAxFactory()->unregisterClass( className, &settings );

	    settings.removeEntry( "/" + module + "." + className + "." + classMajorVersion + "/CLSID/." );
	    settings.removeEntry( "/" + module + "." + className + "." + classMajorVersion + "/Insertable/." );
	    settings.removeEntry( "/" + module + "." + className + "." + classMajorVersion + "/." );

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
    { "QString",	"BSTR" },
    { "QCString",	"BSTR" },
    { "bool",		"VARIANT_BOOL" },
    { "int",		"int" },
    { "uint",		"unsigned int" },
    { "double",		"double" }, 
    { "QColor",		"OLE_COLOR" },
    { "QDate",		"DATE" },
    { "QTime",		"DATE" },
    { "QDateTime",	"DATE" },
    { "QFont",		"IFontDisp*" },
    { "QPixmap",	"IPictureDisp*" },
    { "QVariant",	"VARIANT" },
    { "QValueList<QVariant>", "SAFEARRAY(VARIANT)" },
    // And we support COM data types
    { "BOOL",		"BOOL" },
    { "BSTR",		"BSTR" },
    { "OLE_COLOR",	"OLE_COLOR" },
    { "DATE",		"DATE" },
    { "VARIANT",	"VARIANT" },
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
	return "enum " + qtype;
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
    { "Boolean",	"boolval"	    },
    { "boolean",	"boolval"	    },
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

bool ignoreSlots( const char *test )
{
    return ignore( test, ignore_slots );
}

bool ignoreProps( const char *test )
{
    return ignore( test, ignore_props );
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

    QString filebase = qAxModuleFilename;
    filebase = filebase.left( filebase.findRev( "." ) );
    
    out << "/****************************************************************************" << endl;
    out << "** Interface definition generated from '" << qAxModuleFilename << "'" << endl;
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

    QString appID = qAxFactory()->appID().toString().upper();
    STRIPCB(appID);
    QString typeLibID = qAxFactory()->typeLibID().toString().upper();
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


    QStringList keys = qAxFactory()->featureList();
    QStringList::Iterator key;
    for ( key = keys.begin(); key != keys.end(); ++key ) {
	delete mapping;
	mapping = 0;
	int id = 1;
	int i;

	QString className = *key;
	QWidget *w = qAxFactory()->create( className );
	QMetaObject *mo = w ? w->metaObject() : 0;
	if ( !mo )
	    return E_FAIL;

	QString topclass = qAxFactory()->exposeToSuperClass( className );
	bool hasStockEvents = qAxFactory()->hasStockEvents( className );

	QMetaObject *pmo = mo;
	do {
	    pmo = pmo->superClass();
	} while ( pmo && topclass != pmo->className() );

	int slotoff = pmo ? pmo->slotOffset() : mo->slotOffset();
	int propoff = pmo ? pmo->propertyOffset() : mo->propertyOffset();
	int signaloff = pmo ? pmo->signalOffset() : mo->signalOffset();

	QAxBindable *bind = (QAxBindable*)w->qt_cast( "QAxBindable" );
	bool isBindable =  bind != 0;

	QString classID = qAxFactory()->classID( className ).toString().upper();
	STRIPCB(classID);
	QString interfaceID = qAxFactory()->interfaceID( className ).toString().upper();
	STRIPCB(interfaceID);
	QString eventsID = qAxFactory()->eventsID( className ).toString().upper();
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
	    
	    out << "\tenum " << enumerator << " {" << endl;

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
	    out << "\t};" << endl << endl;
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
		out << "\t\t[id(" << id << ")";
		if ( !write )
		    out << ", readonly";
		if ( scriptable )
		    out << ", bindable";
		if ( !designable )
		    out << ", nonbrowsable";
		if ( isBindable )
		    out << ", requestedit";
		out << "] " << type << " " << name << ";" << endl;
	    }

	    if ( !ok )
		out << "\t******/" << endl;
	    ++id;
	}
	out << endl;
	out << "\tmethods:" << endl;
	for ( i = slotoff; i < mo->numSlots( TRUE ); ++i ) {
	    const QMetaData *slotdata = mo->slot( i, TRUE );
	    if ( !slotdata || slotdata->access != QMetaData::Public )
		continue;

	    bool ok = TRUE;
	    if ( ignore( slotdata->method->name, ignore_slots ) )
		continue;

	    QString slot = renameOverloads( replaceKeyword( slotdata->method->name ) );
	    QString returnType = "void ";
	    QString paramType;
	    slot += "(";
	    for ( int p = 0; p < slotdata->method->count && ok; ++p ) {
		const QUParameter *param = slotdata->method->parameters + p;
		bool returnValue = FALSE;

#if QT_VERSION >= 0x030100
		if ( QUType::isEqual( param->type, &static_QUType_varptr ) ) {
		    QVariant::Type vartype = (QVariant::Type)*(int*)param->typeExtra;
		    QCString type = QVariant::typeToName( vartype );
		    paramType = convertTypes( type, &ok );
		} else 
#endif
		if ( QUType::isEqual( param->type, &static_QUType_QVariant ) ) {
		    QVariant::Type vartype = (QVariant::Type)*(int*)param->typeExtra;
		    QCString type = QVariant::typeToName( vartype );
		    paramType = convertTypes( type, &ok );
		} else if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
		    QCString type = (const char*)param->typeExtra;
		    if ( type.right(1) == "&" )
			type = type.left( type.length()-1 );
		    paramType = convertTypes( type, &ok );
		} else if ( QUType::isEqual( param->type, &static_QUType_enum ) ) {
		    const QUEnum *uenum = (const QUEnum*)param->typeExtra;
		    if ( uenum )
			paramType = convertTypes( uenum->name, &ok );
		    else
			ok = FALSE;
		} else {
		    paramType = convertTypes( param->type->desc(), &ok );
		}
		paramType += " ";

		if ( param->inOut == QUParameter::In ) {
		    slot += " [in] " + paramType;
		} else if ( param->inOut == QUParameter::Out ) {
		    if ( p ) {
			slot += " [out] " + paramType;
		    } else {
			returnType = paramType;
			returnValue = TRUE;
		    }
		} else if ( param->inOut ) {
		    slot += " [in,out] " + paramType;
		}

		if ( returnValue )
		    continue;

		if ( param->inOut & QUParameter::Out )
		    slot += "*";
		if ( param->name )
		    slot += "_" + replaceKeyword( param->name );
		else
		    slot += "p" + QString::number( p );
		if ( p+1 < slotdata->method->count )
		    slot += ", ";
	    }
	    slot += ")";

	    if ( !ok )
		out << "\t/****** Slot parameter uses unsupported datatype" << endl;

	    out << "\t\t[id(" << id << ")] ";
	    out << returnType << slot << ";" << endl;
	    
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
	    QString returnType = "void ";
	    QString paramType;
	    signal += "(";
	    for ( int p = 0; p < signaldata->method->count && ok; ++p ) {
		const QUParameter *param = signaldata->method->parameters + p;
		bool returnValue = FALSE;

#if QT_VERSION >= 0x030100
		if ( QUType::isEqual( param->type, &static_QUType_varptr ) ) {
		    int vartable = (QVariant::Type)*(int*)param->typeExtra;
		    QVariant::Type vartype = (QVariant::Type)qt_variant_types[vartable];
		    QCString type = QVariant::typeToName( vartype );
		    paramType = convertTypes( type, &ok );
		} else 
#endif
		    if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
		    QCString type = (const char*)param->typeExtra;
		    if ( type.right(1) == "&" )
			type = type.left( type.length()-1 );
		    paramType = convertTypes( type, &ok );
		} else if ( QUType::isEqual( param->type, &static_QUType_enum ) ) {
		    const QUEnum *uenum = (const QUEnum*)param->typeExtra;
		    if ( uenum )
			paramType = convertTypes( uenum->name, &ok );
		    else
			ok = FALSE;
		} else {
		    paramType = convertTypes( param->type->desc(), &ok );
		}
		paramType += " ";

		if ( param->inOut == QUParameter::In ) {
		    signal += " [in] " + paramType;
		} else if ( param->inOut == QUParameter::Out ) {
		    if ( p ) {
			signal += " [out] " + paramType;
		    } else {
			returnType = paramType;
			returnValue = TRUE;
		    }
		} else if ( param->inOut ) {
		    signal += " [in,out] " + paramType;
		}

		if ( returnValue )
		    continue;

		if ( param->inOut & QUParameter::Out )
		    signal += "*";
		if ( param->name )
		    signal += "_" + replaceKeyword( param->name );
		else
		    signal += "p" + QString::number( p );
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
	out << "\t\thelpstring(\"" << className << " Class\")";
	const char *classVersion = mo->classInfo( "VERSION" );
	if ( classVersion ) {
	    out << "," << endl;
	    out << "\t\tversion(" << classVersion << ")" << endl;
	} else {
	    out << endl;
	}
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

/////////////////////////////////////////////////////////////////////////////
//
typedef int (*QWinEventFilter) (MSG*);
extern int QAxEventFilter( MSG *pMsg );
extern Q_EXPORT QWinEventFilter qt_set_win_event_filter (QWinEventFilter filter);

#if defined(NEEDS_QMAIN)
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
int qMain( int, char ** );
#else
#if defined( Q_OS_TEMP )
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
EXTERN_C int __cdecl main( int, char ** );
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
EXTERN_C int main( int, char ** );
#endif
#endif

EXTERN_C int WINAPI WinMain(HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GetModuleFileNameA( 0, qAxModuleFilename, MAX_PATH-1 );
    qAxInstance = hInstance;

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
	    qAxInit();

	    QStringList keys = qAxFactory()->featureList();
	    if ( !keys.count() )
		return nRet;

	    StartMonitor();

	    int object = 0;
	    DWORD *dwRegister = new DWORD[keys.count()];
	    QStringList::Iterator key;

	    object = 0;
	    for ( key = keys.begin(); key != keys.end(); ++key, ++object ) {
		IUnknown* p = 0;
		CLSID clsid = qAxFactory()->classID( *key );

		// Create a QClassFactory (implemented in qaxserverbase.cpp)
		HRESULT hRes = GetClassObject( 0, clsid, (void**)&p );
		if ( SUCCEEDED(hRes) )
		    hRes = CoRegisterClassObject( clsid, p, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, dwRegister+object );
		if ( p )
		    p->Release();
	    }

	    qAxIsServer = TRUE;

	    QWinEventFilter old = qt_set_win_event_filter( QAxEventFilter );

	    nRet = main( argc, argv.data() );

	    qt_set_win_event_filter( old );

	    object = 0;
	    for ( key = keys.begin(); key != keys.end(); ++key, ++object ) {
		CoRevokeClassObject( dwRegister[object] );
	    }

	    Sleep(dwPause); //wait for any threads to finish

	    qAxCleanup();
	    CoUninitialize();
	}
	delete[] cmdp;
    }

    return nRet;
}

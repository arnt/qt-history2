#include <qapplication.h>
#include <qdir.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qvariant.h>
#include <private/qucomextra_p.h>

#include "qaxbindable.h"
#include "qaxfactory.h"

#include <qt_windows.h>

#ifdef Q_CC_GNU
#include <olectl.h>
#endif

#define Q_REQUIRED_RPCNDR_H_VERSION 475

// Some global variables to store module information
bool qAxIsServer = FALSE;
HANDLE qAxInstance = 0;
ITypeLib *qAxTypeLibrary = 0;
char qAxModuleFilename[MAX_PATH];

// The QAxFactory instance
static QAxFactoryInterface* _factory;
extern QUnknownInterface *ucm_instantiate();
extern CLSID CLSID_QRect;
extern CLSID CLSID_QSize;
extern CLSID CLSID_QPoint;
extern void qax_shutDown();

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

// Some local variables to handle module lifetime
static unsigned long qAxModuleRef = 0;
static CRITICAL_SECTION qAxModuleSection;


/////////////////////////////////////////////////////////////////////////////
// Server control
/////////////////////////////////////////////////////////////////////////////

void qAxInit()
{
    InitializeCriticalSection( &qAxModuleSection );

    QString libFile( qAxModuleFilename );
    libFile = libFile.lower();
    if ( LoadTypeLibEx( (TCHAR*)libFile.ucs2(), REGKIND_NONE, &qAxTypeLibrary ) != S_OK ) {
	int lastDot = libFile.findRev( '.' );
	libFile = libFile.left( lastDot ) + ".tlb";
	LoadTypeLibEx( (TCHAR*)libFile.ucs2(), REGKIND_NONE, &qAxTypeLibrary );
    }
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

    if ( !ref )
	qax_shutDown();
    return ref;
}

unsigned long qAxLockCount()
{
    return qAxModuleRef;
}

/////////////////////////////////////////////////////////////////////////////
// Registry
/////////////////////////////////////////////////////////////////////////////

extern bool qax_disable_inplaceframe;

// (Un)Register the ActiveX server in the registry.
// The QAxFactory implementation provides the information.
HRESULT UpdateRegistry(BOOL bRegister)
{
    QString file = QString::fromLocal8Bit( qAxModuleFilename );
    QString path = file.left( file.findRev( "\\" )+1 );
    QString module = file.right( file.length() - path.length() );
    module = module.left( module.findRev( "." ) );

    const QString appId = qAxFactory()->appID().toString().upper();
    const QString libId = qAxFactory()->typeLibID().toString().upper();

    qAxInit();
    QString typeLibVersion;
    if (qAxTypeLibrary) {
	TLIBATTR *libAttr = 0;
	qAxTypeLibrary->GetLibAttr(&libAttr);
	if (libAttr) {
	    DWORD major = libAttr->wMajorVerNum;
	    DWORD minor = libAttr->wMinorVerNum;
	    typeLibVersion = QString::number(major) + "." + QString::number(minor);
	    qAxTypeLibrary->ReleaseTLibAttr(libAttr);
	}
    }
    if (typeLibVersion.isEmpty())
	typeLibVersion = "1.0";
    qAxCleanup();

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );

    // we try to create the ActiveX widgets later on...
    int argc = 0;
    QApplication app( argc, 0 );

    if ( bRegister ) {
	if ( file.right( 3 ).lower() == "exe" ) {
	    settings.writeEntry( "/AppID/" + appId + "/.", module );
	    settings.writeEntry( "/AppID/" + module + ".EXE/AppID", appId );
	}

	settings.writeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/0/win32/.", file );
	settings.writeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/FLAGS/.", "0" );
	settings.writeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/HELPDIR/.", path );
	settings.writeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/.", module + " " + typeLibVersion + " Type Library" );

	QStringList keys = qAxFactory()->featureList();
	for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	    const QString className = *key;
	    QObject *object = qAxFactory()->createObject( className );
	    const QMetaObject *mo = object ? object->metaObject() : 0;

	    const QString classId = qAxFactory()->classID(className).toString().upper();
	    const QString eventId = qAxFactory()->eventsID(className).toString().upper();
	    const QString ifaceId = qAxFactory()->interfaceID(className).toString().upper();

	    if ( object ) { // don't register subobject classes
		QString classVersion = mo ? QString(mo->classInfo( "Version", TRUE )) : QString::null;
		if ( classVersion.isNull() )
		    classVersion = "1.0";
		bool insertable = mo && !qstricmp(mo->classInfo("Insertable", TRUE), "yes");
		bool control = object->isWidgetType();
		const QString classMajorVersion = classVersion.left( classVersion.find(".") );
		uint olemisc = OLEMISC_SETCLIENTSITEFIRST
			      |OLEMISC_ACTIVATEWHENVISIBLE
			      |OLEMISC_INSIDEOUT
			      |OLEMISC_CANTLINKINSIDE
			      |OLEMISC_RECOMPOSEONRESIZE;
		if (!control)
		    olemisc |= OLEMISC_INVISIBLEATRUNTIME;
		else if (object->child(0, "QMenuBar") && !qax_disable_inplaceframe)
		    olemisc |= OLEMISC_WANTSTOMENUMERGE;

		settings.writeEntry( "/" + module + "." + className + "." + classMajorVersion + "/.", className + " Class" );
		settings.writeEntry( "/" + module + "." + className + "." + classMajorVersion + "/CLSID/.", classId );
		if (insertable)
		    settings.writeEntry( "/" + module + "." + className + "." + classMajorVersion + "/Insertable/.", QString::null );

		settings.writeEntry( "/" + module + "." + className + "/.", className + " Class" );
		settings.writeEntry( "/" + module + "." + className + "/CLSID/.", classId );
		settings.writeEntry( "/" + module + "." + className + "/CurVer/.", module + "." + className + "." + classMajorVersion );

		settings.writeEntry( "/CLSID/" + classId + "/.", className + " Class" );
		if ( file.right( 3 ).lower() == "exe" )
		    settings.writeEntry( "/CLSID/" + classId + "/AppID", appId );
		if (control)
		    settings.writeEntry( "/CLSID/" + classId + "/Control/.", QString::null );
		if (insertable)
		    settings.writeEntry( "/CLSID/" + classId + "/Insertable/.", QString::null );
		if ( file.right( 3 ).lower() == "dll" )
		    settings.writeEntry( "/CLSID/" + classId + "/InProcServer32/.", file );
		else
		    settings.writeEntry( "/CLSID/" + classId + "/LocalServer32/.", file + " -activex" );
		settings.writeEntry( "/CLSID/" + classId + "/MiscStatus/.", control ? "1" : "0" );
		settings.writeEntry( "/CLSID/" + classId + "/MiscStatus/1/.", QString::number(olemisc) );
		settings.writeEntry( "/CLSID/" + classId + "/Programmable/.", QString::null );
		settings.writeEntry( "/CLSID/" + classId + "/ToolboxBitmap32/.", file + ", 101" );
		settings.writeEntry( "/CLSID/" + classId + "/TypeLib/.", libId );
		settings.writeEntry( "/CLSID/" + classId + "/Version/.", classVersion );
		settings.writeEntry( "/CLSID/" + classId + "/VersionIndependentProgID/.", module + "." + className );
		settings.writeEntry( "/CLSID/" + classId + "/ProgID/.", module + "." + className + ".1" );
	    }

	    settings.writeEntry( "/Interface/" + ifaceId + "/.", "I" + className );
	    settings.writeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid32/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + ifaceId + "/TypeLib/.", libId );
	    settings.writeEntry( "/Interface/" + ifaceId + "/TypeLib/Version", typeLibVersion );

	    settings.writeEntry( "/Interface/" + eventId + "/.", "I" + className + "Events" );
	    settings.writeEntry( "/Interface/" + eventId + "/ProxyStubClsid/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + eventId + "/ProxyStubClsid32/.", "{00020420-0000-0000-C000-000000000046}" );
	    settings.writeEntry( "/Interface/" + eventId + "/TypeLib/.", libId );
	    settings.writeEntry( "/Interface/" + eventId + "/TypeLib/Version", typeLibVersion );

	    qAxFactory()->registerClass( className, &settings );
	}
    } else {
	QStringList keys = qAxFactory()->featureList();
	for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	    const QString className = *key;
	    const QMetaObject *mo = qAxFactory()->metaObject(className);

	    const QString classId = qAxFactory()->classID(className).toString().upper();
	    const QString eventId = qAxFactory()->eventsID(className).toString().upper();
	    const QString ifaceId = qAxFactory()->interfaceID(className).toString().upper();
	    QString classVersion = mo ? QString(mo->classInfo( "Version", TRUE )) : QString::null;
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

	settings.removeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/0/win32/." );
	settings.removeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/0/." );
	settings.removeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/FLAGS/." );
	settings.removeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/HELPDIR/." );
	settings.removeEntry( "/TypeLib/" + libId + "/" + typeLibVersion + "/." );
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// IDL generator
/////////////////////////////////////////////////////////////////////////////

static QStrList *enums = 0;
static QStrList *subtypes = 0;

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
    { "Q_ULLONG",	"CY" },
    { "Q_LLONG",	"CY" },
    { "QByteArray",	"SAFEARRAY(BYTE)" },
    { "QStringList",	"SAFEARRAY(BSTR)" },
    // Userdefined Qt datatypes
#if __REQUIRED_RPCNDR_H_VERSION__ >= Q_REQUIRED_RPCNDR_H_VERSION
    { "QRect",		"struct QRect" },
    { "QSize",		"struct QSize" },
    { "QPoint",		"struct QPoint" },
#endif
    // And we support COM data types
    { "BOOL",		"BOOL" },
    { "BSTR",		"BSTR" },
    { "OLE_COLOR",	"OLE_COLOR" },
    { "DATE",		"DATE" },
    { "VARIANT",	"VARIANT" },
    { "IDispatch*",	"IDispatch*" },
    { "IUnknown*",	"IUnknown*" },
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
    { "decode",		"deCode"	    },
    { "default",	"defaulted"	    },
    { "defaultbind",	"defaultBind"	    },
    { "defaultvalue",	"defaultValue"	    },
    { "encode"		"enCode"	    },
    { "endpoint",	"endPoint"	    },
    { "hidden",		"isHidden"	    },
    { "ignore",		"ignore_"	    },
    { "local",		"local_"	    },
    { "notify",		"notify_"	    },
    { "object",		"object_"	    },
    { "optimize",	"optimize_"	    },
    { "optional",	"optional_"	    },
    { "out",		"out_"		    },
    { "pipe",		"pipe_"		    },
    { "proxy",		"proxy_"	    },
    { "ptr",		"pointer"	    },
    { "readonly",	"readOnly"	    },
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
    "frameGeometry",
    "size",
    "sizeHint",
    "minimumSizeHint",
    "microFocusHint",
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
    "shown",
    "windowTransparency",
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
    "setShown",
    "setHidden",
    "move_1",
    "resize_1",
    "setGeometry_1",
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

static HRESULT classIDL( QObject *o, QMetaObject *mo, const QString &className, bool isBindable, QTextStream &out )
{
    int id = 1;
    int i = 0;
    if ( !mo )
	return 3;

    QString topclass = qAxFactory()->exposeToSuperClass( className );
    if (topclass.isEmpty())
	topclass = "QWidget";
    bool hasStockEvents = qAxFactory()->hasStockEvents( className );

    QMetaObject *pmo = mo;
    do {
	pmo = pmo->superClass();
    } while ( pmo && topclass != pmo->className() );

    int slotoff = pmo ? pmo->slotOffset() : mo->slotOffset();
    int propoff = pmo ? pmo->propertyOffset() : mo->propertyOffset();
    int signaloff = pmo ? pmo->signalOffset() : mo->signalOffset();

    int qtProps = 0;
    int qtSlots = 0;

    if (o && o->isWidgetType()) {
	qtProps = QWidget::staticMetaObject()->numProperties(TRUE);
	qtSlots = QWidget::staticMetaObject()->numProperties(TRUE);
    }

    QString classID = qAxFactory()->classID( className ).toString().upper();
    if (QUuid(classID).isNull())
	return 4;
    STRIPCB(classID);
    QString interfaceID = qAxFactory()->interfaceID( className ).toString().upper();
    if (QUuid(interfaceID).isNull())
	return 5;
    STRIPCB(interfaceID);
    QString eventsID = qAxFactory()->eventsID( className ).toString().upper();
    bool hasEvents = !QUuid(eventsID).isNull();
    STRIPCB(eventsID);

    QString defProp(mo->classInfo("DefaultProperty", TRUE));
    QString defSignal(mo->classInfo("DefaultSignal", TRUE));

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

    out << endl;
    out << "\t[" << endl;
    out << "\t\tuuid(" << interfaceID << ")," << endl;
    out << "\t\thelpstring(\"" << className << " Interface\")" << endl;
    out << "\t]" << endl;
    out << "\tdispinterface I" << className  << endl;
    out << "\t{" << endl;

    out << "\tproperties:" << endl;
    for ( i = propoff; i < mo->numProperties( TRUE ); ++i ) {
	const QMetaProperty *property = mo->property( i, TRUE );
	if ( !property || property->testFlags( QMetaProperty::Override ) )
	    continue;
	if ( i <= qtProps && ignore( property->name(), ignore_props ) )
	    continue;
	if ( mo->findProperty( property->name(), TRUE ) > i )
	    continue;

	bool read = TRUE;
	bool write = property->writable();
	bool designable = property->designable( o );
	bool scriptable = isBindable ? property->scriptable( o ) : FALSE;
	bool ok = TRUE;
	QString type = convertTypes( property->type(), &ok );
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
	    if (defProp == name)
		out << ", uidefault";
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
	if ( i <= qtSlots && ignore( slotdata->method->name, ignore_slots ) )
	    continue;

	QString slot = renameOverloads( replaceKeyword( slotdata->method->name ) );
	if (ignore(slot.latin1(), ignore_slots))
	    continue;

	QString returnType = "void ";
	QString paramType;
	slot += "(";
	for ( int p = 0; p < slotdata->method->count && ok; ++p ) {
	    const QUParameter *param = slotdata->method->parameters + p;
	    bool returnValue = FALSE;

	    if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
		QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
		QCString type = QVariant::typeToName( vartype );
		paramType = convertTypes( type, &ok );
	    } else if ( QUType::isEqual( param->type, &static_QUType_QVariant ) && param->typeExtra ) {
		QVariant::Type vartype = (QVariant::Type)*(int*)param->typeExtra;
		QCString type = QVariant::typeToName( vartype );
		paramType = convertTypes( type, &ok );
	    } else if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
		QCString type = (const char*)param->typeExtra;
		if ( type.right(1) == "&" )
		    type = type.left( type.length()-1 );
		paramType = convertTypes( type, &ok );
		if ( !ok )
		    paramType = convertTypes( type+"*", &ok );
		if ( !ok )
		    ok = subtypes && subtypes->find( type ) != -1;
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

	    if ( param->inOut & QUParameter::Out && !paramType.endsWith("** ") )
		slot += "*";
	    if ( param->name )
		slot += "p_" + replaceKeyword( param->name );
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

    if (hasEvents) {
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
	    bool isDefault = defSignal == signal;
	    QString returnType = "void ";
	    QString paramType;
	    signal += "(";
	    for ( int p = 0; p < signaldata->method->count && ok; ++p ) {
		const QUParameter *param = signaldata->method->parameters + p;
		bool returnValue = FALSE;

		if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
		    QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
		    QCString type = QVariant::typeToName( vartype );
		    paramType = convertTypes( type, &ok );
		} else if ( QUType::isEqual( param->type, &static_QUType_QVariant ) && param->typeExtra ) {
		    QVariant::Type vartype = (QVariant::Type)*(int*)param->typeExtra;
		    QCString type = QVariant::typeToName( vartype );
		    paramType = convertTypes( type, &ok );
		} else if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
		    QCString type = (const char*)param->typeExtra;
		    if ( type.right(1) == "&" )
			type = type.left( type.length()-1 );
		    paramType = convertTypes( type, &ok );
		    if ( !ok )
			ok = subtypes && subtypes->find( type ) != -1;
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
		    signal += "p_" + replaceKeyword( param->name );
		else
		    signal += "p" + QString::number( p );
		if ( p+1 < signaldata->method->count )
		    signal += ", ";
	    }
	    signal += ")";

	    if ( !ok )
		out << "\t/****** Signal parameter uses unsupported datatype" << endl;

	    out << "\t\t[id(" << id << ")";
	    if (isDefault)
		out << ", uidefault";
	    out << "] ";
	    out << returnType << signal << ";" << endl;

	    if ( !ok )
		out << "\t******/" << endl;
	    ++id;
	}
	out << "\t};" << endl << endl;
    }

    out << "\t[" << endl;

    if (qstricmp(mo->classInfo("Aggregatable", TRUE), "no"))
	out << "\t\taggregatable," << endl;
    if (mo->classInfo("LicenseKey", TRUE))
	out << "\t\tlicensed," << endl;
    const char *helpString = mo->classInfo("Description", TRUE);
    if (helpString)
	out << "\t\thelpstring(\"" << helpString << "\")," << endl;
    else
	out << "\t\thelpstring(\"" << className << " Class\")," << endl;
    const char *classVersion = mo->classInfo( "Version", TRUE );
    if ( classVersion )
	out << "\t\tversion(" << classVersion << ")," << endl;
    out << "\t\tuuid(" << classID << ")" << endl;
    out << "\t]" << endl;
    out << "\tcoclass " << className << endl;
    out << "\t{" << endl;
    out << "\t\t[default] dispinterface I" << className << ";" << endl;
    if (hasEvents)
	out << "\t\t[default, source] dispinterface I" << className << "Events;" << endl;
    out << "\t};" << endl;

    return S_OK;
}

#if defined(Q_CC_BOR)
extern "C" __stdcall HRESULT DumpIDL( const QString &outfile, const QString &ver )
#else
extern "C" HRESULT __stdcall DumpIDL( const QString &outfile, const QString &ver )
#endif
{
    qAxIsServer = FALSE;
    QTextStream out;
    if (outfile.contains("\\")) {
	QString outpath = outfile.left( outfile.findRev( "\\" ) );
	QDir dir;
	dir.mkdir( outpath, FALSE );
    }
    QFile file( outfile );
    file.remove();

    QString filebase = qAxModuleFilename;
    filebase = filebase.left( filebase.findRev( "." ) );

    QString appID = qAxFactory()->appID().toString().upper();
    if (QUuid(appID).isNull())
	return 1;
    STRIPCB(appID);
    QString typeLibID = qAxFactory()->typeLibID().toString().upper();
    if (QUuid(typeLibID).isNull())
	return 2;
    STRIPCB(typeLibID);
    QString typelib = filebase.right( filebase.length() - filebase.findRev( "\\" )-1 );

    if ( !file.open( IO_WriteOnly ) )
	return -1;

    out.setDevice( &file );

    QString version( ver.unicode(), ver.length() );
    while ( version.contains( '.' ) > 1 ) {
	int lastdot = version.findRev( '.' );
	version = version.left( lastdot ) + version.right( version.length() - lastdot - 1 );
    }
    if (version.isEmpty())
	version = "1.0";

    QString idQRect(QUuid(CLSID_QRect).toString());
    STRIPCB(idQRect);
    QString idQSize(QUuid(CLSID_QSize).toString());
    STRIPCB(idQSize);
    QString idQPoint(QUuid(CLSID_QPoint).toString());
    STRIPCB(idQPoint);

    out << "/****************************************************************************" << endl;
    out << "** Interface definition generated for ActiveQt project" << endl;
    out << "**" << endl;
    out << "**     '" << qAxModuleFilename << "'" << endl;
    out << "**" << endl;
    out << "** Created:  " << QDateTime::currentDateTime().toString() << endl;
    out << "**" << endl;
    out << "** WARNING! All changes made in this file will be lost!" << endl;
    out << "****************************************************************************/" << endl << endl;

    out << "import \"ocidl.idl\";" << endl;
    out << "#include <olectl.h>" << endl << endl;

    // dummy application to create widgets
    int argc;
    QApplication app( argc, 0 );

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

    out << "\t/************************************************************************" << endl;
    out << "\t** If this causes a compile error in MIDL you need to upgrade the" << endl;
    out << "\t** Platform SDK you are using. Download the SDK from msdn.microsoft.com" << endl;
    out << "\t** and make sure that both the system and the Visual Studio environment" << endl;
    out << "\t** use the correct files." << endl;
    out << "\t**" << endl;

#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t** Required version of MIDL could not be verified. QRect, QSize and QPoint" << endl;
    out << "\t** support needs an updated Platform SDK to be installed." << endl;
    out << "\t*************************************************************************" << endl;
#else
    out << "\t************************************************************************/" << endl;
#endif

    out << endl;
    out << "\t[uuid(" << idQRect << ")]" << endl;
    out << "\tstruct QRect {" << endl;
    out << "\t\tint left;" << endl;
    out << "\t\tint top;" << endl;
    out << "\t\tint right;" << endl;
    out << "\t\tint bottom;" << endl;
    out << "\t};" << endl << endl;

    out << "\t[uuid(" << idQSize << ")]" << endl;
    out << "\tstruct QSize {" << endl;
    out << "\t\tint width;" << endl;
    out << "\t\tint height;" << endl;
    out << "\t};" << endl << endl;

    out << "\t[uuid(" << idQPoint << ")]" << endl;
    out << "\tstruct QPoint {" << endl;
    out << "\t\tint x;" << endl;
    out << "\t\tint y;" << endl;
    out << "\t};" << endl;
#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t*/" << endl;
#endif
    out << endl;

    out << "\t/* Forward declaration of classes that might be used as parameters */" << endl << endl;

    int res = S_OK;
    for ( key = keys.begin(); key != keys.end(); ++key ) {
	QString className = *key;
	QMetaObject *mo = qAxFactory()->metaObject( className );
	// We have meta object information for this type. Forward declare it.
	if ( mo ) {
	    out << "\tcoclass " << className << ";" << endl;
	    QObject *o = qAxFactory()->createObject( className );
	    // It's not a control class, so it is actually a subtype. Define it.
	    if ( !o ) {
		if ( !subtypes )
		    subtypes = new QStrList;
		subtypes->append( className );
		subtypes->append( className + "*" );
		res = classIDL( 0, mo, className, FALSE, out );
		if (res != S_OK)
		    break;
	    }
	    delete o;
	}
    }
    out << endl;
    if (res != S_OK)
	goto ErrorInClass;

    for ( key = keys.begin(); key != keys.end(); ++key ) {
	QString className = *key;
	QObject *o = qAxFactory()->createObject( className );
	if ( !o )
	    continue;
	QAxBindable *bind = (QAxBindable*)o->qt_cast( "QAxBindable" );
	bool isBindable =  bind != 0;

	delete mapping;
	mapping = 0;

	if ( !subtypes )
	    subtypes = new QStrList;
	subtypes->append( className );
	subtypes->append( className + "*" );
	res = classIDL( o, o->metaObject(), className, isBindable, out );
	delete o;
	if (res != S_OK)
	    break;
    }

    out << "};" << endl;

ErrorInClass:
    delete mapping;
    mapping = 0;
    delete enums;
    enums = 0;
    delete subtypes;
    subtypes = 0;

    if (res != S_OK) {
	file.close();
	file.remove();
    }

    return res;
}

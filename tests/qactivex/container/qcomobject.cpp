#include "qcomobject.h"

#include <quuid.h>
#include <qdict.h>
#include <qptrdict.h>
#include <qsettings.h>
#include <ctype.h>

#include <atlbase.h>
CComModule _Module;
int moduleLockCount = 0;
void moduleLock()
{
    if ( !moduleLockCount ) {
	CoInitialize(0);
	_Module.Init( 0, GetModuleHandle(0) );	
    }
    ++moduleLockCount;
}
void moduleUnlock()
{
    if ( !moduleLockCount ) {
	qWarning( "Unbalanced module count!" );
	return;
    }
    if ( !--moduleLockCount ) {
	_Module.Term();
	CoUninitialize();
    }
}

static QMetaObject *tempMetaObj = 0;

#define PropDesignable  0x00001000
#define PropScriptable	0x00002000
#define PropStored	0x00004000
#define PropBindable	0x00008000
#define PropRequesting	0x00010000

#include "../shared/types.h"

/*
    \internal
    \class QAxEventSink qcomobject.cpp

    \brief The QAxEventSink class implements the event sink for all
	   IConnectionPoints implemented in the COM object.
*/

class QAxEventSink : public IDispatch,
		     public IPropertyNotifySink
{
public:
    QAxEventSink( QComBase *com ) : combase( com ), ref( 1 ) {}
    virtual ~QAxEventSink() {}

    // add a connection
    void addConnection( IConnectionPoint *cpoint, IID iid ) 
    {	
	ULONG cookie;
	Connection i1( 0, iid, 0 );
	connections.append( i1 );
	cpoint->Advise( (IUnknown*)(IDispatch*)this, &cookie );
	connections.remove( i1 );
	Connection i2( cpoint, iid, cookie );
	connections.append( i2 );
	cpoint->AddRef();
    }
    
    // disconnect from all connection points
    void unadvise()
    {
	QValueListIterator<Connection> it = connections.begin();
	while ( it != connections.end() ) {
	    Connection connection = *it;
	    ++it;
	    connection.cpoint->Unadvise( connection.cookie );
	    connection.cpoint->Release();
	    connections.remove( connection );
	}
    }

    void addSignal( DISPID memid, const QString &name )
    {
	sigs.insert( memid, name );
	QMap<DISPID,QString>::Iterator it;
	DISPID id = -1;
	for ( it = propsigs.begin(); it!= propsigs.end(); ++it ) {
	    if ( it.data() == name ) {
		id = it.key();
		break;
	    }
	}
	if ( id != -1 )
	    propsigs.remove( id );
    }
    void addProperty( DISPID propid, const QString &name, const QString &signal )
    {
	props.insert( propid, name );
	propsigs.insert( propid, signal );
    }

    // IUnknown
    unsigned long __stdcall AddRef()
    {
	return ref++;
    }
    unsigned long __stdcall Release()
    {
	if ( !--ref ) {
	    delete this;
	    return 0;
	}
	return ref;
    }
    HRESULT __stdcall QueryInterface( REFIID riid, void **ppvObject )
    {
	*ppvObject = 0;
	if ( riid == IID_IUnknown)
	    *ppvObject = (IUnknown*)(IDispatch*)this;
	else if ( riid == IID_IDispatch )
	    *ppvObject = (IDispatch*)this;
	else if ( riid == IID_IPropertyNotifySink )
	    *ppvObject = (IPropertyNotifySink*)this;
	else if ( connections.contains( Connection(0,riid,0) ) )
	    *ppvObject = (IDispatch*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    // IDispatch
    HRESULT __stdcall GetTypeInfoCount( unsigned int *count ) { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo( UINT, LCID, ITypeInfo **info ) { return E_NOTIMPL; }
    HRESULT __stdcall GetIDsOfNames( const _GUID &, wchar_t **, unsigned int, unsigned long, long * ) { return E_NOTIMPL; }

    HRESULT __stdcall Invoke( DISPID dispIdMember, 
			      REFIID riid, 
			      LCID lcid, 
			      WORD wFlags, 
			      DISPPARAMS *pDispParams, 
			      VARIANT *pVarResult, 
			      EXCEPINFO *pExcepInfo, 
			      UINT *puArgErr )
    {
	// verify input
	if ( riid != IID_NULL )
	    return DISP_E_UNKNOWNINTERFACE;
	if ( wFlags != DISPATCH_METHOD )
	    return DISP_E_MEMBERNOTFOUND;

	QMetaObject *meta = combase->metaObject();
	QString signame = sigs[dispIdMember];
	if ( !meta || signame.isEmpty() )
	    return DISP_E_MEMBERNOTFOUND;

	QObject *qobject = combase->qObject();
	if ( qobject->signalsBlocked() )
	    return S_OK;

	// emit the signal "as is"
	int index = meta->findSignal( "signal(const QString&,int,void*)" );
	if ( index != -1 && ((QComObject*)qobject)->receivers( index ) ) {
	    QUObject o[4];
	    static_QUType_QString.set(o+1,signame);
	    static_QUType_int.set(o+2,pDispParams->cArgs);
	    static_QUType_ptr.set(o+3,pDispParams->rgvarg);
	    combase->qt_emit( index, o );
	    static_QUType_ptr.clear(o+3);
	    static_QUType_int.clear(o+2);
	    static_QUType_QString.clear(o+1);
	}

	// get the signal information from the metaobject
	index = meta->findSignal( signame );
	if ( index != -1 && ((QComObject*)qobject)->receivers( index ) ) {
	    const QMetaData *signal = meta->signal( index - meta->signalOffset() );
	    if ( !signal )
		return DISP_E_MEMBERNOTFOUND;

	    // verify parameter count
	    int pcount = signal->method->count;
	    int argcount = pDispParams->cArgs;
	    if ( pcount > argcount )
		return DISP_E_PARAMNOTOPTIONAL;
	    else if ( pcount < argcount )
		return DISP_E_BADPARAMCOUNT;

	    // setup parameters
	    const QUParameter *params = signal->method->parameters;
	    QUObject *objects = pcount ? new QUObject[pcount+1] : 0;
	    int p;
	    for ( p = 0; p < pcount; ++p ) // map the VARIANT to the QUObject
		VARIANTToQUObject( pDispParams->rgvarg[ pcount-p-1 ], objects + p + 1 );

	    // emit the generated signal
	    bool ret = combase->qt_emit( index, objects );

	    for ( p = 0; p < pcount; ++p ) { // update the VARIANT for references and free memory
		VARIANT *arg = &(pDispParams->rgvarg[ pcount-p-1 ]);
		VARTYPE vt = arg->vt;
		QUObject *obj = objects + p + 1;
		switch ( vt )
		{
		case VT_DATE:
		case VT_DATE|VT_BYREF:
		    {
			QDateTime *dt = (QDateTime*)static_QUType_ptr.get( obj );
			if ( vt & VT_BYREF )
			    *arg->pdate = QDateTimeToDATE( *dt );
			delete dt;
		    }
		    break;
		case VT_BSTR|VT_BYREF:
		    {
			BSTR *bstr = arg->pbstrVal;
			QString *str = (QString*)static_QUType_ptr.get( obj );
			*bstr = QStringToBSTR( *str );
			delete str;
		    }
		case VT_UNKNOWN|VT_BYREF:
		    {
			IUnknown *iface = (IUnknown*)static_QUType_ptr.get( obj );
			*arg->ppunkVal = iface;
		    }
		    break;

		case VT_DISPATCH|VT_BYREF:
		    {
			IDispatch *iface = (IDispatch*)static_QUType_ptr.get( obj );
			*arg->ppdispVal = iface;
		    }
		    break;

		default:
		    break;
		}
	    }
	    // cleanup
	    for ( p = 0; p < pcount; ++p ) {
		objects[p].type->clear(objects+p);
	    }
	    delete [] objects;
	    return ret ? S_OK : DISP_E_MEMBERNOTFOUND;
	} else {
	    return S_OK;
	}
    }

    // IPropertyNotifySink
    HRESULT __stdcall OnChanged( DISPID dispID )
    {
	// verify input
	if ( dispID == DISPID_UNKNOWN ) {
	    return S_OK;
	}

	QMetaObject *meta = combase->metaObject();
	QString propname = props[dispID];
	if ( !meta || propname.isEmpty() )
	    return S_OK;

	QObject *qobject = combase->qObject();
	if ( qobject->signalsBlocked() )
	    return S_OK;

	// emit the generic signal
	int index = meta->findSignal( "propertyChanged(const QString&)" );
	if ( index != -1 && ((QComObject*)qobject)->receivers( index ) ) {
	    QUObject o[2];
	    static_QUType_QString.set(o+1,propname);
	    combase->qt_emit( index, o );
	    static_QUType_QString.clear(o+1);
	}

	QString signame = propsigs[dispID];
	if ( signame.isEmpty() )
	    return S_OK;
	// get the signal information from the metaobject
	index = meta->findSignal( signame );
	if ( index != -1 && ((QComObject*)qobject)->receivers( index ) ) {
	    const QMetaData *signal = meta->signal( index - meta->signalOffset() );
	    if ( !signal || signal->method->count != 1 )
		return S_OK;

	    // setup parameters
	    int pindex = meta->findProperty( propname );
	    QUObject o[2];
	    QVariant var;
	    combase->qt_property( pindex + meta->propertyOffset(), 1, &var );
	    if ( !var.isValid() )
		return S_OK;
	    if ( QUType::isEqual( signal->method->parameters->type, &static_QUType_QVariant ) )
		static_QUType_QVariant.set( o+1, var );
	    else
		QVariantToQUObject( var, o[1] );

	    // emit the "changed" signal
	    combase->qt_emit( index, o );
	    o[1].type->clear(o+1);
	}
	return S_OK;
    }
    HRESULT __stdcall OnRequestEdit( DISPID dispID )
    {
	if ( dispID == DISPID_UNKNOWN ) {
	    return S_OK;
	}
	QString propname = props[dispID];
	return combase->propertyWritable( propname ) ? S_OK : S_FALSE;
    }

private:
    struct Connection
    {
	Connection() {}
	Connection( IConnectionPoint *point, IID i, ULONG c ) 
	    : cpoint( point ), iid(i), cookie(c) 
	{}

	IConnectionPoint *cpoint;
	IID iid;
	ULONG cookie;

	bool operator==( const Connection &other ) const { return iid == other.iid; }
    };
    QValueList<Connection> connections;
    QMap<DISPID, QString> sigs;
    QMap<DISPID, QString> propsigs;
    QMap<DISPID, QString> props;

    QComBase *combase;
    long ref;
};



/*!
    \class QComBase qcomobject.h

    \brief The QComBase class is an abstract class that provides an API to initalize and access a COM object.

    \extension QActiveX

    QComBase is an abstract class that cannot be used directly, and is instantiated through the subclasses 
    QComObject and QActiveX. This class however provides the API to access the COM object directly through 
    its IUnknown implementation. If the COM object implements the IDispatch interface, the properties, 
    methods and events of that object become available as Qt properties, slots and signals.

    COM interfaces implemented by the object can be retrieved through queryInterface().

    Properties exposed by the object's IDispatch implementation can be read and written through the property
    system provided by the Qt Object Model (both subclasses are QObjects, so you can use 
    \link QObject::setProperty \endlink setProperty and \link QObject::property \endlink property as with
    QObject).

    Write-functions for properties and other methods exposed by the object's IDispatch implementation can be 
    called directly using dynamicCall(), or indirectly as slots connected to a signal.

    Outgoing events supported by the COM object are emitted as standard Qt signals.

    QComBase transparently converts between Qt data type and the equivalent COM data types. Some COM types,
    for example the VARIANT type VT_CY, has no equivalent Qt data structure. Therefore, QComBase provides 
    a generic signal that delivers the event data as delivered by the COM object. If you need to access 
    properties of unsupported datatypes you have to access the COM object directly through its IDispatch or 
    other interfaces implemented.
*/

/*! 
    \enum QComBase::PropertyBag

    A QMap<QString,QVariant> that can store name - value pairs of properties.
*/

/*!
    Creates a QComBase object that wraps the COM object \a iface. If \a iface is null (the default),
    use setControl() to instantiate a COM object.
*/
QComBase::QComBase( IUnknown *iface )
: ptr( iface ), eventSink( 0 ), metaobj( 0 ), propWritable( 0 )
{
    if ( ptr )
	ptr->AddRef();
}

/*!
    Shuts down the COM object and destroys the QComBase object.

    \sa clear()
*/
QComBase::~QComBase()
{
    clear();

    delete propWritable;
    propWritable = 0;
}

/*!
    \property QComBase::control
    \brief the name of the COM object wrapped by this QComBase object.

    Setting this property initilializes the COM object. Any COM object previously set is shut down.

    The most efficient way to set this property is by using the UUID of the registered component, e.g.
    \code
    ctrl->setControl( "{8E27C92B-1264-101C-8A2F-040224009C02}" );
    \endcode
    The second fastest way is to use the class name of the registered control (with or without version number), e.g.
    \code
    ctrl->setControl( "MSCal.Calendar" );
    \endcode
    The slowest, but easiest way to use is to use the full name of the control, e.g.
    \code
    ctrl->setControl( "Calendar Control 9.0" );
    \endcode
    
    The read function of the control always returns the UUID of the control.
*/
bool QComBase::setControl( const QString &c )
{
    if ( c == ctrl )
	return !ctrl.isEmpty();

    clear();
    ctrl = c;
    QUuid uuid( ctrl );
    if ( uuid.isNull() ) {
	QSettings controls;
	ctrl = controls.readEntry( "/Classes/" + c + "/CLSID/Default" );
	if ( ctrl.isEmpty() ) {
	    QStringList clsids = controls.subkeyList( "/Classes/CLSID" );
	    for ( QStringList::Iterator it = clsids.begin(); it != clsids.end(); ++it ) {
		QString clsid = *it;
		QStringList subkeys = controls.subkeyList( "/Classes/CLSID/" + clsid );
		if ( subkeys.contains( "Control" ) ) {
		    QString name = controls.readEntry( "/Classes/CLSID/" + clsid + "/Default" );
		    if ( name == c ) {
			ctrl = clsid;
			break;
		    }
		}
	    }
	}
    }
    if ( ctrl.isEmpty() )
	ctrl = c;
    initialize( &ptr );
    if ( isNull() ) {
#ifndef QT_NO_DEBUG
	qWarning( "QComBase::setControl: requested control %s could not be instantiated.", c.latin1() );
#endif
	clear();
	return FALSE;
    }
    return TRUE;
}

QString QComBase::control() const
{
    return ctrl;
}

/*!
    Disconnects and destroys the COM object.

    If you reimplement this function you will also have to reimplement
    the destructor to call clear(), and call this implementation at the
    end of your clear() function.
*/
void QComBase::clear()
{
    if ( eventSink ) {
	eventSink->unadvise();
	eventSink->Release();
	eventSink = 0;
    }

    if ( ptr ) {
	ptr->Release();
	ptr = 0;
	moduleUnlock();
    }

    ctrl = QString::null;

    if ( metaobj ) {
	int i;
	// clean up class info
	for ( i = 0; i < metaobj->numClassInfo(); ++i ) {
	    QClassInfo *info = (QClassInfo*)metaobj->classInfo( i );
	    delete [] (char*)info->name;
	    delete [] (char*)info->value;
	}
	if ( metaobj->numClassInfo() )
	    delete [] (QClassInfo*)metaobj->classInfo( 0 );

	// clean up slot info
	for ( i = 0; i < metaobj->numSlots(); ++i ) {
	    const QMetaData *slot_data = metaobj->slot( i );
	    QUMethod *slot = (QUMethod*)slot_data->method;
	    if ( slot ) {
		delete [] (char*)slot->name;
		for ( int p = 0; p < slot->count; ++p ) {
		    const QUParameter *param = &(slot->parameters[p]);
		    delete [] (char*)param->name;
		    if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
			const char *type = (const char*)param->typeExtra;
			delete [] (char*)type;
		    }
		}
		delete [] (QUParameter*)slot->parameters;
		delete slot;
	    }
	    delete [] (char*)slot_data->name;
	}
	if ( metaobj->numSlots() )
	    delete [] (QMetaData*)metaobj->slot( 0 );

	// clean up signal info
	for ( i = 2; i < metaobj->numSignals(); ++i ) { // 0 and 1 are static signals
	    const QMetaData *signal_data = metaobj->signal( i );
	    QUMethod *signal = (QUMethod*)signal_data->method;
	    if ( signal ) {
		delete [] (char*)signal->name;
		for ( int p = 0; p < signal->count; ++p ) {
		    const QUParameter *param = &(signal->parameters[p]);
		    delete [] (char*)param->name;
		    if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
			const char *type = (const char*)param->typeExtra;
			delete [] (char*)type;
		    }
		}
		delete [] (QUParameter*)signal->parameters;
		delete signal;
	    }
	    delete [] (char*)signal_data->name;
	}
	if ( metaobj->numSignals() )
	    delete [] (QMetaData*)metaobj->signal( 0 );

	for ( i = 1; i < metaobj->numProperties(); ++i ) {
	    const QMetaProperty *property = metaobj->property( i );
	    delete [] (char*)property->n;
	    delete [] (char*)property->t;
	}
	if ( metaobj->numProperties() )
	    delete [] (QMetaProperty*)metaobj->property( 0 );
    }
    delete metaobj;
    metaobj = 0;
}

/*!
    \fn bool QComBase::initialize( IUnknown **ptr )

    This virtual function is called by setControl. Reimplement this function to return
    the IUnknown pointer of the COM object in \a ptr, and return TRUE if the object
    initialization succeeded. Otherwise, return FALSE.

    The interface returned in \a ptr should be referenced exactly once when this function
    returns. The interface provided by e.g. CoCreateInstance is already referenced, and
    there is no need to reference it again.
*/


/*!
    Requests the interface \a uuid from the COM object and sets the value of \a iface to the
    provided interface, or to null if the requested interface could not be provided.
    
    Returns the result of the QueryInterface implementation of the COM object.

    \sa control
*/
long QComBase::queryInterface( const QUuid &uuid, void **iface ) const
{
    *iface = 0;
    if ( ptr && !uuid.isNull() )
	return ptr->QueryInterface( uuid, iface );
    
    return E_NOTIMPL;
}

/*!
    \reimp
*/
QMetaObject *QComBase::metaObject() const
{
    if ( metaobj )
	return metaobj;
    QMetaObject* parentObject = parentMetaObject();

    // one signal and one property are always there
    static const QUParameter param_signal_0[] = {
	{ "name", &static_QUType_QString, 0, QUParameter::In },
	{ "argc", &static_QUType_int, 0, QUParameter::In },
	{ "argv", &static_QUType_ptr, "void", QUParameter::In }
    };
    static const QUMethod signal_0 = {"signal", 3, param_signal_0 };

    static const QUParameter param_signal_1[] = {
	{ "name", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"propertyChanged", 1, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "signal(const QString&,int,void*)", &signal_0, QMetaData::Public },
	{ "propertyChanged(const QString&)", &signal_1, QMetaData::Public }
    };
    static const QMetaProperty props_tbl[1] = {
 	{ "QString","control", 259, (QMetaObject**)&tempMetaObj, 0, -1 }
    };

    if ( !ptr ) {
	if ( tempMetaObj )
	    return tempMetaObj;

	tempMetaObj = QMetaObject::new_metaobject(
	    "QComBase", parentObject,
	    0, 0,
	    signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	    props_tbl, 1,
	    0, 0,
#endif // QT_NO_PROPERTIES
	    0, 0 );

	return tempMetaObj;
    }

    // the rest is generated from the IDispatch implementation
    QComBase* that = (QComBase*)this; // mutable
    QSettings iidnames;
    iidnames.insertSearchPath( QSettings::Windows, "/Classes" );

    QDict<QUMethod> slotlist; // QUMethods deleted in ~QActiveX
    QDict<QUMethod> signallist; // QUMethods deleted in ~QActiveX
    QDict<QMetaProperty> proplist;
    proplist.setAutoDelete( TRUE ); // deep copied when creating metaobject
    QDict<QString> infolist; 
    infolist.setAutoDelete( TRUE ); // deep copied when creating metaobject
    QDict<QString> enumlist; 
    enumlist.setAutoDelete( TRUE ); // deep copied when creating metaobject

    // create default signal and slots
/*
    CComPtr<IProvideClassInfo> pci;
    ptr->QueryInterface( IID_IProvideClassInfo, (void**)&pci );
    if ( pci ) {
	CComPtr<ITypeInfo> classinfo;
	pci->GetClassInfo( &classinfo );
	if ( classinfo ) {
	    TYPEATTR *classTypeattr = 0;
	    classinfo->GetTypeAttr( &classTypeattr );
	    if ( classTypeattr ) {
		QUuid classID( classTypeattr->guid );
		QString classIDstr = classID.toString().upper();
		classIDstr = iidnames.readEntry( "/CLSID/" + classIDstr + "/Default", classIDstr );
		infolist.insert( "CoClass", new QString( classIDstr ) );
		QString version( "%1.%1" );
		version = version.arg( classTypeattr->wMajorVerNum ).arg( classTypeattr->wMinorVerNum );
		infolist.insert( "Version", new QString( version ) );

		for ( int iType = 0; iType < classTypeattr->cImplTypes; ++iType ) {
		    int typeflags = 0;
		    classinfo->GetImplTypeFlags( iType, &typeflags );
		    if ( ( typeflags & IMPLTYPEFLAG_FRESTRICTED ) || ( typeflags & IMPLTYPEFLAG_FDEFAULTVTABLE ) )
			continue;
		    bool eventtype = typeflags & IMPLTYPEFLAG_FSOURCE;
		    HREFTYPE reftype;
		    if ( classinfo->GetRefTypeOfImplType( iType, &reftype ) == S_OK ) {
			CComPtr<ITypeInfo> typeinfo;
			classinfo->GetRefTypeInfo( reftype, &typeinfo );
			while ( typeinfo ) {
			    TYPEATTR *typeattr = 0;
			    typeinfo->GetTypeAttr( &typeattr );
			    if ( typeattr ) {
				QUuid uuid( typeattr->guid );
				QString uuidstr = uuid.toString().upper();
				uuidstr = iidnames.readEntry( "/Interface/" + uuidstr + "/Default", uuidstr );
				if ( eventtype ) {
				    static eventcount = 0;
				    infolist.insert( QString("Event Interface %1").arg(++eventcount), new QString( uuidstr ) );
				} else {
				    static interfacecount = 0;
				    infolist.insert( QString("Interface %1").arg(++interfacecount), new QString( uuidstr ) );
				}
				typeinfo->ReleaseTypeAttr( typeattr );
			    }
			    // go up one class
			    HREFTYPE pRefType;
			    typeinfo->GetRefTypeOfImplType( 0, &pRefType );
			    CComPtr<ITypeInfo> baseInfo;
			    typeinfo->GetRefTypeInfo( pRefType, &baseInfo );
			    if ( typeinfo == baseInfo ) // IUnknown inherits IUnknown ???
				break;
			    typeinfo = baseInfo;
			}
		    }
		}
		classinfo->ReleaseTypeAttr( classTypeattr );
	    }
	}
    }
*/
    CComPtr<IDispatch> disp;
    ptr->QueryInterface( IID_IDispatch, (void**)&disp );
    if ( disp ) {
	UINT count;
	disp->GetTypeInfoCount( &count );
	// this is either 0 or 1, but anyway...
	if ( count ) {
	    CComPtr<ITypeInfo> info;
	    disp->GetTypeInfo( 0, LOCALE_USER_DEFAULT, &info );
	    // read type information
	    while ( info ) {
		ushort nFuncs = 0;
		ushort nVars = 0;
		ushort nImpl = 0;
		// get information about type
		TYPEATTR *typeattr;
		info->GetTypeAttr( &typeattr );
		if ( typeattr ) {
		    if ( ( typeattr->typekind != TKIND_DISPATCH && typeattr->typekind != TKIND_INTERFACE ) ||
			 ( typeattr->guid == IID_IDispatch || typeattr->guid == IID_IUnknown ) ) {
			info->ReleaseTypeAttr( typeattr );
			break;
		    }
		    
		    // UUID
		    QUuid uuid( typeattr->guid );
		    QString uuidstr = uuid.toString().upper();
		    uuidstr = iidnames.readEntry( "/Interface/" + uuidstr + "/Default", uuidstr );
		    static interfacecount = 0;
		    infolist.insert( QString("Interface %1").arg(++interfacecount), new QString( uuidstr ) );

		    // get number of functions, variables, and implemented interfaces
		    nFuncs = typeattr->cFuncs;
		    nVars = typeattr->cVars;
		    nImpl = typeattr->cImplTypes;

		    info->ReleaseTypeAttr( typeattr );
		}

		// get information about all functions
		for ( ushort fd = 0; fd < nFuncs ; ++fd ) {
		    FUNCDESC *funcdesc;
		    info->GetFuncDesc( fd, &funcdesc );
		    if ( !funcdesc )
			break;

		    // get function prototype
		    QString function;
		    QString prototype;
		    QStringList parameters;
		    QStringList paramTypes;

		    // get return value
		    TYPEDESC typedesc = funcdesc->elemdescFunc.tdesc;
		    QString returnType = typedescToQString( typedesc );
		    if ( funcdesc->invkind == INVOKE_FUNC && returnType != "void" ) {
			parameters << "return";
			paramTypes << returnType;
		    }

		    BSTR bstrNames[256];
		    UINT maxNames = 255;
		    UINT maxNamesOut;
		    info->GetNames( funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut );
		    for ( int p = 0; p < (int)maxNamesOut; ++ p ) {
			QString paramName = BSTRToQString( bstrNames[p] );
			SysFreeString( bstrNames[p] );

			// function name
			if ( !p ) {
			    function = paramName;
			    prototype = function + "(";
			    continue;
			}

			// parameter
			bool optional = p > funcdesc->cParams - funcdesc->cParamsOpt;
			QString ptype;
			TYPEDESC tdesc = funcdesc->lprgelemdescParam[p - ( (funcdesc->invkind == INVOKE_FUNC) ? 1 : 0 ) ].tdesc;
			ptype = typedescToQString( tdesc );
			if ( ptype == "UNSUPPORTED" ) {
			    tdesc = funcdesc->lprgelemdescParam[p - 1].tdesc;
			    ptype = typedescToQString( tdesc );
			}
			if ( funcdesc->invkind == INVOKE_FUNC )
			    ptype = constRefify( ptype );

			prototype += ptype;
			if ( optional )
			    ptype += "=0";
			paramTypes << ptype;
			parameters << paramName;
			if ( p < funcdesc->cParams )
			    prototype += ",";
		    }
		    if ( !!prototype )
			prototype += ")";

		    QMetaProperty *prop = 0;

		    // get type of function
		    if ( !(funcdesc->wFuncFlags & FUNCFLAG_FHIDDEN) ) switch( funcdesc->invkind ) {
		    case INVOKE_PROPERTYGET: // property
		    case INVOKE_PROPERTYPUT:
			{
			    if ( funcdesc->cParams > 1 ) {
				qWarning( "%s: Too many parameters in property", function.latin1() );
				break;
			    }
			    prop = proplist[function];
			    if ( !prop ) {
				prop = new QMetaProperty;
				proplist.insert( function, prop );
				prop->meta = (QMetaObject**)&metaobj;
				prop->_id = -1;
				prop->enumData = 0;
				prop->flags = PropStored;
				if ( !(funcdesc->wFuncFlags & FUNCFLAG_FNONBROWSABLE) )
				    prop->flags |= PropDesignable;
				if ( !(funcdesc->wFuncFlags & FUNCFLAG_FRESTRICTED) )
				    prop->flags |= PropScriptable;
				if ( funcdesc->wFuncFlags & FUNCFLAG_FREQUESTEDIT )
				    prop->flags |= PropRequesting;

				QString ptype = paramTypes[0];
				if ( ptype.isEmpty() )
				    ptype = returnType;
				if ( ptype != "void" ) {
				    prop->t = new char[ptype.length()+1];
				    prop->t = qstrcpy( (char*)prop->t, ptype );
				} else {
				    prop->t = 0;
				}
				prop->n = new char[function.length()+1];
				prop->n = qstrcpy( (char*)prop->n, function );

				if ( funcdesc->wFuncFlags & FUNCFLAG_FBINDABLE ) {
				    prop->flags |= PropBindable;
				    if ( !eventSink )
					that->eventSink = new QAxEventSink( that );
				    // generate changed signal
				    QString signalName = function + "Changed";
				    QString signalParam = constRefify( ptype );
				    QString signalProto = signalName + "(" + signalParam + ")";
				    QString paramName = "value";
				    if ( !signallist.find( signalProto ) ) {
					QUMethod *signal = new QUMethod;
					signal->name = new char[signalName.length()+1];
					signal->name = qstrcpy( (char*)signal->name, signalName );
					signal->count = 1;
					QUParameter *param = new QUParameter;
					param->name = new char[paramName.length()+1];
					param->name = qstrcpy( (char*)param->name, paramName );
					param->inOut = QUParameter::In;
					QStringToQUType( signalParam, param );
					signal->parameters = param;

					signallist.insert( signalProto, signal );
				    }
				    eventSink->addProperty( funcdesc->memid, function, signalProto );
				}
			    } else if ( !prop->t ) {
				QString ptype = paramTypes[0];
				if ( ptype.isEmpty() )
				    ptype = returnType;
				if ( paramTypes.isEmpty() )
				    paramTypes.append( ptype );
				else
				    paramTypes[0] = ptype;
				prop->t = new char[ptype.length()+1];
				prop->t = qstrcpy( (char*)prop->t, ptype );
			    }
			    if ( funcdesc->invkind == INVOKE_PROPERTYGET ) {
				prop->flags |= QMetaProperty::Readable;
			    } else {
				prop->flags |= QMetaProperty::Writable;
			    }
			    if ( !prop->t )
				break;
			    // fall through to generate put function as slot
			}
			if ( funcdesc->invkind != INVOKE_PROPERTYPUT )
			    break;

		    case INVOKE_FUNC: // method
			{
			    if ( funcdesc->invkind == INVOKE_PROPERTYPUT && prop ) {
				QString set;
				QString pname = function;
				QString firstletter = function.left(1);
				if ( firstletter == firstletter.upper() ) {
				    set = "Set";
				} else {
				    set = "set";
				    function = firstletter.upper() + function.mid(1);
				}
				function = set + function;
				QString ptype;
				if ( prototype.right( 2 ) == "()" ) {
				    ptype = prop->type();
				    if ( ptype.isEmpty() )
					ptype = returnType;
				    prototype = function + "(" + constRefify(ptype) + ")";
				} else {
				    prototype = set + prototype;
				}
				parameters.append( pname );
				paramTypes.append( constRefify(ptype) );
				if ( slotlist.find( prototype ) )
				    break;
			    }
			    bool defargs;
			    QString defprototype = prototype;
			    do {
				defargs = FALSE;
				QUMethod *slot = new QUMethod;
				slot->name = new char[function.length()+1];
				slot->name = qstrcpy( (char*)slot->name, function );
				slot->count = parameters.count();
				QUParameter *params = slot->count ? new QUParameter[slot->count] : 0;
				int offset = parameters[0] == "return" ? 1 : 0;
				for ( int p = 0; p< slot->count; ++p ) {
				    QString paramName = parameters[p];
				    QString paramType = paramTypes[p];
				    if ( paramType.right( 2 ) == "=0" ) {
					paramType.truncate( paramType.length()-2 );
					paramTypes[p] = paramType;
					defargs = TRUE;
					slot->count = p;
					prototype = function + "(";
					for ( int pp = offset; pp < p; ++pp ) {
					    prototype += paramTypes[pp];
					    if ( pp < p-1 )
						prototype += ",";
					}
					prototype += ")";
					break;
				    }
				    params[p].name = new char[paramName.length()+1];
				    params[p].name = qstrcpy( (char*)params[p].name, paramName );
				    params[p].inOut = 0;
				    if ( !p && paramName == "return" ) {
					params[p].inOut = QUParameter::Out;
				    } else if ( funcdesc->lprgelemdescParam + p - offset ) {
					ushort inout = funcdesc->lprgelemdescParam[p-offset].paramdesc.wParamFlags;
					if ( inout & PARAMFLAG_FIN )
					    params[p].inOut |= QUParameter::In;
					if ( inout & PARAMFLAG_FOUT )
					    params[p].inOut |= QUParameter::Out;
				    }

				    QStringToQUType( paramType, params + p );
				}

				slot->parameters = params;
				slotlist.insert( prototype, slot );
				prototype = defprototype;
			    } while ( defargs );
			}
			break;

		    default:
			break;
		    }

#if 0 // documentation in metaobject would be cool?
		    // get function documentation
		    BSTR bstrDocu;
		    info->GetDocumentation( funcdesc->memid, 0, &bstrDocu, 0, 0 );
		    QString strDocu = BSTRToQString( bstrDocu );
		    if ( !!strDocu )
			desc += "[" + strDocu + "]";
		    desc += "\n";
		    SysFreeString( bstrDocu );
#endif

		    info->ReleaseFuncDesc( funcdesc );
		}
		
		// get information about all variables
		for ( ushort vd = 0; vd < nVars; ++vd ) {
		    VARDESC *vardesc;
		    info->GetVarDesc( vd, &vardesc );
		    if ( !vardesc )
			break;

		    // no use if it's not a dispatched variable
		    if ( vardesc->varkind != VAR_DISPATCH ) {
			info->ReleaseVarDesc( vardesc );
			continue;
		    }

		    // get variable type
		    TYPEDESC typedesc = vardesc->elemdescVar.tdesc;
		    QString variableType = typedescToQString( typedesc );

		    // get variable name
		    QString variableName;

		    BSTR bstrNames[256];
		    UINT maxNames = 255;
		    UINT maxNamesOut;
		    info->GetNames( vardesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut );
		    for ( int v = 0; v < (int)maxNamesOut; ++v ) {
			QString varName = BSTRToQString( bstrNames[v] );
			SysFreeString( bstrNames[v] );

			if ( !v ) {
			    variableName = varName;
			    continue;
			}
		    }

		    if ( !(vardesc->wVarFlags & VARFLAG_FHIDDEN) ) {
			// generate meta property
			QMetaProperty *prop = proplist[variableName];
			if ( !prop ) {
			    prop = new QMetaProperty;
			    proplist.insert( variableName, prop );
			    prop->meta = (QMetaObject**)&metaobj;
			    prop->_id = -1;
			    prop->enumData = 0;
			    prop->flags = QMetaProperty::Readable | PropStored;
			    if ( !(vardesc->wVarFlags & VARFLAG_FREADONLY) )
				prop->flags |= QMetaProperty::Writable;;
			    if ( !(vardesc->wVarFlags & VARFLAG_FNONBROWSABLE) )
				prop->flags |= PropDesignable;
			    if ( !(vardesc->wVarFlags & VARFLAG_FRESTRICTED) )
				prop->flags |= PropScriptable;
			    if ( vardesc->wVarFlags & VARFLAG_FREQUESTEDIT )
				prop->flags |= PropRequesting;

			    prop->t = new char[variableType.length()+1];
			    prop->t = qstrcpy( (char*)prop->t, variableType );
			    prop->n = new char[variableName.length()+1];
			    prop->n = qstrcpy( (char*)prop->n, variableName );

			    if ( vardesc->wVarFlags & VARFLAG_FBINDABLE ) {
				prop->flags |= PropBindable;
				if ( !eventSink )
				    that->eventSink = new QAxEventSink( that );
				// generate changed signal
				QString signalName = variableName + "Changed";
				QString signalParam = constRefify( variableType );
				QString signalProto = signalName + "(" + signalParam + ")";
				QString paramName = "value";
				if ( !signallist.find( signalProto ) ) {
				    QUMethod *signal = new QUMethod;
				    signal->name = new char[signalName.length()+1];
				    signal->name = qstrcpy( (char*)signal->name, signalName );
				    signal->count = 1;
				    QUParameter *param = new QUParameter;
				    param->name = new char[paramName.length()+1];
				    param->name = qstrcpy( (char*)param->name, paramName );
				    param->inOut = QUParameter::In;
				    QStringToQUType( signalParam, param );
				    signal->parameters = param;

				    signallist.insert( signalProto, signal );
				}
				eventSink->addProperty( vardesc->memid, variableName, signalProto );
			    }
			}

			// generate a set slot
			if ( !(vardesc->wVarFlags & VARFLAG_FREADONLY) ) {
			    QString firstletter = variableName.left(1);
			    QString set;
			    if ( firstletter == firstletter.upper() ) {
				set = "Set";
			    } else {
				set = "set";
			    }

			    variableType = constRefify( variableType );
			    QString function = set + variableName;
			    QString prototype = function + "(" + variableType + ")";

			    if ( !slotlist.find( prototype ) ) {
				QUMethod *slot = new QUMethod;
				slot->name = new char[ function.length() + 1 ];
				slot->name = qstrcpy( (char*)slot->name, function );
				slot->count = 1;
				QUParameter *params = new QUParameter;
				params->inOut = QUParameter::In;
				params->name = new char[ variableName.length() + 1 ];
				params->name = qstrcpy( (char*)params->name, variableName );

				QStringToQUType( variableType, params );

				slot->parameters = params;
				slotlist.insert( prototype, slot );
			    }
			}

#if 0 // documentation in metaobject would be cool?
			// get function documentation
			BSTR bstrDocu;
			info->GetDocumentation( vardesc->memid, 0, &bstrDocu, 0, 0 );
			QString strDocu = BSTRToQString( bstrDocu );
			if ( !!strDocu )
			    desc += "[" + strDocu + "]";
			desc += "\n";
			SysFreeString( bstrDocu );
#endif
		    }
		    info->ReleaseVarDesc( vardesc );
		}

		if ( !nImpl )
		    break;

		// go up one base class
		HREFTYPE pRefType;
		info->GetRefTypeOfImplType( 0, &pRefType );
		CComPtr<ITypeInfo> baseInfo;
		info->GetRefTypeInfo( pRefType, &baseInfo );
		if ( info == baseInfo ) // IUnknown inherits IUnknown ???
		    break;
		info = baseInfo;
	    }
	}
    }

    CComPtr<IConnectionPointContainer> cpoints;
    ptr->QueryInterface( IID_IConnectionPointContainer, (void**)&cpoints );
    if ( cpoints ) {
	CComPtr<IProvideClassInfo> classinfo;
	cpoints->QueryInterface( IID_IProvideClassInfo, (void**)&classinfo );

	CComPtr<IEnumConnectionPoints> epoints;
	cpoints->EnumConnectionPoints( &epoints );
	if ( epoints ) {
	    ULONG c = 1;
	    epoints->Reset();
	    do {
		CComPtr<IConnectionPoint> cpoint;
		epoints->Next( c, &cpoint, &c );
		if ( !c )
		    break;

		IID iid;
		cpoint->GetConnectionInterface( &iid );
		if ( !eventSink )
		    that->eventSink = new QAxEventSink( that );
		eventSink->addConnection( cpoint, iid );

		if ( classinfo ) {
		    CComPtr<ITypeInfo> info;
		    CComPtr<ITypeInfo> eventinfo;
		    classinfo->GetClassInfo( &info );
		    if ( info ) { // this is the type info of the component, not the event interface
			// get information about type
			TYPEATTR *typeattr;
			info->GetTypeAttr( &typeattr );
			if ( typeattr ) {
			    // UUID
			    if ( !infolist.find( "CoClass" ) ) {
				QUuid uuid( typeattr->guid );
				QString uuidstr = uuid.toString().upper();
				uuidstr = iidnames.readEntry( "/CLSID/" + uuidstr + "/Default", uuidstr );
				infolist.insert( "CoClass", new QString( uuidstr ) );
				QString version( "%1.%1" );
				version = version.arg( typeattr->wMajorVerNum ).arg( typeattr->wMinorVerNum );
				infolist.insert( "Version", new QString( version ) );
			    }

			    // test if one of the interfaces implemented is the one we're looking for
			    for ( int impl = 0; impl < typeattr->cImplTypes; ++impl ) {
				// get the ITypeInfo for the interface
				HREFTYPE reftype;
				info->GetRefTypeOfImplType( impl, &reftype );
				CComPtr<ITypeInfo> eventtype;
				info->GetRefTypeInfo( reftype, &eventtype );
				if ( eventtype ) {
				    TYPEATTR *eventattr;
				    eventtype->GetTypeAttr( &eventattr );
				    // this is it
				    if ( eventattr && eventattr->guid == iid ) {
					QUuid uuid( iid );
					QString uuidstr = uuid.toString().upper();
					uuidstr = iidnames.readEntry( "/Interface/" + uuidstr + "/Default", uuidstr );
					static eventcount = 0;
					infolist.insert( QString("Event Interface %1").arg(++eventcount), new QString( uuidstr ) );
					eventinfo = eventtype;
					eventtype->ReleaseTypeAttr( eventattr );
					break;
				    }
				    eventtype->ReleaseTypeAttr( eventattr );
				}
			    }
			    info->ReleaseTypeAttr( typeattr );

			    // what about other event interfaces?
			    if ( eventinfo ) {
				TYPEATTR *eventattr;
				eventinfo->GetTypeAttr( &eventattr );
				// Number of functions
				ushort nEvents = eventattr->cFuncs;

				// get information about all event functions
				for ( UINT fd = 0; fd < (UINT)nEvents; ++fd ) {
				    FUNCDESC *funcdesc;
				    eventinfo->GetFuncDesc( fd, &funcdesc );
				    if ( !funcdesc )
					break;
				    if ( funcdesc->invkind != INVOKE_FUNC ||
					 funcdesc->funckind != FUNC_DISPATCH ) {
					info->ReleaseFuncDesc( funcdesc );
					continue;
				    }

				    // get return value
				    TYPEDESC typedesc = funcdesc->elemdescFunc.tdesc;
				    QString returnType = typedescToQString( typedesc );

				    // get event function prototype
				    QString function;
				    QString prototype;
				    QStringList parameters;
				    QStringList paramTypes;

				    BSTR bstrNames[256];
				    UINT maxNames = 255;
				    UINT maxNamesOut;
				    eventinfo->GetNames( funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut );
				    int p;
				    for ( p = 0; p < (int)maxNamesOut; ++ p ) {
					QString paramName = BSTRToQString( bstrNames[p] );
					SysFreeString( bstrNames[p] );

					// function name
					if ( !p ) {
					    function = paramName;
					    prototype = function + "(";
					    continue;
					}

					// parameter
					bool optional = p > funcdesc->cParams - funcdesc->cParamsOpt;
					
					TYPEDESC tdesc = funcdesc->lprgelemdescParam[p - ( (funcdesc->invkind == INVOKE_FUNC) ? 1 : 0 ) ].tdesc;
					QString ptype = typedescToQString( tdesc );
					if ( funcdesc->invkind == INVOKE_FUNC )
					    ptype = constRefify( ptype );

					paramTypes << ptype;
					parameters << paramName;
					prototype += ptype;
					if ( optional )
					    prototype += "=0";
					if ( p < funcdesc->cParams )
					    prototype += ",";
				    }
				    if ( !!prototype )
					prototype += ")";

				    if ( !signallist.find( prototype ) ) {
					QUMethod *signal = new QUMethod;
					signal->name = new char[function.length()+1];
					signal->name = qstrcpy( (char*)signal->name, function );
					signal->count = parameters.count();
					QUParameter *params = signal->count ? new QUParameter[signal->count] : 0;
					for ( p = 0; p< signal->count; ++p ) {
					    QString paramName = parameters[p];
					    QString paramType = paramTypes[p];
					    params[p].name = new char[paramName.length()+1];
					    params[p].name = qstrcpy( (char*)params[p].name, paramName );
					    params[p].inOut = 0;
					    if ( funcdesc->lprgelemdescParam + p ) {
						ushort inout = funcdesc->lprgelemdescParam[p].paramdesc.wParamFlags;
						if ( inout & PARAMFLAG_FIN )
						    params[p].inOut |= QUParameter::In;
						if ( inout & PARAMFLAG_FOUT )
						    params[p].inOut |= QUParameter::Out;
					    }

					    QStringToQUType( paramType, params + p );
					}
					signal->parameters = params;
					signallist.insert( prototype, signal );
				    }
				    eventSink->addSignal( funcdesc->memid, prototype );

#if 0 // documentation in metaobject would be cool?
				    // get function documentation
				    BSTR bstrDocu;
				    eventinfo->GetDocumentation( funcdesc->memid, 0, &bstrDocu, 0, 0 );
				    QString strDocu = BSTRToQString( bstrDocu );
				    if ( !!strDocu )
					desc += "[" + strDocu + "]";
				    desc += "\n";
				    SysFreeString( bstrDocu );
#endif
				    eventinfo->ReleaseFuncDesc( funcdesc );
				}
			    }
			}
		    }
		}
	    } while ( c );
	}
    }

    // setup slot data
    int index = 0;
    QMetaData *const slot_data = slotlist.count() ? new QMetaData[slotlist.count()] : 0;
    QDictIterator<QUMethod> slot_it( slotlist );
    while ( slot_it.current() ) {
	QUMethod *slot = slot_it.current();
	slot_data[index].name = new char[slot_it.currentKey().length()+1];
	slot_data[index].name = qstrcpy( (char*)slot_data[index].name, slot_it.currentKey() );
	slot_data[index].method = slot;
	slot_data[index].access = QMetaData::Public;

	++index;
	++slot_it;
    }

    // setup signal data
    index = 0;
    QMetaData *const signal_data = new QMetaData[signallist.count()+2];
    if ( signal_data ) {
	signal_data[index] = signal_tbl[index];
	++index;
	signal_data[index] = signal_tbl[index];
	++index;
    }

    QDictIterator<QUMethod> signal_it( signallist );
    while ( signal_it.current() ) {
	QUMethod *signal = signal_it.current();
	signal_data[index].name = new char[signal_it.currentKey().length()+1];
	signal_data[index].name = qstrcpy( (char*)signal_data[index].name, signal_it.currentKey() );
	signal_data[index].method = signal;
	signal_data[index].access = QMetaData::Public;

	++index;
	++signal_it;
    }

    // setup property data
    index = 0;
    QMetaProperty *const prop_data = new QMetaProperty[proplist.count()+1];
    if ( prop_data ) {
	static const QMetaProperty props_tbl[1] = {
 	    { "QString","control", 259, (QMetaObject**)&metaobj, 0, -1 }
	};

	prop_data[index] = props_tbl[index];
	++index;
    }
    QDictIterator<QMetaProperty> prop_it( proplist );
    while ( prop_it.current() ) {
	QMetaProperty *prop = prop_it.current();
	prop_data[index].t = prop->t;
	prop_data[index].n = prop->n;
	prop_data[index].flags = prop->flags;
	prop_data[index].meta = prop->meta;
	prop_data[index].enumData = prop->enumData;
	prop_data[index]._id = prop->_id;

	++index;
	++prop_it;
    }

    // setup enum data
    index = 0;
    QMetaEnum *const enum_data = enumlist.count() ? new QMetaEnum[enumlist.count()] : 0;
    QDictIterator<QString> enum_it( enumlist );
    while ( enum_it.current() ) {
	QString info = *enum_it.current();
	QString key = enum_it.currentKey();
	enum_data[index].name = new char[key.length()+1];
	enum_data[index].name = qstrcpy( (char*)enum_data[index].name, key );
	//###
	++index;
	++enum_it;
    }

    // setup class info data
    index = 0;
    QClassInfo *const class_info = new QClassInfo[infolist.count()];
    QDictIterator<QString> info_it( infolist );
    while ( info_it.current() ) {
	QString info = *info_it.current();
	QString key = info_it.currentKey();
	class_info[index].name = new char[key.length()+1];
	class_info[index].name = qstrcpy( (char*)class_info[index].name, key );
	class_info[index].value = new char[info.length()+1];
	class_info[index].value = qstrcpy( (char*)class_info[index].value, info );

	++index;
	++info_it;
    }

    // put the metaobject together
    that->metaobj = QMetaObject::new_metaobject( 
	className(), parentObject, 
	slot_data, slotlist.count(),
	signal_data, signallist.count()+2,
	prop_data, proplist.count()+1,
	enum_data, enumlist.count(),
	class_info, infolist.count() );

    return metaobj;
}

/*!
    \internal
*/
bool QComBase::qt_invoke( int _id, QUObject* _o )
{
    const int index = _id - metaObject()->slotOffset();
    if ( !ptr || index < 0 )
	return FALSE;

    // get the IDispatch
    CComPtr<IDispatch> disp;
    ptr->QueryInterface( IID_IDispatch, (void**)&disp );
    if ( !disp )
	return FALSE;

    // get the slot information
    const QMetaData *slot_data = metaObject()->slot( index );
    if ( !slot_data )
	return FALSE;
    const QUMethod *slot = slot_data->method;
    if ( !slot )
	return FALSE;

    // Get the Dispatch ID of the method to be called
    bool fakedslot = FALSE;
    DISPID dispid;
    OLECHAR *names = (TCHAR*)qt_winTchar(slot->name, TRUE );
    disp->GetIDsOfNames( IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid );
    if ( dispid == DISPID_UNKNOWN ) {
	// see if we are calling a property set function as a slot
	if ( QString( slot->name ).left( 3 ) != "set" &&
	     QString( slot->name ).left( 3 ) != "Set" )
	    return FALSE;
	QString realname = slot->name;
	realname = realname.right( realname.length() - 3 );
	OLECHAR *realnames = (TCHAR*)qt_winTchar(realname, TRUE );
	disp->GetIDsOfNames( IID_NULL, &realnames, 1, LOCALE_USER_DEFAULT, &dispid );
	if ( dispid == DISPID_UNKNOWN )
	    return FALSE;

	fakedslot = TRUE;
    }

    // setup the parameters
    VARIANT ret; // Invoke initializes it
    VARIANT *pret = 0;
    if ( slot->parameters && ( slot->parameters[0].inOut & QUParameter::Out ) ) // slot has return value
	pret = &ret;
    int slotcount = slot->count - ( pret ? 1 : 0 );

    VARIANT arg;
    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    params.cArgs = slotcount;
    params.cNamedArgs = fakedslot ? 1 : 0;
    params.rgdispidNamedArgs = fakedslot ? &dispidNamed : 0;
    params.rgvarg = slot->count ? new VARIANTARG[slotcount] : 0;
    int p;
    for ( p = 0; p < slotcount; ++p ) {
	QUObject *obj = _o + p + 1;
	// map the QUObject's type to the VARIANT. ### Maybe it would be better 
	// to convert the QUObject to what the VARIANT is supposed to be, since
	// QUType is rather powerful, and VARIANT is not...
	if ( QUType::isEqual( obj->type, &static_QUType_int ) ) {
	    arg.vt = VT_I4;
	    arg.lVal = static_QUType_int.get( obj );
	} else if ( QUType::isEqual( obj->type, &static_QUType_QString ) ) {
	    arg.vt = VT_BSTR;
	    arg.bstrVal = QStringToBSTR( static_QUType_QString.get( obj ) );
	} else if ( QUType::isEqual( obj->type, &static_QUType_charstar ) ) {
	    arg.vt = VT_BSTR;
	    arg.bstrVal = QStringToBSTR( static_QUType_charstar.get( obj ) );
	} else if ( QUType::isEqual( obj->type, &static_QUType_bool ) ) {
	    arg.vt = VT_BOOL;
	    arg.boolVal = static_QUType_bool.get( obj );
	} else if ( QUType::isEqual( obj->type, &static_QUType_double ) ) {
	    arg.vt = VT_R8;
	    arg.dblVal = static_QUType_double.get( obj );
	} else if ( QUType::isEqual( obj->type, &static_QUType_enum ) ) {
	    arg.vt = VT_I4;
	    arg.lVal = static_QUType_enum.get( obj );
	} else if ( QUType::isEqual( obj->type, &static_QUType_QVariant ) ) {
	    arg = QVariantToVARIANT( static_QUType_QVariant.get( obj ) );
	} else if ( QUType::isEqual( obj->type, &static_QUType_idisp ) ) {
	    arg.vt = VT_DISPATCH;
	    arg.pdispVal = (IDispatch*)static_QUType_ptr.get( obj );
	} else if ( QUType::isEqual( obj->type, &static_QUType_iface ) ) {
	    arg.vt = VT_UNKNOWN;
	    arg.punkVal = (IUnknown*)static_QUType_ptr.get( obj );
	} else if ( QUType::isEqual( obj->type, &static_QUType_ptr ) ) {
	    const QUParameter *param = slot->parameters + p + ( pret ? 1 : 0 );
	    const char *type = (const char*)param->typeExtra;
	    if ( !qstrcmp( type, "int" ) ) {
		arg.vt = VT_I4;
		arg.lVal = *(int*)static_QUType_ptr.get( obj );
	    } else if ( !qstrcmp( type, "QString" ) || !qstrcmp( type, "const QString&" ) ) {
		arg.vt = VT_BSTR;
		arg.bstrVal = QStringToBSTR( *(QString*)static_QUType_ptr.get( obj ) );
	    } else if ( !qstrcmp( type, "QDateTime" ) || !qstrcmp( type, "const QDateTime&" ) ) {
		arg.vt = VT_DATE;
		arg.date = QDateTimeToDATE( *(QDateTime*)static_QUType_ptr.get( obj ) );
	    } else {
		arg.vt = VT_UI4;
		arg.ulVal = (Q_ULONG)static_QUType_ptr.get( obj );
	    }
	    //###
	} else {
	    arg.vt = VT_EMPTY;
	}
	params.rgvarg[ slotcount - p - 1 ] = arg;
    }
    // call the method
    UINT argerr = 0;
    HRESULT hres;
    if ( fakedslot && slotcount == 1 ) {
	hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, pret, 0, &argerr );
    } else {
	hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, pret, 0, &argerr );	    
    }

    // get return value
    if ( pret )
	VARIANTToQUObject( ret, _o );
    // clean up
    for ( p = 0; p < slot->count; ++p ) {
	if ( params.rgvarg[p].vt == VT_BSTR )
	    SysFreeString( params.rgvarg[p].bstrVal );
    }
    delete [] params.rgvarg;

    return checkHRESULT( hres );
}

/*!
    \reimp
*/
bool QComBase::qt_property( int _id, int _f, QVariant* _v )
{
    const int index = _id - metaObject()->propertyOffset();
    if ( index == 0 ) { // control property
	switch( _f ) {
	case 0: setControl( _v->toString() ); break;
	case 1: *_v = control(); break;
	default: if ( _f > 5 ) return FALSE;
	}
	return TRUE;
    } else if ( ptr && index >= 0 ) {
	// get the IDispatch
	CComPtr<IDispatch> disp;
	ptr->QueryInterface( IID_IDispatch, (void**)&disp );
	if ( !disp )
	    return FALSE;

	// get the property information
	const QMetaProperty *prop = metaObject()->property( index );
	if ( !prop )
	    return FALSE;

	//  get the Dispatch ID of the function to be called
	DISPID dispid;
	OLECHAR *names = (TCHAR*)qt_winTchar(prop->n, TRUE );
	disp->GetIDsOfNames( IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid );
	if ( dispid == DISPID_UNKNOWN )
	    return FALSE;
	
	switch ( _f ) {
	case 0: // Set
	    {
		VARIANTARG arg;
		arg.vt = VT_ERROR;
		arg.scode = DISP_E_TYPEMISMATCH;

		// map QVariant to VARIANTARG. ### Maybe it would be better 
		// to convert the QVariant to what the VARIANT is supposed to be,
		// but we don't know that from the QMetaProperty of course.
		if ( qstrcmp( prop->type(), _v->typeName() ) ) {
		    QVariant::Type type = QVariant::nameToType( prop->type() );
		    if ( type != QVariant::Invalid )
			_v->cast( type );
		}
		arg = QVariantToVARIANT( *_v, prop->type() );

		if ( arg.vt == VT_EMPTY ) {
		    qDebug( "QActiveX::setProperty(): Unhandled property type" );
		    return FALSE;
		}
		DISPPARAMS params;
		DISPID dispidNamed = DISPID_PROPERTYPUT;
		params.rgvarg = &arg;
		params.cArgs = 1;
		params.rgdispidNamedArgs = &dispidNamed;
		params.cNamedArgs = 1;

		UINT argerr = 0;
		HRESULT hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, 0, 0, &argerr );
		if ( arg.vt == VT_BSTR )
		    SysFreeString( arg.bstrVal );
		return checkHRESULT( hres );
	    }
	case 1: // Get
	    {
		VARIANTARG arg;
		DISPPARAMS params;
		params.cArgs = 0;
		params.cNamedArgs = 0;
		params.rgdispidNamedArgs = 0;
		params.rgvarg = 0;

		HRESULT hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &arg, 0, 0 );
		if ( !checkHRESULT( hres ) )
		    return FALSE;

		// map result VARIANTARG to QVariant
		*_v = VARIANTToQVariant( arg );;
		return ( _v->isValid() );
	    }
	case 2: // Reset
	    return TRUE;
	case 3: // Designable
	    return prop->flags & PropDesignable;
	case 4: // Scriptable
	    return prop->flags & PropScriptable;
	case 5: // Stored
	    return prop->flags & PropStored;
	default:
	    break;
	}
    }
    return FALSE;
}

/*!
    Calls the function \a id of the COM object passing the parameters \a var1 ... \a var8, 
    and returns the value, or an invalid variant if the function does not return a value.

    To get the ID of a function, use QMetaObject::findSlot( function, TRUE ).
*/
QVariant QComBase::dynamicCall( int id, const QVariant &var1, 
							 const QVariant &var2, 
							 const QVariant &var3, 
							 const QVariant &var4, 
							 const QVariant &var5, 
							 const QVariant &var6, 
							 const QVariant &var7, 
							 const QVariant &var8 )
{
    const QMetaData *slot_data = 0;
    slot_data = metaObject()->slot( id, TRUE );
    QVariant result;

    if ( slot_data ) {
	const QUMethod *slot = 0;
	slot = slot_data->method;

	QUObject obj[9];
	// obj[0] is the result
	int o;
	int ret = slot->count && slot->parameters[0].inOut == QUParameter::Out;
	for ( o = 0; o < slot->count-ret; ++o ) {
	    obj[o+1].type = slot->parameters[o+ret].type;
	}
	QVariantToQUObject( var1, obj[1] );
	QVariantToQUObject( var2, obj[2] );
	QVariantToQUObject( var3, obj[3] );
	QVariantToQUObject( var4, obj[4] );
	QVariantToQUObject( var5, obj[5] );
	QVariantToQUObject( var6, obj[6] );
	QVariantToQUObject( var7, obj[7] );
	QVariantToQUObject( var8, obj[8] );

	qt_invoke( id, obj );

	if ( !QUType::isEqual( obj->type, &static_QUType_Null ) ) {
	    if ( QUType::isEqual( obj->type, &static_QUType_int ) ) {
		result = static_QUType_int.get( obj );
	    } else if ( QUType::isEqual( obj->type, &static_QUType_QString ) ) {
		result = static_QUType_QString.get( obj );
	    } else if ( QUType::isEqual( obj->type, &static_QUType_charstar ) ) {
		result = static_QUType_charstar.get( obj );
	    } else if ( QUType::isEqual( obj->type, &static_QUType_bool ) ) {
		result = QVariant( static_QUType_bool.get( obj ), 0 );
	    } else if ( QUType::isEqual( obj->type, &static_QUType_double ) ) {
		result = static_QUType_double.get( obj );
	    } else if ( QUType::isEqual( obj->type, &static_QUType_enum ) ) {
		result = static_QUType_enum.get( obj );
	    } else if ( QUType::isEqual( obj->type, &static_QUType_QVariant ) ) {
		result = static_QUType_QVariant.get( obj );
	    } else if ( QUType::isEqual( obj->type, &static_QUType_idisp ) ) {
		//###
	    } else if ( QUType::isEqual( obj->type, &static_QUType_iface ) ) {
		//###
	    } else if ( QUType::isEqual( obj->type, &static_QUType_ptr ) && slot->parameters ) {
		const QUParameter *param = slot->parameters;
		const char *type = (const char*)param->typeExtra;
		if ( !qstrcmp( type, "int" ) ) {
		    result = *(int*)static_QUType_ptr.get( obj );
		} else if ( !qstrcmp( type, "QString" ) || !qstrcmp( type, "const QString&" ) ) {
		    result = *(QString*)static_QUType_ptr.get( obj );
		} else if ( !qstrcmp( type, "QDateTime" ) || !qstrcmp( type, "const QDateTime&" ) ) {
		    result = *(QDateTime*)static_QUType_ptr.get( obj );
		}
		//###
	    }
	}

	for ( o = 0; o < 9; ++o )
	    obj[o].type->clear( obj+o );
    }
#if defined(QT_CHECK_RANGE)
    else {
	const char *coclass = metaObject()->classInfo( "CoClass" );
	qWarning( "QActiveX::dynamicCall: %d: No such method in %s %s", id, control().latin1(), 
	    coclass ? coclass: "(unknown)" );
    }
#endif
    return result;
}

/*!
    \overload

    Using this overload is more convenient, but slightly slower. For rapid calls to the
    same function, get the function's ID using QMetaObject::findSlot( function, TRUE ) and
    use the overload above.
    \a function has to be provided as the full prototype, like e.g. in QObject::connect().
*/
QVariant QComBase::dynamicCall( const QCString &function, const QVariant &var1, 
							 const QVariant &var2, 
							 const QVariant &var3, 
							 const QVariant &var4, 
							 const QVariant &var5, 
							 const QVariant &var6, 
							 const QVariant &var7, 
							 const QVariant &var8 )
{
    int index = metaObject()->findSlot( qt_rmWS(function), TRUE );
    if ( index > -1 )
	return dynamicCall( index, var1, var2, var3, var4, var5, var6, var7, var8 );
#if defined(QT_CHECK_RANGE)
    else {
	const char *coclass = metaObject()->classInfo( "CoClass" );
	qWarning( "QActiveX::dynamicCall: %s: No such method in %s %s", (const char*)function, control().latin1(), 
	    coclass ? coclass: "(unknown)" );
    }
#endif
    return QVariant();
}

class QtPropertyBag : public IPropertyBag
{
public:
    QtPropertyBag() :ref( 0 ) {}

    HRESULT __stdcall QueryInterface( REFIID iid, LPVOID *iface )
    {
	*iface = 0;
	if ( iid == IID_IUnknown )
	    *iface = this;
	else if ( iid == IID_IPropertyBag )
	    *iface = this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }
    unsigned long __stdcall AddRef() { return ++ref; }
    unsigned long __stdcall Release() 
    {
	if ( !--ref ) {
	    delete this;
	    return 0;
	}
	return ref;
    }

    HRESULT __stdcall Read( LPCOLESTR name, VARIANT *var, IErrorLog * )
    {
	if ( !var )
	    return E_POINTER;

	QCString property = BSTRToQString((TCHAR*)name).local8Bit();
	QVariant qvar = map[property];
	*var = QVariantToVARIANT( qvar );
	return S_OK;
    }
    HRESULT __stdcall Write( LPCOLESTR name, VARIANT *var )
    {
	if ( !var )
	    return E_POINTER;
	QCString property = BSTRToQString((TCHAR*)name).local8Bit();
	QVariant qvar = VARIANTToQVariant( *var );
	map[property] = qvar;

	return S_OK;
    }

    QComBase::PropertyBag map;

private:
    unsigned long ref;
};

/*!
    Returns the values of all properties exposed by the COM object.
    
    This is more efficient than getting multiple properties individually if the COM object
    supports property bags.
*/
QComBase::PropertyBag QComBase::propertyBag() const
{
    PropertyBag result;
    if ( !ptr )
	return result;
    CComPtr<IPersistPropertyBag> persist;
    ptr->QueryInterface( IID_IPersistPropertyBag, (void**)&persist );
    if ( persist ) {
	QtPropertyBag *pbag = new QtPropertyBag();
	pbag->AddRef();
	persist->Save( pbag, FALSE, TRUE );
	result = pbag->map;
	pbag->Release();
	return result;
    } else {
	QComBase *that = (QComBase*)this;
	for ( int p = 1; p < metaObject()->numProperties( FALSE ); ++p ) {
	    QVariant var;
	    that->qt_property( p+metaObject()->propertyOffset(), 1, &var );
	    result.insert( metaObject()->property( p, FALSE )->name(), var );
	}
    }
    return result;
}

/*!
    Sets the properties of the COM object to the matching values in \a bag.

    \sa propertyBag()
*/
void QComBase::setPropertyBag( const PropertyBag &bag )
{
    if ( !ptr )
	return;
    CComPtr<IPersistPropertyBag> persist;
    ptr->QueryInterface( IID_IPersistPropertyBag, (void**)&persist );
    if ( persist ) {
	QtPropertyBag *pbag = new QtPropertyBag();
	pbag->map = bag;
	pbag->AddRef();
	persist->Load( pbag, 0 );
	pbag->Release();
    } else {
	QComBase *that = (QComBase*)this;
	for ( int p = 1; p < metaObject()->numProperties( FALSE ); ++p ) {
	    QVariant var = bag[metaObject()->property( p, FALSE )->name()];
	    that->qt_property( p+metaObject()->propertyOffset(), 0, &var );
	}
    }
}

/*!
    Returns TRUE if the property \a prop is allowed to change,
    otherwise returns FALSE.
    By default, all properties are allowed to change. Depending on the control
    implementation this setting might be ignored.

    \sa setPropertyWritable(), propertyChanged()
*/
bool QComBase::propertyWritable( const char *prop ) const
{
    if ( !propWritable )
	return TRUE;

    if ( !propWritable->contains( prop ) )
	return TRUE;
    else
	return (*propWritable)[prop];
}

/*!
    Sets the property \a prop to writable if \a ok is TRUE,
    otherwise sets \a prop to be read-only.
    By default, all properties are allowed to change. Depending on the control
    implementation this setting might be ignored.

    \sa propertyWritable(), propertyChanged()
*/
void QComBase::setPropertyWritable( const char *prop, bool ok )
{
    if ( !propWritable )
	propWritable = new QMap<QCString, bool>;

    (*propWritable)[prop] = ok;
}

/*!
    \fn bool QComBase::qt_emit( int, QUObject* );
    \internal 
*/

/*!
    \fn const char *QComBase::className() const
    \internal
*/

/*!
    \fn QObject *QComBase::qObject()
    \internal
*/

/*!
    \fn bool QComBase::isNull() const

    Returns TRUE if there is no COM object loaded by this wrapper, otherwise return FALSE.

    \sa control
*/

/*!
    \fn void QComBase::signal( const QString &name, int argc, void *argv )

    This generic signal gets emitted when the COM object issues the event \a name. \a argc
    is the number of parameters provided by the event (DISPPARAMS.cArgs), and \a argv is 
    the pointer to the parameter values (DISPPARAMS.rgvarg).
    
    Use this signal if the event has parameters of unsupported data types. Otherwise, connect
    directly to the signal \a name.
*/

/*!
    \fn void QComBase::propertyChanged( const QString &name )

    If the COM object supports property notification, this signal gets emitted when the property
    \a name is changed.
*/

/*!
    \class QComObject qcomobject.h
    \brief The QComObject class provides a QObject that wraps a COM object.

    \extension QActiveX

    A QComObject can be instantiated as an empty object, with the name of the COM object
    it should wrap, or with a pointer to the IUnknown that represents an existing COM object.
    If the COM object implements the IDispatch interface, the properties, methods and events of
    that object become available as Qt properties, slots and signals. The baseclass QComBase provides 
    also an API to access the COM object directly through the IUnknown pointer.

    QComObject is as well a QObject and can be used as such, e.g. it can be organized in an object
    hierarchy and receive events.
*/

/*!
    Creates an empty COM object and propagates \a parent and \a name to the QObject constructor. 
    To initialize the object, call \link QComBase::setControl() setControl \endlink.
*/
QComObject::QComObject( QObject *parent, const char *name )
: QObject( parent, name )
{
}

/*! 
    Creates a QComObject that wraps the COM object \a c.
    \a parent and \a name are propagated to the QWidget contructor.
*/
QComObject::QComObject( const QString &c, QObject *parent, const char *name )
: QObject( parent, name )
{
    setControl( c );
}

/*!
    Creates a QComObject that wraps the COM object referenced by \a iface.
    \a parent and \a name are propagated to the QWidget contructor.
*/
QComObject::QComObject( IUnknown *iface, QObject *parent, const char *name )
: QObject( parent, name ), QComBase( iface )
{
}

/*!
    Releases the COM object and destroys the QComObject,
    cleaning all allocated resources.
*/
QComObject::~QComObject()
{
}

/*!
    \reimp
*/
const char *QComObject::className() const
{
    return "QComObject";
}

/*!
    \reimp
*/
bool QComObject::initialize( IUnknown **ptr )
{
    QUuid uuid( control() );
    if ( *ptr || uuid.isNull() )
	return FALSE;
    moduleLock();

    *ptr = 0;
    CoCreateInstance( uuid, 0, CLSCTX_ALL, IID_IUnknown, (void**)ptr );
    if ( !ptr ) {
	moduleUnlock();
	return FALSE;
    }
    metaObject();
    return TRUE;
}

/*!
    \reimp
*/
QMetaObject *QComObject::metaObject() const
{
    return QComBase::metaObject();
}

/*!
    \reimp
*/
QMetaObject *QComObject::parentMetaObject() const
{
    return QObject::staticMetaObject();
}

/*!
    \reimp
*/
void *QComObject::qt_cast( const char *cname )
{
    if ( !qstrcmp( cname, "QComObject" ) ) return this;
    if ( !qstrcmp( cname, "QComBase" ) ) return (QComBase*)this;
    return QObject::qt_cast( cname );
}


/*!
    \reimp
*/
bool QComObject::qt_invoke( int _id, QUObject *_o )
{
    if ( QComBase::qt_invoke( _id, _o ) )
	return TRUE;
    return QObject::qt_invoke( _id, _o );
}

/*!
    \reimp
*/
bool QComObject::qt_emit( int _id, QUObject* _o )
{
    const int index = _id - metaObject()->signalOffset();
    if ( !isNull() && index >= 0 ) {
	// get the list of connections
	QConnectionList *clist = receivers( _id );
	if ( clist ) // call the signal
	    activate_signal( clist, _o );

	return TRUE;
    }
    return QObject::qt_emit( _id, _o );
}

/*!
  \reimp
*/
bool QComObject::qt_property( int _id, int _f, QVariant *_v )
{
    if ( QComBase::qt_property( _id, _f, _v ) )
	return TRUE;
    return QObject::qt_property( _id, _f, _v );
}

/*!
    \fn QObject *QComObject::qObject()
    \reimp
*/

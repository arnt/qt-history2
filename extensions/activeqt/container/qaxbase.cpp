/****************************************************************************
**
** Implementation of the QAxBase class.
**
** Copyright (C) 2001-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//#define QAX_NO_CLASSINFO

#include "qaxobject.h"

#include <quuid.h>
#include <qhash.h>
#include <qsettings.h>
#include <qmetaobject.h>

#ifdef QT_THREAD_SUPPORT
#   include <qmutex.h>
#endif

#include <qt_windows.h>
#include <ocidl.h>
#include <ctype.h>

static QMetaObject *tempMetaObj = 0;

#define NotDesignable	0x00001000
#define NotScriptable	0x00004000
#define NotStored	0x00010000

#define PropBindable	0x00040000
#define PropRequesting	0x00080000
// don't use the upper byte!

#define PredefSignals	3
#define PredefProps	1

#include "../shared/types.h"

/*
    \internal
    \class QAxEventSink qaxbase.cpp

    \brief The QAxEventSink class implements the event sink for all
	   IConnectionPoints implemented in the COM object.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif
*/

class QAxEventSink : public IDispatch,
		     public IPropertyNotifySink
{
public:
    QAxEventSink( QAxBase *com )
	: cpoint( 0 ), ciid( IID_NULL ), combase( com ), ref( 1 )
    {}
    virtual ~QAxEventSink()
    {
	Q_ASSERT( !cpoint );
    }

    QUuid connectionInterface() const
    {
	return ciid;
    }
    QMap<DISPID, QString> signalMap() const
    {
	return sigs;
    }
    QMap<DISPID, QString> propertyMap() const
    {
	return props;
    }
    QMap<DISPID, QString> propSignalMap() const
    {
	return propsigs;
    }

    // add a connection
    void advise( IConnectionPoint *cp, IID iid )
    {
	cpoint = cp;
	cpoint->AddRef();
	ciid = iid;
	cpoint->Advise( (IUnknown*)(IDispatch*)this, &cookie );
    }

    // disconnect from all connection points
    void unadvise()
    {
	combase = 0;
	if ( cpoint ) {
	    cpoint->Unadvise( cookie );
	    cpoint->Release();
	    cpoint = 0;
	}
    }

    void addSignal( DISPID memid, const QString &name )
    {
	sigs.insert( memid, name );
	QMap<DISPID,QString>::Iterator it;
	DISPID id = -1;
	for ( it = propsigs.begin(); it!= propsigs.end(); ++it ) {
	    if ( it.value() == name ) {
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
	else if ( ciid == riid )
	    *ppvObject = (IDispatch*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    // IDispatch
    HRESULT __stdcall GetTypeInfoCount( unsigned int * ) { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo( UINT, LCID, ITypeInfo ** ) { return E_NOTIMPL; }
    HRESULT __stdcall GetIDsOfNames( const _GUID &, wchar_t **, unsigned int, unsigned long, long * ) { return E_NOTIMPL; }

    HRESULT __stdcall Invoke( DISPID dispIdMember,
			      REFIID riid,
			      LCID,
			      WORD wFlags,
			      DISPPARAMS *pDispParams,
			      VARIANT*,
			      EXCEPINFO*,
			      UINT* )
    {
	// verify input
	if ( riid != IID_NULL )
	    return DISP_E_UNKNOWNINTERFACE;
	if ( !(wFlags & DISPATCH_METHOD) )
	    return DISP_E_MEMBERNOTFOUND;
	if ( !combase )
	    return E_UNEXPECTED;

	const QMetaObject *meta = combase->metaObject();
	QString signame = sigs[dispIdMember];
	if ( !meta || signame.isEmpty() )
	    return DISP_E_MEMBERNOTFOUND;

	QObject *qobject = combase->qObject();
	if ( qobject->signalsBlocked() )
	    return S_OK;

	// emit the signal "as is"
/*XXX
	int index = meta->findSignal( "signal(const QString&,int,void*)" );
	if ( index != -1 && ((QAxObject*)qobject)->receivers( index ) ) {
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
	if ( index != -1 && ((QAxObject*)qobject)->receivers( index ) ) {
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
	    bool ok = TRUE;
	    for ( p = 0; p < pcount && ok; ++p ) {// map the VARIANT to the QUObject
		objects[p+1].payload.ptr = 0;
		ok = VARIANTToQUObject( pDispParams->rgvarg[ pcount-p-1 ], objects + p + 1, params + p );
	    }

	    // emit the generated signal if everything went well
	    bool ret = ok && combase->qt_emit( index, objects );

	    // update the VARIANT for references and free memory
	    for ( p = 0; p < pcount; ++p ) {
		const QUParameter *param = params+p;
		if ( param->inOut & QUParameter::Out ) {
		    VARIANT *arg = &(pDispParams->rgvarg[ pcount-p-1 ]);
		    QUObject *obj = objects + p + 1;
		    QUObjectToVARIANT( obj, *arg, param );
		}
		clearQUObject( objects+p+1, param );
	    }
	    delete [] objects;

	    return ret ? S_OK : ( ok ? DISP_E_MEMBERNOTFOUND : DISP_E_TYPEMISMATCH );
	}
*/
	return S_OK;
    }

    // IPropertyNotifySink
    HRESULT __stdcall OnChanged( DISPID dispID )
    {
	// verify input
	if ( dispID == DISPID_UNKNOWN || !combase )
	    return S_OK;

	const QMetaObject *meta = combase->metaObject();
	QString propname = props[dispID];
	if ( !meta || propname.isEmpty() )
	    return S_OK;

	QObject *qobject = combase->qObject();
	if ( qobject->signalsBlocked() )
	    return S_OK;

	// emit the generic signal
/* XXX
	int index = meta->findSignal( "propertyChanged(const QString&)" );
	if ( index != -1 && ((QAxObject*)qobject)->receivers( index ) ) {
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
	if ( index != -1 && ((QAxObject*)qobject)->receivers( index ) ) {
	    const QMetaData *signal = meta->signal( index - meta->signalOffset() );
	    if ( !signal || signal->method->count != 1 )
		return S_OK;

	    // setup parameters
	    int pindex = meta->findProperty( propname );
	    QVariant var;
	    combase->qt_property( pindex + meta->propertyOffset(), 1, &var );
	    if ( !var.isValid() )
		return S_OK;

	    QUObject o[2];
	    o[1].payload.ptr = 0;
	    QVariantToQUObject( var, o[1], signal->method->parameters );

	    // emit the "changed" signal
	    combase->qt_emit( index, o );
	    clearQUObject( o+1, signal->method->parameters );
	}
*/
	return S_OK;
    }
    HRESULT __stdcall OnRequestEdit( DISPID dispID )
    {
	if ( dispID == DISPID_UNKNOWN || !combase )
	    return S_OK;

	QString propname = props[dispID];
	return combase->propertyWritable( propname ) ? S_OK : S_FALSE;
    }

    IConnectionPoint *cpoint;
    IID ciid;
    ULONG cookie;

    QMap<DISPID, QString> sigs;
    QMap<DISPID, QString> propsigs;
    QMap<DISPID, QString> props;

    QAxBase *combase;
    long ref;
};

struct QAxMetaObject : public QMetaObject
{
    // save information about QAxEventSink connections, and connect when found in cache
    QList<QUuid> connectionInterfaces;
    // DISPID -> signal name
    QMap< QUuid, QMap<DISPID, QString> > sigs;
    // DISPID -> property changed signal name
    QMap< QUuid, QMap<DISPID, QString> > propsigs;
    // DISPID -> property name
    QMap< QUuid, QMap<DISPID, QString> > props;
};

static QHash<QString, QAxMetaObject*> *mo_cache = 0;
static int mo_cache_ref = 0;
#ifdef QT_THREAD_SUPPORT
static QMutex cache_mutex;
#endif

/*
    \internal
    \class QAxBasePrivate qaxbase.cpp
*/

class QAxBasePrivate
{
public:
    QAxBasePrivate()
	: useEventSink( TRUE ), useMetaObject( TRUE ), useClassInfo( TRUE ),
	  cachedMetaObject( FALSE ), initialized( FALSE ), tryCache( FALSE ),
	  ptr( 0 ), disp( 0 ), propWritable( 0 ), metaobj( 0 )
    {
#ifdef QT_THREAD_SUPPORT
	// protect initialization
	QMutexLocker locker( &cache_mutex );
#endif
	if ( !mo_cache )
	    mo_cache = new QHash<QString, QAxMetaObject*>;
	mo_cache_ref++;
    }

    ~QAxBasePrivate()
    {
	Q_ASSERT( !ptr );
	Q_ASSERT( !disp );

	delete propWritable;
	propWritable = 0;

#ifdef QT_THREAD_SUPPORT
	// protect cleanup
	QMutexLocker locker( &cache_mutex );
#endif
	if ( !--mo_cache_ref && mo_cache ) {
	    // XXX mo_cache->setAutoDelete( TRUE );
	    delete mo_cache;
	    QHash<QString, QMetaObject*>::ConstIterator it(mo_cache);
	    
	    mo_cache = 0;
	}
    }

    IDispatch *dispatch()
    {
	if ( disp )
	    return disp;

	if ( ptr )
	    ptr->QueryInterface( IID_IDispatch, (void**)&disp );
	return disp;
    }

    QHash<QString, QAxEventSink*> eventSink;
    bool useEventSink	    :1;
    bool useMetaObject	    :1;
    bool useClassInfo	    :1;
    bool cachedMetaObject   :1;
    bool initialized	    :1;
    bool tryCache	    :1;

    IUnknown *ptr;
    IDispatch *disp;

    QMap<QString, bool> *propWritable;
    QAxMetaObject *metaobj;
};

#ifndef QAXPRIVATE_DECL

/*!
    \class QAxBase qaxbase.h

    \brief The QAxBase class is an abstract class that provides an API
    to initalize and access a COM object.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module QAxContainer
    \extension ActiveQt

    QAxBase is an abstract class that cannot be used directly, and is
    instantiated through the subclasses QAxObject and QAxWidget. This
    class provides the API to access the COM object directly
    through its IUnknown implementation. If the COM object implements
    the IDispatch interface, the properties and methods of that object
    become available as Qt properties and slots.

    \code
    connect( buttonBack, SIGNAL(clicked()), webBrowser, SLOT(GoBack()) );
    \endcode

    Properties exposed by the object's IDispatch implementation can be
    read and written through the property system provided by the Qt
    Object Model (both subclasses are QObjects, so you can use \link
    QObject::setProperty() setProperty() \endlink and \link
    QObject::property() property() \endlink as with QObject). Properties
    with multiple parameters are not supported.

    \code
    activeX->setProperty( "text", "some text" );
    int value = activeX->property( "value" );
    \endcode

    Write-functions for properties and other methods exposed by the
    object's IDispatch implementation can be called directly using
    dynamicCall(), or indirectly as slots connected to a signal.

    \code
    webBrowser->dynamicCall( "GoHome()" );
    \endcode

    Outgoing events supported by the COM object are emitted as
    standard Qt signals.

    \code
    connect( webBrowser, SIGNAL(TitleChanged(const QString&)),
	     this, SLOT(setCaption(const QString&)) );
    \endcode

    QAxBase transparently converts between COM data types and the
    equivalent Qt data types. Some COM types have no equivalent Qt data structure.

    Supported COM datatypes are listed in the first column of following table.
    The second column is the Qt type that can be used with the QObject property
    functions. The third column is the Qt type that is used in the prototype of
    generated signals and slots for in-parameters, and the last column is the Qt
    type that is used in the prototype of signals and slots for out-parameters.
    \table
    \header
    \i COM type
    \i Qt property
    \i in-parameter
    \i out-parameter
    \row
    \i VARIANT_BOOL
    \i bool
    \i bool
    \i bool&
    \row
    \i BSTR
    \i QString
    \i const QString&
    \i QString&
    \row
    \i char, short, int, long
    \i int
    \i int
    \i int&
    \row
    \i uchar, ushort, uint, ulong
    \i uint
    \i uint
    \i uint&
    \row
    \i float, double
    \i double
    \i double
    \i double&
    \row
    \i DATE
    \i QDateTime
    \i const QDateTime&
    \i QDateTime&
    \row
    \i CY
    \i Q_LLONG
    \i Q_LLONG
    \i Q_LLONG&
    \row
    \i OLE_COLOR
    \i QColor
    \i const QColor&
    \i QColor&
    \row
    \i SAFEARRAY(VARIANT)
    \i QList\<QVariant\>
    \i const QList\<QVariant\>&
    \i QList\<QVariant\>&
    \row
    \i SAFEARRAY(BYTE)
    \i QByteArray
    \i const QByteArray&
    \i QByteArray&
    \row
    \i SAFEARRAY(BSTR)
    \i QStringList
    \i const QStringList&
    \i QStringList&
    \row
    \i VARIANT
    \i type-dependent
    \i const QVariant&
    \i QVariant&
    \row
    \i IFontDisp*
    \i QFont
    \i const QFont&
    \i QFont&
    \row
    \i IPictureDisp*
    \i QPixmap
    \i const QPixmap&
    \i QPixmap&
    \row
    \i IDispatch*
    \i QAxObject* (read-only)
    \i \c QAxBase::asVariant()
    \i QAxObject* (return value)
    \row
    \i IUnknown*
    \i QAxObject* (read-only)
    \i \c QAxBase::asVariant()
    \i QAxObject* (return value)
    \row
    \i SCODE, DECIMAL
    \i \e unsupported
    \i \e unsupported
    \i \e unsupported
    \endtable

    Supported are also enumerations, and typedefs to supported types.

    To call the methods of a COM interface described by the following IDL
    \code
    dispinterface IControl
    {
    properties:
        [id(1)] BSTR text;
	[id(2)] IFontDisp *font;

    methods:
	[id(6)] void showColumn( [in] int i );
        [id(3)] bool addColumn( [in] BSTR t );
	[id(4)] int fillList( [in, out] SAFEARRAY(VARIANT) *list );
	[id(5)] IDispatch *item( [in] int i );
    };
    \endcode
    use the QAxBase API like this:
    \code
    QAxObject object( "<CLSID>" );

    QString text = object.property( "text" ).toString();
    object.setProperty( "font", QFont( "Times New Roman", 12 ) );

    connect( this, SIGNAL(clicked(int)), &object, SLOT(showColumn(int)) );
    bool ok = object.dynamicCall( "addColumn(const QString&)", "Column 1" ).toBool();

    QList<QVariant> varlist;
    QList<QVariant> parameters;
    parameters << QVariant( varlist );
    int n = object.dynamicCall( "fillList(QList<QVariant>&)", parameters ).toInt();

    QAxObject *item = object.querySubItem( "item(int)", 5 );
    \endcode

    Note that the QList the object should fill has to be provided as an
    element in the parameter list of QVariants.

    If you need to access properties or pass parameters of unsupported
    datatypes you must access the COM object directly through its
    IDispatch implementation or other interfaces. Those interfaces can be 
    retrieved through queryInterface().

    \code
    IUnknown *iface = 0;
    activeX->queryInterface( IID_IUnknown, (void**)&iface );
    if ( iface ) {
        // use the interface
	iface->Release();
    }
    \endcode

    To get the definition of the COM interfaces you will have to use the header
    files provided with the component you want to use. Some compilers can also
    import type libraries using the #import compiler directive. See the component
    documentation to find out which type libraries you have to import, and how to use
    them.

    If you need to react to events that pass parameters of unsupported
    datatypes you can use the generic signal that delivers the event
    data as provided by the COM event.
*/

/*!
    \enum QAxBase::PropertyBag

    A QMap<QString,QVariant> that can store properties as name:value pairs.
*/

/*!
    Creates a QAxBase object that wraps the COM object \a iface. If \a
    iface is 0 (the default), use setControl() to instantiate a COM
    object.
*/
QAxBase::QAxBase( IUnknown *iface )
{
    d = new QAxBasePrivate();
    d->ptr = iface;
    if ( d->ptr ) {
	d->ptr->AddRef();
	d->initialized = TRUE;
    }
}

/*!
    Shuts down the COM object and destroys the QAxBase object.

    \sa clear()
*/
QAxBase::~QAxBase()
{
    clear();

    delete d;
    d = 0;
}

/*!
    \property QAxBase::control
    \brief the name of the COM object wrapped by this QAxBase object.

    Setting this property initilializes the COM object. Any COM object
    previously set is shut down.

    The most efficient way to set this property is by using the
    registered component's UUID, e.g.
    \code
    ctrl->setControl( "{8E27C92B-1264-101C-8A2F-040224009C02}" );
    \endcode
    The second fastest way is to use the registered control's class
    name (with or without version number), e.g.
    \code
    ctrl->setControl( "MSCal.Calendar" );
    \endcode
    The slowest, but easiest way to use is to use the control's full
    name, e.g.
    \code
    ctrl->setControl( "Calendar Control 9.0" );
    \endcode

    If the component's UUID is used the following patterns can be used
    to initialize the control on a remote machine, to initialize a
    licensed control or to connect to a running object:
    \list
    \i To initialize the control on a different machine use the following
    pattern:
    \code
    <domain/username>:<password>@server/{8E27C92B-1264-101C-8A2F-040224009C02}
    \endcode
    \i To initialize a licensed control use the following pattern:
    \code
    {8E27C92B-1264-101C-8A2F-040224009C02}:<LicenseKey>
    \endcode
    \i To connect to an already running object use the following pattern:
    \code
    {8E27C92B-1264-101C-8A2F-040224009C02}&
    \endcode
    \endlist
    The first two patterns can be combined, e.g. to initialize a licensed
    control on a remote machine:
    \code
    ctrl->setControl("DOMAIN/user:password@server/{8E27C92B-1264-101C-8A2F-040224009C02}:LicenseKey");
    \endcode

    The control's read function always returns the control's UUID, if provided including the license
    key, and the name of the server, but not including the username, the domain or the password.
*/
bool QAxBase::setControl( const QString &c )
{
    if ( c == ctrl )
	return !ctrl.isEmpty();

    clear();
    ctrl = c;
    // don't waste time for DCOM requests
    if (ctrl.indexOf("/{") != (int)ctrl.length()-39 && !ctrl.endsWith("}&")) {
	QUuid uuid( ctrl );
	if ( uuid.isNull() ) {
	    CLSID clsid;
	    HRESULT res = CLSIDFromProgID((WCHAR*)c.ucs2(), &clsid);
	    if (res == S_OK)
		ctrl = QUuid(clsid).toString();
	    else {
		QSettings controls;
		ctrl = controls.readEntry( "/Classes/" + c + "/CLSID/Default" );
		if ( ctrl.isEmpty() ) {
		    QStringList clsids = controls.subkeyList( "/Classes/CLSID" );
		    for ( QStringList::Iterator it = clsids.begin(); it != clsids.end(); ++it ) {
			QString clsid = *it;
			QString name = controls.readEntry( "/Classes/CLSID/" + clsid + "/Default" );
			if ( name == c ) {
			    QStringList subkeys = controls.subkeyList( "/Classes/CLSID/" + clsid );
			    if ( subkeys.contains( "Control" ) || subkeys.contains( "Insertable" ) ) {
				ctrl = clsid;
				break;
			    }
			}
		    }
		}
	    }
	}
	if ( ctrl.isEmpty() )
	    ctrl = c;
    }
    d->tryCache = TRUE;
    if ( !initialize( &d->ptr ) )
	d->initialized = TRUE;
    if ( isNull() ) {
#ifndef QT_NO_DEBUG
	qWarning( "QAxBase::setControl: requested control %s could not be instantiated.", c.latin1() );
#endif
	clear();
	return FALSE;
    }
    return TRUE;
}

QString QAxBase::control() const
{
    return ctrl;
}

/*!
    Disables the event sink implementation for this ActiveX container.
    If you don't intend to listen to the ActiveX control's events use
    this function to speed up the meta object generation.

    Some ActiveX controls might be unstable when connected to an event
    sink. To get OLE events you must use standard COM methods to
    register your own event sink. Use queryInterface() to get access
    to the raw COM object.

    Note that this function should be called immediately after
    construction of the object (without passing an object identifier),
    and before calling QAxWidget->setControl().
*/
void QAxBase::disableEventSink()
{
    d->useEventSink = FALSE;
}

/*!
    Disables the meta object generation for this ActiveX container.
    This also disables the event sink and class info generation. If
    you don't intend to use the Qt meta object implementation call
    this function to speed up the meta object generation.

    Some ActiveX controls might be unstable when used with OLE
    automation. Use standard COM methods to use those controls through
    the COM interfaces provided by queryInterface().

    Note that this function must be called immediately after
    construction of the object (without passing an object identifier),
    and before calling QAxWidget->setControl().
*/
void QAxBase::disableMetaObject()
{
    d->useMetaObject	= FALSE;
    d->useEventSink	= FALSE;
    d->useClassInfo	= FALSE;
}

/*!
    Disables the class info generation for this ActiveX container. If
    you don't require any class information about the ActiveX control
    use this function to speed up the meta object generation.

    Note that this function must be called immediately after
    construction of the object (without passing an object identifier),
    and before calling QAxWidget->setControl().
*/
void QAxBase::disableClassInfo()
{
    d->useClassInfo = FALSE;
}

/*!
    Disconnects and destroys the COM object.

    If you reimplement this function you must also reimplement the
    destructor to call clear(), and call this implementation at the
    end of your clear() function.
*/
void QAxBase::clear()
{
    QHash<QString, QAxEventSink*>::Iterator it =  d->eventSink.begin();
    while (it != d->eventSink.end()) {
	QAxEventSink *eventSink = it.value();
	++it;
	eventSink->unadvise();
	eventSink->Release();
    }
    d->eventSink.clear();
    if ( d->disp ) {
	d->disp->Release();
	d->disp = 0;
    }
    if ( d->ptr ) {
	d->ptr->Release();
	d->ptr = 0;
	d->initialized = FALSE;
    }

    ctrl = QString::null;

    if ( d->metaobj && !d->cachedMetaObject ) {
	delete d->metaobj;
	d->metaobj = 0;
    }
}

/*!
    This virtual function is called by setControl() and creates the
    requested COM object. \a ptr is set to the object's IUnknown 
    implementation. The function returns TRUE if the object 
    initialization succeeded; otherwise the function returns FALSE.

    The default implementation interprets the string returned by
    control(), and calls initializeRemote(), initializeLicensed()
    or initializeActive() if the string matches the respective 
    patterns. If no pattern is matched, or if remote or licensed 
    initialization fails, CoCreateInstance is used directly to create 
    the object.

    See the \l control property documentation for details about
    supported patterns.

    The interface returned in \a ptr must be referenced exactly once
    when this function returns. The interface provided by e.g.
    CoCreateInstance is already referenced, and there is no need to
    reference it again.
*/
bool QAxBase::initialize( IUnknown **ptr )
{
    if ( *ptr || control().isEmpty() )
	return FALSE;

    *ptr = 0;

    bool res = FALSE;

    QString ctrl(control());
    if (ctrl.contains("/{")) // DCOM request
	res = initializeRemote(ptr);
    else if (ctrl.contains("}:")) // licensed control
	res = initializeLicensed(ptr);
    else if (ctrl.contains("}&")) // running object
	res = initializeActive(ptr);

    if (!res) // standard
	res = S_OK == CoCreateInstance( QUuid(ctrl), 0, CLSCTX_SERVER, IID_IUnknown, (void**)ptr );

    return *ptr != 0;
}

/*!
    Creates an instance of a licensed control, and returns the IUnknown interface
    to the object in \a ptr. This functions returns TRUE if successful, otherwise
    returns FALSE.

    This function is called by initialize() if the control string contains the 
    substring "}:". The license key needs to follow this substring.

    \sa initialize()
*/
bool QAxBase::initializeLicensed(IUnknown** ptr)
{
    int at = control().lastIndexOf("}:");

    QString clsid(control().left(at));
    QString key(control().mid(at+2));

    IClassFactory *factory = 0;
    CoGetClassObject(QUuid(clsid), CLSCTX_SERVER, 0, IID_IClassFactory, (void**)&factory);
    if (!factory)
	return FALSE;
    initializeLicensedHelper(factory, key, ptr);
    factory->Release();

    return *ptr != 0;
}

/* \internal
    Called by initializeLicensed and initializedRemote to create an object
    via IClassFactory2.
*/
bool QAxBase::initializeLicensedHelper(void *f, const QString &key, IUnknown **ptr)
{
    IClassFactory *factory = (IClassFactory*)f;
    IClassFactory2 *factory2 = 0;
    factory->QueryInterface(IID_IClassFactory2, (void**)&factory2);
    if (factory2) {
	BSTR bkey = QStringToBSTR(key);
	factory2->CreateInstanceLic(0, 0, IID_IUnknown, bkey, (void**)ptr);
	SysFreeString(bkey);
        factory2->Release();
    } else {  // give it a shot without license
	factory->CreateInstance(0, IID_IUnknown, (void**)ptr);
    }
    return *ptr != 0;
}

/*!
    Returns an active instance running on the current machine, and returns the
    IUnknown interface to the running object in \a ptr. This function returns TRUE 
    if successful, otherwise returns FALSE.

    This function is called by initialize() if the control string contains the 
    substring "}&".

    \sa initialize()
*/
bool QAxBase::initializeActive(IUnknown** ptr)
{
    int at = control().lastIndexOf("}&");
    QString clsid(control().left(at));

    GetActiveObject(QUuid(clsid), 0, ptr);

    return *ptr != 0;
}


// There seams to be a naming problem in mingw headers 
#ifdef Q_CC_GNU
#ifndef COAUTHIDENTITY
#define COAUTHIDENTITY AUTH_IDENTITY
#endif
#endif


/*!
    Creates the instance on a remote server, and returns the IUnknown interface
    to the object in \a ptr. This function returns TRUE if successful, otherwise
    returns FALSE.

    This function is called by initialize() if the control string contains the 
    substring "/{". The information about the remote machine needs to be provided
    in front of the substring.

    \sa initialize()
*/
bool QAxBase::initializeRemote(IUnknown** ptr)
{
    int at = control().lastIndexOf("/{");

    QString server(control().left(at));
    QString clsid(control().mid(at+1));

    QString user;
    QString domain;
    QString passwd;
    QString key;

    at = server.indexOf('@');
    if (at != -1) {
	user = server.left(at);
	server = server.mid(at+1);

	at = user.indexOf(':');
	if (at != -1 ) {
	    passwd = user.mid(at+1);
	    user = user.left(at);
	}
	at = user.indexOf('/');
	if (at != -1 ) {
	    domain = user.left(at);
	    user = user.mid(at+1);
	}
    }

    at = clsid.lastIndexOf("}:");
    if (at != -1) {
	key = clsid.mid(at+2);
	clsid = clsid.left(at);
    }

    ctrl = server + "/" + clsid;
    if (!key.isEmpty())
	ctrl = ctrl + ":" + key;

    COAUTHIDENTITY authIdentity;
    authIdentity.User = (ushort*)user.ucs2();
    authIdentity.UserLength = user.length();
    authIdentity.Domain = (ushort*)domain.ucs2();
    authIdentity.DomainLength = domain.length();
    authIdentity.Password = (ushort*)passwd.ucs2();
    authIdentity.PasswordLength = passwd.length();
    authIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    COAUTHINFO authInfo;
    authInfo.dwAuthnSvc = RPC_C_AUTHN_WINNT;
    authInfo.dwAuthzSvc = RPC_C_AUTHZ_NONE;
    authInfo.pwszServerPrincName = 0;
    authInfo.dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
    authInfo.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    authInfo.pAuthIdentityData = &authIdentity;
    authInfo.dwCapabilities = 0;

    COSERVERINFO serverInfo;
    serverInfo.dwReserved1 = 0;
    serverInfo.dwReserved2 = 0;
    serverInfo.pAuthInfo = &authInfo;
    serverInfo.pwszName = (WCHAR*)server.ucs2();

    IClassFactory *factory = 0;
    HRESULT res = CoGetClassObject(QUuid(clsid), CLSCTX_REMOTE_SERVER, &serverInfo, IID_IClassFactory, (void**)&factory);
    if (factory) {
	if (!key.isEmpty())
	    initializeLicensedHelper(factory, key, ptr);
	else
	    res = factory->CreateInstance(0, IID_IUnknown, (void**)ptr);
	factory->Release();
    }
#ifndef QT_NO_DEBUG
    if (res != S_OK)
	qSystemWarning("initializeRemote Failed", res);
#endif

    return res == S_OK;
}

/*!
    Requests the interface \a uuid from the COM object and sets the
    value of \a iface to the provided interface, or to 0 if the
    requested interface could not be provided.

    Returns the result of the QueryInterface implementation of the COM object.

    \sa control
*/
long QAxBase::queryInterface( const QUuid &uuid, void **iface ) const
{
    *iface = 0;
    if (!d->ptr) {
	((QAxBase*)this)->initialize(&d->ptr);
	d->initialized = TRUE;
    }

    if ( d->ptr && !uuid.isNull() )
	return d->ptr->QueryInterface( uuid, iface );

    return E_NOTIMPL;
}

static QString guessTypes( const TYPEDESC &tdesc, ITypeInfo *info, const QHash<QString, QMetaEnum*>& enumlist, const QString &function );

static inline QString usertypeToQString( const TYPEDESC &tdesc, ITypeInfo *info, const QHash<QString, QMetaEnum*>& enumlist, const QString &function )
{
    HREFTYPE usertype = tdesc.hreftype;
    if ( tdesc.vt != VT_USERDEFINED )
	return QString::null;

    QString typeName;
    ITypeInfo *usertypeinfo = 0;
    info->GetRefTypeInfo( usertype, &usertypeinfo );
    if ( usertypeinfo ) {
	ITypeLib *usertypelib = 0;
	UINT index;
	usertypeinfo->GetContainingTypeLib( &usertypelib, &index );
	if ( usertypelib ) {
	    // get type name
	    BSTR usertypename = 0;
	    usertypelib->GetDocumentation( index, &usertypename, 0, 0, 0 );
	    QString userTypeName = BSTRToQString( usertypename );
	    SysFreeString( usertypename );

	    // known enum?
	    QMetaEnum *metaEnum = enumlist[userTypeName];
	    if ( metaEnum )
		typeName = userTypeName;
	    else if ( userTypeName == "OLE_COLOR" || userTypeName == "VB_OLE_COLOR" )
		typeName = "QColor";
	    else if ( userTypeName == "IFontDisp" || userTypeName == "IFontDisp*" || userTypeName == "IFont" || userTypeName == "IFont*" )
		typeName = "QFont";
	    else if ( userTypeName == "Picture" || userTypeName == "Picture*" )
		typeName = "QPixmap";

	    if ( typeName.isEmpty() ) {
		TYPEATTR *typeattr = 0;
		usertypeinfo->GetTypeAttr( &typeattr );
		if ( typeattr ) {
		    if ( typeattr->typekind == TKIND_ALIAS )
			userTypeName = guessTypes( typeattr->tdescAlias, usertypeinfo, enumlist, function );
		    else if ( typeattr->typekind == TKIND_DISPATCH || typeattr->typekind == TKIND_COCLASS )
			userTypeName = "IDispatch";
		}

		usertypeinfo->ReleaseTypeAttr( typeattr );
		typeName = userTypeName;
	    }
	    usertypelib->Release();
	}
	usertypeinfo->Release();
    }
    return typeName;
}

static QString guessTypes( const TYPEDESC &tdesc, ITypeInfo *info, const QHash<QString, QMetaEnum*>& enumlist, const QString &function )
{
    QString str;
    switch ( tdesc.vt ) {
    case VT_VOID:
	str = "void";
	break;
    case VT_BSTR:
	str = "QString";
	break;
    case VT_BOOL:
	str = "bool";
	break;
    case VT_I1:
	str = "char";
	break;
    case VT_I2:
	str = "short";
	break;
    case VT_I4:
    case VT_INT:
	str = "int";
	break;
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_UINT:
	str = "uint";
	break;
    case VT_CY:
	str = "Q_LLONG";
	break;
    case VT_R4:
	str = "float";
	break;
    case VT_R8:
	str = "double";
	break;
    case VT_DATE:
	str = "QDateTime";
	break;
    case VT_DISPATCH:
	str = "IDispatch*";
	break;
    case VT_VARIANT:
	str = "QVariant";
	break;
    case VT_UNKNOWN:
	str = "IUnknown*";
	break;
    case VT_HRESULT:
	str = "HRESULT";
	break;
    case VT_PTR:
	str = guessTypes( *tdesc.lptdesc, info, enumlist, function );
	switch( tdesc.lptdesc->vt ) {
	case VT_BSTR:
	case VT_I1:
	case VT_I2:
	case VT_I4:
	case VT_UI1:
	case VT_UI2:
	case VT_UI4:
	case VT_BOOL:
	case VT_R4:
	case VT_R8:
	case VT_INT:
	case VT_UINT:
	case VT_CY:
	    str += "&";
	    break;
	case VT_PTR:
	    if ( str == "QFont" || str == "QPixmap" ) {
		str += "&";
		break;
	    }
	    // FALLTHROUGH
	default:
	    if ( str == "QColor" )
		str += "&";
	    else if ( str == "QDateTime" )
		str += "&";
	    else if ( str == "QList<QVariant>" )
		str += "&";
	    else if ( str == "QByteArray" )
		str += "&";
	    else if ( str == "QStringList" )
		str += "&";
	    else if ( str == "QVariant" )
		str = "const QVariant&";
	    else if ( !str.isEmpty() && enumlist[str] )
		str += "&";
	    else if ( !str.isEmpty() && str != "QFont" && str != "QPixmap" )
		str += "*";
	}
	break;
    case VT_SAFEARRAY:
	if ( tdesc.lpadesc->tdescElem.vt == VT_UI1 ) {
	    str = "QByteArray";
	    break;
	} else if (tdesc.lpadesc->tdescElem.vt == VT_BSTR) {
	    str = "QStringList";
	    break;
	}
	str = guessTypes( tdesc.lpadesc->tdescElem, info, enumlist, function );
	if ( !str.isEmpty() )
	    str = "QList<" + str + ">";
	break;
    case VT_CARRAY:
	str = guessTypes( tdesc.lpadesc->tdescElem, info, enumlist, function );
	if ( !str.isEmpty() ) {
	    for ( int index = 0; index < tdesc.lpadesc->cDims; ++index )
		str += "[" + QString::number( tdesc.lpadesc->rgbounds[index].cElements ) + "]";
	}
	break;
    case VT_USERDEFINED:
	str = usertypeToQString( tdesc, info, enumlist, function );
	break;

    default:
	break;
    }

    if ( tdesc.vt & VT_BYREF )
	str += "&";

    return str;
}

static inline QString constRefify( const QString& type )
{
    QString crtype;

    if ( type == "QString" )
	crtype = "const QString&";
    else if ( type == "QDateTime" )
	crtype = "const QDateTime&";
    else if ( type == "QVariant" )
	crtype = "const QVariant&";
    else if ( type == "QColor" )
	crtype = "const QColor&";
    else if ( type == "QFont" )
	crtype = "const QFont&";
    else if ( type == "QPixmap" )
	crtype = "const QPixmap&";
    else if ( type == "QList<QVariant>" )
	crtype = "const QList<QVariant>&";
    else if ( type == "QByteArray" )
	crtype = "const QByteArray&";
    else if ( type == "QStringList" )
	crtype = "const QStringList&";
    else
	crtype = type;

    return crtype;
}

static inline void QStringToQUType( const QString& fulltype, QUParameter *param, const QHash<QString, QMetaEnum*> &enumDict )
{
/* XXX
    QString type = fulltype;
    if ( type.startsWith( "const " ) )
	type = type.mid( 6 );
    if ( type.right(1) == "&" )
	type.truncate( type.length()-1 );

    param->typeExtra = 0;
    if ( type == "int" || type == "long" ) {
	param->type = &static_QUType_int;
    } else if ( type == "short" ) {
	param->type = &static_QUType_int;
	param->typeExtra = (void*)2; // byte size if not default
    } else if ( type == "char" ) {
	param->type = &static_QUType_int;
	param->typeExtra = (void*)1;
    } else if ( type == "uint" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::UInt);
    } else if ( type == "Q_LLONG" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::LongLong);
    } else if ( type == "Q_ULLONG" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::ULongLong);
    } else if ( type == "bool" ) {
	param->type = &static_QUType_bool;
    } else if ( type == "QString" ) {
	param->type = &static_QUType_QString;
    } else if ( type == "double" ) {
	param->type = &static_QUType_double;
    } else if ( type == "float" ) {
	param->type = &static_QUType_double;
	param->typeExtra = (void*)4; // byte size if not default
    } else if ( type == "QVariant" ) {
	param->type = &static_QUType_QVariant;
    } else if ( type == "QColor" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::Color);
    } else if ( type == "QDateTime" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::DateTime);
    } else if ( type == "QFont" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::Font);
    } else if ( type == "QPixmap" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::Pixmap);
    } else if ( type == "QList<QVariant>" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::List);
    } else if ( type == "QByteArray" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::ByteArray);
    } else if ( type == "QStringList" ) {
	param->type = &static_QUType_varptr;
	param->typeExtra = new char(QVariant::StringList);
    } else if ( enumDict[type] ) {
	QMetaEnum *enumData = enumDict[type];
	param->type = &static_QUType_enum;
	QUEnum *uEnum = new QUEnum;
	uEnum->count = enumData->count;
	uEnum->name = enumData->name;
	uEnum->set = enumData->set;
	uEnum->items = new QUEnumItem[uEnum->count];
	for ( uint item = 0; item < uEnum->count; ++item ) {
	    QUEnumItem *uEnumItem = (QUEnumItem *)uEnum->items+item;
	    uEnumItem->key = enumData->items[item].key;
	    uEnumItem->value = enumData->items[item].value;
	}
	param->typeExtra = uEnum;
    } else {
	param->type = &static_QUType_ptr;
	QString ptype = type;
	param->typeExtra = new char[ ptype.length() + 1 ];
	param->typeExtra = qstrcpy( (char*)param->typeExtra, ptype );
    }
*/
}

class QUMethod
{
};

class MetaObjectGenerator
{
public:
    MetaObjectGenerator( QAxBase *ax, QAxBasePrivate *dptr );
    ~MetaObjectGenerator();

    QMetaObject *metaObject( const QMetaObject *parentObject );

    void readClassInfo();
    void readEnumInfo();
    void readFuncInfo();
    void readEventInfo();
    QMetaObject *tryCache();

private:
    QAxBase *that;
    QAxBasePrivate *d;

    IDispatch *disp;
    ITypeInfo *typeinfo;
    ITypeLib *typelib;

    QSettings iidnames;
    QString cacheKey;
    QString debugInfo;
    QHash<QString, QUMethod*> slotlist; // QUMethods deleted
    QHash<QString, QUMethod*> signallist; // QUMethods deleted
    QHash<QString, QMetaProperty*> proplist;
#ifndef QAX_NO_CLASSINFO
    QHash<QString, QString*> infolist;
#endif
    QHash<QString, QMetaEnum*> enumlist;
    QHash<QString, QMetaEnum*> enumDict;	    // dict for fast lookup when generation property/parameter information
    QMetaEnum *enum_data;

    QUuid iid_propNotifySink;
};

MetaObjectGenerator::MetaObjectGenerator( QAxBase *ax, QAxBasePrivate *dptr )
: that( ax ), d( dptr ), typeinfo(0), typelib(0), enum_data(0)
{
    /* XXX
    proplist.setAutoDelete( TRUE ); // deep copied when creating metaobject
    infolist.setAutoDelete( TRUE ); // deep copied when creating metaobject
    enumlist.setAutoDelete( TRUE ); // deep copied when creating metaobject
    */

    if ( d->useClassInfo )
	iidnames.insertSearchPath( QSettings::Windows, "/Classes" );

    if ( d )
	disp = d->dispatch();

    iid_propNotifySink = IID_IPropertyNotifySink;
}

MetaObjectGenerator::~MetaObjectGenerator()
{
    if ( typeinfo ) typeinfo->Release();
    if ( typelib ) typelib->Release();

#ifndef QAX_NO_CLASSINFO
    if ( !!debugInfo && d->useClassInfo )
	infolist.insert( "debugInfo", new QString( debugInfo ) );
#endif
}

void MetaObjectGenerator::readClassInfo()
{
    // Read class information
    IProvideClassInfo *classinfo = 0;
    d->ptr->QueryInterface( IID_IProvideClassInfo, (void**)&classinfo );
    if ( classinfo ) {
	ITypeInfo *info = 0;
	classinfo->GetClassInfo( &info );
	TYPEATTR *typeattr = 0;
	if ( info )
	    info->GetTypeAttr( &typeattr );

	QString coClassID;
	if ( typeattr ) {
	    QUuid clsid( typeattr->guid );
	    coClassID = clsid.toString().toUpper();
#ifndef QAX_NO_CLASSINFO
	    // UUID
	    if ( d->useClassInfo && !infolist.find( "CoClass" ) ) {
		QString coClassIDstr = iidnames.readEntry( "/CLSID/" + coClassID + "/Default", coClassID );
		infolist.insert( "CoClass", new QString( coClassIDstr.isEmpty() ? coClassID : coClassIDstr ) );
		QString version( "%1.%1" );
		version = version.arg( typeattr->wMajorVerNum ).arg( typeattr->wMinorVerNum );
		infolist.insert( "Version", new QString( version ) );
	    }
#endif
	    info->ReleaseTypeAttr( typeattr );
	}
	if ( info ) info->Release();
	classinfo->Release();
	classinfo = 0;

	if ( d->tryCache && !coClassID.isEmpty() )
	    cacheKey = QString( "%1$%2$%3" ).arg( coClassID ).arg( (int)d->useEventSink ).arg( (int)d->useClassInfo );
    }

    UINT index = 0;
    if ( disp && !typeinfo )
	disp->GetTypeInfo( index, LOCALE_USER_DEFAULT, &typeinfo );

    if ( typeinfo && !typelib )
	typeinfo->GetContainingTypeLib( &typelib, &index );

    if ( !typelib ) {
	QSettings controls;
	QString tlid = controls.readEntry( "/Classes/CLSID/" + that->control() + "/TypeLib/." );
	QString tlfile;
	if ( !tlid.isEmpty() ) {
	    QStringList versions = controls.subkeyList( "/Classes/TypeLib/" + tlid );
	    QStringList::Iterator vit = versions.begin();
	    while ( tlfile.isEmpty() && vit != versions.end() ) {
		QString version = *vit;
		++vit;
		tlfile = controls.readEntry( "/Classes/Typelib/" + tlid + "/" + version + "/0/win32/." );
	    }
	} else {
	    tlfile = controls.readEntry( "/Classes/CLSID/" + that->control() + "/InprocServer32/." );
	    if (tlfile.isEmpty())
		tlfile = controls.readEntry( "/Classes/CLSID/" + that->control() + "/LocalServer32/." );
	}
	if (!tlfile.isEmpty()) {
	    LoadTypeLib((OLECHAR*)tlfile.ucs2(), &typelib);
	    if (!typelib) {
		tlfile = tlfile.left(tlfile.lastIndexOf('.')) + ".tlb";
		LoadTypeLib((OLECHAR*)tlfile.ucs2(), &typelib);
	    }
	}
    }

    if ( !typeinfo && typelib )
	typelib->GetTypeInfoOfGuid( QUuid(that->control()), &typeinfo );

    if ( !typeinfo || !cacheKey.isEmpty() || !d->tryCache )
	return;

    TYPEATTR *typeattr = 0;
    typeinfo->GetTypeAttr( &typeattr );

    QString interfaceID;
    if ( typeattr ) {
	QUuid iid( typeattr->guid );
	interfaceID = iid.toString().toUpper();

	typeinfo->ReleaseTypeAttr( typeattr );
	// ### event interfaces!!
	if ( !interfaceID.isEmpty() )
	    cacheKey = QString( "%1$%2$%3" ).arg( interfaceID ).arg( (int)d->useEventSink ).arg( (int)d->useClassInfo );
    }
}

void MetaObjectGenerator::readEnumInfo()
{
    if ( !typelib )
	return;

    UINT index = typelib->GetTypeInfoCount();
    for ( UINT i = 0; i < index; ++i ) {
	TYPEKIND typekind;
	typelib->GetTypeInfoType( i, &typekind );
	if ( typekind == TKIND_ENUM ) {
	    // Get the type information for the enum
	    ITypeInfo *enuminfo = 0;
	    typelib->GetTypeInfo( i, &enuminfo );
	    if ( !enuminfo )
		continue;

	    // Get the name of the enumeration
	    BSTR enumname;
	    QString enumName;
	    if ( typelib->GetDocumentation( i, &enumname, 0, 0, 0 ) == S_OK ) {
		enumName = BSTRToQString( enumname );
		SysFreeString( enumname );
	    } else {
		enumName = QString( "enum%1" ).arg( enumlist.count() );
	    }

	    // Get the attributes of the enum type
	    TYPEATTR *typeattr = 0;
	    enuminfo->GetTypeAttr( &typeattr );
	    if ( typeattr ) {
		// Create the QMetaEnum
		QMetaEnum * metaEnum = new QMetaEnum;
		/*
		metaEnum->name = new char[enumName.length() + 1];
		metaEnum->name = qstrcpy( (char*)metaEnum->name, enumName );
		metaEnum->items = typeattr->cVars ? new QMetaEnum::Item[typeattr->cVars] : 0;
		metaEnum->count = typeattr->cVars;
		metaEnum->set = FALSE;*/

		// Get all values of the enumeration
		for ( UINT vd = 0; vd < (UINT)typeattr->cVars; ++vd ) {
		    VARDESC *vardesc = 0;
		    enuminfo->GetVarDesc( vd, &vardesc );
		    if ( vardesc && vardesc->varkind == VAR_CONST ) {
			int value = vardesc->lpvarValue->lVal;
			int memid = vardesc->memid;
			// Get the name of the value
			BSTR valuename;
			QString valueName;
			UINT maxNamesOut;
			enuminfo->GetNames( memid, &valuename, 1, &maxNamesOut );
			if ( maxNamesOut ) {
			    valueName = BSTRToQString( valuename );
			    SysFreeString( valuename );
			} else {
			    valueName = QString( "value%1" ).arg( vd );
			}
			// Add to QMetaEnum
			/* XXX
			QMetaEnum::Item *items = (QMetaEnum::Item*)metaEnum->items;
			items[vd].value = value;
			items[vd].key = new char[valueName.length()+1];
			items[vd].key = qstrcpy( (char*)metaEnum->items[vd].key, valueName );
			*/
		    }
		    enuminfo->ReleaseVarDesc( vardesc );
		}
		// Add QMetaEnum to dict
		enumlist.insert( enumName, metaEnum );
	    }
	    enuminfo->ReleaseTypeAttr( typeattr );
	    enuminfo->Release();
	}
    }

    /* XXX
    // setup enum data array
    index = 0;
    enum_data = enumlist.count() ? new QMetaEnum[enumlist.count()] : 0;
    QDictIterator<QMetaEnum> enum_it( enumlist );
    while ( enum_it.current() ) {
	QMetaEnum metaEnum = *enum_it.current();
	enum_data[index].name = metaEnum.name;
	enum_data[index].count = metaEnum.count;
	enum_data[index].items = metaEnum.items;
	enum_data[index].set = metaEnum.set;
	enumDict.insert( enum_it.currentKey(), enum_data+index );
	++index;
	++enum_it;
    }
    */
}

void MetaObjectGenerator::readFuncInfo()
{
    while ( typeinfo ) {
	ushort nFuncs = 0;
	ushort nVars = 0;
	ushort nImpl = 0;
	// get information about type
	TYPEATTR *typeattr;
	typeinfo->GetTypeAttr( &typeattr );
	bool interesting = TRUE;
	if ( typeattr ) {
	    // get number of functions, variables, and implemented interfaces
	    nFuncs = typeattr->cFuncs;
	    nVars = typeattr->cVars;
	    nImpl = typeattr->cImplTypes;

	    if ( ( typeattr->typekind == TKIND_DISPATCH || typeattr->typekind == TKIND_INTERFACE ) &&
		( typeattr->guid != IID_IDispatch && typeattr->guid != IID_IUnknown ) ) {
#ifndef QAX_NO_CLASSINFO
		if ( d->useClassInfo ) {
		    // UUID
		    QUuid uuid( typeattr->guid );
		    QString uuidstr = uuid.toString().toUpper();
		    uuidstr = iidnames.readEntry( "/Interface/" + uuidstr + "/Default", uuidstr );
		    static int interfacecount = 0;
		    infolist.insert( QString("Interface %1").arg(++interfacecount), new QString( uuidstr ) );
		}
#endif
		typeinfo->ReleaseTypeAttr( typeattr );
	    } else {
		interesting = FALSE;
	    }
	}

	// get information about all functions
	if ( interesting ) for ( ushort fd = 0; fd < nFuncs ; ++fd ) {
	    FUNCDESC *funcdesc = 0;
	    typeinfo->GetFuncDesc( fd, &funcdesc );
	    if ( !funcdesc )
		break;

	    // get function prototype
	    TYPEDESC typedesc = funcdesc->elemdescFunc.tdesc;

	    QString function;
	    QString returnType;
	    QString prototype;
	    QStringList parameters;
	    QStringList paramTypes;

	    // parse function description
	    BSTR bstrNames[256];
	    UINT maxNames = 255;
	    UINT maxNamesOut;
	    typeinfo->GetNames( funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut );
	    QStringList names;
	    int p;
	    for ( p = 0; p < (int)maxNamesOut; ++p ) {
		names << BSTRToQString( bstrNames[p] );
		SysFreeString( bstrNames[p] );
	    }
	    function = names[0];
	    if ( ( maxNamesOut == 3 && function == "QueryInterface" ) ||
		 ( maxNamesOut == 1 && function == "AddRef" ) ||
		 ( maxNamesOut == 1 && function == "Release" ) ||
		 ( maxNamesOut == 9 && function == "Invoke" ) ||
		 ( maxNamesOut == 6 && function == "GetIDsOfNames" ) ||
		 ( maxNamesOut == 2 && function == "GetTypeInfoCount" ) ||
		 ( maxNamesOut == 4 && function == "GetTypeInfo" ) ) {
		typeinfo->ReleaseFuncDesc( funcdesc );
		continue;
	    }

	    p = 0;
	    for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it, ++p ) {
		QString paramName = *it;

		// function name
		if ( !p ) {
		    prototype = function + "(";

		    // get return value
		    returnType = guessTypes( typedesc, typeinfo, enumDict, function );

		    if ( returnType != "void" ) {
			if ( funcdesc->invkind == INVOKE_FUNC || returnType != "HRESULT" ) {
			    parameters << "return";
			    if ( returnType == "HRESULT" )
				returnType = "long";
			    paramTypes << returnType;
			}
		    }
		    continue;
		}

		// parameter
		bool optional = p > ( funcdesc->cParams - funcdesc->cParamsOpt );
		int offset = 0;
		if ( funcdesc->invkind == INVOKE_FUNC || paramTypes.count() )
		    offset = 1;

		TYPEDESC tdesc = funcdesc->lprgelemdescParam[ p - offset ].tdesc;
		PARAMDESC pdesc = funcdesc->lprgelemdescParam[ p - offset ].paramdesc;

		QString ptype = guessTypes( tdesc, typeinfo, enumDict, function );
		if ( pdesc.wParamFlags & PARAMFLAG_FRETVAL ) {
		    returnType = ptype;
		    paramTypes << returnType;
		} else {
		    if ( ptype.startsWith("const ") && pdesc.wParamFlags&PARAMFLAG_FOUT )
			prototype += ptype.mid( 6 );
		    else
			prototype += constRefify( ptype );
		    if ( optional )
			ptype += "=0";
		    paramTypes << ptype;
		    parameters << paramName;
		}
		if ( p < funcdesc->cParams && !(pdesc.wParamFlags & PARAMFLAG_FRETVAL) )
		    prototype += ",";
	    }

	    if ( !!prototype ) {
		if ( prototype.right(1) == "," )
		    prototype[(int)prototype.length()-1] = ')';
		else
		    prototype += ")";
	    }

	    /* XXX
	    QMetaProperty *prop = 0;

	    // get type of function
	    if ( !(funcdesc->wFuncFlags & FUNCFLAG_FHIDDEN) ) switch( funcdesc->invkind ) {
	    case INVOKE_PROPERTYGET: // property
	    case INVOKE_PROPERTYPUT:
		if ( paramTypes.count() <= 1 ) {
		    prop = proplist[function];
		    if ( !prop ) {
			QString ptype = paramTypes[0];
			if ( ptype.isEmpty() )
			    ptype = returnType;

			prop = new QMetaProperty;
			proplist.insert( function, prop );
			prop->meta = (QMetaObject**)&(d->metaobj);
			prop->_id = -1;
			if ( !ptype.isEmpty() )
			    prop->enumData = enumDict.find( ptype );
			else
			    prop->enumData = 0;
			prop->flags = QMetaProperty::Readable;
			if ( funcdesc->wFuncFlags & FUNCFLAG_FNONBROWSABLE )
			    prop->flags |= NotDesignable;
			if ( funcdesc->wFuncFlags & FUNCFLAG_FRESTRICTED )
			    prop->flags |= NotScriptable;
			if ( funcdesc->wFuncFlags & FUNCFLAG_FREQUESTEDIT )
			    prop->flags |= PropRequesting;
			if ( prop->enumData )
			    prop->flags |= QMetaProperty::EnumOrSet;

			if ( ptype != "void" ) {
			    prop->t = new char[ptype.length()+1];
			    prop->t = qstrcpy( (char*)prop->t, ptype );

			    prop->flags |= QVariant::nameToType( prop->t ) << 24;
			} else {
			    prop->t = 0;
			}
			prop->n = new char[function.length()+1];
			prop->n = qstrcpy( (char*)prop->n, function );

			if ( funcdesc->wFuncFlags & FUNCFLAG_FBINDABLE && prop->t ) {
			    prop->flags |= PropBindable;
			    QAxEventSink *eventSink = d->eventSink.find( iid_propNotifySink );
			    if ( !eventSink && d->useEventSink ) {
				eventSink = new QAxEventSink( that );
				d->eventSink.insert( iid_propNotifySink, eventSink );
			    }
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
				QStringToQUType( signalParam, param, enumDict );
				signal->parameters = param;

				signallist.insert( signalProto, signal );
			    }
			    if ( eventSink )
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
		}
		// break for getters, and if we can support the property,
		// otherwise fall through to generate property put as slot
		if ( funcdesc->invkind != INVOKE_PROPERTYPUT && paramTypes.count() <= 1 )
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
			QString ptype = prop->type();
			ptype = prop->type();
			if ( ptype.isEmpty() )
			    ptype = returnType;
			ptype = constRefify( ptype );
			prototype = function + "(" + ptype + ")";

			if ( !!ptype )
			    paramTypes = ptype;
			parameters = pname;
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
				    prototype += constRefify( paramTypes[pp] );
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
			    QStringToQUType( constRefify( paramType ), params + p, enumDict );
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
*/
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
	    typeinfo->ReleaseFuncDesc( funcdesc );
	}

	// get information about all variables
	if ( interesting ) for ( ushort vd = 0; vd < nVars; ++vd ) {
	    VARDESC *vardesc;
	    typeinfo->GetVarDesc( vd, &vardesc );
	    if ( !vardesc )
		break;

	    // no use if it's not a dispatched variable
	    if ( vardesc->varkind != VAR_DISPATCH ) {
		typeinfo->ReleaseVarDesc( vardesc );
		continue;
	    }

	    // get variable name
	    BSTR bstrName;
	    UINT maxNames = 1;
	    UINT maxNamesOut;
	    typeinfo->GetNames( vardesc->memid, &bstrName, maxNames, &maxNamesOut );
	    if ( maxNamesOut != 1 ) {
		typeinfo->ReleaseVarDesc( vardesc );
		continue;
	    }
	    QString variableName = BSTRToQString( bstrName );
	    SysFreeString( bstrName );

	    // get variable type
	    TYPEDESC typedesc = vardesc->elemdescVar.tdesc;
	    QString variableType = guessTypes( typedesc, typeinfo, enumDict, variableName );

/* XXX
	    if ( !(vardesc->wVarFlags & VARFLAG_FHIDDEN) ) {
		// generate meta property
		QMetaProperty *prop = proplist[variableName];
		if ( !prop ) {
		    prop = new QMetaProperty;
		    proplist.insert( variableName, prop );
		    prop->meta = (QMetaObject**)&d->metaobj;
		    prop->_id = -1;
		    if ( !variableType.isEmpty() )
			prop->enumData = enumDict.find( variableType );
		    else
			prop->enumData = 0;
		    prop->flags = QMetaProperty::Readable;
		    if ( !(vardesc->wVarFlags & VARFLAG_FREADONLY) )
			prop->flags |= QMetaProperty::Writable;;
		    if ( vardesc->wVarFlags & VARFLAG_FNONBROWSABLE )
			prop->flags |= NotDesignable;
		    if ( vardesc->wVarFlags & VARFLAG_FRESTRICTED )
			prop->flags |= NotScriptable;
		    if ( vardesc->wVarFlags & VARFLAG_FREQUESTEDIT )
			prop->flags |= PropRequesting;
		    if ( prop->enumData != 0 )
			prop->flags |= QMetaProperty::EnumOrSet;

		    prop->t = new char[variableType.length()+1];
		    prop->t = qstrcpy( (char*)prop->t, variableType );

		    prop->flags |= QVariant::nameToType( prop->t ) << 24;

		    prop->n = new char[variableName.length()+1];
		    prop->n = qstrcpy( (char*)prop->n, variableName );

		    if ( vardesc->wVarFlags & VARFLAG_FBINDABLE ) {
			prop->flags |= PropBindable;
			QAxEventSink *eventSink = d->eventSink.find( iid_propNotifySink );
			if ( !eventSink && d->useEventSink ) {
			    eventSink = new QAxEventSink( that );
			    d->eventSink.insert( iid_propNotifySink, eventSink );
			}
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
			    QStringToQUType( signalParam, param, enumDict );
			    signal->parameters = param;

			    signallist.insert( signalProto, signal );
			}
			if ( eventSink )
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
			variableName = firstletter.upper() + variableName.mid(1);
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

			QStringToQUType( variableType, params, enumDict );

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
*/
	    typeinfo->ReleaseVarDesc( vardesc );
	}

	if ( !nImpl ) {
	    typeinfo->Release();
	    typeinfo = 0;
	    break;
	}

	// go up one base class
	HREFTYPE pRefType;
	typeinfo->GetRefTypeOfImplType( 0, &pRefType );
	ITypeInfo *baseInfo = 0;
	typeinfo->GetRefTypeInfo( pRefType, &baseInfo );
	typeinfo->Release();
	if ( typeinfo == baseInfo ) { // IUnknown inherits IUnknown ???
	    baseInfo->Release();
	    typeinfo = 0;
	    break;
	}
	typeinfo = baseInfo;
    }
}

void MetaObjectGenerator::readEventInfo()
{
    IConnectionPointContainer *cpoints = 0;
    if ( d->useEventSink )
	d->ptr->QueryInterface( IID_IConnectionPointContainer, (void**)&cpoints );
    if ( cpoints ) {
	// Get connection point enumerator
	IEnumConnectionPoints *epoints = 0;
	cpoints->EnumConnectionPoints( &epoints );
	if ( epoints ) {
	    ULONG c = 1;
	    IConnectionPoint *cpoint = 0;
	    epoints->Reset();
	    QList<QUuid> cpointlist;
	    do {
		if ( cpoint ) cpoint->Release();
		cpoint = 0;
		epoints->Next( c, &cpoint, &c );
		if ( !c )
		    break;

		IID conniid;
		cpoint->GetConnectionInterface( &conniid );
		// workaround for typelibrary bug of Word.Application
		QUuid connuuid( conniid );
		if ( cpointlist.contains( connuuid ) )
		    break;

#ifndef QAX_NO_CLASSINFO
		if ( d->useClassInfo ) {
  		    QString uuidstr = connuuid.toString().toUpper();
  		    uuidstr = iidnames.readEntry( "/Interface/" + uuidstr + "/Default", uuidstr );
  		    static int eventcount = 0;
  		    infolist.insert( QString("Event Interface %1").arg(++eventcount), new QString( uuidstr ) );
		}
#endif

		// get information about type
		if ( conniid == IID_IPropertyNotifySink ) {
		    // test whether property notify sink has been created already, and advise on it
		    QAxEventSink *eventSink = d->eventSink[iid_propNotifySink];
		    if ( eventSink )
			eventSink->advise( cpoint, conniid );
		    continue;
		}
		ITypeInfo *eventinfo = 0;
		if ( typelib )
		    typelib->GetTypeInfoOfGuid( conniid, &eventinfo );

		if ( eventinfo ) {
		    // avoid recursion (see workaround above)
		    cpointlist.append( connuuid );

		    TYPEATTR *eventattr;
		    eventinfo->GetTypeAttr( &eventattr );
		    if ( !eventattr )
			continue;
		    if ( eventattr->typekind != TKIND_DISPATCH ) {
			eventinfo->ReleaseTypeAttr( eventattr );
			continue;
		    }

		    QAxEventSink *eventSink = d->eventSink[QUuid(conniid)];
		    if ( !eventSink ) {
			eventSink = new QAxEventSink( that );
			d->eventSink.insert( QUuid(conniid), eventSink );
			eventSink->advise( cpoint, conniid );
		    }

		    // get information about all event functions
		    for ( UINT fd = 0; fd < (UINT)eventattr->cFuncs; ++fd ) {
			FUNCDESC *funcdesc;
			eventinfo->GetFuncDesc( fd, &funcdesc );
			if ( !funcdesc )
			    break;
			if ( funcdesc->invkind != INVOKE_FUNC ||
			     funcdesc->funckind != FUNC_DISPATCH ) {
			    eventinfo->ReleaseTypeAttr( eventattr );
			    eventinfo->ReleaseFuncDesc( funcdesc );
			    continue;
			}

			// get return value
			TYPEDESC typedesc = funcdesc->elemdescFunc.tdesc;
			QString returnType;

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
				returnType = guessTypes( typedesc, eventinfo, enumDict, function );
				prototype = function + "(";
				continue;
			    }

			    // parameter
			    bool optional = p > funcdesc->cParams - funcdesc->cParamsOpt;

			    TYPEDESC tdesc = funcdesc->lprgelemdescParam[p - ( (funcdesc->invkind == INVOKE_FUNC) ? 1 : 0 ) ].tdesc;
			    QString ptype = guessTypes( tdesc, eventinfo, enumDict, function );

			    paramTypes << ptype;
			    parameters << paramName;
			    prototype += constRefify( ptype );
			    if ( optional )
				prototype += "=0";
			    if ( p < funcdesc->cParams )
				prototype += ",";
			}
			if ( !!prototype )
			    prototype += ")";

			if ( !signallist.find( prototype ) ) {
/* XXX
			    QUMethod *signal = new QUMethod;
			    signal->name = new char[function.length()+1];
			    signal->name = qstrcpy( (char*)signal->name, function );
			    signal->count = parameters.count();
			    QUParameter *params = signal->count ? new QUParameter[signal->count] : 0;
			    for ( p = 0; p< signal->count; ++p ) {
				QString paramName = parameters[p];
				QString paramType = constRefify( paramTypes[p] );
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

				QStringToQUType( paramType, params + p, enumDict );
			    }
			    signal->parameters = params;
			    signallist.insert( prototype, signal );
*/
			}
			if ( eventSink )
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
		    eventinfo->ReleaseTypeAttr( eventattr );
		    eventinfo->Release();
		}
	    } while ( c );
	    if ( cpoint ) cpoint->Release();
	    epoints->Release();
	}
	cpoints->Release();
    }
}

QMetaObject *MetaObjectGenerator::tryCache()
{
    if ( mo_cache && !cacheKey.isEmpty() ) {
	d->metaobj = (*mo_cache)[cacheKey];
	if ( d->metaobj ) {
	    d->cachedMetaObject = TRUE;
	    QList<QUuid>::ConstIterator it = d->metaobj->connectionInterfaces.begin();
	    while ( it != d->metaobj->connectionInterfaces.end() ) {
		QUuid iid = *it;
		++it;

		IConnectionPointContainer *cpoints = 0;
		d->ptr->QueryInterface( IID_IConnectionPointContainer, (void**)&cpoints );
		IConnectionPoint *cpoint = 0;
		if ( cpoints )
		    cpoints->FindConnectionPoint( iid, &cpoint );
		if ( cpoint ) {
		    QAxEventSink *sink = new QAxEventSink( that );
		    sink->advise( cpoint, iid );
		    d->eventSink.insert( iid, sink );
		    sink->sigs = d->metaobj->sigs[iid];

		    sink->props = d->metaobj->props[iid];
		    sink->propsigs = d->metaobj->propsigs[iid];
		    cpoints->Release();
		}
	    }

	    return d->metaobj;
	}
    }
    return 0;
}
/*
// some signals and properties are always there
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

static const QUParameter param_signal_2[] = {
    { "code", &static_QUType_int, 0, QUParameter::In },
    { "source", &static_QUType_QString, 0, QUParameter::In },
    { "description", &static_QUType_QString, 0, QUParameter::In },
    { "help", &static_QUType_QString, 0, QUParameter::In },
};
static const QUMethod signal_2 = {"exception", 4, param_signal_2 };
static const QMetaData signal_tbl[] = {
    { "signal(const QString&,int,void*)", &signal_0, QMetaData::Public },
    { "propertyChanged(const QString&)", &signal_1, QMetaData::Public },
    { "exception(int,const QString&,const QString&,const QString&)", &signal_2, QMetaData::Public }
};
static const QMetaProperty props_tbl[] = {
    { "QString","control", 259, (QMetaObject**)&tempMetaObj, 0, -1 }
};
*/

QMetaObject *MetaObjectGenerator::metaObject( const QMetaObject *parentObject )
{
    readClassInfo();
    if ( d->tryCache && tryCache() )
	return d->metaobj;
    readEnumInfo();
    readFuncInfo();
    readEventInfo();

#ifndef QAX_NO_CLASSINFO
    if ( !!debugInfo && d->useClassInfo )
	infolist.insert( "debugInfo", new QString( debugInfo ) );
#endif
/*
    // setup slot data
    UINT index = 0;
    QMetaData *const slot_data = slotlist.count() ? new QMetaData[slotlist.count()] : 0;
    QDictIterator<QUMethod> slot_it( slotlist );
    while ( slot_it.current() ) {
	QUMethod *slot = slot_it.current();
	QString slotname = slot_it.currentKey();
	int pi = slotname.find('(');
	int i = pi;
	while ((i = slotname.find("short", i)) != -1)
	    slotname.replace(i, 5, "int");
	i = pi;
	while ((i = slotname.find("char", i)) != -1)
	    slotname.replace(i, 4, "int");
	i = pi;
	while ((i = slotname.find("float", i)) != -1)
	    slotname.replace(i, 4, "double");

	slot_data[index].name = new char[slotname.length()+1];
	slot_data[index].name = qstrcpy( (char*)slot_data[index].name, slotname );
	slot_data[index].method = slot;
	slot_data[index].access = QMetaData::Public;

	++index;
	++slot_it;
    }

    // setup signal data
    index = 0;
    QMetaData *const signal_data = new QMetaData[signallist.count()+PredefSignals];
    while ( index < PredefSignals ) {
	signal_data[index] = signal_tbl[index];
	++index;
    }

    QDictIterator<QUMethod> signal_it( signallist );
    while ( signal_it.current() ) {
	QUMethod *signal = signal_it.current();
	QString signalname = signal_it.currentKey();
	int pi = signalname.find('(');
	int i = pi;
	while ((i = signalname.find("short", i)) != -1)
	    signalname.replace(i, 5, "int");
	i = pi;
	while ((i = signalname.find("char", i)) != -1)
	    signalname.replace(i, 4, "int");
	i = pi;
	while ((i = signalname.find("float", i)) != -1)
	    signalname.replace(i, 4, "double");

	signal_data[index].name = new char[signalname.length()+1];
	signal_data[index].name = qstrcpy( (char*)signal_data[index].name, signalname );
	signal_data[index].method = signal;
	signal_data[index].access = QMetaData::Public;

	++index;
	++signal_it;
    }

    // setup property data
    index = 0;
    QMetaProperty *const prop_data = new QMetaProperty[proplist.count()+PredefProps];
    while ( index < PredefProps ) {
	prop_data[index].n = new char[strlen(props_tbl[index].n)+1];
	prop_data[index].n = qstrcpy( (char*)prop_data[index].n, props_tbl[index].n );
	prop_data[index].t = new char[strlen(props_tbl[index].t)+1];
	prop_data[index].t = qstrcpy( (char*)prop_data[index].t, props_tbl[index].t );
	prop_data[index].flags = props_tbl[index].flags;
	prop_data[index]._id = props_tbl[index]._id;
	prop_data[index].enumData = props_tbl[index].enumData;
	prop_data[index].meta = (QMetaObject**)&d->metaobj;

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

#ifndef QAX_NO_CLASSINFO
    // setup class info data
    index = 0;
    QClassInfo *const class_info = infolist.count() ? new QMetaClassInfo[infolist.count()] : 0;
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
#endif
*/
    // put the metaobject together
    d->metaobj = new QAxMetaObject;
    d->metaobj->d.data = 0;
    d->metaobj->d.stringdata = 0;
    d->metaobj->d.superdata = 0;
/*XXX
    if ( !cacheKey.isEmpty() ) {
	mo_cache->insert( cacheKey, d->metaobj );
	d->cachedMetaObject = TRUE;
	for (QHash<QString, QAxEventSink*>::Iterator it = d->eventSink.begin(); it != d->eventSink.end(); ++it) {
	    QAxEventSink *sink = it.value();
	    QUuid ciid = sink->connectionInterface();

	    d->metaobj->connectionInterfaces.append( ciid );
	    d->metaobj->sigs.insert( ciid, sink->signalMap() );
	    d->metaobj->props.insert( ciid, sink->propertyMap() );
	    d->metaobj->propsigs.insert( ciid, sink->propSignalMap() );
	}
    }
*/
    return d->metaobj;
}

static const uint qt_meta_data_QAxBase[] = {

 // content:
       1,       // revision
       0,       // classname
       0,   12, // classinfo
       3,   12, // signals
       0,   27, // slots
       1,   27, // properties
       0,   30, // enums/sets

 // signals: signature, parameters, type, tag, flags
      46,   24,   23,   23, 0x0,
      90,   85,   23,   23, 0x0,
     130,  115,   23,   23, 0x0,

 // properties: name, type, flags
     164,  156, 0x03055103,

       0        // eod
};

static const char qt_meta_stringdata_QAxBase[] = {
    "QAxBase\0\0code,source,desc,help\0"
    "exception(int,QString,QString,QString)\0name\0propertyChanged(QString)\0"
    "name,argc,argv\0signal(QString,int,void*)\0QString\0control\0"
};

const QMetaObject QAxBase::staticMetaObject = {
    { &QObject::staticMetaObject,
      qt_meta_stringdata_QAxBase,
      qt_meta_data_QAxBase }
};


/*!
    \reimp

    The metaobject is generated on the fly from the information
    provided by the IDispatch and ITypeInfo interface implementations
    in the COM object.
*/
const QMetaObject *QAxBase::metaObject() const
{
    if ( d->metaobj )
	return d->metaobj;
    const QMetaObject* parentObject = parentMetaObject();

    if (!d->ptr && !d->initialized) {
	((QAxBase*)this)->initialize(&d->ptr);
	d->initialized = TRUE;
    }

#ifdef QT_THREAD_SUPPORT
    // only one thread at a time can generate meta objects
    QMutexLocker locker( &cache_mutex );
#endif

    // return the default meta object if not yet initialized
    if ( !d->ptr || !d->useMetaObject )
	return &staticMetaObject;

    MetaObjectGenerator generator( (QAxBase*)this, d );
    return generator.metaObject( parentObject );
}

/*!
    \fn QString QAxBase::generateDocumentation()

    Returns a rich text string with documentation for the
    wrapped COM object. Dump the string to an HTML-file,
    or use it in e.g. a QTextBrowser widget.
*/

static bool checkHRESULT( HRESULT hres, EXCEPINFO *exc, QAxBase *that, const char *name, uint argerr )
{
    switch( hres ) {
    case S_OK:
	return TRUE;
    case DISP_E_BADPARAMCOUNT:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Bad parameter count.", name );
#endif
	return FALSE;
    case DISP_E_BADVARTYPE:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Bad variant type.", name );
#endif
	return FALSE;
    case DISP_E_EXCEPTION:
	{
#if defined(QT_CHECK_STATE)
	    qWarning( "QAxBase: Error calling IDispatch member %s: Exception thrown by server.", name );
#endif
	    const QMetaObject *mo = that->metaObject();
	    int exceptionSignal = mo->indexOfSignal( "exception(int,const QString&,const QString&,const QString&)" );
	    if ( exceptionSignal >= 0 ) {
		if ( exc->pfnDeferredFillIn )
		    exc->pfnDeferredFillIn( exc );

		unsigned short code = exc->wCode ? exc->wCode : exc->scode;
		QString source = BSTRToQString( exc->bstrSource );
		QString desc = BSTRToQString( exc->bstrDescription );
		QString help = BSTRToQString( exc->bstrHelpFile );
		unsigned long helpContext = exc->dwHelpContext;

		if (helpContext && !help.isEmpty())
		    help += QString(" [%1]").arg(helpContext);

		void *argv[] = {&code, &source, &desc, &help, 0};
		that->qt_metacall(QMetaObject::EmitSignal, exceptionSignal, argv);
	    }
	}
	return FALSE;
    case DISP_E_MEMBERNOTFOUND:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Member not found.", name );
#endif
	return FALSE;
    case DISP_E_NONAMEDARGS:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: No named arguments.", name );
#endif
	return FALSE;
    case DISP_E_OVERFLOW:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Overflow.", name );
#endif
	return FALSE;
    case DISP_E_PARAMNOTFOUND:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Parameter %d not found.", name, argerr );
#endif
	return FALSE;
    case DISP_E_TYPEMISMATCH:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Type mismatch in parameter %d.", name, argerr );
#endif
	return FALSE;
    case DISP_E_UNKNOWNINTERFACE:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Unknown interface.", name );
#endif
	return FALSE;
    case DISP_E_UNKNOWNLCID:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Unknown locale ID.", name );
#endif
	return FALSE;
    case DISP_E_PARAMNOTOPTIONAL:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Non-optional parameter missing.", name );
#endif
	return FALSE;
    default:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Unknown error.", name );
#endif
	return FALSE;
    }
}

/*!
    \internal
*
bool QAxBase::qt_invoke( int _id, QUObject* _o )
{
    const int index = _id - metaObject()->slotOffset();
    if ( !d->ptr || index < 0 )
	return FALSE;

    // get the IDispatch
    IDispatch *disp = d->dispatch();
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
    QString name( slot->name );
    OLECHAR *names = (TCHAR*)name.ucs2();
    disp->GetIDsOfNames( IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid );
    if ( dispid == DISPID_UNKNOWN ) {
	// see if we are calling a property set function as a slot
	if ( QString( slot->name ).left( 3 ) != "set" &&
	     QString( slot->name ).left( 3 ) != "Set" )
	    return FALSE;
	QString realname = slot->name;
	realname = realname.right( realname.length() - 3 );
	OLECHAR *realnames = (TCHAR*)realname.ucs2();
	disp->GetIDsOfNames( IID_NULL, &realnames, 1, LOCALE_USER_DEFAULT, &dispid );
	if ( dispid == DISPID_UNKNOWN )
	    return FALSE;

	fakedslot = TRUE;
    }

    // setup the parameters
    VARIANT ret; // Invoke initializes it
    VARIANT *pret = 0;
    if ( slot->parameters && ( slot->parameters[0].inOut == QUParameter::Out ) ) // slot has return value
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
	QUObjectToVARIANT( obj, arg, slot->parameters + p + ( pret ? 1 : 0 ) );
	params.rgvarg[ slotcount - p - 1 ] = arg;
    }
    // call the method
    UINT argerr = 0;
    HRESULT hres;
    EXCEPINFO excepinfo;
    if ( fakedslot && slotcount == 1 ) {
	hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, pret, &excepinfo, &argerr );
    } else {
	hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, pret, &excepinfo, &argerr );
    }

    // get return value
    if ( pret )
	VARIANTToQUObject( ret, _o, slot->parameters );

    // update out parameters
    for ( p = 0; p < slotcount; ++p ) {
	if ( slot->parameters && slot->parameters[p + (pret ? 1 : 0)].inOut & QUParameter::Out ) {
	    QUObject *obj = _o + p+1;
	    arg = params.rgvarg[ slotcount - p-1 ];
	    VARIANTToQUObject( arg, obj, slot->parameters + p + (pret ? 1 : 0) );
	}
    }

    // clean up
    for ( p = 0; p < slotcount; ++p )
	clearVARIANT( params.rgvarg+p );
    delete [] params.rgvarg;

    return checkHRESULT( hres, &excepinfo, this, slot->name, slotcount-argerr-1 );
}
*/
bool wrapComPointer( QVariant &var, VARTYPE vt, QObject *parent, const char *name )
{
    if ( !var.isNull() && !var.isValid() && ( vt == VT_DISPATCH || vt == VT_UNKNOWN ) ) {
	IUnknown *ptr = (IUnknown *)var.rawAccess();
	var.rawAccess( new QAxObject( ptr, parent, name ) );
	return TRUE;
    }
    return FALSE;
}

/*!
    \reimp
*
bool QAxBase::qt_property( int _id, int _f, QVariant* _v )
{
    const int index = _id - metaObject()->propertyOffset();
    if ( index == 0 ) { // control property
	switch( _f ) {
	case 0: setControl( _v->toString() ); break;
	case 1: *_v = control(); break;
	default: if ( _f > 5 ) return FALSE;
	}
	return TRUE;
    } else if ( d->ptr && index >= 0 ) {
	// get the IDispatch
	IDispatch *disp = d->dispatch();
	if ( !disp )
	    return FALSE;

	// get the property information
	const QMetaProperty *prop = metaObject()->property( index );
	if ( !prop )
	    return FALSE;

	//  get the Dispatch ID of the function to be called
	QString pname( prop->n );

	DISPID dispid;
	OLECHAR *names = (TCHAR*)pname.ucs2();
	disp->GetIDsOfNames( IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid );
	if ( dispid == DISPID_UNKNOWN )
	    return FALSE;

	switch ( _f ) {
	case 0: // Set
	    {
		VARIANTARG arg;
		arg.vt = VT_ERROR;
		arg.scode = DISP_E_TYPEMISMATCH;

		// map QVariant to VARIANTARG.
		QVariantToVARIANT( *_v, arg, prop->type() );

		if ( arg.vt == VT_EMPTY ) {
#ifndef QT_NO_DEBUG
		    qDebug( "QAxBase::setProperty(): Unhandled property type" );
#endif
		    return FALSE;
		}
		DISPPARAMS params;
		DISPID dispidNamed = DISPID_PROPERTYPUT;
		params.rgvarg = &arg;
		params.cArgs = 1;
		params.rgdispidNamedArgs = &dispidNamed;
		params.cNamedArgs = 1;

		UINT argerr = 0;
		EXCEPINFO excepinfo;
		HRESULT hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, 0, &excepinfo, &argerr );
		clearVARIANT( &arg );

		return checkHRESULT( hres, &excepinfo, this, prop->n, argerr );
	    }
	case 1: // Get
	    {
		VARIANTARG arg;
		VariantInit( &arg );
		DISPPARAMS params;
		params.cArgs = 0;
		params.cNamedArgs = 0;
		params.rgdispidNamedArgs = 0;
		params.rgvarg = 0;

		EXCEPINFO excepinfo;
		HRESULT hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &arg, &excepinfo, 0 );
		if ( !checkHRESULT( hres, &excepinfo, this, prop->n, 0 ) )
		    return FALSE;

		// map result VARIANTARG to QVariant
		*_v = VARIANTToQVariant( arg, prop->type() );
		bool comOnlyType = wrapComPointer( *_v, arg.vt, qObject(), prop->name() );
		clearVARIANT( &arg );
		return ( _v->isValid() || comOnlyType );
	    }
	case 2: // Reset
	    return TRUE;
	case 3: // Designable
	    return !prop->testFlags(NotDesignable);
	case 4: // Scriptable
	    return !prop->testFlags(NotScriptable);
	case 5: // Stored
	    return !prop->testFlags(NotStored);
	default:
	    break;
	}
    }
    return FALSE;
}
*/

/*!
    \internal
*/
int QAxBase::qt_metacall(QMetaObject::Call call, int id, void **v)
{
    return 0;
}

/*!
    \internal
*/
bool QAxBase::internalInvoke( const QString &name, void *inout, QVariant vars[], QString &type )
{
    IDispatch *disp = d->dispatch();
    if ( !disp )
	return FALSE;

    int varc = 0;
    while ( vars[varc].isValid() ) {
	if ( vars[varc].type() == -1 )
	    vars[varc] = QVariant();
	varc++;
    }

    QString function = name;
    VARIANT *arg = varc ? new VARIANT[varc] : 0;
    VARIANTARG *res = (VARIANTARG*)inout;
    unsigned short disptype;

    int id = -1;
/*
    if ( function.contains( '(' ) ) {
	disptype = DISPATCH_METHOD;
	id = metaObject()->indexOfSlot(QMetaObject::normalizedSignature(function));
	const QMetaData *slot = 0;
	int retoff = 0;
	if ( id >= 0 ) {
	    slot = metaObject()->slot( id, TRUE );
	    function = slot->method->name;
	    retoff = ( slot->method->count && ( slot->method->parameters->inOut == QUParameter::Out ) ) ? 1 : 0;
	    if ( slot->method->count - retoff < varc )
		varc = slot->method->count - retoff;
	    if ( retoff ) {
		const QUParameter *retparam = slot->method->parameters;
		if ( QUType::isEqual( retparam->type, &static_QUType_ptr ) )
		    type = (const char*)retparam->typeExtra;
		else if ( QUType::isEqual( retparam->type, &static_QUType_QVariant ) && retparam->typeExtra )
		    type = QVariant::typeToName( (QVariant::Type)*(char*)retparam->typeExtra );
		else if ( QUType::isEqual( retparam->type, &static_QUType_varptr )  && retparam->typeExtra )
		    type = QVariant::typeToName( (QVariant::Type)*(char*)retparam->typeExtra );
		else
		    type = retparam->type->desc();
	    }
	} else {
	    function = function.left(function.find('('));
	}
	if (varc) {
	    for ( int i = 0; i < varc; ++i ) {
		const QUParameter *param = slot ? slot->method->parameters + i + retoff : 0;
		VariantInit( arg + (varc-i-1) );
		if (param)
		    QVariantToVARIANT( vars[i], arg[varc-i-1], param );
		else
		    QVariantToVARIANT( vars[i], arg[varc-i-1], "QVariant" );
	    }
	} else if (name.length() > function.length() + 2) {
	    QString args = name.mid(function.length() + 1);
	    // parse argument string int list of arguments
	    QStringList argList;
	    QString curArg;
	    const QChar *c = args.unicode();
	    int index = 0;
	    bool inString = FALSE;
	    bool inEscape = FALSE;
	    while (index < (int)args.length()) {
		QChar cc = *c;
		++c;
		++index;
		switch(cc.latin1()) {
		case 'n':
		    if (inEscape)
			cc = '\n';
		    break;
		case 'r':
		    if (inEscape)
			cc = '\r';
		    break;
		case 't':
		    if (inEscape)
			cc = '\t';
		    break;
		case '\\':
		    if (!inEscape && inString) {
			inEscape = TRUE;
			continue;
		    }
		    break;
		case '"':
		    if (!inEscape) {
			inString = !inString;
			continue;
		    }
		    break;
		case ' ':
		    if (!inString && curArg.isEmpty())
			continue;
		    break;
		case ',':
		case ')':
		    if (inString)
			break;
		    curArg = curArg.stripWhiteSpace();
		    argList += curArg;
		    curArg = QString::null;
		    continue;
		default:
		    break;
		}
		inEscape = FALSE;
		curArg += cc;
	    }

	    varc = argList.count();
	    if (varc) {
		arg = new VARIANT[varc];
		int index = 0;
		for (QStringList::Iterator it = argList.begin(); it != argList.end(); ++it, ++index) {
		    QVariant var(*it);
		    VariantInit( arg + (varc-index-1) );
		    QVariantToVARIANT( var, arg[varc-index-1], "QVariant" );
		}
	    }
	}
    } else {
	id = metaObject()->findProperty( name, TRUE );
	if ( id >= 0 ) {
	    const QMetaProperty *prop = metaObject()->property( id, TRUE );
	    type = prop->type();
	    if ( varc ) {
		varc = 1;
		QVariantToVARIANT( vars[0], arg[0], type );
		res = 0;
		disptype = DISPATCH_PROPERTYPUT;
	    } else {
		disptype = DISPATCH_PROPERTYGET;
	    }
	} else {
	    function = name;
	    if ( varc ) {
		varc = 1;
		QVariantToVARIANT( vars[0], arg[0], "QVariant" );
		res = 0;
		disptype = DISPATCH_PROPERTYPUT;
	    } else {
		disptype = DISPATCH_PROPERTYGET;
	    }
	}
    }
*/
    DISPID dispid;
    OLECHAR *names = (TCHAR*)function.ucs2();
    disp->GetIDsOfNames( IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid );
    if ( dispid == DISPID_UNKNOWN && function.toLower().left(3) == "set" ) {
	function = function.mid( 3 );
	OLECHAR *names = (TCHAR*)function.ucs2();
	disptype = DISPATCH_PROPERTYPUT;
	disp->GetIDsOfNames( IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid );
    }

    if ( dispid == DISPID_UNKNOWN ) {
#ifdef QT_CHECK_STATE
	const char *coclass = metaObject()->classInfo( "CoClass" );
	qWarning( "QAxBase::internalInvoke: %s: No such method or property in %s [%s]", (const char*)name, control().latin1(),
	    coclass ? coclass: "unknown" );

	if (disptype == DISPATCH_METHOD) {
	    for (int i = 0; i < metaObject()->numSlots(TRUE); ++i) {
		const QMetaData *slot = metaObject()->slot(i, TRUE);
		if (QString(slot->name).startsWith(function))
		    qWarning("\tDid you mean %s?", slot->name);
	    }
	}
#endif
	return FALSE;
    }

    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;

    params.cArgs = varc;
    params.cNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? 1 : 0;
    params.rgdispidNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? &dispidNamed : 0;
    params.rgvarg = arg;
    EXCEPINFO excepinfo;
    UINT argerr = 0;

    HRESULT hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, disptype, &params, res, &excepinfo, &argerr );
    if ( hres == DISP_E_MEMBERNOTFOUND && disptype == DISPATCH_METHOD )
	hres = disp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, res, &excepinfo, &argerr );

    if ( disptype == DISPATCH_METHOD && id >= 0 && varc ) {
	for ( int i = 0; i < varc; ++i )
	    if ( arg[varc-i-1].vt & VT_BYREF ) // update out-parameters
		vars[i] = VARIANTToQVariant( arg[varc-i-1], vars[i].typeName() );
    }

    // clean up
    for ( int i = 0; i < varc; ++i )
	clearVARIANT( params.rgvarg+i );
    delete[] arg;

    return checkHRESULT( hres, &excepinfo, this, function, varc-argerr-1 );
}


/*!
    Calls the COM object's method \a function, passing the
    parameters \a var1, \a var1, \a var2, \a var3, \a var4, \a var5,
    \a var6, \a var7 and \a var8, and returns the value returned by
    the method, or an invalid QVariant if the method does not return
    a value or when the function call failed.

    If \a function is a method of the object the string must be provided
    as the full prototype, for example as it would be written in a
    QObject::connect() call.
    \code
    activeX->dynamicCall( "Navigate(const QString&)", "www.trolltech.com" );
    \endcode

    Alternatively a function can be called passing the parameters embedded
    in the string, e.g. above function can also be invoked using
    \code
    activeX->dynamicCall("Navigate(\"www.trolltech.com\");
    \endcode
    All parameters are passed as strings; it depends on the control whether
    they are interpreted correctly, and is slower than using the prototype
    with correctly typed parameters.

    If \a function is a property the string has to be the name of the
    property. The property setter is called when \a var1 is a valid QVariant,
    otherwise the getter is called.
    \code
    activeX->dynamicCall( "Value", 5 );
    QString text = activeX->dynamicCall( "Text" ).toString();
    \endcode
    Note that it is faster to get and set properties using
    QObject::property() and QObject::setProperty().

    It is only possible to call functions through dynamicCall() that
    have parameters or return values of datatypes supported by
    QVariant. See the QAxBase class documentation for a list of
    supported and unsupported datatypes. If you want to call functions
    that have unsupported datatypes in the parameter list, use
    queryInterface() to retrieve the appropriate COM interface, and
    use the function directly.

    \code
    IWebBrowser2 *webBrowser = 0;
    activeX->queryInterface( IID_IWebBrowser2, (void**)&webBrowser );
    if ( webBrowser ) {
        webBrowser->Navigate2( pvarURL );
	webBrowser->Release();
    }
    \endcode

    This is also more efficient.
*/
QVariant QAxBase::dynamicCall( const QString &function, const QVariant &var1,
							const QVariant &var2,
							const QVariant &var3,
							const QVariant &var4,
							const QVariant &var5,
							const QVariant &var6,
							const QVariant &var7,
							const QVariant &var8 )
{
    VARIANTARG res;
    res.vt = VT_EMPTY;

    int varc = 0;
    QVariant vars[9]; // 8 + terminating invalid
    vars[varc++] = var1;
    vars[varc++] = var2;
    vars[varc++] = var3;
    vars[varc++] = var4;
    vars[varc++] = var5;
    vars[varc++] = var6;
    vars[varc++] = var7;
    vars[varc++] = var8;
    vars[varc++] = QVariant();

    QString rettype;
    if ( !internalInvoke( function, &res, vars, rettype ) )
	return QVariant();

    QVariant qvar = VARIANTToQVariant( res, rettype );
    clearVARIANT( &res );

    return qvar;
}

/*!
    \overload

    Calls the COM object's method \a function, passing the
    parameters in \a vars, and returns the value returned by
    the method. If the method does not return a value or when
    the function call failed this function returns an invalid
    QVariant object.

    The QVariant objects in \a vars are updated when the method has
    out-parameters.
*/
QVariant QAxBase::dynamicCall( const QString &function, QList<QVariant> &vars )
{
    VARIANTARG res;
    res.vt = VT_EMPTY;

    const int count = vars.count();
    QVariant *vararray = new QVariant[ count + 1 ];
    int i = 0;
    for ( QList<QVariant>::Iterator it = vars.begin(); it != vars.end(); ++it ) {
	QVariant var = *it;
	if ( !var.isValid() )
	    var.rawAccess( this, (QVariant::Type)-1 );
	vararray[i++] = var;
    }

    QString rettype;
    bool ok = internalInvoke( function, &res, vararray, rettype );
    if ( ok ) {
	vars.clear();
	for ( i = 0; i < count; ++i )
	    vars << vararray[i];
    }
    delete[] vararray;

    QVariant qvar = VARIANTToQVariant( res, rettype );
    clearVARIANT( &res );

    return qvar;
}

/*!
    Returns a pointer to a QAxObject wrapping the COM object provided
    by the method or property \a name, passing passing the parameters
    \a var1, \a var1, \a var2, \a var3, \a var4, \a var5, \a var6,
    \a var7 and \a var8.

    If \a name is provided by a method the string must include the
    full function prototype.

    If \a name is a property the string must be the name of the property,
    and \a var1, ... \a var8 are ignored.

    The returned QAxObject is a child of this object (which is either of
    type QAxObject or QAxWidget), and is deleted when this object is
    deleted. It is however safe to delete the returned object yourself,
    and you should do so when you iterate over lists of subobjects.

    COM enabled applications usually have an object model publishing
    certain elements of the application as dispatch interfaces. Use
    this method to navigate the hierarchy of the object model, e.g.

    \code
    QAxWidget outlook( "Outlook.Application" );
    QAxObject *session = outlook.querySubObject( "Session" );
    if ( session ) {
	QAxObject *defFolder = session->querySubObject(
				"GetDefaultFolder(OlDefaultFolders)",
				"olFolderContacts" );
	//...
    }
    \endcode
*/
QAxObject *QAxBase::querySubObject( const QString &name, const QVariant &var1,
							 const QVariant &var2,
							 const QVariant &var3,
							 const QVariant &var4,
							 const QVariant &var5,
							 const QVariant &var6,
							 const QVariant &var7,
							 const QVariant &var8 )
{
    QAxObject *object = 0;

    VARIANTARG res;

    int varc = 0;
    QVariant vars[9];
    vars[varc++] = var1;
    vars[varc++] = var2;
    vars[varc++] = var3;
    vars[varc++] = var4;
    vars[varc++] = var5;
    vars[varc++] = var6;
    vars[varc++] = var7;
    vars[varc++] = var8;
    vars[varc++] = QVariant();

    QString rettype;
    if ( !internalInvoke( name, &res, vars, rettype ) )
	return 0;

    switch ( res.vt ) {
    case VT_DISPATCH:
	if ( res.pdispVal ) {
	    object = new QAxObject(res.pdispVal, qObject(), QString(qObject()->objectName()) + "/" + name);
	    ((QAxBase*)object)->d->tryCache = TRUE;
	}
	break;
    case VT_UNKNOWN:
	if ( res.punkVal ) {
	    object = new QAxObject(res.punkVal, qObject(), QString(qObject()->objectName()) + "/" + name);
	    ((QAxBase*)object)->d->tryCache = TRUE;
	}
	break;
    case VT_EMPTY:
#ifdef QT_CHECK_STATE
	{
	    const char *coclass = metaObject()->classInfo("CoClass");
	    qWarning( "QAxBase::querySubObject: %s: error calling function or property in %s (%s)"
		, (const char*)name, control().latin1(), coclass ? coclass: "unknown" );
	}
#endif
	break;
    default:
#ifdef QT_CHECK_STATE
	{
	    const char *coclass = metaObject()->classInfo( "CoClass" );
	    qWarning( "QAxBase::querySubObject: %s: method or property is not of interface type in %s (%s)"
		, (const char*)name, control().latin1(), coclass ? coclass: "unknown" );
	}
#endif
	break;
    }

    clearVARIANT( &res );
    return object;
}



class QtPropertyBag : public IPropertyBag
{
public:
    QtPropertyBag() :ref( 0 ) {}
    virtual ~QtPropertyBag() {}
   
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

	QString property = BSTRToQString((TCHAR*)name).local8Bit();
	QVariant qvar = map[property];
	QVariantToVARIANT( qvar, *var, qvar.typeName() );
	return S_OK;
    }
    HRESULT __stdcall Write( LPCOLESTR name, VARIANT *var )
    {
	if ( !var )
	    return E_POINTER;
	QString property = BSTRToQString((TCHAR*)name).local8Bit();
	QVariant qvar = VARIANTToQVariant( *var, 0 );
	map[property] = qvar;

	return S_OK;
    }

    QAxBase::PropertyBag map;

private:
    unsigned long ref;
};

/*!
    Returns a name:value map of all the properties exposed by the COM
    object.

    This is more efficient than getting multiple properties
    individually if the COM object supports property bags.

    \warning It is not guaranteed that the property bag implementation
    of the COM object returns all properties, or that the properties
    returned are the same as those available through the IDispatch
    interface.
*/
QAxBase::PropertyBag QAxBase::propertyBag() const
{
    PropertyBag result;

    if (!d->ptr && !d->initialized) {
	((QAxBase*)this)->initialize(&d->ptr);
	d->initialized = TRUE;
    }

    if ( isNull() )
	return result;
    IPersistPropertyBag *persist = 0;
    d->ptr->QueryInterface( IID_IPersistPropertyBag, (void**)&persist );
    if ( persist ) {
	QtPropertyBag *pbag = new QtPropertyBag();
	pbag->AddRef();
	persist->Save( pbag, FALSE, TRUE );
	result = pbag->map;
	pbag->Release();
	persist->Release();
	return result;
    } else {
/* XXX
	QAxBase *that = (QAxBase*)this;
	const QMetaObject *mo = metaObject();
	for ( int p = 1; p < mo->propertyCount(); ++p ) {
	    // ignore parent class properties!
	    const QMetaProperty *property = mo->property(p);
	    if ( QVariant::nameToType(property->type()) == QVariant::Invalid &&
		!mo->enumeratorNames(FALSE).contains(property->type()) )
		continue;
	    QVariant var;
	    that->qt_property( p+metaObject()->propertyOffset(), 1, &var );
	    result.insert( property->name(), var );
	}
*/
    }
    return result;
}

/*!
    Sets the properties of the COM object to the corresponding values
    in \a bag.

    \warning
    You should only set property bags that have been returned by the
    propertyBag function, as it cannot be guaranteed that the property
    bag implementation of the COM object supports the same properties
    that are available through the IDispatch interface.

    \sa propertyBag()
*/
void QAxBase::setPropertyBag( const PropertyBag &bag )
{
    if (!d->ptr && !d->initialized) {
	initialize(&d->ptr);
	d->initialized = TRUE;
    }

    if ( isNull() )
	return;
    IPersistPropertyBag *persist = 0;
    d->ptr->QueryInterface( IID_IPersistPropertyBag, (void**)&persist );
    if ( persist ) {
	QtPropertyBag *pbag = new QtPropertyBag();
	pbag->map = bag;
	pbag->AddRef();
	persist->Load( pbag, 0 );
	pbag->Release();
	persist->Release();
    } else {
/* XXX
	QAxBase *that = (QAxBase*)this;
	for ( int p = 1; p < metaObject()->numProperties( FALSE ); ++p ) {
	    QVariant var = bag[metaObject()->property( p, FALSE )->name()];
	    that->qt_property( p+metaObject()->propertyOffset(), 0, &var );
	}
*/    
    }
}

/*!
    Returns TRUE if the property \a prop is writable; otherwise
    returns FALSE. By default, all properties are writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa setPropertyWritable(), propertyChanged()
*/
bool QAxBase::propertyWritable( const char *prop ) const
{
    if ( !d->propWritable )
	return TRUE;

    if ( !d->propWritable->contains( prop ) )
	return TRUE;
    else
	return (*d->propWritable)[prop];
}

/*!
    Sets the property \a prop to writable if \a ok is TRUE, otherwise
    sets \a prop to be read-only. By default, all properties are
    writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa propertyWritable(), propertyChanged()
*/
void QAxBase::setPropertyWritable( const char *prop, bool ok )
{
    if ( !d->propWritable )
	d->propWritable = new QMap<QString, bool>;

    (*d->propWritable)[prop] = ok;
}

/*!
    Returns TRUE if there is no COM object loaded by this wrapper;
    otherwise return FALSE.

    \sa control
*/
bool QAxBase::isNull() const
{
    return !d->ptr;
}

/*!
    Returns a QVariant that wraps the COM object. The variant can
    then be used as a parameter in e.g. dynamicCall().
*/
QVariant QAxBase::asVariant() const
{
    if (!d->ptr && !d->initialized) {
	((QAxBase*)this)->initialize(&d->ptr);
	d->initialized = TRUE;
    }

    QVariant var;
    if ( isNull() )
	return var;

    void *iface = d->dispatch();
    if ( !iface ) 
	iface = d->ptr;
    var.rawAccess( iface, (QVariant::Type)1000 );
    return var;
}

/*!
    \fn QObject *QAxBase::qObject()
    \internal
*/

/*!
    \fn void QAxBase::signal( const QString &name, int argc, void *argv )

    This generic signal gets emitted when the COM object issues the
    event \a name. \a argc is the number of parameters provided by the
    event (DISPPARAMS.cArgs), and \a argv is the pointer to the
    parameter values (DISPPARAMS.rgvarg). Note that the order of parameter
    values is turned around, ie. the last element of the array is the first
    parameter in the function.

    \code
    void Receiver::slot( const QString &name, int argc, void *argv )
    {
	VARIANTARG *params = (VARIANTARG*)argv;
	if ( name.startsWith( "BeforeNavigate2(" ) ) {
	    IDispatch *pDisp = params[argc-1].pdispVal;
	    VARIANTARG URL = *params[argc-2].pvarVal;
	    VARIANTARG Flags = *params[argc-3].pvarVal;
	    VARIANTARG TargetFrameName = *params[argc-4].pvarVal;
	    VARIANTARG PostData = *params[argc-5].pvarVal;
	    VARIANTARG Headers = *params[argc-6].pvarVal;
	    bool *Cancel = params[argc-7].pboolVal;
	}
    }
    \endcode

    Use this signal if the event has parameters of unsupported data
    types. Otherwise, connect directly to the signal \a name.
*/

/*!
    \fn void QAxBase::propertyChanged( const QString &name )

    If the COM object supports property notification, this signal gets
    emitted when the property called \a name is changed.
*/

/*!
    \fn void QAxBase::exception( int code, const QString &source, const QString &desc, const QString &help )

    This signal is emitted when the COM object throws an exception while called using the OLE automation
    interface IDispatch. \a code, \a source, \a desc and \a help provide information about the exception as
    provided by the COM server and can be used to provide useful feedback to the end user. \a help includes
    the help file, and the help context ID in brackets, e.g. "filename [id]".
*/

#endif //QAXPRIVATE_DECL

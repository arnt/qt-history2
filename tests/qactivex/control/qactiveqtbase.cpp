#define NOQT_ACTIVEX
#include "qactiveqtbase.h"
#include "qactiveqt.h"
#undef NOQT_ACTIVEX

#include <qapplication.h>
#include "../shared/types.h"

QPtrList<CComTypeInfoHolder> *QActiveQtBase::typeInfoHolderList = 0;

/*!
    \class QActiveQtFactoryInterface qactiveqt.h
    \brief The QActiveQtFactoryInterface class is an interface for the creation of ActiveX components.

    Implement this interface once in your ActiveX server to provide information about the components
    this server can create. The interface inherits the QFeatureListInterface and works key-based. A
    key in this interface is the class identifier of the ActiveX object in the textual format
    {01234567-89AB-CDEF-0123-456789ABCDEF}.

    To instantiate and export your implementation of the factory interface, use the Q_EXPORT_COMPONENT
    and Q_CREATE_INSTANCE macros:

    \code
    class MyActiveQtFactory : public QActiveQtFactoryInterface
    {
	...
    };

    Q_EXPORT_COMPONENT()
    {
	Q_CREATE_INSTANCE( MyActiveQtFactory )
    }
    \endcode

    The QActiveQtFactory class provide a convenient implementation of this interface.
*/

/*!
    \fn QWidget *QActiveQtFactoryInterface::create( const QString &key, QWidget *parent = 0, const char *name = 0 )

    Reimplement this function to return a new widget for \a key. Propagate \a parent and \a name to the
    QWidget constructor. Return 0 if this factory doesn't support the value of \a key.
*/

/*!
    \fn QMetaObject *QActiveQtFactoryInterface::metaObject( const QString &key ) const

    Reimplement this function to return the QMetaObject for \a key. Use the QObject::staticMetaObject() for that.
    The class implementing the ActiveX control has to use the Q_OBJECT macro to generate meta object information.
    Return 0 if this factory doesn't support the value of \a key.
*/

/*!
    \fn QUuid QActiveQtFactoryInterface::interfaceID( const QString &key ) const

    Reimplement this function to return the interface identifier for \a key, or an empty QUuid if
    this factory doesn't support the value of \a key.
*/

/*!
    \fn QUuid QActiveQtFactoryInterface::eventsID( const QString &key ) const

    Reimplement this function to return the identifier of the event interface for \a key, or an empty QUuid if
    this factory doesn't support the value of \a key.
*/

/*!
    \fn QUuid QActiveQtFactoryInterface::typeLibID() const

    Reimplement this function to return the type library identifier for this ActiveX server.
*/

/*!
    \fn QUuid QActiveQtFactoryInterface::appID() const

    Reimplement this function to return the application identifier for this ActiveX server.
*/


/*!
    \class QActiveQtFactory qactiveqt.h
    \brief The QActiveQtFactory class provides a default implementation of the QActiveQtFactoryInterface.

    Derive your ActiveX factory from this class rather than QActiveQtFactoryInterface for convenience, and
    use the Q_EXPORT_ACTIVEX macro to instantiate and export it:

    \code
    class MyActiveQtFactory : public QActiveQtFactory
    {
	...
    };

    Q_EXPORT_ACTIVEX( MyActiveQtFactory,		      // factory class
		    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
		    "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
		    )
    \endcode

    If your ActiveX server supports only a single ActiveX control, you can use a default factory implementation instead
    of implementing the factory on your own. To do that, #define QT_ACTIVEX_DEFAULT in the implementation file for the 
    control before #include'ing the header file declaring the class. The ActiveX control class has to inherit QActiveQt:

    In the header file:
    \code
    #include <qactiveqt.h>
    #include <qwidget.h>

    class TheActiveX : public QWidget, public QActiveQt
    {
        Q_OBJECT
    public:
	TheActiveX( QWidget *parent, const char *name );
	...
    };

    QT_ACTIVEX( TheActiveX, <clsid>, <iid>, <diid>, <libid>, <appid> )
    \endcode

    In the implementation file:
    \code
    #define QT_ACTIVEX_DEFAULT
    #include "theactivex.h"

    TheActiveX::TheActiveX( QWidget *parent, const char *name )
    : QWidget( parent, name )
    {
    ...
    }
    \endcode

    If you implement your own factory, #define QT_ACTIVEX_IMPL before including the declaration files of the ActiveX
    controls your factory support:

    \code
    #define QT_ACTIVEX_IMPL
    #include "activex1.h"
    #include "activex2.h"

    // Factory implementation
    \endcode

    You can then use the values passed to the QT_ACTIVEX macro in the factory implementation:

    \code
    QStringList ActiveQtFactory::featureList() const
    {
	QStringList list;
	list << QUuid( CLSID_ActiveX1 );
	list << QUuid( CLSID_ActiveX2 );
	...
	return list;
    }

    QWidget *ActiveQtFactory::create( const QString &key, QWidget *parent, const char *name )
    {
	if ( QUuid(key) == CLSID_ActiveX1 )
	    return new ActiveX1( parent, name );
	...
	return 0;
    }
    
    ...

    QUuid ActiveQtFactory::interfaceID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_ActiveX1 )
	    return IID_IActiveX1;
	...
	return QUuid();
    }

    QUuid ActiveQtFactory::eventsID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_ActiveX1 )
	    return IID_IActiveX1Events;
	...
	return QUuid();
    }
    \endcode

    The values of all IID_<class>Lib and IID_<class>App are supposed to be equal, so you can use
    any of them in the Q_EXPORT_ACTIVEX macro.
*/

/*!
    Constructs a QActiveQtFactory object that returns \a libid and \a appid
    in the implementation of the respective interface functions.
*/

QActiveQtFactory::QActiveQtFactory( const QUuid &libid, const QUuid &appid )
    : typelib( libid ), app( appid )
{
}

/*!
    \internal
*/
QRESULT QActiveQtFactory::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QActiveQtFactory )
	*iface = this;
    else
	return QE_NOINTERFACE;
    addRef();
    return QS_OK;
}

/*!
    \reimp
*/
QUuid QActiveQtFactory::typeLibID() const
{
    return typelib;
}

/*!
    \reimp
*/
QUuid QActiveQtFactory::appID() const
{
    return app;
}


/*!
    \class QActiveQt qactiveqt.h
    \brief The QActiveQt class provides an interface between the Qt widget and the ActiveX control.

    When implementing ActiveX controls with Qt, inherit your control class from both QWidget (directly or
    indirectly) and this class. The meta object compiler requires you to inherit first from QWidget.

    \code
    class MyActiveX : public QWidget, public QActiveQt
    {
	Q_OBJECT
    public:
	MyActiveX( QWidget *parent = 0, const char *name = 0 );
	...
    };
    \endcode

    If you use the IDC to generate an interface definition from your class declaration, use the QT_ACTIVEX 
    macro to declare the class and the identifiers of the interfaces implemented by your class:

    QT_ACTIVEX( MyActiveX,				    // class
		"{01234567-89AB-CDEF-0123-456789ABCDEF}",   // class ID
		"{01234567-89AB-CDEF-0123-456789ABCDEF}",   // interface ID
		"{01234567-89AB-CDEF-0123-456789ABCDEF}",   // event interface ID
		"{01234567-89AB-CDEF-0123-456789ABCDEF}",   // type library ID
		"{01234567-89AB-CDEF-0123-456789ABCDEF}"    // application ID
	      )
*/

/*!
    Constructs an empty QActiveQt object.
*/
QActiveQt::QActiveQt()
    :activex(0)
{
}

/*!
    Call this function to request permission the change the property \a property
    from the client site hosting this ActiveX control. This function returns TRUE
    when the client site allows the change, otherwise it returns FALSE.

    This function is usually called first in the write function for \a property, and
    the writing is cancelled when the function returns FALSE.

    \code
    void MyActiveQt::setText( const QString &text )
    {
	if ( !requestPropertyChange( "text" ) )
	    return;
	    
	// update property

	propertyChanged( "text" );
    }
    \endcode

    \sa propertyChanged()
*/
bool QActiveQt::requestPropertyChange( const char *property )
{
    if ( !activex )
	return TRUE;
    if ( !activex->proplist )
	activex->readMetaData();

    DISPID dispId = -1;
    QIntDictIterator <QMetaProperty> it( *activex->proplist );
    while ( it.current() && dispId < 0 ) {
	QMetaProperty *mp = it.current();
	if ( !qstrcmp( property, mp->name() ) )
	    dispId = it.currentKey();
	++it;
    }

    return activex->emitRequestPropertyChange( dispId );
}

/*!
    Call this function to notify the client site hosting this ActiveX control that
    the property \a property has been changed.

    This function is usually called last in the write function for \a property.

    \sa requestPropertyChange()
*/
void QActiveQt::propertyChanged( const char *property )
{
    if ( !activex )
	return;

    if ( !activex->proplist )
	activex->readMetaData();

    DISPID dispId = -1;
    QIntDictIterator <QMetaProperty> it( *activex->proplist );
    while ( it.current() && dispId < 0 ) {
	QMetaProperty *mp = it.current();
	if ( !qstrcmp( property, mp->name() ) )
	    dispId = it.currentKey();
	++it;
    }

    activex->emitPropertyChanged( dispId );
}

/*
    Helper class to enumerate all supported event interfaces.
*/
class QAxSignalVec : public IEnumConnectionPoints
{
public:
    QAxSignalVec( const QActiveQtBase::ConnectionPoints &points ) 
	: cpoints( points ), ref(0) 
    {
    }
    QAxSignalVec( const QAxSignalVec &old )
    {
	ref = 0;
	cpoints = old.cpoints;
	for ( QActiveQtBase::ConnectionPointsIterator i = cpoints.begin(); i != cpoints.end(); ++i )
	    (*i)->AddRef();
	it = old.it;
    }
    ~QAxSignalVec()
    {
	for ( QActiveQtBase::ConnectionPointsIterator i = cpoints.begin(); i != cpoints.end(); ++i )
	    (*i)->Release();
    }
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
    STDMETHOD(QueryInterface)(REFIID iid, void **iface)
    {
	*iface = 0;
	if ( iid == IID_IUnknown )
	    *iface = this;
	else if ( iid == IID_IEnumConnectionPoints )
	    *iface = this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }
    STDMETHOD(Next)( ULONG cConnections, IConnectionPoint **cpoint, ULONG *pcFetched )
    {
	unsigned long i;
	for ( i = 0; i < cConnections; i++ ) {
	    if ( it == cpoints.end() )
		break;
	    IConnectionPoint *cp = *it;
	    cp->AddRef();
	    cpoint[i] = cp;
	    ++it;
	}
	*pcFetched = i;
	return i == cConnections ? S_OK : S_FALSE;
    }
    STDMETHOD(Skip)( ULONG cConnections )
    {
	while ( cConnections ) {
	    ++it;
	    --cConnections;
	    if ( it == cpoints.end() )
		return S_FALSE;
	}
	return S_OK;
    }
    STDMETHOD(Reset)()
    {
	it = cpoints.begin();

	return S_OK;
    }
    STDMETHOD(Clone)( IEnumConnectionPoints **ppEnum )
    {
	*ppEnum = new QAxSignalVec( *this );
	(*ppEnum)->AddRef();

	return S_OK;
    }

    QActiveQtBase::ConnectionPoints cpoints;
    QActiveQtBase::ConnectionPointsIterator it;

private:
    unsigned long ref;
};

/*
    Helper class to store and enumerate all connected event listeners.
*/
class QAxConnection : public IConnectionPoint,
		      public IEnumConnections
{
public:
    typedef QValueList<CONNECTDATA> Connections;
    typedef QValueList<CONNECTDATA>::Iterator Iterator;

    QAxConnection( QActiveQtBase *parent, const QUuid &uuid )
	: that(parent), iid( uuid ), ref( 2 )
    {
    }
    QAxConnection( const QAxConnection &old )
    {
	ref = 0;
	connections = old.connections;
	it = old.it;
	that = old.that;
	iid = old.iid;
    }

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
    STDMETHOD(QueryInterface)(REFIID iid, void **iface)
    {
	*iface = 0;
	if ( iid == IID_IUnknown )
	    *iface = (IConnectionPoint*)this;
	else if ( iid == IID_IConnectionPoint )
	    *iface = this;
	else if ( iid == IID_IEnumConnections )
	    *iface = this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }
    STDMETHOD(GetConnectionInterface)(IID *pIID)
    {
	*pIID = iid;
	return S_OK;
    }
    STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer **ppCPC)
    {
	return that->QueryInterface(IID_IConnectionPointContainer, (void**)ppCPC );
    }
    STDMETHOD(Advise)(IUnknown*pUnk, DWORD *pdwCookie)
    {
	IUnknown *checkImpl = 0;
	pUnk->QueryInterface( iid, (void**)&checkImpl );
	if ( !checkImpl )
	    return CONNECT_E_CANNOTCONNECT;
	static DWORD cookie = 0;
	CONNECTDATA cd;
	cd.dwCookie = cookie++;
	cd.pUnk = pUnk;
	cd.pUnk->AddRef();
	connections.append(cd);

	*pdwCookie = 0;
	return S_OK;
    }
    STDMETHOD(Unadvise)(DWORD dwCookie)
    {
	for ( QValueList<CONNECTDATA>::Iterator i = connections.begin(); i != connections.end(); ++i ) {
	    CONNECTDATA cd = *i;
	    if ( cd.dwCookie == dwCookie ) {
		cd.pUnk->Release();
		connections.remove(i);
		return S_OK;
	    }
	}
	return CONNECT_E_NOCONNECTION;
    }
    STDMETHOD(EnumConnections)(IEnumConnections **ppEnum)
    {
	*ppEnum = this;
	AddRef();

	return S_OK;
    }
    STDMETHOD(Next)( ULONG cConnections, CONNECTDATA *cd, ULONG *pcFetched )
    {
	unsigned long i;
	for ( i = 0; i < cConnections; i++ ) {
	    if ( it == connections.end() )
		break;
	    cd[i] = *it;
	    cd[i].pUnk->AddRef();
	    ++it;
	}
	*pcFetched = i;
	return i == cConnections ? S_OK : S_FALSE;
    }
    STDMETHOD(Skip)( ULONG cConnections )
    {
	while ( cConnections ) {
	    ++it;
	    --cConnections;
	    if ( it == connections.end() )
		return S_FALSE;
	}
	return S_OK;
    }
    STDMETHOD(Reset)()
    {
	it = connections.begin();

	return S_OK;
    }
    STDMETHOD(Clone)( IEnumConnections **ppEnum )
    {
	*ppEnum = new QAxConnection( *this );
	(*ppEnum)->AddRef();

	return S_OK;
    }

private:
    QActiveQtBase *that;
    QUuid iid;
    Connections connections;
    Iterator it;

    unsigned long ref;
};


QActiveQtBase::QActiveQtBase( IID QAxClass )
: initNewCalled(FALSE), dirtyflag( FALSE ), activeqt( 0 ), ref( 0 ),
  IID_QAxClass( QAxClass ), slotlist(0), signallist(0),proplist(0), proplist2(0)
{
    _Module.Lock();
    if ( !typeInfoHolderList ) {
	typeInfoHolderList = new QPtrList<CComTypeInfoHolder>;
	typeInfoHolderList->setAutoDelete( TRUE );
    }

    QString clsid = QUuid(QAxClass).toString();
    _tih = new CComTypeInfoHolder();
    _tih->m_pguid = new GUID( _Module.factory()->interfaceID(clsid) );
    _tih->m_plibid = new GUID( _Module.factory()->typeLibID() );
    _tih->m_wMajor = 1;
    _tih->m_wMinor = 0;
    _tih->m_dwRef = 0;
    _tih->m_pInfo = NULL;
    _tih->m_pMap = NULL;
    _tih->m_nCount = 0;
    typeInfoHolderList->append( _tih );

    _tih2 = new CComTypeInfoHolder();
    _tih2->m_pguid = new GUID( QAxClass );
    _tih2->m_plibid = new GUID( _Module.factory()->typeLibID() );
    _tih2->m_wMajor = 1;
    _tih2->m_wMinor = 0;
    _tih2->m_dwRef = 0;
    _tih2->m_pInfo = NULL;
    _tih2->m_pMap = NULL;
    _tih2->m_nCount = 0;
    typeInfoHolderList->append( _tih2 );

    points[IID_IPropertyNotifySink] = new QAxConnection( this, IID_IPropertyNotifySink );
    points[_Module.factory()->eventsID(clsid)] = new QAxConnection( this, _Module.factory()->eventsID(clsid) );
}

QActiveQtBase::~QActiveQtBase()
{
    for ( QActiveQtBase::ConnectionPointsIterator it = points.begin(); it != points.end(); ++it )
	(*it)->Release();
    if ( activeqt ) {
	activeqt->disconnect( this );
	delete activeqt;
    }

    _Module.Unlock();
    delete slotlist;
    delete signallist;
    delete proplist;
    delete proplist2;
}

class HackWidget : public QWidget
{
    friend class QActiveQtBase;
};

LRESULT QActiveQtBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    activeqt = _Module.factory()->create( QUuid(IID_QAxClass) );
    Q_ASSERT(activeqt);
    if ( !activeqt )
	return 0;
    QActiveQt *aqt = (QActiveQt*)activeqt->qt_cast( "QActiveQt" );
    if ( aqt )
	aqt->activex = this;
    ((HackWidget*)activeqt)->clearWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu );
    ((HackWidget*)activeqt)->topData()->ftop = 0;
    ((HackWidget*)activeqt)->topData()->fright = 0;
    ((HackWidget*)activeqt)->topData()->fleft = 0;
    ((HackWidget*)activeqt)->topData()->fbottom = 0;
    ::SetWindowLong( activeqt->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

    // connect the generic slot to all signals of activeqt
    const QMetaObject *mo = activeqt->metaObject();
    for ( int isignal = mo->numSignals( TRUE )-1; isignal >= 0; --isignal )
	connectInternal( activeqt, isignal, this, 2, isignal );
    
    activeqt->installEventFilter( this );
    return 0;
}

LRESULT QActiveQtBase::ForwardMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if ( activeqt ) {
	switch( uMsg ) {
	case WM_SIZE:
	    activeqt->resize( LOWORD(lParam), HIWORD(lParam) );
	    break;
	case WM_PAINT:
	    activeqt->update();
	    return 0;
	case WM_SETFOCUS:
	    ::SendMessage( activeqt->winId(), WM_ACTIVATE, MAKEWPARAM( WA_ACTIVE, 0 ), 0 );
	    break;
	case WM_SHOWWINDOW:
	    ::SetParent( activeqt->winId(), m_hWnd );
	    activeqt->raise();
	    activeqt->move( 0, 0 );
	    if( wParam )
		activeqt->show();
	    else
		activeqt->hide();
	    return 0;
	default:
	    break;
	}
	return ::SendMessage( activeqt->winId(), uMsg, wParam, lParam );
    }
    return 0;
}

void QActiveQtBase::readMetaData()
{
    if ( !slotlist ) {
	slotlist = new QIntDict<QMetaData>;
	signallist = new QMap<int,DISPID>;
	proplist = new QIntDict<QMetaProperty>;
	proplist2 = new QMap<int,DISPID>;

	QMetaObject *mo = activeqt->metaObject();
	for ( int islot = mo->numSlots( TRUE )-1; islot >=0 ; --islot ) {
	    const QMetaData *slot = mo->slot( islot, TRUE );

	    BSTR bstrNames = QStringToBSTR( slot->method->name );
	    UINT cNames = 1;
	    DISPID dispId;
	    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, cNames, LOCALE_USER_DEFAULT, &dispId );
	    if ( dispId >= 0 ) {
		for ( int p = 0; p < (int)cNames; ++ p ) {
		    slotlist->insert( dispId, slot );
		    SysFreeString( bstrNames );
		}
	    }
	}
	CComPtr<IConnectionPointContainer> cpoints;
	QueryInterface( IID_IConnectionPointContainer, (void**)&cpoints );
	if ( cpoints ) {
	    CComPtr<IProvideClassInfo> classinfo;
	    cpoints->QueryInterface( IID_IProvideClassInfo, (void**)&classinfo );
	    if ( classinfo ) {
		CComPtr<ITypeInfo> info;
		CComPtr<ITypeInfo> eventinfo;
		classinfo->GetClassInfo( &info );
		if ( info ) {
		    TYPEATTR *typeattr;
		    info->GetTypeAttr( &typeattr );
		    if ( typeattr ) {
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
				if ( eventattr && eventattr->guid == _Module.factory()->eventsID(QUuid(IID_QAxClass)) ) {
				    eventinfo = eventtype;
				    eventtype->ReleaseTypeAttr( eventattr );
				    break;
				}
				eventtype->ReleaseTypeAttr( eventattr );
			    }
			}
			info->ReleaseTypeAttr( typeattr );
		    }		
		}
		if ( eventinfo ) {
		    for ( int isignal = mo->numSignals( TRUE )-1; isignal >= 0; --isignal ) {
			const QMetaData *signal = mo->signal( isignal, TRUE );

			BSTR bstrNames = QStringToBSTR( signal->method->name );
			UINT cNames = 1;
			DISPID dispId;
			eventinfo->GetIDsOfNames( (BSTR*)&bstrNames, cNames, &dispId );
			if ( dispId >= 0 ) {
			    signallist->insert( isignal, dispId );
			    for ( int p = 0; p < (int)cNames; ++ p )
				SysFreeString( bstrNames );
			} else {
			    signallist->insert( isignal, -1 );
			}
		    }
		}
	    }
	}
	for ( int iproperty = mo->numProperties( TRUE )-1; iproperty >= 0; --iproperty ) {
	    const QMetaProperty *property = mo->property( iproperty, TRUE );

	    BSTR bstrNames = QStringToBSTR( property->name() );
	    UINT cNames = 1;
	    DISPID dispId;
	    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, cNames, LOCALE_USER_DEFAULT, &dispId );
	    if ( dispId >= 0 ) {
		for ( int p = 0; p < (int)cNames; ++p ) {
		    proplist->insert( dispId, property );
		    proplist2->insert( iproperty, dispId );
		    SysFreeString( bstrNames );
		}
	    }
	}
    }
}

bool QActiveQtBase::qt_emit( int isignal, QUObject* _o )
{
    if ( m_nFreezeEvents )
	return TRUE;

    if ( !signallist )
	readMetaData();

    // get the signal information.
    const QMetaData *signal = activeqt->metaObject()->signal( isignal, TRUE );
    if ( !signal )
	return FALSE;
    const int signalcount = signal->method->count;

    // Get the Dispatch ID of the method to be called
    DISPID eventId = signallist->operator [](isignal);
    if ( eventId == -1 )
	return FALSE;

    CComPtr<IConnectionPoint> cpoint;
    FindConnectionPoint( _Module.factory()->eventsID(QUuid(IID_QAxClass)), &cpoint );
    if ( cpoint ) {
	CComPtr<IEnumConnections> clist;
	cpoint->EnumConnections( &clist );
	if ( clist ) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next( cc, (CONNECTDATA*)&c, &cc );
	    if ( cc ) {
		// setup parameters
		unsigned int argErr = 0;
		VARIANT arg;
		DISPPARAMS dispParams;
		dispParams.cArgs = signalcount;
		dispParams.cNamedArgs = 0;
		dispParams.rgdispidNamedArgs = 0;
		dispParams.rgvarg = signalcount ? new VARIANTARG[signalcount] : 0;
		int p;
		for ( p = 0; p < signalcount; ++p ) {
		    QUObject *obj = _o + p + 1;
		    // map the QUObject's type to the VARIANT
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
			const QUParameter *param = signal->method->parameters + p;
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
		    dispParams.rgvarg[ signalcount - p - 1 ] = arg;
		}
		// call listeners
		GUID IID_QAxEvents = _Module.factory()->eventsID(QUuid(IID_QAxClass));
		while ( cc ) {
		    if ( c->pUnk ) {
			CComPtr<IDispatch> disp;
			c->pUnk->QueryInterface( IID_QAxEvents, (void**)&disp );
			if ( disp ) {
			    disp->Invoke( eventId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispParams, 0, 0, &argErr );
			}
			c->pUnk->Release();
		    }
		    clist->Next( cc, (CONNECTDATA*)&c, &cc );
		}

		// clean up
		for ( p = 0; p < signalcount; ++p ) {
		    if ( dispParams.rgvarg[p].vt == VT_BSTR )
			SysFreeString( dispParams.rgvarg[p].bstrVal );
		}
		delete [] dispParams.rgvarg;
	    }	    
	}
    }
    return TRUE;
}

bool QActiveQtBase::emitRequestPropertyChange( DISPID dispId )
{
    CComPtr<IConnectionPoint> cpoint;
    FindConnectionPoint( IID_IPropertyNotifySink, &cpoint );
    if ( cpoint ) {
	CComPtr<IEnumConnections> clist;
	cpoint->EnumConnections( &clist );
	if ( clist ) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next( cc, (CONNECTDATA*)&c, &cc );
	    if ( cc ) {
		while ( cc ) {
		    if ( c->pUnk ) {
			CComPtr<IPropertyNotifySink> sink;
			c->pUnk->QueryInterface( IID_IPropertyNotifySink, (void**)&sink );
			if ( sink && sink->OnRequestEdit( dispId ) == S_FALSE ) {
			    c->pUnk->Release();
			    return FALSE;
			}
			c->pUnk->Release();
		    }
		    clist->Next( cc, (CONNECTDATA*)&c, &cc );
		}
	    }
	}
    }
    return TRUE;
}

void QActiveQtBase::emitPropertyChanged( DISPID dispId )
{
    CComPtr<IConnectionPoint> cpoint;
    FindConnectionPoint( IID_IPropertyNotifySink, &cpoint );
    if ( cpoint ) {
	CComPtr<IEnumConnections> clist;
	cpoint->EnumConnections( &clist );
	if ( clist ) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next( cc, (CONNECTDATA*)&c, &cc );
	    if ( cc ) {
		while ( cc ) {
		    if ( c->pUnk ) {
			CComPtr<IPropertyNotifySink> sink;
			c->pUnk->QueryInterface( IID_IPropertyNotifySink, (void**)&sink );
			if ( sink )
			    sink->OnChanged( dispId );
			c->pUnk->Release();
		    }
		    clist->Next( cc, (CONNECTDATA*)&c, &cc );
		}
	    }
	}
    }
}

HRESULT QActiveQtBase::Invoke( DISPID dispidMember, REFIID riid,
		  LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pvarResult,
		  EXCEPINFO* pexcepinfo, UINT* puArgErr )
{
    if ( riid != IID_NULL )
	return DISP_E_UNKNOWNINTERFACE;

    HRESULT res = DISP_E_MEMBERNOTFOUND;

    if ( !slotlist )
	readMetaData();

    switch ( wFlags ) {
    case DISPATCH_METHOD:
	{
	    const QMetaData *slot = slotlist->find( dispidMember );
	    if ( !slot )
		break;
	    int index = activeqt->metaObject()->findSlot( slot->name, TRUE );
	    if ( index == -1 )
		break;
	    // verify parameter count
	    int pcount = slot->method->count;
	    int argcount = pDispParams->cArgs;
	    if ( pcount > argcount )
		return DISP_E_PARAMNOTOPTIONAL;
	    else if ( pcount < argcount )
		return DISP_E_BADPARAMCOUNT;

	    // setup parameters
	    const QUParameter *params = slot->method->parameters;
	    QUObject *objects = pcount ? new QUObject[pcount+1] : 0;
	    for ( int p = 0; p < pcount; ++p ) // map the VARIANT to the QUObject
		VARIANTToQUObject( pDispParams->rgvarg[ pcount-p-1 ], objects + p + 1 );

	    // call the slot
	    activeqt->qt_invoke( index, objects );

	    // ### update reference parameters and value
	    delete [] objects;
	    res = S_OK;
	}
	break;
    case DISPATCH_PROPERTYPUT:
	{
	    const QMetaProperty *property = proplist->find( dispidMember );
	    if ( !property )
		break;
	    if ( !property->writable() )
		return DISP_E_MEMBERNOTFOUND;
	    if ( !pDispParams->cArgs )
		return DISP_E_PARAMNOTOPTIONAL;
	    if ( pDispParams->cArgs != 1 || 
		 pDispParams->cNamedArgs != 1 ||
		 *pDispParams->rgdispidNamedArgs != DISPID_PROPERTYPUT )
		return DISP_E_BADPARAMCOUNT;

	    emitRequestPropertyChange( dispidMember );

	    QVariant var = VARIANTToQVariant( *pDispParams->rgvarg );
	    if ( !var.isValid() ) {
		if ( puArgErr )
		    *puArgErr = 0;
		return DISP_E_BADVARTYPE;
	    }
	    if ( !activeqt->setProperty( property->name(), var ) ) {
		if ( puArgErr )
		    *puArgErr = 0;
		return DISP_E_TYPEMISMATCH;
	    }

	    emitPropertyChanged( dispidMember );

	    res = S_OK;
	}
	break;
    case DISPATCH_PROPERTYGET:
	{
	    const QMetaProperty *property = proplist->find( dispidMember );
	    if ( !property )
		break;
	    if ( !pvarResult )
		return DISP_E_PARAMNOTOPTIONAL;
	    if ( pDispParams->cArgs || 
		 pDispParams->cNamedArgs )
		return DISP_E_BADPARAMCOUNT;

	    QVariant var = activeqt->property( property->name() );
	    if ( !var.isValid() )
		return DISP_E_MEMBERNOTFOUND;

	    *pvarResult = QVariantToVARIANT( var );
	    res = S_OK;
	}
	break;
    default:
	break;
    }

    if ( res == S_OK )
	return res;
    return E_FAIL;
}

HRESULT QActiveQtBase::EnumConnectionPoints( IEnumConnectionPoints **epoints )
{
    if ( !epoints )
	return E_POINTER;
    *epoints = new QAxSignalVec( points );
    (*epoints)->AddRef();
    return S_OK;
}

HRESULT QActiveQtBase::FindConnectionPoint( REFIID iid, IConnectionPoint **cpoint )
{
    if ( !cpoint )
	return E_POINTER;

    IConnectionPoint *cp = points[iid];
    *cpoint = cp;
    if ( cp ) {
	cp->AddRef();
	return S_OK;
    }
    return CONNECT_E_NOCONNECTION;
}

HRESULT QActiveQtBase::InitNew()
{
    if ( initNewCalled )
	return CO_E_ALREADYINITIALIZED;

    dirtyflag = FALSE;
    initNewCalled = TRUE;
    const QMetaObject *mo = activeqt->metaObject();
    for ( int prop = 0; prop < mo->numProperties( TRUE ); ++prop ) {
	// set property to default value...
    }
    return S_OK;
}

HRESULT QActiveQtBase::Load( IPropertyBag *bag, IErrorLog *log )
{
    if ( initNewCalled )
	return E_UNEXPECTED;
    if ( !bag )
	return E_POINTER;

    if ( !proplist2 )
	readMetaData();

    dirtyflag = FALSE;
    bool error = FALSE;
    const QMetaObject *mo = activeqt->metaObject();
    for ( int prop = 0; prop < mo->numProperties( TRUE ); ++prop ) {
	if ( !proplist2->contains( prop ) )
	    continue;
	const char* property = mo->property( prop, TRUE )->name();
	BSTR bstr = QStringToBSTR( property );
	VARIANT var;
	bag->Read( bstr, &var, 0 );
	if ( !activeqt->setProperty( property, VARIANTToQVariant( var ) ) )
	    error = TRUE;
	SysFreeString(bstr);
    }

    return error ? E_FAIL : S_OK;
}

HRESULT QActiveQtBase::Save( IPropertyBag *bag, BOOL clearDirty, BOOL saveAll)
{
    if ( !bag )
	return E_POINTER;

    if ( !proplist2 )
	readMetaData();

    dirtyflag = FALSE;
    bool error = FALSE;
    const QMetaObject *mo = activeqt->metaObject();
    for ( int prop = 0; prop < mo->numProperties( TRUE ); ++prop ) {
	if ( !proplist2->contains( prop ) )
	    continue;
	const char* property = mo->property( prop, TRUE )->name();
	BSTR bstr = QStringToBSTR( property );
	QVariant qvar;
	if ( !activeqt->qt_property( prop, 1, &qvar ) )
	    error = TRUE;
	VARIANT var = QVariantToVARIANT( qvar );
	bag->Write( bstr, &var );
	SysFreeString(bstr);
    }
    return error ? E_FAIL : S_OK;
}

HRESULT QActiveQtBase::IsDirty()
{
    return dirtyflag ? S_OK : S_FALSE;
}

HRESULT QActiveQtBase::OnAmbientPropertyChange( DISPID dispID )
{
    if ( !m_spClientSite )
	return S_OK;

    CComPtr<IDispatch> disp;
    m_spClientSite->QueryInterface( IID_IDispatch, (void**)&disp );
    if ( !disp )
	return S_OK;

    VARIANT var;
    DISPPARAMS params = { 0, 0, 0, 0 };
    disp->Invoke( dispID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &var, 0, 0 );

    switch( dispID ) {
    case DISPID_AMBIENT_APPEARANCE:
	break;
    case DISPID_AMBIENT_AUTOCLIP:
	break;
    case DISPID_AMBIENT_BACKCOLOR:
    case DISPID_AMBIENT_FORECOLOR:
	{
	    long rgb;
	    if ( var.vt == VT_UI4 )
		rgb = var.ulVal;
	    else if ( var.vt == VT_I4 )
		rgb = var.lVal;
	    else
		break;
	    QPalette pal = activeqt->palette();
	    // OLE_COLOR is BGR
	    pal.setColor( dispID == DISPID_AMBIENT_BACKCOLOR ? QColorGroup::Background : QColorGroup::Foreground, 
		QColor( qBlue(var.lVal), qGreen(var.lVal), qRed(var.lVal) ) );
	    activeqt->setPalette( pal );
	}
	break;
    case DISPID_AMBIENT_DISPLAYASDEFAULT:
	break;
    case DISPID_AMBIENT_DISPLAYNAME:
	if ( var.vt != VT_BSTR )
	    break;
	activeqt->setCaption( BSTRToQString( var.bstrVal ) );
	break;
    case DISPID_AMBIENT_FONT:
	if ( var.vt != VT_DISPATCH )
	    break;
	{
	    IDispatch *d = var.pdispVal;
	    CComPtr<IFont> f;
	    d->QueryInterface( IID_IFont, (void**)&f );
	    if ( f ) {
		BSTR name;
		BOOL bold;
		SHORT charset;
		BOOL italic;
		CY size;
		BOOL strike;
		BOOL underline;
		SHORT weight;
		f->get_Name( &name );
		f->get_Bold( &bold );
		f->get_Charset( &charset );
		f->get_Italic( &italic );
		f->get_Size( &size );
		f->get_Strikethrough( &strike );
		f->get_Underline( &underline );
		f->get_Weight( &weight );
		QFont font( BSTRToQString(name), size.Lo/10000, weight, italic );
		font.setBold( bold );
		font.setStrikeOut( strike );
		font.setUnderline( underline );
		activeqt->setFont( font );
		SysFreeString(name);
	    }
	}
	break;
    case DISPID_AMBIENT_LOCALEID:
	break;
    case DISPID_AMBIENT_MESSAGEREFLECT:
	if ( var.vt != VT_BOOL )
	    break;
	if ( var.boolVal )
	    activeqt->installEventFilter( this );
	else
	    activeqt->removeEventFilter( this );
	break;
    case DISPID_AMBIENT_PALETTE:
	break;
    case DISPID_AMBIENT_SCALEUNITS:
	break;
    case DISPID_AMBIENT_SHOWGRABHANDLES:
	break;
    case DISPID_AMBIENT_SHOWHATCHING:
	break;
    case DISPID_AMBIENT_SUPPORTSMNEMONICS:
	break;
    case DISPID_AMBIENT_TEXTALIGN:
	break;
    case DISPID_AMBIENT_UIDEAD:
	if ( var.vt != VT_BOOL )
	    break;
	activeqt->setEnabled( !var.boolVal );
	break;
    case DISPID_AMBIENT_USERMODE:
	break;
    case DISPID_AMBIENT_RIGHTTOLEFT:
	if ( var.vt != VT_BOOL )
	    break;
	qApp->setReverseLayout( var.boolVal );
	break;
    }

    return S_OK;
}

HRESULT QActiveQtBase::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    if ( !pszUserType )
	return E_POINTER;

    const QMetaObject *mo = _Module.factory()->metaObject( QUuid(IID_QAxClass) );

    switch ( dwFormOfType ) {
    case USERCLASSTYPE_FULL:
	*pszUserType = QStringToBSTR( mo->className() );
	break;
    case USERCLASSTYPE_SHORT:
	if ( !activeqt || activeqt->caption().isEmpty() )
	    *pszUserType = QStringToBSTR( mo->className() );
	else
	    *pszUserType = QStringToBSTR( activeqt->caption() );
	break;
    case USERCLASSTYPE_APPNAME:
	*pszUserType = QStringToBSTR( qApp->name() );
	break;
    }

    return S_OK;
}

HRESULT QActiveQtBase::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::GetMiscStatus\n"));
    return OleRegGetMiscStatus( IID_QAxClass, dwAspect, pdwStatus);
}

HRESULT QActiveQtBase::SetExtent( DWORD dwDrawAspect, SIZEL *psizel )
{
    SIZEL sizel = *psizel;
    if ( activeqt ) {
	SIZE sz;
	AtlHiMetricToPixel( &sizel, &sz );
	QSize min = activeqt->minimumSizeHint();
	sz.cx = QMAX( min.width(), sz.cx );
	sz.cy = QMAX( min.height(), sz.cy );
	AtlPixelToHiMetric( &sz, &sizel );
    }
    return IOleObjectImpl<QActiveQtBase>::SetExtent( dwDrawAspect, &sizel );
}

bool QActiveQtBase::eventFilter( QObject *o, QEvent *e )
{
    if ( !activeqt )
	return QObject::eventFilter( o, e );

    switch( e->type() ) {
    case QEvent::ChildInserted:
	{
	    QChildEvent *ce = (QChildEvent*)e;
	    ce->child()->installEventFilter( this );
	}
	break;
    case QEvent::ChildRemoved:
	{
	    QChildEvent *ce = (QChildEvent*)e;
	    ce->child()->removeEventFilter( this );
	}
	break;
    case QEvent::MouseButtonPress:
	{
	    if ( activeqt->focusWidget() == qApp->focusWidget() )
		break;
	}
	// FALL THROUGH
    case QEvent::FocusIn:    
	{
	    CComPtr<IOleClientSite> clientsite;
	    GetClientSite( &clientsite );
	    if ( clientsite ) {
		CComPtr<IOleControlSite> controlsite;
		clientsite->QueryInterface( IID_IOleControlSite, (void**)&controlsite );
		if ( controlsite )
		    controlsite->OnFocus( TRUE );
	    }
	}
	break;
    default:
	break;
    }    

    return QObject::eventFilter( o, e );
}

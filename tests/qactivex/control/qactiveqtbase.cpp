#define NOQT_ACTIVEX
#include "qactiveqtbase.h"
#include "qactiveqt.h"
#undef NOQT_ACTIVEX

#include <qsettings.h>
#include <private/qwidgetinterface_p.h>
#include <qintdict.h>
#include "../shared/types.h"

static QIntDict<QMetaData>* slotlist = 0;
static QMap<int,DISPID>* signallist = 0;
static QIntDict<QMetaProperty>* proplist = 0;
static QMap<int, DISPID>* proplist2 = 0;

extern "C" QUnknownInterface *ucm_instantiate();
extern QActiveQt *qt_activeqt( QWidget *parent, const char *name, Qt::WFlags f );
extern QMetaObject *qt_activeqt_meta();


/*!
    \class QActiveQt qactiveqt.h
    \brief The QActiveQt class is the base class of all ActiveX controls written with Qt.

    \extension QActiveX
*/

/*!
    Constructs an empty QActiveQt object with parent \a parent and the name \a name,
    using the widget flags \ f.
    \a parent, \a name and \a f are propagated to the QWidget constructor.
*/
QActiveQt::QActiveQt( QWidget *parent, const char *name, WFlags f )
    :QWidget( parent, name, f ), activex(0)
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
    if ( !proplist )
	activex->readMetaData();

    DISPID dispId = -1;
    QIntDictIterator <QMetaProperty> it( *proplist );
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

    if ( !proplist )
	activex->readMetaData();

    DISPID dispId = -1;
    QIntDictIterator <QMetaProperty> it( *proplist );
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
	: that(parent), iid( uuid )
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
	connections.append(cd);

	*pdwCookie = 0;
	return S_OK;
    }	
    STDMETHOD(Unadvise)(DWORD dwCookie)
    {
	for ( QValueList<CONNECTDATA>::Iterator i = connections.begin(); i != connections.end(); ++i ) {
	    CONNECTDATA cd = *i;
	    if ( cd.dwCookie == dwCookie ) {
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


QActiveQtBase::QActiveQtBase()
: initNewCalled(FALSE), dirtyflag( FALSE ), activeqt( 0 )
{
    points[IID_IPropertyNotifySink] = new QAxConnection( this, IID_IPropertyNotifySink );
    points[IID_QAxEvents] = new QAxConnection( this, IID_QAxEvents );
}

QActiveQtBase::~QActiveQtBase()
{
    for ( QActiveQtBase::ConnectionPointsIterator it = points.begin(); it != points.end(); ++it )
	(*it)->Release();
    if ( activeqt ) {
	activeqt->disconnect( this );
	delete activeqt;
    }
}

LRESULT QActiveQtBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    activeqt = qt_activeqt( 0, 0, Qt::WStyle_Customize );
    activeqt->activex = this;
    Q_ASSERT(activeqt);
    ::SetWindowLong( activeqt->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

    // connect the generic slot to all signals of activeqt
    QMetaObject *mo = activeqt->metaObject();
    for ( int isignal = mo->numSignals( TRUE )-1; isignal >= 0; --isignal )
	connectInternal( activeqt, isignal, this, 2, isignal );
    
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

HRESULT QActiveQtBase::UpdateRegistry(BOOL bRegister)
{
    char filename[MAX_PATH];
    GetModuleFileNameA( 0, filename, MAX_PATH-1 );
    QString file = QString::fromLocal8Bit(filename );
    QString path = file.left( file.findRev( "\\" )+1 );
    QString module = file.right( file.length() - path.length() );
    module = module.left( module.findRev( "." ) );
    
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    const QString appID = QUuid( IID_QAxApp ).isNull() ? QString::null : QUuid( IID_QAxApp ).toString().upper();
    const QString classID = QUuid( IID_QAxClass ).toString().upper();
    const QString eventID = QUuid( IID_QAxEvents ).toString().upper();
    const QString libID = QUuid( IID_QAxTypeLib ).toString().upper();
    const QString ifaceID = QUuid( IID_QAxInterface ).toString().upper();
    
    QMetaObject *mo = qt_activeqt_meta();
    const QString className = mo->className();
    
    if ( bRegister ) {
	if ( !appID.isNull() ) {
	    settings.writeEntry( "/AppID/" + appID + "/.", module );
	    settings.writeEntry( "/AppID/" + module + ".EXE/AppID", appID );
	}
	
	settings.writeEntry( "/" + module + "." + className + ".1/.", className + " Class" );
	settings.writeEntry( "/" + module + "." + className + ".1/CLSID/.", classID );
	settings.writeEntry( "/" + module + "." + className + ".1/Insertable/.", QString::null );
	
	settings.writeEntry( "/" + module + "." + className + "/.", className + " Class" );
	settings.writeEntry( "/" + module + "." + className + "/CLSID/.", classID );
	settings.writeEntry( "/" + module + "." + className + "/CurVer/.", module + "." + className + ".1" );
	
	settings.writeEntry( "/CLSID/" + classID + "/.", className + " Class" );
	if ( !appID.isNull() )
	    settings.writeEntry( "/CLSID/" + classID + "/AppID", appID );
	settings.writeEntry( "/CLSID/" + classID + "/Control/.", QString::null );
	settings.writeEntry( "/CLSID/" + classID + "/Insertable/.", QString::null );
	settings.writeEntry( "/CLSID/" + classID + "/LocalServer32/.", file + " -activex" );
	settings.writeEntry( "/CLSID/" + classID + "/MiscStatus/.", "0" );
	settings.writeEntry( "/CLSID/" + classID + "/MiscStatus/1/.", "131473" );
	settings.writeEntry( "/CLSID/" + classID + "/Programmable/.", QString::null );
	settings.writeEntry( "/CLSID/" + classID + "/ToolboxBitmap32/.", file + ", 101" );
	settings.writeEntry( "/CLSID/" + classID + "/TypeLib/.", libID );
	settings.writeEntry( "/CLSID/" + classID + "/Version/.", "1.0" );
	settings.writeEntry( "/CLSID/" + classID + "/VersionIndependentProgID/.", module + "." + className );
	settings.writeEntry( "/CLSID/" + classID + "/ProgID/.", module + "." + className + ".1" );
	settings.writeEntry( "/CLSID/" + classID + "/Implemented Categories/.", QString::null );
	//### TODO: write some list of categories
	
	settings.writeEntry( "/Interface/" + ifaceID + "/.", "I" + className );
	settings.writeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid/.", "{00020424-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid32/.", "{00020424-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + ifaceID + "/TypeLib/.", libID );
	settings.writeEntry( "/Interface/" + ifaceID + "/TypeLib/Version", "1.0" );
	
	settings.writeEntry( "/Interface/" + eventID + "/.", "_I" + className + "Events" );
	settings.writeEntry( "/Interface/" + eventID + "/ProxyStubClsid/.", "{00020420-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + eventID + "/ProxyStubClsid32/.", "{00020420-0000-0000-C000-000000000046}" );
	settings.writeEntry( "/Interface/" + eventID + "/TypeLib/.", libID );
	settings.writeEntry( "/Interface/" + eventID + "/TypeLib/Version", "1.0" );
	
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/0/win32/.", file );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/FLAGS/.", "0" );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/HELPDIR/.", path );
	settings.writeEntry( "/TypeLib/" + libID + "/1.0/.", module + " 1.0 Type Library" );
    } else {
	settings.removeEntry( "/AppID/" + module + ".EXE/AppID" );
	settings.removeEntry( "/AppID/" + appID + "/." );
	
	settings.removeEntry( "/" + module + "." + className + ".1/CLSID/." );
	settings.removeEntry( "/" + module + "." + className + ".1/Insertable/." );
	settings.removeEntry( "/" + module + "." + className + ".1/." );
	
	settings.removeEntry( "/" + module + "." + className + "/CLSID/." );
	settings.removeEntry( "/" + module + "." + className + "/CurVer/." );
	settings.removeEntry( "/" + module + "." + className + "/." );
	
	settings.removeEntry( "/CLSID/" + classID + "/AppID" );
	settings.removeEntry( "/CLSID/" + classID + "/Control/." );
	settings.removeEntry( "/CLSID/" + classID + "/Insertable/." );
	settings.removeEntry( "/CLSID/" + classID + "/LocalServer32/." );
	settings.removeEntry( "/CLSID/" + classID + "/MiscStatus/1/." );
	settings.removeEntry( "/CLSID/" + classID + "/MiscStatus/." );	    
	settings.removeEntry( "/CLSID/" + classID + "/Programmable/." );
	settings.removeEntry( "/CLSID/" + classID + "/ToolboxBitmap32/." );
	settings.removeEntry( "/CLSID/" + classID + "/TypeLib/." );
	settings.removeEntry( "/CLSID/" + classID + "/Version/." );
	settings.removeEntry( "/CLSID/" + classID + "/VersionIndependentProgID/." );
	settings.removeEntry( "/CLSID/" + classID + "/ProgID/." );
	//### TODO: remove some list of categories
	settings.removeEntry( "/CLSID/" + classID + "/Implemented Categories/." );
	settings.removeEntry( "/CLSID/" + classID + "/." );
	
	settings.removeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid/." );
	settings.removeEntry( "/Interface/" + ifaceID + "/ProxyStubClsid32/." );
	settings.removeEntry( "/Interface/" + ifaceID + "/TypeLib/Version" );
	settings.removeEntry( "/Interface/" + ifaceID + "/TypeLib/." );
	settings.removeEntry( "/Interface/" + ifaceID + "/." );
	
	settings.removeEntry( "/Interface/" + eventID + "/ProxyStubClsid/." );
	settings.removeEntry( "/Interface/" + eventID + "/ProxyStubClsid32/." );
	settings.removeEntry( "/Interface/" + eventID + "/TypeLib/Version" );
	settings.removeEntry( "/Interface/" + eventID + "/TypeLib/." );
	settings.removeEntry( "/Interface/" + eventID + "/." );
	
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/0/win32/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/0/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/FLAGS/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/HELPDIR/." );
	settings.removeEntry( "/TypeLib/" + libID + "/1.0/." );
    }
    
    return S_OK;
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
	    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, cNames, LOCALE_SYSTEM_DEFAULT, &dispId );
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
				if ( eventattr && eventattr->guid == IID_QAxEvents ) {
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
			    for ( int p = 0; p < (int)cNames; ++ p ) {
				signallist->insert( isignal, dispId );
				SysFreeString( bstrNames );
			    }
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
	    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, cNames, LOCALE_SYSTEM_DEFAULT, &dispId );
	    if ( dispId >= 0 ) {
		for ( int p = 0; p < (int)cNames; ++p ) {
		    proplist->insert( dispId, property );
		    proplist2->insert( p, dispId );
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

    CComPtr<IConnectionPoint> cpoint;
    FindConnectionPoint( IID_QAxEvents, &cpoint );
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
		while ( cc ) {
		    if ( c->pUnk ) {
			CComPtr<IDispatch> disp;
			c->pUnk->QueryInterface( IID_QAxEvents, (void**)&disp );
			if ( disp ) {
			    disp->Invoke( eventId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispParams, 0, 0, &argErr );
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
	    int index = activeqt->metaObject()->findSlot( slot->name );
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
    return IDispatchImpl<IDispatch, &IID_QAxInterface, &IID_QAxTypeLib>::
	Invoke( dispidMember, riid, lcid, wFlags, pDispParams, pvarResult, pexcepinfo, puArgErr );
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
    if ( cp )
	return S_OK;
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
    disp->Invoke( dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, &var, 0, 0 );

    switch( dispID ) {
    case DISPID_AMBIENT_APPEARANCE:
	break;
    case DISPID_AMBIENT_AUTOCLIP:
	break;
    case DISPID_AMBIENT_BACKCOLOR:
    case DISPID_AMBIENT_FORECOLOR:
	{
	    if ( var.vt != VT_I4 )
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
    }

    return S_OK;
}

HRESULT QActiveQtBase::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    if ( !pszUserType )
	return E_POINTER;

    if ( !activeqt )
	return IOleObjectImpl<QActiveQtBase>::GetUserType( dwFormOfType, pszUserType );

    switch ( dwFormOfType ) {
    case USERCLASSTYPE_FULL:
	return IOleObjectImpl<QActiveQtBase>::GetUserType( dwFormOfType, pszUserType );
    case USERCLASSTYPE_SHORT:
	*pszUserType = QStringToBSTR( activeqt->name() ? activeqt->name() : activeqt->className() );
	break;
    case USERCLASSTYPE_APPNAME:
	*pszUserType = QStringToBSTR( activeqt->caption() );
	break;
    }
    return S_OK;
}

/****************************************************************************
** $Id: $
**
** Implementation of the QAxServerBase classes
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

#include "qaxserverbase.h"
#include "qaxbindable.h"

#include <qapplication.h>

#include "../shared/types.h"


QPtrList<CComTypeInfoHolder> *QAxServerBase::typeInfoHolderList = 0;
GUID IID_IAxServerBase = { 0xbd2ec165, 0xdfc9, 0x4319, { 0x8b, 0x9b, 0x60, 0xa5, 0x74, 0x78, 0xe9, 0xe3} };


/*
    Helper class to enumerate all supported event interfaces.
*/
class QAxSignalVec : public IEnumConnectionPoints
{
public:
    QAxSignalVec( const QAxServerBase::ConnectionPoints &points ) 
	: cpoints( points ), ref(0) 
    {
    }
    QAxSignalVec( const QAxSignalVec &old )
    {
	ref = 0;
	cpoints = old.cpoints;
	for ( QAxServerBase::ConnectionPointsIterator i = cpoints.begin(); i != cpoints.end(); ++i )
	    (*i)->AddRef();
	it = old.it;
    }
    ~QAxSignalVec()
    {
	for ( QAxServerBase::ConnectionPointsIterator i = cpoints.begin(); i != cpoints.end(); ++i )
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

    QAxServerBase::ConnectionPoints cpoints;
    QAxServerBase::ConnectionPointsIterator it;

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

    QAxConnection( QAxServerBase *parent, const QUuid &uuid )
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
	checkImpl->Release();

	CONNECTDATA cd;
	cd.dwCookie = connections.count()+1;
	cd.pUnk = pUnk;
	cd.pUnk->AddRef();
	connections.append(cd);

	*pdwCookie = cd.dwCookie;
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
    QAxServerBase *that;
    QUuid iid;
    Connections connections;
    Iterator it;

    unsigned long ref;
};


/*!
    \class QAxServerBase qaxserverbase.h
    \brief The QAxServerBase class is a ActiveX control hosting a QWidget.

    \internal
*/

/*!
    Constructs a QAxServerBase object wrapping the QWidget \a classname into an ActiveX control.

    The constructor is called by the QClassFactory object provided by the COM server for the
    respective CLSID.
*/
QAxServerBase::QAxServerBase( const QString &classname )
: activeqt( 0 ), initNewCalled(FALSE), dirtyflag( FALSE ), hasStockEvents( FALSE ),
  ref( 0 ), class_name( classname ), slotlist(0), signallist(0),proplist(0), proplist2(0),
  propPageSite( 0 ), propPage( 0 )
{
    m_bWindowOnly = TRUE;
    _Module.Lock();
    if ( !typeInfoHolderList ) {
	typeInfoHolderList = new QPtrList<CComTypeInfoHolder>;
	typeInfoHolderList->setAutoDelete( TRUE );
    }

    _tih = new CComTypeInfoHolder();
    _tih->m_pguid = new GUID( _Module.factory()->interfaceID( class_name ) );
    _tih->m_plibid = new GUID( _Module.factory()->typeLibID() );
    _tih->m_wMajor = 1;
    _tih->m_wMinor = 0;
    _tih->m_dwRef = 0;
    _tih->m_pInfo = NULL;
    _tih->m_pMap = NULL;
    _tih->m_nCount = 0;
    typeInfoHolderList->append( _tih );

    _tih2 = new CComTypeInfoHolder();
    _tih2->m_pguid = new GUID( _Module.factory()->classID( class_name ) );
    _tih2->m_plibid = new GUID( _Module.factory()->typeLibID() );
    _tih2->m_wMajor = 1;
    _tih2->m_wMinor = 0;
    _tih2->m_dwRef = 0;
    _tih2->m_pInfo = NULL;
    _tih2->m_pMap = NULL;
    _tih2->m_nCount = 0;
    typeInfoHolderList->append( _tih2 );

    points[IID_IPropertyNotifySink] = new QAxConnection( this, IID_IPropertyNotifySink );
    points[_Module.factory()->eventsID(class_name)] = new QAxConnection( this, _Module.factory()->eventsID(class_name) );

    internalCreate();
}

/*!
    Destroys the QAxServerBase object, releasing all allocated resources and interfaces.
*/
QAxServerBase::~QAxServerBase()
{
    for ( QAxServerBase::ConnectionPointsIterator it = points.begin(); it != points.end(); ++it )
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

/*
    QueryInterface implementation.

    Calls QAxBindable::queryInterface and returns the result if an interface
    has been provided. Otherwise, calls the ATL implementation of QueryInterface
    using the COM_MAP declared in the class declaration.
*/
HRESULT WINAPI QAxServerBase::QueryInterface( REFIID iid, void **iface )
{
    *iface = 0;
    if ( activeqt ) {
	QAxBindable *aqt = (QAxBindable*)activeqt->qt_cast( "QAxBindable" );
	if ( aqt ) {
	    aqt->queryInterface( iid, iface );
	}
    }
    
    if ( *iface )
	return S_OK;
    return _InternalQueryInterface( iid, iface );
}

class HackWidget : public QWidget
{
    friend class QAxServerBase;
};

/*!
    Creates the QWidget for the classname passed to the c'tor.

    All signals of the widget class are connected to the internal event mapper.
    If the widget implements QAxBindable, stock events are also connected.
*/
bool QAxServerBase::internalCreate()
{
    const QMetaObject *mo = _Module.factory()->metaObject( class_name );

    activeqt = _Module.factory()->create( class_name );
    Q_ASSERT(activeqt);
    if ( !activeqt )
	return FALSE;
    QAxBindable *axb = (QAxBindable*)activeqt->qt_cast( "QAxBindable" );
    if ( axb ) {
	axb->activex = this;
	hasStockEvents = axb->hasStockEvents();
    }
    if ( !axb || !axb->stayTopLevel() ) {
	((HackWidget*)activeqt)->clearWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu );
	((HackWidget*)activeqt)->topData()->ftop = 0;
	((HackWidget*)activeqt)->topData()->fright = 0;
	((HackWidget*)activeqt)->topData()->fleft = 0;
	((HackWidget*)activeqt)->topData()->fbottom = 0;
	::SetWindowLong( activeqt->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
    }

    // connect the generic slot to all signals of activeqt
    for ( int isignal = mo->numSignals( TRUE )-1; isignal >= 0; --isignal )
	connectInternal( activeqt, isignal, this, 2, isignal );
    // install an event filter for stock events
    activeqt->installEventFilter( this );

    return TRUE;
}

/*!
    Makes sure that the Qt widget for the ActiveX has been initialized.
*/
LRESULT QAxServerBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if ( !activeqt )
	internalCreate();
    return 0;
}

/*!
    Destroys the Qt widget.
*/
LRESULT QAxServerBase::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if ( activeqt ) {
	delete activeqt;
	activeqt = 0;
    }
    return 0;
}

/*!
    Forwards all messages sent to the ActiveX window to the Qt widget.
*/
LRESULT QAxServerBase::ForwardMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
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
	    {
		QAxBindable *axb = (QAxBindable*)activeqt->qt_cast( "QAxBindable" );
		if ( !axb || !axb->stayTopLevel() ) {
		    ::SetParent( activeqt->winId(), m_hWnd );
		    activeqt->raise();
		    activeqt->move( 0, 0 );
		}
		if( wParam )
		    activeqt->show();
		else
		    activeqt->hide();
	    }
	    return 0;
	default:
	    break;
	}
	return ::SendMessage( activeqt->winId(), uMsg, wParam, lParam );
    }
    return 0;
}

/*!
    Creates mappings between DISPIDs and Qt signal/slot/property data.
*/
void QAxServerBase::readMetaData()
{
    if ( !activeqt )
	return;

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
				if ( eventattr && eventattr->guid == _Module.factory()->eventsID( class_name ) ) {
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

/*!
    Catches all signals emitted by the Qt widget and fires the respective COM event.
*/
bool QAxServerBase::qt_emit( int isignal, QUObject* _o )
{
    if ( m_nFreezeEvents )
	return TRUE;

    if ( !signallist )
	readMetaData();

    // get the signal information.
    bool stockEvent = isignal < 0;
    const QMetaData *signal = stockEvent ? 0 : activeqt->metaObject()->signal( isignal, TRUE );
    if ( !signal && !stockEvent )
	return FALSE;
    int signalcount = signal ? signal->method->count : 0;
    if ( stockEvent ) {
	switch( isignal ) {
	case DISPID_KEYDOWN:
	case DISPID_KEYUP:
	    signalcount = 2;
	    break;	
	case DISPID_KEYPRESS:
	    signalcount = 1;
	    break;
	case DISPID_MOUSEDOWN:
	case DISPID_MOUSEMOVE:
	case DISPID_MOUSEUP:
	    signalcount = 4;
	    break;
	default:
	    signalcount = 0;
	    break;
	}
    }
    if ( signalcount && !_o ) {
	qWarning( "Internal Error: missing %d arguments in qt_emit", signalcount );
	return FALSE;
    }

    // Get the Dispatch ID of the method to be called
    DISPID eventId = stockEvent ? isignal : signallist->operator [](isignal);
    if ( eventId == -1 )
	return FALSE;

    // For all connected event sinks...
    CComPtr<IConnectionPoint> cpoint;
    FindConnectionPoint( _Module.factory()->eventsID( class_name ), &cpoint );
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
		    QUObjectToVARIANT( obj, arg, signal->method->parameters + p );
		    dispParams.rgvarg[ signalcount - p - 1 ] = arg;
		}
		// call listeners (through IDispatch)
		GUID IID_QAxEvents = _Module.factory()->eventsID( class_name );
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

/*!
    Call IPropertyNotifySink of connected clients.
*/
bool QAxServerBase::emitRequestPropertyChange( DISPID dispId )
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
			    // a client disallows the property to change
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

/*!
    Call IPropertyNotifySink of connected clients.
*/
void QAxServerBase::emitPropertyChanged( DISPID dispId )
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

//**** IDispatch
/*!
    Map the COM call to the Qt slot/property for \a dispidMember.
*/
HRESULT QAxServerBase::Invoke( DISPID dispidMember, REFIID riid,
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
	    QUObject *objects = 0;
	    const QUParameter *params = slot->method->parameters;
	    if ( pcount ) {
		int retoff = ( params[0].inOut & QUParameter::Out ) ? 1 : 0;
		objects = new QUObject[pcount+1];
		for ( int p = 0; p < pcount; ++p ) {
		    // map the VARIANT to the QUObject, and try to get the required type
		    objects[p+1].type = params[p+retoff].type;  // first object is return value
		    VARIANTToQUObject( pDispParams->rgvarg[ pcount-p-1 ], objects + p + 1 );
		}
	    }

	    // call the slot
	    activeqt->qt_invoke( index, objects );

	    // ### update reference parameters and value
	    for ( int p = 0; p < pcount; ++p ) {
		objects[p+1].type->clear( objects + p + 1 );
	    }
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

	    QVariant var = VARIANTToQVariant( *pDispParams->rgvarg, property->type() );
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

//**** IConnectionPointContainer
/*!
    Provide the IEnumConnectionPoints implemented in the QAxSignalVec class.
*/
HRESULT QAxServerBase::EnumConnectionPoints( IEnumConnectionPoints **epoints )
{
    if ( !epoints )
	return E_POINTER;
    *epoints = new QAxSignalVec( points );
    (*epoints)->AddRef();
    return S_OK;
}

/*!
    Provide the IConnectionPoint implemented in the QAxConnection for \a iid.
*/
HRESULT QAxServerBase::FindConnectionPoint( REFIID iid, IConnectionPoint **cpoint )
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

//**** IPersistPropertyBag
/*!
    Initialize the properties of the Qt widget.
*/
HRESULT QAxServerBase::InitNew()
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

/*!
    Set the properties of the Qt widget to the values provided in the \a bag.
*/
HRESULT QAxServerBase::Load( IPropertyBag *bag, IErrorLog * /*log*/ )
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
	const QMetaProperty *property = mo->property( prop, TRUE );
	const char* pname = property->name();
	BSTR bstr = QStringToBSTR( pname );
	VARIANT var;
	var.vt = VT_EMPTY;
	HRESULT res = bag->Read( bstr, &var, 0 );
	if ( res != S_OK || !activeqt->setProperty( pname, VARIANTToQVariant( var, property->type() ) ) )
	    error = TRUE;
	SysFreeString(bstr);
    }

    return error ? E_FAIL : S_OK;
}

/*!
    Save the properties of the Qt widget into the \a bag.
*/
HRESULT QAxServerBase::Save( IPropertyBag *bag, BOOL /*clearDirty*/, BOOL /*saveAll*/ )
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

//**** IPersistStorage
/*!
    \reimp

    See documentation of IPersistStorage::IsDirty.
*/
HRESULT QAxServerBase::IsDirty()
{
    return dirtyflag ? S_OK : S_FALSE;
}

//**** IOleControl
/*!
    Return E_NOTIMPL
*/
HRESULT QAxServerBase::GetControlInfo( LPCONTROLINFO )
{
    return E_NOTIMPL;
}

/*!
    Turns event firing on and off.
*/
HRESULT QAxServerBase::FreezeEvents( BOOL bFreeze )
{
    // member of CComControl
    if ( bFreeze )
	m_nFreezeEvents++;
    else
	m_nFreezeEvents--;

    return S_OK;
}

/*!
    Return E_NOTIMPL
*/
HRESULT QAxServerBase::OnMnemonic( LPMSG )
{
    return E_NOTIMPL;
}
/*!
    Update the ambient properties of the Qt widget.
*/
HRESULT QAxServerBase::OnAmbientPropertyChange( DISPID dispID )
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
	    pal.setColor( dispID == DISPID_AMBIENT_BACKCOLOR ? QColorGroup::Background : QColorGroup::Foreground, 
		OLEColorToQColor( rgb ) );		
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
		QFont qfont = IFontToQFont( f );
		activeqt->setFont( qfont );
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

static inline LPOLESTR QStringToOLESTR( const QString &qstring )
{
    LPOLESTR olestr = (wchar_t*)CoTaskMemAlloc(qstring.length()*2+2);
    memcpy( olestr, (ushort*)qstring.unicode(), qstring.length()*2 );
    olestr[qstring.length()] = 0;
    return olestr;
}

//**** IOleObject
/*!
    \reimp

    See documentation of IOleObject::GetUserType.
*/
HRESULT QAxServerBase::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    if ( !pszUserType )
	return E_POINTER;

    switch ( dwFormOfType ) {
    case USERCLASSTYPE_FULL:
	*pszUserType = QStringToOLESTR( class_name );
	break;
    case USERCLASSTYPE_SHORT:
	if ( !activeqt || activeqt->caption().isEmpty() )
	    *pszUserType = QStringToOLESTR( class_name );
	else
	    *pszUserType = QStringToOLESTR( activeqt->caption() );
	break;
    case USERCLASSTYPE_APPNAME:
	*pszUserType = QStringToOLESTR( qApp->name() );
	break;
    }

    return S_OK;
}

/*!
    \reimp

    See documentation of IOleObject::GetMiscStatus.
*/
HRESULT QAxServerBase::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::GetMiscStatus\n"));
    return OleRegGetMiscStatus( _Module.factory()->classID( class_name ), dwAspect, pdwStatus);
}

//**** ISpecifyPropertyPages
/*!
    Returns information about a single property page. 
    The page has the same ID as the object (CLSID).
*/
HRESULT QAxServerBase::GetPages( CAUUID *pPages )
{
    if ( !pPages )
	return E_POINTER;
    
    int pages = 1;
    pPages->cElems = pages;
    pPages->pElems = (GUID*)CoTaskMemAlloc( sizeof(GUID) * pages );
    *(pPages->pElems) = _Module.factory()->classID( class_name );
    
    return S_OK;
}

//**** IPropertyPage
#include <qlabel.h>
#include <qlayout.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qpushbutton.h>

/*
    Helper class that provides a QWidget that docks into the COM property site.
*/
class QAxPropertyPage : public QWidget
{
    Q_OBJECT
public:
    QAxPropertyPage( HWND parent, QAxServerBase *base )
	: QWidget( 0, "prop page" ), hWndParent( parent ), that( base )
    {
	topData()->ftop = 0;
	topData()->fright = 0;
	topData()->fleft = 0;
	topData()->fbottom = 0;
	::SetWindowLong( winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

	QVBoxLayout *vbox = new QVBoxLayout( this );
	QHBoxLayout *hbox = new QHBoxLayout( 0 );
    
	listProperties = new QListView( this );
	listProperties->setAllColumnsShowFocus( TRUE );
	listProperties->addColumn( tr("Property") );
	listProperties->addColumn( tr("Value") );
	listProperties->header()->setClickEnabled( FALSE );
    
	QLabel *valueLabel = new QLabel( "Property &Value: ", this );
	editValue = new QLineEdit( this );
	QPushButton *setButton = new QPushButton( "&Set Value", this );

	valueLabel->setBuddy( editValue );
    
	hbox->addWidget( valueLabel );
	hbox->addWidget( editValue );
	hbox->addWidget( setButton );
    
	vbox->addWidget( listProperties );
	vbox->addLayout( hbox );

	connect( setButton, SIGNAL(clicked()), this, SLOT(setValue()) );
	connect( listProperties, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(currentChanged(QListViewItem*)) );
    }

    void updateProperties()
    {
	listProperties->clear();

	QPtrListIterator<IAxServerBase> it( that->propObjects );
	while ( it.current() ) {
	    IAxServerBase *ibase = it.current();
	    ++it;

	    QWidget *activex = ibase->widget();
	    QMetaObject *mo = activex->metaObject();

	    QString topclass = _Module.factory()->exposeToSuperClass( activex->className() );
	    QMetaObject *pmo = mo;
	    do {
		pmo = pmo->superClass();
	    } while ( pmo && topclass != pmo->className() );
	    int propoff = pmo ? pmo->propertyOffset() : mo->propertyOffset();

	    const int numprops = mo->numProperties( TRUE );
	    for ( int i = propoff; i < numprops; ++i ) {
		const QMetaProperty *property = mo->property( i, TRUE );
		Q_ASSERT( property );
		if ( !property || !property->writable() || !property->designable( activex ) || 
		      property->testFlags( QMetaProperty::Override ) )
		    continue;
		QListViewItem *item = listProperties->findItem( property->name(), 0 );
		if ( !item ) {
		    item = new QListViewItem( listProperties, property->name(), "qax_unset" );
		}
		QVariant var = activex->property( property->name() );
		QString valueText;
		
		switch ( var.type() ) {
		case QVariant::Color:
		    {
			QColor col = var.toColor();
			valueText = col.name();
		    }
		    break;
		case QVariant::Font:
		    {
			QFont fnt = var.toFont();
			valueText = fnt.toString();
		    }
		    break;
		    
		default:
		    valueText = var.toString();
		    break;
		}
		if ( item->text( 1 ) == "qax_unset" ) {
		    item->setText( 1, valueText );
		} else if ( item->text( 1 ) != valueText ) {
		    item->setText( 1, QString::null );
		}
	    }
	}
	listProperties->setCurrentItem( listProperties->firstChild() );
    }
    
    void applyChanged()
    {
	QListViewItemIterator itemit( listProperties );
	while ( itemit.current() ) {
	    QListViewItem *item = itemit.current();
	    ++itemit;

	    QPtrListIterator<IAxServerBase> it( that->propObjects );
	    IAxServerBase *ibase = it.current();
	    QWidget *activex = ibase->widget();
    
	    QVariant var = activex->property( item->text( 0 ) );
	    switch ( var.type() ) {
	    case QVariant::Color:
		{
		    QColor col;
		    col.setNamedColor( item->text(1) );
		    if ( !col.isValid() )
			continue;
		    var = col;
		}
		break;
	    case QVariant::Font:
		{
		    QFont fnt;
		    if ( !fnt.fromString( item->text(1) ) )
			continue;
		    
		    var = fnt;
		}
		break;
		
	    default:
		var = item->text(1);
		break;
	    }

	    while ( it.current() ) {
		ibase = it.current();
		++it;

		activex = ibase->widget();
		activex->setProperty( item->text(0), var );
	    }
	}
    }

    HWND hWndParent;

protected slots:
    void setValue()
    {
	QListViewItem *current = listProperties->currentItem();
	if ( !current )
	    return;
	
	QVariant var = that->widget()->property( current->text( 0 ) );
	switch( var.type() ) {
	case QVariant::Color:
	    {
		QColor col;
		col.setNamedColor( editValue->text() );
		if ( !col.isValid() )
		    return;
	    }
	    break;
	case QVariant::Font:
	    {
		QFont fnt;
		if ( !fnt.fromString( editValue->text() ) )
		    return;
	    }
	    break;
	}
	if ( editValue->text() != var.toString() ) {
	    current->setText( 1, editValue->text() );
	    that->propPageSite->OnStatusChange( PROPPAGESTATUS_DIRTY );
	}
    }

    void currentChanged( QListViewItem *current )
    {
	if ( !current )
	    return;
	
	editValue->setText( current->text( 1 ) );
    }

protected:
    void showEvent( QShowEvent *e )
    {
	updateProperties();

	QWidget::showEvent( e );
    }

private:
    QAxServerBase *that;
    QListView *listProperties;
    QLineEdit *editValue;

};

#include "qaxserverbase.moc"

/*!
    Sets the property page site.

    The property page calls OnStatusChange on that site.
*/
HRESULT QAxServerBase::SetPageSite( IPropertyPageSite *pPageSite )
{
    if ( !pPageSite && !propPageSite )
	return E_UNEXPECTED;

    if ( propPageSite )
	propPageSite->Release();
    
    propPageSite = pPageSite;
    if ( propPageSite )
	propPageSite->AddRef();

    return S_OK;
}

/*!
    Creates the property pages.
*/
HRESULT QAxServerBase::Activate( HWND hWndParent, LPCRECT pRect, BOOL bModal )
{
    if ( !pRect )
	return E_POINTER;

    propPage = new QAxPropertyPage( hWndParent, this );
    propPage->setGeometry( pRect->left, pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top );

    QAxBindable *qaxbind = (QAxBindable*)activeqt->qt_cast( "QAxBindable" );
    QWidget *page = /*qaxbind ? qaxbind->propertyPage() :*/ 0;
    if ( page )
	page->reparent( propPage, QPoint(0,0) );

    return S_OK;
}

/*!
    Destroys the property pages.
*/
HRESULT QAxServerBase::Deactivate()
{
    SetObjects( 0, 0 );
    delete propPage;
    propPage = 0;

    return S_OK;
}

/*!
    Returns page information.
*/
HRESULT QAxServerBase::GetPageInfo( PROPPAGEINFO *pPageInfo )
{
    if ( !pPageInfo )
	return E_POINTER;
    
    pPageInfo->cb = sizeof(PROPPAGEINFO);
    pPageInfo->size.cx = 100;
    pPageInfo->size.cy = 100;

    pPageInfo->pszTitle = QStringToOLESTR( "The Title" );
    pPageInfo->pszDocString = QStringToOLESTR( "The DocString" );
    pPageInfo->pszHelpFile = QStringToOLESTR( "The HelpFile" );
    pPageInfo->dwHelpContext = 0;

    return S_OK;
}

/*!
    Sets the objects the property page should display the properties for.
*/
HRESULT QAxServerBase::SetObjects( ULONG cObjects, IUnknown **ppUnk )
{
    QPtrListIterator<IAxServerBase> it( propObjects );
    while ( it.current() ) {
	it.current()->Release();
	++it;
    }
    propObjects.clear();

    if ( !ppUnk )
	return E_POINTER;
    for ( uint o = 0; o < cObjects; ++o ) {
	IUnknown *obj = ppUnk[o];
	IAxServerBase *iface;
	obj->QueryInterface( IID_IAxServerBase, (void**)&iface );
	if ( !iface )
	    return E_NOINTERFACE;
	propObjects.append( iface );
    }

    return S_OK;
}

/*!
    Shows and hides the page.
*/
HRESULT QAxServerBase::Show( UINT nCmdShow )
{
    if ( !propPage )
	return E_UNEXPECTED;
    if ( nCmdShow == SW_HIDE ) {
	propPage->hide();
    } else {
	QRect g = propPage->geometry();
	::SetParent( propPage->winId(), propPage->hWndParent );
	propPage->setGeometry( g );
	propPage->raise();
	propPage->show();
    }

    return S_OK;
}

/*!
    Places the page.
*/
HRESULT QAxServerBase::Move( LPCRECT pRect )
{
    if ( !pRect )
	return E_POINTER;
    if ( !propPage )
	return E_UNEXPECTED;

    propPage->setGeometry( pRect->left, pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top );

    return S_OK;
}

/*!
    Returns S_OK when the page is dirty (ie. values differ from object values).
*/
HRESULT QAxServerBase::IsPageDirty()
{
    return S_OK; //S_FALSE
}

/*!
   Returns S_OK when changes have been applied to the objects.
*/
HRESULT QAxServerBase::Apply()
{
    if ( !propPage )
	return E_UNEXPECTED;

    propPage->applyChanged();
    return S_OK; //S_FALSE(?)
}

/*!
    \reimp
*/
HRESULT QAxServerBase::Help( LPCOLESTR pszHelpDir )
{
    return E_NOTIMPL;
}

/*!
    \reimp
*/
HRESULT QAxServerBase::TranslateAccelerator( MSG *pMsg )
{
    if ( !pMsg )
	return E_POINTER;

    return E_NOTIMPL;
}

/*!
    \reimp
*/
HRESULT QAxServerBase::EditProperty( DISPID dispID )
{
    return E_NOTIMPL;
}

static int mapModifiers( int state )
{
    int ole = 0;
    if ( state & Qt::ShiftButton )
	ole |= 1;
    if ( state & Qt::ControlButton )
	ole |= 2;
    if ( state & Qt::AltButton )
	ole |= 4;

    return ole;
}

/*!
    \reimp
*/
bool QAxServerBase::eventFilter( QObject *o, QEvent *e )
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
    case QEvent::KeyPress:
	if ( o == activeqt && hasStockEvents ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    QUObject obj[3];
	    static_QUType_int.set( obj+1, ke->key() );
	    static_QUType_int.set( obj+2, mapModifiers( ke->state() ) );
	    qt_emit( DISPID_KEYDOWN, obj );
	    if ( ke->ascii() )
		qt_emit( DISPID_KEYPRESS, obj );
	}
	break;
    case QEvent::KeyRelease:
	if ( o == activeqt && hasStockEvents ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    QUObject obj[3];
	    static_QUType_int.set( obj+1, ke->key() );
	    static_QUType_int.set( obj+2, mapModifiers( ke->state() ) );
	    qt_emit( DISPID_KEYUP, obj );
	}
	break;
    case QEvent::MouseMove:
	if ( o == activeqt && hasStockEvents ) {
	    QMouseEvent *me = (QMouseEvent*)e;
	    QUObject obj[5]; // 0 = return value
	    static_QUType_int.set( obj+1, me->state() & Qt::MouseButtonMask );
	    static_QUType_int.set( obj+2, mapModifiers( me->state() ) );
	    static_QUType_int.set( obj+3, me->x() );
	    static_QUType_int.set( obj+4, me->y() );
	    qt_emit( DISPID_MOUSEMOVE, obj );
	}
	break;
    case QEvent::MouseButtonRelease:
	if ( o == activeqt && hasStockEvents ) {
	    QMouseEvent *me = (QMouseEvent*)e;
	    QUObject obj[5]; // 0 = return value
	    static_QUType_int.set( obj+1, me->button() );
	    static_QUType_int.set( obj+2, mapModifiers( me->state() ) );
	    static_QUType_int.set( obj+3, me->x() );
	    static_QUType_int.set( obj+4, me->y() );
	    qt_emit( DISPID_MOUSEUP, obj );
	    qt_emit( DISPID_CLICK, 0 );
	}
	break;
    case QEvent::MouseButtonDblClick:
	if ( o == activeqt && hasStockEvents ) {
	    QMouseEvent *me = (QMouseEvent*)e;
	    qt_emit( DISPID_DBLCLICK, 0 );
	}
	break;
    case QEvent::MouseButtonPress:
	{
	    if ( o == activeqt && hasStockEvents ) {
		QMouseEvent *me = (QMouseEvent*)e;
		QUObject obj[5]; // 0 = return value
		static_QUType_int.set( obj+1, me->button() );
		static_QUType_int.set( obj+2, mapModifiers( me->state() ) );
		static_QUType_int.set( obj+3, me->x() );
		static_QUType_int.set( obj+4, me->y() );
		qt_emit( DISPID_MOUSEDOWN, obj );
	    }
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

/****************************************************************************
** $Id: $
**
** Declaration and implementation of QAxServerBase and helper classes
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

#include <qapplication.h>
#include <qbuffer.h>
#include <qfocusdata.h>
#include <qguardedptr.h>
#include <qintdict.h>
#include <qmenubar.h>
#include <qmetaobject.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qpixmap.h>
#include <qptrdict.h>
#include <qstatusbar.h>
#include <qwhatsthis.h>
#include <olectl.h>

#include "qaxfactory.h"
#include "qaxbindable.h"

#include "../shared/types.h"

GUID IID_IAxServerBase = { 0xbd2ec165, 0xdfc9, 0x4319, { 0x8b, 0x9b, 0x60, 0xa5, 0x74, 0x78, 0xe9, 0xe3} };
struct IAxServerBase : public IUnknown
{
    virtual QObject *qObject() = 0;
    virtual QWidget *widget() = 0;
    virtual void emitPropertyChanged( const char*, long dispid = -1 ) = 0;
    virtual bool emitRequestPropertyChange( const char*, long dispid = -1 ) = 0;
};

// in qaxservermain.cpp
extern ITypeLib *qAxTypeLibrary;
extern QAxFactoryInterface *qAxFactory();
extern unsigned long qAxLock();
extern unsigned long qAxUnlock();
extern void* qAxInstance;


static QPtrDict<QAxServerBase> *ax_ServerMapper = 0;
static QPtrDict<QAxServerBase> *axServerMapper()
{
    if ( !ax_ServerMapper ) {
	ax_ServerMapper = new QPtrDict<QAxServerBase>;
    }
    return ax_ServerMapper;
}
static void axTakeServer( HWND hWnd )
{
    if ( !ax_ServerMapper )
	return;

    axServerMapper()->take( hWnd );
    if ( !axServerMapper()->count() ) {
	delete ax_ServerMapper;
	ax_ServerMapper = 0;
    }
}


/*
    \class QAxServerBase qaxserverbase.cpp
    \brief The QAxServerBase class is an ActiveX control hosting a QWidget.
    \internal
*/
class QAxServerBase : 
    public QObject,
    public IAxServerBase,
    public IDispatch,
    public IOleObject,
    public IOleControl,
    public IViewObject2,
    public IOleInPlaceObject,
    public IOleInPlaceActiveObject,
    public IProvideClassInfo2,
    public IConnectionPointContainer,
    public IPersistStorage,
    public IPersistPropertyBag
{
public:
    typedef QMap<QUuid,IConnectionPoint*> ConnectionPoints;
    typedef QMap<QUuid,IConnectionPoint*>::Iterator ConnectionPointsIterator;

    QAxServerBase( const QString &classname );

    ~QAxServerBase();

// Window creation
    HWND create(HWND hWndParent, RECT& rcPos );
    HMENU createPopup( QPopupMenu *popup, HMENU oldMenu = 0 );
    void createMenu( QMenuBar *menuBar );

    static LRESULT CALLBACK ActiveXProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    int PreTranslateMessage( MSG *pMsg );

// IUnknown
    unsigned long WINAPI AddRef()
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = ++ref;
	LeaveCriticalSection( &refCountSection );

	return r;
    }
    unsigned long WINAPI Release()
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = --ref;
	LeaveCriticalSection( &refCountSection );

	if ( !r ) {
	    delete this;
	    return 0;
	}
	return r;
    }
    HRESULT WINAPI QueryInterface( REFIID iid, void **iface );

// IAxServerBase
    QObject *qObject()
    {
        return this;
    }
    QWidget *widget()
    {
	return activeqt;
    }

    void emitPropertyChanged( const char*, long dispid = -1 );
    bool emitRequestPropertyChange( const char*, long dispid = -1 );
    void readMetaData();

// IDispatch
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr);

// IProvideClassInfo
    STDMETHOD(GetClassInfo)(ITypeInfo** pptinfo);

// IProvideClassInfo2
    STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID);

// IOleObject
    STDMETHOD(Advise)( IAdviseSink* pAdvSink, DWORD* pdwConnection );
    STDMETHOD(Close)( DWORD dwSaveOption );
    STDMETHOD(DoVerb)( LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect );
    STDMETHOD(EnumAdvise)( IEnumSTATDATA** ppenumAdvise );
    STDMETHOD(EnumVerbs)( IEnumOLEVERB** ppEnumOleVerb );
    STDMETHOD(GetClientSite)( IOleClientSite** ppClientSite );
    STDMETHOD(GetClipboardData)( DWORD dwReserved, IDataObject** ppDataObject );
    STDMETHOD(GetExtent)( DWORD dwDrawAspect, SIZEL* psizel );
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHOD(GetMoniker)( DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk );
    STDMETHOD(GetUserClassID)( CLSID* pClsid );
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHOD(InitFromData)( IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved );
    STDMETHOD(IsUpToDate)();
    STDMETHOD(SetClientSite)( IOleClientSite* pClientSite );
    STDMETHOD(SetColorScheme)( LOGPALETTE* pLogPal );
    STDMETHOD(SetExtent)( DWORD dwDrawAspect, SIZEL* psizel );
    STDMETHOD(SetHostNames)( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj );
    STDMETHOD(SetMoniker)( DWORD dwWhichMoniker, IMoniker* ppmk );
    STDMETHOD(Unadvise)( DWORD dwConnection );
    STDMETHOD(Update)();

// IViewObject
    STDMETHOD(Draw)( DWORD dwAspect, LONG lIndex, void *pvAspect, DVTARGETDEVICE *ptd,
		    HDC hicTargetDevice, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
		    BOOL(__stdcall*pfnContinue)(DWORD), DWORD dwContinue );
    STDMETHOD(GetColorSet)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		    HDC hicTargetDev, LOGPALETTE **ppColorSet );
    STDMETHOD(Freeze)( DWORD dwAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze );
    STDMETHOD(Unfreeze)( DWORD dwFreeze );
    STDMETHOD(SetAdvise)( DWORD aspects, DWORD advf, IAdviseSink *pAdvSink );
    STDMETHOD(GetAdvise)( DWORD *aspects, DWORD *advf, IAdviseSink **pAdvSink );

// IViewObject2
    STDMETHOD(GetExtent)( DWORD dwAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL lpsizel );

// IOleControl
    STDMETHOD(FreezeEvents)(BOOL);
    STDMETHOD(GetControlInfo)(LPCONTROLINFO);
    STDMETHOD(OnAmbientPropertyChange)(DISPID);
    STDMETHOD(OnMnemonic)(LPMSG);

// IOleWindow
    STDMETHOD(GetWindow)(HWND *pHwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

// IOleInPlaceObject
    STDMETHOD(InPlaceDeactivate)();
    STDMETHOD(UIDeactivate)();
    STDMETHOD(SetObjectRects)(LPCRECT lprcPosRect, LPCRECT lprcClipRect);
    STDMETHOD(ReactivateAndUndo)();

// IOleInPlaceActiveObject
    STDMETHOD(TranslateAccelerator)( MSG *pMsg );
    STDMETHOD(OnFrameWindowActivate)( BOOL );
    STDMETHOD(OnDocWindowActivate)( BOOL fActivate );
    STDMETHOD(ResizeBorder)( LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow );
    STDMETHOD(EnableModeless)( BOOL );

// IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints**);
    STDMETHOD(FindConnectionPoint)(REFIID, IConnectionPoint**);

// IPersist
    STDMETHOD(GetClassID)(GUID*clsid)
    {
	*clsid = qAxFactory()->classID( class_name );
	return S_OK;
    }

// IPersistPropertyBag
    STDMETHOD(InitNew)(VOID);
    STDMETHOD(Load)(IPropertyBag *, IErrorLog *);
    STDMETHOD(Save)(IPropertyBag *, BOOL, BOOL);

// IPersistStorage
    STDMETHOD(InitNew)(IStorage *pStg );
    STDMETHOD(IsDirty)(VOID);
    STDMETHOD(Load)(IStorage *pStg );
    STDMETHOD(Save)(IStorage *pStg, BOOL fSameAsLoad );
    STDMETHOD(SaveCompleted)( IStorage *pStgNew );
    STDMETHOD(HandsOffStorage)();

// QObject
    bool qt_emit( int, QUObject* );

    bool eventFilter( QObject *o, QEvent *e );
private:
    void update();
    void updateGeometry();
    bool internalCreate();
    HRESULT internalActivate();

    friend class QAxBindable;
    friend class QAxPropertyPage;

    QWidget* activeqt;
    QAxAggregated *aggregatedObject;
    ConnectionPoints points;

    unsigned initNewCalled	:1;
    unsigned dirtyflag		:1;
    unsigned hasStockEvents	:1;
    unsigned stayTopLevel	:1;
    unsigned isInPlaceActive	:1;
    unsigned isUIActive		:1;
    unsigned wasUIActive	:1;
    unsigned isBindable		:1;
    short freezeEvents;

    HWND m_hWnd;
    HWND& m_hWndCD;

    HMENU hmenuShared;
    HOLEMENU holemenu;
    HWND hwndMenuOwner;
    QMap<HMENU, QPopupMenu*> menuMap;
    QGuardedPtr<QMenuBar> menuBar;
    QGuardedPtr<QStatusBar> statusBar;
    QGuardedPtr<QPopupMenu> currentPopup;

    SIZE sizeExtent;
    RECT rcPos;

    CRITICAL_SECTION refCountSection;
    CRITICAL_SECTION createWindowSection;

    unsigned long ref;

    QString class_name;

    QIntDict<QMetaData>* slotlist;
    QMap<int,DISPID>* signallist;
    QIntDict<QMetaProperty>* proplist;
    QMap<int, DISPID>* proplist2;

    IAdviseSink *m_spAdviseSink;
    IOleAdviseHolder *m_spOleAdviseHolder;
    IOleClientSite *m_spClientSite;
    IOleInPlaceSiteWindowless *m_spInPlaceSite;
    IOleInPlaceFrame *m_spInPlaceFrame;
    ITypeInfo *m_spTypeInfo;
    IStorage *m_spStorage;

    IPropertyPageSite *propPageSite;
    QAxPropertyPage *propPage;
    QPtrList<IAxServerBase> propObjects;
};

/*
    Helper class to enumerate all supported event interfaces.
*/
class QAxSignalVec : public IEnumConnectionPoints
{
public:
    QAxSignalVec( const QAxServerBase::ConnectionPoints &points )
	: cpoints( points ), ref(0)
    {
	InitializeCriticalSection( &refCountSection );
    }
    QAxSignalVec( const QAxSignalVec &old )
    {
	InitializeCriticalSection( &refCountSection );
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

	DeleteCriticalSection( &refCountSection );
    }

    unsigned long __stdcall AddRef() 
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = ++ref;
	LeaveCriticalSection( &refCountSection );
	return ++r;
    }
    unsigned long __stdcall Release()
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = --ref;
	LeaveCriticalSection( &refCountSection );

	if ( !r ) {
	    delete this;
	    return 0;
	}
	return r;
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
    CRITICAL_SECTION refCountSection;

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
	InitializeCriticalSection( &refCountSection );
    }
    QAxConnection( const QAxConnection &old )
    {
	InitializeCriticalSection( &refCountSection );
	ref = 0;
	connections = old.connections;
	it = old.it;
	that = old.that;
	iid = old.iid;
	QValueList<CONNECTDATA>::Iterator it = connections.begin();
	while ( it != connections.end() ) {
	    CONNECTDATA connection = *it;
	    ++it;
	    connection.pUnk->AddRef();
	}
    }
    ~QAxConnection()
    {
	DeleteCriticalSection( &refCountSection );
    }

    unsigned long __stdcall AddRef() 
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = ++ref;
	LeaveCriticalSection( &refCountSection );
	return ++r;
    }
    unsigned long __stdcall Release()
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = --ref;
	LeaveCriticalSection( &refCountSection );

	if ( !r ) {
	    delete this;
	    return 0;
	}
	return r;
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
	{
	    IDispatch *checkImpl = 0;
	    pUnk->QueryInterface( iid, (void**)&checkImpl );
	    if ( !checkImpl )
		return CONNECT_E_CANNOTCONNECT;
	    checkImpl->Release();
	}

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
	QValueList<CONNECTDATA>::Iterator it = connections.begin();
	while ( it != connections.end() ) {
	    CONNECTDATA cd = *it;
	    if ( cd.dwCookie == dwCookie ) {
		cd.pUnk->Release();
		connections.remove(it);
		return S_OK;
	    }
	    ++it;
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

    CRITICAL_SECTION refCountSection;
    unsigned long ref;
};


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
	InitializeCriticalSection( &refCountSection );

	// COM only knows the CLSID, but QAxFactory is class name based...
	QStringList keys = qAxFactory()->featureList();
	for ( QStringList::Iterator  key = keys.begin(); key != keys.end(); ++key ) {
	    if ( qAxFactory()->classID( *key ) == clsid ) {
		className = *key;
		break;
	    }
	}
    }
    ~QClassFactory()
    {
	DeleteCriticalSection( &refCountSection );
    }

    // IUnknown
    unsigned long WINAPI AddRef()
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = ++ref;
	LeaveCriticalSection( &refCountSection );
	return ++r;
    }
    unsigned long WINAPI Release()
    {
	EnterCriticalSection( &refCountSection );
	unsigned long r = --ref;
	LeaveCriticalSection( &refCountSection );

	if ( !r ) {
	    delete this;
	    return 0;
	}
	return r;
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
	if ( pUnkOuter )
	    return CLASS_E_NOAGGREGATION;

	// Make sure a QApplication instance is present (inprocess case)
	if ( !qApp ) {
	    qax_ownQApp = TRUE;
	    int argc = 0;
	    (void)new QApplication( argc, 0 );
#if defined(UNICODE)
	    if ( qWinVersion() & Qt::WV_NT_based )
		hhook = SetWindowsHookEx( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
	    else
#endif
		hhook = SetWindowsHookExA( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );	    
	}

	// Create the ActiveX wrapper
	QAxServerBase *activeqt = new QAxServerBase( className );
	return activeqt->QueryInterface( iid, ppObject );
    }
    HRESULT WINAPI LockServer( BOOL fLock )
    {
	if ( fLock )
	    qAxLock();
	else
	    qAxUnlock();

	return S_OK;
    }

    QString className;

protected:
    CRITICAL_SECTION refCountSection;
    unsigned long ref;
};

// Create a QClassFactory object for class \a iid
HRESULT WINAPI GetClassObject( void *pv, REFIID iid, void **ppUnk )
{
    QClassFactory *factory = new QClassFactory( iid );
    if ( !factory )
	return E_OUTOFMEMORY;
    if ( factory->className.isEmpty() )
	return E_NOINTERFACE;
	
    return factory->QueryInterface( IID_IClassFactory, ppUnk );
}


/*!
    Constructs a QAxServerBase object wrapping the QWidget \a
    classname into an ActiveX control.

    The constructor is called by the QClassFactory object provided by
    the COM server for the respective CLSID.
*/
QAxServerBase::QAxServerBase( const QString &classname )
: activeqt( 0 ), aggregatedObject( 0 ), ref( 0 ), class_name( classname ),
  slotlist(0), signallist(0),proplist(0), proplist2(0),
  propPageSite( 0 ), propPage( 0 ), m_hWnd(0), m_hWndCD( m_hWnd ),
  hmenuShared(0), hwndMenuOwner(0)
{
    initNewCalled	= FALSE;
    dirtyflag		= FALSE;
    hasStockEvents	= FALSE;
    stayTopLevel	= FALSE;
    isInPlaceActive	= FALSE;
    isUIActive		= FALSE;
    wasUIActive		= FALSE;
    isBindable		= FALSE;
    freezeEvents = 0;

    sizeExtent.cx = 2500;
    sizeExtent.cy = 2500;

    rcPos.left = rcPos.top = 0;
    rcPos.right = rcPos.bottom = 20;

    m_spAdviseSink = 0;
    m_spOleAdviseHolder = 0;
    m_spClientSite = 0;
    m_spInPlaceSite = 0;
    m_spInPlaceFrame = 0;
    m_spTypeInfo = 0;
    m_spStorage = 0;

    InitializeCriticalSection( &refCountSection );
    InitializeCriticalSection( &createWindowSection );

    qAxLock();

    points[IID_IPropertyNotifySink] = new QAxConnection( this, IID_IPropertyNotifySink );
    points[qAxFactory()->eventsID(class_name)] = new QAxConnection( this, qAxFactory()->eventsID(class_name) );

    internalCreate();
}

/*!
    Destroys the QAxServerBase object, releasing all allocated
    resources and interfaces.
*/
QAxServerBase::~QAxServerBase()
{
    for ( QAxServerBase::ConnectionPointsIterator it = points.begin(); it != points.end(); ++it ) {
	if ( it.data() )
	    (*it)->Release();
    }
    delete aggregatedObject;
    aggregatedObject = 0;
    if ( activeqt ) {
	axTakeServer( m_hWnd );
	axTakeServer( activeqt->winId() );
	activeqt->disconnect( this );
	QWidget *aqt = activeqt;
	activeqt = 0;
	delete aqt;
    }

    if ( m_spAdviseSink ) m_spAdviseSink->Release();
    m_spAdviseSink = 0;
    if ( m_spOleAdviseHolder ) m_spOleAdviseHolder->Release();
    m_spOleAdviseHolder = 0;
    if ( m_spClientSite ) m_spClientSite->Release();
    m_spClientSite = 0;
    if ( m_spInPlaceFrame ) m_spInPlaceFrame->Release();
    m_spInPlaceFrame = 0;
    if ( m_spInPlaceSite ) m_spInPlaceSite->Release();
    m_spInPlaceSite = 0;
    if ( m_spTypeInfo ) m_spTypeInfo->Release();
    m_spTypeInfo = 0;
    if ( m_spStorage ) m_spStorage->Release();
    m_spStorage = 0;

    DeleteCriticalSection( &refCountSection );
    DeleteCriticalSection( &createWindowSection );

    qAxUnlock();
    delete slotlist;
    delete signallist;
    delete proplist;
    delete proplist2;
}

/*
    QueryInterface implementation.
*/
HRESULT WINAPI QAxServerBase::QueryInterface( REFIID iid, void **iface )
{
    *iface = 0;

    if ( iid == IID_IUnknown) {
	*iface = (IUnknown*)(IDispatch*)this;
    } else {
	HRESULT res = S_OK;
	if ( aggregatedObject )
	    res = aggregatedObject->queryInterface( iid, iface );
	if ( *iface )
	    return res;
    }

    if ( !(*iface) ) {
	if ( iid == IID_IDispatch)
	    *iface = (IDispatch*)this;
	else if ( iid == IID_IAxServerBase)
	    *iface = (IAxServerBase*)this;
	else if ( iid == IID_IOleObject)
	    *iface = (IOleObject*)this;
	else if ( iid == IID_IViewObject)
	    *iface = (IViewObject*)this;
	else if ( iid == IID_IViewObject2)
	    *iface = (IViewObject2*)this;
	else if ( iid == IID_IOleControl)
	    *iface = (IOleControl*)this;
	else if ( iid == IID_IOleWindow)
	    *iface = (IOleWindow*)(IOleInPlaceObject*)this;
	else if ( iid == IID_IOleInPlaceObject)
	    *iface = (IOleInPlaceObject*)this;
	else if ( iid == IID_IOleInPlaceActiveObject)
	    *iface = (IOleInPlaceActiveObject*)this;
	else if ( iid == IID_IConnectionPointContainer)
	    *iface = (IConnectionPointContainer*)this;
	else if ( iid == IID_IProvideClassInfo)
	    *iface = (IProvideClassInfo*)this;
	else if ( iid == IID_IProvideClassInfo2)
	    *iface = (IProvideClassInfo2*)this;
	else if ( iid == IID_IPersistStorage )
	    *iface = (IPersistStorage*)this;
	else if ( iid == IID_IPersistPropertyBag)
	    *iface = (IPersistPropertyBag*)this;
	else
	    return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
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
    if ( activeqt )
	return TRUE;

    activeqt = qAxFactory()->create( class_name );
    Q_ASSERT(activeqt);
    if ( !activeqt )
	return FALSE;

    hasStockEvents = qAxFactory()->hasStockEvents( class_name );
    stayTopLevel = qAxFactory()->stayTopLevel( class_name );
    const QMetaObject *mo = activeqt->metaObject();
    QAxBindable *axb = (QAxBindable*)activeqt->qt_cast( "QAxBindable" );
    if ( axb ) {
	isBindable = TRUE;
	// no addref; this is aggregated
	axb->activex = this;
	aggregatedObject = axb->createAggregate();
	if ( aggregatedObject ) {
	    aggregatedObject->controlling_unknown = (IUnknown*)(IDispatch*)this;
	    aggregatedObject->the_widget = activeqt;
	}
    }
    if ( !stayTopLevel ) {
	((HackWidget*)activeqt)->clearWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu );
	((HackWidget*)activeqt)->topData()->ftop = 0;
	((HackWidget*)activeqt)->topData()->fright = 0;
	((HackWidget*)activeqt)->topData()->fleft = 0;
	((HackWidget*)activeqt)->topData()->fbottom = 0;
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    ::SetWindowLong( activeqt->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
	else
#endif
	    ::SetWindowLongA( activeqt->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
    }

    activeqt->setGeometry( rcPos.left, rcPos.top, rcPos.right-rcPos.left, rcPos.bottom-rcPos.top );
    updateGeometry();

    // connect the generic slot to all signals of activeqt
    for ( int isignal = mo->numSignals( TRUE )-1; isignal >= 0; --isignal )
	connectInternal( activeqt, isignal, this, 2, isignal );
    // install an event filter for stock events
    activeqt->installEventFilter( this );

    return TRUE;
}

class HackMenuData : public QMenuData
{
    friend class QAxServerBase;
};

/*
    Message handler. \a hWnd is always the ActiveX widget hosting the Qt widget. 
    \a uMsg is handled as follows
    \list
    \i WM_CREATE The ActiveX control is created
    \i WM_DESTROY The QWidget is destroyed
    \i WM_SHOWWINDOW The QWidget is parented into the ActiveX window
    \i WM_PAINT The QWidget is updated
    \i WM_SIZE The QWidget is resized to the new size
    \i WM_SETFOCUS and
    \i WM_KILLFOCUS The client site is notified about the focus transfer
    \i WM_MOUSEACTIVATE The ActiveX is activated
    \endlist

    The semantics of \a wParam and \a lParam depend on the value of \a uMsg.
*/
LRESULT CALLBACK QAxServerBase::ActiveXProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_CREATE ) {
	QAxServerBase *that;
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based ) {
	    CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
	    that = (QAxServerBase*)cs->lpCreateParams;
	} else
#endif
	{
	    CREATESTRUCTA *cs = (CREATESTRUCTA*)lParam;
	    that = (QAxServerBase*)cs->lpCreateParams;
	}
	axServerMapper()->insert( hWnd, that );
	that->m_hWnd = hWnd;

#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
	else
#endif
	    return ::DefWindowProcA( hWnd, uMsg, wParam, lParam );
    }

    QAxServerBase *that = axServerMapper()->find( hWnd );
    if ( that ) switch ( uMsg )
    {
    case WM_NCDESTROY:
	that->m_hWnd = 0;
	axTakeServer( hWnd );
	break;

    case WM_DESTROY:
	if ( that->activeqt ) {
	    if ( that->aggregatedObject )
		that->aggregatedObject->the_widget = 0;
	    delete that->activeqt;
	    that->activeqt = 0;
	}
	break;

    case WM_SHOWWINDOW:
	that->internalCreate();
	if( wParam ) {
	    if ( !that->stayTopLevel ) {
		::SetParent( that->activeqt->winId(), that->m_hWnd );
		that->activeqt->raise();
		that->activeqt->move( 0, 0 );
	    }
	    that->activeqt->show();
	} else {
	    that->activeqt->hide();
	}
	break;

    case WM_SIZE:
	if ( that->activeqt )
	    that->activeqt->resize( LOWORD(lParam), HIWORD(lParam) );
	break;

    case WM_SETFOCUS:
	if ( that->isInPlaceActive && that->m_spClientSite ) {
	    that->DoVerb(OLEIVERB_UIACTIVATE, NULL, that->m_spClientSite, 0, that->m_hWndCD, &that->rcPos);
	    if ( that->isUIActive ) {
		IOleControlSite *spSite = 0;
		that->m_spClientSite->QueryInterface( IID_IOleControlSite, (void**)&spSite );
		if ( spSite ) {
		    spSite->OnFocus(TRUE);
		    spSite->Release();
		}
		if ( that->activeqt->focusWidget() )
		    that->activeqt->focusWidget()->setFocus();
		else {
		    QFocusData *focusData = ((HackWidget*)that->activeqt)->focusData();
		    QWidget *candidate = focusData->first();
		    if ( candidate->focusPolicy() != QWidget::NoFocus )
			candidate->setFocus();
		    else
			((HackWidget*)candidate)->focusNextPrevChild( TRUE );
		}
	    }
	}
	break;

    case WM_KILLFOCUS:
	if ( that->isInPlaceActive && that->isUIActive && that->m_spClientSite ) {
	    IOleControlSite *spSite = 0;
	    that->m_spClientSite->QueryInterface( IID_IOleControlSite, (void**)&spSite );
	    if ( spSite ) {
		if ( !::IsChild(that->m_hWndCD, ::GetFocus()) )
		    spSite->OnFocus(FALSE);
		spSite->Release();
	    }
	}
	break;

    case WM_MOUSEACTIVATE:
	that->DoVerb(OLEIVERB_UIACTIVATE, NULL, that->m_spClientSite, 0, that->m_hWndCD, &that->rcPos);
	break;

    case WM_INITMENUPOPUP:
	if ( that->activeqt ) {
	    that->currentPopup = that->menuMap[(HMENU)wParam];
	    if ( !that->currentPopup )
		break;

	    const QMetaObject *mo = that->currentPopup->metaObject();
	    int index = mo->findSignal( "aboutToShow()", TRUE );
	    if ( index < 0 )
		break;

	    QUObject o;
	    that->currentPopup->qt_emit( index, &o );

	    that->createPopup( that->currentPopup, (HMENU)wParam );

	    return 0;
	}
	break;

    case WM_MENUSELECT:
    case WM_COMMAND:
	if ( that->activeqt ) {
	    QMenuBar *menuBar = that->menuBar;
	    if ( !menuBar )
		break;

	    int qtid = 0;
	    bool menuClosed = FALSE;
	    if ( uMsg == WM_COMMAND )
		qtid = int(wParam);
	    else if ( !lParam )
		menuClosed = TRUE;
	    else
		qtid = LOWORD(wParam);

	    QMenuData *menu = 0;
	    QMenuItem *qitem = menuClosed ? 0 : menuBar->findItem( qtid, &menu );
	    if ( uMsg == WM_MENUSELECT && !qitem && !menuClosed ) {
		qtid |= 0xffff0000;
		qitem = menuBar->findItem( qtid, &menu );
	    }
	    QObject *menuObject = 0;
	    if ( menuClosed )
		menuObject = that->currentPopup;
	    else if ( ((HackMenuData*)menu)->isMenuBar )
		menuObject = (QMenuBar*)menu;
	    else if ( ((HackMenuData*)menu)->isPopupMenu )
		menuObject = (QPopupMenu*)menu;

	    if ( menuObject && ( menuClosed || qitem ) ) {
		const QMetaObject *mo = menuObject->metaObject();
		int index = -1;
		if ( uMsg == WM_COMMAND )
		    index = mo->findSignal( "activated(int)", TRUE );
		else if ( menuClosed )
		    index = mo->findSignal( "aboutToHide()", TRUE );
		else
		    index = mo->findSignal( "highlighted(int)", TRUE );

		if ( index < 0 )
		    break;

		if ( menuClosed ) {
		    QUObject o;
		    menuObject->qt_emit( index, &o );
		} else {
		    QUObject o[2];
		    static_QUType_int.set( o+1, qtid );
		    if ( uMsg == WM_COMMAND && qitem->signal() )	    // activate signal
			qitem->signal()->activate();
		    menuObject->qt_emit( index, o );
		}

		return 0;
	    }
	}
	break;

    default:
	break;
    }

#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
    else
#endif
	return ::DefWindowProcA( hWnd, uMsg, wParam, lParam );
}

/*!
    Creates the window hosting the QWidget.
*/
HWND QAxServerBase::create(HWND hWndParent, RECT& rcPos )
{
     // ##why not create the QWidget here?
    static ATOM atom = 0;
    HINSTANCE hInst = (HINSTANCE)qAxInstance;
    EnterCriticalSection( &createWindowSection );
    if ( !atom ) {
#ifdef UNICODE
	if ( qWinVersion() & Qt::WV_NT_based ) {
	    WNDCLASSW wcTemp;
	    wcTemp.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	    wcTemp.cbClsExtra = 0;
	    wcTemp.cbWndExtra = 0;
	    wcTemp.hbrBackground = HBRUSH(COLOR_WINDOW+1);
	    wcTemp.hCursor = 0;
	    wcTemp.hIcon = 0;
	    wcTemp.hInstance = hInst;
	    wcTemp.lpszClassName = L"QAxControl";
	    wcTemp.lpszMenuName = 0;
	    wcTemp.lpfnWndProc = ActiveXProc;

	    atom = RegisterClassW( &wcTemp );

	} else
#endif
	{
	    WNDCLASSA wcTemp;
	    wcTemp.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	    wcTemp.cbClsExtra = 0;
	    wcTemp.cbWndExtra = 0;
	    wcTemp.hbrBackground = HBRUSH(COLOR_WINDOW+1);
	    wcTemp.hCursor = 0;
	    wcTemp.hIcon = 0;
	    wcTemp.hInstance = hInst;
	    wcTemp.lpszClassName = "QAxControl";
	    wcTemp.lpszMenuName = 0;
	    wcTemp.lpfnWndProc = ActiveXProc;

	    atom = RegisterClassA( &wcTemp );
	}
    }
    LeaveCriticalSection( &createWindowSection );
    if ( !atom )
	return 0;
    
    Q_ASSERT( !m_hWnd );

    HWND hWnd = 0;
#ifdef UNICODE
    if ( qWinVersion() & Qt::WV_NT_based )
	hWnd = ::CreateWindowW( (TCHAR*)MAKELONG(atom, 0), 0,
	    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	    rcPos.left, rcPos.top, rcPos.right - rcPos.left,
	    rcPos.bottom - rcPos.top, hWndParent, 0, hInst, this );
    else
#endif
	hWnd = ::CreateWindowA( (char*)MAKELONG(atom, 0), 0,
	    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	    rcPos.left, rcPos.top, rcPos.right - rcPos.left,
	    rcPos.bottom - rcPos.top, hWndParent, 0, hInst, this );

    Q_ASSERT(m_hWnd == hWnd);

    internalCreate();

    return hWnd;
}

/*
    Recoursively creates Win32 submenus.
*/
HMENU QAxServerBase::createPopup( QPopupMenu *popup, HMENU oldMenu )
{
    HMENU popupMenu = oldMenu ? oldMenu : CreatePopupMenu();
    menuMap[popupMenu] = popup;

    if ( oldMenu ) while ( GetMenuItemCount(oldMenu) ) {
	DeleteMenu( oldMenu, 0, MF_BYPOSITION );
    }

    for ( uint i = 0; i < popup->count(); ++i ) {
	int qid = popup->idAt(i);
	QMenuItem *qitem = popup->findItem( qid );
	if ( !qitem )
	    continue;

	uint flags = qitem->isEnabled() ? MF_ENABLED : MF_GRAYED;
	if ( qitem->isSeparator() )
	    flags |= MF_SEPARATOR;
	else if ( qitem->popup() )
	    flags |= MF_POPUP;
	else
	    flags |= MF_STRING;
	if ( qitem->isChecked() )
	    flags |= MF_CHECKED;

	UINT itemId = qitem->popup() ? (UINT_PTR)createPopup( qitem->popup() ) : qid;
	QT_WA( {
	    AppendMenuW( popupMenu, flags, itemId, (TCHAR*)qitem->text().ucs2() );
	}, {
	    AppendMenuA( popupMenu, flags, itemId, qitem->text().local8Bit() );
	} );
    }
    if ( oldMenu )
	DrawMenuBar( hwndMenuOwner );
    return popupMenu;
}

/*!
    Creates a Win32 menubar.
*/
void QAxServerBase::createMenu( QMenuBar *menuBar )
{
    hmenuShared = ::CreateMenu();

    int edit = 0;
    int object = 0;
    int help = 0;

    for ( uint i = 0; i < menuBar->count(); ++i ) {
	int qid = menuBar->idAt( i );
	QMenuItem *qitem = menuBar->findItem( qid );
	if ( !qitem )
	    continue;

	uint flags = qitem->isEnabled() ? MF_ENABLED : MF_GRAYED;
	if ( qitem->isSeparator() )
	    flags |= MF_SEPARATOR;
	else if ( qitem->popup() )
	    flags |= MF_POPUP;
	else
	    flags |= MF_STRING;

	if ( qitem->text() == activeqt->tr("&Edit") )
	    edit++;
	else if ( qitem->text() == activeqt->tr("&Help") )
	    help++;
	else
	    object++;

	UINT itemId = qitem->popup() ? (UINT)createPopup( qitem->popup() ) : qid;
	QT_WA( {
	    AppendMenuW( hmenuShared, flags, itemId, (TCHAR*)qitem->text().ucs2() );
	} , {
	    AppendMenuA( hmenuShared, flags, itemId, qitem->text().local8Bit() );
	} );
    }

    OLEMENUGROUPWIDTHS menuWidths = {0,edit,0,object,0,help};
    HRESULT hres = m_spInPlaceFrame->InsertMenus( hmenuShared, &menuWidths );
    if ( FAILED(hres) ) {
	::DestroyMenu( hmenuShared );
	return;	   
    }

    m_spInPlaceFrame->GetWindow( &hwndMenuOwner );

    holemenu = OleCreateMenuDescriptor( hmenuShared, &menuWidths );
    hres = m_spInPlaceFrame->SetMenu( hmenuShared, holemenu, m_hWnd );
    if ( FAILED(hres) ) {
	::DestroyMenu( hmenuShared );
	OleDestroyMenuDescriptor( holemenu );
    }
}

extern bool ignoreSlots( const char *test );
extern bool ignoreProps( const char *test );

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

	const QMetaObject *mo = activeqt->metaObject();
	for ( int islot = mo->numSlots( TRUE )-1; islot >=0 ; --islot ) {
	    const QMetaData *slot = mo->slot( islot, TRUE );

	    if ( ignoreSlots( slot->method->name ) || slot->access != QMetaData::Public )
		continue;

	    BSTR bstrNames = QStringToBSTR( slot->method->name );
	    DISPID dispId;
	    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, 1, LOCALE_USER_DEFAULT, &dispId );
	    if ( dispId >= 0 )
		slotlist->insert( dispId, slot );
	    SysFreeString( bstrNames );
	}
	IConnectionPointContainer *cpoints = 0;
	QueryInterface( IID_IConnectionPointContainer, (void**)&cpoints );
	if ( cpoints ) {
	    IProvideClassInfo *classinfo = 0;
	    cpoints->QueryInterface( IID_IProvideClassInfo, (void**)&classinfo );
	    if ( classinfo ) {
		ITypeInfo *info = 0;
		ITypeInfo *eventinfo = 0;
		classinfo->GetClassInfo( &info );
		if ( info ) {
		    TYPEATTR *typeattr = 0;
		    info->GetTypeAttr( &typeattr );
		    if ( typeattr ) {
			for ( int impl = 0; impl < typeattr->cImplTypes && !eventinfo; ++impl ) {
			    // get the ITypeInfo for the interface
			    HREFTYPE reftype;
			    info->GetRefTypeOfImplType( impl, &reftype );
			    ITypeInfo *eventtype = 0;
			    info->GetRefTypeInfo( reftype, &eventtype );
			    if ( eventtype ) {
				TYPEATTR *eventattr;
				eventtype->GetTypeAttr( &eventattr );
				// this is it
				if ( eventattr && eventattr->guid == qAxFactory()->eventsID( class_name ) )
				    eventinfo = eventtype;

				eventtype->ReleaseTypeAttr( eventattr );
				if ( eventtype != eventinfo ) 
				    eventtype->Release();
			    }
			}
			info->ReleaseTypeAttr( typeattr );
		    }
		    info->Release();
		}
		if ( eventinfo ) {
		    for ( int isignal = mo->numSignals( TRUE )-1; isignal >= 0; --isignal ) {
			const QMetaData *signal = mo->signal( isignal, TRUE );

			BSTR bstrNames = QStringToBSTR( signal->method->name );
			DISPID dispId;
			eventinfo->GetIDsOfNames( (BSTR*)&bstrNames, 1, &dispId );
			if ( dispId >= 0 )
			    signallist->insert( isignal, dispId );
			else
			    signallist->insert( isignal, -1 );
			SysFreeString( bstrNames );
		    }
		    eventinfo->Release();
		}
		classinfo->Release();
	    }
	    cpoints->Release();
	}
	for ( int iproperty = mo->numProperties( TRUE )-1; iproperty >= 0; --iproperty ) {
	    const QMetaProperty *property = mo->property( iproperty, TRUE );

	    if ( ignoreProps( property->name() ) )
		continue;

	    BSTR bstrNames = QStringToBSTR( property->name() );
	    DISPID dispId;
	    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, 1, LOCALE_USER_DEFAULT, &dispId );
	    if ( dispId >= 0 ) {
		proplist->insert( dispId, property );
		proplist2->insert( iproperty, dispId );
	    }
	    SysFreeString( bstrNames );
	}
    }
}


/*!
    \internal
    Updates the view, or asks the client site to do so.
*/
void QAxServerBase::update()
{
    if ( isInPlaceActive ) {
	if ( m_hWndCD )
	    ::InvalidateRect( m_hWndCD, 0, TRUE );
	else if ( m_spInPlaceSite )
	    m_spInPlaceSite->InvalidateRect( NULL, TRUE );
    } else if (m_spAdviseSink) {
	m_spAdviseSink->OnViewChange( DVASPECT_CONTENT, -1 );
    }
}

/*!
    \internal

    Updates the internal size values.
*/
void QAxServerBase::updateGeometry()
{
    QSize sizeHint;
    if ( activeqt )
	sizeHint = activeqt->sizeHint();
    if ( sizeHint.isValid() ) {
	QPaintDeviceMetrics pmetric( activeqt );

	sizeExtent.cx = MAP_PIX_TO_LOGHIM( sizeHint.width(), pmetric.logicalDpiX() );
	sizeExtent.cy = MAP_PIX_TO_LOGHIM( sizeHint.height(), pmetric.logicalDpiY() );
    }
}

static bool checkHRESULT( HRESULT hres )
{
    const char *name = 0;
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
#if defined(QT_CHECK_STATE)
	    qWarning( "QAxBase: Error calling IDispatch member %s: Exception thrown by server.", name );
#endif
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
	qWarning( "QAxBase: Error calling IDispatch member %s: Parameter not found.", name );
#endif
	return FALSE;
    case DISP_E_TYPEMISMATCH:
#if defined(QT_CHECK_STATE)
	qWarning( "QAxBase: Error calling IDispatch member %s: Type mismatch.", name );
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
    Catches all signals emitted by the Qt widget and fires the respective COM event.

    \a isignal is the Qt Meta Object index of the received signal, and \a _o the
    signal parameters.
*/
bool QAxServerBase::qt_emit( int isignal, QUObject* _o )
{
    if ( isignal == -1 && sender() && m_spInPlaceSite ) {
	if ( (QStatusBar*)((QObject*)sender())->qt_cast( "QStatusBar" ) != statusBar )
	    return TRUE;

	QString message = static_QUType_QString.get( _o+1 );
	m_spInPlaceFrame->SetStatusText( QStringToBSTR(message) );

	return TRUE;
    }

    if ( freezeEvents )
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
    IConnectionPoint *cpoint = 0;
    FindConnectionPoint( qAxFactory()->eventsID( class_name ), &cpoint );
    if ( cpoint ) {
	IEnumConnections *clist = 0;
	cpoint->EnumConnections( &clist );
	if ( clist ) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next( cc, (CONNECTDATA*)&c, &cc );
	    if ( cc ) {
		// setup parameters
		unsigned int argErr = 0;
		DISPPARAMS dispParams;
		dispParams.cArgs = signalcount;
		dispParams.cNamedArgs = 0;
		dispParams.rgdispidNamedArgs = 0;
		dispParams.rgvarg = signalcount ? new VARIANTARG[signalcount] : 0;
		int p;
		for ( p = 0; p < signalcount; ++p ) {
		    QUObject *obj = _o + p + 1;
		    VARIANT *arg = dispParams.rgvarg + (signalcount - p - 1);
		    VariantInit( arg );

		    const QUParameter *param = signal ? signal->method->parameters + p : 0;
		    QUObjectToVARIANT( obj, *arg, param );
		}
		// call listeners (through IDispatch)
		GUID IID_QAxEvents = qAxFactory()->eventsID( class_name );
		while ( cc ) {
		    if ( c->pUnk ) {
			IDispatch *disp = 0;
			c->pUnk->QueryInterface( IID_QAxEvents, (void**)&disp );
			if ( disp ) {
			    disp->Invoke( eventId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispParams, 0, 0, &argErr );
			    if ( signal && signal->method ) for ( p = 0; p < signalcount; ++p ) {
				const QUParameter *param = signal->method->parameters + p;
				if ( param->inOut & QUParameter::Out ) {
				    QUObject *obj = _o + p + 1;
				    if ( obj->type )
					obj->type->clear( obj );
				    VARIANTToQUObject( dispParams.rgvarg[ signalcount - p - 1 ], obj, param );
				}
			    }
			    disp->Release();
			}
			c->pUnk->Release(); // AddRef'ed by clist->Next implementation
		    }
		    clist->Next( cc, (CONNECTDATA*)&c, &cc );
		}

		// clean up
		for ( p = 0; p < signalcount; ++p )
		    clearVARIANT( dispParams.rgvarg+p );
		delete [] dispParams.rgvarg;
	    }
	    clist->Release();
	}
	cpoint->Release();
    }
    return TRUE;
}

/*!
    Call IPropertyNotifySink of connected clients.
    \a dispId specifies the ID of the property that changed.
*/
bool QAxServerBase::emitRequestPropertyChange( const char *property, long dispId )
{
    IConnectionPoint *cpoint = 0;
    FindConnectionPoint( IID_IPropertyNotifySink, &cpoint );
    if ( cpoint ) {
	IEnumConnections *clist = 0;
	cpoint->EnumConnections( &clist );
	if ( clist ) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next( cc, (CONNECTDATA*)&c, &cc );
	    if ( cc ) {
		if ( dispId == -1 ) {
		    if ( !proplist )
			readMetaData();

		    QIntDictIterator <QMetaProperty> it( *proplist );
		    while ( it.current() && dispId < 0 ) {
			QMetaProperty *mp = it.current();
			if ( !qstrcmp( property, mp->name() ) )
			    dispId = it.currentKey();
			++it;
		    }
		}
		if ( dispId != -1 ) while ( cc ) {
		    if ( c->pUnk ) {
			IPropertyNotifySink *sink = 0;
			c->pUnk->QueryInterface( IID_IPropertyNotifySink, (void**)&sink );
			bool disallows = sink && sink->OnRequestEdit( dispId ) == S_FALSE;
			sink->Release();
			c->pUnk->Release();
			if ( disallows ) { // a client disallows the property to change
			    clist->Release();
			    cpoint->Release();
			    return FALSE;
			}
		    }
		    clist->Next( cc, (CONNECTDATA*)&c, &cc );
		}
	    }
	    clist->Release();
	}
	cpoint->Release();
    }
    return TRUE;
}

/*!
    Call IPropertyNotifySink of connected clients.
    \a dispId specifies the ID of the property that changed.
*/
void QAxServerBase::emitPropertyChanged( const char *property, long dispId )
{
    IConnectionPoint *cpoint = 0;
    FindConnectionPoint( IID_IPropertyNotifySink, &cpoint );
    if ( cpoint ) {
	IEnumConnections *clist = 0;
	cpoint->EnumConnections( &clist );
	if ( clist ) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next( cc, (CONNECTDATA*)&c, &cc );
	    if ( cc ) {
		if ( dispId == -1 ) {
		    if ( !proplist )
			readMetaData();
		    QIntDictIterator <QMetaProperty> it( *proplist );
		    while ( it.current() && dispId < 0 ) {
			QMetaProperty *mp = it.current();
			if ( !qstrcmp( property, mp->name() ) )
			    dispId = it.currentKey();
			++it;
		    }
		}
		if ( dispId != -1 ) while ( cc ) {
		    if ( c->pUnk ) {
			IPropertyNotifySink *sink = 0;
			c->pUnk->QueryInterface( IID_IPropertyNotifySink, (void**)&sink );
			if ( sink ) {
			    sink->OnChanged( dispId );
			    sink->Release();
			}
			c->pUnk->Release();
		    }
		    clist->Next( cc, (CONNECTDATA*)&c, &cc );
		}
	    }
	    clist->Release();
	}
	cpoint->Release();
    }
}

//**** IProvideClassInfo
/*
    Provide the ITypeInfo implementation for the COM class.
*/
HRESULT WINAPI QAxServerBase::GetClassInfo(ITypeInfo** pptinfo)
{
    if ( !pptinfo )
	return E_POINTER;

    if ( !qAxTypeLibrary )
	return DISP_E_BADINDEX;

    return qAxTypeLibrary->GetTypeInfoOfGuid( qAxFactory()->classID( class_name ), pptinfo );
}

//**** IProvideClassInfo2
/*
    Provide the ID of the event interface.
*/
HRESULT WINAPI QAxServerBase::GetGUID(DWORD dwGuidKind, GUID* pGUID)
{
    if ( !pGUID )
	return E_POINTER;

    if ( dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID ) {
	*pGUID = qAxFactory()->eventsID( class_name );
	return S_OK;
    }
    *pGUID = GUID_NULL;
    return E_FAIL;
}

//**** IDispatch
/*
    Returns the number of class infos for this IDispatch.
*/
HRESULT WINAPI QAxServerBase::GetTypeInfoCount(UINT* pctinfo)
{
    if ( !pctinfo )
	return E_POINTER;

    *pctinfo = qAxTypeLibrary ? 1 : 0;
    return S_OK;
}

/*
    Provides the ITypeInfo for this IDispatch implementation.
*/
HRESULT WINAPI QAxServerBase::GetTypeInfo(UINT itinfo, LCID /*lcid*/, ITypeInfo** pptinfo)
{
    if ( !pptinfo )
	return E_POINTER;

    if ( !qAxTypeLibrary )
	return DISP_E_BADINDEX;

    if ( m_spTypeInfo ) {
	*pptinfo = m_spTypeInfo;
	(*pptinfo)->AddRef();
	return S_OK;
    }

    HRESULT res = qAxTypeLibrary->GetTypeInfoOfGuid( qAxFactory()->interfaceID( class_name ), pptinfo );
    m_spTypeInfo = *pptinfo;
    m_spTypeInfo->AddRef();

    return res;
}

/*
    Provides the names of the methods implemented in this IDispatch implementation.
*/
HRESULT WINAPI QAxServerBase::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
				     LCID /*lcid*/, DISPID* rgdispid)
{
    if ( !rgszNames || !rgdispid )
	return E_POINTER;

    if ( !qAxTypeLibrary )
	return DISP_E_UNKNOWNNAME;

    if ( !m_spTypeInfo )
	qAxTypeLibrary->GetTypeInfoOfGuid( qAxFactory()->interfaceID( class_name ), &m_spTypeInfo );

    if ( !m_spTypeInfo )
	return DISP_E_UNKNOWNNAME;

    return m_spTypeInfo->GetIDsOfNames( rgszNames, cNames, rgdispid );
}

/*
    Map the COM call to the Qt slot/property for \a dispidMember.
*/
HRESULT WINAPI QAxServerBase::Invoke( DISPID dispidMember, REFIID riid,
		  LCID /*lcid*/, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pvarResult,
		  EXCEPINFO* /*pexcepinfo*/, UINT* puArgErr )
{
    if ( riid != IID_NULL )
	return DISP_E_UNKNOWNINTERFACE;
    if ( !activeqt )
	return E_UNEXPECTED;

    HRESULT res = DISP_E_MEMBERNOTFOUND;

    if ( !slotlist )
	readMetaData();

    QSize oldSizeHint = activeqt->sizeHint();

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
	    const QUParameter *params = slot->method->parameters;
	    int pcount = slot->method->count;
	    int retoff = 0;
	    if ( pcount ) {
		retoff = ( params[0].inOut == QUParameter::Out ) ? 1 : 0;
		pcount -= retoff;
	    }
	    if ( retoff && !pvarResult )
		return DISP_E_PARAMNOTOPTIONAL;
	    int argcount = pDispParams->cArgs;
	    if ( pcount > argcount )
		return DISP_E_PARAMNOTOPTIONAL;
	    else if ( pcount < argcount )
		return DISP_E_BADPARAMCOUNT;

	    // setup parameters
	    bool ok = TRUE;
	    QUObject *objects = 0;
	    if ( pcount ) {
		objects = new QUObject[pcount+1];
		for ( int p = 0; p < pcount; ++p ) {
		    // map the VARIANT to the QUObject, and try to get the required type
		    objects[p+1].payload.ptr = 0;
		    if ( !VARIANTToQUObject( pDispParams->rgvarg[ pcount-p-1 ], objects + p + 1, params + p + retoff ) ) {
			if ( puArgErr )
			    *puArgErr = pcount-p-1;
			ok = FALSE;
		    }
		}
	    } else if ( retoff ) {
		objects = new QUObject[1];
	    }
	    if ( objects )
		objects[0].payload.ptr = 0;

	    // call the slot if everthing went fine.
	    if ( ok ) {
		activeqt->qt_invoke( index, objects );

		// update reference parameters and value
		for ( int p = 0; p < pcount; ++p ) {
		    const QUParameter *param = params + p + (retoff ? 1 : 0 );
		    if ( param->inOut & QUParameter::Out ) {
			if ( !QUObjectToVARIANT( objects+p+1, pDispParams->rgvarg[ pcount-p-1 ], param ) )
			    ok = FALSE;
		    }
		    clearQUObject( objects+p+1, param );
		}
		if ( retoff ) {
		    QUObjectToVARIANT( objects, *pvarResult, params );
		    clearQUObject( objects, params );
		}
	    }

	    delete [] objects;
	    res = ok ? S_OK : DISP_E_TYPEMISMATCH;
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

	    if ( isBindable && !emitRequestPropertyChange( property->name(), dispidMember ) )
		break;

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
	    
	    if ( isBindable )
		emitPropertyChanged( property->name(), dispidMember );
	    res = S_OK;

	    if ( m_spAdviseSink )
		m_spAdviseSink->OnViewChange( DVASPECT_CONTENT, 0 );
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
		res =  DISP_E_MEMBERNOTFOUND;
	    else if ( !QVariantToVARIANT( var, *pvarResult, property->type() ) )
		res = DISP_E_TYPEMISMATCH;
	    else
		res = S_OK;
	}
	break;
    default:
	break;
    }

    QSize sizeHint = activeqt->sizeHint();
    if ( oldSizeHint != sizeHint ) {
	updateGeometry();
	if ( m_spInPlaceSite ) {
	    RECT rect;
	    rect.left = rcPos.left;
	    rect.right = rcPos.left + sizeHint.width();
	    rect.top = rcPos.top;
	    rect.bottom = rcPos.top + sizeHint.height();
	    m_spInPlaceSite->OnPosRectChange( &rect );
	}
    }

    return res;
}

//**** IConnectionPointContainer
/*
    Provide the IEnumConnectionPoints implemented in the QAxSignalVec class.
*/
HRESULT WINAPI QAxServerBase::EnumConnectionPoints( IEnumConnectionPoints **epoints )
{
    if ( !epoints )
	return E_POINTER;
    *epoints = new QAxSignalVec( points );
    (*epoints)->AddRef();
    return S_OK;
}

/*
    Provide the IConnectionPoint implemented in the QAxConnection for \a iid.
*/
HRESULT WINAPI QAxServerBase::FindConnectionPoint( REFIID iid, IConnectionPoint **cpoint )
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
/*
    Initialize the properties of the Qt widget.
*/
HRESULT WINAPI QAxServerBase::InitNew()
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

/*
    Set the properties of the Qt widget to the values provided in the \a bag.
*/
HRESULT WINAPI QAxServerBase::Load( IPropertyBag *bag, IErrorLog * /*log*/ )
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

/*
    Save the properties of the Qt widget into the \a bag.
*/
HRESULT WINAPI QAxServerBase::Save( IPropertyBag *bag, BOOL /*clearDirty*/, BOOL /*saveAll*/ )
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
	const QMetaProperty *property = mo->property( prop, TRUE );
	BSTR bstr = QStringToBSTR( property->name() );
	QVariant qvar;
	if ( !activeqt->qt_property( prop, 1, &qvar ) )
	    error = TRUE;
	VARIANT var;
	QVariantToVARIANT( qvar, var, property->type() );
	bag->Write( bstr, &var );
	SysFreeString(bstr);
    }
    return error ? E_FAIL : S_OK;
}

//**** IPersistStorage
/*
    \reimp

    See documentation of IPersistStorage::IsDirty.
*/
HRESULT WINAPI QAxServerBase::IsDirty()
{
    return dirtyflag ? S_OK : S_FALSE;
}

HRESULT WINAPI QAxServerBase::InitNew(IStorage *pStg )
{
    if ( initNewCalled )
	return CO_E_ALREADYINITIALIZED;

    dirtyflag = FALSE;
    initNewCalled = TRUE;

    m_spStorage = pStg;
    if ( m_spStorage )
	m_spStorage->AddRef();
    return S_OK;
}

HRESULT WINAPI QAxServerBase::Load(IStorage *pStg )
{
    if ( initNewCalled )
	return CO_E_ALREADYINITIALIZED;

    IStream *spStream = 0;
    HRESULT hres = pStg->OpenStream( L"SomeStreamName", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &spStream );
    if ( !spStream )
	return E_FAIL;

    STATSTG stat;
    hres = spStream->Stat( &stat, STATFLAG_NONAME );
    QByteArray qtarray;
    if ( hres != S_OK ) {
    } else {
	ULONG read;
	if ( stat.cbSize.HighPart )
	    return S_FALSE;
	qtarray.resize( stat.cbSize.LowPart );
	spStream->Read( qtarray.data(), stat.cbSize.LowPart, &read );
    }

    readMetaData();

    QBuffer qtbuffer( qtarray );
    qtbuffer.open( IO_ReadOnly | IO_Translate );
    QDataStream qtstream( &qtbuffer );
    while ( !qtbuffer.atEnd() ) {
	QCString property;
	QVariant value;
	qtstream >> property;
	qtstream >> value;

	activeqt->setProperty( property, value );
    }

    spStream->Release();

    dirtyflag = FALSE;
    initNewCalled = TRUE;

    m_spStorage = pStg;
    if ( m_spStorage )
	m_spStorage->AddRef();

    return S_OK;
}

HRESULT WINAPI QAxServerBase::Save(IStorage *pStg, BOOL fSameAsLoad )
{
    IStream *spStream = 0;
    HRESULT hres = pStg->CreateStream( L"SomeStreamName", STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStream );
    if ( !spStream )
	return E_FAIL;

    QBuffer qtbuffer;
    qtbuffer.open( IO_WriteOnly | IO_Translate );
    QDataStream qtstream( &qtbuffer );
    
    readMetaData();

    const QMetaObject *mo = activeqt->metaObject();
    for ( int prop = 0; prop < mo->numProperties( TRUE ); ++prop ) {
	if ( !proplist2->contains( prop ) )
	    continue;
	QCString property = mo->property( prop, TRUE )->name();
	QVariant qvar = activeqt->property( property );
	if ( qvar.isValid() ) {
	    qtstream << property;
	    qtstream << qvar;
	}
    }

    qtbuffer.close();
    QByteArray qtarray = qtbuffer.buffer();
    ULONG written = 0;
    char *data = qtarray.data();
    spStream->Write( data, qtarray.size(), &written );
    spStream->Commit( STGC_DEFAULT );

    spStream->Release();
    return S_OK;
}

HRESULT WINAPI QAxServerBase::SaveCompleted( IStorage *pStgNew )
{
    if ( pStgNew ) {
	if ( m_spStorage )
	    m_spStorage->Release();
	m_spStorage = pStgNew;
	m_spStorage->AddRef();
    }
    return S_OK;
}

HRESULT WINAPI QAxServerBase::HandsOffStorage()
{
    if ( m_spStorage ) m_spStorage->Release();
    m_spStorage = 0;

    return S_OK;
}


//**** IViewObject
class HackPainter : public QPainter
{
    friend class QAxServerBase;
};

/*
    Draws the widget into the provided device context.
*/
HRESULT WINAPI QAxServerBase::Draw( DWORD dwAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		HDC hicTargetDev, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
		BOOL(__stdcall* /*pfnContinue*/)(DWORD), DWORD /*dwContinue*/ )
{
    if ( !lprcBounds )
	return E_INVALIDARG;

    internalCreate();

    switch ( dwAspect ) {
    case DVASPECT_CONTENT:
    case DVASPECT_OPAQUE:
    case DVASPECT_TRANSPARENT:
	break;
    default:
	return DV_E_DVASPECT;
    }
    if (!ptd)
	hicTargetDev = 0;

    bool bDeleteDC = FALSE;
    if ( !hicTargetDev ) {
	hicTargetDev = ::CreateDCA("DISPLAY", NULL, NULL, NULL);
	bDeleteDC = (hicTargetDev != hdcDraw);
    }

    RECTL rectBoundsDP = *lprcBounds;
    bool bMetaFile = GetDeviceCaps(hdcDraw, TECHNOLOGY) == DT_METAFILE;
    if ( !bMetaFile  ) {
	::LPtoDP(hicTargetDev, (LPPOINT)&rectBoundsDP, 2);
	SaveDC(hdcDraw);
	SetMapMode(hdcDraw, MM_TEXT);
	SetWindowOrgEx(hdcDraw, 0, 0, 0);
	SetViewportOrgEx(hdcDraw, 0, 0, 0);
    }
    lprcBounds = &rectBoundsDP;
    RECTL rc = *lprcBounds;

    activeqt->resize( rc.right - rc.left, rc.bottom - rc.top );
    QPixmap pm = QPixmap::grabWidget( activeqt );
    BOOL res = ::BitBlt( hdcDraw, 0, 0, pm.width(), pm.height(), pm.handle(), 0, 0, SRCCOPY );

    if ( !res ) {
	QPainter painter( activeqt );
	HDC oldDC = ((HackPainter*)&painter)->hdc;
	((HackPainter*)&painter)->hdc = hdcDraw;

	painter.drawText( rc.left, rc.top, "I don't know how to draw myself!" );

	((HackPainter*)&painter)->hdc = oldDC;
    }

    if ( bDeleteDC )
	DeleteDC( hicTargetDev );
    if (!bMetaFile)
	RestoreDC( hdcDraw, -1 );

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetColorSet( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		HDC hicTargetDev, LOGPALETTE **ppColorSet )
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::Freeze( DWORD dwAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze )
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::Unfreeze( DWORD dwFreeze )
{
    return E_NOTIMPL;
}

/*
    Stores the provided advise sink.
*/
HRESULT WINAPI QAxServerBase::SetAdvise( DWORD /*aspects*/, DWORD /*advf*/, IAdviseSink *pAdvSink )
{
    if ( !pAdvSink && m_spAdviseSink )
	m_spAdviseSink->Release();

    m_spAdviseSink = pAdvSink;
    if ( m_spAdviseSink )
	m_spAdviseSink->AddRef();
    return S_OK;
}

/*
    Returns the advise sink.
*/
HRESULT WINAPI QAxServerBase::GetAdvise( DWORD* /*aspects*/, DWORD* /*advf*/, IAdviseSink **ppAdvSink )
{
    if ( !ppAdvSink )
	return E_POINTER;

    *ppAdvSink = m_spAdviseSink;
    if ( *ppAdvSink )
	(*ppAdvSink)->AddRef();
    return S_OK;
}

//**** IViewObject2
/*
    Returns the current size.
*/
HRESULT WINAPI QAxServerBase::GetExtent( DWORD /*dwAspect*/, LONG /*lindex*/, DVTARGETDEVICE* /*ptd*/, LPSIZEL lpsizel )
{
    updateGeometry();
    *lpsizel = sizeExtent;
    return S_OK;
}

//**** IOleControl
/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetControlInfo( LPCONTROLINFO )
{
    return E_NOTIMPL;
}

/*
    Turns event firing on and off.
*/
HRESULT WINAPI QAxServerBase::FreezeEvents( BOOL bFreeze )
{
    // member of CComControl
    if ( bFreeze )
	freezeEvents++;
    else
	freezeEvents--;

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::OnMnemonic( LPMSG )
{
    return E_NOTIMPL;
}

/*
    Update the ambient properties of the Qt widget.
*/
HRESULT WINAPI QAxServerBase::OnAmbientPropertyChange( DISPID dispID )
{
    if ( !m_spClientSite || !activeqt )
	return S_OK;

    IDispatch *disp = 0;
    m_spClientSite->QueryInterface( IID_IDispatch, (void**)&disp );
    if ( !disp )
	return S_OK;

    VARIANT var;
    DISPPARAMS params = { 0, 0, 0, 0 };
    disp->Invoke( dispID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &var, 0, 0 );
    disp->Release();
    disp = 0;

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
	    IFont *f = 0;
	    d->QueryInterface( IID_IFont, (void**)&f );
	    if ( f ) {
		QFont qfont = IFontToQFont( f );
		activeqt->setFont( qfont );
		f->Release();
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

//**** IOleWindow
/*
    Returns the HWND of the control.
*/
HRESULT WINAPI QAxServerBase::GetWindow( HWND *pHwnd )
{
    if ( !pHwnd )
	return E_POINTER;
    *pHwnd = m_hWnd;
    return S_OK;
}

/*
    Enters What's This mode.
*/
HRESULT WINAPI QAxServerBase::ContextSensitiveHelp( BOOL fEnterMode )
{
    if ( fEnterMode )
	QWhatsThis::enterWhatsThisMode();
    else
	QWhatsThis::leaveWhatsThisMode();
    return S_OK;
}

//**** IOleInPlaceObject
/*
    Deactivates the control in place.
*/
HRESULT WINAPI QAxServerBase::InPlaceDeactivate()
{
    if ( !isInPlaceActive )
	return S_OK;
    UIDeactivate();

    isInPlaceActive = FALSE;

    // if we have a window, tell it to go away.
    if (m_hWndCD) {
	if (::IsWindow(m_hWndCD))
	    ::DestroyWindow(m_hWndCD);
	m_hWndCD = 0;
    }

    if (m_spInPlaceSite)
	m_spInPlaceSite->OnInPlaceDeactivate();

    return S_OK;
}

/*
    Deactivates the control's user interface.
*/
HRESULT WINAPI QAxServerBase::UIDeactivate()
{
    // if we're not UIActive, not much to do.
    if (!isUIActive)
	return S_OK;

    isUIActive = FALSE;

    // notify frame windows, if appropriate, that we're no longer ui-active.
    OLEINPLACEFRAMEINFO frameInfo;
    frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
    RECT rcPos, rcClip;

    HWND hwndParent;
    if (m_spInPlaceSite->GetWindow(&hwndParent) == S_OK) {
	if ( m_spInPlaceFrame ) m_spInPlaceFrame->Release();
	m_spInPlaceFrame = 0;
	IOleInPlaceUIWindow *spInPlaceUIWindow = 0;

	m_spInPlaceSite->GetWindowContext(&m_spInPlaceFrame, &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);
	if ( spInPlaceUIWindow ) {
	    spInPlaceUIWindow->SetActiveObject(0, 0);
	    spInPlaceUIWindow->Release();
	}
	if ( m_spInPlaceFrame ) {
	    m_spInPlaceFrame->RemoveMenus( hmenuShared );
	    holemenu = 0;
	    m_spInPlaceFrame->SetMenu( 0, 0, m_hWnd );
	    if ( hmenuShared ) {
		DestroyMenu( hmenuShared );
		menuMap.clear();
	    }
	    hwndMenuOwner = 0;
	    menuBar = 0;
	    statusBar = 0;
	    m_spInPlaceFrame->SetActiveObject(0, 0);
	    m_spInPlaceFrame->Release();
	    m_spInPlaceFrame = 0;
	}
    }
    // we don't need to explicitly release the focus here since somebody
    // else grabbing the focus is what is likely to cause us to get lose it
    m_spInPlaceSite->OnUIDeactivate(FALSE);

    return S_OK;
}

/*
    Positions the control, and applies requested clipping.
*/
HRESULT WINAPI QAxServerBase::SetObjectRects(LPCRECT prcPos, LPCRECT prcClip)
{
    if ( prcPos == 0 || prcClip == 0 )
	return E_POINTER;

    rcPos = *prcPos;
    if (m_hWndCD) {
	// the container wants us to clip, so figure out if we really need to
	RECT rcIXect;
	BOOL b = IntersectRect(&rcIXect, prcPos, prcClip);
	HRGN tempRgn = 0;
	if (b && !EqualRect(&rcIXect, prcPos)) {
	    OffsetRect(&rcIXect, -(prcPos->left), -(prcPos->top));
	    tempRgn = CreateRectRgnIndirect(&rcIXect);
	}

	::SetWindowRgn(m_hWndCD, tempRgn, TRUE);
	::SetWindowPos(m_hWndCD, 0, prcPos->left,
	    prcPos->top, prcPos->right - prcPos->left, prcPos->bottom - prcPos->top,
	    SWP_NOZORDER | SWP_NOACTIVATE);
    }

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::ReactivateAndUndo()
{
    return E_NOTIMPL;
}

//**** IOleInPlaceActiveObject

int QAxEventFilter( MSG *pMsg )
{
    if ( !ax_ServerMapper )
	return 0;
    if ( pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST )
	return 0;

    int ret = 0;
    QWidget *aqt = QWidget::find( pMsg->hwnd );
    if ( !aqt )
	return ret;

    HWND baseHwnd = ::GetParent( aqt->winId() );
    QAxServerBase *axbase = 0;
    while ( !axbase && baseHwnd ) {
	axbase = axServerMapper()->find( baseHwnd );
	baseHwnd = ::GetParent( baseHwnd );
    }
    if ( !axbase )
	return ret;

    return axbase->PreTranslateMessage( pMsg );
}

int QAxServerBase::PreTranslateMessage( MSG *pMsg )
{
    if ( TranslateAccelerator( pMsg ) == S_OK )
	return TRUE;

    if ( !m_spClientSite )
	return FALSE;

    IOleControlSite *controlSite = 0;
    m_spClientSite->QueryInterface( IID_IOleControlSite, (void**)&controlSite );
    if ( !controlSite )
	return FALSE;

    DWORD dwKeyMod = 0;
    if (::GetKeyState(VK_SHIFT) < 0)
	dwKeyMod += 1;	// KEYMOD_SHIFT
    if (::GetKeyState(VK_CONTROL) < 0)
	dwKeyMod += 2;	// KEYMOD_CONTROL
    if (::GetKeyState(VK_MENU) < 0)
	dwKeyMod += 4;	// KEYMOD_ALT
    controlSite->TranslateAccelerator(pMsg, dwKeyMod);
    controlSite->Release();

    return FALSE;
}

HRESULT WINAPI QAxServerBase::TranslateAccelerator( MSG *pMsg )
{
    if ( pMsg->message != WM_KEYDOWN )
	return S_FALSE;

    switch ( LOWORD( pMsg->wParam ) ) {
    case VK_TAB:
	{
	    if ( isUIActive ) {
		bool shift = ::GetKeyState(VK_SHIFT) < 0;
		QFocusData *data = ((HackWidget*)activeqt)->focusData();
		bool giveUp = TRUE;
		if ( shift ) {
		    if ( activeqt->focusWidget() != data->first() ) {
			giveUp = FALSE;
			((HackWidget*)activeqt)->focusNextPrevChild( FALSE );
		    }
		} else {
		    if ( activeqt->focusWidget() != data->last() ) {
			giveUp = FALSE;
			((HackWidget*)activeqt)->focusNextPrevChild( TRUE );
		    }
		}
		if ( giveUp ) {
/*		    IOleControlSite *spSite = 0;
		    m_spClientSite->QueryInterface( IID_IOleControlSite, (void**)&spSite );
		    if ( spSite ) {
			spSite->OnFocus(FALSE);
			spSite->Release();
		    }*/
		} else {
		    return S_OK;
		}
	    }
	}
	return S_FALSE;
    }
    return S_FALSE;
}

HRESULT WINAPI QAxServerBase::OnFrameWindowActivate( BOOL fActivate )
{
    if ( fActivate ) {
	if ( wasUIActive )
	    ::SetFocus( m_hWnd );
    } else {
	wasUIActive = isUIActive;
    }
    return S_OK;
}

HRESULT WINAPI QAxServerBase::OnDocWindowActivate( BOOL fActivate )
{
    return S_OK;
}

HRESULT WINAPI QAxServerBase::ResizeBorder( LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow )
{
    return S_OK;
}

HRESULT WINAPI QAxServerBase::EnableModeless( BOOL fEnable )
{
    return S_OK;
}

//**** IOleObject

static inline LPOLESTR QStringToOLESTR( const QString &qstring )
{
    LPOLESTR olestr = (wchar_t*)CoTaskMemAlloc(qstring.length()*2+2);
    memcpy( olestr, (ushort*)qstring.unicode(), qstring.length()*2 );
    olestr[qstring.length()] = 0;
    return olestr;
}

/*
    \reimp

    See documentation of IOleObject::GetUserType.
*/
HRESULT WINAPI QAxServerBase::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
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

/*
    Returns the status flags registered for this control.
*/
HRESULT WINAPI QAxServerBase::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    return OleRegGetMiscStatus( qAxFactory()->classID( class_name ), dwAspect, pdwStatus);
}

/*
    Stores the provided advise sink.
*/
HRESULT WINAPI QAxServerBase::Advise( IAdviseSink* pAdvSink, DWORD* pdwConnection )
{
    HRESULT hr = S_OK;
    if ( !m_spOleAdviseHolder )
	hr = CreateOleAdviseHolder( &m_spOleAdviseHolder );
    if (SUCCEEDED(hr))
	hr = m_spOleAdviseHolder->Advise( pAdvSink, pdwConnection );
    return hr;
}

/*
    Closes the control.
*/
HRESULT WINAPI QAxServerBase::Close( DWORD dwSaveOption )
{
    if ( m_hWndCD ) {
	if ( m_spClientSite )
	    m_spClientSite->OnShowWindow( FALSE );
    }
    if ( isInPlaceActive ) {
	HRESULT hr = InPlaceDeactivate();
	if (FAILED(hr))
	    return hr;
    }
    if ( m_hWndCD ) {
	if ( IsWindow(m_hWndCD) )
	    DestroyWindow(m_hWndCD);
	m_hWndCD = 0;
    }

    if ( m_spInPlaceSite ) m_spInPlaceSite->Release();
    m_spInPlaceSite = 0;
    if ( m_spAdviseSink ) m_spAdviseSink->Release();
    m_spAdviseSink = 0;
    return S_OK;
}

/*
    Executes the steps to activate the control.
*/
HRESULT QAxServerBase::internalActivate()
{
    HRESULT hr;

    if ( !m_spClientSite )
	return S_OK;

    if ( !m_spInPlaceSite )
	hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_spInPlaceSite);

    if ( !m_spInPlaceSite )
	return E_FAIL;

    if ( !isInPlaceActive ) {
	BOOL bNoRedraw = FALSE;
	hr = m_spInPlaceSite->CanInPlaceActivate();
	if (FAILED(hr)) // CanInPlaceActivate returns anything but S_FALSE or S_OK
	    return hr;
	if ( hr != S_OK ) // CanInPlaceActivate returned S_FALSE.
	    return E_FAIL;
	m_spInPlaceSite->OnInPlaceActivate();
    }

    isInPlaceActive = TRUE;

    // get location in the parent window,
    // as well as some information about the parent
    OLEINPLACEFRAMEINFO frameInfo;
    RECT rcPos, rcClip;
    if ( m_spInPlaceFrame ) m_spInPlaceFrame->Release();
    m_spInPlaceFrame = 0;
    IOleInPlaceUIWindow *spInPlaceUIWindow = 0;
    frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
    HWND hwndParent;
    if ( m_spInPlaceSite->GetWindow(&hwndParent) == S_OK ) {
	m_spInPlaceSite->GetWindowContext(&m_spInPlaceFrame, &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);

	if (m_hWndCD) {
	    ::ShowWindow(m_hWndCD, SW_SHOW);
	    if (!::IsChild(m_hWndCD, ::GetFocus()) && activeqt->focusPolicy() != QWidget::NoFocus )
		::SetFocus(m_hWndCD);
	} else {
	    create(hwndParent, rcPos);
	}

	SetObjectRects(&rcPos, &rcClip);
    }

    // Gone active by now, take care of UIACTIVATE
    bool canTakeFocus = activeqt->focusPolicy() != QWidget::NoFocus;
    if ( !canTakeFocus ) {
	QObjectList *list = activeqt->queryList();
	if ( list ) {
	    QObjectListIt it( *list );
	    QObject *o = 0;
	    while ( ( o = it.current() ) && !canTakeFocus ) {
		++it;
		if ( !o->isWidgetType() )
		    continue;
		QWidget *w = (QWidget*)o;
		canTakeFocus = w->focusPolicy() != QWidget::NoFocus;		    
	    }
	    delete list;
	}
    }
    if ( !isUIActive && canTakeFocus ) {
	isUIActive = TRUE;
	hr = m_spInPlaceSite->OnUIActivate();
	if ( FAILED(hr) ) {
	    if ( m_spInPlaceFrame ) m_spInPlaceFrame->Release();
	    m_spInPlaceFrame = 0;
	    if ( spInPlaceUIWindow ) spInPlaceUIWindow->Release();
	    return hr;
	}

	if ( isInPlaceActive ) {
	    HWND hwnd = m_hWndCD;
	    if ( !::IsChild( hwnd, ::GetFocus() ) )
		::SetFocus( hwnd );
	}

	if ( m_spInPlaceFrame ) {
	    hr = m_spInPlaceFrame->SetActiveObject( this, QStringToBSTR(class_name) );
	    if ( !FAILED(hr) ) {
		menuBar = activeqt ? (QMenuBar*)activeqt->child( 0, "QMenuBar" ) : 0;
		if ( menuBar ) {
		    createMenu( menuBar );
		    menuBar->hide();
		}
		statusBar = activeqt ? (QStatusBar*)activeqt->child( 0, "QStatusBar" ) : 0;
		if ( statusBar ) {
		    const int index = statusBar->metaObject()->findSignal( "messageChanged(const QString&)" );
		    connectInternal( statusBar, index, (QObject*)this, 2, -1 );
		    statusBar->hide();
		}
	    }
	}
	if ( spInPlaceUIWindow ) {
	    spInPlaceUIWindow->SetActiveObject( this, QStringToBSTR(class_name) );
	    spInPlaceUIWindow->SetBorderSpace(0);
	    spInPlaceUIWindow->Release();
	}
    }

    m_spClientSite->ShowObject();

    return S_OK;
}

/*
    Executes the "verb" \a iVerb.
*/
HRESULT WINAPI QAxServerBase::DoVerb( LONG iVerb, LPMSG /*lpmsg*/, IOleClientSite* /*pActiveSite*/, LONG /*lindex*/,
			       HWND /*hwndParent*/, LPCRECT /*prcPosRect*/ )
{
    HRESULT hr = E_NOTIMPL;
    switch (iVerb)
    {
    case OLEIVERB_SHOW:
	hr = internalActivate();
	if (SUCCEEDED(hr))
	    hr = S_OK;
	break;

    case OLEIVERB_PRIMARY:
    case OLEIVERB_INPLACEACTIVATE:
	hr = internalActivate();
	if (SUCCEEDED(hr)) {
	    hr = S_OK;
	    update();
	}
	break;

    case OLEIVERB_UIACTIVATE:
	if ( !isUIActive ) {
	    hr = internalActivate();
	    if (SUCCEEDED(hr))
		hr = S_OK;
	}
	break;

    case OLEIVERB_HIDE:
	UIDeactivate();
	if ( m_hWnd )
	    ::ShowWindow( m_hWnd, SW_HIDE );
	hr = S_OK;
	return hr;

    default:
	break;
    }
    return hr;
}

/*
    Returns the list of advise connections.
*/
HRESULT WINAPI QAxServerBase::EnumAdvise( IEnumSTATDATA** ppenumAdvise )
{
    HRESULT hRes = E_FAIL;
    if ( m_spOleAdviseHolder )
	hRes = m_spOleAdviseHolder->EnumAdvise(ppenumAdvise);
    return hRes;
}

/*
    Returns an enumerator for the verbs registered for this class.
*/
HRESULT WINAPI QAxServerBase::EnumVerbs( IEnumOLEVERB** ppEnumOleVerb )
{
    if ( !ppEnumOleVerb )
	return E_POINTER;
    return OleRegEnumVerbs(qAxFactory()->classID( class_name ), ppEnumOleVerb);
}

/*
    Returns the current client site..
*/
HRESULT WINAPI QAxServerBase::GetClientSite( IOleClientSite** ppClientSite )
{
    if ( !ppClientSite )
	return E_POINTER;
    *ppClientSite = m_spClientSite;
    if ( *ppClientSite )
	(*ppClientSite)->AddRef();
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetClipboardData( DWORD, IDataObject** )
{
    return E_NOTIMPL;
}

/*
    Returns the current size.
*/
HRESULT WINAPI QAxServerBase::GetExtent( DWORD dwDrawAspect, SIZEL* psizel )
{
    if ( dwDrawAspect != DVASPECT_CONTENT )
	return E_FAIL;
    if ( !psizel )
	return E_POINTER;

    return GetExtent( 0, 0, 0, psizel );
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetMoniker( DWORD, DWORD, IMoniker**  )
{
    return E_NOTIMPL;
}

/*
    Returns the CLSID of this class.
*/
HRESULT WINAPI QAxServerBase::GetUserClassID( CLSID* pClsid )
{
    if ( !pClsid )
	return E_POINTER;
    *pClsid = qAxFactory()->classID( class_name );
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::InitFromData( IDataObject*, BOOL, DWORD )
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::IsUpToDate()
{
    return S_OK;
}

/*
    Stores the client site.
*/
HRESULT WINAPI QAxServerBase::SetClientSite( IOleClientSite* pClientSite )
{
    if ( !pClientSite && m_spClientSite )
	m_spClientSite->Release();

    m_spClientSite = pClientSite;
    if ( m_spClientSite )
	m_spClientSite->AddRef();
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetColorScheme( LOGPALETTE* )
{
    return E_NOTIMPL;
}

/*
    Tries to set the size of the control.
*/
HRESULT WINAPI QAxServerBase::SetExtent( DWORD dwDrawAspect, SIZEL* psizel )
{
    if ( dwDrawAspect != DVASPECT_CONTENT )
	return DV_E_DVASPECT;
    if ( !psizel )
	return E_POINTER;

    QSize minSizeHint;
    if ( activeqt ) 
	minSizeHint = activeqt->minimumSizeHint();

    if ( minSizeHint.isValid() ) {
	QPaintDeviceMetrics pmetric( activeqt );

	SIZEL minSize;
	minSize.cx = MAP_PIX_TO_LOGHIM( minSizeHint.width(), pmetric.logicalDpiX() );
	minSize.cy = MAP_PIX_TO_LOGHIM( minSizeHint.height(), pmetric.logicalDpiY() );

	if ( minSize.cx > psizel->cx || minSize.cy > psizel->cy )
	    return E_FAIL;
    }

    BOOL bResized = FALSE;
    if ( psizel->cx != sizeExtent.cx || psizel->cy != sizeExtent.cy ) {
	sizeExtent = *psizel;
	bResized = TRUE;
    }

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetHostNames( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj )
{
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetMoniker( DWORD, IMoniker* )
{
    return E_NOTIMPL;
}

/*
    Disconnects an advise sink.
*/
HRESULT WINAPI QAxServerBase::Unadvise( DWORD dwConnection )
{
    HRESULT hRes = E_FAIL;
    if ( m_spOleAdviseHolder )
	hRes = m_spOleAdviseHolder->Unadvise(dwConnection);
    return hRes;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::Update()
{
    return S_OK;
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

/*
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
	break;
    default:
	break;
    }

    return QObject::eventFilter( o, e );
}

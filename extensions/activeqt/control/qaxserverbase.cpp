/****************************************************************************
**
** Copyright (C) 2001-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qbuffer.h>
#include <qeventloop.h>
#include <qfile.h>
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
#include <ocidl.h>
#include <olectl.h>

#include "qaxfactory.h"
#include "qaxbindable.h"

#include "../shared/types.h"

#ifndef Q_OS_WIN64
#define ULONG_PTR DWORD
#endif

extern HHOOK qax_hhook;

// in qaxserver.cpp
extern ITypeLib *qAxTypeLibrary;
extern QAxFactoryInterface *qAxFactory();
extern unsigned long qAxLock();
extern unsigned long qAxUnlock();
extern HANDLE qAxInstance;

// in qaxserverdll.cpp
extern bool qax_ownQApp;

struct QAxExceptInfo
{
    QAxExceptInfo( int c, const QString &s, const QString &d, const QString &x )
	: code(c), src(s), desc(d), context(x)
    {
    }
    int code;
    QString src;
    QString desc;
    QString context;
};
static QAxExceptInfo *qAxException = 0;

// documentation in qaxbindable.cpp
void QAxBindable::reportError( int code, const QString &src, const QString &desc, const QString &context )
{
    if ( qAxException )
	delete qAxException;
    qAxException = new QAxExceptInfo( code, src, desc, context );
}

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
\if defined(commercial_edition)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

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
    public IPersistStreamInit,
    public IPersistStorage,
    public IPersistPropertyBag,
    public IDataObject
{
public:
    typedef QMap<QUuid,IConnectionPoint*> ConnectionPoints;
    typedef QMap<QUuid,IConnectionPoint*>::Iterator ConnectionPointsIterator;

    QAxServerBase( const QString &classname, IUnknown *outerUnknown );
    QAxServerBase( QObject *o );

    void init();

    ~QAxServerBase();

// Window creation
    HWND create(HWND hWndParent, RECT& rcPos );
    HMENU createPopup( QPopupMenu *popup, HMENU oldMenu = 0 );
    void createMenu( QMenuBar *menuBar );
    void removeMenu();

    static LRESULT CALLBACK ActiveXProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Object registration with OLE
    void registerActiveObject(IUnknown *object);
    void revokeActiveObject();

// IUnknown
    unsigned long WINAPI AddRef()
    {
	if ( m_outerUnknown )
	    return m_outerUnknown->AddRef();

	EnterCriticalSection( &refCountSection );
	unsigned long r = ++ref;
	LeaveCriticalSection( &refCountSection );

	return r;
    }
    unsigned long WINAPI Release()
    {
    	if ( m_outerUnknown )
	    return m_outerUnknown->Release();

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
    HRESULT InternalQueryInterface( REFIID iid, void **iface );

// IAxServerBase
    IUnknown *clientSite() const
    {
	return m_spClientSite;
    }

    void emitPropertyChanged( const char*, long dispid = -1 );
    bool emitRequestPropertyChange( const char*, long dispid = -1 );
    QObject *qObject() const
    {
	return theObject;
    }
    void readMetaData();
    bool isPropertyExposed(int index);

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
		    BOOL(__stdcall*pfnContinue)(ULONG_PTR), ULONG_PTR dwContinue );
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
    STDMETHOD(TranslateAcceleratorW)( MSG *pMsg );
    STDMETHOD(TranslateAcceleratorA)( MSG *pMsg );
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

// IPersistStreamInit
    STDMETHOD(InitNew)(VOID);
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

// IPersistPropertyBag
    STDMETHOD(Load)(IPropertyBag *, IErrorLog *);
    STDMETHOD(Save)(IPropertyBag *, BOOL, BOOL);

// IPersistStorage
    STDMETHOD(InitNew)(IStorage *pStg );
    STDMETHOD(Load)(IStorage *pStg );
    STDMETHOD(Save)(IStorage *pStg, BOOL fSameAsLoad );
    STDMETHOD(SaveCompleted)( IStorage *pStgNew );
    STDMETHOD(HandsOffStorage)();

// IDataObject
    STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    STDMETHOD(GetDataHere)(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */);
    STDMETHOD(QueryGetData)(FORMATETC* /* pformatetc */);
    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* /* pformatectIn */,FORMATETC* /* pformatetcOut */);
    STDMETHOD(SetData)(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */, BOOL /* fRelease */);
    STDMETHOD(EnumFormatEtc)(DWORD /* dwDirection */, IEnumFORMATETC** /* ppenumFormatEtc */);
    STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);

// QObject
    bool qt_emit( int, QUObject* );

    bool eventFilter( QObject *o, QEvent *e );
private:
    void update();
    void updateGeometry();
    void updateMask();
    bool internalCreate();
    void internalBind();
    void internalConnect();
    HRESULT internalActivate();

    friend class QAxBindable;
    friend class QAxPropertyPage;

    QAxAggregated *aggregatedObject;
    ConnectionPoints points;

    union {
	QWidget *widget;
	QObject *object;
    } qt;
    QGuardedPtr<QObject> theObject;
    unsigned isWidget		:1;
    unsigned ownObject		:1;
    unsigned initNewCalled	:1;
    unsigned dirtyflag		:1;
    unsigned hasStockEvents	:1;
    unsigned stayTopLevel	:1;
    unsigned isInPlaceActive	:1;
    unsigned isUIActive		:1;
    unsigned wasUIActive	:1;
    unsigned inDesignMode	:1;
    unsigned canTakeFocus	:1;
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
    unsigned long ole_ref;

    QString class_name;

    QIntDict<QMetaData>* slotlist;
    QMap<int,DISPID>* signallist;
    QIntDict<QMetaProperty>* proplist;

    IUnknown *m_outerUnknown;
    IAdviseSink *m_spAdviseSink;
    IOleAdviseHolder *m_spOleAdviseHolder;
    IOleClientSite *m_spClientSite;
    IOleInPlaceSiteWindowless *m_spInPlaceSite;
    IOleInPlaceFrame *m_spInPlaceFrame;
    ITypeInfo *m_spTypeInfo;
    IStorage *m_spStorage;
};

class QAxServerAggregate : public IUnknown
{
public:
    QAxServerAggregate( const QString &className, IUnknown *outerUnknown )
	: m_outerUnknown( outerUnknown ), ref(0)
    {
	object = new QAxServerBase( className, outerUnknown );
	object->registerActiveObject(this);

	InitializeCriticalSection( &refCountSection );
	InitializeCriticalSection( &createWindowSection );
    }
    ~QAxServerAggregate()
    {
	DeleteCriticalSection( &refCountSection );
	DeleteCriticalSection( &createWindowSection );

	delete object;
    }

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
    HRESULT WINAPI QueryInterface( REFIID iid, void **iface )
    {
	*iface = 0;
	
	HRESULT res = E_NOINTERFACE;
	if ( iid == IID_IUnknown ) {
	    *iface = (IUnknown*)this;
	    AddRef();
	    return S_OK;
	}
	return object->InternalQueryInterface( iid, iface );
    }

private:
    QAxServerBase *object;
    IUnknown *m_outerUnknown;
    unsigned long ref;

    CRITICAL_SECTION refCountSection;
    CRITICAL_SECTION createWindowSection;
};

bool QAxFactory::createObjectWrapper(QObject *object, IDispatch **wrapper)
{
    *wrapper = 0;
    QAxServerBase *obj = new QAxServerBase( object );
    obj->QueryInterface( IID_IDispatch, (void**)wrapper );
    if (*wrapper)
	return TRUE;

    delete obj;
    return FALSE;
}


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
	return r;
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
	if (pcFetched)
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

extern Q_EXPORT void qWinProcessConfigRequests();
LRESULT CALLBACK axs_FilterProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if ( qApp ) {
	qApp->sendPostedEvents();
	qApp->eventLoop()->activateSocketNotifiers();
	qWinProcessConfigRequests();
    }

    return CallNextHookEx( qax_hhook, nCode, wParam, lParam );
}

// COM Factory class, mapping COM requests to ActiveQt requests.
// One instance of this class for each ActiveX the server can provide.
class QClassFactory : public IClassFactory2
{
public:
    QClassFactory( CLSID clsid )
	: ref( 0 ), licensed(FALSE)
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

	QMetaObject *mo = qAxFactory()->metaObject(className);
	if (mo && mo->classInfo("LicenseKey", TRUE)) {
	    licensed = TRUE;
	    classKey = QString::fromLatin1(mo->classInfo("LicenseKey", TRUE));
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
	else if ( iid == IID_IClassFactory2 && licensed )
	    *iface = (IClassFactory2*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    HRESULT WINAPI CreateInstanceHelper( IUnknown *pUnkOuter, REFIID iid, void **ppObject )
    {
	if ( pUnkOuter ) {
	    if (iid != IID_IUnknown)
		return CLASS_E_NOAGGREGATION;
	    QMetaObject *mo = qAxFactory()->metaObject(className);
	    if (mo && !qstrcmp(mo->classInfo("Aggregatable", TRUE), "no"))
		return CLASS_E_NOAGGREGATION;
	}

	// Make sure a QApplication instance is present (inprocess case)
	if ( !qApp ) {
	    qax_ownQApp = TRUE;
	    int argc = 0;
	    (void)new QApplication( argc, 0 );
	    QT_WA( {
		qax_hhook = SetWindowsHookExW( WH_GETMESSAGE, axs_FilterProc, 0, GetCurrentThreadId() );
	    }, {
		qax_hhook = SetWindowsHookExA( WH_GETMESSAGE, axs_FilterProc, 0, GetCurrentThreadId() );
	    } );
	}

	HRESULT res;
	// Create the ActiveX wrapper - aggregate if requested
	if ( pUnkOuter ) {
	    QAxServerAggregate *aggregate = new QAxServerAggregate( className, pUnkOuter );
	    res = aggregate->QueryInterface( iid, ppObject );
	    if (FAILED(res))
		delete aggregate;
	} else {
	    QAxServerBase *activeqt = new QAxServerBase( className, pUnkOuter );
	    res = activeqt->QueryInterface( iid, ppObject );
	    if (FAILED(res))
		delete activeqt;
	    else
		activeqt->registerActiveObject((IUnknown*)(IDispatch*)activeqt);
	}
	return res;
    }

    // IClassFactory
    HRESULT WINAPI CreateInstance( IUnknown *pUnkOuter, REFIID iid, void **ppObject )
    {
	// class is licensed
	if (licensed && !qAxFactory()->validateLicenseKey(className, QString()))
	    return CLASS_E_NOTLICENSED;

	return CreateInstanceHelper(pUnkOuter, iid, ppObject);
    }
    HRESULT WINAPI LockServer( BOOL fLock )
    {
	if ( fLock )
	    qAxLock();
	else
	    qAxUnlock();

	return S_OK;	
    }

    // IClassFactory2
    HRESULT WINAPI RequestLicKey(DWORD, BSTR *pKey)
    {
	if (!pKey)
	    return E_POINTER;
	*pKey = 0;

	// This of course works only on fully licensed machines
	if (!qAxFactory()->validateLicenseKey(className, QString()))
	    return CLASS_E_NOTLICENSED;

	*pKey = QStringToBSTR(classKey);
	return S_OK;
    }

    HRESULT WINAPI GetLicInfo(LICINFO *pLicInfo)
    {
	if (!pLicInfo)
	    return E_POINTER;
	pLicInfo->cbLicInfo = sizeof(LICINFO);

	// class specific license key?
	QMetaObject *mo = qAxFactory()->metaObject(className);
	const char *key = mo->classInfo("LicenseKey", TRUE);
	pLicInfo->fRuntimeKeyAvail = key && key[0];

	// machine fully licensed?
	pLicInfo->fLicVerified = qAxFactory()->validateLicenseKey(className, QString());

	return S_OK;
    }

    HRESULT WINAPI CreateInstanceLic(IUnknown *pUnkOuter, IUnknown *pUnkReserved, REFIID iid, BSTR bKey, PVOID *ppObject)
    {
	QString licenseKey(BSTRToQString(bKey));
	if (!qAxFactory()->validateLicenseKey(className, licenseKey))
	    return CLASS_E_NOTLICENSED;
	return CreateInstanceHelper(pUnkOuter, iid, ppObject);
    }

    QString className;

protected:
    CRITICAL_SECTION refCountSection;
    unsigned long ref;
    bool licensed;
    QString classKey;
};

// Create a QClassFactory object for class \a iid
HRESULT GetClassObject( REFIID clsid, REFIID iid, void **ppUnk )
{
    QClassFactory *factory = new QClassFactory( clsid );
    if ( !factory )
	return E_OUTOFMEMORY;
    if ( factory->className.isEmpty() ) {
	delete factory;
	return E_NOINTERFACE;
    }
    HRESULT res = factory->QueryInterface( iid, ppUnk );
    if ( res != S_OK )
	delete factory;
    return res;
}


/*!
    Constructs a QAxServerBase object wrapping the QWidget \a
    classname into an ActiveX control.

    The constructor is called by the QClassFactory object provided by
    the COM server for the respective CLSID.
*/
QAxServerBase::QAxServerBase( const QString &classname, IUnknown *outerUnknown )
: aggregatedObject(0), ref(0), ole_ref(0), class_name(classname),
  slotlist(0), signallist(0),proplist(0),
  m_hWnd(0), m_hWndCD(m_hWnd), hmenuShared(0), hwndMenuOwner(0),
  m_outerUnknown(outerUnknown)
{
    init();

    internalCreate();
}

/*!
    Constructs a QAxServerBase object wrapping \a o.
*/
QAxServerBase::QAxServerBase( QObject *o )
: aggregatedObject(0), ref( 0), ole_ref(0),
  slotlist(0), signallist(0),proplist(0),
  m_hWnd(0), m_hWndCD( m_hWnd ), hmenuShared(0), hwndMenuOwner(0),
  m_outerUnknown(0)
{
    init();

    qt.object = o;
    if ( o ) {
	theObject = o;
	isWidget = FALSE;
	class_name = o->className();
    }
    internalBind();
    internalConnect();
}

/*!
    Initializes data members.
*/
void QAxServerBase::init()
{
    qt.object = 0;
    isWidget		= FALSE;
    ownObject		= FALSE;
    initNewCalled	= FALSE;
    dirtyflag		= FALSE;
    hasStockEvents	= FALSE;
    stayTopLevel	= FALSE;
    isInPlaceActive	= FALSE;
    isUIActive		= FALSE;
    wasUIActive		= FALSE;
    inDesignMode	= FALSE;
    canTakeFocus	= FALSE;
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
}

/*!
    Destroys the QAxServerBase object, releasing all allocated
    resources and interfaces.
*/
QAxServerBase::~QAxServerBase()
{
    revokeActiveObject();

    for ( QAxServerBase::ConnectionPointsIterator it = points.begin(); it != points.end(); ++it ) {
	if ( it.data() )
	    (*it)->Release();
    }
    delete aggregatedObject;
    aggregatedObject = 0;
    if ( theObject ) {
	axTakeServer( m_hWnd );
	if ( qt.widget->isWidgetType() )
	    axTakeServer( qt.widget->winId() );
	qt.object->disconnect( this );
	QObject *aqt = qt.object;
	qt.object = 0;
	if ( ownObject )
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
}

/*
    Registering with OLE
*/
void QAxServerBase::registerActiveObject(IUnknown *object)
{
    extern char qAxModuleFilename[MAX_PATH];
    if (ole_ref || !qt.object || !QString::fromLocal8Bit(qAxModuleFilename).lower().endsWith(".exe"))
	return;

    const QMetaObject *mo = qt.object->metaObject();
    if (!qstrcmp(mo->classInfo("RegisterObject", TRUE), "yes"))
	RegisterActiveObject(object, qAxFactory()->classID(class_name), ACTIVEOBJECT_WEAK, &ole_ref);
}

void QAxServerBase::revokeActiveObject()
{
    if (!ole_ref)
	return;

    RevokeActiveObject(ole_ref, 0);
    ole_ref = 0;
}

/*
    QueryInterface implementation.
*/
HRESULT WINAPI QAxServerBase::QueryInterface( REFIID iid, void **iface )
{
    if ( m_outerUnknown )
	return m_outerUnknown->QueryInterface( iid, iface );

    return InternalQueryInterface( iid, iface );
}

HRESULT QAxServerBase::InternalQueryInterface( REFIID iid, void **iface )
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
	if ( iid == qAxFactory()->interfaceID(class_name) )
	    *iface = (IDispatch*)this;
	if ( iid == IID_IDispatch )
	    *iface = (IDispatch*)this;
	else if ( iid == IID_IAxServerBase)
	    *iface = (IAxServerBase*)this;
	else if ( iid == IID_IOleObject)
	    *iface = (IOleObject*)this;
	else if ( iid == IID_IConnectionPointContainer)
	    *iface = (IConnectionPointContainer*)this;
	else if ( iid == IID_IProvideClassInfo)
	    *iface = (IProvideClassInfo*)this;
	else if ( iid == IID_IProvideClassInfo2)
	    *iface = (IProvideClassInfo2*)this;
	else if ( iid == IID_IPersistStreamInit )
	    *iface = (IPersistStreamInit*)this;
	else if ( iid == IID_IPersistStorage )
	    *iface = (IPersistStorage*)this;
	else if ( iid == IID_IPersistPropertyBag)
	    *iface = (IPersistPropertyBag*)this;
	else if ( iid == IID_IViewObject)
	    *iface = (IViewObject*)this;
	else if ( iid == IID_IViewObject2)
	    *iface = (IViewObject2*)this;
	else if ( isWidget ) {
	    if ( iid == IID_IOleControl)
		*iface = (IOleControl*)this;
	    else if ( iid == IID_IOleWindow)
		*iface = (IOleWindow*)(IOleInPlaceObject*)this;
	    else if ( iid == IID_IOleInPlaceObject)
		*iface = (IOleInPlaceObject*)this;
	    else if ( iid == IID_IOleInPlaceActiveObject)
		*iface = (IOleInPlaceActiveObject*)this;
	    else if ( iid == IID_IDataObject)
		*iface = (IDataObject*)this;
	}
    }
    if ( !*iface )
	return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

class HackWidget : public QWidget
{
    friend class QAxServerBase;
};

/*!
    Detects and initilaizes implementation of QAxBindable in objects.
*/
void QAxServerBase::internalBind()
{
    QAxBindable *axb = (QAxBindable*)qt.object->qt_cast( "QAxBindable" );
    if ( axb ) {
	// no addref; this is aggregated
	axb->activex = this;
	if (!aggregatedObject)
	    aggregatedObject = axb->createAggregate();
	if ( aggregatedObject ) {
	    aggregatedObject->controlling_unknown = (IUnknown*)(IDispatch*)this;
	    aggregatedObject->the_object = qt.object;
	}
    }
}

/*!
    Connects object signals to event dispatcher.
*/
void QAxServerBase::internalConnect()
{
    QUuid eventsID = qAxFactory()->eventsID(class_name);
    if ( !eventsID.isNull() ) {
	if (!points[eventsID])
	    points[eventsID] = new QAxConnection( this, eventsID );
	// connect the generic slot to all signals of qt.object
	const QMetaObject *mo = qt.object->metaObject();
	for ( int isignal = mo->numSignals( TRUE )-1; isignal >= 0; --isignal )
	    connectInternal( qt.object, isignal, this, 2, isignal );
    }
}

/*!
    Creates the QWidget for the classname passed to the c'tor.

    All signals of the widget class are connected to the internal event mapper.
    If the widget implements QAxBindable, stock events are also connected.
*/
bool QAxServerBase::internalCreate()
{
    if ( qt.object )
	return TRUE;

    qt.object = qAxFactory()->createObject( class_name );
    Q_ASSERT(qt.object);
    if ( !qt.object )
	return FALSE;

    theObject = qt.object;
    ownObject = TRUE;
    isWidget = qt.object->isWidgetType();
    hasStockEvents = qAxFactory()->hasStockEvents( class_name );
    stayTopLevel = qAxFactory()->stayTopLevel( class_name );

    internalBind();
    if ( isWidget ) {
	if ( !stayTopLevel ) {
	    ((HackWidget*)qt.widget)->clearWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu );
	    ((HackWidget*)qt.widget)->topData()->ftop = 0;
	    ((HackWidget*)qt.widget)->topData()->fright = 0;
	    ((HackWidget*)qt.widget)->topData()->fleft = 0;
	    ((HackWidget*)qt.widget)->topData()->fbottom = 0;
	    QT_WA( {
		::SetWindowLongW( qt.widget->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
	    }, {
		::SetWindowLongA( qt.widget->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
	    } );
	}

	qt.widget->setGeometry( rcPos.left, rcPos.top, rcPos.right-rcPos.left, rcPos.bottom-rcPos.top );
	updateGeometry();
    }

    internalConnect();
    // install an event filter for stock events
    if ( isWidget )
	qt.object->installEventFilter( this );

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
	QT_WA( {
	    CREATESTRUCTW *cs = (CREATESTRUCTW*)lParam;
	    that = (QAxServerBase*)cs->lpCreateParams;
	}, {
	    CREATESTRUCTA *cs = (CREATESTRUCTA*)lParam;
	    that = (QAxServerBase*)cs->lpCreateParams;
	} );

	axServerMapper()->insert( hWnd, that );
	that->m_hWnd = hWnd;

	QT_WA( {
	    return ::DefWindowProcW( hWnd, uMsg, wParam, lParam );
	}, {
	    return ::DefWindowProcA( hWnd, uMsg, wParam, lParam );
	} );
    }

    QAxServerBase *that = axServerMapper()->find( hWnd );
    if ( that ) switch ( uMsg )
    {
    case WM_NCDESTROY:
	that->m_hWnd = 0;
	axTakeServer( hWnd );
	break;

    case WM_QUERYENDSESSION:
    case WM_DESTROY:
	if ( that->qt.widget && that->ownObject ) {
	    if ( that->aggregatedObject )
		that->aggregatedObject->the_object = 0;
	    delete that->qt.widget;
	    that->qt.widget = 0;
	}
	break;

    case WM_SHOWWINDOW:
	if( wParam ) {
	    that->internalCreate();
	    if ( !that->stayTopLevel ) {
		::SetParent( that->qt.widget->winId(), that->m_hWnd );
		that->qt.widget->raise();
		that->qt.widget->move( 0, 0 );
	    }
	    that->qt.widget->show();
	} else if ( that->qt.widget ) {
	    that->qt.widget->hide();
	}
	break;

    case WM_ERASEBKGND:
	that->updateMask();
	break;

    case WM_SIZE:
	if ( that->qt.widget )
	    that->qt.widget->resize( LOWORD(lParam), HIWORD(lParam) );
	break;

    case WM_SETFOCUS:
	if ( that->isInPlaceActive && that->m_spClientSite && !that->inDesignMode && that->canTakeFocus ) {
	    that->DoVerb(OLEIVERB_UIACTIVATE, NULL, that->m_spClientSite, 0, that->m_hWndCD, &that->rcPos);
	    if ( that->isUIActive ) {
		IOleControlSite *spSite = 0;
		that->m_spClientSite->QueryInterface( IID_IOleControlSite, (void**)&spSite );
		if ( spSite ) {
		    spSite->OnFocus(TRUE);
		    spSite->Release();
		}
		if ( that->qt.widget->focusWidget() && !that->inDesignMode )
		    that->qt.widget->focusWidget()->setFocus();
		else {
		    QFocusData *focusData = ((HackWidget*)that->qt.widget)->focusData();
		    QWidget *candidate = 0;
		    if ( ::GetKeyState(VK_SHIFT) < 0 )
			candidate = focusData->last();
		    else
			candidate = focusData->first();
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
	if ( that->qt.widget ) {
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
	if ( that->qt.widget ) {
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

    QT_WA( {
	return ::DefWindowProcW( hWnd, uMsg, wParam, lParam );
    }, {
	return ::DefWindowProcA( hWnd, uMsg, wParam, lParam );
    } );
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
	QT_WA( {
	    WNDCLASSW wcTemp;
	    wcTemp.style = CS_DBLCLKS;
	    wcTemp.cbClsExtra = 0;
	    wcTemp.cbWndExtra = 0;
	    wcTemp.hbrBackground = 0;
	    wcTemp.hCursor = 0;
	    wcTemp.hIcon = 0;
	    wcTemp.hInstance = hInst;
	    wcTemp.lpszClassName = L"QAxControl";
	    wcTemp.lpszMenuName = 0;
	    wcTemp.lpfnWndProc = ActiveXProc;

	    atom = RegisterClassW( &wcTemp );
	}, {
	    WNDCLASSA wcTemp;
	    wcTemp.style = CS_DBLCLKS;
	    wcTemp.cbClsExtra = 0;
	    wcTemp.cbWndExtra = 0;
	    wcTemp.hbrBackground = 0;
	    wcTemp.hCursor = 0;
	    wcTemp.hIcon = 0;
	    wcTemp.hInstance = hInst;
	    wcTemp.lpszClassName = "QAxControl";
	    wcTemp.lpszMenuName = 0;
	    wcTemp.lpfnWndProc = ActiveXProc;

	    atom = RegisterClassA( &wcTemp );
	} );
    }
    LeaveCriticalSection( &createWindowSection );
    if ( !atom  && GetLastError() != ERROR_CLASS_ALREADY_EXISTS )
	return 0;
    
    Q_ASSERT( !m_hWnd );

    HWND hWnd = 0;

    QT_WA( {
	hWnd = ::CreateWindowW( L"QAxControl", 0,
	    WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	    rcPos.left, rcPos.top, rcPos.right - rcPos.left,
	    rcPos.bottom - rcPos.top, hWndParent, 0, hInst, this );
    }, {
	hWnd = ::CreateWindowA( "QAxControl", 0,
	    WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	    rcPos.left, rcPos.top, rcPos.right - rcPos.left,
	    rcPos.bottom - rcPos.top, hWndParent, 0, hInst, this );
    } );
    this->rcPos = rcPos;

    Q_ASSERT(m_hWnd == hWnd);

    internalCreate();
    updateMask();

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

	if ( qitem->text() == qt.widget->tr("&Edit") )
	    edit++;
	else if ( qitem->text() == qt.widget->tr("&Help") )
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
	hmenuShared = 0;
	return;
    }

    m_spInPlaceFrame->GetWindow( &hwndMenuOwner );

    holemenu = OleCreateMenuDescriptor( hmenuShared, &menuWidths );
    hres = m_spInPlaceFrame->SetMenu( hmenuShared, holemenu, m_hWnd );
    if ( FAILED(hres) ) {
	::DestroyMenu( hmenuShared );
	hmenuShared = 0;
	OleDestroyMenuDescriptor( holemenu );
    }
}

/*!
    Remove the Win32 menubar.
*/
void QAxServerBase::removeMenu()
{
    if ( hmenuShared )
	m_spInPlaceFrame->RemoveMenus( hmenuShared );
    holemenu = 0;
    m_spInPlaceFrame->SetMenu( 0, 0, m_hWnd );
    if ( hmenuShared ) {
	DestroyMenu( hmenuShared );
	hmenuShared = 0;
	menuMap.clear();
    }
    hwndMenuOwner = 0;
}

extern bool ignoreSlots( const char *test );
extern bool ignoreProps( const char *test );

/*!
    Creates mappings between DISPIDs and Qt signal/slot/property data.
*/
void QAxServerBase::readMetaData()
{
    if ( !theObject )
	return;

    if ( !slotlist ) {
	slotlist = new QIntDict<QMetaData>;
	signallist = new QMap<int,DISPID>;
	proplist = new QIntDict<QMetaProperty>;

	int qtProps = 0;
	int qtSlots = 0;

	if (theObject->isWidgetType()) {
	    qtProps = QWidget::staticMetaObject()->numProperties(TRUE);
	    qtSlots = QWidget::staticMetaObject()->numProperties(TRUE);
	}

	const QMetaObject *mo = qt.object->metaObject();
	for ( int islot = mo->numSlots( TRUE )-1; islot >=0 ; --islot ) {
	    const QMetaData *slot = mo->slot( islot, TRUE );

	    if (islot <= qtSlots && ignoreSlots( slot->method->name ) || slot->access != QMetaData::Public)
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

	    if (iproperty <= qtProps && ignoreProps( property->name() ))
		continue;

	    BSTR bstrNames = QStringToBSTR( property->name() );
	    DISPID dispId;
	    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, 1, LOCALE_USER_DEFAULT, &dispId );
	    if ( dispId >= 0 && !proplist->find( dispId ) )
		proplist->insert( dispId, property );
	    SysFreeString( bstrNames );
	}
    }
}

/*!
    \internal
    Returns TRUE if the property \a index is exposed to COM and should
    be saved/loaded.
*/
bool QAxServerBase::isPropertyExposed(int index)
{
    if (!theObject)
	return FALSE;

    bool result = FALSE;
    QMetaObject *mo = theObject->metaObject();

    int qtProps = 0;
    if (theObject->isWidgetType())
	qtProps = QWidget::staticMetaObject()->numProperties(TRUE);
    const QMetaProperty *property = mo->property( index, TRUE );
    if (index <= qtProps && ignoreProps( property->name() ))
	return result;

    BSTR bstrNames = QStringToBSTR( property->name() );
    DISPID dispId;
    GetIDsOfNames( IID_NULL, (BSTR*)&bstrNames, 1, LOCALE_USER_DEFAULT, &dispId );
    result = dispId != DISPID_UNKNOWN;
    SysFreeString( bstrNames );

    return result;
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
    if ( !isWidget || !qt.widget)
	return;

    QSize sizeHint = qt.widget->sizeHint();
    if ( sizeHint.isValid() ) {
	QPaintDeviceMetrics pmetric( qt.widget );

	sizeExtent.cx = MAP_PIX_TO_LOGHIM( sizeHint.width(), pmetric.logicalDpiX() );
	sizeExtent.cy = MAP_PIX_TO_LOGHIM( sizeHint.height(), pmetric.logicalDpiY() );
    }
}

/*!
    \internal
    
    Updates the mask of the widget parent.
*/
void QAxServerBase::updateMask()
{
    if ( !isWidget || !qt.widget || !qt.widget->autoMask() )
	return;

    HRGN hrgn = CreateRectRgn(0,0,0,0);
    int regionType = GetWindowRgn( qt.widget->winId(), hrgn );
    if ( regionType != ERROR ) {
	SetWindowRgn( m_hWnd, hrgn, TRUE );
    } else {
	DeleteObject(hrgn);
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

	if ( statusBar->isHidden() ) {
	    QString message = static_QUType_QString.get( _o+1 );
	    m_spInPlaceFrame->SetStatusText( QStringToBSTR(message) );
	}
	return TRUE;
    }

    if ( freezeEvents || inDesignMode )
	return TRUE;

    if ( !signallist )
	readMetaData();

    // get the signal information.
    bool stockEvent = isignal < 0;
    const QMetaData *signal = stockEvent ? 0 : qt.object->metaObject()->signal( isignal, TRUE );
    if ( !signal && !stockEvent )
	return FALSE;
    int signalcount = signal ? signal->method->count : 0;
    bool retValue = signalcount ? ( signal->method->parameters->inOut == QUParameter::Out ) : FALSE;
    if ( retValue )
	signalcount--;
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
		VARIANT retval;
		VariantInit( &retval );
		VARIANT *pretval = retValue ? &retval : 0;
		// call listeners (through IDispatch)
		GUID IID_QAxEvents = qAxFactory()->eventsID( class_name );
		while ( cc ) {
		    if ( c->pUnk ) {
			IDispatch *disp = 0;
			c->pUnk->QueryInterface( IID_QAxEvents, (void**)&disp );
			if ( disp ) {
			    disp->Invoke( eventId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispParams, pretval, 0, &argErr );
			    if ( signal && signal->method ) {
				for ( p = 0; p < signalcount; ++p ) {
				    const QUParameter *param = signal->method->parameters + p;
				    if ( param->inOut & QUParameter::Out ) {
					QUObject *obj = _o + p + 1;
					if ( obj->type )
					    obj->type->clear( obj );
					VARIANTToQUObject( dispParams.rgvarg[ signalcount - p - 1 ], obj, param );
				    }
				}
				if ( pretval )
				    VARIANTToQUObject( retval, _o, signal->method->parameters );
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
    
    *pptinfo = 0;
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
		  EXCEPINFO* pexcepinfo, UINT* puArgErr )
{
    if ( riid != IID_NULL )
	return DISP_E_UNKNOWNINTERFACE;
    if ( !theObject )
	return E_UNEXPECTED;

    HRESULT res = DISP_E_MEMBERNOTFOUND;

    if ( !slotlist )
	readMetaData();

    QSize oldSizeHint;
    if ( isWidget )
	oldSizeHint = qt.widget->sizeHint();

    switch ( wFlags ) {
    case DISPATCH_PROPERTYGET|DISPATCH_METHOD:
    case DISPATCH_PROPERTYGET:
	{
	    const QMetaProperty *property = proplist->find( dispidMember );
	    if ( property ) {
		if ( !pvarResult )
		    return DISP_E_PARAMNOTOPTIONAL;
		if ( pDispParams->cArgs ||
		     pDispParams->cNamedArgs )
		    return DISP_E_BADPARAMCOUNT;

		QVariant var = qt.object->property( property->name() );
		if ( !var.isValid() )
		    res =  DISP_E_MEMBERNOTFOUND;
		else if ( !QVariantToVARIANT( var, *pvarResult, property->type() ) )
		    res = DISP_E_TYPEMISMATCH;
		else
		    res = S_OK;
		break;
	    } else if ( wFlags == DISPATCH_PROPERTYGET ) {
		break;
	    }
	}
	// FALLTHROUGH if wFlags == DISPATCH_PROPERTYGET|DISPATCH_METHOD AND not a property.
    case DISPATCH_METHOD:
	{
	    const QMetaData *slot = slotlist->find( dispidMember );
	    if ( !slot )
		break;
	    int index = qt.object->metaObject()->findSlot( slot->name, TRUE );
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
		qt.object->qt_invoke( index, objects );

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
		    if (pvarResult)
			QUObjectToVARIANT( objects, *pvarResult, params );
		    clearQUObject( objects, params );
		}
	    }

	    delete [] objects;
	    res = ok ? S_OK : DISP_E_TYPEMISMATCH;
	}
	break;
    case DISPATCH_PROPERTYPUT:
    case DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF:
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

	    QVariant var = VARIANTToQVariant( *pDispParams->rgvarg, property->type() );
	    if ( !var.isValid() ) {
		if ( puArgErr )
		    *puArgErr = 0;
		return DISP_E_BADVARTYPE;
	    }
	    if ( !qt.object->setProperty( property->name(), var ) ) {
		if ( puArgErr )
		    *puArgErr = 0;
		return DISP_E_TYPEMISMATCH;
	    }
	    
	    res = S_OK;

	    if ( m_spAdviseSink )
		m_spAdviseSink->OnViewChange( DVASPECT_CONTENT, 0 );
	}
	break;

    default:
	break;
    }

    if ( qAxException ) {
	if ( pexcepinfo ) {
	    memset( pexcepinfo, 0, sizeof(EXCEPINFO) );

	    pexcepinfo->wCode = qAxException->code;
	    if ( !qAxException->src.isNull() )
		pexcepinfo->bstrSource = QStringToBSTR(qAxException->src);
	    if ( !qAxException->desc.isNull() )
		pexcepinfo->bstrDescription = QStringToBSTR(qAxException->desc);
	    if ( !qAxException->context.isNull() ) {
		QString context = qAxException->context;
		int contextID = 0;
		int br = context.find( '[' );
		if ( br != -1 ) {
		    context = context.mid( br+1 );
		    context = context.left( context.length() - 1 );
		    contextID = context.toInt();

		    context = qAxException->context;
		    context = context.left( br-1 );
		}
		pexcepinfo->bstrHelpFile = QStringToBSTR(context);
		pexcepinfo->dwHelpContext = contextID;
	    }
	}
	delete qAxException;
	qAxException = 0;
	return DISP_E_EXCEPTION;
    } else if ( isWidget ) {
	QSize sizeHint = qt.widget->sizeHint();
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
	updateMask();
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

//**** IPersistStream
/*
    \reimp

    See documentation of IPersistStorage::IsDirty.
*/
HRESULT WINAPI QAxServerBase::IsDirty()
{
    return dirtyflag ? S_OK : S_FALSE;
}

HRESULT WINAPI QAxServerBase::Load( IStream *pStm )
{
    STATSTG stat;
    HRESULT hres = pStm->Stat( &stat, STATFLAG_NONAME );
    QByteArray qtarray;
    if ( hres != S_OK ) {
    } else {
	ULONG read;
	if ( stat.cbSize.HighPart )
	    return S_FALSE;
	qtarray.resize( stat.cbSize.LowPart );
	pStm->Read( qtarray.data(), stat.cbSize.LowPart, &read );
    }

    readMetaData();

    QBuffer qtbuffer( qtarray );
    qtbuffer.open( IO_ReadOnly | IO_Translate );
    QDataStream qtstream( &qtbuffer );
    int version;
    qtstream >> version;
    qtstream.setVersion(version);

    const QMetaObject *mo = qt.object->metaObject();
    while ( !qtbuffer.atEnd() ) {
	QCString propname;
	QVariant value;
	qtstream >> propname;
	qtstream >> value;

	int idx = mo->findProperty(propname, TRUE);
	const QMetaProperty *property = mo->property( idx, TRUE );
	if (property && property->writable())
	    qt.object->setProperty( propname, value );
    }
    return S_OK;
}

HRESULT WINAPI QAxServerBase::Save( IStream *pStm, BOOL clearDirty )
{
    QBuffer qtbuffer;
    qtbuffer.open( IO_WriteOnly | IO_Translate );
    QDataStream qtstream( &qtbuffer );
    qtstream << qtstream.version();
    
    readMetaData();

    const QMetaObject *mo = qt.object->metaObject();

    for ( int prop = 0; prop < mo->numProperties( TRUE ); ++prop ) {
	if ( !isPropertyExposed( prop ) )
	    continue;
	QCString property = mo->property( prop, TRUE )->name();
	QVariant qvar = qt.object->property( property );
	if ( qvar.isValid() ) {
	    qtstream << property;
	    qtstream << qvar;
	}
    }

    qtbuffer.close();
    QByteArray qtarray = qtbuffer.buffer();
    ULONG written = 0;
    char *data = qtarray.data();
    ULARGE_INTEGER newsize;
    newsize.HighPart = 0;
    newsize.LowPart = qtarray.size();
    pStm->SetSize( newsize );
    pStm->Write( data, qtarray.size(), &written );
    pStm->Commit( STGC_ONLYIFCURRENT );
    
    return S_OK;
}

HRESULT WINAPI QAxServerBase::GetSizeMax( ULARGE_INTEGER *pcbSize )
{
    readMetaData();

    const QMetaObject *mo = qt.object->metaObject();

    int np = mo->numProperties( TRUE );
    pcbSize->HighPart = 0;
    pcbSize->LowPart = np * 50;

    return S_OK;
}

//**** IPersistStorage

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
    if ( InitNew( pStg ) != S_OK )
	return CO_E_ALREADYINITIALIZED;

    IStream *spStream = 0;
    HRESULT hres = pStg->OpenStream( L"SomeStreamName", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &spStream );
    if ( !spStream )
	return E_FAIL;

    Load( spStream );

    spStream->Release();

    updateGeometry();

    return S_OK;
}

HRESULT WINAPI QAxServerBase::Save(IStorage *pStg, BOOL fSameAsLoad )
{
    IStream *spStream = 0;
    HRESULT hres = pStg->CreateStream( L"SomeStreamName", STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStream );
    if ( !spStream )
	return E_FAIL;

    Save( spStream, TRUE );

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
    return S_OK;
}

/*
    Set the properties of the Qt widget to the values provided in the \a bag.
*/
HRESULT WINAPI QAxServerBase::Load( IPropertyBag *bag, IErrorLog * /*log*/ )
{
    if ( !bag )
	return E_POINTER;

    if ( InitNew() != S_OK )
	return E_UNEXPECTED;

    readMetaData();

    bool error = FALSE;
    const QMetaObject *mo = qt.object->metaObject();
    for ( int prop = 0; prop < mo->numProperties( TRUE ); ++prop ) {
	if ( !isPropertyExposed( prop ) )
	    continue;
	const QMetaProperty *property = mo->property( prop, TRUE );
	const char* pname = property->name();
	BSTR bstr = QStringToBSTR( pname );
	VARIANT var;
	var.vt = VT_EMPTY;
	HRESULT res = bag->Read( bstr, &var, 0 );
	if ( property->writable() ) {
	    if ( res != S_OK || !qt.object->setProperty( pname, VARIANTToQVariant( var, property->type() ) ) )
		error = TRUE;
	}
	SysFreeString(bstr);
    }
    
    updateGeometry();

    return /*error ? E_FAIL :*/ S_OK;
}

/*
    Save the properties of the Qt widget into the \a bag.
*/
HRESULT WINAPI QAxServerBase::Save( IPropertyBag *bag, BOOL clearDirty, BOOL /*saveAll*/ )
{
    if ( !bag )
	return E_POINTER;

    readMetaData();

    if ( clearDirty )
	dirtyflag = FALSE;
    bool error = FALSE;
    const QMetaObject *mo = qt.object->metaObject();
    for ( int prop = 0; prop < mo->numProperties( TRUE ); ++prop ) {
	if ( !isPropertyExposed( prop ) )
	    continue;
	const QMetaProperty *property = mo->property( prop, TRUE );
	BSTR bstr = QStringToBSTR( property->name() );
	QVariant qvar;
	if ( !qt.object->qt_property( prop, 1, &qvar ) )
	    error = TRUE;
	VARIANT var;
	QVariantToVARIANT( qvar, var, property->type() );
	bag->Write( bstr, &var );
	SysFreeString(bstr);
    }
    return /*error ? E_FAIL :*/ S_OK;
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
		BOOL(__stdcall* /*pfnContinue*/)(ULONG_PTR), ULONG_PTR /*dwContinue*/ )
{
    if ( !lprcBounds )
	return E_INVALIDARG;

    internalCreate();

    if ( !isWidget || !qt.widget )
	return OLE_E_BLANK;

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

    RECTL rc = *lprcBounds;
    bool bMetaFile = GetDeviceCaps(hdcDraw, TECHNOLOGY) == DT_METAFILE;
    if ( !bMetaFile  )
	::LPtoDP(hicTargetDev, (LPPOINT)&rc, 2);

    qt.widget->resize( rc.right - rc.left, rc.bottom - rc.top );
    QPixmap pm = QPixmap::grabWidget( qt.widget );
    BOOL res = ::BitBlt( hdcDraw, rc.left, rc.top, pm.width(), pm.height(), pm.handle(), 0, 0, SRCCOPY );
    if ( !res ) {
	QPainter painter( qt.widget );
	HDC oldDC = ((HackPainter*)&painter)->hdc;
	((HackPainter*)&painter)->hdc = hdcDraw;

	painter.drawText( rc.left, rc.top, "I don't know how to draw myself!" );

	((HackPainter*)&painter)->hdc = oldDC;
    }

    if ( bDeleteDC )
	DeleteDC( hicTargetDev );

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
    if ( m_spAdviseSink ) m_spAdviseSink->Release();

    m_spAdviseSink = pAdvSink;
    if ( m_spAdviseSink ) m_spAdviseSink->AddRef();
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
    if ( !m_spClientSite || !theObject )
	return S_OK;

    IDispatch *disp = 0;
    m_spClientSite->QueryInterface( IID_IDispatch, (void**)&disp );
    if ( !disp )
	return S_OK;

    VARIANT var;
    VariantInit( &var );
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
	if ( isWidget ) {
	    long rgb;
	    if ( var.vt == VT_UI4 )
		rgb = var.ulVal;
	    else if ( var.vt == VT_I4 )
		rgb = var.lVal;
	    else
		break;
	    QPalette pal = qt.widget->palette();
	    pal.setColor( dispID == DISPID_AMBIENT_BACKCOLOR ? QColorGroup::Background : QColorGroup::Foreground,
		OLEColorToQColor( rgb ) );
	    qt.widget->setPalette( pal );
	}
	break;
    case DISPID_AMBIENT_DISPLAYASDEFAULT:
	break;
    case DISPID_AMBIENT_DISPLAYNAME:
	if ( var.vt != VT_BSTR || !isWidget )
	    break;
	qt.widget->setCaption( BSTRToQString( var.bstrVal ) );
	break;
    case DISPID_AMBIENT_FONT:
	if ( var.vt != VT_DISPATCH || !isWidget )
	    break;
	{
	    IDispatch *d = var.pdispVal;
	    IFont *f = 0;
	    d->QueryInterface( IID_IFont, (void**)&f );
	    if ( f ) {
		QFont qfont = IFontToQFont( f );
		qt.widget->setFont( qfont );
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
	    qt.widget->installEventFilter( this );
	else
	    qt.widget->removeEventFilter( this );
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
	if ( var.vt != VT_BOOL || !isWidget )
	    break;
	qt.widget->setEnabled( !var.boolVal );
	break;
    case DISPID_AMBIENT_USERMODE:
	if ( var.vt != VT_BOOL )
	    break;
	inDesignMode = !var.boolVal;
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
	    removeMenu();
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

/*
    This event filter is called by QtWndProc.
    only messages sent to Qt widgets are visible here.
*/
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

    HRESULT hres = axbase->TranslateAcceleratorW( pMsg );
    if ( hres == S_OK )
	return 1;
    return 0;
}

Q_EXPORT int qt_translateKeyCode(int);

HRESULT WINAPI QAxServerBase::TranslateAcceleratorW( MSG *pMsg )
{
    if ( pMsg->message != WM_KEYDOWN || !isWidget )
	return S_FALSE;

    DWORD dwKeyMod = 0;
    if (::GetKeyState(VK_SHIFT) < 0)
	dwKeyMod |= 1;	// KEYMOD_SHIFT
    if (::GetKeyState(VK_CONTROL) < 0)
	dwKeyMod |= 2;	// KEYMOD_CONTROL
    if (::GetKeyState(VK_MENU) < 0)
	dwKeyMod |= 4;	// KEYMOD_ALT

    switch ( LOWORD( pMsg->wParam ) ) {
    case VK_TAB:
	if ( isUIActive ) {
	    bool shift = ::GetKeyState(VK_SHIFT) < 0;
	    QFocusData *data = ((HackWidget*)qt.widget)->focusData();
	    bool giveUp = TRUE;
	    if ( shift ) {
		if ( qt.widget->focusWidget() != data->first() ) {
		    giveUp = FALSE;
		    ((HackWidget*)qt.widget)->focusNextPrevChild( FALSE );
		    if ( qt.widget->focusWidget() == data->last() )
			giveUp = TRUE;
		}
	    } else {
		if ( qt.widget->focusWidget() != data->last() ) {
		    giveUp = FALSE;
		    ((HackWidget*)qt.widget)->focusNextPrevChild( TRUE );
		}
	    }
	    if ( giveUp ) {
		HWND hwnd = ::GetParent( m_hWndCD );
		::SetFocus( hwnd );
	    } else {
		return S_OK;
	    }
	}
	break;

    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
	if ( isUIActive )
	    return S_FALSE;
	break;

    default:
	if (isUIActive && qt.widget->focusWidget()) {
	    int state = NoButton;
	    if (dwKeyMod & 1)
		state |= ShiftButton;
	    if (dwKeyMod & 2)
		state |= ControlButton;
	    if (dwKeyMod & 4)
		state |= AltButton;

	    int key = qt_translateKeyCode(pMsg->wParam);
	    QKeyEvent override(QEvent::AccelOverride, key, 0, state);
	    override.ignore();
	    QApplication::sendEvent(qt.widget->focusWidget(), &override);
	    if (override.isAccepted())
		return S_FALSE;
	}
	break;
    }

    if ( !m_spClientSite )
	return S_FALSE;

    IOleControlSite *controlSite = 0;
    m_spClientSite->QueryInterface( IID_IOleControlSite, (void**)&controlSite );
    if ( !controlSite )
	return S_FALSE;

    HRESULT hres = controlSite->TranslateAcceleratorW(pMsg, dwKeyMod);

    controlSite->Release();

    return hres;
}

HRESULT WINAPI QAxServerBase::TranslateAcceleratorA( MSG *pMsg )
{
    return TranslateAcceleratorW( pMsg );
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
    if (!isWidget)
	return S_OK;

    EnableWindow(qt.widget->winId(), fEnable);
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
	if ( !qt.widget || !isWidget || qt.widget->caption().isEmpty() )
	    *pszUserType = QStringToOLESTR( class_name );
	else
	    *pszUserType = QStringToOLESTR( qt.widget->caption() );
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
    if ( dwSaveOption != OLECLOSE_NOSAVE && m_spClientSite )
	m_spClientSite->SaveObject();
    if ( isInPlaceActive ) {
	HRESULT hr = InPlaceDeactivate();
	if (FAILED(hr))
	    return hr;
    }
    if ( m_hWndCD ) {
	if ( IsWindow(m_hWndCD) )
	    DestroyWindow(m_hWndCD);
	m_hWndCD = 0;
	if ( m_spClientSite )
	    m_spClientSite->OnShowWindow( FALSE );
    }

    if ( m_spInPlaceSite ) m_spInPlaceSite->Release();
    m_spInPlaceSite = 0;

    if ( m_spAdviseSink )
	m_spAdviseSink->OnClose();
    return S_OK;
}

bool qax_disable_inplaceframe = FALSE;

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

    OnAmbientPropertyChange( DISPID_AMBIENT_USERMODE );

    if ( isWidget ) {
	HWND hwndParent;
	if ( m_spInPlaceSite->GetWindow(&hwndParent) == S_OK ) {
	    m_spInPlaceSite->GetWindowContext(&m_spInPlaceFrame, &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);

	    if (m_hWndCD) {
		::ShowWindow(m_hWndCD, SW_SHOW);
		if (!::IsChild(m_hWndCD, ::GetFocus()) && qt.widget->focusPolicy() != QWidget::NoFocus )
		    ::SetFocus(m_hWndCD);
	    } else {
		create(hwndParent, rcPos);
	    }

	    if ( !qt.widget->testWState( WState_Resized ) )
		SetObjectRects(&rcPos, &rcClip);
	}

	// Gone active by now, take care of UIACTIVATE
	canTakeFocus = qt.widget->focusPolicy() != QWidget::NoFocus && !inDesignMode;
	if ( !canTakeFocus && !inDesignMode ) {
	    QObjectList *list = qt.widget->queryList();
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
		    menuBar = ( qt.widget && !qax_disable_inplaceframe ) ? (QMenuBar*)qt.widget->child( 0, "QMenuBar" ) : 0;
		    if ( menuBar && !menuBar->isVisible() ) {
			createMenu( menuBar );
			menuBar->hide();
			menuBar->installEventFilter( this );
		    }
		    statusBar = qt.widget ? (QStatusBar*)qt.widget->child( 0, "QStatusBar" ) : 0;
		    if ( statusBar && !statusBar->isVisible() ) {
			const int index = statusBar->metaObject()->findSignal( "messageChanged(const QString&)" );
			connectInternal( statusBar, index, (QObject*)this, 2, -1 );
			statusBar->hide();
			statusBar->installEventFilter( this );
		    }
		}
	    }
	    if ( spInPlaceUIWindow ) {
		spInPlaceUIWindow->SetActiveObject( this, QStringToBSTR(class_name) );
		spInPlaceUIWindow->SetBorderSpace(0);
		spInPlaceUIWindow->Release();
	    }
	}
	ShowWindow( m_hWnd, SW_NORMAL );
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
    if ( m_spClientSite ) m_spClientSite->Release();

    m_spClientSite = pClientSite;
    if ( m_spClientSite ) m_spClientSite->AddRef();
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
    if ( !isWidget || !qt.widget ) {
	sizeExtent = *psizel;
	return S_OK;
    }

    QSize minSizeHint = qt.widget->minimumSizeHint();
    if ( minSizeHint.isValid() ) {
	QPaintDeviceMetrics pmetric( qt.widget );

	SIZEL minSize;
	minSize.cx = MAP_PIX_TO_LOGHIM( minSizeHint.width(), pmetric.logicalDpiX() );
	minSize.cy = MAP_PIX_TO_LOGHIM( minSizeHint.height(), pmetric.logicalDpiY() );

	psizel->cx = QMAX( minSize.cx, psizel->cx );
	psizel->cy = QMAX( minSize.cy, psizel->cy );
    }

    if ( psizel->cx != sizeExtent.cx || psizel->cy != sizeExtent.cy )
	sizeExtent = *psizel;

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

//**** IDataObject
/*
    Calls IViewObject::Draw after setting up the parameters.
*/
HRESULT WINAPI QAxServerBase::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    if (!pmedium)
	return E_POINTER;
    if (!qt.widget)
	return E_UNEXPECTED;

    memset(pmedium, 0, sizeof(STGMEDIUM));

    if ((pformatetcIn->tymed & TYMED_MFPICT) == 0)
	return DATA_E_FORMATETC;

    QPaintDeviceMetrics pmetric( qt.widget );
    int width = MAP_LOGHIM_TO_PIX(sizeExtent.cx, pmetric.logicalDpiX());
    int height = MAP_LOGHIM_TO_PIX(sizeExtent.cy, pmetric.logicalDpiY());
    RECTL rectl = {0, 0, width, height};

    HDC hdc = CreateMetaFile(0);
    SaveDC(hdc);
    SetWindowOrgEx(hdc, 0, 0, 0);
    SetWindowExtEx(hdc, rectl.right, rectl.bottom, 0);

    Draw(pformatetcIn->dwAspect, pformatetcIn->lindex, 0, pformatetcIn->ptd, 0, hdc, &rectl, &rectl, 0, 0);

    RestoreDC(hdc, -1);
    HMETAFILE hMF = CloseMetaFile(hdc);
    if (!hMF)
	return E_UNEXPECTED;

    HGLOBAL hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(METAFILEPICT));
    if (!hMem) {
	DeleteMetaFile(hMF);
	return ResultFromScode(STG_E_MEDIUMFULL);
    }

    LPMETAFILEPICT pMF = (LPMETAFILEPICT)GlobalLock(hMem);
    pMF->hMF = hMF;
    pMF->mm = MM_ANISOTROPIC;
    pMF->xExt = sizeExtent.cx;
    pMF->yExt = sizeExtent.cy;
    GlobalUnlock(hMem);

    pmedium->tymed = TYMED_MFPICT;
    pmedium->hGlobal = hMem;
    pmedium->pUnkForRelease = 0;

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::DAdvise(FORMATETC * /*pformatetc*/, DWORD /*advf*/, 
				      IAdviseSink * /*pAdvSink*/, DWORD * /*pdwConnection*/)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::DUnadvise(DWORD /*dwConnection*/)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::EnumDAdvise(IEnumSTATDATA ** /*ppenumAdvise*/)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetDataHere(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::QueryGetData(FORMATETC* /* pformatetc */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetCanonicalFormatEtc(FORMATETC* /* pformatectIn */,FORMATETC* /* pformatetcOut */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetData(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */, BOOL /* fRelease */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::EnumFormatEtc(DWORD /* dwDirection */, IEnumFORMATETC** /* ppenumFormatEtc */)
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

/*
    \reimp
*/
bool QAxServerBase::eventFilter( QObject *o, QEvent *e )
{
    if ( !theObject )
	return QObject::eventFilter( o, e );

    if ((e->type() == QEvent::Show || e->type() == QEvent::Hide) && (o == statusBar || o == menuBar)) {
	if ( o == menuBar ){
	    if ( e->type() == QEvent::Hide ) {
		createMenu( menuBar );
	    } else if ( e->type() == QEvent::Show ) {
		removeMenu();
	    }
	} else if ( statusBar ) {
	    statusBar->setSizeGripEnabled( FALSE );
	}
	updateGeometry();
	if ( m_spInPlaceSite ) {
	    RECT rect;
	    rect.left = rcPos.left;
	    rect.right = rcPos.left + qt.widget->sizeHint().width();
	    rect.top = rcPos.top;
	    rect.bottom = rcPos.top + qt.widget->sizeHint().height();
	    m_spInPlaceSite->OnPosRectChange( &rect );
	}
    }

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
	if ( o == qt.object && hasStockEvents ) {
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
	if ( o == qt.object && hasStockEvents ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    QUObject obj[3];
	    static_QUType_int.set( obj+1, ke->key() );
	    static_QUType_int.set( obj+2, mapModifiers( ke->state() ) );
	    qt_emit( DISPID_KEYUP, obj );
	}
	break;
    case QEvent::MouseMove:
	if ( o == qt.object && hasStockEvents ) {
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
	if ( o == qt.object && hasStockEvents ) {
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
	if ( o == qt.object && hasStockEvents ) {
	    QMouseEvent *me = (QMouseEvent*)e;
	    qt_emit( DISPID_DBLCLICK, 0 );
	}
	break;
    case QEvent::MouseButtonPress:
	if ( o == qt.widget && hasStockEvents ) {
	    QMouseEvent *me = (QMouseEvent*)e;
	    QUObject obj[5]; // 0 = return value
	    static_QUType_int.set( obj+1, me->button() );
	    static_QUType_int.set( obj+2, mapModifiers( me->state() ) );
	    static_QUType_int.set( obj+3, me->x() );
	    static_QUType_int.set( obj+4, me->y() );
	    qt_emit( DISPID_MOUSEDOWN, obj );
	}
	break;
    case QEvent::Show:
	if (m_hWnd && o == qt.widget)
	    ShowWindow(m_hWnd, SW_SHOW);
	updateMask();
	break;
    case QEvent::Hide:
	if (m_hWnd && o == qt.widget)
	    ShowWindow(m_hWnd, SW_HIDE);
	break;
    case QEvent::Resize:
	updateMask();
	break;
    default:
	break;
    }

    return QObject::eventFilter( o, e );
}

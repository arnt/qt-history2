/****************************************************************************
** $Id: $
**
** Implementation of the QAxWidget class
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
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

#include "qaxwidget.h"

#include <qapplication.h>
#include <qfocusdata.h>
#include <qobjectlist.h>
#include <qstatusbar.h>
#include <qguardedptr.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmetaobject.h>
#include <qpaintdevicemetrics.h>
#include <qregexp.h>
#include <qwhatsthis.h>

#include <olectl.h>

#include "../shared/types.h"

static HHOOK hhook = 0;
static int hhookref = 0;

static ushort mouseTbl[] = {
    WM_MOUSEMOVE,	QEvent::MouseMove,		0,
    WM_LBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::LeftButton,
    WM_LBUTTONUP,	QEvent::MouseButtonRelease,	Qt::LeftButton,
    WM_LBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::LeftButton,
    WM_RBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::RightButton,
    WM_RBUTTONUP,	QEvent::MouseButtonRelease,	Qt::RightButton,
    WM_RBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::RightButton,
    WM_MBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::MidButton,
    WM_MBUTTONUP,	QEvent::MouseButtonRelease,	Qt::MidButton,
    WM_MBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::MidButton,
    0,			0,				0
};

static int translateButtonState( int s, int type, int button )
{
    int bst = 0;
    if ( s & MK_LBUTTON )
	bst |= Qt::LeftButton;
    if ( s & MK_MBUTTON )
	bst |= Qt::MidButton;
    if ( s & MK_RBUTTON )
	bst |= Qt::RightButton;
    if ( s & MK_SHIFT )
	bst |= Qt::ShiftButton;
    if ( s & MK_CONTROL )
	bst |= Qt::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	bst |= Qt::AltButton;

    // Translate from Windows-style "state after event"
    // to X-style "state before event"
    if ( type == QEvent::MouseButtonPress ||
	 type == QEvent::MouseButtonDblClick )
	bst &= ~button;
    else if ( type == QEvent::MouseButtonRelease )
	bst |= button;

    return bst;
}

/*  \class QAxHostWidget qaxwidget.cpp
    \brief The QAxHostWindow class is the actual container widget.
    \internal
*/
class QAxHostWidget : public QWidget
{
    friend class QAxHostWindow;
public:
    QAxHostWidget( QWidget *parent, QAxHostWindow *ax );
    ~QAxHostWidget();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void show();

    bool qt_emit( int isignal, QUObject *obj );
    QAxHostWindow *clientSite() const
    {
	return axhost;
    }

protected:
    bool event( QEvent *e );
    void resizeEvent( QResizeEvent *e );
    void focusInEvent( QFocusEvent *e );
    void focusOutEvent( QFocusEvent *e );
    void windowActivationChange( bool oldActive );
    bool focusNextPrevChild( bool next );

    QAxHostWindow *axhost;

private:
    int setFocusTimer;
};

QAxHostWidget::QAxHostWidget( QWidget *parent, QAxHostWindow *ax )
    : QWidget( parent, "QAxHostWidget", WResizeNoErase|WRepaintNoErase ), axhost( ax )
{
    setBackgroundMode( NoBackground );
    setFocusTimer = 0;
}

QAxHostWidget::~QAxHostWidget()
{
}


/*  \class QAxHostWindow qaxwidget.cpp
    \brief The QAxHostWindow class implements the client site interfaces.
    \internal
*/
class QAxHostWindow : public IDispatch,
		      public IOleClientSite,
		      public IOleControlSite,
		      public IOleInPlaceSite,
		      public IOleInPlaceFrame,
		      public IAdviseSink
{
    friend class QAxHostWidget;
public:
    QAxHostWindow( QAxWidget *c, bool inited );
    ~QAxHostWindow();
    void releaseAll();
    void deactivate();
    QWidget *hostWidget() const
    {
	return host;
    }
    IOleInPlaceActiveObject *inPlaceObject() const
    {
	return m_spInPlaceActiveObject;
    }

// IUnknown
    unsigned long WINAPI AddRef();
    unsigned long WINAPI Release();
    STDMETHOD(QueryInterface)(REFIID iid, void **iface );

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
			      UINT *puArgErr );
    void emitAmbientPropertyChange( DISPID dispid );

// IOleClientSite
    STDMETHOD(SaveObject)();
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk );
    STDMETHOD(GetContainer)(LPOLECONTAINER FAR* ppContainer);
    STDMETHOD(ShowObject)();
    STDMETHOD(OnShowWindow)(BOOL fShow);
    STDMETHOD(RequestNewObjectLayout)();

// IOleControlSite
    STDMETHOD(OnControlInfoChanged)();
    STDMETHOD(LockInPlaceActive)(BOOL fLock);
    STDMETHOD(GetExtendedControl)(IDispatch** ppDisp);
    STDMETHOD(TransformCoords)(POINTL* pPtlHimetric, POINTF* pPtfContainer, DWORD dwFlags);
    STDMETHOD(TranslateAcceleratorA)(LPMSG lpMsg, DWORD grfModifiers);
    STDMETHOD(TranslateAcceleratorW)(LPMSG lpMsg, DWORD grfModifiers);
    STDMETHOD(OnFocus)(BOOL fGotFocus );
    STDMETHOD(ShowPropertyFrame)();

// IOleWindow
    STDMETHOD(GetWindow)(HWND *phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

// IOleInPlaceSite
    STDMETHOD(CanInPlaceActivate)();
    STDMETHOD(OnInPlaceActivate)();
    STDMETHOD(OnUIActivate)();
    STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo );
    STDMETHOD(Scroll)(SIZE scrollExtant);
    STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
    STDMETHOD(OnInPlaceDeactivate)();
    STDMETHOD(DiscardUndoState)();
    STDMETHOD(DeactivateAndUndo)();
    STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

// IOleInPlaceFrame
    STDMETHOD(InsertMenus( HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths ));
    STDMETHOD(SetMenu( HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject ));
    STDMETHOD(RemoveMenus( HMENU hmenuShared ));
    STDMETHOD(SetStatusText( LPCOLESTR pszStatusText ));
    STDMETHOD(EnableModeless( BOOL fEnable ));
    STDMETHOD(TranslateAcceleratorA( LPMSG lpMsg, WORD grfModifiers ));
    STDMETHOD(TranslateAcceleratorW( LPMSG lpMsg, WORD grfModifiers ));

// IOleInPlaceUIWindow
    STDMETHOD(GetBorder( LPRECT lprectBorder ));
    STDMETHOD(RequestBorderSpace( LPCBORDERWIDTHS pborderwidths ));
    STDMETHOD(SetBorderSpace( LPCBORDERWIDTHS pborderwidths ));
    STDMETHOD(SetActiveObject( IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName ));

// IAdviseSink
    STDMETHOD_(void, OnDataChange)(FORMATETC* /*pFormatetc*/, STGMEDIUM* /*pStgmed*/)
    {
    }
    STDMETHOD_(void, OnViewChange)(DWORD /*dwAspect*/, LONG /*lindex*/)
    {
    }
    STDMETHOD_(void, OnRename)(IMoniker* /*pmk*/)
    {
    }
    STDMETHOD_(void, OnSave)()
    {
    }
    STDMETHOD_(void, OnClose)()
    {
    }

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    bool invisibleAtRuntime() const;

    bool qt_emit( int isignal, QUObject *obj );

protected:
    void windowActivationChange( bool oldActive );

private:
    struct OleMenuItem {
	OleMenuItem( HMENU hm = 0, int ID = 0, QPopupMenu *pop = 0 )
	    : hMenu(hm), id(ID), subMenu(pop)
	{}
	HMENU hMenu;
	int id;
	QPopupMenu *subMenu;
    };
    QPopupMenu *generatePopup( HMENU subMenu, QWidget *parent );

    IOleObject *m_spOleObject;
    IOleControl *m_spOleControl;
    IOleInPlaceObject *m_spInPlaceObject;
    IOleInPlaceActiveObject *m_spInPlaceActiveObject;

    bool inPlaceModelessEnabled :1;

    DWORD m_dwOleObject;
    HWND m_menuOwner;
    CONTROLINFO controlInfo;

    QSize sizehint;

    unsigned long ref;
    QGuardedPtr<QAxWidget> widget;
    QGuardedPtr<QAxHostWidget> host;
    QGuardedPtr<QStatusBar> statusBar;
    QGuardedPtr<QMenuBar> menuBar;
    QMap<int,OleMenuItem> menuItemMap;
};

// The filter procedure listening to user interaction on the control
LRESULT CALLBACK FilterProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    static bool reentrant = FALSE;
    static QPoint pos;
    static POINT gpos={-1,-1};
    QEvent::Type type;				// event parameters
    int	   button;
    int	   state;
    int	   i;

    if ( !reentrant && lParam ) {
	reentrant = TRUE;
	MSG *msg = (MSG*)lParam;
	const uint message = msg->message;
	if ( ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST )
	   ||( message >= WM_KEYFIRST && message <= WM_KEYLAST ) ) {
	    HWND hwnd = msg->hwnd;
	    QWidget *widget = 0;
	    QAxWidget *ax = 0;
	    while ( !ax && hwnd ) {
		widget = QWidget::find( hwnd );
		if ( widget )
		    ax = (QAxWidget*)widget->qt_cast( "QAxWidget" );
		hwnd = ::GetParent( hwnd );
	    }
	    if ( ax && msg->hwnd != ax->winId() ) {
		if ( message >= WM_KEYFIRST && message <= WM_KEYLAST ) {
		    QAxHostWidget *host = (QAxHostWidget*)ax->child( "QAxHostWidget", "QWidget" );
		    if ( host ) {
			QAxHostWindow *site = host->clientSite();
			bool eaten = FALSE;
			// give the control a chance, otherwise propagate to Qt
			if ( site && site->inPlaceObject() )
			    eaten = !site->inPlaceObject()->TranslateAccelerator( msg );
			if ( !eaten )
			    SendMessage( host->winId(), message, msg->wParam, msg->lParam );
		    }
		} else {
		    for ( i=0; (UINT)mouseTbl[i] != message && mouseTbl[i]; i += 3 )
			;
		    if ( !mouseTbl[i] )
			return CallNextHookEx( hhook, nCode, wParam, lParam );
		    type   = (QEvent::Type)mouseTbl[++i];	// event type
		    button = mouseTbl[++i];			// which button
		    state  = translateButtonState( msg->wParam, type, button ); // button state
		    DWORD ol_pos = GetMessagePos();
		    gpos.x = LOWORD(ol_pos);
		    gpos.y = HIWORD(ol_pos);
		    pos = widget->mapFromGlobal( QPoint(gpos.x, gpos.y) );

		    QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
		    QApplication::sendEvent( ax, &e );
		}
	    }
	}
	reentrant = FALSE;
    }
    return CallNextHookEx( hhook, nCode, wParam, lParam );
}

QAxHostWindow::QAxHostWindow( QAxWidget *c, bool bInited )
: ref(1), widget( c )
{
    host = new QAxHostWidget( widget, this );

    m_spOleObject = 0;
    m_spOleControl = 0;
    m_spInPlaceObject = 0;
    m_spInPlaceActiveObject = 0;

    inPlaceModelessEnabled = TRUE;
    m_dwOleObject = 0;
    m_menuOwner = 0;
    memset(&controlInfo, 0, sizeof(controlInfo));

    statusBar = 0;
    menuBar = 0;

    HRESULT hr = S_OK;

    m_spOleObject = 0;
    widget->queryInterface( IID_IOleObject, (void**)&m_spOleObject );
    if (m_spOleObject) {
	DWORD dwMiscStatus = 0;
	m_spOleObject->GetMiscStatus( DVASPECT_CONTENT, &dwMiscStatus );
	if( dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST )
	    m_spOleObject->SetClientSite( this );

	if ( !bInited ) {
	    IPersistStreamInit *spPSI = 0;
	    m_spOleObject->QueryInterface( IID_IPersistStreamInit, (void**)&spPSI );
	    if ( spPSI ) {
		spPSI->InitNew();
		spPSI->Release();
	    }
	}

	if( !(dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) )
	    m_spOleObject->SetClientSite( this );
	
	IViewObject *spViewObject = 0;
	m_spOleObject->QueryInterface(IID_IViewObject, (void**) &spViewObject);
	
	m_spOleObject->Advise( this, &m_dwOleObject );
	IAdviseSink *spAdviseSink = 0;
	QueryInterface( IID_IAdviseSink, (void**)&spAdviseSink );
	if ( spAdviseSink && spViewObject ) {
	    if ( spViewObject )
		spViewObject->SetAdvise( DVASPECT_CONTENT, 0, spAdviseSink );
	    spAdviseSink->Release();
	}
	if ( spViewObject )
	    spViewObject->Release();

	m_spOleObject->SetHostNames( OLESTR("AXWIN"), 0 );
	QPaintDeviceMetrics pdm( widget );

	if ( !(dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME) ) {
	    SIZEL hmSize;
	    hmSize.cx = MAP_PIX_TO_LOGHIM( 250, pdm.logicalDpiX() );
	    hmSize.cy = MAP_PIX_TO_LOGHIM( 250, pdm.logicalDpiY() );

	    m_spOleObject->SetExtent( DVASPECT_CONTENT, &hmSize );
	    m_spOleObject->GetExtent( DVASPECT_CONTENT, &hmSize );

	    sizehint.setWidth( MAP_LOGHIM_TO_PIX( hmSize.cx, pdm.logicalDpiX() ) );
	    sizehint.setHeight( MAP_LOGHIM_TO_PIX( hmSize.cy, pdm.logicalDpiY() ) );
	} else {
	    sizehint = QSize( 0, 0 );
	}
	if ( !(dwMiscStatus & OLEMISC_NOUIACTIVATE ) ) {
	    host->setFocusPolicy( QWidget::StrongFocus );
	} else {
	    host->setFocusPolicy( QWidget::NoFocus );
	}

	RECT rcPos = { host->x(), host->y(), 
		       host->x()+sizehint.width(), host->y()+sizehint.height() };

	hr = m_spOleObject->DoVerb( OLEIVERB_INPLACEACTIVATE, 0, (IOleClientSite*)this, 0, host->winId(), &rcPos );

	m_spOleObject->QueryInterface( IID_IOleControl, (void**)&m_spOleControl );
	if ( m_spOleControl ) {
	    m_spOleControl->OnAmbientPropertyChange( DISPID_AMBIENT_BACKCOLOR );
	    m_spOleControl->OnAmbientPropertyChange( DISPID_AMBIENT_FORECOLOR );
	    m_spOleControl->OnAmbientPropertyChange( DISPID_AMBIENT_FONT );

	    controlInfo.cb = sizeof(controlInfo);
	    m_spOleControl->GetControlInfo( &controlInfo );
	}

	BSTR userType;
	m_spOleObject->GetUserType( USERCLASSTYPE_SHORT, &userType );
	widget->setCaption( BSTRToQString( userType ) );
	CoTaskMemFree( userType );
    }
    IObjectWithSite *spSite;
    widget->queryInterface(IID_IObjectWithSite, (void**)&spSite);
    if ( spSite ) {
	spSite->SetSite( (IUnknown*)(IDispatch*)this );
	spSite->Release();
    }
}

QAxHostWindow::~QAxHostWindow()
{
    if ( host )
	host->axhost = 0;
}

void QAxHostWindow::releaseAll()
{
    if ( m_spOleObject ) {
	m_spOleObject->Unadvise( m_dwOleObject );
	m_spOleObject->Release();
    }
    m_spOleObject = 0;
    if ( m_spOleControl ) m_spOleControl->Release();
    m_spOleControl = 0;
    if ( m_spInPlaceObject ) m_spInPlaceObject->Release();
    m_spInPlaceObject = 0;
    if ( m_spInPlaceActiveObject ) m_spInPlaceActiveObject->Release();
    m_spInPlaceActiveObject = 0;
}

void QAxHostWindow::deactivate()
{
    if ( m_spInPlaceObject ) m_spInPlaceObject->InPlaceDeactivate();
    // if this assertion fails the control didn't call OnInPlaceDeactivate
    Q_ASSERT( m_spInPlaceObject == 0 ); 
}

//**** IUnknown
unsigned long WINAPI QAxHostWindow::AddRef()
{
    return ++ref;
}

unsigned long WINAPI QAxHostWindow::Release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

HRESULT WINAPI QAxHostWindow::QueryInterface( REFIID iid, void **iface )
{
    *iface = 0;
    
    if ( iid == IID_IUnknown )
	*iface = (IUnknown*)(IDispatch*)this;
    else if ( iid == IID_IDispatch )
	*iface = (IDispatch*)this;
    else if ( iid == IID_IOleClientSite )
	*iface = (IOleClientSite*)this;
    else if ( iid == IID_IOleControlSite )
	*iface = (IOleControlSite*)this;
    else if ( iid == IID_IOleWindow )
	*iface = (IOleWindow*)(IOleInPlaceSite*)this;
    else if ( iid == IID_IOleInPlaceSite )
	*iface = (IOleInPlaceSite*)this;
    else if ( iid == IID_IOleInPlaceFrame )
	*iface = (IOleInPlaceFrame*)this;
    else if ( iid == IID_IOleInPlaceUIWindow )
	*iface = (IOleInPlaceUIWindow*)this;
    else if ( iid == IID_IAdviseSink )
	*iface = (IAdviseSink*)this;
    else
	return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

#if defined(QT_PLUGIN)
extern bool runsInDesignMode;
#endif

//**** IDispatch
HRESULT WINAPI QAxHostWindow::Invoke(DISPID dispIdMember,
			      REFIID /*riid*/,
			      LCID /*lcid*/,
			      WORD /*wFlags*/,
			      DISPPARAMS * /*pDispParams*/,
			      VARIANT *pVarResult,
			      EXCEPINFO * /*pExcepInfo*/,
			      UINT * /*puArgErr*/ )
{
    if ( !pVarResult )
	return E_POINTER;

    switch( dispIdMember ) {
    case DISPID_AMBIENT_USERMODE:
#if defined(QT_PLUGIN)
	pVarResult->vt = VT_BOOL;
	if ( runsInDesignMode && hostWidget() ) {
	    bool isPreview = !hostWidget()->topLevelWidget()->inherits( "MainWindow" );
	    pVarResult->boolVal = isPreview;
	} else {
	    pVarResult->boolVal = TRUE;
	}
	return S_OK;
#endif

    case DISPID_AMBIENT_AUTOCLIP:	
    case DISPID_AMBIENT_SUPPORTSMNEMONICS:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = TRUE;
	return S_OK;

    case DISPID_AMBIENT_SHOWHATCHING:
    case DISPID_AMBIENT_SHOWGRABHANDLES:
    case DISPID_AMBIENT_DISPLAYASDEFAULT:
    case DISPID_AMBIENT_MESSAGEREFLECT:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = FALSE;
	return S_OK;

    case DISPID_AMBIENT_DISPLAYNAME:
	pVarResult->vt = VT_BSTR;
	pVarResult->bstrVal = QStringToBSTR( widget->caption() );
	return S_OK;

    case DISPID_AMBIENT_FONT:
	pVarResult->vt = VT_DISPATCH;
	pVarResult->pdispVal = QFontToIFont( widget->font() );
	return S_OK;

    case DISPID_AMBIENT_BACKCOLOR:
	pVarResult->vt = VT_UI4;
	pVarResult->lVal = QColorToOLEColor( widget->paletteBackgroundColor() );
	return S_OK;

    case DISPID_AMBIENT_FORECOLOR:
	pVarResult->vt = VT_UI4;
	pVarResult->lVal = QColorToOLEColor( widget->paletteForegroundColor() );
	return S_OK;

    case DISPID_AMBIENT_UIDEAD:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = !widget->isEnabled();
	return S_OK;

    default:
	break;
    }

    return DISP_E_MEMBERNOTFOUND;
}

void QAxHostWindow::emitAmbientPropertyChange( DISPID dispid )
{
    if ( m_spOleControl )
	m_spOleControl->OnAmbientPropertyChange( dispid );
}

//**** IOleClientSite
HRESULT WINAPI QAxHostWindow::SaveObject()
{
    return E_NOTIMPL;
}

HRESULT WINAPI QAxHostWindow::GetMoniker( DWORD, DWORD, IMoniker **ppmk )
{
    if ( !ppmk )
	return E_POINTER;

    *ppmk = 0;
    return E_NOTIMPL;
}

HRESULT WINAPI QAxHostWindow::GetContainer( LPOLECONTAINER *ppContainer )
{
    if ( !ppContainer )
	return E_POINTER;

    *ppContainer = 0;
    return E_NOINTERFACE;
}

HRESULT WINAPI QAxHostWindow::ShowObject()
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::OnShowWindow( BOOL /*fShow*/ )
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::RequestNewObjectLayout()
{
    return E_NOTIMPL;
}

//**** IOleControlSite
HRESULT WINAPI QAxHostWindow::OnControlInfoChanged()
{
    if ( m_spOleControl )
	m_spOleControl->GetControlInfo( &controlInfo );

    return S_OK;
}

HRESULT WINAPI QAxHostWindow::LockInPlaceActive(BOOL /*fLock*/)
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::GetExtendedControl(IDispatch** ppDisp)
{
    if ( !ppDisp )
	return E_POINTER;
    
    *ppDisp = 0;
    return E_NOTIMPL;
}

HRESULT WINAPI QAxHostWindow::TransformCoords(POINTL* /*pPtlHimetric*/, POINTF* /*pPtfContainer*/, DWORD /*dwFlags*/)
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::TranslateAcceleratorA(LPMSG /*lpMsg*/, DWORD /*grfModifiers*/)
{
    return S_FALSE;
}

HRESULT WINAPI QAxHostWindow::TranslateAcceleratorW(LPMSG /*lpMsg*/, DWORD /*grfModifiers*/)
{
    return S_FALSE;
}

HRESULT WINAPI QAxHostWindow::OnFocus( BOOL /*bGotFocus*/ )
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::ShowPropertyFrame()
{
    return E_NOTIMPL;
}

//**** IOleWindow
HRESULT WINAPI QAxHostWindow::GetWindow( HWND *phwnd )
{
    if ( !phwnd )
	return E_POINTER;

    *phwnd = host->winId();
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::ContextSensitiveHelp( BOOL fEnterMode )
{
    if ( fEnterMode )
	QWhatsThis::enterWhatsThisMode();
    else
	QWhatsThis::leaveWhatsThisMode();

    return S_OK;
}

//**** IOleInPlaceSite
HRESULT WINAPI QAxHostWindow::CanInPlaceActivate()
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::OnInPlaceActivate()
{
    OleLockRunning( m_spOleObject, TRUE, FALSE );
    m_spOleObject->QueryInterface( IID_IOleInPlaceObject, (void**) &m_spInPlaceObject );
    if ( m_spInPlaceObject ) {
	RECT rcPos = { host->x(), host->y(), host->x()+host->width(), host->y()+host->height() };
	m_spInPlaceObject->SetObjectRects( &rcPos, &rcPos );
    }
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::OnUIActivate()
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::GetWindowContext( IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo )
{
    if ( !ppFrame || !ppDoc || !lprcPosRect || !lprcClipRect || !lpFrameInfo )
	return E_POINTER;

    QueryInterface(IID_IOleInPlaceFrame, (void**)ppFrame);
    QueryInterface(IID_IOleInPlaceUIWindow, (void**)ppDoc);

    ::GetClientRect( host->winId(), lprcPosRect );
    ::GetClientRect( host->winId(), lprcClipRect );

    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp = FALSE;
    ACCEL ac[] = {
	{ 0,VK_TAB,12 },
	{ 0, 0, 0 }
    };
    lpFrameInfo->haccel = CreateAcceleratorTable(ac, 2);
    lpFrameInfo->cAccelEntries = 2;
    lpFrameInfo->hwndFrame = widget->topLevelWidget()->winId();

    return S_OK;
}

HRESULT WINAPI QAxHostWindow::Scroll( SIZE /*scrollExtant*/ )
{
    return S_FALSE;
}

HRESULT WINAPI QAxHostWindow::OnUIDeactivate( BOOL )
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::OnInPlaceDeactivate()
{
    if ( m_spInPlaceObject )
	m_spInPlaceObject->Release();
    m_spInPlaceObject = 0;
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::DiscardUndoState()
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::DeactivateAndUndo()
{
    IOleInPlaceObject *inPlace = 0;
    widget->queryInterface( IID_IOleInPlaceObject, (void**)&inPlace );
    if ( inPlace ) {
	inPlace->UIDeactivate();
	inPlace->Release();
    }

    return S_OK;
}

HRESULT WINAPI QAxHostWindow::OnPosRectChange( LPCRECT /*lprcPosRect*/ )
{
    // ###
    return S_OK;
}

//**** IOleInPlaceFrame
HRESULT WINAPI QAxHostWindow::InsertMenus( HMENU /*hmenuShared*/, LPOLEMENUGROUPWIDTHS lpMenuWidths )
{
    QMenuBar *mb = menuBar;
    QWidget *p = widget;
    while ( !mb && p ) {
	mb = (QMenuBar*)p->child( 0, "QMenuBar" );
	p = p->parentWidget( TRUE );
    }
    if ( !mb )
	return E_NOTIMPL;
    menuBar = mb;

    QPopupMenu *fileMenu = 0;
    QPopupMenu *viewMenu = 0;
    QPopupMenu *windowMenu = 0;
    for ( uint i = 0; i < mb->count(); ++i ) {
	int id = mb->idAt( i );
	QString menuText = mb->text( id );
	if ( menuText == "&File" ) {
	    QMenuItem *item = mb->findItem( id );
	    if ( item )
		fileMenu = item->popup();
	} else if ( menuText == "&View" ) {
	    QMenuItem *item = mb->findItem( id );
	    if ( item )
		viewMenu = item->popup();
	} else if ( menuText == "&Window" ) {
	    QMenuItem *item = mb->findItem( id );
	    if ( item )
		windowMenu = item->popup();
	}
    }
    if ( fileMenu )
	lpMenuWidths->width[0] = fileMenu->count();
    if ( viewMenu )
	lpMenuWidths->width[2] = viewMenu->count();
    if ( windowMenu )
	lpMenuWidths->width[4] = windowMenu->count();

    return S_OK;
}

static int menuItemEntry( HMENU menu, int index, MENUITEMINFOA item, QString &text, QPixmap &icon )
{
    if ( item.fType == MFT_STRING && item.cch ) {
	char *titlebuf = new char[item.cch+1];
	item.dwTypeData = titlebuf;
	item.cch++;
	::GetMenuItemInfoA( menu, index, TRUE, &item );
	text = QString::fromLocal8Bit( titlebuf );
	delete []titlebuf;
	return MFT_STRING;
    } 
#if 0
    else if ( item.fType == MFT_BITMAP ) {
	HBITMAP hbm = (HBITMAP)LOWORD(item.hbmpItem);
	SIZE bmsize;
	GetBitmapDimensionEx( hbm, &bmsize );
	QPixmap pixmap( 1,1 );
	QPaintDeviceMetrics pdm( &pixmap );
	QSize sz( MAP_LOGHIM_TO_PIX( bmsize.cx, pdm.logicalDpiX() ),
	    MAP_LOGHIM_TO_PIX( bmsize.cy, pdm.logicalDpiY() ) );

	pixmap.resize( bmsize.cx, bmsize.cy );
	if ( !pixmap.isNull() ) {
	    HDC hdc = ::CreateCompatibleDC( pixmap.handle() );
	    ::SelectObject( hdc, hbm );
	    BOOL res = ::BitBlt( pixmap.handle(), 0, 0, pixmap.width(), pixmap.height(), hdc, 0, 0, SRCCOPY );
	    ::DeleteObject( hdc );
	}

	icon = pixmap;
    }
#endif
    return -1;
}

QPopupMenu *QAxHostWindow::generatePopup( HMENU subMenu, QWidget *parent )
{
    QPopupMenu *popup = 0;
    int count = GetMenuItemCount( subMenu );
    if ( count )
	popup = new QPopupMenu( parent );
    for ( int i = 0; i < count; ++i ) {
	MENUITEMINFOA item;
	memset( &item, 0, sizeof(MENUITEMINFOA) );
	item.cbSize = sizeof(MENUITEMINFOA);
	item.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
	::GetMenuItemInfoA( subMenu, i, TRUE, &item );

	int qtid = 0;
	QPopupMenu *popupMenu = 0;
	if ( item.fType == MFT_SEPARATOR ) {
	    qtid = popup->insertSeparator();
	} else {
	    QString text;
	    QPixmap icon;
	    QKeySequence accel;
	    popupMenu = item.hSubMenu ? generatePopup( item.hSubMenu, popup ) : 0;
	    int res = menuItemEntry( subMenu, i, item, text, icon );

	    int lastSep = text.findRev( QRegExp("[\\s]") );
	    if ( lastSep != -1 ) {
		QString keyString = text.right( text.length() - lastSep );
		accel = keyString;
		if ( (int)accel )
		    text = text.left( lastSep-1 );
	    }

	    switch ( res ) {
	    case MFT_STRING:
		if ( popupMenu )
		    qtid = popup->insertItem( text, popupMenu );
		else
		    qtid = popup->insertItem( text );
		break;
	    case MFT_BITMAP:
		if ( popupMenu )
		    qtid = popup->insertItem( icon, popupMenu );
		else
		    qtid = popup->insertItem( icon );
		break;
	    }
	    if ( (int)accel )
		popup->setAccel( accel, qtid );
	}

	if ( qtid != 0 ) {
	    OleMenuItem oleItem( subMenu, item.wID, popupMenu );
	    menuItemMap.insert( qtid, oleItem );
	}
    }

    return popup;
}

HRESULT WINAPI QAxHostWindow::SetMenu( HMENU hmenuShared, HOLEMENU /*holemenu*/, HWND hwndActiveObject )
{
    if ( hmenuShared ) {
	m_menuOwner = hwndActiveObject;
	QMenuBar *mb = menuBar;
	QWidget *p = widget;
	while ( !mb && p ) {
	    mb = (QMenuBar*)p->child( 0, "QMenuBar" );
	    p = p->parentWidget( TRUE );
	}
	if ( !mb )
	    return E_NOTIMPL;
	menuBar = mb;

	int count = GetMenuItemCount( hmenuShared );
	for ( int i = 0; i < count; ++i ) {
	    MENUITEMINFOA item;
	    memset( &item, 0, sizeof(MENUITEMINFOA) );
	    item.cbSize = sizeof(MENUITEMINFOA);
	    item.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
	    ::GetMenuItemInfoA( hmenuShared, i, TRUE, &item );

	    int qtid = 0;
	    QPopupMenu *popupMenu = 0;
	    if ( item.fType == MFT_SEPARATOR ) {
		qtid = menuBar->insertSeparator();
	    } else {
		QString text;
		QPixmap icon;
		popupMenu = item.hSubMenu ? generatePopup( item.hSubMenu, menuBar ) : 0;

		switch( menuItemEntry( hmenuShared, i, item, text, icon ) ) {
		case MFT_STRING:
		    if ( popupMenu )
			qtid = menuBar->insertItem( text, popupMenu );
		    else
			qtid = menuBar->insertItem( text );
		    break;
		case MFT_BITMAP:
		    if ( popupMenu )
			qtid = menuBar->insertItem( icon, popupMenu );
		    else
			qtid = menuBar->insertItem( icon );
		    break;
		default:
		    break;
		}
	    }

	    if ( qtid != 0 ) {
		OleMenuItem oleItem( hmenuShared, item.wID, popupMenu );
		menuItemMap.insert( qtid, oleItem );
	    }
	}
	if ( count ) {
	    const QMetaObject *mbmo = menuBar->metaObject();
	    int index = mbmo->findSignal( "activated(int)" );
	    Q_ASSERT( index != -1 );
	    menuBar->disconnect( SIGNAL(activated(int)), host );
	    host->connectInternal( menuBar, index, host, 2, 0 );
	}
    } else if ( menuBar ) {
	m_menuOwner = 0;
	QMap<int, OleMenuItem>::Iterator it;
	for ( it = menuItemMap.begin(); it != menuItemMap.end(); ++it ) {
	    int qtid = it.key();
	    QMenuItem *qtitem = menuBar->findItem( qtid );
	    if ( qtitem ) {
		QPopupMenu *popupMenu = qtitem->popup();
		menuBar->removeItem( qtid );
		if ( popupMenu )
		    delete popupMenu;
	    }
	}
	menuItemMap.clear();
    }
    return S_OK;
}

bool QAxHostWindow::qt_emit( int isignal, QUObject *obj )
{
    if ( !m_spOleObject )
	return FALSE;

    switch ( isignal ) {
    case 0: // activated(int)
	{
	    int qtid = static_QUType_int.get( obj+1 );
	    OleMenuItem oleItem = menuItemMap[qtid];
	    if ( oleItem.hMenu )
		::PostMessageA( m_menuOwner, WM_COMMAND, oleItem.id, 0 );
	}
	break;

    default:
	return FALSE;
    }
    return TRUE;
}

HRESULT WINAPI QAxHostWindow::RemoveMenus( HMENU /*hmenuShared*/ )
{
    //###
    return E_NOTIMPL;
}

HRESULT WINAPI QAxHostWindow::SetStatusText( LPCOLESTR pszStatusText )
{
    QStatusBar *sb = statusBar;
    QWidget *p = widget;
    while ( !sb && p ) {
	sb = (QStatusBar*)p->child( 0, "QStatusBar" );
	p = p->parentWidget( TRUE );
    }
    if ( !sb )
	return E_NOTIMPL;
    statusBar = sb;

    sb->message( BSTRToQString( (BSTR)pszStatusText ) );
    return S_OK;
}

extern Q_EXPORT void qt_enter_modal(QWidget*);
extern Q_EXPORT void qt_leave_modal(QWidget*);

HRESULT WINAPI QAxHostWindow::EnableModeless( BOOL fEnable )
{
    if ( !fEnable )
	qt_enter_modal( host );
    else 
	qt_leave_modal( host );

    return S_OK;
}

HRESULT WINAPI QAxHostWindow::TranslateAcceleratorA( LPMSG lpMsg, WORD grfModifiers )
{
    return TranslateAcceleratorA( lpMsg, (DWORD)grfModifiers );
}

HRESULT WINAPI QAxHostWindow::TranslateAcceleratorW( LPMSG lpMsg, WORD grfModifiers )
{
    return TranslateAcceleratorW( lpMsg, (DWORD)grfModifiers );
}

//**** IOleInPlaceUIWindow
HRESULT WINAPI QAxHostWindow::GetBorder( LPRECT lprectBorder )
{
    RECT border = { 0,0, 100, 10 };
    *lprectBorder = border;
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::RequestBorderSpace( LPCBORDERWIDTHS /*pborderwidths*/ )
{
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::SetBorderSpace( LPCBORDERWIDTHS pborderwidths )
{
    if ( !pborderwidths )
	return S_OK;

    bool needsSpace = pborderwidths->left || pborderwidths->top || pborderwidths->right || pborderwidths->bottom;
    if ( !needsSpace ) {
	//### remove our toolbars
	return S_OK;
    }
    return S_OK;
}

HRESULT WINAPI QAxHostWindow::SetActiveObject( IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName )
{
    if ( pszObjName )
	widget->setCaption( BSTRToQString( (BSTR)pszObjName ) );

    if ( m_spInPlaceActiveObject )
	m_spInPlaceActiveObject->Release();

    m_spInPlaceActiveObject = pActiveObject;
    if ( m_spInPlaceActiveObject ) m_spInPlaceActiveObject->AddRef();

    return S_OK;
}

bool QAxHostWindow::invisibleAtRuntime() const
{
    if ( m_spOleObject ) {
	DWORD dwMiscStatus;
	m_spOleObject->GetMiscStatus( DVASPECT_CONTENT, &dwMiscStatus );
	if( dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME )
	    return TRUE;
    }
    return FALSE;
}

QSize QAxHostWindow::sizeHint() const
{
    return sizehint;
}

QSize QAxHostWindow::minimumSizeHint() const
{
    if ( !m_spOleObject )
	return QSize();

    SIZE sz = { 0, 0 };
    m_spOleObject->SetExtent( DVASPECT_CONTENT, &sz );
    HRESULT res = m_spOleObject->GetExtent( DVASPECT_CONTENT, &sz );
    if ( SUCCEEDED(res) ) {
	QPaintDeviceMetrics pmetric( widget );
	return QSize( MAP_LOGHIM_TO_PIX( sz.cx, pmetric.logicalDpiX() ),
		      MAP_LOGHIM_TO_PIX( sz.cy, pmetric.logicalDpiY() ) );
    }
    return QSize();
}

void QAxHostWindow::windowActivationChange( bool /*oldActive*/ )
{
    if ( m_spInPlaceActiveObject ) {
	QWidget *modal = QApplication::activeModalWidget();
	if ( modal && inPlaceModelessEnabled ) {
	    m_spInPlaceActiveObject->EnableModeless( FALSE );
	    inPlaceModelessEnabled = FALSE;
	} else if ( !inPlaceModelessEnabled ) {
	    m_spInPlaceActiveObject->EnableModeless( TRUE );
	    inPlaceModelessEnabled = TRUE;
	}
	m_spInPlaceActiveObject->OnFrameWindowActivate( widget->isActiveWindow() );
    }
}


//**** QWidget

bool QAxHostWidget::qt_emit( int isignal, QUObject *obj )
{
    if ( axhost )
	return axhost->qt_emit( isignal, obj );
    return FALSE;
}

void QAxHostWidget::show()
{
    if ( axhost && !axhost->invisibleAtRuntime() ) {
	QWidget::show();
	QApplication::sendPostedEvents( 0, QEvent::LayoutHint );
	int w = width();
	int h = height();
	resize( w-1, h-1 );
	resize( w, h );
    }
}

QSize QAxHostWidget::sizeHint() const
{
    if ( axhost )
	return axhost->sizeHint();
    return QWidget::sizeHint();
}

QSize QAxHostWidget::minimumSizeHint() const
{
    QSize size;
    if ( axhost )
	size = axhost->minimumSizeHint();
    if ( size.isValid() )
	return size;
    return QWidget::minimumSizeHint();
}

void QAxHostWidget::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    if ( !axhost )
	return;

    QPaintDeviceMetrics pdm( this );

    SIZEL hmSize;
    hmSize.cx = MAP_PIX_TO_LOGHIM( width(), pdm.logicalDpiX() );
    hmSize.cy = MAP_PIX_TO_LOGHIM( height(), pdm.logicalDpiY() );

    if ( axhost->m_spOleObject )
	axhost->m_spOleObject->SetExtent( DVASPECT_CONTENT, &hmSize );
    if ( axhost->m_spInPlaceObject ) {
	RECT rcPos = { x(), y(), x()+width(), y()+height() };
	axhost->m_spInPlaceObject->SetObjectRects( &rcPos, &rcPos );
    }
}

bool QAxHostWidget::event( QEvent *e )
{
    switch ( e->type() ) {
    case QEvent::Timer:
	if ( axhost && ((QTimerEvent*)e)->timerId() == setFocusTimer ) {
	    killTimer( setFocusTimer );
	    setFocusTimer = 0;
	    RECT rcPos = { x(), y(), x()+sizeHint().width(), y()+sizeHint().height() };
	    HRESULT res = axhost->m_spOleObject->DoVerb( OLEIVERB_UIACTIVATE, 0, (IOleClientSite*)axhost, 0, winId(), &rcPos );
	}
	break;
    }

    return QWidget::event( e );
}

void QAxHostWidget::focusInEvent( QFocusEvent *e )
{
    QWidget::focusInEvent( e );
    if ( !axhost || axhost->m_spInPlaceActiveObject || !axhost->m_spOleObject )
	return;

    // this is called by QWidget::setFocus which calls ::SetFocus on "this",
    // so we have to UIActivate the control after all that had happend.
    setFocusTimer = startTimer( 0 );
}

void QAxHostWidget::focusOutEvent( QFocusEvent *e )
{
    QWidget::focusOutEvent( e );
    if ( QFocusEvent::reason() == QFocusEvent::Popup )
	return;

    if ( !axhost || !axhost->m_spInPlaceActiveObject || !axhost->m_spInPlaceObject )
	return;

    axhost->m_spInPlaceObject->UIDeactivate();
}

bool QAxHostWidget::focusNextPrevChild( bool next )
{
    if ( axhost && axhost->m_spInPlaceActiveObject ) {
	MSG msg;
	msg.hwnd = winId();
	msg.message = WM_KEYDOWN;
	msg.wParam = VK_TAB;
	msg.lParam = 1;

	if ( axhost->m_spInPlaceActiveObject->TranslateAccelerator( &msg ) == S_OK )
	    return TRUE;
    }

    return QWidget::focusNextPrevChild( next );
}

void QAxHostWidget::windowActivationChange( bool oldActive )
{
    if ( axhost )
	axhost->windowActivationChange( oldActive );
}


/*!
    \class QAxWidget qaxwidget.h
    \brief The QAxWidget class is a QWidget that wraps an ActiveX control.

    \extension ActiveQt
    \module QAxContainer

    A QAxWidget can be instantiated as an empty object, with the name
    of the ActiveX control it should wrap, or with an existing
    interface pointer to the ActiveX control. The ActiveX control's
    properties, methods and events which only use \link QAxBase
    supported data types\endlink, become available as Qt properties,
    slots and signals. The base class QAxBase provides an API to
    access the ActiveX directly through the IUnknown pointer.

    QAxWidget is a QWidget and can be used as such, e.g. it can be
    organized in a widget hierarchy, receive events or act as an event
    filter. Standard widget properties, e.g. \link QWidget::enabled
    enabled \endlink are supported, but it depends on the ActiveX
    control to implement support for ambient properties like e.g.
    palette or font. QAxWidget tries to provide the necessary hints.

    \warning
    You can subclass QAxObject, but you cannot use the Q_OBJECT macro
    in the subclass (the generated moc-file will not compile), so you
    cannot add further signals, slots or properties. This limitation is 
    due to the metaobject information generated in runtime. 
    To work around this problem, aggregate the QAxObject as a member of 
    the QObject subclass.

    \important dynamicCall() querySubObject()
*/

/*!
    Creates an empty QAxWidget widget and propagates \a parent, \a
    name and \a f to the QWidget constructor. To initialize a control,
    call \link QAxBase::setControl() setControl \endlink.
*/
QAxWidget::QAxWidget( QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name, f ), container( 0 )
{
}

/*!
    Creates an QAxWidget widget and initializes the ActiveX control \a c.
    \a parent, \a name and \a f are propagated to the QWidget contructor.
*/
QAxWidget::QAxWidget( const QString &c, QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name ? name : c.latin1(), f ), container( 0 )
{
    setControl( c );
}

/*!
    Creates a QAxWidget that wraps the COM object referenced by \a iface.
    \a parent, \a name and \a f are propagated to the QWidget contructor.
*/
QAxWidget::QAxWidget( IUnknown *iface, QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name, f ), QAxBase( iface ), container( 0 )
{
}

/*!
    Shuts down the ActiveX control and destroys the QAxWidget widget,
    cleaning up all allocated resources.

    \sa clear()
*/
QAxWidget::~QAxWidget()
{
    clear();
}

/*!
    \reimp
*/
bool QAxWidget::initialize( IUnknown **ptr )
{
    if ( *ptr || control().isEmpty() )
	return FALSE;

    *ptr = 0;

    HRESULT hres = CoCreateInstance( QUuid(control()), 0, CLSCTX_SERVER, IID_IUnknown, (void**)ptr );
    if ( !SUCCEEDED(hres) )
	return FALSE;

    return createHostWindow( hres == S_FALSE );
}

/*!
    Creates the client site for the ActiveX control, and returns TRUE if
    the control could be embedded successfully, otherwise returns FALSE.
    If \a initialized is TRUE the control has already been initialized.

    This function is called by initialize(). If you reimplement initialize
    to customize the actual control instantiation, call this function in your
    reimplementation to have the control embedded by the default client side.
    Creates the client site for the ActiveX control, and returns TRUE if
    the control could be embedded successfully, otherwise returns FALSE.
*/
bool QAxWidget::createHostWindow( bool initialized )
{
    container = new QAxHostWindow( this, initialized );

    if ( !hhook ) {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    hhook = SetWindowsHookEx( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
	else
#endif
	    hhook = SetWindowsHookExA( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
    }
    ++hhookref;
    container->hostWidget()->resize( size() );
    container->hostWidget()->show();

    if ( container->hostWidget()->focusPolicy() != NoFocus ) {
	setFocusProxy( container->hostWidget() );
	setFocusPolicy( container->hostWidget()->focusPolicy() );
    }
    if ( parentWidget() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint ) );

    return TRUE;
}

/*!
    \reimp

    Shuts down the ActiveX control.
*/
void QAxWidget::clear()
{
    if ( isNull() )
	return;
    if ( !!control() ) {
	if ( hhook ) {
	    if ( !--hhookref ) {
		UnhookWindowsHookEx( hhook );
		hhook = 0;
	    }
	}
    }

    if ( container )
	container->deactivate();

    QAxBase::clear();
    setFocusPolicy( NoFocus );

    if ( container ) {
	container->releaseAll();
	container->Release();
    }
    container = 0;
}

/*!
    \fn QObject *QAxWidget::qObject()
    \reimp
*/

/*!
    \reimp
*/
const char *QAxWidget::className() const
{
    return "QAxWidget";
}

/*!
    \reimp
*/
QMetaObject *QAxWidget::metaObject() const
{
    return QAxBase::metaObject();
}

/*!
    \reimp
*/
QMetaObject *QAxWidget::parentMetaObject() const
{
    return QWidget::staticMetaObject();
}

/*!
    \reimp
*/
void *QAxWidget::qt_cast( const char *cname )
{
    if ( !qstrcmp( cname, "QAxWidget" ) ) return this;
    if ( !qstrcmp( cname, "QAxBase" ) ) return (QAxBase*)this;
    return QWidget::qt_cast( cname );
}


/*!
    \reimp
*/
bool QAxWidget::qt_invoke( int _id, QUObject *_o )
{
    if ( QAxBase::qt_invoke( _id, _o ) )
	return TRUE;
    return QWidget::qt_invoke( _id, _o );
}

/*!
    \reimp
*/
bool QAxWidget::qt_emit( int _id, QUObject* _o )
{
    const int index = _id - metaObject()->signalOffset();
    if ( !isNull() && index >= 0 ) {
	// get the list of connections
	QConnectionList *clist = receivers( _id );
	if ( clist ) // call the signal
	    activate_signal( clist, _o );

	return TRUE;
    }
    return QWidget::qt_emit( _id, _o );
}

/*!
    \reimp
*/
bool QAxWidget::qt_property( int _id, int _f, QVariant *_v )
{
    if ( QAxBase::qt_property( _id, _f, _v ) )
	return TRUE;
    return QWidget::qt_property( _id, _f, _v );
}

/*!
    \reimp
*/
void QAxWidget::enabledChange( bool old )
{
    QWidget::enabledChange( old );

    if ( old == isEnabled() || isNull() )
	return;

    container->emitAmbientPropertyChange( DISPID_AMBIENT_UIDEAD );
}

/*!
    \reimp
*/
QSize QAxWidget::sizeHint() const
{
    if ( container ) {
	QSize sh = container->sizeHint();
	if ( sh.isValid() )
	    return sh;
    }

    return QWidget::sizeHint();
}

/*!
    \reimp
*/
QSize QAxWidget::minimumSizeHint() const
{
    if ( container ) {
	QSize sh = container->minimumSizeHint();
	if ( sh.isValid() )
	    return sh;
    }

    return QWidget::minimumSizeHint();
}

/*!
    \reimp
*/
void QAxWidget::fontChange( const QFont &old )
{
    QWidget::fontChange( old );
    if ( isNull() )
	return;

    container->emitAmbientPropertyChange( DISPID_AMBIENT_FONT );
}

/*!
    \reimp
*/
void QAxWidget::paletteChange( const QPalette &old )
{
    QWidget::paletteChange( old );
    if ( isNull() )
	return;

    container->emitAmbientPropertyChange( DISPID_AMBIENT_BACKCOLOR );
    container->emitAmbientPropertyChange( DISPID_AMBIENT_FORECOLOR );
}

/*!
    \reimp
*/
void QAxWidget::windowActivationChange( bool old )
{
    QWidget::windowActivationChange( old );
    if ( isNull() )
	return;

    IOleInPlaceActiveObject *inplace = 0;
    queryInterface( IID_IOleInPlaceActiveObject, (void**)&inplace );
    if ( inplace ) {
	inplace->OnFrameWindowActivate( isActiveWindow() );
	inplace->Release();
    }
}

/*!
    \reimp
*/
void QAxWidget::resizeEvent( QResizeEvent *e )
{
    if ( container && container->hostWidget() )
	container->hostWidget()->resize( size() );

    QWidget::resizeEvent( e );
}

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
#include <qobjectlist.h>
#include <qstatusbar.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <olectl.h>

#include "../shared/types.h"

static HHOOK hhook = 0;
static int hhookref = 0;
#define HIMETRIC_PER_INCH   2540

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
	bool mouse = message >= WM_MOUSEFIRST && message <= WM_MOUSELAST;
	bool key = message == WM_CHAR ||
		   message == WM_KEYDOWN ||
		   message == WM_KEYUP ||
		   message == WM_SYSKEYDOWN ||
		   message == WM_SYSKEYUP ||
		   message == WM_IME_CHAR ||
		   message == WM_IME_KEYDOWN;
	if ( mouse || key ) {
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
		if ( key ) {
		    HRESULT hres = E_NOTIMPL;
		    IOleControl *control = 0;
		    ax->queryInterface( IID_IOleControl, (void**)&control );
		    if ( control ) {
			CONTROLINFO ci;
			ci.cb = sizeof(CONTROLINFO);
			control->GetControlInfo( &ci );
			if ( ci.cAccel ) {
			    int res = TranslateAccelerator( ax->winId(), ci.hAccel, msg );
			    if ( res )
				hres = control->OnMnemonic( msg );
			}
			control->Release();
		    }
		    if ( hres != S_OK )
			::SendMessage( ax->winId(), message, msg->wParam, msg->lParam );
		} else {
		    for ( i=0; (UINT)mouseTbl[i] != message || !mouseTbl[i]; i += 3 )
			;
		    if ( !mouseTbl[i] )
			return FALSE;
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


/*  \class QAxHostWindow qaxwidget.cpp
    \brief The QAxHostWindow class implements the client site interfaces.
    \internal
*/
class QAxHostWindow : public QWidget, 
		      public IDispatch,
		      public IOleClientSite,
		      public IOleControlSite,
		      public IOleInPlaceSite,
		      public IOleInPlaceFrame,
		      public IAdviseSink
{
public:
    QAxHostWindow( QAxWidget *c, IUnknown **ppUnk );
    ~QAxHostWindow();
    void releaseAll();

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

protected:
    void resizeEvent( QResizeEvent *e );

private:
    IOleObject *m_spOleObject;
    IOleControl *m_spOleControl;
    IOleInPlaceObject *m_spInPlaceObject;

    DWORD m_dwOleObject;

    QSize sizehint;

    unsigned long ref;
    QAxWidget *widget;
};

QAxHostWindow::QAxHostWindow( QAxWidget *c, IUnknown **ppUnk )
: QWidget( c, "QAxHostWindow", WResizeNoErase|WRepaintNoErase ), ref(1), widget( c )
{
    m_spOleObject = 0;
    m_spOleControl = 0;
    m_spInPlaceObject = 0;

    m_dwOleObject = 0;

    sizehint = QSize( 250, 250 );
    
    setBackgroundMode( NoBackground );

    HRESULT hr = S_OK;
    DWORD dwMiscStatus = 0;
    DWORD dwViewObjectType = 0;

    hr = CoCreateInstance( QUuid(widget->control()), 0, CLSCTX_SERVER, IID_IUnknown, (void**)ppUnk );
    bool bInited = hr == S_FALSE;
    
    if ( !SUCCEEDED(hr) || !*ppUnk )
	return;

    hr = S_OK;

    m_spOleObject = 0;
    widget->queryInterface( IID_IOleObject, (void**)&m_spOleObject );
    if (m_spOleObject) {
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
	hr = m_spOleObject->QueryInterface(IID_IViewObjectEx, (void**) &spViewObject);
	if ( FAILED(hr) ) {
	    hr = m_spOleObject->QueryInterface(IID_IViewObject2, (void**) &spViewObject);
	    dwViewObjectType = 3;
	} else {
	    dwViewObjectType = 7;
	}
	if ( FAILED(hr) ) {
	    hr = m_spOleObject->QueryInterface(IID_IViewObject, (void**) &spViewObject);
	    dwViewObjectType = 1;
	}
	
	m_spOleObject->Advise( this, &m_dwOleObject );
	IAdviseSink *spAdviseSink = 0;
	QueryInterface( IID_IAdviseSink, (void**)&spAdviseSink );	
	if ( spAdviseSink && spViewObject ) {
	    if ( dwViewObjectType )
		spViewObject->SetAdvise( DVASPECT_CONTENT, 0, spAdviseSink );
	    spAdviseSink->Release();
	}
	if ( spViewObject )
	    spViewObject->Release();

	m_spOleObject->SetHostNames( OLESTR("AXWIN"), 0 );
	QPaintDeviceMetrics pdm( widget );

#define MAP_PIX_TO_LOGHIM(x,ppli)   ( (HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli) )

	SIZEL hmSize;
	hmSize.cx = MAP_PIX_TO_LOGHIM( sizehint.width(), pdm.logicalDpiX() );
	hmSize.cy = MAP_PIX_TO_LOGHIM( sizehint.height(), pdm.logicalDpiY() );

	m_spOleObject->SetExtent( DVASPECT_CONTENT, &hmSize );
	m_spOleObject->GetExtent( DVASPECT_CONTENT, &hmSize );

#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH )
	sizehint.setWidth( MAP_LOGHIM_TO_PIX( hmSize.cx, pdm.logicalDpiX() ) );
	sizehint.setHeight( MAP_LOGHIM_TO_PIX( hmSize.cy, pdm.logicalDpiY() ) );

	RECT rcPos = { x(), y(), x()+sizehint.width(), y()+sizehint.height() };

	hr = m_spOleObject->DoVerb( OLEIVERB_INPLACEACTIVATE, 0, (IOleClientSite*)this, 0, winId(), &rcPos );

	m_spOleObject->QueryInterface( IID_IOleControl, (void**)&m_spOleControl );
	if ( m_spOleControl ) {
	    m_spOleControl->OnAmbientPropertyChange( DISPID_AMBIENT_BACKCOLOR );
	    m_spOleControl->OnAmbientPropertyChange( DISPID_AMBIENT_FORECOLOR );
	    m_spOleControl->OnAmbientPropertyChange( DISPID_AMBIENT_FONT );
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

HRESULT QAxHostWindow::QueryInterface( REFIID iid, void **iface )
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
    else
	return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

//**** IDispatch
HRESULT QAxHostWindow::Invoke(DISPID dispIdMember, 
			      REFIID riid, 
			      LCID lcid, 
			      WORD wFlags, 
			      DISPPARAMS *pDispParams, 
			      VARIANT *pVarResult, 
			      EXCEPINFO *pExcepInfo, 
			      UINT *puArgErr )
{
    if ( !pVarResult )
	return E_POINTER;

    switch( dispIdMember ) {
    case DISPID_AMBIENT_AUTOCLIP:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = TRUE;
	return S_OK;

    case DISPID_AMBIENT_USERMODE:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = TRUE;
	return S_OK;

    case DISPID_AMBIENT_SUPPORTSMNEMONICS:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = TRUE;
	return S_OK;

    case DISPID_AMBIENT_DISPLAYASDEFAULT:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = FALSE;
	return S_OK;

    case DISPID_AMBIENT_DISPLAYNAME:
	pVarResult->vt = VT_BSTR;
	pVarResult->bstrVal = QStringToBSTR( widget->caption() );
	return S_OK;

    case DISPID_AMBIENT_MESSAGEREFLECT:
	pVarResult->vt = VT_BOOL;
	pVarResult->boolVal = FALSE;
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
HRESULT QAxHostWindow::SaveObject()
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::GetMoniker( DWORD, DWORD, IMoniker **ppmk )
{
    if ( !ppmk )
	return E_POINTER;

    *ppmk = 0;
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::GetContainer( LPOLECONTAINER *ppContainer )
{
    return E_NOINTERFACE;
}

HRESULT QAxHostWindow::ShowObject()
{
    return S_OK;
}

HRESULT QAxHostWindow::OnShowWindow( BOOL fShow )
{
    return S_OK;
}

HRESULT QAxHostWindow::RequestNewObjectLayout()
{
    return E_NOTIMPL;
}

//**** IOleControlSite
HRESULT QAxHostWindow::OnControlInfoChanged()
{
    return S_OK;
}

HRESULT QAxHostWindow::LockInPlaceActive(BOOL /*fLock*/)
{
    return S_OK;
}

HRESULT QAxHostWindow::GetExtendedControl(IDispatch** ppDisp)
{
    if ( !ppDisp )
	return E_POINTER;
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::TransformCoords(POINTL* /*pPtlHimetric*/, POINTF* /*pPtfContainer*/, DWORD /*dwFlags*/)
{
    return S_OK;
}

HRESULT QAxHostWindow::TranslateAcceleratorA(LPMSG /*lpMsg*/, DWORD /*grfModifiers*/)
{
    return S_FALSE;
}

HRESULT QAxHostWindow::TranslateAcceleratorW(LPMSG /*lpMsg*/, DWORD /*grfModifiers*/)
{
    return S_FALSE;
}

HRESULT QAxHostWindow::OnFocus( BOOL bGotFocus )
{
    if ( bGotFocus ) {
	QFocusEvent e( QEvent::FocusIn );
	QApplication::sendEvent( widget, &e );
    } else {
	QFocusEvent e( QEvent::FocusOut );
	QApplication::sendEvent( widget, &e );
    }
    return S_OK;
}

HRESULT QAxHostWindow::ShowPropertyFrame()
{
    return E_NOTIMPL;
}

//**** IOleWindow
HRESULT QAxHostWindow::GetWindow( HWND *phwnd )
{
    if ( !phwnd )
	return E_POINTER;

    *phwnd = winId();
    return S_OK;
}

HRESULT QAxHostWindow::ContextSensitiveHelp( BOOL fEnterMode )
{
    if ( fEnterMode )
	QWhatsThis::enterWhatsThisMode();
    else
	QWhatsThis::leaveWhatsThisMode();

    return S_OK;
}

//**** IOleInPlaceSite
HRESULT QAxHostWindow::CanInPlaceActivate()
{
    return S_OK;
}

HRESULT QAxHostWindow::OnInPlaceActivate()
{
    OleLockRunning( m_spOleObject, TRUE, FALSE );
    m_spOleObject->QueryInterface( IID_IOleInPlaceObject, (void**) &m_spInPlaceObject );
    if ( m_spInPlaceObject ) {
	RECT rcPos = { x(), y(), x()+width(), y()+height() };
	m_spInPlaceObject->SetObjectRects( &rcPos, &rcPos );
    }
    return S_OK;
}

HRESULT QAxHostWindow::OnUIActivate()
{
    return S_OK;
}

HRESULT QAxHostWindow::GetWindowContext( IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo )
{
    if ( !ppFrame || !ppDoc || !lprcPosRect || !lprcClipRect || !lpFrameInfo )
	return E_POINTER;

    QueryInterface(IID_IOleInPlaceFrame, (void**)ppFrame);
    QueryInterface(IID_IOleInPlaceUIWindow, (void**)ppDoc);

    ::GetClientRect( winId(), lprcPosRect );
    ::GetClientRect( winId(), lprcClipRect );

    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp = FALSE;
    ACCEL ac = { 0,0,0 };
    lpFrameInfo->haccel = CreateAcceleratorTable(&ac, 1);;
    lpFrameInfo->cAccelEntries = 1;
    lpFrameInfo->hwndFrame = widget->winId();

    return S_OK;
}

HRESULT QAxHostWindow::Scroll( SIZE scrollExtant )
{
    return S_FALSE;
}

HRESULT QAxHostWindow::OnUIDeactivate( BOOL )
{
    return S_OK;
}

HRESULT QAxHostWindow::OnInPlaceDeactivate()
{
    if ( m_spInPlaceObject )
	m_spInPlaceObject->Release();
    m_spInPlaceObject = 0;
    return S_OK;
}

HRESULT QAxHostWindow::DiscardUndoState()
{
    return S_OK;
}

HRESULT QAxHostWindow::DeactivateAndUndo()
{
    IOleInPlaceObject *inPlace = 0;
    widget->queryInterface( IID_IOleInPlaceObject, (void**)&inPlace );
    if ( inPlace ) {
	inPlace->UIDeactivate();
	inPlace->Release();
    }

    return S_OK;
}

HRESULT QAxHostWindow::OnPosRectChange( LPCRECT lprcPosRect )
{
    return S_OK;
}

//**** IOleInPlaceFrame
HRESULT QAxHostWindow::InsertMenus( HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::SetMenu( HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::RemoveMenus( HMENU hmenuShared )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::SetStatusText( LPCOLESTR pszStatusText )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::EnableModeless( BOOL fEnable )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::TranslateAcceleratorA( LPMSG lpMsg, WORD grfModifiers )
{
    return TranslateAcceleratorA( lpMsg, (DWORD)grfModifiers );
}

HRESULT QAxHostWindow::TranslateAcceleratorW( LPMSG lpMsg, WORD grfModifiers )
{
    return TranslateAcceleratorW( lpMsg, (DWORD)grfModifiers );
}

//**** IOleInPlaceUIWindow
HRESULT QAxHostWindow::GetBorder( LPRECT lprectBorder )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::RequestBorderSpace( LPCBORDERWIDTHS pborderwidths )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::SetBorderSpace( LPCBORDERWIDTHS pborderwidths )
{
    return E_NOTIMPL;
}

HRESULT QAxHostWindow::SetActiveObject( IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName )
{
    return E_NOTIMPL;
}

//**** QWidget

QSize QAxHostWindow::sizeHint() const
{
    return sizehint;
}

QSize QAxHostWindow::minimumSizeHint() const
{
    if ( !m_spOleObject )
	return QWidget::minimumSizeHint();

    SIZE sz = { 0, 0 };
    m_spOleObject->SetExtent( DVASPECT_CONTENT, &sz );
    HRESULT res = m_spOleObject->GetExtent( DVASPECT_CONTENT, &sz );
    if ( SUCCEEDED(res) ) {
	QPaintDeviceMetrics pmetric( this );
	return QSize( MAP_LOGHIM_TO_PIX( sz.cx, pmetric.logicalDpiX() ),
		      MAP_LOGHIM_TO_PIX( sz.cy, pmetric.logicalDpiY() ) );
    }

    return QWidget::minimumSizeHint();
}

void QAxHostWindow::resizeEvent( QResizeEvent *e )
{
    QPaintDeviceMetrics pdm( this );

    SIZEL hmSize;
    hmSize.cx = MAP_PIX_TO_LOGHIM( width(), pdm.logicalDpiX() );
    hmSize.cy = MAP_PIX_TO_LOGHIM( height(), pdm.logicalDpiY() );

    if ( m_spOleObject )
	m_spOleObject->SetExtent( DVASPECT_CONTENT, &hmSize );
    if ( m_spInPlaceObject ) {
	RECT rcPos = { x(), y(), x()+width(), y()+height() };
	m_spInPlaceObject->SetObjectRects( &rcPos, &rcPos );
    }
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

    container = new QAxHostWindow( this, ptr );

    if ( !*ptr ) {
	container->Release();
	container = 0;
	return FALSE;
    }

    if ( !hhook ) {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    hhook = SetWindowsHookEx( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
	else
#endif
	    hhook = SetWindowsHookExA( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
    }
    ++hhookref;
    container->resize( size() );
    container->show();

    setFocusPolicy( StrongFocus );
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

    QAxBase::clear();
    setFocusPolicy( NoFocus );

    if ( container ) {
	container->releaseAll();
	removeChild( container );
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
    if ( container )
	container->resize( size() );

    QWidget::resizeEvent( e );
}

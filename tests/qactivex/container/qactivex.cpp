//#define COCREATE

#include "qactivex.h"

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>

#include <qapplication.h>
#include <qmetaobject.h>
#include <qobjectlist.h>
#include <qstatusbar.h>

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
	    QWidget *widget = QWidget::find( hwnd );
	    while ( !widget && hwnd ) {
		hwnd = ::GetParent( hwnd );
		widget = QWidget::find( hwnd );
	    }
	    QActiveX *ax = widget ? (QActiveX*)widget->qt_cast( "QActiveX" ) : 0;
	    if ( ax && msg->hwnd != ax->winId() ) {
		if ( key ) {
		    bool res = ::SendMessage( ax->winId(), message, msg->wParam, msg->lParam );
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
		    if ( message == WM_LBUTTONDOWN ) {
			//ax->setFocus();
		    }
		    // this would eat the event, but it doesn't work with designer...
		    //if ( e.isAccepted() ) 
			//msg->message = WM_NULL;
		}
	    }
	}
	reentrant = FALSE;
    }
    return CallNextHookEx( hhook, nCode, wParam, lParam );
}

extern BSTR QStringToBSTR( const QString &str );
extern QString BSTRToQString( BSTR bstr );

// {D02B1C36-3607-4959-ADA0-C93E945A67CB}
static const GUID IID_IQtClientSite = 
{ 0xd02b1c36, 0x3607, 0x4959, { 0xad, 0xa0, 0xc9, 0x3e, 0x94, 0x5a, 0x67, 0xcb } };


struct IQtClientSite : public IUnknown
{
    virtual void connect() = 0;
    virtual void disconnect() = 0;

    virtual int background() = 0;
    virtual int foreground() = 0;
    virtual IFont *font() = 0;
};

/*
    \internal
    \class QClientSite qactivex.cpp

    \brief The QClientSite class implements the interfaces for the OLE client 
	   to communicate with the OLE object.
*/
class QClientSite : public IQtClientSite,
		    public IDispatch,
		    public IOleClientSite,
		    public IOleInPlaceSiteEx

/*		    public IAdviseSink2,
		    public IAdviseSinkEx,
		    public IOleControlSite,
		    public IOleContainer,
		    public IOleInPlaceFrame,
		    public IOleDocumentSite,
		    public ISimpleFrameSite,*/
		    
{
public:
    QClientSite( QActiveX *ax ) : ref( 1 ), oleconnection( 0 ), activex( ax ), control( 0 ), activeObject( 0 )
    {
	Q_ASSERT(activex);
    }
    ~QClientSite()
    {
	disconnect();
    }

    // IQtClientSite
    void connect()
    {
	activex->queryInterface( IID_IUnknown, (void**)&control );
	if ( control ) {
	    CComPtr<IOleObject> ole;
	    control->QueryInterface( IID_IOleObject, (void**)&ole );
	    if ( ole ) {
		ole->Advise( (IAdviseSink*)(IAdviseSink2*)this, &oleconnection );
		// ole->SetClientSite( 0 );
		ole->SetClientSite( (IOleClientSite*)this );
		QString appname;
		if ( qApp->mainWidget() )
		    appname = qApp->mainWidget()->caption();
		else 
		    appname = activex->topLevelWidget()->caption();
		if ( appname.isEmpty() )
		    appname = qApp->name();
		ole->SetHostNames( QStringToBSTR( appname ), 0 );
	    }

	    CComPtr<IOleDocumentView> document;
	    control->QueryInterface( IID_IOleDocumentView, (void**)&document );
	    if ( document ) {
		document->SetInPlaceSite( 0 );
		document->SetInPlaceSite( (IOleInPlaceSite*)(IOleInPlaceSiteEx*)this );
		document->UIActivate( TRUE );
	    }

	    CComPtr<IViewObject> view;
	    control->QueryInterface( IID_IViewObject, (void**)&view );
	    if ( view ) {
		view->SetAdvise( DVASPECT_CONTENT, ADVF_PRIMEFIRST, 0 );
		view->SetAdvise( DVASPECT_CONTENT, ADVF_PRIMEFIRST, (IAdviseSink*)(IAdviseSink2*)this );
	    }

	    CComPtr<IViewObjectEx> viewex;
	    control->QueryInterface( IID_IViewObjectEx, (void**)&viewex );
	    if ( viewex ) {
		SIZE hsz;
		DVEXTENTINFO info;
		info.cb = sizeof(DVEXTENTINFO);
		info.dwExtentMode = DVEXTENT_CONTENT;
		if ( viewex->GetNaturalExtent( DVASPECT_CONTENT, 0, 0, 0, &info, &hsz ) == S_OK ) {
		    SIZE psz;
		    AtlHiMetricToPixel( &info.sizelProposed, &psz );
		    activex->extent = QSize( psz.cx, psz.cy );
		    if ( activex->extent.isValid() )
			QApplication::postEvent( activex->parentWidget(), new QEvent( QEvent::LayoutHint ) );
		}
	    }
	    
	    if ( ole ) {
		RECT rc = { 0, 0, activex->width(), activex->height() };
		ole->DoVerb( OLEIVERB_UIACTIVATE, 0, (IOleClientSite*)this, 0/*reserved*/, activex->winId(), &rc );
	    }
	}
    }
    void disconnect()
    {
	if ( control ) {
	    if ( oleconnection ) {
		CComPtr<IOleObject> ole;
		control->QueryInterface( IID_IOleObject, (void**)&ole );
		if ( ole ) {
		    ole->Unadvise( oleconnection );
		    ole->SetClientSite( 0 );
		}
	    }

	    CComPtr<IViewObject> view;
	    control->QueryInterface( IID_IViewObject, (void**)&view );
	    if ( view ) {
		view->SetAdvise( DVASPECT_CONTENT, ADVF_ONLYONCE, 0 );
	    }

	    CComPtr<IOleDocumentView> document;
	    control->QueryInterface( IID_IOleDocumentView, (void**)&document );
	    if ( document ) {
		document->SetInPlaceSite( 0 );
	    }

	    control->Release();
	    control = 0;
	}
	if ( activeObject ) {
	    activeObject->Release();
	    activeObject = 0;
	}
    }

    int foreground()
    {
	QColor col = activex->palette().color( QPalette::Active, QColorGroup::Foreground );
	return qRgb( col.blue(), col.green(), col.red() );
    }
    int background()
    {
	QColor col = activex->palette().color( QPalette::Active, QColorGroup::Background );
	return qRgb( col.blue(), col.green(), col.red() );
    }
    IFont *font()
    {
	FONTDESC fdesc;
	memset( &fdesc, 0, sizeof(fdesc) );
	fdesc.cbSizeofstruct = sizeof(FONTDESC);
	QFont font = activex->font();
	fdesc.cySize.Lo = font.pointSize() * 10000;
	fdesc.fItalic = font.italic();
	fdesc.fStrikethrough = font.strikeOut();
	fdesc.fUnderline = font.underline();
	fdesc.lpstrName = QStringToBSTR( font.family() );
	fdesc.sWeight = font.weight();
	
	IFont *f;
	OleCreateFontIndirect( &fdesc, IID_IDispatch, (void**)&f );
	return f;
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
	else if ( riid == IID_IQtClientSite )
	    *ppvObject = (IQtClientSite*)this;
	else if ( riid == IID_IDispatch )
	    *ppvObject = (IDispatch*)this;
	else if ( riid == IID_IOleClientSite )
	    *ppvObject = (IOleClientSite*)this;
	else if ( riid == IID_IOleInPlaceSite )
	    *ppvObject = (IOleInPlaceSite*)(IOleInPlaceSiteEx*)this;
	else if ( riid == IID_IOleInPlaceSiteEx )
	    *ppvObject = (IOleInPlaceSiteEx*)this;
/*	else if ( riid == IID_IAdviseSink )
	    *ppvObject = (IAdviseSink*)(IAdviseSink2*)this;
	else if ( riid == IID_IAdviseSink2 )
	    *ppvObject = (IAdviseSink2*)this;
	else if ( riid == IID_IAdviseSinkEx )
	    *ppvObject = (IAdviseSinkEx*)this;
	else if ( riid == IID_IOleControlSite )
	    *ppvObject = (IOleControlSite*)this;
	else if ( riid == IID_IOleWindow )
	    *ppvObject = (IOleWindow*)(IOleInPlaceUIWindow*)this;
	else if ( riid == IID_IOleInPlaceUIWindow )
	    *ppvObject = (IOleInPlaceUIWindow*)this;
	else if ( riid == IID_IParseDisplayName )
	    *ppvObject = (IParseDisplayName*)(IOleContainer*)this;
	else if ( riid == IID_IOleContainer )
	    *ppvObject = (IOleContainer*)this;
	else if ( riid == IID_IOleInPlaceFrame )
	    *ppvObject = (IOleInPlaceFrame*)this;
	else if ( riid == IID_IOleDocumentSite )
	    *ppvObject = (IOleDocumentSite*)this;
	else if ( riid == IID_ISimpleFrameSite )
	    *ppvObject = (ISimpleFrameSite*)this;*/
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
	if ( riid != IID_NULL )
	    return DISP_E_UNKNOWNINTERFACE;
	if ( !pVarResult )
	    return DISP_E_PARAMNOTOPTIONAL;
	if ( wFlags != DISPATCH_PROPERTYGET )
	    return DISP_E_MEMBERNOTFOUND;

	pVarResult->vt = VT_ERROR;
	switch ( dispIdMember ) {
	case DISPID_AMBIENT_BACKCOLOR: // OLE_COLOR is BGR
	    pVarResult->vt = VT_I4;
	    pVarResult->lVal = background();
	    break;
	case DISPID_AMBIENT_FORECOLOR:
	    pVarResult->vt = VT_I4;
	    pVarResult->lVal = foreground();
	    break;
	case DISPID_AMBIENT_DISPLAYNAME: // VT_BSTR
	    pVarResult->vt = VT_BSTR;
	    pVarResult->bstrVal = SysAllocStringByteLen( 0, 0 );
	    break;
	case DISPID_AMBIENT_FONT: // OLE_FONT; IFont
	    pVarResult->vt = VT_DISPATCH;
	    pVarResult->pdispVal = (IDispatch*)font();
	    break;
	case DISPID_AMBIENT_LOCALEID: // VT_I4
	    pVarResult->vt = VT_I4;
	    pVarResult->lVal = LOCALE_SYSTEM_DEFAULT;
	    break;
	case DISPID_AMBIENT_MESSAGEREFLECT: // VT_BOOL
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;
	case DISPID_AMBIENT_SCALEUNITS: // VT_BSTR
	    break;
	case DISPID_AMBIENT_TEXTALIGN: // VT_I2; 0 - General; 1 - left; 2 - center; 3 - right; 4 - full
	    pVarResult->vt = VT_I2;
	    pVarResult->iVal = 0;
	    break;
	case DISPID_AMBIENT_USERMODE: // VT_BOOL; FALSE - Design; TRUE - Interaction
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = TRUE;
	    break;
	case DISPID_AMBIENT_UIDEAD: // VT_BOOL
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;
	case DISPID_AMBIENT_SHOWGRABHANDLES: // VT_BOOL
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;
	case DISPID_AMBIENT_SHOWHATCHING: // VT_BOOL
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;
	case DISPID_AMBIENT_DISPLAYASDEFAULT: // VT_BOOL
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;
	case DISPID_AMBIENT_SUPPORTSMNEMONICS: // VT_BOOL
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;
	case DISPID_AMBIENT_AUTOCLIP: // VT_BOOL
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;
	case DISPID_AMBIENT_APPEARANCE: // VT_BOOL: 0-flat; 1-3D
	    pVarResult->vt = VT_BOOL;
	    pVarResult->boolVal = FALSE;
	    break;

	case DISPID_AMBIENT_CODEPAGE:
	case DISPID_AMBIENT_PALETTE:
	case DISPID_AMBIENT_CHARSET:
	case DISPID_AMBIENT_TRANSFERPRIORITY:
	case DISPID_AMBIENT_RIGHTTOLEFT:
	case DISPID_AMBIENT_TOPTOBOTTOM:
	default:
	    break;
	}

	return pVarResult->vt != VT_ERROR ? S_OK : DISP_E_MEMBERNOTFOUND;
    }

    // IOleClientSite
    HRESULT __stdcall SaveObject()
    {
	qDebug( "IOleClientSite::SaveObject" );
	return S_OK;
    }
    HRESULT __stdcall GetMoniker( DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk )
    {
	return E_NOTIMPL;
    }
    HRESULT __stdcall GetContainer( IOleContainer **ppContainer )
    {
	qDebug( "IOleClientSite::GetContainer" );
	return QueryInterface( IID_IOleContainer, (void**)ppContainer );
    }
    HRESULT __stdcall ShowObject()
    {
	qDebug( "IOleClientSite::ShowObject" );
	if ( !activex->extent.isValid() ) {
	    CComPtr<IOleObject> ole;
	    control->QueryInterface( IID_IOleObject, (void**)&ole );
	    if ( ole ) {
		SIZE sz, psz;
		if ( ole->GetExtent( DVASPECT_CONTENT, &sz ) == S_OK ) {
		    AtlHiMetricToPixel( &sz, &psz );
		    activex->extent = QSize( psz.cx, psz.cy );
		    if ( activex->extent.isValid() )
			QApplication::postEvent( activex->parentWidget(), new QEvent( QEvent::LayoutHint ) );
		}
	    }
	}
	activex->show();
	return S_OK;
    }
    HRESULT __stdcall OnShowWindow( BOOL fShow )
    {
	qDebug( "IOleClientSite::OnShowWindow" );
	return S_OK;
    }
    HRESULT __stdcall RequestNewObjectLayout()
    {
	qDebug( "IOleClientSite::RequestNewObjectLayout" );
	CComPtr<IOleObject> ole;
	control->QueryInterface( IID_IOleObject, (void**)&ole );
	if ( ole ) {
	    SIZE sz;
	    ole->GetExtent( DVASPECT_CONTENT, &sz );
	    ole->SetExtent( DVASPECT_CONTENT, &sz );
	    SIZE psz;
	    AtlHiMetricToPixel( &sz, &psz );
	    activex->extent = QSize( psz.cx, psz.cy );
	    if ( activex->extent.isValid() )
		QApplication::postEvent( activex->parentWidget(), new QEvent( QEvent::LayoutHint ) );
	}
	return E_NOTIMPL;//###
    }

    // IAdviseSink
    void __stdcall OnDataChange( FORMATETC *pFormatetc, STGMEDIUM *pStgmed )
    {
	qDebug( "IAdviseSink::OnDataChange" );
    }
    void __stdcall OnViewChange( DWORD dwAspect, LONG lindex )
    {
	qDebug( "IAdviseSink::OnViewChange" );
    }
    void __stdcall OnRename( IMoniker *pmk )
    {
	qDebug( "IAdviseSink::OnRename" );
    }
    void __stdcall OnSave()
    {
	qDebug( "IAdviseSink::OnSave" );
    }
    void __stdcall OnClose()
    {
	qDebug( "IAdviseSink::OnClose" );
    }

    // IAdviseSink2
    void __stdcall OnLinkSrcChange( IMoniker *pmk )
    {
	qDebug( "IAdviseSink2::OnLinkSrcChange" );
    }

    // IAdviseSinkEx
    void __stdcall OnViewStatusChange( DWORD dwViewStatus )
    {
	qDebug( "IAdviseSinkEx::OnViewStatusChange" );
    }

    // IOleControlSite
    HRESULT __stdcall OnControlInfoChanged()
    {
	qDebug( "IOleControlSite::OnControlinfoChanged" );
	CComPtr<IOleControl> ole;
	control->QueryInterface( IID_IOleControl, (void**)&ole );
	if ( ole ) {
	    CONTROLINFO info;
	    info.cb = sizeof(CONTROLINFO);
	    ole->GetControlInfo( &info );
	}
	return S_OK;
    }
    HRESULT __stdcall LockInPlaceActive( BOOL fLock )
    {
	qDebug( "IOleControlSite::LockInPlaceActive" );
	return E_NOTIMPL;
    }
    HRESULT __stdcall GetExtendedControl( IDispatch **ppDisp )
    {
	if ( !ppDisp )
	    return E_POINTER;
	*ppDisp = 0;

	qDebug( "IOleControlSite::GetExtendedControl" );
	return E_NOTIMPL;
    }
    HRESULT __stdcall TransformCoords( POINTL *pPtlHimetric, POINTF *pPtfContainer, DWORD dwFlags )
    {
	if ( !pPtlHimetric || !pPtfContainer )
	    return E_POINTER;

	qDebug( "IOleControlSite::TransformCoords" );
	return E_NOTIMPL;
    }
    HRESULT __stdcall TranslateAccelerator( MSG *pMsg, DWORD grfModifiers )
    {
	qDebug( "IOleControlSite::TranslateAccelerator" );
	return E_NOTIMPL;
    }
    HRESULT __stdcall OnFocus( BOOL fGotFocus )
    {
	if ( fGotFocus )
	    qDebug( "IOleControlSite::OnFocus(in)" );
	else
	    qDebug( "IOleControlSite::OnFocus(out)" );
	return S_OK;
    }
    HRESULT __stdcall ShowPropertyFrame()
    {
	qDebug( "IOleControlSite::ShowPropertyFrame" );
	return E_NOTIMPL;
    }

    // IOleWindow
    HRESULT __stdcall GetWindow( HWND *phwnd )
    {
	Q_ASSERT( activex );
	*phwnd = activex->winId();
	return S_OK;
    }
    HRESULT __stdcall ContextSensitiveHelp( BOOL fEnterMode )
    {
	qDebug( "IOleWindow::ContextSensitiveHelp" );
/*	if ( fEnterMode )
	    QWhatsThis::enterWhatsThisMode();
	else
	    QWhatsThis::leaveWhatsThisMode();*/

	return S_OK;
    }
    
    // IOleInPlaceUIWindow
    HRESULT __stdcall GetBorder( RECT *lprectBorder )
    {
	qDebug( "IOleInPlaceUIWindow::GetBorder" );
	RECT border = { 0, 0, 200, 50 };
	*lprectBorder = border;
	return S_OK; //INPLACE_E_NOTOOLSPACE;
    }
    HRESULT __stdcall RequestBorderSpace( LPCBORDERWIDTHS pborderwidths )
    {
	qDebug( "IOleInPlaceUIWindow::RequestBorderSpace" );
	return S_OK; //INPLACE_E_NOTOOLSPACE;
    }
    HRESULT __stdcall SetBorderSpace( LPCBORDERWIDTHS pborderwidths )
    {
	qDebug( "IOleInPlaceUIWindow::SetBorderSpace" );
	return S_OK;
    }
    HRESULT __stdcall SetActiveObject( IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName )
    {
	qDebug( "IOleInPlaceUIWindow::SetActiveObject" );
	if ( !pActiveObject )
	    return E_INVALIDARG;

	activeObject = pActiveObject;
	activeObject->AddRef();
/*
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = activex->width();
	rc.bottom = activex->height();
	activeObject->ResizeBorder( &rc, (IOleInPlaceUIWindow*)this, TRUE );
*/
	return S_OK;
    }

    // IParseDisplayName
    HRESULT __stdcall ParseDisplayName( IBindCtx *pbc, LPOLESTR pszDisplayName, ULONG *pchEaten, IMoniker **ppmkOut )
    {
	*ppmkOut = 0;

	qDebug( "IParseDisplayName::ParseDisplayName" );
	if ( *ppmkOut )
	    (*ppmkOut)->AddRef();
	return S_OK;
    }

    // IOleContainer
    HRESULT __stdcall EnumObjects( DWORD grfFlags, IEnumUnknown **ppenum )
    {
	qDebug( "IOleContainer::EnumObjects" );
	*ppenum = 0;
	return E_NOTIMPL;
    }
    HRESULT __stdcall LockContainer( BOOL fLock )
    {
	qDebug( "IOleContainer::LockContainer" );
	return S_OK;
    }

    // IOleInPlaceSite
    HRESULT __stdcall CanInPlaceActivate()
    {
	qDebug( "IOleInPlaceSite::CanInPlaceActivate" );
	return S_OK;
    }
    HRESULT __stdcall OnInPlaceActivate()
    {
	qDebug( "IOleInPlaceSite::OnInPlaceActivate" );
	return S_OK;
    }
    HRESULT __stdcall OnUIActivate()
    {
	qDebug( "IOleInPlaceSite::OnUIActivate" );
	return S_OK;
    }
    HRESULT __stdcall GetWindowContext( IOleInPlaceFrame **ppFrame, 
					IOleInPlaceUIWindow **ppDoc, 
					LPRECT lprcPosRect, 
					LPRECT lprcClipRect, 
					LPOLEINPLACEFRAMEINFO lpFrameInfo )
    {
	/*
	if ( !lprcPosRect || !lprcClipRect || !lpFrameInfo ) {
	    *ppFrame = 0;
	    *ppDoc = 0;
	    return E_INVALIDARG;
	}

	qDebug( "IOleInPlaceSite::GetWindowContext" );
	QueryInterface( IID_IOleInPlaceFrame, (void**)ppFrame );
	QueryInterface( IID_IOleInPlaceUIWindow, (void**)ppDoc );
	RECT posRect = { 0, 0, activex->width(), activex->height() };
	RECT clipRect = posRect;
	*lprcPosRect = posRect;
	*lprcClipRect = clipRect;

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->cAccelEntries = 0;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->hwndFrame = activex->winId();
	*/

	return S_OK;
    }
    HRESULT __stdcall Scroll( SIZE scrollExtant )
    {
	qDebug( "IOleInPlaceSite::Scroll" );
	//###
	CComPtr<IOleInPlaceObject> inplace;
	control->QueryInterface( IID_IOleInPlaceObject, (void**)&inplace );
	if ( inplace ) {
	    // inplace->SetObjectRects( &posRect, &clipRect );
	}
	return S_OK;
    }
    HRESULT __stdcall OnUIDeactivate( BOOL fUndoable )
    {
	qDebug( "IOleInPlaceSite::OnUIDeactivate" );
	return S_OK;
    }
    HRESULT __stdcall OnInPlaceDeactivate()
    {
	qDebug( "IOleInPlaceSite::OnInPlaceDeactivate" );
	return S_OK;
    }
    HRESULT __stdcall DiscardUndoState()
    {
	qDebug( "IOleInPlaceSite::DiscardUndoState" );
	return S_OK;
    }
    HRESULT __stdcall DeactivateAndUndo()
    {
	qDebug( "IOleInPlaceSite::DeactivateAndUndo" );
	return S_OK;
    }
    HRESULT __stdcall OnPosRectChange( LPCRECT lprcPosRect )
    {
	qDebug( "IOleInPlaceSite::OnPosRectChange" );
	CComPtr<IOleInPlaceObject> inplace;
	control->QueryInterface( IID_IOleInPlaceObject, (void**)&inplace );
	if ( inplace ) {
	    RECT posRect = { 0, 0, activex->width(), activex->height() };
	    RECT clipRect = { 0, 0, activex->width(), activex->height() };
	    /*activex->extent = QSize( lprcPosRect->right - lprcPosRect->left,
				     lprcPosRect->bottom - lprcPosRect->top );*/

	    inplace->SetObjectRects( &posRect, &clipRect );
	}
	return S_OK;
    }

    // IOleInPlaceSiteEx
    HRESULT __stdcall OnInPlaceActivateEx( BOOL *pfNoRedraw, DWORD dwFlags )
    {
	qDebug( "IOleInPlaceSiteEx::OnInPlaceActivateEx" );
	return S_OK;
    }
    HRESULT __stdcall OnInPlaceDeactivateEx( BOOL fNoRedraw )
    {
	qDebug( "IOleInPlaceSiteEx::OnInPlaceDeactivateEx" );
	return S_OK;
    }
    HRESULT __stdcall RequestUIActivate()
    {
	qDebug( "IOleInPlaceSiteEx::RequestUIActivate" );
	return S_OK;
    }

    // IOleInPlaceFrame
    HRESULT __stdcall InsertMenus( HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths )
    {
	qDebug( "IOleInPlaceFrame::InsertMenus" );
	return S_OK;
    }
    HRESULT __stdcall SetMenu( HMENU hmenuShared, HOLEMENU olemenu, HWND hwndActiveObject )
    {
	qDebug( "IOleInPlaceFrame::SetMenu" );
	return S_OK;
    }
    HRESULT __stdcall RemoveMenus( HMENU hmenuShared )
    {
	qDebug( "IOleInPlaceFrame::RemoveMenus" );
	return S_OK;
    }
    HRESULT __stdcall SetStatusText( LPCOLESTR pszStatusText )
    {
	qDebug( "IOleInPlaceFrame::SetStatusText" );
	QWidget *tlw = activex->topLevelWidget();
	QObjectList *list = tlw->queryList( "QStatusBar" );
	QObjectListIt it( *list );
	while ( it.current() ) {
	    QStatusBar *bar = (QStatusBar*)it.current();
	    ++it;
	    bar->message( BSTRToQString( (BSTR)pszStatusText ) );
	}
	delete list;

	return S_OK;
    }
    HRESULT __stdcall EnableModeless( BOOL fEnable )
    {
	qDebug( "IOleInPlaceFrame::EnableModeless" );
	return S_OK;
    }
    HRESULT __stdcall TranslateAccelerator( LPMSG lpmsg, WORD wId )
    {
	qDebug( "IOleInPlaceFrame::TranslateAccelerator" );
	return S_FALSE;
    }

    // IOleDocumentSite
    HRESULT __stdcall ActivateMe( IOleDocumentView *pViewToActivate )
    {
	qDebug( "IOleDocumentSite::ActivateMe" );
	pViewToActivate->SetInPlaceSite( (IOleInPlaceSite*)(IOleInPlaceSiteEx*)this );
	return S_OK;
    }

    // ISimpleFrameSite
    HRESULT __stdcall PreMessageFilter( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT *plResult, DWORD *pdwCookie )
    {
	return S_OK;
    }
    HRESULT __stdcall PostMessageFilter( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT *plResult, DWORD pdwCookie )
    {
	return E_NOTIMPL;
    }

private:
    ulong ref;
    DWORD oleconnection;
    QActiveX *activex;

    IUnknown *control;
    IOleInPlaceActiveObject *activeObject;
};


/*!
    \class QActiveX qactivex.h
    \brief The QActiveX class is a QWidget that wraps an ActiveX control.

    \extension QActiveX

    A QActiveX can be instantiated as an empty object or with the name of the ActiveX control
    it should wrap.The properties, methods and events of the ActiveX control become available as Qt properties, 
    slots and signals. The baseclass QComBase provides also an API to access the ActiveX directly 
    through the IUnknown pointer.

    QActiveX is a QWidget and can be used as such, e.g. it can be organized in a widget hierarchy, receive events 
    or act as an event filter. Standard widget properties, e.g. \link QWidget::enabled \endlink enabled are supported,
    but it depends on the ActiveX control to implement support for ambient properties like e.g. palette or font. 
    QActiveX tries to provide the necessary hints.
*/

/*!
    Creates an empty QActiveX widget and propagates \a parent and \a name to the QWidget constructor. 
    To initialize a control, call \link QComBase::setControl() setControl \endlink.
*/
QActiveX::QActiveX( QWidget *parent, const char *name )
: QWidget( parent, name ), clientsite( 0 )
{
}

/*!
    Creates an QActiveX widget and initializes the ActiveX control \a c.
    \a parent and \a name are propagated to the QWidget contructor.
*/
QActiveX::QActiveX( const QString &c, QWidget *parent, const char *name )
: QWidget( parent, name ), clientsite( 0 )
{
    setControl( c );
}

/*!
    Shuts down the ActiveX control and destroys the QActiveX widget, 
    cleaning up all allocated resources.

    \sa clear()
*/
QActiveX::~QActiveX()
{
    clear();
    if ( clientsite )
	clientsite->Release();
}

/*!
    Initializes the ActiveX control.
*/
bool QActiveX::initialize( IUnknown **ptr )
{
    if ( *ptr || control().isEmpty() )
	return FALSE;
    CoInitialize( 0 );
    _Module.Init( 0, GetModuleHandle( 0 ) );


    if ( !clientsite )
	clientsite = new QClientSite( this );
    CComPtr<IQtClientSite> qtclient;
    clientsite->QueryInterface( IID_IQtClientSite, (void**)&qtclient );

    *ptr = 0;
#ifdef COCREATE
    CoCreateInstance( QUuid(control()), NULL, CLSCTX_INPROC_SERVER|
		CLSCTX_INPROC_HANDLER|CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)ptr );
#else
    CAxWindow axWindow = winId();
    axWindow.CreateControlEx( (unsigned short*)qt_winTchar( control(), TRUE ), 0, 0, ptr );
#endif
    if ( !*ptr ) {
	_Module.Term();
	CoUninitialize();
	return FALSE;
    }
    CComPtr<IOleObject> ole;
    (*ptr)->QueryInterface( IID_IOleObject, (void**)&ole );
    if ( !ole )
	return TRUE;

    BSTR userType;
    ole->GetUserType( USERCLASSTYPE_SHORT, &userType );
    setCaption( BSTRToQString( userType ) );
    CoTaskMemFree( userType );

    metaObject();

    bool quickActivated = FALSE;
#ifdef COCREATE
    // try to quickactivate control
    CComPtr<IQuickActivate> quick;
    (*ptr)->QueryInterface( IID_IQuickActivate, (void**)&quick );
    if ( quick ) {
	QACONTAINER qaContainer;
	memset( &qaContainer, 0, sizeof(QACONTAINER) );
	qaContainer.cbSize = sizeof(QACONTAINER);

	qaContainer.colorBack = qtclient ? qtclient->background() : 0;
	qaContainer.colorFore = qtclient ? qtclient->foreground() : 0;
	qaContainer.dwAmbientFlags = QACONTAINER_UIDEAD;
	qaContainer.dwAppearance = 0;
	qaContainer.hpal = 0;
	qaContainer.lcid = LOCALE_SYSTEM_DEFAULT;
	qaContainer.pAdviseSink = 0; //(IAdviseSinkEx*)clientsite;
	qaContainer.pBindHost = 0;
	qaContainer.pClientSite = clientsite;
	qaContainer.pFont = qtclient ? qtclient->font() : 0;
	qaContainer.pOleControlSite = 0; //(IOleControlSite*)clientsite;
	qaContainer.pPropertyNotifySink = 0;
	qaContainer.pServiceProvider = 0;
	qaContainer.pUndoMgr = 0;
	qaContainer.pUnkEventSink = 0;
	
	QACONTROL qaControl;
	qaControl.cbSize = sizeof(QACONTROL);
	quickActivated = quick->QuickActivate( &qaContainer, &qaControl ) == S_OK;
    }
#else
    quickActivated = TRUE;
#endif

    DWORD miscStatus;
#ifdef COCREATE
    // Set client site if quick activation was not successfull
    if ( !quickActivated ) {
	ole->GetMiscStatus( DVASPECT_CONTENT, &miscStatus );
	if ( miscStatus & OLEMISC_SETCLIENTSITEFIRST && qtclient )
	    qtclient->connect();
    }
#endif
    // Initialize the control

    // Set client site if not yet done
    if ( !quickActivated ) {
	ole->GetMiscStatus( DVASPECT_CONTENT, &miscStatus );
	if( !(miscStatus & OLEMISC_SETCLIENTSITEFIRST) && qtclient)
	    qtclient->connect();
    }

    if ( !hhook )
	hhook = SetWindowsHookEx( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );

    ++hhookref;

    setFocusPolicy( StrongFocus );

    if ( isVisible() ) {
	RECT r = { x(), y(), x()+width(), y()+height() };
	ole->DoVerb( OLEIVERB_SHOW, NULL, clientsite, 0, winId(), &r );
    }

    if ( parentWidget() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint ) );

    return TRUE;
}

/*!
    Shuts down the ActiveX control.
*/
void QActiveX::clear()
{
    if ( !!control() ) {
	if ( hhook ) {
	    if ( !--hhookref ) {
		UnhookWindowsHookEx( hhook );
		hhook = 0;
	    }
	}
    }

    bool wasVisible = isVisible();
    bool wasHidden = isHidden();
    QRect geom = geometry();
    hide();
    destroy();
    create();
    setGeometry( geom );
    if ( wasVisible )
	show();
    else if ( !wasHidden )
	clearWState( WState_ForceHide );

    QComBase::clear();
    setFocusPolicy( NoFocus );

    if ( clientsite ) {
	IQtClientSite* qtclient;
	clientsite->QueryInterface( IID_IQtClientSite, (void**)&qtclient );
	if ( qtclient ) {
	    qtclient->disconnect();
	    qtclient->Release();
	}
    }
}

/*!
    \fn QObject *QActiveX::qObject()
    \reimp
*/

/*!
    \reimp
*/
const char *QActiveX::className() const
{
    return "QActiveX";
}

/*!
    \reimp
*/
QMetaObject *QActiveX::metaObject() const
{
    return QComBase::metaObject();
}

/*!
    \reimp
*/
QMetaObject *QActiveX::parentMetaObject() const
{
    return QWidget::staticMetaObject();
}

/*!
    \reimp
*/
void *QActiveX::qt_cast( const char *cname )
{
    if ( !qstrcmp( cname, "QActiveX" ) ) return this;
    if ( !qstrcmp( cname, "QComBase" ) ) return (QComBase*)this;
    return QWidget::qt_cast( cname );
}


/*!
    \reimp
*/
bool QActiveX::qt_invoke( int _id, QUObject *_o )
{
    if ( QComBase::qt_invoke( _id, _o ) )
	return TRUE;
    return QWidget::qt_invoke( _id, _o );
}

/*!
    \reimp
*/
bool QActiveX::qt_emit( int _id, QUObject* _o )
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
bool QActiveX::qt_property( int _id, int _f, QVariant *_v )
{
    if ( QComBase::qt_property( _id, _f, _v ) )
	return TRUE;
    return QWidget::qt_property( _id, _f, _v );
}

/*!
    \reimp
*/
void QActiveX::enabledChange( bool old )
{
    QWidget::enabledChange( old );

    if ( old == isEnabled() )
	return;

    CAxWindow ax = winId();
    ax.EnableWindow( isEnabled() );
}

/*!
    \reimp
*/
QSize QActiveX::sizeHint() const
{
    if ( isNull() )
	return QWidget::sizeHint();

    if ( extent.isValid() )
	return extent;

    return QWidget::sizeHint();
}

/*!
    \reimp
*/
QSize QActiveX::minimumSizeHint() const
{
    if ( isNull() )
	return QWidget::minimumSizeHint();

    if ( extent.isValid() )
	return extent;

    return QWidget::minimumSizeHint();
}

/*!
    \reimp
*/
void QActiveX::fontChange( const QFont &old )
{
    QWidget::fontChange( old );

    QFont f = font();
    if ( f == old )
	return;

    CAxWindow ax = winId();
    ax.SetFont( f.handle(), TRUE );

    CComPtr<IOleControl> ole;
    queryInterface( IID_IOleControl, (void**)&ole );
    if ( ole ) {
	ole->OnAmbientPropertyChange( DISPID_AMBIENT_FONT );
    }
}

/*!
    \reimp
*/
void QActiveX::paletteChange( const QPalette &old )
{
    QWidget::paletteChange( old );
    if ( palette() == old )
	return;

    CComPtr<IOleControl> ole;
    queryInterface( IID_IOleControl, (void**)&ole );
    if ( ole ) {
	ole->OnAmbientPropertyChange( DISPID_AMBIENT_BACKCOLOR );
	ole->OnAmbientPropertyChange( DISPID_AMBIENT_FORECOLOR );
    }
}

/*!
    \reimp
*/
void QActiveX::setUpdatesEnabled( bool on )
{
    CAxWindow ax = winId();
    ax.SetRedraw( on );

    QWidget::setUpdatesEnabled( on );
}

/*!
    \reimp
*/
void QActiveX::windowActivationChange( bool old )
{
    QWidget::windowActivationChange( old );

    CComPtr<IOleInPlaceActiveObject> inplace;
    queryInterface( IID_IOleInPlaceActiveObject, (void**)&inplace );
    if ( inplace )
	inplace->OnFrameWindowActivate( isActiveWindow() );
}

#include "qactivex.h"

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
#include <comdef.h>
#include <oleidl.h>

#include <qapplication.h>
#include <qmetaobject.h>

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
	if ( msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST ) {
	    HWND hwnd = msg->hwnd;
	    QWidget *widget = QWidget::find( hwnd );
	    while ( !widget && hwnd ) {
		hwnd = ::GetParent( hwnd );
		widget = QWidget::find( hwnd );
	    }
	    if ( widget && widget->inherits( "QActiveX" ) ) {
		//::SendMessage( widget->winId(), msg.message, msg.wParam, msg.lParam );
		for ( i=0; (UINT)mouseTbl[i] != msg->message || !mouseTbl[i]; i += 3 )
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
		QApplication::sendEvent( widget, &e );
		// this would eat the event, but it doesn't work with designer...
/*		if ( e.isAccepted() ) 
		    msg->message = WM_NULL;*/
	    }
	}
	reentrant = FALSE;
    }
    return CallNextHookEx( hhook, nCode, wParam, lParam );
}

extern BSTR QStringToBSTR( const QString &str );

/*!
    \class QClientSite
    \brief The QClientSite class implements the interfaces for the OLE client to communicate with the OLE object.
*/
class QClientSite : public IDispatch,
		    public IAdviseSink,
		    public IOleControlSite,
		    public IOleClientSite,
		    //public IOleContainer,
		    public IOleInPlaceSite,
		    public IOleInPlaceFrame,
		    public ISimpleFrameSite
{
public:
    QClientSite( QActiveX *ax ) : ref( 0 ), activex( ax )
    {
	control = ax->iface();
	if ( control ) {
	    control->AddRef();

	    CComPtr<IOleObject> ole;
	    control->QueryInterface( IID_IOleObject, (void**)&ole );
	    if ( ole ) {
		ole->Advise( this, &oleconnection );
		ole->SetClientSite( this );
		QString appname;
		if ( qApp->mainWidget() )
		    appname = qApp->mainWidget()->caption();
		else 
		    appname = activex->topLevelWidget()->caption();
		if ( appname.isEmpty() )
		    appname = qApp->name();
		ole->SetHostNames( QStringToBSTR( appname ), 0 );
	    }

	    CComPtr<IOleInPlaceActiveObject> inplace;
	    control->QueryInterface( IID_IOleInPlaceActiveObject, (void**)&inplace );
	    if ( inplace ) {
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = activex->width();
		rc.bottom = activex->height();
		inplace->ResizeBorder( &rc, this, TRUE );
	    }
	}
    }
    ~QClientSite()
    {
	if ( control ) {
	    CComPtr<IOleObject> ole;
	    control->QueryInterface( IID_IOleObject, (void**)&ole );
	    if ( ole )
		ole->Unadvise( oleconnection );

	    control->Release();
	}
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
	else if ( riid == IID_IAdviseSink )
	    *ppvObject = (IAdviseSink*)this;
	else if ( riid == IID_IOleControlSite )
	    *ppvObject = (IOleControlSite*)this;
	else if ( riid == IID_IOleWindow )
	    *ppvObject = (IOleWindow*)(IOleInPlaceUIWindow*)this;
	else if ( riid == IID_IOleInPlaceUIWindow )
	    *ppvObject = (IOleInPlaceUIWindow*)this;
	else if ( riid == IID_IOleClientSite )
	    *ppvObject = (IOleClientSite*)this;
/*	else if ( riid == IID_IOleContainer )
	    *ppvObject = (IOleContainer*)this;*/
	else if ( riid == IID_IOleInPlaceSite )
	    *ppvObject = (IOleInPlaceSite*)this;
	else if ( riid == IID_IOleInPlaceFrame )
	    *ppvObject = (IOleInPlaceFrame*)this;
	else if ( riid == IID_ISimpleFrameSite )
	    *ppvObject = (ISimpleFrameSite*)this;
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
	case DISPID_AMBIENT_BACKCOLOR: // OLE_COLOR
	    pVarResult->vt = VT_I4;
	    pVarResult->lVal = 0;
	    break;
	case DISPID_AMBIENT_DISPLAYNAME: // VT_BSTR
	    pVarResult->vt = VT_BSTR;
	    pVarResult->bstrVal = SysAllocStringByteLen( 0, 0 );
	    break;
	case DISPID_AMBIENT_FONT: // OLE_FONT
	    break;
	case DISPID_AMBIENT_FORECOLOR: // OLE_COLOR
	    pVarResult->vt = VT_I4;
	    pVarResult->lVal = 10000;
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

    // IOleControlSite
    HRESULT __stdcall OnControlInfoChanged()
    {
	qDebug( "IOleControlSite::OnControlinfoChanged" );
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
	return E_NOTIMPL;
    }
    HRESULT __stdcall TransformCoords( POINTL *pPtlHimetric, POINTF *pPtfContainer, DWORD dwFlags )
    {
	if ( !pPtlHimetric || !pPtfContainer )
	    return E_POINTER;
	return E_NOTIMPL;
    }
    HRESULT __stdcall TranslateAccelerator( MSG *pMsg, DWORD grfModifiers )
    {
	qDebug( "IOleControlSite::TranslateAccelerator" );
	return E_NOTIMPL;
    }
    HRESULT __stdcall OnFocus( BOOL fGotFocus )
    {
	qDebug( "IOleControlSite::OnFocus" );
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
	return INPLACE_E_NOTOOLSPACE;
    }
    HRESULT __stdcall RequestBorderSpace( LPCBORDERWIDTHS pborderwidths )
    {
	qDebug( "IOleInPlaceUIWindow::RequestBorderSpace" );
	return INPLACE_E_NOTOOLSPACE;
    }
    HRESULT __stdcall SetBorderSpace( LPCBORDERWIDTHS pborderwidths )
    {
	qDebug( "IOleInPlaceUIWindow::SetBorderSpace" );
	return S_OK;
    }
    HRESULT __stdcall SetActiveObject( IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName )
    {
	qDebug( "IOleInPlaceUIWindow::SetActiveObject" );
	return S_OK;
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
	return E_NOTIMPL;
    }

    // IOleContainer ###


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
					LPRECT lprecPosRect, 
					LPRECT lprcClipRect, 
					LPOLEINPLACEFRAMEINFO lpFrameInfo )
    {
	qDebug( "IOleInPlaceSite::GetWindowContext" );
	QueryInterface( IID_IOleInPlaceFrame, (void**)ppFrame );
	QueryInterface( IID_IOleInPlaceUIWindow, (void**)ppDoc );

	return S_OK;
    }
    HRESULT __stdcall Scroll( SIZE scrollExtant )
    {
	qDebug( "IOleInPlaceSite::Scroll" );
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
	// IOleInPlaceObject::SetObjectRects( ... );
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
    IUnknown *control;
    DWORD oleconnection;
    QActiveX *activex;
};


/*!
    \class QActiveX qactivex.h
    \brief The QActiveX class provides a QWidget that wraps an ActiveX control.
*/

/*!
    Since we create the metaobject for this class on the fly we have to make sure
    staticMetaObject works, otherwise it's impossible to subclass QActiveX control
    and have metadata in the subclass.
*/

//static QActiveX *currentInstance = 0;

/*!
    Creates an empty QActiveX widget. To initialize a control, use \link QComBase::setControl setControl \endlink.
*/
QActiveX::QActiveX( QWidget *parent, const char *name )
: QWidget( parent, name ), clientsite( 0 )
{
    //currentInstance = this;
}

/*!
    Creates an QActiveX widget and initializes the ActiveX control \a c.
*/
QActiveX::QActiveX( const QString &c, QWidget *parent, const char *name )
: QWidget( parent, name ), clientsite( 0 )
{
    setControl( c );
    //currentInstance = this;
}

/*!
    Shuts down the ActiveX control and destroys the QActiveX widget, 
    cleaning all allocated resources.
*/
QActiveX::~QActiveX()
{
    clear();
}

/*!
    Initializes the ActiveX control.  
*/
void QActiveX::initialize( IUnknown **ptr )
{
    if ( *ptr || control().isEmpty() )
	return;
    CoInitialize( 0 );
    _Module.Init( 0, GetModuleHandle( 0 ) );

    CAxWindow axWindow = winId();
    *ptr = 0;
    axWindow.CreateControlEx( (unsigned short*)qt_winTchar( control(), TRUE ), 0, 0, ptr );
    if ( !*ptr ) {
	_Module.Term();
	CoUninitialize();
	return;
    }

    if ( clientsite )
	clientsite->Release();
    clientsite = new QClientSite( this );
    clientsite->AddRef();

    if ( !hhook )
	hhook = SetWindowsHookEx( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );

    ++hhookref;

    if ( parentWidget() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint ) );

    fontChange( font() );
}

/*!
    Destroys the ActiveX control.
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

    if ( clientsite ) {
	clientsite->Release();
	clientsite = 0;
    }

    bool wasVisible = isVisible();
    QRect geom = geometry();
    hide();
    destroy();
    create();
    setGeometry( geom );
    if ( wasVisible )
	show();


    QComBase::clear();
}

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
/*
QMetaObject *QActiveX::staticMetaObject()
{
    Q_ASSERT( currentInstance );
    QMetaObject *mo = currentInstance->QActiveX::metaObject();
    currentInstance = 0;
    return mo;
}
*/

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
void QActiveX::enabledChange( bool /*old*/ )
{
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

    CComPtr<IOleObject> ole;
    queryInterface( IID_IOleObject, (void**)&ole );
    if ( ole ) {
	SIZE sz;
	if ( ole->GetExtent( DVASPECT_CONTENT, &sz ) == S_OK )
	    return QSize( sz.cx, sz.cy );
    }
    return QWidget::sizeHint();
}

/*!
    \reimp
*/
QSize QActiveX::minimumSizeHint() const
{
    if ( isNull() )
	return QWidget::minimumSizeHint();

    return QWidget::minimumSizeHint();
}

/*!
    \reimp
*/
void QActiveX::fontChange( const QFont &/*old*/ )
{
    QFont f = font();
    CAxWindow ax = winId();
    ax.SetFont( f.handle(), TRUE );
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

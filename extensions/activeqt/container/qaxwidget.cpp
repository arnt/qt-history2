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

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>

extern int moduleLockCount;
extern void moduleLock();
extern void moduleUnlock();

#include <qapplication.h>
#include <qobjectlist.h>
#include <qstatusbar.h>
#include <qlayout.h>
#include <qmetaobject.h>

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
		    CComPtr<IOleControl> control;
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

/*!
    \class QAxWidget qactivex.h
    \brief The QAxWidget class is a QWidget that wraps an ActiveX control.

    \extension ActiveQt
    \module QAxContainer

    A QAxWidget can be instantiated as an empty object or with the name of the ActiveX control
    it should wrap. The properties, methods and events of the ActiveX control become available as Qt properties, 
    slots and signals as long as only supported data types are used (see the \link QAxBase QAxBase class
    documentation \endlink for a list of supported and unsupported data types). The baseclass QAxBase provides 
    an API to access the ActiveX directly through the IUnknown pointer.

    QAxWidget is a QWidget and can be used as such, e.g. it can be organized in a widget hierarchy, receive events 
    or act as an event filter. Standard widget properties, e.g. \link QWidget::enabled \endlink enabled are supported,
    but it depends on the ActiveX control to implement support for ambient properties like e.g. palette or font. 
    QAxWidget tries to provide the necessary hints.

    \important dynamicCall()
*/

/*!
    Creates an empty QAxWidget widget and propagates \a parent, \a name and \a f to the QWidget constructor. 
    To initialize a control, call \link QAxBase::setControl() setControl \endlink.
*/
QAxWidget::QAxWidget( QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name, f ), clientsite( 0 ), host( 0 )
{
    initContainer();
}

/*!
    Creates an QAxWidget widget and initializes the ActiveX control \a c.
    \a parent, \a name and \a f are propagated to the QWidget contructor.
*/
QAxWidget::QAxWidget( const QString &c, QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name, f ), clientsite( 0 ), host( 0 )
{
    initContainer();

    setControl( c );
}

/*!
    Creates a QAxWidget that wraps the COM object referenced by \a iface.
    \a parent, \a name and \a f are propagated to the QWidget contructor.
*/
QAxWidget::QAxWidget( IUnknown *iface, QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name, f ), QAxBase( iface )
{
}

/*!
    \internal
*/
void QAxWidget::initContainer()
{
    container = new QWidget( this );
    container->resize( size() );
    container->show();    
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

static QAxWidget *current = 0;

class QAxHostWindow : public CAxHostWindow
{
public:
    QAxHostWindow()
	: CAxHostWindow(), activex( current )
    {
    }

    ~QAxHostWindow()
    {
    }

    STDMETHOD(OnFocus)(BOOL bGotFocus )
    {
	if ( bGotFocus ) {
	    QFocusEvent e( QEvent::FocusIn );
	    QApplication::sendEvent( activex, &e );
	} else {
	    QFocusEvent e( QEvent::FocusOut );
	    QApplication::sendEvent( activex, &e );
	}
	return S_OK;
    }

private:
    QAxWidget *activex;
};

/*!
    \reimp
*/
bool QAxWidget::initialize( IUnknown **ptr )
{
    if ( *ptr || control().isEmpty() )
	return FALSE;
    moduleLock();

    *ptr = 0;

    AtlAxWinInit();
    HRESULT hr;
    CComPtr<IUnknown> spUnkContainer;
    CComPtr<IUnknown> spUnkControl;
    IStream *pStream = 0;
    IID iidSink = IID_NULL;
    IUnknown *punkSink = 0;
    
    current = this;
    CComPolyObject<QAxHostWindow> *axhost = new CComPolyObject<QAxHostWindow>( 0 );
    current = 0;
    axhost->SetVoid( 0 );
    axhost->InternalFinalConstructAddRef();
    hr = axhost->FinalConstruct();
    axhost->InternalFinalConstructRelease();
    
    if ( hr == S_OK )
	hr = axhost->QueryInterface(IID_IUnknown, (void**)&spUnkContainer);
    if ( hr != S_OK )
	delete axhost;
    
    if (SUCCEEDED(hr)) {
	CComPtr<IAxWinHostWindow> pAxWindow;
	spUnkContainer->QueryInterface(IID_IAxWinHostWindow, (void**)&pAxWindow);
	CComBSTR bstrName( (TCHAR*)qt_winTchar(control(), TRUE) );
	hr = pAxWindow->CreateControlEx(bstrName, container->winId(), pStream, &spUnkControl, iidSink, punkSink);
    }
    if (SUCCEEDED(hr)) {
	host = spUnkContainer.p;
	spUnkContainer.p = NULL;
    } else {
	host = NULL;
    }
    if (SUCCEEDED(hr)) {
	*ptr = spUnkControl.p;
	spUnkControl.p = NULL;
    } else {
	*ptr = NULL;
    }

    if ( !*ptr ) {
	moduleUnlock();
	return FALSE;
    }

    metaObject();
    if ( !hhook )
	hhook = SetWindowsHookEx( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
    ++hhookref;

    setFocusPolicy( StrongFocus );
    if ( parentWidget() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint ) );

    CComPtr<IOleObject> ole;
    queryInterface( IID_IOleObject, (void**)&ole );
    if ( !ole )
	return TRUE;

    BSTR userType;
    ole->GetUserType( USERCLASSTYPE_SHORT, &userType );
    setCaption( BSTRToQString( userType ) );
    CoTaskMemFree( userType );

    ole->GetClientSite( &clientsite );

    if ( isVisible() ) {
	RECT r = { x(), y(), x()+width(), y()+height() };
	ole->DoVerb( OLEIVERB_SHOW, NULL, clientsite, 0, container->winId(), &r );
    }

    if ( clientsite ) {
	CComPtr<IAxWinAmbientDispatch> adispatch;
	clientsite->QueryInterface( IID_IAxWinAmbientDispatch, (void**)&adispatch );
	if ( adispatch ) {
	    adispatch->put_BackColor( QColorToOLEColor( palette().color( QPalette::Active, QColorGroup::Background ) ) );
	    adispatch->put_ForeColor( QColorToOLEColor( palette().color( QPalette::Active, QColorGroup::Foreground ) ) );
	    adispatch->put_Font( QFontToIFont( font() ) );
	}
    }

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

    delete container;
    container = 0;
    initContainer();

    QAxBase::clear();
    setFocusPolicy( NoFocus );

    if ( clientsite ) {
	clientsite->Release();
	clientsite = 0;
    }
    if ( host ) {
	host->Release();
	host = 0;
    }
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

    CAxWindow ax = container->winId();
    ax.EnableWindow( isEnabled() );
    CComPtr<IOleControl> ole;
    queryInterface( IID_IOleControl, (void**)&ole );
    if ( ole ) {
	ole->OnAmbientPropertyChange( DISPID_AMBIENT_UIDEAD );
    }
}

/*!
    \reimp
*/
QSize QAxWidget::sizeHint() const
{
    if ( isNull() )
	return QWidget::sizeHint();

    if ( !extent.isValid() ) {
	QAxWidget *that = (QAxWidget*)this;
	CComPtr<IOleObject> ole;
	queryInterface( IID_IOleObject, (void**)&ole );
	if ( ole ) {
	    SIZE sz = { 5000, 5000 };
	    ole->SetExtent( DVASPECT_CONTENT, &sz );
	    HRESULT res = ole->GetExtent( DVASPECT_CONTENT, &sz );
	    if ( SUCCEEDED(res) ) {
		SIZE psz;
		AtlHiMetricToPixel( &sz, &psz );
		that->extent.setWidth( psz.cx );
		that->extent.setHeight( psz.cy );
	    }
	}
    }

    if ( extent.isValid() )
	return extent;

    return QWidget::sizeHint();
}

/*!
    \reimp
*/
QSize QAxWidget::minimumSizeHint() const
{
    if ( isNull() )
	return QWidget::minimumSizeHint();

    CComPtr<IOleObject> ole;
    queryInterface( IID_IOleObject, (void**)&ole );
    if ( ole ) {
	SIZE sz = { -1, -1 };
	ole->SetExtent( DVASPECT_CONTENT, &sz );
	HRESULT res = ole->GetExtent( DVASPECT_CONTENT, &sz );
	if ( SUCCEEDED(res) ) {
	    SIZE psz;
	    AtlHiMetricToPixel( &sz, &psz );
	    return QSize( psz.cx, psz.cy );
	}
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

    CAxWindow ax = container->winId();
    ax.SetFont( font().handle(), TRUE );

    if ( clientsite ) {
	CComPtr<IAxWinAmbientDispatch> adispatch;
	clientsite->QueryInterface( IID_IAxWinAmbientDispatch, (void**)&adispatch );
	if ( adispatch )
	    adispatch->put_Font( QFontToIFont( font() ) );
    }
}

/*!
    \reimp
*/
void QAxWidget::paletteChange( const QPalette &old )
{
    QWidget::paletteChange( old );
    if ( isNull() )
	return;

    if ( clientsite ) {
	CComPtr<IAxWinAmbientDispatch> adispatch;
	clientsite->QueryInterface( IID_IAxWinAmbientDispatch, (void**)&adispatch );
	if ( adispatch ) {
	    adispatch->put_BackColor( QColorToOLEColor( palette().color( QPalette::Active, QColorGroup::Background ) ) );
	    adispatch->put_ForeColor( QColorToOLEColor( palette().color( QPalette::Active, QColorGroup::Foreground ) ) );
	}
    }
}

/*!
    \reimp
*/
void QAxWidget::setUpdatesEnabled( bool on )
{
    if ( !isNull() ) {
	CAxWindow ax = container->winId();
	ax.SetRedraw( on );
    }

    QWidget::setUpdatesEnabled( on );
}

/*!
    \reimp
*/
void QAxWidget::windowActivationChange( bool old )
{
    QWidget::windowActivationChange( old );
    if ( isNull() )
	return;

    CComPtr<IOleInPlaceActiveObject> inplace;
    queryInterface( IID_IOleInPlaceActiveObject, (void**)&inplace );
    if ( inplace )
	inplace->OnFrameWindowActivate( isActiveWindow() );
}

/*!
    \reimp
*/
void QAxWidget::reparent( QWidget *parent, WFlags f, const QPoint &p, bool showIt )
{
    QWidget::reparent( parent, f, p, showIt );
    QApplication::postEvent( this, new QEvent( QEvent::Reparent ) );

    if ( container )
	container->resize( size() );
}

/*!
    \reimp
*/
bool QAxWidget::event( QEvent *e )
{
    switch( e->type() ) {
    case QEvent::Reparent:
	{
	    QSize sz = size();
	    resize( 0, 0 );
	    resize( sz );
	}
	break;
    default:
	break;
    }

    return QWidget::event( e );
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

/****************************************************************************
** $Id: $
**
** Implementation of QActiveXControl
**
*****************************************************************************/

#include "qactivex.h"
#include <qapplication.h>
#include <qpainter.h>

#include <windows.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>


Q_EXPORT extern bool qt_win_use_simple_timers;

static int activex_init = 0;			// initialization count
static QApplication *qt_app = 0;		// global application object


/*!
  Constructs a Qt ActiveX control. The constructor does not take any
  parent widget argument since the control will be inserted in a control
  container.
*/

QActiveXControl::QActiveXControl( const char *name )
    : QWidget( 0, name, WStyle_Customize )
{
    com_control = 0;
    is_active   = FALSE;
    old_parent  = 0;
    old_style   = 0;
#if defined(QT_CHECK_STATE)
    if ( qt_app == 0 )
	warning( "QActiveXControl: Qt ActiveX support not initialized" );
#endif
}


/*!
  Destroys the Qt ActiveX control
*/

QActiveXControl::~QActiveXControl()
{
#if defined(QT_CHECK_STATE)
    if ( qt_app == 0 )
	warning( "QActiveXControl: Qt ActiveX support not initialized" );
#endif
}


void QActiveXControl::updateControl()
{
    if ( com_control ) {
	CComControlBase *ccb = (CComControlBase*)com_control;
	ccb->SendOnDataChange();
	ccb->FireViewChange();
    }
}

void QActiveXControl::setComControl( void *control )
{
    com_control = control;
}


/*!
  Reimplement this virtual function to draw the control when it is
  deactivated.
  \sa isActive()
*/

void QActiveXControl::drawControl( QPainter *, const QRect & )
{
    debug("draw control");
}


/*!
  Normal paint event. For testing only. This function will be removed.
*/

void QActiveXControl::paintEvent( QPaintEvent * )
{
    debug("paint event");
}


/*!
  This event is sent at control activation. An active control has
  its own window for displaying data and receiving user events.
  Reimplement the standard event handlers (paintEvent, mousePressEvent
  etc.) for drawing and handling events for the active control.

  \sa deactivateEvent(), isActive()
*/

void QActiveXControl::activateEvent( QEvent * )
{
    debug("activate event");
}


/*!
  This event is sent at control deactivation. A deactivated/passive
  control does not have its own window, but draws in the container
  window or on some other paint device (e.g. a printer).
  Reimplement the drawControl() function for drawing when the control
  is deactivated.

  \sa activateEvent(), isActive()
*/

void QActiveXControl::deactivateEvent( QEvent * )
{
    debug("deactivate event");
}


void QActiveXControl::setActive( bool active, HANDLE parentWindow )
{
    ASSERT( is_active != active );
    if ( is_active == active )
	return;
    is_active = active;
    if ( active ) {
	old_parent = GetParent( winId() );
	old_style  = GetWindowLong( winId(), GWL_STYLE );
	SetParent( winId(), (HWND)parentWindow );
	SetWindowLong( winId(), GWL_STYLE, WS_CHILD
		       | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	show();
    } else {
	hide();
	SetParent( winId(), (HWND)old_parent );
	SetWindowLong( winId(), GWL_STYLE, old_style );
	old_parent = 0;
    }
    QEvent e( active ?
	      QEvent::ActivateControl : QEvent::DeactivateControl );
    QApplication::sendEvent( this, &e );
}


class QAtlPaintDevice : public QPaintDevice {
public:
    QAtlPaintDevice( HDC h )
	: QPaintDevice(QInternal::System) { hdc = h; }
};


/*
  Draws the control. Calls drawControl.
*/

void QActiveXControl::drawControlAtl( void *atlDrawInfo )
{
    ATL_DRAWINFO *di = (ATL_DRAWINFO*)atlDrawInfo;
    RECT& rect = *(RECT*)di->prcBounds;
    QRect r( QPoint(rect.left,rect.top), QPoint(rect.right,rect.bottom) );
    QPainter p;
    QAtlPaintDevice pd( di->hdcDraw );
    p.begin( &pd );
    drawControl( &p, r );
    p.end();
}


/*!
  Handles the control activation/deactivation events.
*/

bool QActiveXControl::event( QEvent *e )
{
    switch ( e->type() ) {
	case QEvent::ActivateControl:
	    activateEvent( e );
	    break;
	case QEvent::DeactivateControl:
	    deactivateEvent( e );
	    break;
	default:
	    return QWidget::event( e );
    }
    return TRUE;
}


/*!
  Initializes the Qt ActiveX support. You can call this function from
  DllMain().

  Example:
  \code
    extern "C"
    BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID )
    {
	if (dwReason == DLL_PROCESS_ATTACH) {
	    _Module.Init(ObjectMap, hInstance);
	    DisableThreadLibraryCalls(hInstance);
	    QActiveXControl::initialize();
	} else if (dwReason == DLL_PROCESS_DETACH) {
	    QActiveXControl::terminate();
	    _Module.Term();
	}
	return TRUE;
    }
  \endcode

  \sa terminate()
*/

void QActiveXControl::initialize()
{
    if ( ++activex_init == 1 ) {
	int dummy = 0;
	ASSERT( qt_app == 0 );
	qt_app = new QApplication( dummy, 0 );
	qt_win_use_simple_timers = TRUE;
	debug( "Created Qt application" );
    }
}


/*!
  Terminates and cleans up the Qt ActiveX support. You can call this
  function from DllMain().

  Example:
  \code
    extern "C"
    BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID )
    {
	if (dwReason == DLL_PROCESS_ATTACH) {
	    _Module.Init(ObjectMap, hInstance);
	    DisableThreadLibraryCalls(hInstance);
	    QActiveXControl::initialize();
	} else if (dwReason == DLL_PROCESS_DETACH) {
	    QActiveXControl::terminate();
	    _Module.Term();
	}
	return TRUE;
    }
  \endcode

  \sa initialize()
*/

void QActiveXControl::terminate()
{
    --activex_init;
    if ( activex_init == 0 ) {
	ASSERT( qt_app != 0 );
	delete qt_app;
	qt_app = 0;
	debug( "Deleted Qt application" );
    } else if ( activex_init < 0 ) {
#if defined(QT_CHECK_STATE)
	warning( "QActiveXControl: Each call to 'initialize' must be balanced "
		 "by a corresponding\n\tcall to 'terminate'" );
#endif
    }
}

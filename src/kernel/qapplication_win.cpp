/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_win.cpp#62 $
**
** Implementation of Win32 startup routines and event handling
**
** Created : 931203
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpainter.h"
#include "qpmcache.h"
#include <ctype.h>

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qapplication_win.cpp#62 $");


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static char	appName[120];			// application name
static HANDLE	appInst;			// handle to app instance
static HANDLE	appPrevInst;			// handle to prev app instance
static int	appCmdShow;			// main window show command
static int	numZeroTimers = 0;		// number of full-speed timers
static HWND	curWin	  = 0;			// current window
static HANDLE	displayDC = 0;			// display device context
static QWidget *desktopWidget	= 0;		// desktop window widget

// static HWND	autoCapture	= 0;		// Qt auto-capture window

#if defined(DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif

static bool	app_do_modal	= FALSE;	// modal mode
static bool	app_exit_loop	= FALSE;	// flag to exit local loop

static QWidgetList *modal_stack = 0;		// stack of modal widgets
static QWidgetList *popupWidgets= 0;		// list of popup widgets
static bool	    popupCloseDownMode = FALSE;

typedef void  (*VFPTR)();
typedef Q_DECLARE(QListM,void) QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

static void	msgHandler( QtMsgType, const char * );

static void	cleanupPostedEvents();

static void	initTimers();
static void	cleanupTimers();
static bool	activateTimer( uint );
static void	activateZeroTimers();

WindowsVersion	qt_winver = WV_NT;

QObject	       *qt_clipboard = 0;

static bool	qt_try_modal( QWidget *, MSG * );

static int	translateKeyCode( int );

#if defined(_WS_WIN32_)
#define __export
#endif

extern "C" LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

class QETWidget : public QWidget		// event translator widget
{
public:
    void	setWFlags( WFlags f )	{ QWidget::setWFlags(f); }
    void	clearWFlags( WFlags f ) { QWidget::clearWFlags(f); }
    QWExtra    *xtra()			{ return QWidget::extraData(); }
    bool	winEvent( MSG *m )	{ return QWidget::winEvent(m); }
    bool	translateMouseEvent( const MSG &msg );
    bool	translateKeyEvent( const MSG &msg, bool grab );
    bool	translatePaintEvent( const MSG &msg );
    bool	translateConfigEvent( const MSG &msg );
    bool	translateCloseEvent( const MSG &msg );
};


typedef Q_DECLARE(QArrayM,pchar) ArgV;


/*****************************************************************************
  WinMain() - initializes Windows and calls user's startup function main()
 *****************************************************************************/

#if defined(NEEDS_QMAIN)
int qMain( int, char ** );
#else
extern "C" int main( int, char ** );
#endif


extern "C"
int APIENTRY WinMain( HANDLE instance, HANDLE prevInstance,
		      LPSTR  cmdParam, int cmdShow )
{
  // Install default debug handler

    qInstallMsgHandler( msgHandler );

  // Create command line

    GetModuleFileName( instance, appName, sizeof(appName) );

    char *p = cmdParam;
    int	 argc = 1;
    ArgV argv( 8 );
    argv[0] = appName;

    while ( *p ) {				// parse cmd line arguments

	while ( isspace(*p) )			// skip white space
	    p++;

	if ( *p ) {				// arg starts
	    int quote;
	    char *start, *r;
	    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
		quote = *p;
		start = ++p;
	    } else {
		quote = 0;
		start = p;
	    }
	    r = start;
	    while ( *p ) {
		if ( *p == '\\' ) {		// escape char?
		    p++;
		    if ( *p == '\"' || *p == '\'' )
			;			// yes
		    else
			p--;			// treat \ literally
		} else if ( quote ) {
		    if ( *p == quote ) {
			p++;
			if ( isspace(*p) )
			    break;
			quote = 0;
		    }
		} else {
		    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
			quote = *p++;
			continue;
		    } else if ( isspace(*p) )
			break;
		}
		*r++ = *p++;
	    }
	    if ( *p )
		p++;
	    *r = '\0';

	    if ( argc >= (int)argv.size() )	// expand array
		argv.resize( argv.size()*2 );
	    argv[argc++] = start;
	}
    }
    p = strrchr( argv[0], '\\' );
    if ( p )
	strcpy( appName, p+1 );

  // Get Windows parameters

    appInst = instance;
    appPrevInst = prevInstance;
    appCmdShow = cmdShow;


  // Detect the Windows version

    DWORD dwVer = GetVersion();
    if ( dwVer < 0x80000000 )
	qt_winver = WV_NT;		// Windows NT 3.x
    else if ( LOBYTE(LOWORD(dwVer)) < 4 )
	qt_winver = WV_32s;		// Win32s
    else
	qt_winver = WV_95;		// Windows 95

  // Call user main()

    int retcode = main( argc, argv.data() );

    return retcode;
}


/*****************************************************************************
  qt_init() - initializes Qt for Windows
 *****************************************************************************/

void qt_init( int *argcptr, char **argv )
{
#if defined(DEBUG)
    int argc = *argcptr;
    int i, j;

  // Get command line params

    j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QString arg = argv[i];
	if ( arg == "-nograb" )
	    appNoGrab = !appNoGrab;
	else
	    argv[j++] = argv[i];
    }
    *argcptr = j;
#endif // DEBUG

  // Misc. initialization

    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
    qApp->setName( appName );
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    cleanupPostedEvents();			// remove list of posted events
    if ( postRList ) {
	VFPTR f = (VFPTR)postRList->first();
	while ( f ) {				// call post routines
	    (*f)();
	    postRList->remove();
	    f = (VFPTR)postRList->first();
	}
	delete postRList;
    }
    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
    if ( displayDC )
	ReleaseDC( 0, displayDC );
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

static void msgHandler( QtMsgType, const char *str )
{
    OutputDebugString( str );
    OutputDebugString( "\n" );
}


void qAddPostRoutine( CleanUpFunction p )
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	CHECK_PTR( postRList );
    }
    postRList->insert( 0, (void *)p );		// store at list head
}


char *qAppName()				// get application name
{
    return appName;
}

HANDLE qWinAppInst()				// get Windows app handle
{
    return appInst;
}

HANDLE qWinAppPrevInst()			// get Windows prev app handle
{
    return appPrevInst;
}

int qWinAppCmdShow()				// get main window show command
{
    return appCmdShow;
}

HANDLE qt_display_dc()				// get display DC
{
    if ( !displayDC )
	displayDC = GetDC( 0 );
    return displayDC;
}

bool qt_nograb()				// application no-grab option
{
    return appNoGrab;
}


const char *qt_reg_winclass( int type )		// register window class
{
    static bool widget = FALSE;
    static bool popup  = FALSE;
    const char *className;
    uint style = 0;
    if ( type == 0 ) {
	className = "QWidget";
	if ( !widget ) {
	    widget = TRUE;
	    style = CS_DBLCLKS;
	}
    } else if ( type == 1 ) {
	className = "QPopup";
	if ( !popup ) {
	    popup = TRUE;
	    style = CS_DBLCLKS | CS_SAVEBITS;
	}
    } else {
#if defined(DEBUG)
	warning( "Qt internal error: Invalid window class type", type );
#endif
	className = 0;
    }
    if ( style != 0 ) {
	WNDCLASS wc;
	wc.style	 = style;
	wc.lpfnWndProc	 = (WNDPROC)WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = qWinAppInst();
	wc.hIcon	 = type == 0 ? LoadIcon(0,IDI_APPLICATION) : 0;
	wc.hCursor	 = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName	 = 0;
	wc.lpszClassName = className;
	RegisterClass( &wc );
    }
    return className;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;			// set main widget
}


QWidget *QApplication::desktop()
{
    if ( !desktopWidget ) {			// not created yet
	desktopWidget = new QWidget( 0, "desktop", WType_Desktop );
	CHECK_PTR( desktopWidget );
    }
    return desktopWidget;
}


/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef Q_DECLARE(QListM,QCursor) QCursorList;

static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    POINT    p;
    HANDLE   win;
    QWidget *w;
    p.x = x;  p.y = y;
    win = WindowFromPoint( p );
    if ( !win )
	return 0;
    w = QWidget::find( win );
    if ( !w )
	return 0;
    if ( child ) {
	HANDLE cwin = ChildWindowFromPoint( win, p );
	if ( cwin && cwin != win )
	    return QWidget::find( cwin );
    }
    return w;
}


void QApplication::beep()
{
    MessageBeep( MB_OK );
}


/*****************************************************************************
  QApplication management of posted events
 *****************************************************************************/

class QPEObject : public QObject		// trick to set/clear pendEvent
{
public:
    void setPendEventFlag()	{ pendEvent = TRUE; }
    void clearPendEventFlag()	{ pendEvent = FALSE; }
};

class QPEvent : public QEvent			// trick to set/clear posted
{
public:
    QPEvent( int type ) : QEvent( type ) {}
    void setPostedFlag()	{ posted = TRUE; }
    void clearPostedFlag()	{ posted = FALSE; }
};

struct QPostEvent {
    QPostEvent( QObject *r, QEvent *e ) { receiver=r; event=e; }
   ~QPostEvent()			{ delete event; }
    QObject  *receiver;
    QEvent   *event;
};

Q_DECLARE(QListM,QPostEvent);
static QListM(QPostEvent) *postedEvents = 0;	// list of posted events


void QApplication::postEvent( QObject *receiver, QEvent *event )
{
    if ( !postedEvents ) {			// create list
	postedEvents = new QListM(QPostEvent);
	CHECK_PTR( postedEvents );
	postedEvents->setAutoDelete( TRUE );
    }
#if defined(CHECK_NULL)
    if ( receiver == 0 )
	warning( "QApplication::postEvent: Unexpeced NULL receiver" );
#endif
    ((QPEObject*)receiver)->setPendEventFlag();
    ((QPEvent*)event)->setPostedFlag();
    postedEvents->append( new QPostEvent(receiver,event) );
}

static void sendPostedEvents()			// transmit posted events
{
    int count = postedEvents ? postedEvents->count() : 0;
    while ( count-- ) {				// just send to existing recvs
	register QPostEvent *pe = postedEvents->first();
	if ( pe->event ) {			// valid event
	    QApplication::sendEvent( pe->receiver, pe->event );
	    ((QPEvent*)pe->event)->clearPostedFlag();
	}
	((QPEObject*)pe->receiver)->clearPendEventFlag();
	postedEvents->remove( (uint)0 );
    }
}

void qRemovePostedEvents( QObject *receiver )	// remove receiver from list
{
    if ( !postedEvents )
	return;
    register QPostEvent *pe = postedEvents->first();
    while ( pe ) {
	if ( pe->receiver == receiver ) {	// remove this receiver
	    ((QPEObject*)pe->receiver)->clearPendEventFlag();
	    ((QPEvent*)pe->event)->clearPostedFlag();
	    postedEvents->remove();
	    pe = postedEvents->current();
	}
	else
	    pe = postedEvents->next();
    }
}

void qRemovePostedEvent( QEvent *event )	// remove event in list
{
    if ( !postedEvents )
	return;
    register QPostEvent *pe = postedEvents->first();
    while ( pe ) {
	if ( pe->event == event )		// make this event invalid
	    pe->event = 0;			//   will not be sent!
	pe = postedEvents->next();
    }
}

static void cleanupPostedEvents()		// cleanup list
{
    delete postedEvents;
    postedEvents = 0;
}


/*****************************************************************************
  Safe configuration (move,resize,setGeometry) mechanism to avoid
  recursion when processing messages.
 *****************************************************************************/

#include "qqueue.h"

struct QWinConfigRequest {
    WId	 id;					// widget to be configured
    int	 req;					// 0=move, 1=resize, 2=setGeo
    int	 x, y, w, h;				// request parameters
};

Q_DECLARE(QQueueM,QWinConfigRequest);
static QQueueM(QWinConfigRequest) *configRequests = 0;

void qWinRequestConfig( WId id, int req, int x, int y, int w, int h )
{
    if ( !configRequests )			// create queue
	configRequests = new QQueueM(QWinConfigRequest);
    QWinConfigRequest *r = new QWinConfigRequest;
    r->id = id;					// create new request
    r->req = req;
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    configRequests->enqueue( r );		// store request in queue
}

static void qWinProcessConfigRequests()		// perform requests in queue
{
    if ( !configRequests )
	return;
    QWinConfigRequest *r;
    while ( 1 ) {
	if ( configRequests->isEmpty() )
	    break;
	r = configRequests->dequeue();
	QWidget *w = QWidget::find( r->id );
	if ( w ) {				// widget exists
	    if ( w->testWFlags( WConfigPending ) ) // biting our tail
		return;
	    if ( r->req == 0 )
		w->move( r->x, r->y );
	    else
	    if ( r->req == 1 )
		w->resize( r->w, r->h );
	    else
		w->setGeometry( r->x, r->y, r->w, r->h );
	}
	delete r;
    }
    delete configRequests;
    configRequests = 0;
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    enter_loop();
    return quit_code;
}


int QApplication::enter_loop()
{
    loop_level++;

    while ( !quit_now && !app_exit_loop ) {

	MSG msg;

	if ( numZeroTimers ) {			// activate full-speed timers
	    int m;
	    while ( numZeroTimers && !(m=PeekMessage(&msg,0,0,0,PM_REMOVE)) )
		activateZeroTimers();
	    if ( !m )				// no event
		continue;
	}
	else if ( !GetMessage(&msg,0,0,0) )
	    break;

	if ( winEventFilter( &msg ) )		// pass through event filter
	    continue;

	if ( msg.message == WM_TIMER ) {	// timer message received
	    activateTimer( msg.wParam );
	    continue;
	}

	if ( msg.message == WM_KEYDOWN || msg.message == WM_KEYUP ) {
	    if ( translateKeyCode(msg.wParam) == 0 ) {
		TranslateMessage( &msg );	// translate to WM_CHAR
		continue;
	    }
	}
	DispatchMessage( &msg );		// send to WndProc
	if ( configRequests )			// any pending configs?
	    qWinProcessConfigRequests();
	if ( postedEvents && postedEvents->count() )
	    sendPostedEvents();
    }

    app_exit_loop = FALSE;
    loop_level--;

    return 0;
}

void QApplication::exit_loop()
{
    app_exit_loop = TRUE;
}


bool QApplication::winEventFilter( MSG * )	// Windows event filter
{
    return FALSE;
}


void QApplication::winFocus( QWidget *w, bool gotFocus )
{
    if ( gotFocus ) {
	while ( w->parentWidget() )		// go to top level
	    w = w->parentWidget();
	while ( w->focusChild )			// go down focus chain
	    w = w->focusChild;
	if ( w != focus_widget && w->acceptFocus() ) {
	    focus_widget = w;
	    QFocusEvent in( Event_FocusIn );
	    QApplication::sendEvent( w, &in );
	}
    } else {
	if ( focus_widget ) {
	    QFocusEvent out( Event_FocusOut );
	    QWidget *widget = focus_widget;
	    focus_widget = 0;
	    QApplication::sendEvent( widget, &out );
	}
    }
}


//
// WndProc() receives all messages from the main event loop
//

extern "C"
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam,
			  LPARAM lParam )
{
    if ( !qApp )				// unstable app state
	return DefWindowProc(hwnd,message,wParam,lParam);

    MSG msg;
    msg.hwnd = hwnd;				// create MSG structure
    msg.message = message;			// time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;

    if ( qApp->winEventFilter(&msg) )		// send through app filter
	return 0;

    QETWidget *widget = (QETWidget*)QWidget::find( hwnd );
    if ( !widget )				// don't know this widget
	return DefWindowProc(hwnd,message,wParam,lParam);

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, &msg) )
	    return 0;

    if ( widget->winEvent(&msg) )		// send through widget filter
	return 0;

    int evt_type = Event_None;
    bool result = TRUE;

    if ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST ) {
	if ( widget->isEnabled() ) {
	    if ( message == WM_LBUTTONDOWN )
		widget->setFocus();
	    widget->translateMouseEvent( msg ); // mouse event
	}
    }
    else
    switch ( message ) {

	case WM_KEYDOWN:			// keyboard event
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR: {
	    QWidget *g = QWidget::keyboardGrabber();
	    if ( g )
		widget = (QETWidget*)g;
	    else if ( qApp->focusWidget() )
		widget = (QETWidget*)qApp->focusWidget();
	    if ( widget->isEnabled() )
		widget->translateKeyEvent( msg, g != 0 );
	    }
	    break;

	case WM_PAINT:				// paint event
	    result = widget->translatePaintEvent( msg );
	    break;

	case WM_ERASEBKGND: {			// erase window background
	    HDC	     hdc = (HDC)wParam;
	    RECT     rect;
	    HBRUSH   brush;
	    HPALETTE oldPal;
	    GetClientRect( hwnd, &rect );
	    if ( QColor::hPal() ) {
		oldPal = SelectPalette( hdc, QColor::hPal(), FALSE );
		RealizePalette( hdc );
	    }
	    const QPixmap *bgpm = widget->backgroundPixmap();
	    if ( bgpm )
		brush = CreatePatternBrush( bgpm->hbm() );
	    else
		brush = CreateSolidBrush( widget->backgroundColor().pixel() );
	    HBRUSH oldBrush = SelectObject( hdc, brush );
	    PatBlt( hdc, 0, 0, rect.right, rect.bottom, PATCOPY );
	    SelectObject( hdc, oldBrush );
	    DeleteObject( brush );
	    if ( QColor::hPal() ) {
		SelectPalette( hdc, oldPal, TRUE );
		RealizePalette( hdc );
	    }
	    }
	    break;

	case WM_MOVE:				// move window
	case WM_SIZE:				// resize window
	    result = widget->translateConfigEvent( msg );
	    break;

	case WM_ACTIVATE:
	    qApp->winFocus( widget, LOWORD(wParam) == WA_INACTIVE ? 0 : 1 );
	    break;

	case WM_PALETTECHANGED:			// our window changed palette
	    if ( QColor::hPal() && (WId)wParam == widget->winId() )
		return 0;			// otherwise: FALL THROUGH!

	case WM_QUERYNEWPALETTE:		// realize own palette
	    if ( QColor::hPal() ) {
		HDC hdc = GetDC( widget->winId() );
		HPALETTE hpalOld = SelectPalette( hdc, QColor::hPal(), FALSE );
		uint n = RealizePalette( hdc );
		if ( n )
		    InvalidateRect( widget->winId(), 0, TRUE );
		SelectPalette( hdc, hpalOld, TRUE );
		RealizePalette( hdc );
		ReleaseDC( widget->winId(), hdc );
		return n;
	    }
	    break;

	case WM_CLOSE:				// close window
	    if ( widget->translateCloseEvent(msg) )
		delete widget;
	    return 0;				// always handled
	    break;

	case WM_DESTROY:			// destroy window
	    if ( hwnd == curWin ) {
		QEvent leave( Event_Leave );
		QApplication::sendEvent( widget, &leave );
		curWin = 0;
	    }
	    result = FALSE;
	    break;

	case WM_NCACTIVATE:
	    if ( !wParam && popupWidgets )
		result = TRUE;
	    else
		result = FALSE;
	    break;

#if 0
	case WM_NCPAINT:
	    if ( widget->isModal() ) {
		// Its not possible to get a dialog frame for normal windows.
		// Windows always add the system menu when you ask for a close box.
		// The solution is then to draw the dialog frame ourselves.
		HANDLE hdc;
		int x, y;
		RECT r;
		DefWindowProc( hwnd, message, wParam, lParam );
		hdc = GetWindowDC( hwnd );
		GetWindowRect( hwnd, (LPRECT)&rc2 );
		x = GetSystemMetrics(SM_CXSIZE)	  +
		    GetSystemMetrics(SM_CXBORDER) +
		    GetSystemMetrics(SM_CXFRAME);
		y = GetSystemMetrics(SM_CYFRAME);
		rc1.left = x;
		rc1.top = y;
		rc1.right = rc2.right - rc2.left - 2*x -
			     GetSystemMetrics( SM_CXFRAME );
		rc1.bottom = GetSystemMetrics( SM_CYSIZE );

		SetBkColor( hdc, GetSysColor(COLOR_ACTIVECAPTION) );
		SetTextColor( hdc, GetSysColor(COLOR_CAPTIONTEXT) );
		DrawText( hdc, "X", -1,(LPRECT)&rc1, DT_RIGHT );
		ReleaseDC( hwnd, hdc );
		return 0;
	    } else {
		result = FALSE;
	    }
	    break;
#endif

	case WM_GETMINMAXINFO:
	    if ( widget->xtra() ) {
		MINMAXINFO *mmi = (MINMAXINFO *)lParam;
		QWExtra	   *x = widget->xtra();
		QRect	   f  = widget->frameGeometry();
		QSize	   s  = widget->size();
		if ( x->minw > 0 )
		    mmi->ptMinTrackSize.x = x->minw + f.width()	 - s.width();
		if ( x->minh > 0 )
		    mmi->ptMinTrackSize.y = x->minh + f.height() - s.height();
		if ( x->maxw < QCOORD_MAX )
		    mmi->ptMaxTrackSize.x = x->maxw + f.width()	 - s.width();
		if ( x->maxh < QCOORD_MAX )
		    mmi->ptMaxTrackSize.y = x->maxh + f.height() - s.height();
		return 0;
	    }
	    break;

	case WM_CHANGECBCHAIN:
	case WM_DRAWCLIPBOARD:
	    if ( qt_clipboard ) {
		QCustomEvent e( Event_Clipboard, &msg );
		QApplication::sendEvent( qt_clipboard, &e );
		return 0;
	    }
						// NOTE: fall-through!
	default:
	    result = FALSE;			// event was not processed
	    break;

    }

    if ( evt_type != Event_None ) {		// simple event
	QEvent e( evt_type );
	result = QApplication::sendEvent(widget, &e);
    }
    return result ? 0 : DefWindowProc(hwnd,message,wParam,lParam);
}


/*****************************************************************************
  Modal widgets; We have implemented our own modal widget mechanism
  to get total control.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  qt_enter_modal()
	Enters modal state and returns when the widget is hidden/closed
	Arguments:
	    QWidget *widget	A modal widget

  qt_leave_modal()
	Leaves modal state for a widget
	Arguments:
	    QWidget *widget	A modal widget
 *****************************************************************************/

bool qt_modal_state()
{
    return app_do_modal;
}


void qt_enter_modal( QWidget *widget )
{
    if ( !modal_stack ) {			// create modal stack
	modal_stack = new QWidgetList;
	CHECK_PTR( modal_stack );
    }
    modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
    qApp->enter_loop();
}


void qt_leave_modal( QWidget *widget )
{
    if ( modal_stack && modal_stack->removeRef(widget) ) {
	if ( modal_stack->isEmpty() ) {
	    delete modal_stack;
	    modal_stack = 0;
	}
	qApp->exit_loop();
    }
    app_do_modal = modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, MSG *msg )
{
    if ( popupWidgets )				// popup widget mode
	return TRUE;

    QWidget *modal=0, *top=modal_stack->getFirst();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(WType_Modal) )	// widget is modal
	modal = widget;
    if ( modal == top )				// don't block event
	return TRUE;

    bool block_event = FALSE;
    int	 type  = msg->message;

    if ( (type >= WM_MOUSEFIRST && type <= WM_MOUSELAST) ||
	 (type >= WM_KEYFIRST	&& type <= WM_KEYLAST) ) {
	block_event = TRUE;
    }

    return !block_event;
}


/*****************************************************************************
  Popup widget mechanism

  qt_open_popup()
	Adds a widget to the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be added

  qt_close_popup()
	Removes a widget from the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be removed
 *****************************************************************************/

void qt_open_popup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 && !qt_nograb() ) {
	SetCapture( popup->winId() );		// grab mouse/keyboard
	popup->grabKeyboard();
    }
}

void qt_close_popup( QWidget *popup )
{
    if ( !popupWidgets )
	return;
    popupWidgets->removeRef( popup );
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() ) {			// grabbing not disabled
	    ReleaseCapture();
	    popup->releaseKeyboard();
	}
    }
}


/*****************************************************************************
  Timer handling; Our routines depend on Windows timer functions, but we
  need some extra handling to activate objects at timeout.

  Implementation note: There are two types of timer identifiers. Windows
  timer ids (internal use) are stored in TimerInfo.  Qt timer ids are
  indexes (+1) into the timerVec vector.

  NOTE: These functions are for internal use. QObject::startTimer() and
	QObject::killTimer() are for public use.
	The QTimer class provides a high-level interface which translates
	timer events into signals.

  qStartTimer( interval, obj )
	Starts a timer which will run until it is killed with qKillTimer()
	Arguments:
	    int interval	timer interval in milliseconds
	    QObject *obj	where to send the timer event
	Returns:
	    int			timer identifier, or zero if not successful

  qKillTimer( timerId )
	Stops a timer specified by a timer identifier.
	Arguments:
	    int timerId		timer identifier
	Returns:
	    bool		TRUE if successful

  qKillTimer( obj )
	Stops all timers that are sent to the specified object.
	Arguments:
	    QObject *obj	object receiving timer events
	Returns:
	    bool		TRUE if successful
 *****************************************************************************/

//
// Internal data structure for timers
//

#include "qvector.h"
#include "qintdict.h"

struct TimerInfo {				// internal timer info
    uint     ind;				// - Qt timer identifier - 1
    uint     id;				// - Windows timer identifier
    bool     zero;				// - zero timing
    QObject *obj;				// - object to receive events
};
typedef Q_DECLARE(QVectorM,TimerInfo)  TimerVec;	// vector of TimerInfo structs
typedef Q_DECLARE(QIntDictM,TimerInfo) TimerDict; // fast dict of timers

static const int MaxTimers  = 256;		// max number of timers
static TimerVec *timerVec   = 0;		// timer vector
static TimerDict *timerDict = 0;		// timer dict


//
// Timer activation (called from the event loop when WM_TIMER arrives)
//

static bool activateTimer( uint id )		// activate timer
{
    if ( !timerVec )				// should never happen
	return FALSE;
    register TimerInfo *t = timerDict->find( id );
    if ( !t )					// no such timer id
	return FALSE;
    QTimerEvent e( t->ind + 1 );
    QApplication::sendEvent( t->obj, &e );	// send event
    return TRUE;				// timer event was processed
}

static void activateZeroTimers()		// activate full-speed timers
{
    if ( !timerVec )
	return;
    int i=0;
    register TimerInfo *t;
    int n = numZeroTimers;
    while ( n-- ) {
	while ( TRUE ) {
	    t = timerVec->at(i++);
	    if ( t && t->zero )
		break;
	    else if ( i == MaxTimers )		// should not happen
		return;
	}
	QTimerEvent e( t->ind + 1 );
	QApplication::sendEvent( t->obj, &e );
    }
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerVec = new TimerVec( MaxTimers );
    CHECK_PTR( timerVec );
    timerVec->setAutoDelete( TRUE );
    timerDict = new TimerDict( 29 );
    CHECK_PTR( timerDict );
}

static void cleanupTimers()			// remove pending timers
{
    register TimerInfo *t;
    if ( !timerVec )				// no timers were used
	return;
    for ( int i=0; i<MaxTimers; i++ ) {		// kill all pending timers
	t = timerVec->at( i );
	if ( t && !t->zero )
	    KillTimer( 0, t->id );
    }
    delete timerDict;
    timerDict = 0;
    delete timerVec;
    timerVec  = 0;
}


//
// Main timer functions for starting and killing timers
//

int qStartTimer( int interval, QObject *obj )	// start timer
{
    register TimerInfo *t;
    if ( !timerVec )				// initialize timer data
	initTimers();
    int ind = timerVec->findRef( 0 );		// get free timer
    if ( ind == -1 || !obj )			// cannot create timer
	return 0;
    t = new TimerInfo;				// create timer entry
    CHECK_PTR( t );
    t->ind  = ind;
    t->obj  = obj;
    t->zero = interval == 0;
    if ( t->zero ) {				// add zero timer
	t->id = (uint)50000 + ind;		// unique, high id
	numZeroTimers++;
    } else {
	t->id = SetTimer( 0, 0, (uint)interval, 0 );
	if ( t->id == 0 ) {
#if defined(DEBUG)
	    warning( "qStartTimer: No more Windows timers" );
#endif
	    delete t;				// could not set timer
	    return 0;
	}
    }
    timerVec->insert( ind, t );			// store in timer vector
    timerDict->insert( t->id, t );		// store in dict
    return ind + 1;				// return index in vector
}

bool qKillTimer( int ind )			// kill timer with id
{
    if ( !timerVec || ind <= 0 || ind > MaxTimers )
	return FALSE;
    register TimerInfo *t = timerVec->at(ind-1);
    if ( !t )
	return FALSE;
    if ( t->zero )
	numZeroTimers--;
    else
	KillTimer( 0, t->id );
    timerDict->remove( t->id );
    timerVec->remove( ind-1 );
    return TRUE;
}

bool qKillTimer( QObject *obj )			// kill timer(s) for obj
{
    if ( !timerVec )
	return FALSE;
    register TimerInfo *t;
    for ( int i=0; i<MaxTimers; i++ ) {
	t = timerVec->at( i );
	if ( t && t->obj == obj ) {		// object found
	    if ( t->zero )
		numZeroTimers--;
	    else
		KillTimer( 0, t->id );
	    timerDict->remove( t->id );
	    timerVec->remove( i );
	}
    }
    return TRUE;
}


/*****************************************************************************
  Event translation; translates Windows events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// Non-client mouse messages are not translated
//

static ushort mouseTbl[] = {
    WM_MOUSEMOVE,	Event_MouseMove,		0,
    WM_LBUTTONDOWN,	Event_MouseButtonPress,		LeftButton,
    WM_LBUTTONUP,	Event_MouseButtonRelease,	LeftButton,
    WM_LBUTTONDBLCLK,	Event_MouseButtonDblClick,	LeftButton,
    WM_RBUTTONDOWN,	Event_MouseButtonPress,		RightButton,
    WM_RBUTTONUP,	Event_MouseButtonRelease,	RightButton,
    WM_RBUTTONDBLCLK,	Event_MouseButtonDblClick,	RightButton,
    WM_MBUTTONDOWN,	Event_MouseButtonPress,		MidButton,
    WM_MBUTTONUP,	Event_MouseButtonRelease,	MidButton,
    WM_MBUTTONDBLCLK,	Event_MouseButtonDblClick,	MidButton,
    0,			0,				0
};

static int translateButtonState( int s )
{
    int bst = 0;
    if ( s & MK_LBUTTON )
	bst |= LeftButton;
    if ( s & MK_MBUTTON )
	bst |= MidButton;
    if ( s & MK_RBUTTON )
	bst |= RightButton;
    if ( s & MK_SHIFT )
	bst |= ShiftButton;
    if ( s & MK_CONTROL )
	bst |= ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	bst |= AltButton;
    return bst;
}

extern QCursor *qt_grab_cursor();

bool QETWidget::translateMouseEvent( const MSG &msg )
{
    static bool	  capture = FALSE;
    static QPoint pos;
    int	   type;				// event parameters
    int	   button;
    int	   state;
    int	   i;

    for ( i=0; mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3 ) ;
    if ( !mouseTbl[i] )
	return FALSE;
    type   = mouseTbl[++i];			// event type
    button = mouseTbl[++i];			// which button
    state  = translateButtonState( msg.wParam ); // button state

    if ( type == Event_MouseMove ) {
	QCursor *c = qt_grab_cursor();
	if ( !c )
	    c = QApplication::overrideCursor();
	if ( c )				// application cursor defined
	    SetCursor( c->handle() );
	else					// use widget cursor
	    SetCursor( cursor().handle() );
	if ( curWin != winId() ) {		// new current window
	    if ( curWin ) {			// send leave event
		QWidget *curWidget = QWidget::find(curWin);
		if ( curWidget ) {
		    QEvent leave( Event_Leave );
		    QApplication::sendEvent( curWidget, &leave );
		}
	    }
	    curWin = winId();
	    QEvent enter( Event_Enter );	// send enter event
	    QApplication::sendEvent( this, &enter );
	}
	if ( (state == 0 || !capture) && !testWFlags(WState_TrackMouse) )
	    return TRUE;			// no button
	POINT curPos;
	GetCursorPos( &curPos );		// compress mouse move
	ScreenToClient( winId(), &curPos );
	if ( curPos.x == pos.x() && curPos.y == pos.y() )
	    return TRUE;			// same position
	pos.rx() = (short)curPos.x;
	pos.ry() = (short)curPos.y;
    } else {
	pos.rx() = LOWORD(msg.lParam);		// get position
	pos.ry() = HIWORD(msg.lParam);
    }
    if ( popupWidgets ) {			// oops, in popup mode
	QWidget *popup = popupWidgets->last();
	if ( popup != this ) {
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( mapToGlobal(pos) );
	}
	QMouseEvent e( type, pos, button, state );
	QApplication::sendEvent( popup, &e );
    }
    else {
	int bs = state & (LeftButton | RightButton | MidButton);
	if ( (type == Event_MouseButtonPress ||
	      type == Event_MouseButtonDblClick) && bs == button ) {
	    if ( QWidget::mouseGrabber() == 0 ) {
		SetCapture( winId() );
		capture = TRUE;
	    }
	}
	else if ( type == Event_MouseButtonRelease && bs == 0 ) {
	    if ( QWidget::mouseGrabber() == 0 ) {
		ReleaseCapture();
		capture = FALSE;
	    }
	}
	QMouseEvent e( type, pos, button, state );
	QApplication::sendEvent( this, &e );	// send event
	if ( type != Event_MouseMove )
	    pos.rx() = pos.ry() = -9999;	// init for move compression
    }
    return TRUE;
}


//
// Keyboard event translation
//

#include "qkeycode.h"

static ushort KeyTbl[] = {			// keyboard mapping table
    VK_ESCAPE,		Key_Escape,		// misc keys
    VK_TAB,		Key_Tab,
    VK_BACK,		Key_Backspace,
    VK_RETURN,		Key_Return,
    VK_INSERT,		Key_Insert,
    VK_DELETE,		Key_Delete,
//  VK_CLEAR,		Key_Clear,
    VK_PAUSE,		Key_Pause,
    VK_SNAPSHOT,	Key_Print,
    VK_HOME,		Key_Home,		// cursor movement
    VK_END,		Key_End,
    VK_LEFT,		Key_Left,
    VK_UP,		Key_Up,
    VK_RIGHT,		Key_Right,
    VK_DOWN,		Key_Down,
    VK_PRIOR,		Key_Prior,
    VK_NEXT,		Key_Next,
    VK_SHIFT,		Key_Shift,		// modifiers
    VK_CONTROL,		Key_Control,
//  VK_????,		Key_Meta,
//  VK_????,		Key_Meta,
    VK_MENU,		Key_Alt,
    VK_CAPITAL,		Key_CapsLock,
    VK_NUMLOCK,		Key_NumLock,
    VK_SCROLL,		Key_ScrollLock,
    VK_NUMPAD0,		Key_0,			// numeric keypad
    VK_NUMPAD1,		Key_1,
    VK_NUMPAD2,		Key_2,
    VK_NUMPAD3,		Key_3,
    VK_NUMPAD4,		Key_4,
    VK_NUMPAD5,		Key_5,
    VK_NUMPAD6,		Key_6,
    VK_NUMPAD7,		Key_7,
    VK_NUMPAD8,		Key_8,
    VK_NUMPAD9,		Key_9,
    VK_MULTIPLY,	Key_Asterisk,
    VK_ADD,		Key_Plus,
    VK_SEPARATOR,	Key_Comma,
    VK_SUBTRACT,	Key_Minus,
    VK_DECIMAL,		Key_Period,
    VK_DIVIDE,		Key_Slash,
    0,			0
};

static int translateKeyCode( int key )		// get Key_... code
{
    int code;
    if ( (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') )
	code = 0;				// wait for WM_CHAR instead
    else if ( key >= VK_F1 && key <= VK_F24 )	// function keys
	code = Key_F1 + (key - VK_F1);		// assumes contiguous codes!
    else {
	int i = 0;				// any other keys
	code = 0;
	while ( KeyTbl[i] && code == 0 ) {
	    if ( key == (int)KeyTbl[i] )
		code = KeyTbl[i+1];
	    else
		i += 2;
	}
    }
    return code;
}

bool QETWidget::translateKeyEvent( const MSG &msg, bool grab )
{
    int type;
    int code;
    int ascii = 0;
    int state = 0;

    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= AltButton;

    if ( msg.message == WM_CHAR ) {		// translated keyboard code
	type = Event_KeyPress;
	code = ascii = msg.wParam;
	if ( code >= 'a' && code <= 'z' )
	    code = toupper( code );
	if ( (state & ControlButton) != 0 ) {
	    if ( code >= 1 && code <= 26 )	// Ctrl+'A'..'Z'
		code += 'A' - 1;
	}
    } else {
	code = translateKeyCode( msg.wParam );
	if ( code == 0 )
	    return FALSE;			// virtual key not found
	type = msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN ?
	       Event_KeyPress : Event_KeyRelease;
    }
    if ( type == Event_KeyPress && !grab ) {	// send accel event to tlw
	QKeyEvent a( Event_Accel, code, ascii, state );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	if ( a.isAccepted() )
	    return TRUE;
    }
    QKeyEvent e( type, code, ascii, state );
    return QApplication::sendEvent( this, &e ); // send event
}


//
// Paint event translation
//

bool QETWidget::translatePaintEvent( const MSG & )
{
    PAINTSTRUCT ps;
    RECT rect;
    GetUpdateRect( winId(), &rect, FALSE );
    QRect r( QPoint(rect.left,rect.top), QPoint(rect.right,rect.bottom) );
    QPaintEvent e( r );
    setWFlags( WState_PaintEvent );
    hdc = BeginPaint( winId(), &ps );
    QApplication::sendEvent( this, &e );
    EndPaint( winId(), &ps );
    hdc = 0;
    clearWFlags( WState_PaintEvent );
    return TRUE;
}


//
// Window move and resize (configure) events
//

bool QETWidget::translateConfigEvent( const MSG &msg )
{
    if ( !testWFlags(WState_Created) )		// in QWidget::create()
	return TRUE;
    setWFlags( WConfigPending );		// set config flag
    QRect r = geometry();
    WORD a = LOWORD(msg.lParam);
    WORD b = HIWORD(msg.lParam);
    if ( msg.message == WM_SIZE ) {		// resize event
	QSize oldSize = size();
	QSize newSize( a, b );
	r.setSize( newSize );
	setCRect( r );
	QResizeEvent e( newSize, oldSize );
	QApplication::sendEvent( this, &e );
	if ( isTopLevel() ) {			// update caption/icon text
	    if ( IsIconic(winId()) && iconText() )
		SetWindowText( winId(), iconText() );
	    else if ( caption() )
		SetWindowText( winId(), caption() );
	}
	update();
    } else if ( msg.message == WM_MOVE ) {	// move event
	QPoint oldPos = pos();
	QPoint newPos( a, b );
	r.moveTopLeft( newPos );
	setCRect( r );
	QMoveEvent e( newPos, oldPos );
	QApplication::sendEvent( this, &e );
    }
    clearWFlags( WConfigPending );		// clear config flag
    return TRUE;
}


//
// Close window event translation
//

bool QETWidget::translateCloseEvent( const MSG & )
{
    WId	 id	= winId();
    bool isMain = qApp->mainWidget() == this;
    QCloseEvent e;
    bool accept = QApplication::sendEvent( this, &e );
    if ( !QWidget::find(id) ) {			// widget was deleted
	if ( isMain )
	    qApp->quit();
	return FALSE;
    }
    if ( accept ) {
	hide();
	if ( isMain )
	    qApp->quit();
	else if ( testWFlags(WDestructiveClose) )
	    return TRUE;
    }
    return FALSE;
}

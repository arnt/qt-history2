/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp_win.cpp#5 $
**
** Implementation of Windows + NT startup routines and event handling
**
** Author  : Haavard Nord
** Created : 931203
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qevent.h"
#include "qview.h"
#include "qwininfo.h"
#include "qstring.h"
#include <ctype.h>
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapp_win.cpp#5 $";
#endif


// --------------------------------------------------------------------------
// Internal variables and functions
//

static char	appName[120];			// application name
static HANDLE	appInst;			// handle to app instance
static HANDLE	prevAppInst;			// handle to prev app instance
static int	appCmdShow;			// main window show command
static int	numZeroTimers = 0;		// number of full-speed timers
static HWND	curWin = 0;			// current window
static QWidget *curWidget;
#if defined(USE_HEARTBEAT)
static int	heartBeat = 0;			// heatbeat timer
#endif

typedef void  (*VFPTR)();
static VFPTR   *cleanupRvec = 0;		// vector of cleanup routines
static uint	cleanupRcount = 0;

static void	debugHandler( const char * );	// default debug handler

static void	initTimers();
static void	cleanupTimers();
static bool	activateTimer( uint );
static void	activateZeroTimers();

static int translateKeyCode( int );

class QETWidget : public QWidget {		// event translator widget
public:
    bool translateMouseEvent( const MSG & );
    bool translateKeyEvent( const MSG & );
    bool translatePaintEvent( const MSG & );
    bool translateConfigEvent( const MSG & );
    bool translateCloseEvent( const MSG & );
};


// --------------------------------------------------------------------------
// WinMain() - initializes Windows and calls user's startup function qMain()
//

extern "C"
int PASCAL WinMain( HANDLE instance, HANDLE prevInstance,
		    LPSTR  cmdParam, int cmdShow )
{
#if defined(TRACE_FS)
    startFSTrace();
#endif

  // Install default debug handler

    installDebugHandler( (void (*)(char*))debugHandler );

  // Create command line

    GetModuleFileName( instance, appName, sizeof(appName) );
    char *p = cmdParam;
    int	 argc = 1;
    char **argv = new pchar[ strlen( p )/3 + 3 ];
    argv[0] = appName;
    while ( *p ) {				// parse cmd line arguments
	while ( isspace(*p) )
	    p++;
	argv[argc++] = p;
	while ( !isspace(*p) && *p )
	    p++;
	if ( *p )
	    *p++ = '\0';
    }
    p = strrchr( argv[0], '\\' );
    if ( p )
	strcpy( appName, p+1 );

  // Get Windows parameters

    appInst = instance;
    prevAppInst = prevInstance;
    appCmdShow = cmdShow;

  // Misc. initialization

    QWinInfo::initialize();
    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
#if defined(USE_HEARTBEAT)
    heartBeat = SetTimer( 0, 0, 100, 0 );
#endif

  // Call user-provided main routine

    int returnCode = qMain( argc, argv );

  // Cleanup

    if ( cleanupRvec ) {
	while ( cleanupRcount )			// call cleanup routines
	    (*(cleanupRvec[--cleanupRcount]))();
	delete cleanupRvec;
    }
    if ( qApp )
	delete qApp;
    QApplication::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
    cleanupTimers();
#if defined(USE_HEARTBEAT)
    KillTimer( 0, heartBeat );
#endif

#if defined(TRACE_FS)
    stopFSTrace();
#endif
    return returnCode;
}


void debugHandler( const char *str )		// print debug message
{
    OutputDebugString( str );
    OutputDebugString( "\n" );
}


void qAddCleanupRoutine( void (*p)() )		// add cleanup routine
{
    if ( !cleanupRvec )
	cleanupRvec = new VFPTR[16];		// plenty of room
    cleanupRvec[cleanupRcount++] = p;		// save pointer to routine
}


// --------------------------------------------------------------------------
// Global functions that export important data
//

char *qAppName()				// get application name
{
    return appName;
}

HANDLE qWinAppInst()				// get Windows app handle
{
    return appInst;
}

HANDLE qWinPrevAppInst()			// get Windows prev app handle
{
    return prevAppInst;
}

int qWinAppCmdShow()				// get main window show command
{
    return appCmdShow;
}


// --------------------------------------------------------------------------
// Safe configuration (move,resize,changeGeometry) mechanism to avoid
// recursion when processing messages.
//

#include "qqueue.h"

struct QWinConfigRequest {
    WId	 id;					// widget to be configured
    int	 req;					// 0=move, 1=resize, 2=changeG
    int	 x, y, w, h;				// request parameters
};

declare(QQueueM,QWinConfigRequest);
static QQueue(QWinConfigRequest) *configRequests = 0;

void qWinRequestConfig( WId id, int req, int x, int y, int w, int h )
{
    if ( !configRequests )			// create queue
	configRequests = new QQueue(QWinConfigRequest);
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
	    if ( w->testFlag( WWin_Config ) )	// biting our tail
		return;
	    if ( r->req == 0 )
		w->move( r->x, r->y );
	    else
	    if ( r->req == 1 )
		w->resize( r->w, r->h );
	    else
		w->changeGeometry( r->x, r->y, r->w, r->h );
	}
	delete r;
    }
    delete configRequests;
    configRequests = 0;
}


// --------------------------------------------------------------------------
// Main event loop
//

int QApplication::exec( QWidget *mainWidget )	// main event loop
{
    MSG msg;
    main_widget = mainWidget;			// set main widget
    while ( TRUE ) {				// message loop
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
#if defined(USE_HEARTBEAT)
	if ( msg.message == WM_TIMER ) {	// timer message received
	    if ( msg.wParam != heartBeat )
		activateTimer( msg.wParam );
	    else
	    if ( curWin && qApp ) {		// process heartbeat
		POINT p;
		GetCursorPos( &p );
		if ( WindowFromPoint( p ) != curWin ) {
		    QEvent leave( Event_Leave );
		    SEND_EVENT( curWidget, &leave );
		    curWin = 0;
		}
	    }
	    continue;
	}
#else
	if ( msg.message == WM_TIMER ) {	// timer message received
	    activateTimer( msg.wParam );
	    continue;
	}
#endif
	if ( msg.message == WM_KEYDOWN || msg.message == WM_KEYUP ) {
	    if ( translateKeyCode(msg.wParam) == 0 ) {
		TranslateMessage( &msg );	// translate to WM_CHAR
		continue;
	    }
	}
	DispatchMessage( &msg );		// send to WndProc
	if ( configRequests )			// any pending configs?
	    qWinProcessConfigRequests();
	if ( quit_now )				// request to quit application
	    break;
    }
    return quit_code;
}


// --------------------------------------------------------------------------
// WndProc receives all messages from the main event loop
//

#if defined(_WS_WIN32_)
#define __export
#endif

extern "C" LRESULT CALLBACK __export
WndProc( HWND hwnd, UINT message, WORD wParam, LONG lParam )
{
    if ( !qApp )				// unstable app state
	return DefWindowProc(hwnd,message,wParam,lParam);

    QETWidget *widget = (QETWidget*)QWidget::find( hwnd );
    if ( !widget )				// don't know this widget
	return DefWindowProc(hwnd,message,wParam,lParam);

    MSG msg;
    msg.hwnd = hwnd;				// create MSG structure
    msg.message = message;			// time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;

    int evt_type = Event_None;
    bool result = TRUE;

    if ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST )
	widget->translateMouseEvent( msg );	// mouse event
    else
    switch ( message ) {

	case WM_KEYDOWN:			// keyboard event
	case WM_KEYUP:
	case WM_CHAR:
	    result = widget->translateKeyEvent( msg );
	    break;

	case WM_PAINT:				// paint event
	    result = widget->translatePaintEvent( msg );
	    break;

#if defined(USE_HEARTBEAT)
	case WM_NCHITTEST: {			// non-client mouse event
	    LRESULT r = DefWindowProc(hwnd,message,wParam,lParam);
	    if ( r != HTCLIENT && hwnd == curWin ) {
		QEvent leave( Event_Leave );
		SEND_EVENT( widget, &leave );
		curWin = 0;
	    }
	    return r;
	    }
#endif

#if !defined(STUPID_WINDOWS_NT)
	case WM_ERASEBKGND: {			// erase window background
	    HDC hdc = (HDC)wParam;
	    RECT rect;
	    HBRUSH hbr;
	    GetClientRect( hwnd, &rect );
	    hbr = CreateSolidBrush( widget->backgroundColor().pixel() );
	    FillRect( hdc, &rect, hbr );
	    DeleteObject( hbr );
	    }
	    break;
#endif

	case WM_MOVE:				// move window
	case WM_SIZE:				// resize window
	    result = widget->translateConfigEvent( msg );
	    break;

#if defined(TEST_WINDOWS_PALETTE)
	case WM_PALETTECHANGED:			// our window changed palette
	    result = TRUE;

	case WM_QUERYNEWPALETTE:		// realize own palette
	    return QColor::realizePal( widget );
#endif
	case WM_CLOSE:				// close window
	    if ( widget->translateCloseEvent( msg ) )
		delete widget;
	    return 0;				// always handled
	    break;

	case WM_DESTROY:			// destroy window
	    if ( hwnd == curWin ) {
		QEvent leave( Event_Leave );
		SEND_EVENT( widget, &leave );
		curWin = 0;
	    }
	    result = FALSE;
	    break;

	default:
	    result = FALSE;			// event was not processed
	    break;

    }

    if ( evt_type != Event_None ) {		// simple event
	QEvent evt( evt_type );
	result = SEND_EVENT(widget, &evt);
    }
    return result ? 0 : DefWindowProc(hwnd,message,wParam,lParam);
}


bool QApplication::winEventFilter( MSG * )	// Windows event filter
{
    return FALSE;
}


// --------------------------------------------------------------------------
// Timer handling; Our routines depend on Windows timer functions, but we
// need some extra handling to activate objects at timeout.
// We also keep an internal countdown variable to have longer timeouts.
// Max timeout is around 25 days.  Windows is limited to max 65 seconds.
//
// Implementation note: There are two types of timer identifiers. Windows
// timer ids (internal use) are stored in TimerInfo.  Qt timer ids are
// indexes (+1) into the timerVec vector.
//
// NOTE: These functions are for internal use. QObject::startTimer() and
//	 QObject::killTimer() are for public use.
//	 The QTimer class provides a high-level interface which translates
//	 timer events into signals.
//
// qStartTimer( interval, obj )
//	Starts a timer which will run until it is killed with qKillTimer()
//	Arguments:
//	    long interval	timer interval in milliseconds
//	    QObject *obj	where to send the timer event
//	Returns:
//	    int			timer identifier, or zero if not successful
//
// qKillTimer( timerId )
//	Stops a timer specified by a timer identifier.
//	Arguments:
//	    int timerId		timer identifier
//	Returns:
//	    bool		TRUE if successful
//
// qKillTimer( obj )
//	Stops all timers that are sent to the specified object.
//	Arguments:
//	    QObject *obj	object receiving timer events
//	Returns:
//	    bool		TRUE if successful
//

// --------------------------------------------------------------------------
// Internal data structure for timers
//

#include "qvector.h"
#include "qintdict.h"

struct TimerInfo {				// internal timer info
    uint     ind;				// - Qt timer identifier - 1
    uint     id;				// - Windows timer identifier
    bool     zero;				// - zero timing
    uint     maxcount;				// - max count for countdown
    uint     countdown;				// - countdown variable
    QObject *obj;				// - object to receive events
};
typedef declare(QVectorM,TimerInfo) TimerVec;	// vector of TimerInfo structs
typedef declare(QIntDictM,TimerInfo) TimerDict; // fast dict of timers

static const MaxTimers = 64;			// max number of timers
static TimerVec *timerVec = 0;			// timer vector
static TimerDict *timerDict = 0;		// timer dict


// --------------------------------------------------------------------------
// Timer activation (called from the event loop)
//

static bool activateTimer( uint id )		// activate timer
{
    if ( !timerVec )				// should never happen
	return FALSE;
    register TimerInfo *t = timerDict->find( id );
    if ( !t )					// no such timer id
	return FALSE;
    if ( t->countdown )				// we're not ready yet
	t->countdown--;
    else {
	t->countdown = t->maxcount;		// start counting again
	QTimerEvent evt( t->ind + 1 );
	SEND_EVENT( t->obj, &evt );		// send event
    }
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
	QTimerEvent evt( t->ind + 1 );
	SEND_EVENT( t->obj, &evt );
    }

}


// --------------------------------------------------------------------------
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
    delete timerVec;
}


// --------------------------------------------------------------------------
// Main timer functions for starting and killing timers
//

int qStartTimer( long interval, QObject *obj )	// start timer
{
    register TimerInfo *t;
    if ( !timerVec )				// initialize timer data
	initTimers();
    int ind = timerVec->findRef( 0 ) + 1;	// get free timer
    if ( ind == 0 || !obj )			// cannot create timer
	return 0;
    t = new TimerInfo;				// create timer entry
    CHECK_PTR( t );
    t->ind = ind;
    if ( interval > 65535 ) {			// use long intervals
	t->maxcount = (uint)(interval >> 16);
	interval /= t->maxcount + 1;
    }
    else
	t->maxcount = 0;
    t->countdown = t->maxcount;
    t->obj = obj;
    t->zero = interval == 0;
    if ( t->zero ) {				// add zero timer
	t->id = (uint)50000 + ind;		// unique, high id
	numZeroTimers++;
    }
    else {
	t->id = SetTimer( 0, 0, (uint)interval, 0 );
	if ( t->id == 0 ) {
	    delete t;				// could not set timer
	    return 0;
	}
    }
    timerVec->insert( ind-1, t );		// store in timer vector
    timerDict->insert( t->id, t );		// store in dict
    return ind;					// return index in vector
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

bool qKillTimer( QObject *obj )			// kill timer for obj
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


// --------------------------------------------------------------------------
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
// NOTE:	if ( s & (Mod1Mask | Mod2Mask) )
//	bst |= AltButton;
    return bst;
}

bool QETWidget::translateMouseEvent( const MSG &msg )
{
    static bool capture = FALSE;
    int	   type;				// event parameters
    QPoint pos;
    int	   button;
    int	   state;

    pos.rx() = LOWORD(msg.lParam);		// get position
    pos.ry() = HIWORD(msg.lParam);
    for ( int i=0; mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3 ) ;
    if ( !mouseTbl[i] )
	return FALSE;
    type = mouseTbl[++i];			// event type
    button = mouseTbl[++i];			// which button
    state = translateButtonState( msg.wParam ); // button state
    if ( type == Event_MouseMove ) {
	SetCursor( cursor().handle() );
	if ( curWin != id() ) {			// new current window
	    if ( curWin ) {			// send leave event
		QEvent leave( Event_Leave );
		SEND_EVENT( curWidget, &leave );
	    }
	    curWin = id();
	    curWidget = this;
	    QEvent enter( Event_Enter );	// send enter event
	    SEND_EVENT( this, &enter );
	}
	if ( (state == 0 || !capture) && !testFlag(WEtc_MouMove) )
	    return TRUE;			// no button
    }
    int bs = state & (LeftButton | RightButton | MidButton);
    if ( (type == Event_MouseButtonPress ||
	  type == Event_MouseButtonDblClick) && bs == button ) {
	SetCapture( id() );
	capture = TRUE;
    }
    else
    if ( type == Event_MouseButtonRelease && bs == 0 ) {
	ReleaseCapture();
	capture = FALSE;
    }
    QMouseEvent evt( type, pos, button, state );
    return SEND_EVENT( this, &evt );		// send event
}


// --------------------------------------------------------------------------
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
    VK_NUMPAD0,		Key_0,			// keypad
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
    int code = 0;
    if ( (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') )
	return 0;				// wait for WM_CHAR instead
    else
    if ( key >= VK_F1 && key <= VK_F24 )	// function keys
	code = Key_F1 + (key - VK_F1);		// assumes contiguous codes!
    else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] && code == 0 ) {
	    if ( key == (int)KeyTbl[i] )
		code = KeyTbl[i+1];
	    else
		i += 2;
	}
    }
    return code;
}

bool QETWidget::translateKeyEvent( const MSG &msg )
{
    int type;
    int code = 0;
    int ascii = 0;
    int state = 0;

    if ( msg.message == WM_CHAR ) {		// translated keyboard code
	type = Event_KeyPress;
	code = ascii = msg.wParam;
    }
    else {
	code = translateKeyCode( msg.wParam );
	if ( code == 0 )
	    return FALSE;			// virtual key not found
	type = msg.message == WM_KEYDOWN ?
	       Event_KeyPress : Event_KeyRelease;
    }
    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= ControlButton;
    QKeyEvent evt( type, code, ascii, state );
    return SEND_EVENT( this, &evt );		// send event
}


// --------------------------------------------------------------------------
// Paint event translation
//

bool QETWidget::translatePaintEvent( const MSG &msg )
{
    PAINTSTRUCT ps;
    QPaintEvent evt( clientRect() );
    setFlag( WState_Paint );
    hdc = BeginPaint( id(), &ps );
#if defined(STUPID_WINDOWS_NT)
    RECT rect;
    HBRUSH hbr;
    GetClientRect( id(), &rect );
    hbr = CreateSolidBrush( backgroundColor().pixel() );
    FillRect( hdc, &rect, hbr );
    DeleteObject( hbr );
#endif
    bool res = SEND_EVENT( this, &evt );
    EndPaint( id(), &ps );
    hdc = 0;
    clearFlag( WState_Paint );
    return res;
}


// --------------------------------------------------------------------------
// Window move and resize (configure) events
//

bool QETWidget::translateConfigEvent( const MSG &msg )
{
    setFlag( WWin_Config );			// set config flag
    QRect r = clientGeometry();			// get widget geometry
    WORD a = LOWORD(msg.lParam);
    WORD b = HIWORD(msg.lParam);
    if ( msg.message == WM_SIZE ) {		// resize event
	QSize newSize( a, b );
	r.setSize( newSize );
	setRect( r );
	QResizeEvent evt( geometry().size() );
	SEND_EVENT( this, &evt );
	if ( testFlag(WType_Overlap) && isParentType() ) {
	    QView *v = (QView *)this;		// parent type, i.e. QView
	    if ( IsIconic(v->id()) && v->iconText() )
		SetWindowText( v->id(), v->iconText() );
	    else
		SetWindowText( v->id(), v->caption() );
	}
	else
	if ( !testFlag(WType_Overlap) )		// manual redraw
	    update();
    }
    else
    if ( msg.message == WM_MOVE ) {		// move event
	QPoint newPos( a, b );
	r.setTopLeft( newPos );
	setRect( r );
	QMoveEvent evt( geometry().topLeft() );
	SEND_EVENT( this, &evt );
    }
    clearFlag( WWin_Config );			// clear config flag
    return TRUE;
}


// --------------------------------------------------------------------------
// Close window event translation
//

bool QETWidget::translateCloseEvent( const MSG &msg )
{
    QEvent evt( Event_Close );
    if ( SEND_EVENT( this, &evt ) ) {		// close widget
	hide();
	if ( qApp->mainWidget() == this || nWidgets() == 1 )
	    qApp->quit();
	else
	    return TRUE;			// delete widget
    }
    return FALSE;
}


// --------------------------------------------------------------------------
// QWinInfo::initialize() - Gets window system specific attributes
//

bool QWinInfo::initialize()
{
    HDC gdc;					// global DC

    gdc = GetDC( 0 );
    if ( GetDeviceCaps(gdc, RASTERCAPS) & RC_PALETTE )
	nColors = GetDeviceCaps( gdc, SIZEPALETTE );
    else
	nColors = GetDeviceCaps( gdc, NUMCOLORS );

    dispType = ColorDisplay;			// well, could be LCD...

    int width  = GetSystemMetrics( SM_CXSCREEN );
    int height = GetSystemMetrics( SM_CYSCREEN );
    dispSize = QSize( width, height );

    width  = GetDeviceCaps( gdc, HORZSIZE );
    height = GetDeviceCaps( gdc, VERTSIZE );
    dispSizeMM = QSize( width, height );

    ReleaseDC( 0, gdc );
    return TRUE;

}

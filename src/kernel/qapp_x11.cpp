/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp_x11.cpp#5 $
**
** Implementation of X11 startup routines and event handling
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qevent.h"
#include "qwidget.h"
#include "qwininfo.h"
#include "qstring.h"
#include <stdlib.h>
#include <signal.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapp_x11.cpp#5 $";
#endif


// --------------------------------------------------------------------------
// Internal variables and functions
//

static char    *appName;			// application name
static Display *display;			// X11 display
static char    *display_name = 0;		// X11 display name
static int	screen;				// X11 screen number
static Window	root_win;			// X11 root window
static Atom	wm_delete_window;		// delete window protocol

typedef void  (*VFPTR)();
static VFPTR   *cleanupRvec = 0;		// vector of cleanup routines
static uint	cleanupRcount = 0;

static void	trapSignals( int signo );	// default signal handler
static int	trapIOErrors( Display * );	// default X11 IO error handler

static void	initTimers();
static void	cleanupTimers();
static timeval *waitTimer();
static bool	activateTimer();
static timeval	watchtime;			// watch if time is turned back

void		qResetColorAvailFlag();		// defined in qcolor.cpp


class QETWidget : public QWidget {		// event translator widget
public:
    bool translateMouseEvent( const XEvent * );
    bool translateKeyEvent( const XEvent * );
    bool translatePaintEvent( const XEvent * );
    bool translateConfigEvent( const XEvent * );
    bool translateCloseEvent( const XEvent * );
};


#if defined(_OS_SUN_)
#define SIG_HANDLER SIG_PF
#else
#define SIG_HANDLER __sighandler_t
#endif


// --------------------------------------------------------------------------
// main() - initializes X and calls user's startup function qMain()
//

int main( int argc, char **argv )
{
#if defined(TRACE_FS)
    startFSTrace();
#endif

  // Install default traps

    signal( SIGQUIT, (SIG_HANDLER)trapSignals );
    signal( SIGINT, (SIG_HANDLER)trapSignals );
    XSetIOErrorHandler( trapIOErrors );

  // Set application name

    char *p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

  // Get command line params

    if ( argc > 2 && strcmp(argv[1],"-display") == 0 ) {
	display_name = argv[2];
	argc -= 2;
	argv += 2;
    }

  // Connect to X server

    if ( ( display = XOpenDisplay(display_name) ) == 0 ) {
	fatal( "%s: cannot connect to X server %s", appName,
	       XDisplayName(display_name) );
	return 1;
    }

  // Get X parameters

    screen = DefaultScreen(display);
    root_win = RootWindow(display,screen);

  // Support protocols

    wm_delete_window = XInternAtom( display, "WM_DELETE_WINDOW", FALSE );

  // Misc. initialization

    QWinInfo::initialize();
    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    gettimeofday( &watchtime, 0 );

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

    XCloseDisplay( display );			// close X display, clean-up

#if defined(TRACE_FS)
    stopFSTrace();
#endif
    return returnCode;
}


static void trapSignals( int signo )		// default signal handler
{
    warning( "%s: Signal %d received", appName, signo );
    exit( 0 );
}

static int trapIOErrors( Display * )		// default X11 IO error handler
{
    fatal( "%s: Fatal IO error: client killed", appName );
    return 0;
}


void qAddCleanupRoutine( void (*p)() )		// add cleanup routine
{
    if ( !cleanupRvec )
	cleanupRvec = new VFPTR[16];		// plenty of room
    cleanupRvec[cleanupRcount++] = p;		// save pointer to routine
}


// --------------------------------------------------------------------------
// Global functions that important data
//

char *qAppName()				// get application name
{
    return appName;
}

Display *qXDisplay()				// get current X display
{
    return display;
}

int qXScreen()					// get current X screen
{
    return screen;
}

Window qXRootWin()				// get X root window
{
    return root_win;
}

Atom qXDelWinProtocol()				// get X delete win protocol
{
    return wm_delete_window;
}


// --------------------------------------------------------------------------
// Main event loop
//

int QApplication::exec( QWidget *mainWidget )	// main event loop
{
    fd_set in_fdset;				// used by select()
    int	   x_fd = XConnectionNumber( display ); // X network socket
    int	   fd_width = x_fd + 1;

    XEvent event;
    main_widget = mainWidget;			// set main widget

    while ( quit_now == FALSE ) {		// until qapp->quit() called

	while ( XPending(display) ) {		// also flushes output buffer

	    int evt_type = Event_None;

	    if ( quit_now )			// quit between events
		break;
	    XNextEvent( display, &event );	// get next event

	    if ( x11EventFilter( &event ) )	// send event through filter
		continue;

	    QETWidget *widget = (QETWidget*)QWidget::find( event.xany.window );
	    if ( !widget )			// don't know this window
		continue;

	    switch ( event.type ) {

		case ButtonPress:		// mouse event
		case ButtonRelease:
		case MotionNotify:
		    if ( !widget->isDisabled() )
			widget->translateMouseEvent( &event );
		    break;

		case KeyPress:			// keyboard event
		case KeyRelease:
		    if ( !widget->isDisabled() )
			widget->translateKeyEvent( &event );
		    break;

		case MappingNotify:		// keyboard mapping changed
		    XRefreshKeyboardMapping( &event.xmapping );
		    break;

		case Expose:			// paint event
		    widget->translatePaintEvent( &event );
		    break;

		case ConfigureNotify:		// window move/resize event
		    widget->translateConfigEvent( &event );
		    break;

		case FocusIn:			// focus change
		case FocusOut:
		    break;

		case EnterNotify:		// enter window
		    evt_type = Event_Enter;
		    break;

		case LeaveNotify:		// leave window
		    evt_type = Event_Leave;
		    break;

		case ClientMessage:		// client message
		    if ( (event.xclient.format == 32) &&
			 (event.xclient.data.l[0] == wm_delete_window) ) {
			if ( widget->translateCloseEvent( &event ) )
			    delete widget;
		    }
		    break;

		case CirculateRequest:		// change stacking order
		    break;

		case ReparentNotify:		// window manager reparents
		    if ( event.xreparent.parent != root_win ) {
			XWindowAttributes a1, a2;
			Window parent = event.xreparent.parent;
			XGetWindowAttributes( widget->dpy, widget->id(), &a1 );
			XGetWindowAttributes( widget->dpy, parent, &a2 );
			QRect *r = &widget->rect;
			XWindowAttributes *a;
			if ( a1.x == 0 && a1.y == 0 && (a2.x + a2.y > 0) )
			    a = &a2;		// typical for mwm, fvwm
			else
			    a = &a1;		// typical for twm, olwm
			a->x += a2.border_width;// a->x = parent frame width
			a->y += a2.border_width;// a->y = parent caption height
			widget->ncrect = QRect(QPoint(r->left() - a->x,
						      r->top() - a->y),
					       QPoint(r->right() + a->x,
						      r->bottom() + a->x) );
		    }
		    break;

		default:
		    widget->x11Event( &event );
		    break;
	    }

	    if ( evt_type != Event_None ) {	// simple event
		QEvent evt( evt_type );
		SEND_EVENT( widget, &evt );
	    }
	}

	if ( quit_now )				// break immediatly
	    break;

	FD_ZERO( &in_fdset );
	FD_SET( x_fd, &in_fdset );
	timeval *tm = waitTimer();		// wait for timer or X event
	select( fd_width, &in_fdset, NULL, NULL, tm );
	qResetColorAvailFlag();			// color approx. optimization
	activateTimer();			// activate timer(s)
    }

    return quit_code;
}


bool QApplication::x11EventFilter( XEvent * )	// X11 event filter
{
    return FALSE;
}


// --------------------------------------------------------------------------
// Modal windows; Since Xlib has little support for this we have to check
// that no other modeless application windows can be raised in front of
// the modal window.
//
// qXEnterModal()
//	Enters modal window mode and returns when the window is closed
//	Arguments:
//	    QWidget *widget	A modal widget
//	Returns:
//	    int			The return code from QWidget::close()

#include "qstack.h"

declare(QStackM,QWidget) *modalWidgets;		// stack of modal widgets

int qXEnterModal( QWidget *widget )
{
    if ( !modalWidgets ) {			// create modal widget stack
	modalWidgets = new QStack(QWidget);
	CHECK_PTR( modalWidgets );
    }
    modalWidgets->push( widget );
    return qApp->exec( 0 );
}


// --------------------------------------------------------------------------
// Timer handling; Xlib has no application timer support so we'll have to
// make our own from scratch.
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

#include "qlist.h"

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};
typedef declare(QListM,TimerInfo) TimerList;	// list of TimerInfo structs

static const MaxTimers = 256;			// max number of timers
static const TimerBitVecLen = MaxTimers/8+1;	// size of timer bit vector
static uchar *timerBitVec = 0;			// timer bit vector
static TimerList *timerList = 0;		// timer list


// --------------------------------------------------------------------------
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline timeval operator+( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000 ) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}

static inline timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}


// --------------------------------------------------------------------------
// Internal functions for manipulating timer data structures
// The timerBitVec array is used for keeping track of timer identifiers
//

static inline void setTimerBit( int id )	// set timer bit
{
    --id;
    timerBitVec[id/8] |= (1 << (id & 7));
}

static inline void clearTimerBit( int id )	// clear timer bit
{
    --id;
    timerBitVec[id/8] &= ~(1 << (id & 7));
}

static inline int testTimerBit( int id )	// test timer bit
{
    --id;
    return timerBitVec[id/8] & (1 << (id & 7));
}

static int allocTimerId()			// find avail timer identifier
{
    for ( int i=0; i<TimerBitVecLen; i++ ) {
	if ( timerBitVec[i] != 0xff ) {
	    int j = i*8 + 1;
	    while ( testTimerBit(j) )
		j++;
	    return j;
	}
    }
    return 0;
}

static void insertTimer( const TimerInfo *ti )	// insert timer info into list
{
    TimerInfo *t = timerList->first();
    int index = 0;
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
	t = timerList->next();
	index++;
    }
    timerList->insertAt( ti, index );		// inserts sorted
}

static inline void getTime( timeval &t )	// get time of day
{
    gettimeofday( &t, 0 );
#if defined(_OS_SUN_)
    while ( t.tv_usec >= 1000000 ) {		// correct if NTP daemon bug
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	}
	else {
	    t.tv_usec = 0;
	    break;
	}
    }
#endif
}

static void repairTimer( const timeval &time )	// repair broken timer
{
    if ( !timerList )				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}


// --------------------------------------------------------------------------
// Timer activation functions (called from the event loop)
//

static timeval *waitTimer()			// time to wait for next timer
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if ( timerList && timerList->count() ) {	// there are waiting timers
	getTime( currentTime );
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if ( currentTime < t->timeout )		// time to wait
	    tm = t->timeout - currentTime;
	else {
	    tm.tv_sec = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	return &tm;
    }
    return 0;					// no timers
}

static bool activateTimer()			// activate timer(s)
{
    if ( !timerList || !timerList->count() )	// no timers
	return FALSE;
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    register TimerInfo *t;
    while ( maxcount-- ) {			// avoid starvation
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if ( currentTime < t->timeout )		// no timer has expired
	    break;
	timerList->take();			// unlink from list
	t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	QTimerEvent evt( t->id );
	SEND_EVENT( t->obj, &evt );		// send event
    }
    return TRUE;
}


// --------------------------------------------------------------------------
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerBitVec = new uchar[TimerBitVecLen];
    CHECK_PTR( timerBitVec );
    memset( timerBitVec, 0, sizeof(uchar)*TimerBitVecLen );
    timerList = new TimerList;
    CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	delete timerBitVec;
    }
}


// --------------------------------------------------------------------------
// Main timer functions for starting and killing timers
//

int qStartTimer( long interval, QObject *obj )	// start timer
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 || id > MaxTimers || !obj )	// cannot create timer
	return 0;
    setTimerBit( id );				// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    t->id = id;
    t->interval.tv_sec = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    return id;
}

bool qKillTimer( int id )			// kill timer with id
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 || id > MaxTimers || !testTimerBit(id) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	clearTimerBit( id );			// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )			// kill timers for obj
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    clearTimerBit( t->id );
	    timerList->remove();
	    t = timerList->current();
	}
	else
	    t = timerList->next();
    }
    return TRUE;
}


// --------------------------------------------------------------------------
// Mouse event translation
//
// Xlib doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

#define ABS(x) ((x)>=0?(x):-(x))

int translateButtonState( int s )		// translate button state
{
    int bst = 0;
    if ( s & Button1Mask )
	bst |= LeftButton;
    if ( s & Button2Mask )
	bst |= MidButton;
    if ( s & Button3Mask )
	bst |= RightButton;
    if ( s & ShiftMask )
	bst |= ShiftButton;
    if ( s & ControlMask )
	bst |= ControlButton;
    if ( s & (Mod1Mask | Mod2Mask) )
	bst |= AltButton;
    return bst;
}

bool QETWidget::translateMouseEvent( const XEvent *event )
{
    static Window lastWindow = 0;		// keep state; get doubleclicks
    static uint lastButton;
    static Time lastTime;
    static int lastX, lastY;
    int	   type;				// event parameters
    QPoint pos;
    int	   button;
    int	   state;

    if ( event->type == MotionNotify ) {	// mouse move
	type = Event_MouseMove;
	pos.rx() = event->xmotion.x;
	pos.ry() = event->xmotion.y;
	button = 0;				// no button causes motion
	state = translateButtonState( event->xmotion.state );
    }
    else {					// button press or release
	pos.rx() = event->xbutton.x;
	pos.ry() = event->xbutton.y;
	switch ( event->xbutton.button ) {
	    case Button1: button = LeftButton;	break;
	    case Button2: button = MidButton;	break;
	    case Button3: button = RightButton; break;
	}
	state = translateButtonState( event->xbutton.state );
	if ( event->type == ButtonPress ) {	// mouse button pressed
	    if ( lastWindow == event->xbutton.window &&
		 lastButton == button &&
		 (int)event->xbutton.time - lastTime < 400 &&
		 ABS(event->xbutton.x - lastX) < 5 &&
		 ABS(event->xbutton.y - lastY) < 5 ) {
		type = Event_MouseButtonDblClick;
	    }
	    else
		type = Event_MouseButtonPress;
	    lastWindow = event->xbutton.window; // save state
	    lastButton = event->xbutton.button;
	    lastTime = event->xbutton.time;
	    lastX = event->xbutton.x;
	    lastY = event->xbutton.y;
	}
	else					// mouse button released
	    type = Event_MouseButtonRelease;
    }
    QMouseEvent evt( type, pos, button, state );
    return SEND_EVENT( this, &evt );
}


// --------------------------------------------------------------------------
// Keyboard event translation
//

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include "qkeycode.h"

static int KeyTbl[] = {				// keyboard mapping table
    XK_Escape,		Key_Escape,		// misc keys
    XK_Tab,		Key_Tab,
    XK_BackSpace,	Key_Backspace,
    XK_Return,		Key_Return,
    XK_Insert,		Key_Insert,
    XK_Delete,		Key_Delete,
    XK_Clear,		Key_Delete,
    XK_Pause,		Key_Pause,
    XK_Print,		Key_Print,
    0x1005FF60,		Key_SysReq,		// hardcoded Sun SysReq
    0x1007ff00,		Key_SysReq,		// hardcoded X386 SysReq
    XK_Home,		Key_Home,		// cursor movement
    XK_End,		Key_End,
    XK_Left,		Key_Left,
    XK_Up,		Key_Up,
    XK_Right,		Key_Right,
    XK_Down,		Key_Down,
    XK_Prior,		Key_Prior,
    XK_Next,		Key_Next,
    XK_Shift_L,		Key_Shift,		// modifiers
    XK_Shift_R,		Key_Shift,
    XK_Shift_Lock,	Key_Shift,
    XK_Control_L,	Key_Control,
    XK_Control_R,	Key_Control,
    XK_Meta_L,		Key_Meta,
    XK_Meta_R,		Key_Meta,
    XK_Alt_L,		Key_Alt,
    XK_Alt_R,		Key_Alt,
    XK_Caps_Lock,	Key_CapsLock,
    XK_Num_Lock,	Key_NumLock,
    XK_Scroll_Lock,	Key_ScrollLock,
    XK_KP_Space,	Key_Space,		// keypad
    XK_KP_Tab,		Key_Tab,
    XK_KP_Enter,	Key_Enter,
    XK_KP_Equal,	Key_Equal,
    XK_KP_Multiply,	Key_Asterisk,
    XK_KP_Add,		Key_Plus,
    XK_KP_Separator,	Key_Comma,
    XK_KP_Subtract,	Key_Minus,
    XK_KP_Decimal,	Key_Period,
    XK_KP_Divide,	Key_Slash,
    0,			0
};

bool QETWidget::translateKeyEvent( const XEvent *event )
{
    int	   type;
    int	   code = 0;
    char   ascii[16];
    int	   count;
    int	   state;
    KeySym key;

    type = (event->type == KeyPress) ? Event_KeyPress : Event_KeyRelease;

    count = XLookupString( &((XEvent*)event)->xkey, ascii, 16, &key, NULL );
    state = translateButtonState( event->xkey.state );

    if ( key >= XK_space && key <= XK_asciitilde ) { // 7 bit ASCII Latin-1
	code = Key_Space + ((int)key - XK_space);
	if ( key >= XK_a && key <= XK_z )	// virtual code is uppercase
	    code -= (XK_a - XK_A);
    }
    else
    if ( key >= XK_F1 && key <= XK_F24 )	// function keys
	code = Key_F1 + ((int)key - XK_F1);	// assumes contiguous codes!
    else
    if ( key >= XK_KP_0 && key <= XK_KP_9 )	// numeric keypad keys
	code = Key_0 + ((int)key - XK_KP_0);	// assumes contiguous codes!
    else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] && code==0 ) {
	    if ( key == KeyTbl[i] )
		code = KeyTbl[i+1];
	    else
		i += 2;
	}
	if ( code == Key_Tab && (state & ShiftButton) == ShiftButton ) {
	    code = Key_Backtab;
	    ascii[0] = 0;
	}
    }
#if defined(DEBUG)
    if ( code == 0 ) {				// cannot translate keysym
	debug( "translateKey: No translation for X keysym %s (0x%x)",
	       XKeysymToString(XLookupKeysym(&((XEvent*)event)->xkey,0)),
	       key );
	return FALSE;
    }
#endif
    QKeyEvent evt( type, code, count > 0 ? ascii[0] : 0, state );
    return SEND_EVENT( this, &evt );
}


// --------------------------------------------------------------------------
// Paint event translation
//
// When receiving many expose events, we compress them (union of all expose
// rectangles) into one event which is sent to the widget
//

bool QETWidget::translatePaintEvent( const XEvent *event )
{
    bool  firstTime = TRUE;
    QRect paintRect;
    int	  type = event->xany.type;
    XEvent *xevt = (XEvent*)event;

    while ( TRUE ) {
	QRect rect( QPoint(xevt->xexpose.x,xevt->xexpose.y),
		    QSize(xevt->xexpose.width,xevt->xexpose.height) );
	if ( firstTime ) {			// set rectangle
	    paintRect = rect;
	    firstTime = FALSE;
	}
	else					// make union rectangle
	    paintRect = paintRect.unite( rect );
	if ( !XCheckTypedWindowEvent( display, id(), type, xevt ) )
	    break;
	if ( qApp->x11EventFilter( xevt ) )	// send event through filter
	    break;
    }

    QPaintEvent evt( paintRect );
    setFlag( WState_Paint );
    bool res = SEND_EVENT( this, &evt );
    clearFlag( WState_Paint );
    return res;
}


// --------------------------------------------------------------------------
// Safe configuration (move,resize,changeGeometry) mechanism to avoid
// getting two events back when performing a geometry change from the
// program.
//

#include "qintdict.h"

declare(QIntDictM,int);
static QIntDictM(int) *configCount = 0;

void qXRequestConfig( const QWidget *widget )	// add user config request
{
    if ( !widget->isVisible() )			// no need if invisible
	return;
    if ( !configCount )	{			// create dict
	configCount = new QIntDictM(int);
	configCount->setAutoDelete( TRUE );
    }
    int *count = configCount->find( (long)widget );
    if ( count ) {				// early config request there
	(*count)++;
#if defined(CHECK_STATE)
	if ( *count > 64 )
	    fatal( "%s: Internal error.  Config event stack overflow!",
		   appName );
#endif
    }
    else {					// make new request
	count = new int;
	*count = 1;
	configCount->insert( (long)widget, count );
    }
}

bool qXPassConfigEvent( const QWidget *widget )	// pass config event to app?
{
    int *count = configCount ? configCount->find( (long)widget ) : 0;
    if ( count ) {
	(*count)--;
	if ( *count <= 0 )
	    configCount->remove( (long)widget );
	if ( configCount->count() == 0 ) {	// dict becomes empty
	    delete configCount;
	    configCount = 0;
	}
    }
    return count == 0;
}


// --------------------------------------------------------------------------
// ConfigureNotify (window move and resize) event translation
//
// The problem with ConfigureNotify is that one cannot trust x and y values
// in the xconfigure struct. Top level widgets are reparented by the window
// manager, and (x,y) is sometimes relative to the parent window, but not
// always!
//

bool QETWidget::translateConfigEvent( const XEvent *event )
{
    if ( !qXPassConfigEvent(this) )		// skip event
	return FALSE;
    QPoint newPos( event->xconfigure.x, event->xconfigure.y );
    QSize newSize( event->xconfigure.width, event->xconfigure.height );
    QRect r = clientGeometry();
    bool  moveOk = event->xconfigure.send_event;
    if ( newSize != clientSize() ) {		// size changed
	r.setSize( newSize );
	setRect( r );
	QResizeEvent evt( newSize );
	SEND_EVENT( this, &evt );		// send resize event
	if ( !parentWidget() ) {		// top level widget
	    int x, y;
	    Window child;
	    XTranslateCoordinates( display, id(), DefaultRootWindow(display),
				   newPos.x(), newPos.y(), &x, &y, &child );
	    newPos = QPoint(x,y) - newPos;	// get right position
	}
	moveOk = TRUE;
    }
    if ( newPos != r.topLeft() && moveOk ) {	// position changed
	r.setTopLeft( newPos );
	setRect( r );
	QMoveEvent evt( geometry().topLeft() );
	SEND_EVENT( this, &evt );		// send move event
    }
    return TRUE;
}


// --------------------------------------------------------------------------
// Close window event translation
//

bool QETWidget::translateCloseEvent( const XEvent * )
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
    int dispDepth = DisplayPlanes( display, screen );
    nColors = 1 << dispDepth;
    if ( dispDepth == 1 )			// two colors
	dispType = MonochromeDisplay;
    else {					// grayscale or color
	XVisualInfo vis_info;
	int i = DirectColor;
	while ( !XMatchVisualInfo(display,screen,dispDepth,i,&vis_info) )
	    i--;
	if ( i < StaticColor )
	    dispType = GrayscaleDisplay;
	else
	    dispType = ColorDisplay;
    }
    dispSize = QSize( DisplayWidth(display,screen),
		      DisplayHeight(display,screen) );
    dispSizeMM = QSize( DisplayWidthMM(display,screen),
			DisplayHeightMM(display,screen) );
    return TRUE;
}

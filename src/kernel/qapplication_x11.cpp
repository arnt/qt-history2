/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_x11.cpp#41 $
**
** Implementation of X11 startup routines and event handling
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qevent.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpainter.h"
#include <stdlib.h>
#include <signal.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
#define  CHECK_MEMORY
#include <qmemchk.h>
#endif

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapplication_x11.cpp#41 $";
#endif


// --------------------------------------------------------------------------
// Internal variables and functions
//

static char    *appName;			// application name
static char    *appFont = 0;			// application font
static char    *appBGCol = 0;			// application bg color
static char    *appFGCol = 0;			// application fg color
static char    *tlwGeometry = 0;		// top level widget geometry
static char    *tlwTitle = 0;			// top level widget title
static bool     tlwIconic = FALSE;		// top level widget iconified
static int	appArgc;			// argument count
static char   **appArgv;			// argument vector
static Display *appDpy;				// X11 application display
static char    *appDpyName = 0;			// X11 display name
static bool	appSync = FALSE;		// X11 synchronization
static int	appScreen;			// X11 screen number
static Window	appRootWin;			// X11 root window
static GC	app_gc1 = 0;			// read-only GC
static GC	app_gc2 = 0;			// temporary GC
static QWidget *desktopWidget = 0;		// root window widget
Atom		q_wm_delete_window;		// delete window protocol
#if defined(DEBUG)
static bool	appMemChk = FALSE;		// memory checking (debugging)
#endif

static Window	mouseActWindow = 0;		// window where mouse is
static int	mouseButtonPressed = 0;		// last mouse button pressed
static int	mouseButtonState = 0;		// mouse button state
static Time	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse position in act window

typedef void  (*VFPTR)();
typedef void  (*VFPTR_ARG)( int, char ** );
typedef declare(QListM,void) QVFuncList;
static QVFuncList *preRList = 0;		// list of pre routines
static QVFuncList *postRList = 0;		// list of post routines

static void	trapSignals( int signo );	// default signal handler
static int	trapIOErrors( Display * );	// default X11 IO error handler

static void	cleanupPostedEvents();

static void	initTimers();
static void	cleanupTimers();
static timeval *waitTimer();
static bool	activateTimer();
static timeval	watchtime;			// watch if time is turned back

void		qResetColorAvailFlag();		// defined in qcolor.cpp


class QETWidget : public QWidget {		// event translator widget
public:
    void setFlag( WFlags n )		{ QWidget::setFlag(n); }
    void clearFlag( WFlags n )		{ QWidget::clearFlag(n); }
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
    int i;
#if defined(DEBUG)
    int mcBufSize   = 100000;			// default memchk settings
    char *mcLogFile = "MEMCHK.LOG";
    for ( i=1; i<argc; i++ ) {			// look for -memchk argument
	if ( *argv[i] != '-' )
	    break;
	if ( strcmp(argv[i],"-memchk") == 0 )
	    appMemChk = !appMemChk;
	else if ( strcmp(argv[i],"-membuf") == 0 ) {
	    if ( ++i < argc ) mcBufSize = atoi(argv[i]);
	}
	else if ( strcmp(argv[i],"-memlog") == 0 ) {
	    if ( ++i < argc ) mcLogFile = argv[i];
	}
    }
    if ( appMemChk ) {				// start memory checking
	memchkSetBufSize( mcBufSize );
	memchkSetLogFile( mcLogFile );
	memchkStart();
    }
#endif

    appArgc = argc;				// save arguments
    appArgv = argv;

    if ( preRList ) {
	VFPTR_ARG f = (VFPTR_ARG)preRList->first();
	while ( f ) {				// call pre routines
	    (*f)( argc, argv );
	    preRList->remove();
	    f = (VFPTR_ARG)preRList->first();
	}
    }

  // Install default traps

    signal( SIGQUIT, (SIG_HANDLER)trapSignals );
    signal( SIGINT, (SIG_HANDLER)trapSignals );
    XSetIOErrorHandler( trapIOErrors );

  // Set application name

    char *p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

  // Get command line params

    for ( i=1; i<argc; i++ ) {
	QString arg = argv[i];
	if ( arg[0] != '-' )
	    break;
	else if ( arg == "-display" ) {
	    if ( ++i < argc ) appDpyName = argv[i];
	}
	else if ( arg == "-fn" || arg == "-font" ) {
	    if ( ++i < argc ) appFont = argv[i];
	}
	else if ( arg == "-bg" || arg == "-background" ) {
	    if ( ++i < argc ) appBGCol = argv[i];
	}
	else if ( arg == "-fg" || arg == "-foreground" ) {
	    if ( ++i < argc ) appFGCol = argv[i];
	}
	else if ( arg == "-name" ) {
	    if ( ++i < argc ) appName = argv[i];
	}
	else if ( arg == "-title" ) {
	    if ( ++i < argc ) tlwTitle = argv[i];
	}
	else if ( arg == "-geometry" ) {
	    if ( ++i < argc ) tlwGeometry = argv[i];
	}
	else if ( arg == "-iconic" )
	    tlwIconic = !tlwIconic;
	else if ( arg == "-sync" )
	    appSync = !appSync;
	else
	    break;
    }
    if ( i > 1 ) {				// shift arguments
	argc -= --i;
	argv[i] = argv[0];
	argv = &argv[i];
    }

  // Connect to X server

    if ( ( appDpy = XOpenDisplay(appDpyName) ) == 0 ) {
	fatal( "%s: cannot connect to X server %s", appName,
	       XDisplayName(appDpyName) );
	return 1;
    }

    if ( appSync )				// if "-sync" argument
	XSynchronize( appDpy, TRUE );

  // Get X parameters

    appScreen = DefaultScreen(appDpy);
    appRootWin = RootWindow(appDpy,appScreen);

  // Support protocols

    q_wm_delete_window = XInternAtom( appDpy, "WM_DELETE_WINDOW", FALSE );

  // Misc. initialization

    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
    gettimeofday( &watchtime, 0 );

  // Call user-provided main routine

    int returnCode = qMain( argc, argv );

  // Cleanup

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
    delete preRList;
    if ( qApp )
	delete qApp;
    QApplication::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
    cleanupTimers();
    QPainter::cleanup();

    if ( app_gc1 )
	XFreeGC( appDpy, app_gc1 );
    if ( app_gc2 )
	XFreeGC( appDpy, app_gc2 );

    XCloseDisplay( appDpy );			// close X display

#if defined(DEBUG)
    if ( appMemChk )
	memchkStop();				// finish memory checking
#endif
    return returnCode;
}


static void trapSignals( int signo )		// default signal handler
{
#if defined(DEBUG)
    if ( appMemChk )
	memchkSetReporting( FALSE );		// ignore error messages
#endif
    warning( "%s: Signal %d received", appName, signo );
    exit( 0 );
}

static int trapIOErrors( Display * )		// default X11 IO error handler
{
    fatal( "%s: Fatal IO error: client killed", appName );
    return 0;
}


void qAddPreRoutine( void (*p)() )		// add pre routine
{
    if ( !preRList ) {
	preRList = new QVFuncList;
	CHECK_PTR( preRList );
    }
    preRList->append( (void *)p );		// store at list tail
}

void qAddPostRoutine( void (*p)() )		// add post routine
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	CHECK_PTR( postRList );
    }
    postRList->insert( (void *)p );		// store at list head
}


// --------------------------------------------------------------------------
// Some implementation-specific functions
//

char *qAppName()				// get application name
{
    return appName;
}

Display *qXDisplay()				// get current X display
{
    return appDpy;
}

int qXScreen()					// get current X screen
{
    return appScreen;
}

Window qXRootWin()				// get X root window
{
    return appRootWin;
}

GC qXGetReadOnlyGC()				// get read-only GC
{
    if ( !app_gc1 )
	app_gc1 = XCreateGC( appDpy, appRootWin, 0, 0 );
    return app_gc1;
}

GC qXGetTempGC()				// get use'n throw GC
{
    if ( !app_gc2 )
	app_gc2 = XCreateGC( appDpy, appRootWin, 0, 0 );
    return app_gc2;
}

QWidget *QApplication::desktop()
{
    if ( !desktopWidget ) {			// not created yet
	desktopWidget = new QWidget( 0, "desktop", WType_Desktop );
	CHECK_PTR( desktopWidget );
    }
    return desktopWidget;
}


void QApplication::setCursor( const QCursor &c )// set application cursor
{
    if ( appCursor )
	delete appCursor;
    appCursor = new QCursor( c );
    CHECK_PTR( appCursor );
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testFlag(WCursorSet) )		//   set a cursor
	    XDefineCursor( w->display(), w->id(), appCursor->handle() );
	++it;
    }
    XFlush( appDpy );				// make X execute it NOW
}

void QApplication::restoreCursor()		// restore application cursor
{
    if ( !appCursor )				// there is no app cursor
	return;
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// set back to original cursors
	if ( w->testFlag(WCursorSet) )
	    XDefineCursor( w->display(), w->id(), w->cursor().handle() );
	++it;
    }
    XFlush( appDpy );
    delete appCursor;				// reset appCursor
    appCursor = 0;
}


void QApplication::setFont( const QFont &f,  bool forceAllWidgets )
{						// set application font
    if ( appFont )
	delete appFont;
    appFont = new QFont( f );
    CHECK_PTR( appFont );
    QFont::setDefaultFont( *appFont );
    if ( forceAllWidgets ) {			// set for all widgets now
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    w->setFont( *appFont );
	    ++it;
	}
    }
}


void QApplication::flushX()			// flush X output buffer
{
    XFlush( appDpy );
}

void QApplication::syncX()			// synchronize with X server
{
    XSync( appDpy, FALSE );			// don't discard events
}


// --------------------------------------------------------------------------
// QApplication management of posted events
//

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

declare(QListM,QPostEvent);
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


// --------------------------------------------------------------------------
// Special lookup functions for windows that have been recreated recently
//

static QWidgetIntDict *wPRmapper = 0;		// alternative widget mapper

void qPRCreate( const QWidget *widget, Window oldwin )
{						// QWidget::recreate mechanism
    if ( !wPRmapper ) {
	wPRmapper = new QWidgetIntDict;
	CHECK_PTR( wPRmapper );
    }
    wPRmapper->insert( (long)oldwin, widget );	// add old window to mapper
    QETWidget *w = (QETWidget *)widget;
    w->setFlag( WRecreated );			// set recreated flag
}

void qPRCleanup( QETWidget *widget )
{
    if ( !(wPRmapper && widget->testFlag(WRecreated)) )
	return;					// not a recreated widget
    QWidgetIntDictIt it(*wPRmapper);
    QWidget *w;
    while ( (w=it.current()) ) {
	if ( w == widget ) {			// found widget
	    widget->clearFlag( WRecreated );	// clear recreated flag
	    wPRmapper->remove( it.currentKey());// old window no longer needed
	    if ( wPRmapper->count() == 0 ) {	// became empty
		delete wPRmapper;		// then reset alt mapper
		wPRmapper = 0;
	    }
	    return;
	}
	++it;
    }
}

QETWidget *qPRFindWidget( Window oldwin )
{
    return wPRmapper ? (QETWidget*)wPRmapper->find((long)oldwin) : 0;
}


// --------------------------------------------------------------------------
// Main event loop
//

int QApplication::exec( QWidget *mainWidget )	// main event loop
{
    fd_set in_fdset;				// used by select()
    int	   x_fd = XConnectionNumber( appDpy );	// X network socket
    int	   fd_width = x_fd + 1;

    XEvent event;
    main_widget = mainWidget;			// set main widget
    if ( main_widget )				// give WM command line
	XSetWMProperties( main_widget->display(), main_widget->id(),
			  0, 0, appArgv, appArgc, 0, 0, 0 );

    while ( quit_now == FALSE ) {		// until qapp->quit() called

	if ( postedEvents && postedEvents->count() )
	    sendPostedEvents();

	while ( XPending(appDpy) ) {		// also flushes output buffer

	    if ( quit_now )			// quit between events
		break;
	    XNextEvent( appDpy, &event );	// get next event

	    if ( x11EventFilter( &event ) )	// send event through filter
		continue;

	    QETWidget *widget = (QETWidget*)QWidget::find( event.xany.window );

	    if ( wPRmapper ) {			// just did a widget recreate?
		if ( widget == 0 ) {		// not in std widget mapper
		    switch ( event.type ) {	// only for mouse/key events
			case ButtonPress:
			case ButtonRelease:
			case MotionNotify:
			case KeyPress:
			case KeyRelease:
			    widget = qPRFindWidget( event.xany.window );
			    break;
		    }
		}
		else if ( widget->testFlag(WRecreated) )
		    qPRCleanup( widget );	// remove from alt mapper
	    }
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

		case GraphicsExpose:
		case Expose:			// paint event
		    widget->translatePaintEvent( &event );
		    break;

		case ConfigureNotify:		// window move/resize event
		    widget->translateConfigEvent( &event );
		    break;

		case FocusIn: {			// got focus
		    QFocusEvent focusEvent( Event_FocusIn );
		    QApplication::sendEvent( widget, &focusEvent );
		    }
		    break;

		case FocusOut: {		// lost focus
		    QFocusEvent focusEvent( Event_FocusOut );
		    QApplication::sendEvent( widget, &focusEvent );
		    }
		    break;

		case EnterNotify:		// enter window
		    break;			// ignored

		case LeaveNotify:		// leave window
		    break;			// ignored

		case UnmapNotify:		// window hidden
		    widget->clearFlag( WState_Visible );
		    break;

		case MapNotify:			// window shown
		    widget->setFlag( WState_Visible );
		    break;

		case ClientMessage:		// client message
		    if ( (event.xclient.format == 32) ) {
			long *l = event.xclient.data.l;
			if ( *l == q_wm_delete_window ) {
			    if ( widget->translateCloseEvent( &event ) )
				delete widget;
			}
		    }
		    break;

		case CirculateRequest:		// change stacking order
		    break;

		case ReparentNotify:		// window manager reparents
		    if ( event.xreparent.parent != appRootWin ) {
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
// Popup widget mechanism
//
// qXOpenPopup()
//	Adds a widget to the list of popup widgets
//	Arguments:
//	    QWidget *widget	The popup widget to be added
//
// qXClosePopup()
//	Removes a widget from the list of popup widgets
//	Arguments:
//	    QWidget *widget	The popup widget to be removed
//

QWidgetList *popupWidgets = 0;			// list of popup widgets
bool popupCloseDownMode = FALSE;

void qXOpenPopup( QWidget *popup )		// add popup widget
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 ) {		// grab mouse
	XGrabKeyboard( popup->display(), popup->id(), TRUE,
		       GrabModeSync, GrabModeSync, CurrentTime );
	XAllowEvents( popup->display(), SyncKeyboard, CurrentTime );
	XGrabPointer( popup->display(), popup->id(), TRUE,
		      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		      EnterWindowMask | LeaveWindowMask,
		      GrabModeSync, GrabModeAsync,
		      None, None, CurrentTime );
	XAllowEvents( popup->display(), SyncPointer, CurrentTime );
    }
}

void qXClosePopup( QWidget *popup )		// remove popup widget
{
    if ( !popupWidgets )
	return;
    if ( popupWidgets->findRef(popup) != -1 )
	popupWidgets->remove();
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	XUngrabKeyboard( popup->display(), CurrentTime );
	XAllowEvents( popup->display(), ReplayPointer, CurrentTime );
    }
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
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
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
    timerList->insert( index, ti );		// inserts sorted
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
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
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
    CHECK_PTR( t );
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
    static bool buttonDown = FALSE;
    static bool manualGrab = FALSE;
    int	   type;				// event parameters
    QPoint pos;
    int	   button = 0;
    int	   state;

    if ( event->type == MotionNotify ) {	// mouse move
	XEvent *xevent = (XEvent *)event;
	while ( XCheckTypedWindowEvent(display(), id(), MotionNotify, xevent) )
	    ;					// compress motion events
	type = Event_MouseMove;
	pos.rx() = xevent->xmotion.x;
	pos.ry() = xevent->xmotion.y;
	state = translateButtonState( xevent->xmotion.state );
	if ( !buttonDown ) {
	    state &= ~(LeftButton|MidButton|RightButton);
	    if ( !testFlag(WMouseTracking) )
		type = 0;			// don't send event
	}
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
	    buttonDown = TRUE;
	    if ( mouseActWindow == event->xbutton.window &&
		 mouseButtonPressed == button &&
		 (int)event->xbutton.time - mouseButtonPressTime < 400 &&
		 ABS(event->xbutton.x - mouseXPos) < 5 &&
		 ABS(event->xbutton.y - mouseYPos) < 5 ) {
		type = Event_MouseButtonDblClick;
	    }
	    else
		type = Event_MouseButtonPress;
	    mouseButtonPressTime = event->xbutton.time;
	}
	else {					// mouse button released
	    if ( manualGrab ) {			// release manual grab
		manualGrab = FALSE;
		XUngrabPointer( display(), CurrentTime );
	    }
	    if ( !buttonDown )			// unexpected event
		return FALSE;
	    type = Event_MouseButtonRelease;
	    if ( (state & (LeftButton|MidButton|RightButton)) == 0 )
		buttonDown = FALSE;	    
	}
    }
    mouseActWindow = id();			// save mouse event params
    mouseButtonPressed = button;
    mouseButtonState = state;
    mouseXPos = pos.x();
    mouseYPos = pos.y();
    if ( type == 0 )				// don't send event
	return FALSE;
    if ( popupWidgets ) {			// oops, in popup mode
	QWidget *popup = popupWidgets->last();
	if ( popup != this ) {
	    if ( testFlag(WType_Popup) && clientRect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( mapToGlobal(pos) );
	}
	QMouseEvent e( type, pos, button, state );
	QApplication::sendEvent( popup, &e );
	if ( popupWidgets )			// still in popup mode
	    XAllowEvents( display(), SyncPointer, CurrentTime );
	else {					// left popup mode
	    if ( type != Event_MouseButtonRelease && state != 0 ) {
		manualGrab = TRUE;		// need to manually grab
		XGrabPointer( display(), mouseActWindow, FALSE,
			      ButtonPressMask | ButtonReleaseMask |
			      ButtonMotionMask |
			      EnterWindowMask | LeaveWindowMask,
			      GrabModeAsync, GrabModeAsync,
			      None, None, CurrentTime );
	    }
	}
    }
    else {
	QMouseEvent e( type, pos, button, state );
	if ( popupCloseDownMode ) {
	    popupCloseDownMode = FALSE;
	    if ( testFlag(WType_Popup) )	// ignore replayed event
		return TRUE;
	}
	QApplication::sendEvent( this, &e );
    }
    return TRUE;
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
    QKeyEvent e( type, code, count > 0 ? ascii[0] : 0, state );
    if ( popupWidgets ) {			// oops, in popup mode
	QWidget *popup = popupWidgets->last();
	QApplication::sendEvent( popup, &e );	// send event to popup instead
	if ( popupWidgets )			// still in popup mode
	    XAllowEvents( display(), SyncKeyboard, CurrentTime );
	return TRUE;
    }
    return QApplication::sendEvent( this, &e );
}


// --------------------------------------------------------------------------
// Paint event translation
//
// When receiving many expose events, we compress them (union of all expose
// rectangles) into one event which is sent to the widget.
//

bool QETWidget::translatePaintEvent( const XEvent *event )
{
    bool  firstTime = TRUE;
    QRect paintRect;
    int	  type = event->xany.type;
    XEvent *xevent = (XEvent *)event;

    while ( TRUE ) {
	QRect rect( QPoint(xevent->xexpose.x,xevent->xexpose.y),
		    QSize(xevent->xexpose.width,xevent->xexpose.height) );
	if ( firstTime ) {			// set rectangle
	    paintRect = rect;
	    firstTime = FALSE;
	}
	else					// make union rectangle
	    paintRect = paintRect.unite( rect );
	if ( !XCheckTypedWindowEvent( display(), id(), type, xevent ) )
	    break;
	if ( qApp->x11EventFilter( xevent ) )	// send event through filter
	    break;
    }

    QPaintEvent e( paintRect );
    setFlag( WState_Paint );
    QApplication::sendEvent( this, &e );
    clearFlag( WState_Paint );
    return TRUE;
}


// --------------------------------------------------------------------------
// ConfigureNotify (window move and resize) event translation
//
// The problem with ConfigureNotify is that one cannot trust x and y values
// in the xconfigure struct. Top level widgets are reparented by the window
// manager, and (x,y) is sometimes relative to the parent window, but not
// always!  It is safer (but slower) to fetch the window attributes.
//

bool QETWidget::translateConfigEvent( const XEvent *event )
{
    if ( !parentWidget() ) {			// top level widget
	XWindowAttributes a;
	XGetWindowAttributes( display(), id(), &a );
	QSize  newSize( a.width, a.height );
	QPoint newPos( a.x, a.y );
	QRect r = clientGeometry();
	if ( newSize != clientSize() ) {	// size changed
	    r.setSize( newSize );
	    setRect( r );
	    QResizeEvent e( newSize );
	    QApplication::sendEvent( this, &e );
	}
	if ( newPos != geometry().topLeft() ) {	// position changed
	    int x, y;
	    Window child;
	    XTranslateCoordinates( display(), id(),
				   DefaultRootWindow(display()),
				   newPos.x(), newPos.y(), &x, &y, &child );
	    newPos = QPoint(x,y) - newPos;	// get position excl frame
	    r.setTopLeft( newPos );
	    setRect( r );
	    QMoveEvent e( geometry().topLeft() );
	    QApplication::sendEvent( this, &e );
	}
    }
}


// --------------------------------------------------------------------------
// Close window event translation
//

bool QETWidget::translateCloseEvent( const XEvent * )
{
    QEvent e( Event_Close );
    if ( QApplication::sendEvent( this, &e ) ) {// close widget
	hide();
	if ( qApp->mainWidget() == this )
	    qApp->quit();
	else
	    return TRUE;			// delete widget
    }
    return FALSE;
}

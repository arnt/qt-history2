/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_x11.cpp#129 $
**
** Implementation of X11 startup routines and event handling
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qwidget.h"
#include "qobjcoll.h"
#include "qwidcoll.h"
#include "qpainter.h"
#include "qpmcache.h"
#define gettimeofday	__hide_gettimeofday
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#undef gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );

#if defined(_OS_LINUX_) && defined(DEBUG)
#include <qfile.h>
#include <qstring.h>
#include <unistd.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qapplication_x11.cpp#129 $")


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static char    *appName;			// application name
static char    *appFont		= 0;		// application font
static char    *appBGCol	= 0;		// application bg color
static char    *appFGCol	= 0;		// application fg color
static char    *mwGeometry	= 0;		// main widget geometry
static char    *mwTitle		= 0;		// main widget title
static bool	mwIconic	= FALSE;	// main widget iconified
static Display *appDpy		= 0;		// X11 application display
static char    *appDpyName	= 0;		// X11 display name
static bool	appSync		= FALSE;	// X11 synchronization
#if defined(DEBUG)
static bool	appNoGrab	= FALSE;	// X11 grabbing enabled
static bool	appDoGrab	= FALSE;	// X11 grabbing override (gdb)
#endif
static int	appScreen;			// X11 screen number
static Window	appRootWin;			// X11 root window
static bool	app_save_rootinfo = FALSE;	// save root info

static bool	app_do_modal	= FALSE;	// modal mode
static int	app_loop_level	= 1;		// event loop level
static bool	app_exit_loop	= FALSE;	// flag to exit local loop
static int	app_Xfd;			// X network socket
static fd_set   app_readfds;			// fd set for reading
static fd_set   app_writefds;			// fd set for writing
static fd_set   app_exceptfds;			// fd set for exceptions

static GC	app_gc_ro	= 0;		// read-only GC
static GC	app_gc_tmp	= 0;		// temporary GC
static GC	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC	app_gc_tmp_m	= 0;		// temporary GC (monochrome)
static QWidget *desktopWidget	= 0;		// root window widget
Atom		q_wm_delete_window;		// delete window protocol

static Window	mouseActWindow	     = 0;	// window where mouse is
static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static Time	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse position in act window

typedef void  (*VFPTR)();
typedef declare(QListM,void) QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

static void	trapSignals( int signo );	// default signal handler
static int	trapIOErrors( Display * );	// default X11 IO error handler

static void	cleanupPostedEvents();

static void	initTimers();
static void	cleanupTimers();
static timeval *waitTimer();
static bool	activateTimer();
static timeval	watchtime;			// watch if time is turned back

static void	qt_save_rootinfo();
static bool	qt_try_modal( QWidget *, XEvent * );
void		qt_reset_color_avail();		// defined in qcol_x11.cpp


class QETWidget : public QWidget		// event translator widget
{
public:
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
    bool translateMouseEvent( const XEvent * );
    bool translateKeyEvent( const XEvent * );
    bool translatePaintEvent( const XEvent * );
    bool translateConfigEvent( const XEvent * );
    bool translateCloseEvent( const XEvent * );
};


#if defined(_OS_SUN_)
#define SIG_HANDLER SIG_PF
#elif defined(_OS_HPUX_)
typedef void (*SIG_HANDLER)(int);
#else
#define SIG_HANDLER __sighandler_t
#endif


//
// qt_init() - initializes Qt for X-Windows
//

void qt_init( int *argcptr, char **argv )
{
    int argc = *argcptr;
    int i, j;

  // Install default traps

    signal( SIGQUIT, (SIG_HANDLER)trapSignals );
    signal( SIGINT, (SIG_HANDLER)trapSignals );
    XSetIOErrorHandler( trapIOErrors );

  // Set application name

    char *p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

  // Get command line params

    j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QString arg = argv[i];
	if ( arg == "-display" ) {
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
	    if ( ++i < argc ) mwTitle = argv[i];
	}
	else if ( arg == "-geometry" ) {
	    if ( ++i < argc ) mwGeometry = argv[i];
	}
	else if ( arg == "-iconic" )
	    mwIconic = !mwIconic;
	else if ( strncmp(arg,"-style=",7) == 0 ) {
	    QString s = &arg[7];
	    s = s.lower();
	    int style = -1;
	    if ( s == "mac" || s == "macintosh" )
		style = MacStyle;
	    else if ( s == "windows" )
		style = WindowsStyle;
	    else if ( s == "win3" || s == "windows3" )
		style = WindowsStyle;
	    else if ( s == "pm" )
		style = PMStyle;
	    else if ( s == "motif" )
		style = MotifStyle;
	    if ( style != -1 )
		QApplication::setStyle( (GUIStyle)style );
	}
#if defined(DEBUG)
	else if ( arg == "-sync" )
	    appSync = !appSync;
	else if ( arg == "-nograb" )
	    appNoGrab = !appNoGrab;
	else if ( arg == "-dograb" )
	    appDoGrab = !appDoGrab;
#endif
	else
	    argv[j++] = argv[i];
    }

    *argcptr = j;

#if defined(DEBUG) && defined(_OS_LINUX_)
    if ( !appNoGrab && !appDoGrab ) {
	QString s;
	s.sprintf( "/proc/%d/cmdline", getppid() );
	QFile f( s );
	if ( f.open( IO_ReadOnly ) ) {
	    s.truncate( 0 );
	    int c;
	    while ( (c = f.getch()) > 0 ) {
		if ( c == '/' )
		    s.truncate( 0 );
		else
		    s += (char)c;
	    }
	    if ( s == "gdb" ) {
		appNoGrab = TRUE;
		debug( "Qt: gdb: -nograb added to command-line options"
		       "\tUse the -dograb option to enforce grabbing" );
	    }
	    f.close();
	}
    }
#endif

  // Connect to X server

    if ( ( appDpy = XOpenDisplay(appDpyName) ) == 0 ) {
	fatal( "%s: cannot connect to X server %s", appName,
	       XDisplayName(appDpyName) );
    }
    app_Xfd = XConnectionNumber( appDpy );	// set X network socket

    if ( appSync )				// if "-sync" argument
	XSynchronize( appDpy, TRUE );

  // Get X parameters

    appScreen  = DefaultScreen(appDpy);
    appRootWin = RootWindow(appDpy,appScreen);

  // Support protocols

    q_wm_delete_window = XInternAtom( appDpy, "WM_DELETE_WINDOW", FALSE );

  // Misc. initialization

    app_loop_level = 0;
    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
    gettimeofday( &watchtime, 0 );

    qApp->setName( appName );
    if ( appFont ) {				// set application font
	QFont font;
	font.setRawMode( TRUE );
	font.setFamily( appFont );
	QApplication::setFont( font );
    }
    if ( appBGCol || appFGCol ) {		// set application colors
	QColor bg;
	QColor fg;
	if ( appBGCol )
	    bg = QColor(appBGCol);
	else
	    bg = lightGray;
	if ( appFGCol )
	    fg = QColor(appFGCol);
	else
	    fg = black;
	QColorGroup cg( fg, bg, bg.light(), bg.dark(), bg.dark(150), fg,
			white );
	QPalette pal( cg, cg, cg );
	QApplication::setPalette( pal );
    }
    setlocale( LC_ALL, "ISO-8859-1" );		// use correct char set mapping
}


//
// qt_cleanup() - cleans up when the application is finished
//

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

    if ( app_save_rootinfo )			// root window must keep state
	qt_save_rootinfo();
    QPixmapCache::clear();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
    cleanupTimers();
    QPainter::cleanup();

#define CLEANUP_GC(g) if (g) XFreeGC(appDpy,g)
    CLEANUP_GC(app_gc_ro);
    CLEANUP_GC(app_gc_ro_m);
    CLEANUP_GC(app_gc_tmp);
    CLEANUP_GC(app_gc_tmp_m);

    XCloseDisplay( appDpy );			// close X display
    appDpy = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
    Atom prop, type;
    int	 format;
    unsigned long length, after;
    unsigned char *data;

    prop = XInternAtom( appDpy, "_XSETROOT_ID", TRUE );
    if ( prop != None ) {			// kill old pixmap
	if ( XGetWindowProperty( appDpy, appRootWin, prop, 0, 1, TRUE,
				 AnyPropertyType, &type, &format, &length,
				 &after, &data ) == Success ) {
	    if ( type == XA_PIXMAP && format == 32 && length == 1 &&
		 after == 0 && data ) {
		XKillClient( appDpy, *((Pixmap*)data) );
		XFree( data );
	    }
	    Pixmap dummy = XCreatePixmap( appDpy, appRootWin, 1, 1, 1 );
	    XChangeProperty( appDpy, appRootWin, prop, XA_PIXMAP, 32,
			     PropModeReplace, (uchar *)&dummy, 1 );
	    XSetCloseDownMode( appDpy, RetainPermanent );
	}
    }
}

void qt_updated_rootinfo()
{
    app_save_rootinfo = TRUE;
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


/*----------------------------------------------------------------------------
  \relates QApplication
  Adds a global routine that will be called from the QApplication destructor.
  This function is normally used to add cleanup routines.

  CleanUpFunctions is defined as <code> typedef void
  (*CleanUpFunction)(); </code>, i.e. a pointer to a function that
  takes no arguments and returns nothing.

  Example of use:
  \code
    static int *global_ptr = 0;

    void cleanup_ptr()
    {
	delete [] global_ptr;
    }

    void init_ptr()
    {
	global_ptr = new int[100];		// allocate data
	qAddPostRoutine( cleanup_ptr );		// delete later
    }
  \endcode
 ----------------------------------------------------------------------------*/

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

Display *qt_xdisplay()				// get current X display
{
    return appDpy;
}

int qt_xscreen()				// get current X screen
{
    return appScreen;
}

WId qt_xrootwin()				// get X root window
{
    return appRootWin;
}

bool qt_nograb()				// application no-grab option
{
#if defined(DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}

GC qt_xget_readonly_gc( bool monochrome )	// get read-only GC
{
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_ro_m ) {			// create GC for bitmap
	    Pixmap pm =	 XCreatePixmap( appDpy, appRootWin, 8, 8, 1 );
	    app_gc_ro_m = XCreateGC( appDpy, pm, 0, 0 );
	    XFreePixmap( appDpy, pm );
	}
	gc = app_gc_ro_m;
    }
    else {					// create standard GC
	if ( !app_gc_ro )
	    app_gc_ro = XCreateGC( appDpy, appRootWin, 0, 0 );
	gc = app_gc_ro;
    }
    return gc;
}

GC qt_xget_temp_gc( bool monochrome )		// get use'n throw GC
{
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m ) {			// create GC for bitmap
	    Pixmap pm =	 XCreatePixmap( appDpy, appRootWin, 8, 8, 1 );
	    app_gc_tmp_m = XCreateGC( appDpy, pm, 0, 0 );
	    XFreePixmap( appDpy, pm );
	}
	gc = app_gc_tmp_m;
    }
    else {					// create standard GC
	if ( !app_gc_tmp )
	    app_gc_tmp = XCreateGC( appDpy, appRootWin, 0, 0 );
	gc = app_gc_tmp;
    }
    return gc;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \fn QWidget *QApplication::mainWidget() const
  Returns the main application widget, or 0 if there is not a defined
  main widget.
  \sa setMainWidget()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the main widget of the application.

  The special thing about the main widget is that destroying the main
  widget (i.e. the program calls QWidget::close() or the user
  double-clicks the window close box) will leave the main event loop and
  \link QApplication::quit() exit the application\endlink.

  For X-Windows, this function also resizes and moves the main widget
  according to the \e -geometry command-line option, so you should
  \link QWidget::setGeometry() set the default geometry\endlink before
  calling setMainWidget().

  \sa mainWidget(), exec(), quit()
 ----------------------------------------------------------------------------*/

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;			// set main widget
    if ( main_widget ) {			// give WM command line
	XSetWMProperties( main_widget->display(), main_widget->id(),
			  0, 0, app_argv, app_argc, 0, 0, 0 );
	if ( mwTitle )
	    XStoreName( appDpy, main_widget->id(), mwTitle );
	if ( mwGeometry ) {			// parse geometry
	    int	x, y;
	    int w, h;
	    int m = XParseGeometry( mwGeometry, &x, &y, (uint*)&w, (uint*)&h );
	    if ( (m & XValue) == 0 )	  x = main_widget->geometry().x();
	    if ( (m & YValue) == 0 )	  y = main_widget->geometry().y();
	    if ( (m & WidthValue) == 0 )  w = main_widget->width();
	    if ( (m & HeightValue) == 0 ) h = main_widget->height();
	    int minw, minh, maxw, maxh;
	    if ( main_widget->minimumSize(&minw,&minh) ) {
		w = QMAX(w,minw);
		h = QMAX(h,minh);
	    }
	    if ( main_widget->maximumSize(&maxw,&maxh) ) {
		w = QMIN(w,maxw);
		h = QMIN(h,maxh);
	    }
	    main_widget->setGeometry( x, y, w, h );
	}
    }
}


/*----------------------------------------------------------------------------
  \fn QWidget *QApplication::focusWidget() const
  Returns the application widget that has the keyboard input focus, or null
  if no application widget has the focus.
  \sa QWidget::setFocus(), QWidget::hasFocus()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn QWidget *QApplication::desktop()
  Returns the desktop widget (also called the root window).

  The desktop widget is useful for obtaining the size of the screen.
  It can also be used to draw on the desktop.

  \code
    QWidget *d = QApplication::desktop();
    int w=d->width();			// returns screen width
    int h=d->height();			// returns screen height
    d->setBackgroundColor( red );	// makes desktop red
  \endcode
 ----------------------------------------------------------------------------*/

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

typedef declare(QListM,QCursor) QCursorList;

static QCursorList *cursorStack = 0;

/*----------------------------------------------------------------------------
  \fn QCursor *QApplication::cursor()
  Returns the active application cursor.

  This function returns 0 if no application cursor has been defined (i.e. the
  internal cursor stack is empty).

  \sa setCursor(), restoreCursor()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the application cursor to \e cursor.

  Application cursor are intended for showing the user that the application
  is in a special state, for example during an operation that might take some
  time.

  This cursor will be displayed in all application widgets until
  restoreCursor() or another setCursor() is called.

  Application cursors are stored on an internal stack.  setCursor() pushes
  the cursor onto the stack, and restoreCursor() pops the active cursor off
  the stack.  Every setCursor() must have an corresponding restoreCursor(),
  otherwise the stack will get out of sync.

  Example:
  \code
    QApplication::setCursor( waitCursor );
    calculate_mandelbrot();			// lunch time...
    QApplication::restoreCursor();
  \endcode

  \sa cursor(), restoreCursor(), QWidget::setCursor()
 ----------------------------------------------------------------------------*/

void QApplication::setCursor( const QCursor &cursor )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    cursorStack->append( app_cursor );
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testWFlags(WCursorSet) )	//   set a cursor
	    XDefineCursor( w->display(), w->id(), app_cursor->handle() );
	++it;
    }
    XFlush( appDpy );				// make X execute it NOW
}

/*----------------------------------------------------------------------------
  Restores the effect of setCursor().

  If setCursor() has been called twice, calling restoreCursor() will activate
  the first cursor set.  Calling restoreCursor() a second time will give
  the widgets their old cursors back.

  Application cursors are stored on an internal stack.  setCursor() pushes
  the cursor onto the stack, and restoreCursor() pops the active cursor
  off the stack.  Every setCursor() must have an corresponding
  restoreCursor(), otherwise the stack will get out of sync.  cursor()
  returns 0 if the cursor stack is empty.

  \sa setCursor(), cursor().
 ----------------------------------------------------------------------------*/

void QApplication::restoreCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// set back to original cursors
	if ( w->testWFlags(WCursorSet) )
	    XDefineCursor( w->display(), w->id(),
			   app_cursor ? app_cursor->handle()
			   	      : w->cursor().handle() );
	++it;
    }
    XFlush( appDpy );
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	QWidget *w;
	QObjectListIt it( *p->children() );
	it.toLast();
	while ( it.current() ) {
	    if ( it.current()->isWidgetType() ) {
		w = (QWidget*)it.current();
		if ( w->isVisible() && w->geometry().contains(pos) ) {
		    QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		    return c ? c : w;
		}
	    }
	    --it;
	}
    }
    return 0;
}

static Window findClientWindow( Window win, Atom WM_STATE, bool leaf )
{
    Atom   type = None;
    int	   format, i;
    ulong  nitems, after;
    uchar *data;
    Window root, parent, target=0, *children=0;
    uint   nchildren;
    XGetWindowProperty( appDpy, win, WM_STATE, 0, 0, False, AnyPropertyType,
			&type, &format, &nitems, &after, &data );
    if ( data )
	XFree( (char *)data );
    if ( type )
	return win;
    if ( !XQueryTree(appDpy,win,&root,&parent,&children,&nchildren) ) {
	if ( children )
	    XFree( (char *)children );
	return 0;
    }
    for ( i=nchildren-1; !target && i >= 0; i-- )
	target = findClientWindow( children[i], WM_STATE, leaf );
    if ( children )
	XFree( (char *)children );
    return target;
}


/*----------------------------------------------------------------------------
  Returns a pointer to the widget at global screen position \e (x,y), or a
  null pointer if there is no Qt widget there.

  Returns a child widget if \e child is TRUE or a top level widget if
  \e child is FALSE.

  \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
 ----------------------------------------------------------------------------*/

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    Window target;
    if ( !XTranslateCoordinates(appDpy, appRootWin, appRootWin,
				x, y, &x, &y, &target) )
	return 0;
    if ( !target || target == appRootWin )
	return 0;
    static Atom WM_STATE = 0;
    if ( WM_STATE == 0 )
	WM_STATE = XInternAtom( appDpy, "WM_STATE", TRUE );
    if ( !WM_STATE )
	return 0;
    target = findClientWindow( target, WM_STATE, TRUE );
    QWidget *w = QWidget::find( target );
    if ( !w )
	return 0;
    if ( child ) {
	QWidget *c = findChildWidget( w, w->mapFromParent(QPoint(x,y)) );
	w = c ? c : w;
    }
    return w;
}

/*----------------------------------------------------------------------------
  \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Flushes the X event queue in the X-Windows implementation.
  Does nothing on other platforms.
  \sa syncX()
 ----------------------------------------------------------------------------*/

void QApplication::flushX()
{
    XFlush( appDpy );
}

/*----------------------------------------------------------------------------
  Synchronizes with the X server in the X-Windows implementation.
  Does nothing on other platforms.
  \sa flushX()
 ----------------------------------------------------------------------------*/

void QApplication::syncX()
{
    XSync( appDpy, FALSE );			// don't discard events
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

declare(QListM,QPostEvent);
static QListM(QPostEvent) *postedEvents = 0;	// list of posted events

/*----------------------------------------------------------------------------
  Stores the event in a queue and returns immediatly.

  When control returns to the main event loop, all events that are
  stored in the queue will be sent using the notify() function.

  \sa sendEvent()
 ----------------------------------------------------------------------------*/

void QApplication::postEvent( QObject *receiver, QEvent *event )
{
    if ( !postedEvents ) {			// create list
	postedEvents = new QListM(QPostEvent);
	CHECK_PTR( postedEvents );
	postedEvents->setAutoDelete( TRUE );
    }
    if ( receiver == 0 ) {
#if defined(CHECK_NULL)
	warning( "QApplication::postEvent: Unexpeced null receiver" );
#endif
	return;
    }
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
  Special lookup functions for windows that have been recreated recently
 *****************************************************************************/

static QWidgetIntDict *wPRmapper = 0;		// alternative widget mapper

void qPRCreate( const QWidget *widget, Window oldwin )
{						// QWidget::recreate mechanism
    if ( !wPRmapper ) {
	wPRmapper = new QWidgetIntDict;
	CHECK_PTR( wPRmapper );
    }
    wPRmapper->insert( (long)oldwin, widget );	// add old window to mapper
    QETWidget *w = (QETWidget *)widget;
    w->setWFlags( WRecreated );			// set recreated flag
}

void qPRCleanup( QETWidget *widget )
{
    if ( !(wPRmapper && widget->testWFlags(WRecreated)) )
	return;					// not a recreated widget
    QWidgetIntDictIt it(*wPRmapper);
    QWidget *w;
    while ( (w=it.current()) ) {
	if ( w == widget ) {			// found widget
	    widget->clearWFlags( WRecreated );	// clear recreated flag
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


/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocknot.h) provides installable callbacks
  for select() throught the internal function qt_set_socket_handler().
 *****************************************************************************/

struct QSockNot {
    QObject *obj;
    int	     fd;
};

typedef declare(QListM,QSockNot)	 QSNList;
typedef declare(QListIteratorM,QSockNot) QSNListIt;

static int	sn_highest = -1;
static QSNList *sn_read   = 0;
static QSNList *sn_write  = 0;
static QSNList *sn_except = 0;

static fd_set   sn_readfds;			// fd set for reading
static fd_set   sn_writefds;			// fd set for writing
static fd_set   sn_exceptfds;			// fd set for exceptions

static struct SN_Type {
    QSNList **list;
    fd_set   *fdspec;
    fd_set   *fdres;
} sn_vec[3] = {
    { &sn_read,   &sn_readfds,	 &app_readfds },
    { &sn_write,  &sn_writefds,  &app_writefds },
    { &sn_except, &sn_exceptfds, &app_exceptfds } };


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNList  *list = *sn_vec[type].list;
    fd_set   *fds  =  sn_vec[type].fdspec;
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( !list ) {
	    list = new QSNList;			// create new list
	    CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd  = sockfd;
	if ( list->isEmpty() )
	    list->insert( 0, sn );
	else {					// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(DEBUG)
	    if ( p && p->fd == sockfd ) {
	        static const char *t[] = { "read", "write", "exception" };
		warning( "QSocketNotifier: Multiple socket notifiers for "
			 "same socket %d and type %s", sockfd, t[type] );
	    }
#endif
	    list->insert( list->at(), sn );
	}
	FD_SET( sockfd, fds );
	sn_highest = QMAX(sn_highest,sockfd);
    }
    else {					// disable notifier
	if ( list == 0 )
	    return FALSE;			// no such fd set
	QSockNot *sn = list->first();
	while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
	    sn = list->next();
	if ( !sn )				// not found
	    return FALSE;
	list->remove();				// remove this notifier
	FD_CLR( sockfd, fds );			// clear fd bit
	if ( list->isEmpty() ) {		// no more notifiers
	    delete list;			// delete list
	    *sn_vec[type].list = 0;
	}
	if ( sn_highest == sockfd ) {		// find highest fd
	    sn_highest = -1;
	    for ( int i=0; i<3; i++ ) {
		if ( *sn_vec[i].list )		// list is fd-sorted
		    sn_highest = QMAX(sn_highest,
				      (*(sn_vec[i].list))->getFirst()->fd);
	    }
	}
    }
    return TRUE;
}


typedef declare(QIntDictM,QObject)	   QObjRndDict;
typedef declare(QIntDictIteratorM,QObject) QObjRndDictIt;

static QObjRndDict *sn_rnd_dict = 0;

static void sn_cleanup()
{
    delete sn_rnd_dict;
}

//
// We choose a random activation order to be more fair under high load.
// If a constant order is used and a peer early in the list can
// saturate the IO, it might grab our attention completely.
// Also, if we're using a straight list, the callback routines may
// delete other entries from the list before those other entries are 
// processed.
//

static void sn_activate()
{
    if ( !sn_rnd_dict ) {
	sn_rnd_dict = new QObjRndDict( 53 );
	CHECK_PTR( sn_rnd_dict );
	qAddPostRoutine( sn_cleanup );
    }
    for ( int i=0; i<3; i++ ) {			// for each list...
	if ( *sn_vec[i].list ) {		// any entries?
	    QSNList  *list = *sn_vec[i].list;
	    fd_set   *fds = sn_vec[i].fdres;
	    QSockNot *sn = list->first();
	    while ( sn ) {
		if ( FD_ISSET(sn->fd,fds) )	// store away for activation
		    sn_rnd_dict->insert( rand(), sn->obj );
		sn = list->next();
	    }
	}
    }
    if ( sn_rnd_dict->count() > 0 ) {		// activate entries
	QEvent event( Event_SockAct );
	QObjRndDictIt it( *sn_rnd_dict );
	while ( it.current() ) {
	    QApplication::sendEvent( it.current(), &event );
	    ++it;
	}
	sn_rnd_dict->clear();
    }
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Enters the main event loop and waits until quit() is called or
  the \link setMainWidget() main widget\endlink is destroyed.
  Returns the value that was specified to quit().

  It is necessary to call this function to start event handling.
  The main event loop receives \link QWidget::event() events\endlink from
  the window system and dispatches these to the application widgets.

  Generally, no user interaction can take place before calling exec().
  As a special case, modal widgets like QMessageBox can be used before
  calling exec(), because modal widget have a local event loop.

  \sa quit(), setMainWidget()
 ----------------------------------------------------------------------------*/

int QApplication::exec()
{
    enter_loop();
    return quit_code;
}


/*----------------------------------------------------------------------------
  This function enters the main event loop (recursively).
  Do not call it unless you are an expert.
  \sa exit_loop()
 ----------------------------------------------------------------------------*/

int QApplication::enter_loop()
{
    app_loop_level++;				// increment loop level count

    while ( quit_now == FALSE && !app_exit_loop ) {

	if ( postedEvents && postedEvents->count() )
	    sendPostedEvents();

	XEvent event;
	while ( XPending(appDpy) ) {		// also flushes output buffer

	    if ( quit_now )			// quit between events
		break;
	    XNextEvent( appDpy, &event );	// get next event

	    if ( x11EventFilter(&event) )	// send through app filter
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
		else if ( widget->testWFlags(WRecreated) )
		    qPRCleanup( widget );	// remove from alt mapper
	    }
	    if ( !widget )			// don't know this window
		continue;

	    if ( widget->x11Event(&event) )     // send trough widget filter
		continue;

	    if ( app_do_modal )			// modal event handling
		if ( !qt_try_modal(widget, &event) )
		    continue;

	    switch ( event.type ) {

		case ButtonPress:		// mouse event
		case ButtonRelease:
		case MotionNotify:
		    if ( !widget->isDisabled() ) {
			if ( event.type == ButtonPress &&
			     event.xbutton.button == Button1 )
			    widget->setFocus();
			widget->translateMouseEvent( &event );
		    }
		    break;

		case KeyPress:			// keyboard event
		case KeyRelease: {
		    QWidget *w = QWidget::keyboardGrabber();
		    if ( w )
			widget = (QETWidget*)w;
		    else if ( focus_widget )
			widget = (QETWidget*)focus_widget;
		    else
			widget = (QETWidget*)widget->topLevelWidget();
		    if ( !widget->isDisabled() )
			widget->translateKeyEvent( &event );
		    }
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
		    QWidget *w = widget;
		    while ( w->parentWidget() ) // go to top level
			w = w->parentWidget();
		    while ( w->focusChild )	// go down focus chain
			w = w->focusChild;
		    if ( w != focus_widget && w->acceptFocus() ) {
			focus_widget = w;
			QFocusEvent in( Event_FocusIn );
			QApplication::sendEvent( w, &in );
		    }
		    }
		    break;

		case FocusOut:			// lost focus
		    if ( focus_widget ) {
			QFocusEvent out( Event_FocusOut );
			QWidget *w = focus_widget;
			focus_widget = 0;
			QApplication::sendEvent( w, &out );
		    }
		    break;

		case EnterNotify:		// enter window
		    break;			// ignored

		case LeaveNotify:		// leave window
		    break;			// ignored

		case UnmapNotify:		// window hidden
		    widget->clearWFlags( WState_Visible );
		    break;

		case MapNotify:			// window shown
		    widget->setWFlags( WState_Visible );
		    break;

		case ClientMessage:		// client message
		    if ( (event.xclient.format == 32) ) {
			long *l = event.xclient.data.l;
			if ( *l == (long)q_wm_delete_window ) {
			    if ( widget->translateCloseEvent( &event )
				 && app_loop_level < 2 )
				delete widget;
			}
		    }
		    break;

		case ReparentNotify:		// window manager reparents
		    if ( event.xreparent.parent != appRootWin ) {
			XWindowAttributes a1, a2;
			while ( XCheckTypedWindowEvent( widget->display(),
							widget->id(),
							ReparentNotify,
							&event ) )
			    ;			// skip old reparent events
			Window parent = event.xreparent.parent;
			XGetWindowAttributes( widget->display(), widget->id(),
					      &a1 );
			XGetWindowAttributes( widget->display(), parent, &a2 );
			QRect *r = &widget->crect;
			XWindowAttributes *a;
			if ( a1.x == 0 && a1.y == 0 && (a2.x + a2.y > 0) )
			    a = &a2;		// typical for mwm, fvwm
			else
			    a = &a1;		// typical for twm, olwm
			a->x += a2.border_width;// a->x = parent frame width
			a->y += a2.border_width;// a->y = parent caption height
			widget->frect = QRect(QPoint(r->left() - a->x,
						     r->top()  - a->y),
					      QPoint(r->right()  + a->x,
						     r->bottom() + a->x) );
		    }
		    break;

		default:
		    widget->x11Event( &event );
		    break;
	    }
	}

	if ( quit_now || app_exit_loop )	// break immediatly
	    break;

	timeval *tm = waitTimer();		// wait for timer or X event

	if ( sn_highest >= 0 ) {
	    if ( sn_read )
		app_readfds = sn_readfds;
	    else
		FD_ZERO( &app_readfds );
	    if ( sn_write )
		app_writefds = sn_writefds;
	    if ( sn_except )
		app_exceptfds = sn_exceptfds;
	}
	else {
	    FD_ZERO( &app_readfds );
	}
	FD_SET( app_Xfd, &app_readfds );

#if defined(_OS_HPUX_)
#define FDCAST (int*)
#else
#define FDCAST
#endif
	
	int nsel;
	nsel = select( QMAX(app_Xfd,sn_highest)+1,
		       FDCAST (&app_readfds),
		       FDCAST (sn_write  ? &app_writefds  : 0),
		       FDCAST (sn_except ? &app_exceptfds : 0),
		       tm );
#undef FDCAST

	if ( nsel == -1 ) {
	    if ( errno == EINTR || errno == EAGAIN ) {
		errno = 0;
		continue;
	    }
	    else
		; // select error
	}
	else if ( nsel > 0 && sn_highest >= 0 ) {
	    sn_activate();
	}
	qt_reset_color_avail();			// color approx. optimization
	activateTimer();			// activate timer(s)
    }
    app_exit_loop = FALSE;
    return 0;
}


/*----------------------------------------------------------------------------
  This function leaves from a recursive call to the main event loop.
  Do not call it unless you are an expert.
  \sa enter_loop()
 ----------------------------------------------------------------------------*/

void QApplication::exit_loop()
{
    app_exit_loop = TRUE;
}


/*----------------------------------------------------------------------------
  This virtual function is only implemented under X-Windows.

  If you create an application that inherits QApplication and reimplement this
  function, you get direct access to all X events that the are received
  from the X server.

  Return TRUE if you want to stop the event from being dispatched, or return
  FALSE for normal event dispatching.
 ----------------------------------------------------------------------------*/

bool QApplication::x11EventFilter( XEvent * )
{
    return FALSE;
}


/*****************************************************************************
  Modal widgets; Since Xlib has little support for this we roll our own
  modal widget mechanism.
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

static QWidgetList *modal_stack = 0;		// stack of modal widgets


bool qt_modal_state()				// application in modal state?
{
    return app_do_modal;
}


void qt_enter_modal( QWidget *widget )		// enter modal state
{
    if ( !modal_stack ) {			// create modal stack
	modal_stack = new QWidgetList;
	CHECK_PTR( modal_stack );
    }
    modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
    qApp->enter_loop();
}


void qt_leave_modal( QWidget *widget )		// leave modal state
{
    if ( modal_stack && modal_stack->findRef(widget) >= 0 ) {
	modal_stack->remove();
	if ( modal_stack->isEmpty() ) {
	    delete modal_stack;
	    modal_stack = 0;
	}
	qApp->exit_loop();
    }
    app_do_modal = modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, XEvent *event )
{
    bool     block_event  = FALSE;
    bool     expose_event = FALSE;
    QWidget *modal = 0;

    switch ( event->type ) {
	case ButtonPress:			// disallow mouse/key events
	case ButtonRelease:
	case MotionNotify:
	case KeyPress:
	case KeyRelease:
	case ClientMessage:
	    block_event	 = TRUE;
	    break;
	case Expose:
	    expose_event = TRUE;
	    break;
    }

    if ( widget->testWFlags(WType_Modal) )	// widget is modal
	modal = widget;
    else {					// widget is not modal
	while ( widget->parentWidget() ) {	// find overlapped parent
	    if ( widget->testWFlags(WType_Overlap) )
		break;
	    widget = widget->parentWidget();
	}
	if ( widget->testWFlags(WType_Popup) )	// popups are ok
	    return TRUE;
	if ( widget->testWFlags(WType_Modal) )	// is it modal?
	    modal = widget;
    }

    ASSERT( modal_stack && modal_stack->getFirst() );
    QWidget *top = modal_stack->getFirst();

    if ( modal == top )				// don't block event
	return TRUE;

    if ( top->parentWidget() == 0 && (block_event || expose_event) )
	XRaiseWindow( appDpy, top->id() );	// raise app-modal widget

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

QWidgetList *popupWidgets = 0;			// list of popup widgets
bool popupCloseDownMode = FALSE;

void qt_open_popup( QWidget *popup )		// add popup widget
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 && !qt_nograb() ){ // grab mouse/keyboard
	XGrabKeyboard( popup->display(), popup->id(), TRUE,
		       GrabModeSync, GrabModeSync, CurrentTime );
	XAllowEvents( popup->display(), SyncKeyboard, CurrentTime );
	XGrabPointer( popup->display(), popup->id(), TRUE,
		      (uint)(ButtonPressMask | ButtonReleaseMask |
		      ButtonMotionMask | EnterWindowMask | LeaveWindowMask),
		      GrabModeSync, GrabModeAsync,
		      None, None, CurrentTime );
	XAllowEvents( popup->display(), SyncPointer, CurrentTime );
    }
}

void qt_close_popup( QWidget *popup )		// remove popup widget
{
    if ( !popupWidgets )
	return;
    popupWidgets->removeRef( popup );
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() ) {			// grabbing not disabled
	    XUngrabKeyboard( popup->display(), CurrentTime );
	    if ( mouseButtonState != 0 )	// mouse release event
		XAllowEvents( popup->display(), AsyncPointer, CurrentTime );
	    else {				// mouse press event
		mouseButtonPressTime -= 10000;	// avoid double click
		XAllowEvents( popup->display(), ReplayPointer, CurrentTime );
	    }
	    XFlush( popup->display() );
	}
    }
}


/*****************************************************************************
  Timer handling; Xlib has no application timer support so we'll have to
  make our own from scratch.
 
  NOTE: These functions are for internal use. QObject::startTimer() and
 	 QObject::killTimer() are for public use.
 	 The QTimer class provides a high-level interface which translates
 	 timer events into signals.
 
  qStartTimer( interval, obj )
 	Starts a timer which will run until it is killed with qKillTimer()
 	Arguments:
 	    long interval	timer interval in milliseconds
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


//
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


//
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


//
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
	if ( !t || currentTime < t->timeout )	// no timer has expired
	    break;
	timerList->take();			// unlink from list
	t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
    }
    return TRUE;
}


//
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
	timerList = 0;
	delete [] timerBitVec;
	timerBitVec = 0;
    }
}


//
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


/*****************************************************************************
  Event translation; translates X events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// Xlib doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

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
	    if ( !testWFlags(WMouseTracking) )
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
		 QABS(event->xbutton.x - mouseXPos) < 5 &&
		 QABS(event->xbutton.y - mouseYPos) < 5 ) {
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
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
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
			      (uint)(ButtonPressMask | ButtonReleaseMask |
			      ButtonMotionMask |
			      EnterWindowMask | LeaveWindowMask),
			      GrabModeAsync, GrabModeAsync,
			      None, None, CurrentTime );
	    }
	}
    }
    else {
	QWidget *widget = this;
	QWidget *mg = QWidget::mouseGrabber();
	if ( mg && mg != this ) {
	    widget = mg;
	    pos = mapToGlobal( pos );
	    pos = mg->mapFromGlobal( pos );
	}
	QMouseEvent e( type, pos, button, state );
	if ( popupCloseDownMode ) {
	    popupCloseDownMode = FALSE;
	    if ( testWFlags(WType_Popup) )	// ignore replayed event
		return TRUE;
	}
	QApplication::sendEvent( widget, &e );
    }
    return TRUE;
}


//
// Keyboard event translation
//

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include "qkeycode.h"

static KeySym KeyTbl[] = {			// keyboard mapping table
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
    XK_KP_Space,	Key_Space,		// numeric keypad
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
    int	   code = -1;
    char   ascii[16];
    int	   count;
    int	   state;
    KeySym key;

    type = (event->type == KeyPress) ? Event_KeyPress : Event_KeyRelease;

    count = XLookupString( &((XEvent*)event)->xkey, ascii, 16, &key, NULL );
    state = translateButtonState( event->xkey.state );

    // commentary in X11/keysymdef says that X codes match ASCII, so it
    // is safe to use the locale functions to process X codes
    if ( key < 256 )
	code = isprint(key) ? toupper(key) : 0; // upper-case key, if known
    else
    if ( key >= XK_F1 && key <= XK_F24 )	// function keys
	code = Key_F1 + ((int)key - XK_F1);	// assumes contiguous codes!
    else
    if ( key >= XK_KP_0 && key <= XK_KP_9 )	// numeric keypad keys
	code = Key_0 + ((int)key - XK_KP_0);	// assumes contiguous codes!
    else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] && code == -1 ) {
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
#if 0 // defined(DEBUG)
    if ( code == -1 ) {				// cannot translate keysym
	debug( "translateKey: No translation for X keysym %s (0x%x)",
	       XKeysymToString(XLookupKeysym(&((XEvent*)event)->xkey,0)),
	       key );
	return FALSE;
    }
#endif
#if 0 // defined(DEBUG)
    if ( count > 1 ) {
	ascii[15] = '\0';		// ### need to support and test this
	debug( "translateKey: Multibyte translation disabled (%d, %s)",
	       count, ascii );
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


//
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
    setWFlags( WState_Paint );
    QApplication::sendEvent( this, &e );
    clearWFlags( WState_Paint );
    return TRUE;
}


//
// ConfigureNotify (window move and resize) event translation
//
// The problem with ConfigureNotify is that one cannot trust x and y values
// in the xconfigure struct. Top level widgets are reparented by the window
// manager, and (x,y) is sometimes relative to the parent window, but not
// always!  It is safer (but slower) to translate the window coordinates.
//

bool QETWidget::translateConfigEvent( const XEvent *event )
{
    if ( parentWidget() && !testWFlags(WType_Modal) )
	return TRUE;				// child widget
    Window child;
    int	   x, y;
    XTranslateCoordinates( display(), id(), DefaultRootWindow(display()),
			   0, 0, &x, &y, &child );
    QPoint newPos( x, y );
    QSize  newSize( event->xconfigure.width, event->xconfigure.height );
    QRect  r = geometry();
    if ( newSize != size() ) {			// size changed
	QSize oldSize = size();
	r.setSize( newSize );
	setCRect( r );
	QResizeEvent e( newSize, oldSize );
	QApplication::sendEvent( this, &e );
    }
    if ( newPos != geometry().topLeft() ) {
	QPoint oldPos = pos();
	r.setTopLeft( newPos );
	setCRect( r );
	QMoveEvent e( frameGeometry().topLeft(), oldPos );
	QApplication::sendEvent( this, &e );
    }
    return TRUE;
}


//
// Close window event translation
//

bool QETWidget::translateCloseEvent( const XEvent * )
{
    QCloseEvent e;
    if ( QApplication::sendEvent(this, &e) ) {	// accepts close
	hide();
	if ( qApp->mainWidget() == this )
	    qApp->quit();
	else
	    return TRUE;			// delete this widget
    }
    return FALSE;
}

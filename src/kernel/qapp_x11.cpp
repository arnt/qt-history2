/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp_x11.cpp#102 $
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
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#undef gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );

#if defined(DEBUG) && !defined(CHECK_MEMORY)
#define	 CHECK_MEMORY
#include <qmemchk.h>
#endif

#if defined(_OS_LINUX_)
#include <qfile.h>
#include <qstring.h>
#include <unistd.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qapp_x11.cpp#102 $")


// --------------------------------------------------------------------------
// Internal variables and functions
//

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
#if defined(DEBUG)
static bool	appMemChk	= FALSE;	// memory checking (debugging)
#endif

static bool	app_do_modal	= FALSE;	// modal mode
static int	app_loop_level	= 1;		// event loop level
static bool	app_exit_loop	= FALSE;	// flag to exit local loop
static int	app_Xfd;			// X network socket
static int	app_Xfd_width;
static fd_set	app_fdset;

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
typedef void  (*VFPTR_ARG)( int, char ** );
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
#else
#define SIG_HANDLER __sighandler_t
#endif


// --------------------------------------------------------------------------
// qt_init() - initializes Qt for X-Windows
//

void qt_init( int *argcptr, char **argv )
{
    int argc = *argcptr;
    int i, j;
#if defined(DEBUG)
    int mcBufSize = 100000;			// default memchk settings
    const char *mcLogFile = "MEMCHK.LOG";
#endif

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
	else if ( arg == "-style=windows" )
	    QApplication::setStyle( WindowsStyle );
	else if ( arg == "-style=motif" )
	    QApplication::setStyle( MotifStyle );
#if defined(DEBUG)
	else if ( arg == "-sync" )
	    appSync = !appSync;
	else if ( arg == "-nograb" )
	    appNoGrab = !appNoGrab;
	else if ( arg == "-dograb" )
	    appDoGrab = !appDoGrab;
	else if ( arg == "-memchk" )
	    appMemChk = !appMemChk;
	else if ( arg == "-membuf" ) {
	    if ( ++i < argc ) mcBufSize = atoi(argv[i]);
	}
	else if ( arg == "-memlog" ) {
	    if ( ++i < argc ) mcLogFile = argv[i];
	}
	else if ( arg == "-testdpy" )
	    appDpy = (Display *)1;
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

#if defined(DEBUG)
    if ( appMemChk ) {				// perform memory checking
	memchkSetBufSize( mcBufSize );
	memchkSetLogFile( mcLogFile );
	memchkStart();
    }
#endif

  // Connect to X server

#if defined(DEBUG)
    if ( appDpy != 0 ) {
	debug( "Start testing display, trying XOpenDisplay 1000 times" );
	Display *d;
	for ( int i=0; i<1000; i++ ) {
	    d = XOpenDisplay(appDpyName);
	    if ( d )
		XCloseDisplay( d );
	    else
		debug( "  Error at run #%d", i );
	}
	debug( "Done testing display" );
    }
#endif

    if ( ( appDpy = XOpenDisplay(appDpyName) ) == 0 ) {
	fatal( "%s: cannot connect to X server %s", appName,
	       XDisplayName(appDpyName) );
    }
    app_Xfd = XConnectionNumber( appDpy );	// set X network socket
    app_Xfd_width = app_Xfd + 1;

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


// --------------------------------------------------------------------------
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

#if defined(DEBUG)
    if ( appMemChk )
	memchkStop();				// finish memory checking
#endif
}


// --------------------------------------------------------------------------
// Platform specific global and internal functions
//

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


/*!
  \relates QApplication
  Adds a global routine that will be called from the QApplication destructor.
  This function is normally used to add cleanup routines.

  Example of use:
  \code
    static int *global_ptr = 0;

    void cleanup_ptr()
    {
	delete global_ptr;
    }

    void init_ptr()
    {
	global_ptr = new int[100];		// allocate data
	qAddPostRoutine( cleanup_ptr );		// delete later
    }
  \endcode
*/

void qAddPostRoutine( void (*p)() )		// add post routine
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


// --------------------------------------------------------------------------
// Platform specific QApplication members
//

/*!
  Sets the main widget of the application.

  On X11, this function also resizes and moves the main widget
  according to the \e -geometry command-line option, so you should
  \link QWidget::setGeometry() set default geometry \endlink before
  calling setMainWidget().

  When the user destroys the main widget, the application leaves the
  main event loop and quits.

  \sa mainWidget(), exec(), quit() */

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;			// set main widget
    if ( main_widget ) {			// give WM command line
	XSetWMProperties( main_widget->display(), main_widget->id(),
			  0, 0, app_argv, app_argc, 0, 0, 0 );
	if ( mwTitle )
	    XStoreName( appDpy, main_widget->id(), mwTitle );
	if ( mwGeometry ) {			// parse geometry
	    int	 x, y;
	    uint w, h;
	    int m = XParseGeometry( mwGeometry, &x, &y, &w, &h );
	    if ( (m & XValue) == 0 )	  x = main_widget->geometry().x();
	    if ( (m & YValue) == 0 )	  y = main_widget->geometry().y();
	    if ( (m & WidthValue) == 0 )  w = main_widget->width();
	    if ( (m & HeightValue) == 0 ) h = main_widget->height();
	    main_widget->setGeometry( x, y, w, h );
	}
    }
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
    if ( app_cursor )
	delete app_cursor;
    app_cursor = new QCursor( c );
    CHECK_PTR( app_cursor );
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testWFlags(WCursorSet) )	//   set a cursor
	    XDefineCursor( w->display(), w->id(), app_cursor->handle() );
	++it;
    }
    XFlush( appDpy );				// make X execute it NOW
}

void QApplication::restoreCursor()		// restore application cursor
{
    if ( !app_cursor )				// there is no app cursor
	return;
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// set back to original cursors
	if ( w->testWFlags(WCursorSet) )
	    XDefineCursor( w->display(), w->id(), w->cursor().handle() );
	++it;
    }
    XFlush( appDpy );
    delete app_cursor;				// reset app_cursor
    app_cursor = 0;
}


#if 0

static bool isLeafWidget( Window win )
{
    QWidget *w = QWidget::find( win );
    if ( !w || !w->children() )
	return FALSE;
    QObjectListIt it( *w->children() );
    while ( it.current() ) {
	if ( it.current()->isWidgetType() )
	    return FALSE;
	++it;
    }
    return TRUE;
}

static Window findChild( Window win, Atom WM_STATE, bool leaf )
{
    Window root, parent, target=0, *children;
    uint   nchildren;
    Atom   type = None;
    int	   format, i;
    ulong  nitems, after;
    uchar *data;
    QWidget *w;

    if ( !XQueryTree(appDpy, win, &root, &parent, &children, &nchildren) )
	return 0;
    for ( i=nchildren-1; !target && i>=0; i++ ) {
	w = QWidget::find( children[i] );
	if ( w )
	    type = 1;
	else
	    XGetWindowProperty( appDpy, children[i], WM_STATE, 0, 0, False,
				AnyPropertyType, &type, &format, &nitems,
				&after, &data );
	if ( type && isLeafWidget(children[i]) )
	    target = children[i];
    }
    for ( i=0; !target && i<(int)nchildren; i++ )
	target = findChild( children[i], WM_STATE, leaf );
    if ( children )
	XFree( (char *)children );
    return target;
}
#endif

static QWidget *findChildWidget( QWidget *p, QPoint pos )
{
    if ( p->children() ) {
	QWidget *w;
	QObjectListIt it( *p->children() );
	it.toLast();
	while ( it.current() ) {
	    if ( it.current()->isWidgetType() ) {
		w = (QWidget*)it.current();
		if ( w->geometry().contains(pos) ) {
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
    if ( type )
	return win;
    if ( !XQueryTree(appDpy,win,&root,&parent,&children,&nchildren) )
	return 0;
    for ( i=nchildren-1; !target && i >= 0; i-- )
	target = findClientWindow( children[i], WM_STATE, leaf );
    if ( children )
	XFree( (char *)children );
    return target;
}


/*!
  Returns a pointer to the widget at global screen position \e (x,y), or a
  null pointer if there is no Qt widget there.

  Returns a child widget if \e child is TRUE or a top level widget if
  \e child is FALSE.

  \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/

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

/*!
  \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )
*/


#if 0

/*!
  Returns the handle pointer to the window at global screen position \e
  (x,y) or 0 if this function fails.

  Returns a child window if \e child is TRUE or a top level widget if
  \e child is FALSE.

  \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard(),
  QPixmap::grabWindow()
*/

HANDLE QApplication::windowAt( int x, int y, bool child )
{
    Window target;
    if ( !XTranslateCoordinates(appDpy, appRootWin, appRootWin,
				x, y, &x, &y, &target) )
	return 0;
    if ( !target || target == appRootWin )
	return appRootWin;
    static Atom WM_STATE = 0;
    if ( WM_STATE == 0 )
	WM_STATE = XInternAtom( appDpy, "WM_STATE", TRUE );
    if ( !WM_STATE )
	return 0;
    target = findClientWindow( target, WM_STATE, TRUE );
    if ( child ) {
	Window c = findChildWindow( target, w->mapFromParent(QPoint(x,y)) );
	target = c ? c : target;
    }
    return target;
}

/*!
  \overload HANDLE QApplication::windowAt( const QPoint &pos, bool child )
*/

#endif


/*!  Flushes the X event queue in the X-Windows implementation.	 Does
  nothing on other platforms. \sa syncX() */

void QApplication::flushX()			// flush X output buffer
{
    XFlush( appDpy );
}

/*!  Synchronizes with the X server in the X-Windows implementation.
  Does nothing on other platforms. \sa flushX() */

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


// --------------------------------------------------------------------------
// Main event loop
//

/*!  This function may be removed in the next release of the Qt
  library.  This is what we currently think is a nice and pretty
  main():

  \code
    #include <qapp.h>				// defines QApplication
    #include <qpushbt.h>			// defines QPushButton

    int main( int argc, char **argv )
    {
	QApplication app( argc, argv );		// create app object
	QPushButton  hi( "Hello, world" );	// create a push button
	app.setMainWidget( &hi );		// define as main widget
	hi.show();				// show button
	return a.exec();			// run main event loop
    }
  \endcode

  Ie. call setMainWidget(), then exec() with no parameters. */

int QApplication::exec( QWidget *mainWidget )	// enter main event loop
{
    debug( "QApplication: exec( QWidget * ) IS OBSOLETE"
	   "  Use setMainWidget to set the main widget" );
    setMainWidget( mainWidget );
    return exec();
}


/*!
  Enters the main event loop and waits until quit() is called or
  the \link setMainWidget() main widget\endlink is destroyed.
  Returns the value that was specified to quit().

  It is necessary to call this function to start event handling.
  The main event loop receives \link QWidget::event() events\endlink from
  the window system and dispatches these to the application widgets.  No
  user interaction can take place before calling exec().

  As a special case, modal widgets like QMessageBox can be used before
  calling exec(), because modal widget have a local event loop.
  \sa quit(), setMainWidget()
*/

int QApplication::exec()			// enter main event loop
{
    enter_loop();
    return quit_code;
}


int QApplication::enter_loop()			// local event loop
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
		else if ( widget->testWFlags(WRecreated) )
		    qPRCleanup( widget );	// remove from alt mapper
	    }
	    if ( !widget )			// don't know this window
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
		case KeyRelease:
		    if ( focus_widget )
			widget = (QETWidget*)focus_widget;
		    else
			widget = (QETWidget*)widget->topLevelWidget();
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

	if ( quit_now || app_exit_loop )	// break immediatly
	    break;

	FD_ZERO( &app_fdset );
	FD_SET( app_Xfd, &app_fdset );
	timeval *tm = waitTimer();		// wait for timer or X event
	select( app_Xfd_width, &app_fdset, NULL, NULL, tm );
	qt_reset_color_avail();			// color approx. optimization
	activateTimer();			// activate timer(s)
    }
    app_exit_loop = FALSE;
    return 0;
}

void QApplication::exit_loop()
{
    app_exit_loop = TRUE;
}


bool QApplication::x11EventFilter( XEvent * )	// X11 event filter
{
    return FALSE;
}


// --------------------------------------------------------------------------
// Modal widgets; Since Xlib has little support for this we roll our own
// modal widget mechanism.
// A modal widget without a parent becomes application-modal.
// A modal widget with a parent becomes modal to its parent and grandparents..
//
// qt_enter_modal()
//	Enters modal state and returns when the widget is hidden/closed
//	Arguments:
//	    QWidget *widget	A modal widget
//
// qt_leave_modal()
//	Leaves modal state for a widget
//	Arguments:
//	    QWidget *widget	A modal widget
//

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


// --------------------------------------------------------------------------
// Popup widget mechanism
//
// qt_open_popup()
//	Adds a widget to the list of popup widgets
//	Arguments:
//	    QWidget *widget	The popup widget to be added
//
// qt_close_popup()
//	Removes a widget from the list of popup widgets
//	Arguments:
//	    QWidget *widget	The popup widget to be removed
//

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
    if ( popupWidgets->findRef(popup) != -1 )
	popupWidgets->remove();
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


// --------------------------------------------------------------------------
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
    setWFlags( WState_Paint );
    QApplication::sendEvent( this, &e );
    clearWFlags( WState_Paint );
    return TRUE;
}


// --------------------------------------------------------------------------
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


// --------------------------------------------------------------------------
// Close window event translation
//

bool QETWidget::translateCloseEvent( const XEvent * )
{
    QCloseEvent e;
    if ( QApplication::sendEvent( this, &e ) ) {// close widget
	hide();
	if ( qApp->mainWidget() == this )
	    qApp->quit();
	else
	    return TRUE;			// accepts close
    }
    return FALSE;
}

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_fb.cpp#2 $
**
** Implementation of Qt/FB startup routines and event handling
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

// Started with from qapplication_x11.cpp,v 2.399 1999/10/22 14:39:33

// NOT REVISED

#define select		_qt_hide_select
#define gettimeofday	_qt_hide_gettimeofday

#include "qglobal.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qtextcodec.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qsocketnotifier.h"
#include "qsessionmanager.h"
#include "qvaluelist.h"
#include "qdict.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "../extensions/network/src/qsocket.h"

#include <stdlib.h>
#ifdef QT_SM_SUPPORT
#include <pwd.h>
#endif
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#define	 GC GC_QQQ

#if defined(_OS_LINUX_) && defined(DEBUG)
#include "qfile.h"
#include <unistd.h>
#endif

#if defined(_OS_IRIX_)
#include <bstring.h>
#endif

#include <sys/time.h>

#if defined(_OS_AIX_) && defined(_CC_GNU_)
#include <sys/select.h>
#include <unistd.h>
#endif

#if defined(_OS_QNX_)
#include <sys/select.h>
#endif

#undef gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );
#undef select
extern "C" int select( int, void *, void *, void *, struct timeval * );

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

const int QTFB_PORT=0x4642; // FB

class QtFBEventSource;

static char    *appName;			// application name
static char    *appFont		= 0;		// application font
static char    *appBGCol	= 0;		// application bg color
static char    *appFGCol	= 0;		// application fg color
static char    *appBTNCol	= 0;		// application btn color
static char    *mwGeometry	= 0;		// main widget geometry
static char    *mwTitle		= 0;		// main widget title
static bool	mwIconic	= FALSE;	// main widget iconified

static bool	app_do_modal	= FALSE;	// modal mode
static QtFBEventSource*	app_dpy;		// QtFB `display'
static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static int	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse position in act window

static QWidgetList *modal_stack  = 0;		// stack of modal widgets
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
static bool	    popupGrabOk;

static bool sm_blockUserInput = FALSE;		// session management

// one day in the future we will be able to have static objects in libraries....
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups


typedef void  (*VFPTR)();
typedef QList<void> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

static void	initTimers();
static void	cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval        *qt_wait_timer();
int	        qt_activate_timers();

QObject	       *qt_clipboard = 0;

QWidget	       *qt_button_down	 = 0;		// widget got last button-down

extern bool qt_is_gui_used; // qwidget.cpp

// Events (to be moved)
struct QtFBAnyEvent {
    int type;
    int window;
};

struct QtFBMouseEvent {
    int type;
    int zero_window;
    int x_root, y_root, state;
    int time; // milliseconds
};

struct QtFBFocusEvent {
    int type;
    int window;
    uchar get_focus;
};

struct QtFBKeyEvent {
    int type;
    int window;
    int unicode_or_keycode;
    int state;
    uchar is_text;
    uchar is_press;
    uchar is_auto_repeat;
};

struct QtFBPaintEvent {
    int type;
    int window;
    int x, y, width, height;
};

union QtFBEvent {
    enum Type { Mouse, Focus, Key, Paint };

    int type;
    QtFBAnyEvent any;
    QtFBMouseEvent mouse;
    QtFBFocusEvent focus;
    QtFBKeyEvent key;
    QtFBPaintEvent paint;
};

class QtFBEventSource : public QSocket {
public:
    QtFBEventSource( QObject* parent ) :
	QSocket(parent)
    {
	if ( !app_dpy->connectToHost("localhost", QTFB_PORT) ) {
	    qWarning( "%s: cannot connect to QtFB server", appName );
	    exit( 1 );
	}
    }

    bool eventPending();
    void getEvent( QtFBEvent* event );
};


static bool	qt_try_modal( QWidget *, QtFBEvent * );

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();


// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();

class QtFBMouseEvent;
class QtFBKeyEvent;
class QtFBPaintEvent;

class QETWidget : public QWidget		// event translator widget
{
public:
    void setWState( WFlags f )		{ QWidget::setWState(f); }
    void clearWState( WFlags f )	{ QWidget::clearWState(f); }
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
    bool translateMouseEvent( const QtFBMouseEvent * );
    bool translateKeyEvent( const QtFBKeyEvent *, bool grab );
    bool translatePaintEvent( const QtFBPaintEvent * );
    bool translateWheelEvent( int global_x, int global_y, int delta, int state );
};

/*****************************************************************************
  qt_init() - initializes Qt/FB
 *****************************************************************************/

void qt_init( int *argcptr, char **argv )
{
    char *p;
    int argc = *argcptr;
    int j;

    // Set application name

    p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

    // Get command line params

    j = 1;
    for ( int i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( arg == "-fn" || arg == "-font" ) {
	    if ( ++i < argc )
		appFont = argv[i];
	} else if ( arg == "-bg" || arg == "-background" ) {
	    if ( ++i < argc )
		appBGCol = argv[i];
	} else if ( arg == "-btn" || arg == "-button" ) {
	    if ( ++i < argc )
		appBTNCol = argv[i];
	} else if ( arg == "-fg" || arg == "-foreground" ) {
	    if ( ++i < argc )
		appFGCol = argv[i];
	} else if ( arg == "-name" ) {
	    if ( ++i < argc )
		appName = argv[i];
	} else if ( arg == "-title" ) {
	    if ( ++i < argc )
		mwTitle = argv[i];
	} else if ( arg == "-geometry" ) {
	    if ( ++i < argc )
		mwGeometry = argv[i];
	} else {
	    argv[j++] = argv[i];
	}
    }

    *argcptr = j;

    // Connect to FB server

    if( qt_is_gui_used ) {
	app_dpy = new QtFBEventSource(qApp);
    }

    // Get display parameters

    if( qt_is_gui_used ) {
	// Set paintdevice parameters

	// XXX initial info sent from server

	// Misc. initialization

	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();
    }
    gettimeofday( &watchtime, 0 );

    if( qt_is_gui_used ) {
	qApp->setName( appName );

	QFont f;
	f = QFont( "Times", 10 );
	QApplication::setFont( f );
    }
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( postRList ) {
	VFPTR f = (VFPTR)postRList->first();
	while ( f ) {				// call post routines
	    (*f)();
	    postRList->remove();
	    f = (VFPTR)postRList->first();
	}
	delete postRList;
	postRList = 0;
    }

    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();

    if ( qt_is_gui_used ) {
	delete app_dpy;
    }
    app_dpy = 0;

    delete activeBeforePopup;
    activeBeforePopup = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qAddPostRoutine( Q_CleanUpFunction p )
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

/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;
    if ( main_widget ) {			// give WM command line
	if ( mwTitle ) {
	    // XXX
	}
	if ( mwGeometry ) {			// parse geometry
	    // XXX
	}
    }
}


/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor> QCursorList;

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
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testWState(WState_OwnCursor) )	{ //   set a cursor
	    // XXX XDefineCursor( w->x11Display(), w->winId(), app_cursor->handle() );
	}
	++it;
    }
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// set back to original cursors
	if ( w->testWState(WState_OwnCursor) ) {
	    // XXX XDefineCursor( w->x11Display(), w->winId(),
			   // XXX app_cursor ? app_cursor->handle()
			   // XXX : w->cursor().handle() );
	}
	++it;
    }
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
    }
}


void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
	tellAllWidgets = (++app_tracking == 1);
    } else {
	tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {
	    if ( app_tracking > 0 ) {		// switch on
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( TRUE );
		    w->clearWState(WState_MouseTracking);
		}
	    } else {				// switch off
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setWState(WState_MouseTracking);
		    w->setMouseTracking( FALSE );
		}
	    }
	    ++it;
	}
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos );

static QWidget *findWidget( const QObjectList& list, const QPoint &pos, bool rec )
{
    QWidget *w;
    QObjectListIt it( list );
    it.toLast();
    while ( it.current() ) {
	if ( it.current()->isWidgetType() ) {
	    w = (QWidget*)it.current();
	    if ( w->isVisible() && w->geometry().contains(pos) ) {
		if ( !rec )
		    return w;
		QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		return c ? c : w;
	    }
	}
	--it;
    }
    return 0;
}

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	return findWidget( *p->children(), pos, TRUE );
    }
    return 0;
}

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    // XXX not a fast function...
    QWidgetList *list = topLevelWidgets();

    QPoint pos(x,y);

    if ( list ) {
	QWidget *w;
	QWidgetListIt it( *list );
	it.toLast();
	while ( it.current() ) {
	    w = (QWidget*)it.current();
	    if ( w->isVisible() && w->geometry().contains(pos) ) {
		if ( !child )
		    return w;
		QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		return c ? c : w;
	    }
	    --it;
	}
	delete list;
	return 0;
    } else {
	return 0;
    }
}

void QApplication::flushX()
{
}

void QApplication::syncX()
{
}

void QApplication::beep()
{
}



/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

struct QSockNot {
    QObject *obj;
    int	     fd;
    fd_set  *queue;
};

typedef QList<QSockNot> QSNList;
typedef QListIterator<QSockNot> QSNListIt;

static int	sn_highest = -1;
static QSNList *sn_read	   = 0;
static QSNList *sn_write   = 0;
static QSNList *sn_except  = 0;

static fd_set	sn_readfds;			// fd set for reading
static fd_set	sn_writefds;			// fd set for writing
static fd_set	sn_exceptfds;			// fd set for exceptions
static fd_set	sn_queued_read;
static fd_set	sn_queued_write;
static fd_set	sn_queued_except;

static struct SN_Type {
    QSNList **list;
    fd_set   *fdspec;
    fd_set   *fdres;
    fd_set   *queue;
} sn_vec[3] = {
    { &sn_read,	  &sn_readfds,	 &app_readfds,   &sn_queued_read },
    { &sn_write,  &sn_writefds,	 &app_writefds,  &sn_queued_write },
    { &sn_except, &sn_exceptfds, &app_exceptfds, &sn_queued_except } };


static QSNList *sn_act_list = 0;


static void sn_cleanup()
{
    delete sn_act_list;
    sn_act_list = 0;
    for ( int i=0; i<3; i++ ) {
	delete *sn_vec[i].list;
	*sn_vec[i].list = 0;
    }
}


static void sn_init()
{
    if ( !sn_act_list ) {
	sn_act_list = new QSNList;
	CHECK_PTR( sn_act_list );
	qAddPostRoutine( sn_cleanup );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNList  *list = *sn_vec[type].list;
    fd_set   *fds  =  sn_vec[type].fdspec;
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( !list ) {
	    sn_init();
	    list = new QSNList;			// create new list
	    CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
	sn->queue = sn_vec[type].queue;
	if ( list->isEmpty() ) {
	    list->insert( 0, sn );
	} else {				// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(CHECK_STATE)
	    if ( p && p->fd == sockfd ) {
		static const char *t[] = { "read", "write", "exception" };
		qWarning( "QSocketNotifier: Multiple socket notifiers for "
			 "same socket %d and type %s", sockfd, t[type] );
	    }
#endif
	    if ( p )
		list->insert( list->at(), sn );
	    else
		list->append( sn );
	}
	FD_SET( sockfd, fds );
	sn_highest = QMAX(sn_highest,sockfd);

    } else {					// disable notifier

	if ( list == 0 )
	    return FALSE;			// no such fd set
	QSockNot *sn = list->first();
	while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
	    sn = list->next();
	if ( !sn )				// not found
	    return FALSE;
	FD_CLR( sockfd, fds );			// clear fd bit
	FD_CLR( sockfd, sn->queue );
	if ( sn_act_list )
	    sn_act_list->removeRef( sn );	// remove from activation list
	list->remove();				// remove notifier found above
	if ( sn_highest == sockfd ) {		// find highest fd
	    sn_highest = -1;
	    for ( int i=0; i<3; i++ ) {
		if ( *sn_vec[i].list && (*sn_vec[i].list)->count() )
		    sn_highest = QMAX(sn_highest,  // list is fd-sorted
				      (*sn_vec[i].list)->getFirst()->fd);
	    }
	}
    }

    return TRUE;
}


//
// We choose a random activation order to be more fair under high load.
// If a constant order is used and a peer early in the list can
// saturate the IO, it might grab our attention completely.
// Also, if we're using a straight list, the callback routines may
// delete other entries from the list before those other entries are
// processed.
//

static int sn_activate()
{
    if ( !sn_act_list )
	sn_init();
    int i, n_act = 0;
    for ( i=0; i<3; i++ ) {			// for each list...
	if ( *sn_vec[i].list ) {		// any entries?
	    QSNList  *list = *sn_vec[i].list;
	    fd_set   *fds  = sn_vec[i].fdres;
	    QSockNot *sn   = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, fds ) &&	// store away for activation
		     !FD_ISSET( sn->fd, sn->queue ) ) {
		    sn_act_list->insert( (rand() & 0xff) %
					 (sn_act_list->count()+1),
					 sn );
		    FD_SET( sn->fd, sn->queue );
		}
		sn = list->next();
	    }
	}
    }
    if ( sn_act_list->count() > 0 ) {		// activate entries
	QEvent event( QEvent::SockAct );
	QSNListIt it( *sn_act_list );
	QSockNot *sn;
	while ( (sn=it.current()) ) {
	    ++it;
	    sn_act_list->removeRef( sn );
	    if ( FD_ISSET(sn->fd, sn->queue) ) {
		FD_CLR( sn->fd, sn->queue );
		QApplication::sendEvent( sn->obj, &event );
		n_act++;
	    }
	}
    }
    return n_act;
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    quit_now = FALSE;
    quit_code = 0;
    enter_loop();
    return quit_code;
}


bool QApplication::processNextEvent( bool canWait )
{
    QtFBEvent event;
    int	   nevents = 0;

    if (qt_is_gui_used ) {
	sendPostedEvents();

	while ( app_dpy->eventPending() ) {	// also flushes output buffer
	    if ( app_exit_loop )		// quit between events
		return FALSE;
	    app_dpy->getEvent( &event );	// get next event
	    nevents++;

	    if ( fbProcessEvent( &event ) == 1 )
		return TRUE;
	}
    }
    if ( app_exit_loop )			// break immediately
	return FALSE;

    sendPostedEvents();

    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or X event
    if ( !canWait ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }
    if ( sn_highest >= 0 ) {			// has socket notifier(s)
	if ( sn_read )
	    app_readfds = sn_readfds;
	else
	    FD_ZERO( &app_readfds );
	if ( sn_write )
	    app_writefds = sn_writefds;
	if ( sn_except )
	    app_exceptfds = sn_exceptfds;
    } else {
	FD_ZERO( &app_readfds );
    }

    int nsel;

#if defined(_OS_WIN32_)
#define FDCAST (fd_set*)
#else
#define FDCAST (void*)
#endif

    nsel = select( sn_highest+1,
		   FDCAST (&app_readfds),
		   FDCAST (sn_write  ? &app_writefds  : 0),
		   FDCAST (sn_except ? &app_exceptfds : 0),
		   tm );
#undef FDCAST

    if ( nsel == -1 ) {
	if ( errno == EINTR || errno == EAGAIN ) {
	    errno = 0;
	    return (nevents > 0);
	} else {
	    ; // select error
	}
    } else if ( nsel > 0 && sn_highest >= 0 ) {
	nevents += sn_activate();
    }

    nevents += qt_activate_timers();		// activate timers

    return (nevents > 0);
}

int QApplication::fbProcessEvent( QtFBEvent* event )
{
    if ( fbEventFilter(event) )			// send through app filter
	return 1;

    QETWidget *widget = (QETWidget*)QWidget::find( (WId)event->any.window );

    QETWidget *keywidget=0;
    bool grabbed=FALSE;
    if ( event->type==QtFBEvent::Key ) {
	keywidget = (QETWidget*)QWidget::keyboardGrabber();
	if ( keywidget ) {
	    grabbed = TRUE;
	} else {
	    if ( focus_widget )
		keywidget = (QETWidget*)focus_widget;
	    else if ( widget )
		keywidget = (QETWidget*)widget->topLevelWidget();
	}
    } else if ( event->type==QtFBEvent::Mouse ) {
	// window not reported - work it out ourselves
	// XXX speed up these subfunctions
	widget = (QETWidget*)widgetAt(event->mouse.x_root, event->mouse.y_root, TRUE);
    }

    if ( !widget ) {				// don't know this window
	QWidget* popup = QApplication::activePopupWidget();
	if ( popup ) {
	
	    /*
	      That is more than suboptimal. The real solution should
	      do some keyevent and buttonevent translation, so that
	      the popup still continues to work as the user expects.
	      Unfortunately this translation is currently only
	      possible with a known widget. I'll change that soon
	      (Matthias).
	     */
	
	    // Danger - make sure we don't lock the server
	    switch ( event->type ) {
	    case QtFBEvent::Mouse:
	    case QtFBEvent::Key:
		do {
		    popup->close();
		} while ( (popup = qApp->activePopupWidget()) );
		return 1;
	    }
	}
	return -1;
    }

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, event) ) {
	    return 1;
	}

    if ( widget->fbEvent(event) )		// send through widget filter
	return 1;

    switch ( event->type ) {

    case QtFBEvent::Mouse:			// mouse event
	widget->translateMouseEvent( &event->mouse );
	break;

    case QtFBEvent::Key:				// keyboard event
	if ( keywidget ) // should always exist
	    keywidget->translateKeyEvent( &event->key, grabbed );
	break;

    case QtFBEvent::Paint:				// paint event
	widget->translatePaintEvent( &event->paint );
	break;

    case QtFBEvent::Focus:
	if ( event->focus.get_focus ) {			// got focus
	    if ( widget == desktop() )
		return TRUE; // not interesting
	    if ( inPopupMode() ) // some delayed focus event to ignore
		break;
	    active_window = widget->topLevelWidget();

	    QFocusEvent::setReason( QFocusEvent::ActiveWindow );
	    QWidget *w = widget->focusWidget();
	    while ( w && w->focusProxy() )
		w = w->focusProxy();
	    if ( w && w->isFocusEnabled() )
		w->setFocus();
	    else
		widget->focusNextPrevChild( TRUE );
	    if ( !focus_widget ) {
		if ( widget->focusWidget() )
		    widget->focusWidget()->setFocus();
		else
		    widget->topLevelWidget()->setFocus();
	    }
	    QFocusEvent::resetReason();
	} else {	// lost focus
	    if ( widget == desktop() )
		return TRUE; // not interesting
	    if ( focus_widget && !inPopupMode() ) {
		QFocusEvent::setReason( QFocusEvent::ActiveWindow );
		active_window = 0;
		QFocusEvent out( QEvent::FocusOut );
		QWidget *widget = focus_widget;
		focus_widget = 0;
		QApplication::sendEvent( widget, &out );
		QFocusEvent::resetReason();
	    }
	}
	break;
    default:
	break;
    }

    return 0;
}


void QApplication::processEvents( int maxtime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !app_exit_loop && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}


bool QApplication::x11EventFilter( XEvent * )
{
    return FALSE;
}



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
}


void qt_leave_modal( QWidget *widget )
{
    if ( modal_stack && modal_stack->removeRef(widget) ) {
	if ( modal_stack->isEmpty() ) {
	    delete modal_stack;
	    modal_stack = 0;
	}
    }
    app_do_modal = modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, QtFBEvent *event )
{
    if ( qApp->activePopupWidget() )
	return TRUE;
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=modal_stack->getFirst();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(Qt::WType_Modal) )	// widget is modal
	modal = widget;
    if ( modal == top )				// don't block event
	return TRUE;

#ifdef ALLOW_NON_APPLICATION_MODAL
    if ( top && top->parentWidget() ) {
	// Not application-modal
	// Does widget have a child in modal_stack?
	bool unrelated = TRUE;
	modal = modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != widget ) {
		p = p->parentWidget();
	    }
	    modal = modal_stack->next();
	    if ( p ) unrelated = FALSE;
	}
	if ( unrelated ) return TRUE;		// don't block event
    }
#endif

    bool block_event  = FALSE;
    bool paint_event = FALSE;

    switch ( event->type ) {
	case QtFBEvent::Mouse:			// disallow mouse/key events
	case QtFBEvent::Key:
	case QtFBEvent::Focus:
	    block_event	 = TRUE;
	    break;
	case QtFBEvent::Paint:
	    paint_event = TRUE;
	    break;
    }

    if ( top->parentWidget() == 0 && (block_event || paint_event) )
	top->raise();

    return !block_event;
}


void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 ){ // grab mouse/keyboard
	// XXX grab keyboard
    }

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason( QFocusEvent::ActiveWindow );
    active_window = popup;
    if (active_window->focusWidget())
	active_window->focusWidget()->setFocus();
    else
	active_window->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;

    popupWidgets->removeRef( popup );
    if (popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( popupGrabOk ) {	// grabbing not disabled
	    // XXX ungrab keyboard
	}
	active_window = (*activeBeforePopup);
	    QFocusEvent::setReason( QFocusEvent::ActiveWindow );
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	    QFocusEvent::resetReason();
    }
     else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	 QFocusEvent::setReason( QFocusEvent::ActiveWindow );
	 active_window = popupWidgets->getLast();
	 if (active_window->focusWidget())
	     active_window->focusWidget()->setFocus();
	 else
	     active_window->setFocus();
	 QFocusEvent::resetReason();
     }
}


/*****************************************************************************
  Functions returning the active popup and modal widgets.
 *****************************************************************************/

QWidget *QApplication::activePopupWidget()
{
    return popupWidgets ? popupWidgets->getLast() : 0;
}


QWidget *QApplication::activeModalWidget()
{
    return modal_stack ? modal_stack->getLast() : 0;
}


//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef QList<TimerInfo> TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list


//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline timeval &operator+=( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ( (t1.tv_usec += t2.tv_usec) >= 1000000 ) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
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

static int allocTimerId()			// find avail timer identifier
{
    int i = timerBitVec->size()-1;
    while ( i >= 0 && (*timerBitVec)[i] )
	i--;
    if ( i < 0 ) {
	i = timerBitVec->size();
	timerBitVec->resize( 4 * i );
	for( int j=timerBitVec->size()-1; j > i; j-- )
	    timerBitVec->clearBit( j );
    }
    timerBitVec->setBit( i );
    return i+1;
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
    while ( t.tv_usec >= 1000000 ) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
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

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.
*/

timeval *qt_wait_timer()
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
	if ( currentTime < t->timeout ) {	// time to wait
	    tm = t->timeout - currentTime;
	} else {
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	return &tm;
    }
    return 0;					// no timers
}

/*
  Activates the timer events that have expired. Returns the number of timers
  (not 0-timer) that were activated.
*/

int qt_activate_timers()
{
    if ( !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    int n_act = 0;
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
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
    }
    return n_act;
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	timerList = 0;
	delete timerBitVec;
	timerBitVec = 0;
    }
}


//
// Main timer functions for starting and killing timers
//

int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 ||
	 id > (int)timerBitVec->size() || !obj )// cannot create timer
	return 0;
    timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    CHECK_PTR( t );
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    return id;
}

bool qKillTimer( int id )
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 ||
	 id > (int)timerBitVec->size() || !timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	timerBitVec->clearBit( id-1 );		// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    timerBitVec->clearBit( t->id-1 );
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}


/*****************************************************************************
  Event translation; translates FB events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// FB doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

static int translateButtonState( int s )
{
    // No translation required at this time
    return s;
}

bool QETWidget::translateMouseEvent( const QtFBMouseEvent *event )
{
    static bool manualGrab = FALSE;
    QEvent::Type type;				// event parameters
    QPoint pos;
    QPoint globalPos;
    int	   button = 0;
    int	   state;

    static int old_x_root = -1;
    static int old_y_root = -1;
    static int old_state = -1;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    pos = mapFromGlobal(QPoint( event->x_root, event->y_root ));
    globalPos.rx() = event->x_root;
    globalPos.ry() = event->y_root;
    state = translateButtonState( event->state );
    if ( qt_button_down && (state & (LeftButton |
				     MidButton |
				     RightButton ) ) == 0 )
	qt_button_down = 0;

    const int AnyButton = (LeftButton | MidButton | RightButton );

    if ( event->x_root != old_x_root || event->y_root != old_y_root ) {
	// mouse move
	// XXX compress motion events
	type = QEvent::MouseMove;
    } else if ( /* XXX detect enter/leave */ 0 ) {
	type = QEvent::MouseMove;
	if ( !qt_button_down )
	    state = state & ~AnyButton;
    } else if ( (event->state&AnyButton) != (old_state&AnyButton) ) {
	for ( int button = LeftButton; button <= RightButton; button<<=1 ) {
	    if ( (event->state&button) != (old_state&button) ) {
		// button press or release
		if ( isEnabled() ) {
		    QWidget* w = this;
		    while ( w->focusProxy() )
			w = w->focusProxy();
		    if ( w->focusPolicy() & QWidget::ClickFocus ) {
			QFocusEvent::setReason( QFocusEvent::Mouse );
			w->setFocus();
			QFocusEvent::resetReason();
		    }
		}
		if ( event->state&button ) {
		    qt_button_down = findChildWidget( this, pos );	//magic for masked widgets
		    if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
			qt_button_down = this;
		    if ( /*XXX mouseActWindow == this &&*/
			 mouseButtonPressed == button &&
			 (long)event->time -(long)mouseButtonPressTime
			       < QApplication::doubleClickInterval() &&
			 QABS(event->x_root - mouseXPos) < 5 &&
			 QABS(event->y_root - mouseYPos) < 5 ) {
			type = QEvent::MouseButtonDblClick;
			mouseButtonPressTime -= 2000;	// no double-click next time
		    } else {
			type = QEvent::MouseButtonPress;
			mouseButtonPressTime = event->time;
		    }
		    mouseButtonPressed = button; 	// save event params for
		    mouseXPos = globalPos.x();		// future double click tests
		    mouseYPos = globalPos.y();
		} else {				// mouse button released
		    if ( manualGrab ) {			// release manual grab
			manualGrab = FALSE;
			// XXX XUngrabPointer( x11Display(), CurrentTime );
		    }

		    type = QEvent::MouseButtonRelease;
		}
	    }
	}
    }
    //XXX mouseActWindow = winId();			// save some event params
    mouseButtonState = state;
    if ( type == 0 )				// don't send event
	return FALSE;

    if ( qApp->inPopupMode() ) {			// in popup mode
	QWidget *popup = qApp->activePopupWidget();
	if ( popup != this ) {
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( globalPos );
	}
	bool releaseAfter = FALSE;
	QWidget *popupChild  = findChildWidget( popup, pos );
	QWidget *popupTarget = popupChild ? popupChild : popup;

	if (popup != popupOfPopupButtonFocus){
	    popupButtonFocus = 0;
	    popupOfPopupButtonFocus = 0;
	}

	if ( !popupTarget->isEnabled() )
	    return FALSE;

	switch ( type ) {
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonDblClick:
		popupButtonFocus = popupChild;
		popupOfPopupButtonFocus = popup;
		break;
	    case QEvent::MouseButtonRelease:
		releaseAfter = TRUE;
		break;
	    default:
		break;				// nothing for mouse move
	}

	if ( popupButtonFocus ) {
	    QMouseEvent e( type, popupButtonFocus->mapFromGlobal(globalPos),
			   globalPos, button, state );
	    QApplication::sendEvent( popupButtonFocus, &e );
	    if ( releaseAfter ) {
		popupButtonFocus = 0;
		popupOfPopupButtonFocus = 0;
	    }
	} else {
	    QMouseEvent e( type, pos, globalPos, button, state );
	    QApplication::sendEvent( popup, &e );
	}

	if ( releaseAfter )
	    qt_button_down = 0;

	// XXX WWA: don't understand
	if ( qApp->inPopupMode() ) {			// still in popup mode
	    //XXX if ( popupGrabOk )
		//XXX XAllowEvents( x11Display(), SyncPointer, CurrentTime );
	} else {
	    /* XXX
	    if ( type != QEvent::MouseButtonRelease && state != 0 &&
		 QWidget::find((WId)mouseActWindow) ) {
		manualGrab = TRUE;		// need to manually grab
		XGrabPointer( x11Display(), mouseActWindow, FALSE,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
			      ButtonMotionMask |
			      EnterWindowMask | LeaveWindowMask),
			      GrabModeAsync, GrabModeAsync,
			      None, None, CurrentTime );
	    }
	    */
	}

    } else {
	QWidget *widget = this;
	QWidget *w = QWidget::mouseGrabber();
	if ( !w && qt_button_down )
	    w = qt_button_down;
	if ( w && w != this ) {
	    widget = w;
	    pos = mapToGlobal( pos );
	    pos = w->mapFromGlobal( pos );
	}

	if ( popupCloseDownMode ) {
	    popupCloseDownMode = FALSE;
	    if ( testWFlags(WType_Popup) )	// ignore replayed event
		return TRUE;
	}

	if ( type == QEvent::MouseButtonRelease &&
	     (state & (~button) & ( LeftButton |
				    MidButton |
				    RightButton)) == 0 ) {
	    qt_button_down = 0;
	}

	QMouseEvent e( type, pos, globalPos, button, state );
	QApplication::sendEvent( widget, &e );
    }
    return TRUE;
}


bool QETWidget::translateKeyEvent( const QtFBKeyEvent *event, bool grab )
{
    int	   code = -1;
    int	   state = event->state;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    QWidget *tlw = topLevelWidget();

    if ( !isEnabled() )
	return TRUE;

    QEvent::Type type = event->is_press ?
	QEvent::KeyPress : QEvent::KeyRelease;
    bool    autor = event->is_auto_repeat;
    QString text;
    char   ascii = 0;
    if ( event->is_text ) {
	QChar ch(event->unicode_or_keycode);
	text += ch;
	ascii = ch.latin1();
    }

    bool isAccel = FALSE;
    if (!grab) { // test for accel if the keyboard is not grabbed
	QKeyEvent a( QEvent::AccelAvailable, code, ascii, state, text, FALSE,
		     int(text.length()) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	isAccel = a.isAccepted();
    }

    if ( !isAccel && !text.isEmpty() && testWState(WState_CompressKeys) ) {
	// the widget wants key compression so it gets it

	// XXX not implemented
    }


    // process accelerates before popups
    QKeyEvent e( type, code, ascii, state, text, autor,
		 int(text.length()) );
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel event to tlw if the keyboard is not grabbed
	QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor,
		     int(text.length()) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	if ( a.isAccepted() )
	    return TRUE;
    }
    return QApplication::sendEvent( this, &e );
}


bool QETWidget::translatePaintEvent( const QtFBPaintEvent *event )
{
    QRect  paintRect( event->x, event->y,
		      event->width, event->height );

    QRegion paintRegion( paintRect );

    QPaintEvent e( paintRegion );
    setWState( WState_InPaintEvent );
    qt_set_paintevent_clipping( this, paintRegion);
    QApplication::sendEvent( this, &e );
    qt_clear_paintevent_clipping();
    clearWState( WState_InPaintEvent );
    return TRUE;
}

void  QApplication::setCursorFlashTime( int msecs )
{
    cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

void QApplication::setDoubleClickInterval( int ms )
{
    mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}



/*****************************************************************************
  Session management support (-D QT_SM_SUPPORT to enable it)
 *****************************************************************************/


class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData;
    d->sessionId = session;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

HANDLE QSessionManager::handle() const
{
    return 0;
}

bool QSessionManager::allowsInteraction()
{
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return TRUE;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}

void QSessionManager::setRestartHint( QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand( const QStringList& command)
{
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand( const QStringList& command)
{
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    return d->discardCommand;
}

void QSessionManager::setProperty( const QString&, const QString&)
{
}

void QSessionManager::setProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return FALSE;
}

void QSessionManager::requestPhase2()
{
}

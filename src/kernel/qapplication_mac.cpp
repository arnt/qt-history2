/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_mac.cpp
**
** Implementation of Mac startup routines and event handling
**
** Created : 001018
**
** Copyrigght (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// NOT REVISED
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "qglobal.h"
#include "qt_mac.h"

#include "qapplication.h"
#include "qapplication_p.h"
#include "qcolor_p.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "qobjectlist.h"
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
#include "qptrdict.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qpaintdevicemetrics.h"
#include "qcursor.h"

#if !defined(QMAC_QMENUBAR_NO_NATIVE)
#include "qmenubar.h"
#endif

/*****************************************************************************
  QApplication debug facilities
 *****************************************************************************/
//#define DEBUG_KEY_MAPS
//#define DEBUG_MOUSE_MAPS

#define QMAC_SPEAK_TO_ME
#ifdef QMAC_SPEAK_TO_ME
#include "qvariant.h"
#include "qregexp.h"
#endif

#ifdef Q_WS_MAC9
#define QMAC_EVENT_NOWAIT 0.01
#else
#define QMAC_EVENT_NOWAIT kEventDurationNoWait
#endif

#ifdef Q_WS_MACX
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <qdir.h>
#elif defined(Q_WS_MAC9)
#include <ctype.h>
#endif

#if defined(QT_THREAD_SUPPORT)
#include "qthread.h"
#endif

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static int mouse_button_state = 0;
static bool	app_do_modal	= FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;		// stack of modal widgets
static char    *appName;                        // application name
static Cursor *currentCursor;                  //current cursor
QObject	       *qt_clipboard = 0;
QWidget	       *qt_button_down	 = 0;		// widget got last button-down
QWidget        *qt_mouseover = 0;
QPtrDict<void> unhandled_dialogs;             //all unhandled dialogs (ie mac file dialog)

static EventLoopTimerRef mac_select_timer = NULL;
static EventLoopTimerUPP mac_select_timerUPP = NULL;
static EventHandlerRef app_proc_handler = NULL;
static EventHandlerUPP app_proc_handlerUPP = NULL;

//special case popup handlers - look where these are used, they are very hacky,
//and very special case, if you plan on using these variables be VERY careful!!
static bool qt_closed_popup = FALSE;
EventRef qt_replay_event = NULL;

void qt_mac_destroy_widget(QWidget *w)
{
    if(qt_button_down == w)
	qt_button_down = NULL;
    if(qt_mouseover == w)
	qt_mouseover = NULL;
}

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping(QPaintDevice *dev);

static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;

typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

void qt_install_preselect_handler( VFPTR );
void qt_remove_preselect_handler( VFPTR );
static QVFuncList *qt_preselect_handler = 0;
void qt_install_postselect_handler( VFPTR );
void qt_remove_postselect_handler( VFPTR );
static QVFuncList *qt_postselect_handler = 0;
void qt_install_preselect_handler( VFPTR handler )
{
    if ( !qt_preselect_handler )
	qt_preselect_handler = new QVFuncList;
    qt_preselect_handler->append( handler );
}
void qt_remove_preselect_handler( VFPTR handler )
{
    if ( qt_preselect_handler ) {
	QVFuncList::Iterator it = qt_preselect_handler->find( handler );
	if ( it != qt_preselect_handler->end() )
		qt_preselect_handler->remove( it );
    }
}
void qt_install_postselect_handler( VFPTR handler )
{
    if ( !qt_postselect_handler )
	qt_postselect_handler = new QVFuncList;
    qt_postselect_handler->prepend( handler );
}
void qt_remove_postselect_handler( VFPTR handler )
{
    if ( qt_postselect_handler ) {
	QVFuncList::Iterator it = qt_postselect_handler->find( handler );
	if ( it != qt_postselect_handler->end() )
		qt_postselect_handler->remove( it );
    }
}

//timer stuff
static void	initTimers();
static void	cleanupTimers();
static int      activateNullTimers();

/* Event masks */

// internal Qt types
const UInt32 kEventClassQt = 'cute';
enum {
    //types
    typeQWidget = 1,  /* QWidget * */
    //params
    kEventParamTimer = 'qtim',     /* typeUInt */
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    //events
    kEventQtRequestPropagate = 10,
    kEventQtRequestSelect = 11,
    kEventQtRequestContext = 12,
    kEventQtRequestTimer = 13
};
static bool request_updates_pending = FALSE;
void qt_event_request_updates()
{
    if(request_updates_pending)
	return;
    request_updates_pending = TRUE;

    EventRef upd = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestPropagate, GetCurrentEventTime(),
		kEventAttributeUserEvent, &upd);
    PostEventToQueue( GetCurrentEventQueue(), upd, kEventPriorityHigh );
}

static EventTypeSpec events[] = {
    { kEventClassWindow, kEventWindowUpdate },
    { kEventClassWindow, kEventWindowActivated },
    { kEventClassWindow, kEventWindowDeactivated },
    { kEventClassWindow, kEventWindowShown },
    { kEventClassWindow, kEventWindowHidden },
    { kEventClassWindow, kEventWindowBoundsChanged },

    { kEventClassQt, kEventQtRequestPropagate },
    { kEventClassQt, kEventQtRequestSelect },
    { kEventClassQt, kEventQtRequestContext },
    { kEventClassQt, kEventQtRequestTimer },

    { kEventClassMouse, kEventMouseWheelMoved },
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseMoved },

    { kEventClassKeyboard, kEventRawKeyUp },
    { kEventClassKeyboard, kEventRawKeyDown },
    { kEventClassKeyboard, kEventRawKeyRepeat },

    { kEventClassApplication, kEventAppActivated },
    { kEventClassApplication, kEventAppDeactivated },

    { kEventClassMenu, kEventMenuOpening },
    { kEventClassMenu, kEventMenuTargetItem },

    { kEventClassCommand, kEventCommandProcess }
};

/* platform specific implementations */
void qt_init( int* /* argcptr */, char **argv, QApplication::Type )
{
    // Set application name
    char *p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];
#ifdef Q_WS_MACX
    //special hack to change working directory to a resource fork when running from finder
    if(p && !QDir::isRelativePath(p) && QDir::currentDirPath() == "/") {
	QString path = argv[0];
	int rfork = path.findRev(QString("/") + appName + ".app/");
	if(rfork != -1)
	    QDir::setCurrent(path.left(rfork+1));
    }
#endif

    if ( qt_is_gui_used ) {
	qApp->setName( appName );
	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();

	QWidget *tlw = new QWidget(NULL, "empty_widget", Qt::WDestructiveClose);
	tlw->hide();

	if(!app_proc_handler) {
	    app_proc_handlerUPP = NewEventHandlerUPP(QApplication::globalEventProcessor);
	    InstallEventHandler( GetApplicationEventTarget(), app_proc_handlerUPP,
				 GetEventTypeCount(events), events, (void *)qApp, &app_proc_handler);
	}
	if(!mac_select_timer) {
	    mac_select_timerUPP = NewEventLoopTimerUPP(QApplication::qt_select_timer_callbk);
	    InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0.1,
				  mac_select_timerUPP, (void *)qApp, &mac_select_timer);
	}
    }
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( qt_is_gui_used ) {
	if(app_proc_handler) {
	    RemoveEventHandler(app_proc_handler);
	    app_proc_handler = NULL;
	}
	if(app_proc_handlerUPP) {
	    DisposeEventHandlerUPP(app_proc_handlerUPP);
	    app_proc_handlerUPP = NULL;
	}
	if(mac_select_timer) {
	    RemoveEventLoopTimer(mac_select_timer);
	    mac_select_timer = NULL;
	}
	if(mac_select_timerUPP) {
	    DisposeEventLoopTimerUPP(mac_select_timerUPP);
	    mac_select_timerUPP = NULL;
	}
    }

    if ( postRList ) {
	QVFuncList::Iterator it = postRList->begin();
	while ( it != postRList->end() ) {	// call post routines
	    (**it)();
	    postRList->remove( it );
	    it = postRList->begin();
	}
	delete postRList;
	postRList = 0;
    }
    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
    QFont::cleanup();
    QColor::cleanup();

#if !defined(QMAC_QMENUBAR_NO_NATIVE)
    QMenuBar::cleanup();
#endif
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
}

void qt_updated_rootinfo()
{

}

bool qt_wstate_iconified( WId )
{
    return FALSE;
}

void qAddPostRoutine( QtCleanUpFunction p)
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	Q_CHECK_PTR( postRList );
    }
    postRList->prepend( p );
}


void qRemovePostRoutine( QtCleanUpFunction p )
{
    if ( !postRList ) return;

    QVFuncList::Iterator it = postRList->begin();

    while ( it != postRList->end() ) {
	if ( *it == p ) {
	    postRList->remove( it );
	    it = postRList->begin();
	}
    }
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

extern QWidget * mac_mouse_grabber;
extern QWidget * mac_keyboard_grabber;

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor> QCursorList;
static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace)
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	Q_CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor(cursor);
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    if(cursorStack->isEmpty()) {
	app_cursor = NULL;
	delete cursorStack;
	cursorStack = NULL;
    }
}

#endif

void QApplication::setGlobalMouseTracking( bool b)
{
    if(b)
	app_tracking++;
    else
	app_tracking--;
}


QWidget *recursive_match(QWidget *widg, int x, int y)
{
    // Keep looking until we find ourselves in a widget with no kiddies
    // where the x,y is
    if(!widg)
	return 0;

    const QObjectList *objl=widg->children();
    if(!objl) // No children
	return widg;

    QObjectListIt it(*objl);
    for(it.toLast(); it.current(); --it) {
	if((*it)->isWidgetType()) {
	    QWidget *curwidg=(QWidget *)(*it);
	    if(curwidg->isVisible() && !curwidg->isTopLevel()) {
		int wx=curwidg->x(), wy=curwidg->y();
		int wx2=wx+curwidg->width(), wy2=wy+curwidg->height();
		if(x>=wx && y>=wy && x<=wx2 && y<=wy2) {
		    return recursive_match(curwidg,x-wx,y-wy);
		}
	    }
	}
    }
    // If we get here, it's within a widget that has children, but isn't in any
    // of the children
    return widg;
}

QWidget *QApplication::widgetAt( int x, int y, bool child)
{
    //find the tld
    Point p;
    p.h=x;
    p.v=y;
    WindowPtr wp;
    FindWindow(p,&wp);
    if(!wp || unhandled_dialogs.find((void *)wp))
	return NULL; //oh well, not my widget!

    //get that widget
    QWidget * widget=QWidget::find((WId)wp);
    if(!widget) {
	qWarning("Couldn't find %d",(int)wp);
	return 0;
    }

    //find the child
    if(child) {
	QMacSavedPortInfo savedInfo(widget);
	GlobalToLocal( &p ); //now map it to the window
	widget = recursive_match(widget, p.h, p.v);
    }
    return widget;
}

void QApplication::beep()
{
    SysBeep(0);
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    quit_now = FALSE;
    quit_code = 0;

#if defined(QT_THREAD_SUPPORT)
    qApp->unlock();
#endif

    enter_loop();

    return quit_code;
}

/* timer code */
struct TimerInfo {
    QObject *obj;

    enum { TIMER_ZERO, TIMER_MAC } type;
    EventLoopTimerRef mac_timer;
    int id;
};
static int zero_timer_count = 0;
typedef QList<TimerInfo> TimerList;	// list of TimerInfo structs
static TimerList *timerList	= 0;		// timer list
static EventLoopTimerUPP timerUPP = NULL;       //UPP

/* timer call back */
QMAC_PASCAL static void qt_activate_timers(EventLoopTimerRef, void *data)
{
    EventRef tmr = NULL;
    int t =((TimerInfo *)data)->id;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestTimer, GetCurrentEventTime(),
		kEventAttributeUserEvent, &tmr );
    SetEventParameter(tmr, kEventParamTimer, typeInteger, sizeof(t), &t);
    PostEventToQueue( GetCurrentEventQueue(), tmr, kEventPriorityHigh );
}

//
// Timer initialization and cleanup routines
//
static void initTimers()			// initialize timers
{
    timerUPP = NewEventLoopTimerUPP(qt_activate_timers);
    Q_CHECK_PTR( timerUPP );
    timerList = new TimerList;
    Q_CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
    zero_timer_count = 0;
}

static void cleanupTimers()			// cleanup timer data structure
{
    zero_timer_count = 0;
    if ( timerList ) {
	for( register TimerInfo *t = timerList->first(); t; t = timerList->next() ) {
	    if(t->type == TimerInfo::TIMER_MAC)
		RemoveEventLoopTimer(t->mac_timer);
	    else
		zero_timer_count--;
	}
	delete timerList;
	timerList = 0;
    }
    if(timerUPP) {
	DisposeEventLoopTimerUPP(timerUPP);
	timerUPP = NULL;
    }

}

static int activateNullTimers()
{
    if(!zero_timer_count)
	return 0;

    int ret = 0;
    for(register TimerInfo *t = timerList->first(); ret != zero_timer_count && t; t = timerList->next()) {
	if(t->type == TimerInfo::TIMER_ZERO) {
	    ret++;
	    QTimerEvent e( t->id );
	    QApplication::sendEvent( t->obj, &e );	// send event
	}
    }
    return ret;
}

//
// Main timer functions for starting and killing timers
//
int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    TimerInfo *t = new TimerInfo;		// create timer
    Q_CHECK_PTR( t );
    if(interval == 0) {
    static int serial_id = 666;
	t->mac_timer = NULL;
	t->id = serial_id++;
	zero_timer_count++;
    } else {
	EventTimerInterval mint = (((EventTimerInterval)interval) / 1000);
	if(InstallEventLoopTimer(GetMainEventLoop(), mint, mint, timerUPP, t, &t->mac_timer) ) {

	    delete t;
	    return 0;
	}
	t->id = (int)t->mac_timer;
    }
    t->type = interval ? TimerInfo::TIMER_MAC : TimerInfo::TIMER_ZERO;
    t->obj = obj;
    timerList->append(t);
    return (int)t->id;
}

bool qKillTimer( int id )
{
    if ( !timerList || id <= 0)
	return FALSE;				// not init'd or invalid timer
    register TimerInfo *t = timerList->first();
    while ( t && (t->id != id) ) // find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	if(t->type == TimerInfo::TIMER_MAC)
	    RemoveEventLoopTimer(t->mac_timer);
	else
	    zero_timer_count--;
	return timerList->remove();
    }
    return FALSE; // id not found
}

bool qKillTimer( QObject *obj )
{
    if ( !timerList )				// not initialized
	return FALSE;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    if(t->type == TimerInfo::TIMER_MAC)
		RemoveEventLoopTimer(t->mac_timer);
	    else
		zero_timer_count--;
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}

#ifdef Q_OS_MACX
//socket stuff
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

static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

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
	Q_CHECK_PTR( sn_act_list );
	qAddPostRoutine( sn_cleanup );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(QT_CHECK_RANGE)
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
	    Q_CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	Q_CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
	sn->queue = sn_vec[type].queue;
	if ( list->isEmpty() ) {
	    list->insert( 0, sn );
	} else {				// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(QT_CHECK_STATE)
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
		if ( FD_ISSET( sn->fd, fds ) &&	!FD_ISSET( sn->fd, sn->queue ) ) {
		    sn_act_list->insert( (rand() & 0xff) % (sn_act_list->count()+1), sn );
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
#else
bool qt_set_socket_handler( int, int, QObject *, bool )
{
    return FALSE;
}
//#warning "need to implement sockets on mac9"
#endif

static bool request_select_pending = FALSE;
QMAC_PASCAL void
QApplication::qt_select_timer_callbk(EventLoopTimerRef, void *)
{
    if(request_select_pending)
	return;
    request_select_pending = TRUE;

    EventRef sel = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestSelect, GetCurrentEventTime(),
		kEventAttributeUserEvent, &sel);
    PostEventToQueue( GetCurrentEventQueue(), sel, kEventPriorityStandard );
}

bool QApplication::processNextEvent( bool canWait )
{
    int	   nevents = 0;
    bool broke_early = FALSE;

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif

    if(qt_is_gui_used) {

	if(qt_replay_event) {	//ick
	    EventRef ev = qt_replay_event;
	    qt_replay_event = NULL;
	    SendEventToApplication(ev);
	    ReleaseEvent(ev);
	}

	sendPostedEvents();
	//try to send null timers..
	activateNullTimers();

	/* Where noted (by wtf) we have to hack around brokenness in
	   the OSX RC, hopefully this will be fixed before we ship,
	   otherwise these hacks will have to be left in until we can
	   convince apple to fix it
	*/
	EventRef event;
	OSStatus ret;
	while(GetNumEventsInQueue(GetCurrentEventQueue())) {
	    do {
		ret = ReceiveNextEvent( 0, 0, QMAC_EVENT_NOWAIT, FALSE, &event ); //wtf^3
		if(ret != noErr) {
		    broke_early = TRUE;
		    break;
		}

		//wtf^4: That's right kids, apple is that lame!
		UInt32 ekind = GetEventKind(event), eclass=GetEventClass(event);
		if(0 && eclass == kEventClassMouse && ekind == kEventMouseDown) {
		    EventRecord event_hack;
		    if(WaitNextEvent(mDownMask, &event_hack, 0, NULL))
			nevents++;
		} else {
		    ReceiveNextEvent( 0, 0, QMAC_EVENT_NOWAIT, TRUE, &event );
		    if(SendEventToApplication(event) == noErr)
			nevents++;
		    ReleaseEvent(event);
		}
	    } while(GetNumEventsInQueue(GetCurrentEventQueue()));
	    sendPostedEvents();
	}
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	QMenuBar::macUpdateMenuBar();
#endif
    }

    //if there are no events in the queue, this is a good time to do "stuff"
    sendPostedEvents();
#ifndef QT_NO_CLIPBOARD
    if(qt_clipboard) { //manufacture an event so the clipboard can see if it has changed
	QEvent ev(QEvent::Clipboard);
	QApplication::sendEvent(qt_clipboard, &ev);
    }
#endif

    if( quit_now || app_exit_loop ) {
#if defined(QT_THREAD_SUPPORT)
	qApp->unlock( FALSE );
#endif
	return FALSE;
    }

    if(!broke_early && canWait) {
	EventRef event;
	ReceiveNextEvent( 0, 0, kEventDurationForever, FALSE, &event );
    }

#if defined(QT_THREAD_SUPPORT)
    qApp->unlock( FALSE );
#endif
    return nevents > 0;
}

/* key maps */
#ifdef DEBUG_KEY_MAPS
#define MAP_KEY(x) x, #x
#else
#define MAP_KEY(x) x
#endif

struct key_sym
{
    int mac_code;
    int qt_code;
#ifdef DEBUG_KEY_MAPS
    const char *desc;
#endif
};

static key_sym modifier_syms[] = {
{ shiftKey, MAP_KEY(Qt::ShiftButton) },
{ rightShiftKeyBit, MAP_KEY(Qt::ShiftButton) },
{ controlKey, MAP_KEY(Qt::ControlButton) },
{ rightControlKey, MAP_KEY(Qt::ControlButton) },
{ cmdKey, MAP_KEY(Qt::ControlButton) },
{ optionKey, MAP_KEY(Qt::AltButton) },
{ rightOptionKey, MAP_KEY(Qt::AltButton) },
{   0, MAP_KEY(0) } };
static int get_modifiers(int key)
{
#ifdef DEBUG_KEY_MAPS
    qDebug("**Mapping modifier: %d", key);
#endif
    int ret = 0;
    for(int i = 0; modifier_syms[i].qt_code; i++) {
	if(key & modifier_syms[i].mac_code) {
#ifdef DEBUG_KEY_MAPS
	    qDebug("got modifier: %s", modifier_syms[i].desc);
#endif
	    ret |= modifier_syms[i].qt_code;
	}
    }
    return ret;
}

static key_sym key_syms[] = {
{ kHomeCharCode, MAP_KEY(Qt::Key_Home) },
{ kEnterCharCode, MAP_KEY(Qt::Key_Enter) },
{ kEndCharCode, MAP_KEY(Qt::Key_End) },
{ kBackspaceCharCode, MAP_KEY(Qt::Key_Backspace) },
{ kTabCharCode, MAP_KEY(Qt::Key_Tab) },
{ kPageUpCharCode, MAP_KEY(Qt::Key_PageUp) },
{ kPageDownCharCode, MAP_KEY(Qt::Key_PageDown) },
{ kReturnCharCode, MAP_KEY(Qt::Key_Return) },
{ kEscapeCharCode, MAP_KEY(Qt::Key_Escape) },
{ kLeftArrowCharCode, MAP_KEY(Qt::Key_Left) },
{ kRightArrowCharCode, MAP_KEY(Qt::Key_Right) },
{ kUpArrowCharCode, MAP_KEY(Qt::Key_Up) },
{ kDownArrowCharCode, MAP_KEY(Qt::Key_Down) },
{ kHelpCharCode, MAP_KEY(Qt::Key_Help) },
{ kDeleteCharCode, MAP_KEY(Qt::Key_Delete) },
//ascii maps, for debug
{ '-', MAP_KEY(Qt::Key_hyphen) },
{ ':', MAP_KEY(Qt::Key_Colon) },
{ ';', MAP_KEY(Qt::Key_Semicolon) },
{ '<', MAP_KEY(Qt::Key_Less) },
{ '=', MAP_KEY(Qt::Key_Equal) },
{ '>', MAP_KEY(Qt::Key_Greater) },
{ '?', MAP_KEY(Qt::Key_Question) },
{ '@', MAP_KEY(Qt::Key_At) },
{ ' ', MAP_KEY(Qt::Key_Space) },
{ '!', MAP_KEY(Qt::Key_Exclam) },
{ '"', MAP_KEY(Qt::Key_QuoteDbl) },
{ '#', MAP_KEY(Qt::Key_NumberSign) },
{ '$', MAP_KEY(Qt::Key_Dollar) },
{ '%', MAP_KEY(Qt::Key_Percent) },
{ '&', MAP_KEY(Qt::Key_Ampersand) },
{ '\'', MAP_KEY(Qt::Key_Apostrophe) },
{ '(', MAP_KEY(Qt::Key_ParenLeft) },
{ ')', MAP_KEY(Qt::Key_ParenRight) },
{ '*', MAP_KEY(Qt::Key_Asterisk) },
{ '+', MAP_KEY(Qt::Key_Plus) },
{ ',', MAP_KEY(Qt::Key_Comma) },
{ '-', MAP_KEY(Qt::Key_Minus) },
{ '.', MAP_KEY(Qt::Key_Period) },
{ '/', MAP_KEY(Qt::Key_Slash) },
{ '[', MAP_KEY(Qt::Key_BracketLeft) },
{ ']', MAP_KEY(Qt::Key_BracketRight) },
{ '\\', MAP_KEY(Qt::Key_Backslash) },
{ '_', MAP_KEY(Qt::Key_Underscore) },
{ '`', MAP_KEY(Qt::Key_QuoteLeft) },
{ '{', MAP_KEY(Qt::Key_BraceLeft) },
{ '}', MAP_KEY(Qt::Key_BraceRight) },
{ '|', MAP_KEY(Qt::Key_Bar) },
{ '~', MAP_KEY(Qt::Key_AsciiTilde) },
//function keys?
#if 0
{ 32, MAP_KEY(Qt::Key_F1) },
{ 33, MAP_KEY(Qt::Key_F2) },
{ 34, MAP_KEY(Qt::Key_F3) },
{ 35, MAP_KEY(Qt::Key_F4) },
{ 36, MAP_KEY(Qt::Key_F5) },
{ 37, MAP_KEY(Qt::Key_F6) },
{ 38, MAP_KEY(Qt::Key_F7) },
{ 39, MAP_KEY(Qt::Key_F8) },
{ 40, MAP_KEY(Qt::Key_F9) },
{ 41, MAP_KEY(Qt::Key_F10) },
{ 42, MAP_KEY(Qt::Key_F11) },
{ 43, MAP_KEY(Qt::Key_F12) },
#endif
//terminator
{   0, MAP_KEY(0) } };
static int get_key(int key)
{
#ifdef DEBUG_KEY_MAPS
    qDebug("**Mapping key: %d", key);
#endif
    for(int i = 0; key_syms[i].qt_code; i++) {
	if(key_syms[i].mac_code == key) {
#ifdef DEBUG_KEY_MAPS
	    qDebug("got key: %s", key_syms[i].desc);
#endif
	    return key_syms[i].qt_code;
	}
    }

    //general cases..
    if(key >= '0' && key <= '9') {
#ifdef DEBUG_KEY_MAPS
	qDebug("General case Qt::Key_%c", key);
#endif
	return (key - '0') + Qt::Key_0;
    }
    char tup = toupper(key);
    if(tup >= 'A' && tup <= 'Z') {
#ifdef DEBUG_KEY_MAPS
	qDebug("General case Qt::Key_%c %d", tup, (tup - 'A') + Qt::Key_A);
#endif
	return (tup - 'A') + Qt::Key_A;
    }

#ifdef DEBUG_KEY_MAPS
    qDebug("Unknown case.. %s:%d %d", __FILE__, __LINE__, key);
#endif
    return Qt::Key_unknown;
}


bool QApplication::do_mouse_down( Point *pt )
{
    WindowPtr wp;
    short windowPart;
    windowPart = FindWindow( *pt, &wp );
    QWidget *widget = QWidget::find( (WId)wp );
    bool in_widget = FALSE;

    switch( windowPart ) {
    case inGoAway:
	if( widget ) {
	    widget->close();
	} else {
	    qWarning("Close for unknown widget");
	}
	break;
    case inDrag:
    {
	    DragWindow( wp, *pt, 0 );
	    QPoint np, op(widget->crect.x(), widget->crect.y());    
	    {
	        QMacSavedPortInfo savedInfo(widget);
	        Point p = { 0, 0 };
	        LocalToGlobal(&p);
	        np = QPoint(p.h, p.v);
	    }
	    if(np != op) {
	        widget->crect = QRect( np, widget->crect.size());
	        QMoveEvent qme( np, op);
	    }
	}
	break;
    case inContent:
	in_widget = TRUE;
	break;
    case inGrow:
    {
	Rect limits;
	if( widget ) {
	    if ( widget->extra ) {
		SetRect( &limits, widget->extra->minw, widget->extra->minh,
			 widget->extra->maxw, widget->extra->maxh);
	    }
	}
	int growWindowSize = GrowWindow( wp, *pt, &limits);
	if( growWindowSize) {
	    // nw/nh might not match the actual size if setSizeIncrement is used
	    int nw = LoWord( growWindowSize );
	    int nh = HiWord( growWindowSize );
	    if(nw != widget->width() || nh != widget->height()) {
	        if( nw < desktop()->width() && nw > 0 && nh < desktop()->height() && nh > 0 && widget) 
		        widget->resize(nw, nh);
		}
	}
	break;
    }
    case inCollapseBox:
	if( TrackBox( wp, *pt, windowPart ) == true ) {
	    if(widget)
		widget->showMinimized();
	}
	break;
    case inZoomIn:
	if( TrackBox( wp, *pt, windowPart ) == true ) {
	    if(widget)
		widget->showNormal();
	}
	break;
    case inZoomOut:
	if( TrackBox( wp, *pt, windowPart ) == true ) {
	    if(widget)
		widget->showMaximized();
	}
	break;
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
    case inMenuBar:
	MenuSelect(*pt); //allow menu tracking
	break;
#endif
    default:
	qDebug("Unhandled case in mouse_down.. %d", windowPart);
	break;
    }
    return in_widget;
}

void QApplication::wakeUpGuiThread()
{
}

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	Q_CHECK_PTR( qt_modal_stack );
    }
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	}
    }
    app_do_modal = qt_modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, EventRef event )
{
   if ( qApp->activePopupWidget() )
	return TRUE;
    // a bit of a hack: use WStyle_Tool as a general ignore-modality
    // flag, also for complex widgets with children.
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=QApplication::activeModalWidget();

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )			// don't block event
	return TRUE;

    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader ) )
	groupLeader = groupLeader->parentWidget();

    if ( groupLeader ) {
	// Does groupLeader have a child in qt_modal_stack?
	bool unrelated = TRUE;
	modal = qt_modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != groupLeader && !p->testWFlags( Qt::WGroupLeader) ) {
		p = p->parentWidget();
	    }
	    modal = qt_modal_stack->next();
	    if ( p == groupLeader ) unrelated = FALSE;
	}

	if ( unrelated )
	    return TRUE;		// don't block event
    }

    bool block_event  = FALSE;
    bool paint_event = FALSE;

    UInt32 ekind = GetEventKind(event), eclass=GetEventClass(event);
    switch(eclass) {
    case kEventClassMouse:
	block_event = ekind != kEventMouseMoved;
	break;
    case kEventClassKeyboard:
	block_event = TRUE;
	break;
    case kEventClassWindow:
	paint_event = ekind == kEventWindowUpdate;
	break;
    }

    if ( !top->parentWidget() && (block_event || paint_event) )
	top->raise();

    return !block_event;
}

//context menu hack
static EventLoopTimerRef mac_trap_context = NULL;
static bool request_context_pending = FALSE;
QMAC_PASCAL void
QApplication::qt_trap_context_mouse(EventLoopTimerRef r, void *d)
{
    QWidget *w = (QWidget *)d;
    EventLoopTimerRef otc = mac_trap_context;
    RemoveEventLoopTimer(mac_trap_context);
    mac_trap_context = NULL;
    if(r != otc || w != qt_button_down || request_context_pending)
	return;
    request_context_pending = TRUE;

    EventRef ctx = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestContext, GetCurrentEventTime(),
		kEventAttributeUserEvent, &ctx );
    SetEventParameter(ctx, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue( GetCurrentEventQueue(), ctx, kEventPriorityStandard );
}

QMAC_PASCAL OSStatus
QApplication::globalEventProcessor(EventHandlerCallRef, EventRef event, void *data)
{
    bool remove_context_timer = TRUE;
    QApplication *app = (QApplication *)data;
    if ( app->macEventFilter( event ) )
	return 1;
    QWidget *widget = NULL;

    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass)
    {
    case kEventClassQt:
	remove_context_timer = FALSE;
	if(ekind == kEventQtRequestPropagate) {
	    request_updates_pending = FALSE;
	    QApplication::sendPostedEvents();
	    if(QWidgetList *list   = qApp->topLevelWidgets()) {
		for ( QWidget     *widget = list->first(); widget; widget = list->next() ) {
		    if ( !widget->isHidden() && !widget->isDesktop())
			widget->propagateUpdates();
		}
		delete list;
	    }
	} else if(ekind == kEventQtRequestSelect) {
	    request_select_pending = FALSE;
	    if ( qt_preselect_handler ) {
		QVFuncList::Iterator end = qt_preselect_handler->end();
		for ( QVFuncList::Iterator it = qt_preselect_handler->begin();
		      it != end; ++it )
		    (**it)();
	    }
#ifdef Q_OS_MACX
	    timeval tm;
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
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
	    int nsel = select( sn_highest + 1, (&app_readfds), (sn_write  ? &app_writefds  : 0),
			       (sn_except ? &app_exceptfds : 0), &tm );
#else
//#warning "need to implement sockets on mac9"
#endif

	    if ( qt_postselect_handler ) {
		QVFuncList::Iterator end = qt_postselect_handler->end();
		for ( QVFuncList::Iterator it = qt_postselect_handler->begin();
		      it != end; ++it )
		    (**it)();
	    }

#ifdef Q_OS_MACX
	    if ( nsel == -1 ) {
		if ( errno == EINTR || errno == EAGAIN ) {
		    errno = 0;
		}
	    } else if ( nsel > 0 && sn_highest >= 0 ) {
		qt_event_request_updates();
		sn_activate();
	    }
#else
//#warning "need to implement sockets on mac9"
#endif
	} else if(ekind == kEventQtRequestContext) {
	    request_context_pending = FALSE;
	    //figure out which widget to send it to
	    QPoint where = QCursor::pos();
	    QWidget *widget = NULL;
	    GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
			      sizeof(widget), NULL, &widget);
	    if(!widget) {
		if( qt_button_down )
		    widget = qt_button_down;
		else
		    widget = QApplication::widgetAt( where.x(), where.y(), true );
	    }
	    if ( widget ) {
		QPoint plocal(widget->mapFromGlobal( where ));
		QContextMenuEvent qme( QContextMenuEvent::Mouse, plocal, where, 0 );
		QApplication::sendEvent( widget, &qme );
		if(qme.isAccepted()) { //once this happens the events before are pitched
		    qt_button_down = NULL;
		    mouse_button_state = 0;
		}
	    }
	} else if(ekind == kEventQtRequestTimer) {
	    if(!timerList)
		break;
	    int id = 0;
	    GetEventParameter(event, kEventParamTimer, typeInteger, NULL, sizeof(id), NULL, &id);
	    register TimerInfo *t = timerList->first();
	    while ( t && (t->id != id) ) // find timer info in list
		t = timerList->next();
	    if ( t ) {					// id found
		QTimerEvent e( id );
		QApplication::sendEvent( t->obj, &e );	// send event
	    }
	}
	break;
    case kEventClassMouse:
    {
	if( (ekind == kEventMouseDown && mouse_button_state ) ||
	    (ekind == kEventMouseUp && !mouse_button_state) ) {
#ifdef DEBUG_MOUSE_MAPS
	    qDebug("Dropping mouse event..");
#endif
	    return 0;
	}

	QEvent::Type etype = QEvent::None;
	int keys;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL,
			  sizeof(keys), NULL, &keys);
	keys = get_modifiers(keys);
	int button=QEvent::NoButton, state=0, wheel_delta=0, after_state=mouse_button_state;
	if(ekind == kEventMouseDown || ekind == kEventMouseUp) {
	    EventMouseButton mb;
	    GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL,
			      sizeof(mb), NULL, &mb);

	    if(mb == kEventMouseButtonPrimary)
		button = QMouseEvent::LeftButton;
	    else if(mb == kEventMouseButtonSecondary)
		button = QMouseEvent::RightButton;
	    else
		button = QMouseEvent::MidButton;
	}

	Point where;
	GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL,
			  sizeof(where), NULL, &where);

	switch(ekind) {
	case kEventMouseDown:
	{
	    UInt32 count;
	    GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL,
			      sizeof(count), NULL, &count);
	    if(count == 2)
		etype = QEvent::MouseButtonDblClick;
	    else
		etype = QEvent::MouseButtonPress;
	    after_state  = button;
	    break;
	}
	case kEventMouseUp:
	    etype = QEvent::MouseButtonRelease;
	    state = after_state;
	    after_state = 0;
	    break;
	case kEventMouseDragged:
	case kEventMouseMoved:
	    etype = QEvent::MouseMove;
	    state = after_state;
	    break;
	case kEventMouseWheelMoved:
	{
	    long int mdelt;
	    GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, NULL,
			      sizeof(mdelt), NULL, &mdelt);
	    wheel_delta = mdelt * 100;
	    state = after_state;
	    break;
	}
	}

	//handle popup's first
	QWidget *popupwidget = NULL;
	bool special_close = FALSE;
	if( app->inPopupMode() ) {
	    qt_closed_popup = FALSE;

	    WindowPtr wp;
	    FindWindow(where,&wp);
	    if(wp) {
		QWidget *clt=QWidget::find((WId)wp);
		if(clt && clt->isPopup())
		    popupwidget = clt;
	    }
	    if(!popupwidget)
		popupwidget = activePopupWidget();
	    QMacSavedPortInfo savedInfo(popupwidget);
	    Point gp = where;
	    GlobalToLocal( &gp ); //now map it to the window
	    popupwidget = recursive_match(popupwidget, gp.h, gp.v);

	    QPoint p( where.h, where.v );
	    QPoint plocal(popupwidget->mapFromGlobal( p ));
	    bool was_context = FALSE;
	    if(etype == QEvent::MouseButtonPress && 	
	       ((button == QMouseEvent::RightButton) ||
		(button == QMouseEvent::LeftButton && (keys & Qt::ControlButton)))) {
		QContextMenuEvent cme(QContextMenuEvent::Mouse, plocal, p, keys );
		QApplication::sendEvent( popupwidget, &cme );
		was_context = cme.isAccepted();
	    }
	    if(!was_context) {
		if(wheel_delta) {
		    QWheelEvent qwe( plocal, p, wheel_delta, state | keys);
		    QApplication::sendEvent( popupwidget, &qwe);
		} else {
		    QMouseEvent qme( etype, plocal, p, button | keys, state | keys );
		    QApplication::sendEvent( popupwidget, &qme );
		}
		if(etype == QEvent::MouseButtonPress && app->activePopupWidget() != popupwidget &&
		   qt_closed_popup)
		    special_close = TRUE;
	    }
	}

	if(ekind == kEventMouseDown && !app->do_mouse_down( &where ))
	    return 0;

	mouse_button_state = after_state;
	if(special_close) {
	    qt_replay_event = CopyEvent(event);
	    return 0;
	}

	//figure out which widget to send it to
	if( ekind != kEventMouseDown && qt_button_down )
	    widget = qt_button_down;
	else if( mac_mouse_grabber )
	    widget = mac_mouse_grabber;
	else
	    widget = QApplication::widgetAt( where.h, where.v, true );

	if ( widget && app_do_modal && !qt_try_modal(widget, event) )
	    return 1;

	switch(ekind) {
	case kEventMouseDragged:
	case kEventMouseMoved:
	{
	    //set the cursor up
	    Cursor *n = NULL;
	    if(!widget) //not over the app, don't set a cursor..
		;
	    else if(widget->extra && widget->extra->curs)
		n = (Cursor *)widget->extra->curs->handle();
	    else if(cursorStack)
		n = (Cursor *)app_cursor->handle();
	    if(!n)
		n = (Cursor *)arrowCursor.handle(); //I give up..
	    if(currentCursor != n)
		SetCursor(currentCursor = n);
	    if ( qt_mouseover != widget ) {
		qt_dispatchEnterLeave( widget, qt_mouseover );
		qt_mouseover = widget;
	    }
	    break;
	}
	case kEventMouseDown:
	    if(button == QMouseEvent::LeftButton && !mac_trap_context) {
		remove_context_timer = FALSE;
		InstallEventLoopTimer(GetMainEventLoop(), 2, 0,
				      NewEventLoopTimerUPP(qt_trap_context_mouse), widget,
				      &mac_trap_context);
	    }
	    qt_button_down = widget;
	    break;
	case kEventMouseUp:
	    qt_button_down = NULL;
	    break;
	}

	//finally send the event to the widget if its not the popup
	if ( widget && widget != popupwidget ) {
	    if(ekind == kEventMouseDown) {
		QWidget* w = widget;
		while ( w->focusProxy() )
		    w = w->focusProxy();
		if(QWidget *tlw = w->topLevelWidget()) {
		    tlw->raise();
		    if(tlw->isTopLevel() && !tlw->isPopup() &&
		       (tlw->isModal() || !tlw->isDialog()))
			app->setActiveWindow(tlw);
		}
		if ( w->focusPolicy() & QWidget::ClickFocus ) {
		    QFocusEvent::setReason( QFocusEvent::Mouse);
		    w->setFocus();
		    QFocusEvent::resetReason();
		}
	    } else if(ekind == kEventMouseWheelMoved) {
		QWidget* w = widget;
		while( w->focusProxy() )
		    w = w->focusProxy();
		if( w->focusPolicy() & QWidget::WheelFocus ) {
		    QFocusEvent::setReason( QFocusEvent::Mouse );
		    w->setFocus();
		    QFocusEvent::resetReason();
		}
	    }

	    QPoint p( where.h, where.v );
	    QPoint plocal(widget->mapFromGlobal( p ));
	    bool was_context = FALSE;
	    if(etype == QEvent::MouseButtonPress && 	
	       ((button == QMouseEvent::RightButton) ||
		(button == QMouseEvent::LeftButton && (keys & Qt::ControlButton)))) {
		QContextMenuEvent cme(QContextMenuEvent::Mouse, plocal, p, keys );
		QApplication::sendEvent( widget, &cme );
		was_context = cme.isAccepted();
	    }
	    if(!was_context) {
#ifdef DEBUG_MOUSE_MAPS
		char *desc = NULL;
		switch(ekind) {
		case kEventMouseDown: desc = "MouseButtonPress"; break;
		case kEventMouseUp: desc = "MouseButtonRelease"; break;
		case kEventMouseDragged:
		case kEventMouseMoved: desc = "MouseMove"; break;
		case kEventMouseWheelMoved: desc = "MouseWheelMove"; break;
		}
		qDebug("%d %d - Would send (%s) event to %s %s (%d %d %d)", p.x(), p.y(), desc,
		       widget->name(), widget->className(), button|keys, state|keys,
		       wheel_delta);
#endif
		if(wheel_delta) {
		    QWheelEvent qwe( plocal, p, wheel_delta, state | keys);
		    QApplication::sendEvent( widget, &qwe);
		} else {
#ifdef QMAC_SPEAK_TO_ME
		    if(etype == QMouseEvent::MouseButtonDblClick && (keys & Qt::AltButton)) {
			QVariant v = widget->property("text");
			if(!v.isValid()) v = widget->property("caption");
			if(v.isValid()) {
			    QString s = v.toString();
			    s.replace(QRegExp("(\\&|\\<[^\\>]*\\>)"), "");
			    SpeechChannel ch;
			    NewSpeechChannel(NULL, &ch);
			    SpeakText(ch, s.latin1(), s.length());
			}
		    }
#endif
		    QMouseEvent qme( etype, plocal, p, button | keys, state | keys );
		    QApplication::sendEvent( widget, &qme );
		}
	    }
	}
	break;
    }
    case kEventClassKeyboard:
    {
	UInt32 modif;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modif), NULL, &modif);
	int modifiers = get_modifiers(modif);

	UInt32 keyc;
	GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyc), NULL, &keyc);
	UInt32 state = 0L;
	char chr = KeyTranslate((void *)GetScriptManagerVariable(smKCHRCache),
				(modif & shiftKey) | keyc, &state);
	int mychar=get_key(chr);
	QString mystr = QChar(chr);

	QEvent::Type etype = (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress;

	if( mac_keyboard_grabber )
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;

	if(widget) {
	    if ( app_do_modal && !qt_try_modal(widget, event) )
		return 1;

	    bool isAccel = FALSE;
	    if(etype == QEvent::KeyPress && !mac_keyboard_grabber) {
		QKeyEvent aa(QEvent::AccelOverride, mychar, chr, modifiers, mystr, ekind == kEventRawKeyRepeat,
			     mystr.length());
		aa.ignore();
		QApplication::sendEvent( widget, &aa );
		if ( !aa.isAccepted() ) {
		    QKeyEvent a(QEvent::Accel, mychar, chr, modifiers, mystr, ekind == kEventRawKeyRepeat,
				mystr.length());
		    a.ignore();
		    QApplication::sendEvent( widget->topLevelWidget(), &a );
		    if ( a.isAccepted() )
			isAccel = TRUE;
#if !defined(QMAC_QMENUBAR_NO_NATIVE) //In native menubar mode we offer the event to the menubar...
		    if( !isAccel ) {
			MenuRef menu;
			MenuItemIndex idx;
			if(IsMenuKeyEvent(NULL, event, kNilOptions, &menu, &idx)) {
			    QMenuBar::activate(menu, idx);
			    isAccel = TRUE;
			}
		    }
#endif
		}
	    }
	    if(!isAccel) {
		if((modifiers & (Qt::ControlButton | Qt::AltButton)) || (mychar > 127 || mychar < 0)) {
		    mystr = QString();
		    chr = 0;
		}

		QKeyEvent ke(etype,mychar, chr, modifiers, mystr, ekind == kEventRawKeyRepeat, mystr.length());
		QApplication::sendEvent(widget,&ke);
	    }
	}
	break;
    }
    case kEventClassWindow:
    {
	WindowRef wid;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
			  sizeof(WindowRef), NULL, &wid);
	widget = QWidget::find( (WId)wid );

	if(!widget) {
	    if(ekind == kEventWindowShown )
		unhandled_dialogs.insert((void *)wid, (void *)1);
	    else if(ekind == kEventWindowHidden)
		unhandled_dialogs.remove((void *)wid);		
	    else
		qWarning("Couldn't find EventClassWindow widget for %d", (int)wid);
	    break;
	}

	if(ekind == kEventWindowUpdate) {
	    remove_context_timer = FALSE;
	    widget->propagateUpdates();
	} else if(ekind == kEventWindowBoundsChanged) {
	    UInt32 flags;
	    GetEventParameter(event, kEventParamAttributes, typeUInt32, NULL, sizeof(flags), NULL, &flags);
	    Rect nr;
	    GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(nr), NULL, &nr);
	    if((flags & kWindowBoundsChangeOriginChanged)) {
		int ox = widget->crect.x(), oy = widget->crect.y();
		int nx = nr.left, ny = nr.top;
		widget->crect.setRect( nx, ny, widget->width(), widget->height() );
		QMoveEvent qme( widget->crect.topLeft(), QPoint( ox, oy) );
		QApplication::sendEvent( widget, &qme );
	    } 
	    if((flags & kWindowBoundsChangeSizeChanged)) {
		// nw/nh might not match the actual size if setSizeIncrement is used
		int nw = nr.right - nr.left, nh = nr.bottom - nr.top;
		widget->resize(nw, nh);
		if(widget->isVisible())
		    widget->propagateUpdates();
	    }
	} else if(ekind == kEventWindowActivated) {
	    if(widget) {
		widget->raise();
		QWidget *tlw = widget->topLevelWidget();
		if(tlw->isTopLevel() && !tlw->isPopup() && (tlw->isModal() || !tlw->isDialog()))
		    app->setActiveWindow(tlw);
		if (widget->focusWidget())
		    widget->focusWidget()->setFocus();
		else
		    widget->setFocus();
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
		QMenuBar::macUpdateMenuBar();
#endif
	    }
	} else if(ekind == kEventWindowDeactivated) {
	    if(active_window && widget == active_window)
		app->setActiveWindow(NULL);
	    while(app->inPopupMode())
		app->activePopupWidget()->close();
	}
	break;
    case kEventClassApplication:
	if(ekind == kEventAppActivated)
	    app->clipboard()->loadScrap(FALSE);
	else if(ekind == kEventAppDeactivated) {
	    app->clipboard()->saveScrap();
	    app->setActiveWindow(NULL);
	}
	break;

    case kEventClassMenu:
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	if(ekind == kEventMenuOpening) {
	    Boolean first;
	    GetEventParameter(event, kEventParamMenuFirstOpen, typeBoolean, NULL, sizeof(first), NULL, &first);
	    if(first) {
		MenuRef mr;
		GetEventParameter(event, kEventParamDirectObject, typeMenuRef, NULL, sizeof(mr), NULL, &mr);
		QMenuBar::macUpdatePopup(mr);
	    }
	} else if(ekind == kEventMenuTargetItem) {
	    MenuRef mr;
	    GetEventParameter(event, kEventParamDirectObject, typeMenuRef, NULL, sizeof(mr), NULL, &mr);
	    MenuItemIndex idx;
	    GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex, NULL, sizeof(idx), NULL, &idx);
	    QMenuBar::activate(mr, idx, TRUE);
	}
#endif
	break;
    case kEventClassCommand:
	if(ekind == kEventCommandProcess) {
	    HICommand cmd;
	    GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(cmd), NULL, &cmd);
	    if(cmd.commandID == kHICommandQuit)
		qApp->closeAllWindows();
#if !defined(QMAC_QMENUBAR_NO_NATIVE) //offer it to the menubar..
	    else
		QMenuBar::activate(cmd.menu.menuRef, cmd.menu.menuItemIndex);
#endif
	}
	break;
    }
    }

    if(remove_context_timer) {
	if(mac_trap_context) {
	    RemoveEventLoopTimer(mac_trap_context);
	    mac_trap_context = NULL;
	}
	if(request_context_pending) {
	    request_context_pending = FALSE;
	    EventRef er;
	    const EventTypeSpec eventspec = { kEventClassQt, kEventQtRequestContext };
	    while(1) {
		OSStatus ret = ReceiveNextEvent( 1, &eventspec, QMAC_EVENT_NOWAIT, TRUE, &er );
		if(ret == eventLoopTimedOutErr || ret == eventLoopQuitErr)
		    break;
		ReleaseEvent(er);
	    }
	}
    }
    return noErr;
}

void QApplication::processEvents( int maxtime)
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !quit_now && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}

extern uint qGlobalPostedEventsCount();

bool QApplication::hasPendingEvents()
{
    return qGlobalPostedEventsCount() || GetNumEventsInQueue(GetCurrentEventQueue());
}

bool QApplication::macEventFilter( EventRef )
{
    return 0;
}



void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	Q_CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
    }
    popupWidgets->append( popup );		// add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    active_window = popup;
    QFocusEvent::setReason( QFocusEvent::Popup );
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

    qt_closed_popup = !popup->geometry().contains( QCursor::pos() );
    if (popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	// restore the former active window immediately, although
	// we'll get a focusIn later
	active_window = (*activeBeforePopup);
	QFocusEvent::setReason( QFocusEvent::Popup );
	if ( active_window )
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	QFocusEvent::resetReason();
    } else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	active_window = popupWidgets->getLast();
	QFocusEvent::setReason( QFocusEvent::Popup );
	if (active_window->focusWidget())
	    active_window->focusWidget()->setFocus();
	else
	    active_window->setFocus();
	QFocusEvent::resetReason();
    }
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


//FIXME: What is the default value on the Mac?
int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}


void QApplication::setWheelScrollLines( int n )
{
    wheel_scroll_lines = n;
}

int QApplication::wheelScrollLines()
{
    return wheel_scroll_lines;
}

void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    switch (effect) {
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeMenu:
	if ( enable )
	    animate_menu = TRUE;
	fade_menu = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_FadeTooltip:
	if ( enable )
	    animate_tooltip = TRUE;
	fade_tooltip = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }
}

bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( !animate_ui )
	return FALSE;

    switch( effect ) {
    case UI_AnimateMenu:
	return animate_menu;
    case UI_FadeMenu:
	return fade_menu;
    case UI_AnimateCombo:
	return animate_combo;
    case UI_AnimateTooltip:
	return animate_tooltip;
    case UI_FadeTooltip:
	return fade_tooltip;
    default:
	return animate_ui;
    }
}


#if 0
#include <stdlib.h>
void* operator new[](size_t size) { return malloc(size); }
void* operator new(size_t size) { return malloc(size); }
void operator delete[](void *p) { free(p); }
void operator delete[](void *p, size_t) { free(p); }
void operator delete(void *p) { free(p); }
pvoid operator delete(void *p, size_t) { free(p); }
#endif

/****************************************************************************
** $Id$
**
** Implementation of Mac startup routines and event handling
**
** Created : 001018
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
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

// NOT REVISED
#include "qglobal.h"
#include "qt_mac.h"

#include "qapplication.h"
#include "private/qapplication_p.h"
#include "private/qcolor_p.h"
#include "qwidget.h"
#include "private/qwidget_p.h"
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
#include <qsettings.h>
#include <qstylefactory.h>
#include <qstyle.h>

//#define QMAC_LAME_TIME_LIMITED
#ifdef QMAC_LAME_TIME_LIMITED
#  include <qtimer.h>
#  include <qmessagebox.h>
#endif

#if !defined(QMAC_QMENUBAR_NO_NATIVE)
#  include "qmenubar.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>


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
#  define QMAC_EVENT_NOWAIT 0.01
#else
#  define QMAC_EVENT_NOWAIT kEventDurationNoWait
#endif

#ifdef Q_WS_MACX
#  include <sys/time.h>
#  include <sys/select.h>
#  include <unistd.h>
#  include <qdir.h>
#elif defined(Q_WS_MAC9)
typedef int timeval;
#  include <ctype.h>
#endif

#if defined(QT_THREAD_SUPPORT)
#  include "qthread.h"
#endif

//for qt_mac.h
QPaintDevice *qt_mac_safe_pdev = 0;
#ifdef QT_THREAD_SUPPORT
QMutex *qt_mac_port_mutex = NULL;
#endif

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static struct {
    int last_modifiers, last_button;
    EventTime last_time;
    bool active, use_qt_time_limit;
} qt_mac_dblclick = { 0, 0, -2, 0, 0 };
static int mouse_button_state = 0;
static int keyboard_modifiers_state = 0;
static bool	app_do_modal	= FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;		// stack of modal widgets
extern bool qt_mac_in_drag; //qdnd_mac.cpp
extern bool qt_resolve_symlinks; // from qapplication.cpp
static char    *appName;                        // application name
QGuardedPtr<QWidget> qt_button_down;		// widget got last button-down
static QGuardedPtr<QWidget> qt_mouseover;
static QPtrDict<void> unhandled_dialogs;        //all unhandled dialogs (ie mac file dialog)
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif
#if defined(Q_OS_MACX)
static int qt_thread_pipe[2];
#endif
static EventLoopTimerRef mac_context_timer = NULL;
static EventLoopTimerUPP mac_context_timerUPP = NULL;
static EventLoopTimerRef mac_select_timer = NULL;
static EventLoopTimerUPP mac_select_timerUPP = NULL;
static DMExtendedNotificationUPP mac_display_changeUPP = NULL;
static EventHandlerRef app_proc_handler = NULL;
static EventHandlerUPP app_proc_handlerUPP = NULL;
//popup variables
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
//timer stuff
static void	initTimers();
static void	cleanupTimers();

/*****************************************************************************
  External functions
 *****************************************************************************/
// Paint event clipping magic - qpainter_mac.cpp
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping(QPaintDevice *dev);
extern void qt_mac_set_cursor(const QCursor *, const Point *); //Cursor switching - qcursor_mac.cpp
QCString p2qstring(const unsigned char *); //qglobal.cpp

//special case popup handlers - look where these are used, they are very hacky,
//and very special case, if you plan on using these variables be VERY careful!!
static bool qt_closed_popup = FALSE;
static EventRef qt_replay_event = NULL;

static QMAC_PASCAL void qt_mac_display_change_callbk(void *, SInt16 msg, void *)
{
    if(msg == kDMNotifyEvent) {
	if(QDesktopWidget *dw = qApp->desktop())
	    QApplication::postEvent(dw, new QResizeEvent(dw->size(), dw->size()));
    }
}

static short qt_mac_find_window( int x, int y, QWidget **w=NULL )
{
    Point p;
    p.h = x;
    p.v = y;
    WindowPtr wp;
    short ret = FindWindow(p, &wp);
#ifndef QMAC_NO_FAKECURSOR
    if(wp && !unhandled_dialogs.find((void *)wp)) {
	QWidget *tmp_w = QWidget::find((WId)wp);
	if(tmp_w && !strcmp(tmp_w->className(),"QMacCursorWidget")) {
	    tmp_w->hide();
	    ret = qt_mac_find_window(x, y, w);
	    tmp_w->show();
	    return ret;
	}
    }
#endif
    if(w) {
	if(wp && !unhandled_dialogs.find((void *)wp)) {
	    *w = QWidget::find((WId)wp);
	    if(!*w)
		qWarning("qt_mac_find_window: Couldn't find %d",(int)wp);
	} else {
	    *w = NULL;
	}
    }
    return ret;
}

bool qt_nograb()				// application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}
void qt_mac_clear_mouse_state()
{
    mouse_button_state = 0;
    qt_button_down = 0;
}


//pre/post select callbacks
typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
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

/* Event masks */

// internal Qt types
const UInt32 kEventClassQt = 'cute';
enum {
    //types
    typeQWidget = 1,  /* QWidget *  */
    typeTimerInfo = 2, /* TimerInfo * */
    //params
    kEventParamTimer = 'qtim',     /* typeTimerInfo */
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    //events
    kEventQtRequestPropagateWindowUpdates = 10,
    kEventQtRequestPropagateWidgetUpdates = 11,
    kEventQtRequestSelect = 12,
    kEventQtRequestContext = 13,
#ifndef QMAC_QMENUBAR_NO_NATIVE
    kEventQtRequestMenubarUpdate = 14,
#endif
    kEventQtRequestTimer = 15,
    kEventQtRequestWakeup = 16
};
static bool request_updates_pending = FALSE;
void qt_event_request_updates()
{
    if(request_updates_pending)
	return;
    request_updates_pending = TRUE;

    EventRef upd = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestPropagateWindowUpdates, 
		GetCurrentEventTime(), kEventAttributeUserEvent, &upd);
    PostEventToQueue(GetMainEventQueue(), upd, kEventPriorityHigh);
    ReleaseEvent(upd);
}
static QValueList<WId> request_updates_pending_list;
void qt_event_request_updates(QWidget *w, const QRegion &r, bool subtract)
{
    w->createExtra();
    if(subtract) {
	if(w->extra->has_dirty_area) {
	    w->extra->dirty_area -= r;
	    if(w->extra->dirty_area.isEmpty()) {
		request_updates_pending_list.remove(w->winId());
		w->extra->has_dirty_area = FALSE;
	    }
	}
	return;
    } else if(w->extra->has_dirty_area) {
	w->extra->dirty_area |= r;	
	return;
    }
    w->extra->has_dirty_area = TRUE;
    w->extra->dirty_area = r;
    //now maintain the list of widgets to be updated
    if(request_updates_pending_list.isEmpty()) {
	EventRef upd = NULL;
	CreateEvent(NULL, kEventClassQt, kEventQtRequestPropagateWidgetUpdates, 
		    GetCurrentEventTime(), kEventAttributeUserEvent, &upd);
	PostEventToQueue(GetMainEventQueue(), upd, kEventPriorityStandard);
	ReleaseEvent(upd);
    }
    request_updates_pending_list.append(w->winId());
}

#ifndef QMAC_QMENUBAR_NO_NATIVE
static bool request_menubarupdate_pending = FALSE;
void qt_event_request_menubarupdate()
{
    if(request_menubarupdate_pending)
	return;
    request_menubarupdate_pending = TRUE;

    EventRef upd = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestMenubarUpdate, GetCurrentEventTime(),
		kEventAttributeUserEvent, &upd);
    PostEventToQueue(GetMainEventQueue(), upd, kEventPriorityHigh);
    ReleaseEvent(upd);
}
#endif

static EventTypeSpec events[] = {
    { kEventClassQt, kEventQtRequestTimer },
    { kEventClassQt, kEventQtRequestWakeup },
    { kEventClassQt, kEventQtRequestSelect },
    { kEventClassQt, kEventQtRequestContext },
    { kEventClassQt, kEventQtRequestMenubarUpdate },
    { kEventClassQt, kEventQtRequestPropagateWindowUpdates },
    { kEventClassQt, kEventQtRequestPropagateWidgetUpdates },

    { kEventClassWindow, kEventWindowUpdate },
    { kEventClassWindow, kEventWindowActivated },
    { kEventClassWindow, kEventWindowDeactivated },
    { kEventClassWindow, kEventWindowShown },
    { kEventClassWindow, kEventWindowHidden },
    { kEventClassWindow, kEventWindowBoundsChanged },

    { kEventClassMouse, kEventMouseWheelMoved },
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseMoved },

    { kEventClassApplication, kEventAppActivated },
    { kEventClassApplication, kEventAppDeactivated },

    { kEventClassMenu, kEventMenuOpening },
    { kEventClassMenu, kEventMenuTargetItem },

    { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
    { kEventClassKeyboard, kEventRawKeyModifiersChanged },
    { kEventClassKeyboard, kEventRawKeyRepeat },
    { kEventClassKeyboard, kEventRawKeyUp },
    { kEventClassKeyboard, kEventRawKeyDown },

    { kEventClassCommand, kEventCommandProcess },
    { kEventClassAppleEvent, kEventAppleEvent },
    { kAppearanceEventClass, kAEAppearanceChanged }
};

/* platform specific implementations */
void qt_init(int* argcptr, char **argv, QApplication::Type)
{
    if(qt_is_gui_used) {
	ProcessSerialNumber psn;
	if(GetCurrentProcess(&psn) == noErr) {
	    if(!mac_display_changeUPP) {
		mac_display_changeUPP = NewDMExtendedNotificationUPP(qt_mac_display_change_callbk);
		DMRegisterExtendedNotifyProc( mac_display_changeUPP, NULL, 0, &psn);
	    }
#ifdef Q_WS_MACX
	    SetFrontProcess(&psn);
#endif
	}
    }

    // Get command line params
    int argc = *argcptr;
    int i, j = 1;
    for(i=1; i<argc; i++) {
	if(argv[i] && *argv[i] != '-') {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
#if defined(QT_DEBUG)
	if (arg == "-nograb")
	    appNoGrab = !appNoGrab;
	else
#endif // QT_DEBUG
#ifdef Q_WS_MACX
	//just ignore it, this seems to be passed from the finder (no clue what it does) FIXME
	    if(arg.left(5) == "-psn_"); 
	else
#endif
	    argv[j++] = argv[i];
    }
    *argcptr = j;

    // Set application name
    char *p = strrchr(argv[0], '/');
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

    qApp->setName(appName);
    if (qt_is_gui_used) {
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	QMenuBar::initialize();
#endif
	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();
#if defined(QT_THREAD_SUPPORT)
	QThread::initialize();
#endif

	{ //create an empty widget on startup and this can be used for a port anytime
	    QWidget *tlw = new QWidget(NULL, "empty_widget", Qt::WDestructiveClose);
	    tlw->hide();
	    qt_mac_safe_pdev = tlw;
	}
#if defined(QT_THREAD_SUPPORT)
	qt_mac_port_mutex = new QMutex(TRUE);
#endif
	RegisterAppearanceClient();

	if(!app_proc_handler) {
	    app_proc_handlerUPP = NewEventHandlerUPP(QApplication::globalEventProcessor);
	    InstallEventHandler(GetApplicationEventTarget(), app_proc_handlerUPP,
				GetEventTypeCount(events), events, (void *)qApp, 
				&app_proc_handler);
	}

	if(QApplication::app_style) {
	    QEvent ev(QEvent::Style);
	    QApplication::sendSpontaneousEvent(QApplication::app_style, &ev);
	}
    } else {
#if defined(Q_OS_MACX)
	pipe( qt_thread_pipe );
#endif
    }
    QApplication::qt_mac_apply_settings();
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if(app_proc_handler) {
	RemoveEventHandler(app_proc_handler);
	app_proc_handler = NULL;
    }
    if(app_proc_handlerUPP) {
	DisposeEventHandlerUPP(app_proc_handlerUPP);
	app_proc_handlerUPP = NULL;
    }
    cleanupTimers();
    QPixmapCache::clear();
    if(qt_is_gui_used) {
	QPainter::cleanup();
	QFont::cleanup();
	QColor::cleanup();
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	QMenuBar::cleanup();
#endif
	if(qt_mac_safe_pdev) {
	    delete qt_mac_safe_pdev;
	    qt_mac_safe_pdev = NULL;
	}
    } else {
#ifdef Q_OS_MACX
	close(qt_thread_pipe[0]);
	close(qt_thread_pipe[1]);
#endif
    }
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/
void qt_updated_rootinfo()
{
}

bool qt_wstate_iconified( WId )
{
    return FALSE;
}

const char *qAppName()				// get application name
{
    return appName;
}

/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/
extern QWidget * mac_mouse_grabber;
extern QWidget * mac_keyboard_grabber;

void QApplication::setMainWidget(QWidget *mainWidget)
{
    main_widget = mainWidget;
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/
typedef QPtrList<QCursor> QCursorList;
static QCursorList *cursorStack = 0;
void QApplication::setOverrideCursor(const QCursor &cursor, bool replace)
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


QWidget *qt_recursive_match(QWidget *widg, int x, int y)
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
		    if(!curwidg->testWFlags(Qt::WMouseNoMask) &&
		       curwidg->extra && !curwidg->extra->mask.isNull() &&
		       !curwidg->extra->mask.contains(QPoint(x-wx, y-wy)))
			continue;
		    return qt_recursive_match(curwidg,x-wx,y-wy);
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
    QWidget *widget;
    qt_mac_find_window( x, y, &widget);
    if(!widget)
	return 0;

    //find the child
    if(child) {
	QPoint p = widget->mapFromGlobal(QPoint(x, y));
	widget = qt_recursive_match(widget, p.x(), p.y());
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
    enter_loop();
    return quit_code;
}

/* timer code */
struct TimerInfo {
    int id;
    QObject *obj;
    bool pending;
    //type switches
    enum TimerType { TIMER_ZERO, TIMER_QT, TIMER_MAC, TIMER_ANY } type;
    union {
	EventLoopTimerRef mac_timer;
	struct {
	    timeval  interval;
	    timeval  timeout;
	} qt_timer;
    } u;
};
static int zero_timer_count = 0;
typedef QPtrList<TimerInfo> TimerList;	// list of TimerInfo structs
static TimerList *timerList	= 0;		// timer list
static EventLoopTimerUPP timerUPP = NULL;       //UPP

#ifdef Q_OS_MACX
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
static timeval	watchtime;			// watch if time is turned back
timeval	*qt_wait_timer_max = 0;
static inline void getTime(timeval &t)	// get time of day
{
    gettimeofday(&t, 0);
    while(t.tv_usec >= 1000000) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while (t.tv_usec < 0) {
	if (t.tv_sec > 0) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}
static void repairTimer(const timeval &time)	// repair broken timer
{
    if (!timerList)				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while(t) {				// repair all timers
	if(t->type == TimerInfo::TIMER_QT) 
	    t->u.qt_timer.timeout = t->u.qt_timer.timeout - diff;
	else
	    qDebug("%s:%d This can't happen!", __FILE__, __LINE__);
	t = timerList->next();
    }
}
static timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if (timerList && timerList->count()) {	// there are waiting timers
	getTime(currentTime);
	if (first) {
	    if (currentTime < watchtime)	// clock was turned back
		repairTimer(currentTime);
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if(t->type == TimerInfo::TIMER_QT) {
	    if(currentTime < t->u.qt_timer.timeout) {	// time to wait
		tm = t->u.qt_timer.timeout - currentTime;
	    } else {
		tm.tv_sec  = 0;			// no time to wait
		tm.tv_usec = 0;
	    }
	    if(qt_wait_timer_max && *qt_wait_timer_max < tm)
		tm = *qt_wait_timer_max;
	} else {
	    qDebug("%s:%d This can't happen!", __FILE__, __LINE__);
	}
	return &tm;
    }
    if (qt_wait_timer_max) {
	tm = *qt_wait_timer_max;
	return &tm;
    }
    return 0;					// no timers
}
#endif

static void insertTimer(const TimerInfo *ti)	// insert timer info into list
{
    int index = 0;
    for(TimerInfo *t = timerList->first(); t; index++) {	// list is sorted by timeout
	if(t->type == TimerInfo::TIMER_QT && ti->u.qt_timer.timeout < t->u.qt_timer.timeout)
	    break;
	t = timerList->next();
    }
    timerList->insert(index, ti);		// inserts sorted
}

/* timer call back */
QMAC_PASCAL static void qt_activate_mac_timer(EventLoopTimerRef, void *data)
{
    TimerInfo *tmr = ((TimerInfo *)data);
    if(QMacBlockingFunction::blocking()) { //just send it immediately
	QTimerEvent e( tmr->id );
	QApplication::sendEvent( tmr->obj, &e );	// send event
	QApplication::flush(); //make sure to flush changes
	return;
    } 
    if(tmr->type != TimerInfo::TIMER_MAC) { //can't really happen, can it?
	qWarning("%s: %d Whoaaaaa!!!", __FILE__, __LINE__);
	return;
    }
    if(tmr->pending)
	return;
    tmr->pending = TRUE;
    EventRef tmr_ev = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestTimer, GetCurrentEventTime(),
		kEventAttributeUserEvent, &tmr_ev );
    SetEventParameter(tmr_ev, kEventParamTimer, typeTimerInfo, sizeof(tmr), &tmr);
    PostEventToQueue( GetMainEventQueue(), tmr_ev, kEventPriorityStandard );
    ReleaseEvent(tmr_ev);
}

//central cleanup
QMAC_PASCAL static Boolean find_timer_event(EventRef event, void *d)
{
    if(GetEventClass(event) != kEventClassQt || GetEventKind(event) != kEventQtRequestTimer)
	return false; //short circuit our tests..
    TimerInfo *t;
    GetEventParameter(event, kEventParamTimer, typeTimerInfo, NULL, sizeof(t), NULL, &t);
    if(t == ((TimerInfo *)d)) 
	return true;
    return false;
}

static bool killTimer(TimerInfo *t, bool remove=TRUE)
{
    t->pending = TRUE;
    if(t->type == TimerInfo::TIMER_MAC) {
	RemoveEventLoopTimer(t->u.mac_timer);
	if(t->pending) {
	    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
	    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)t);
	    DisposeEventComparatorUPP(fnc);
	}
    } else if(t->type == TimerInfo::TIMER_ZERO) {
	zero_timer_count--;
    }
    return remove ? timerList->remove() : TRUE;
}

//
// Timer initialization and cleanup routines
//
static void initTimers()			// initialize timers
{
    timerUPP = NewEventLoopTimerUPP(qt_activate_mac_timer);
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
	for( register TimerInfo *t = timerList->first(); t; t = timerList->next() )
	    killTimer(t, FALSE);
	delete timerList;
	timerList = 0;
    }
    if(timerUPP) {
	DisposeEventLoopTimerUPP(timerUPP);
	timerUPP = NULL;
    }
}

static int qt_activate_timers(TimerInfo::TimerType types = TimerInfo::TIMER_ANY)
{
    if(types == TimerInfo::TIMER_ZERO) {
	if(!zero_timer_count)
	    return 0;
	int ret = 0;
	for(register TimerInfo *t = timerList->first();
	    ret != zero_timer_count && t; t = timerList->next()) {
	    if(t->type == TimerInfo::TIMER_ZERO) {
		ret++;
		QTimerEvent e(t->id);
		QApplication::sendEvent(t->obj, &e);	// send event
	    }
	}
	return ret;
    }

    if (!timerList || !timerList->count())	// no timers
	return 0;
    int n_act = 0;
#ifdef Q_OS_MACX
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    register TimerInfo *t;
    while (maxcount--) {			// avoid starvation
	getTime(currentTime);			// get current time
	if (first) {
	    if (currentTime < watchtime)	// clock was turned back
		repairTimer(currentTime);
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if(t->type == TimerInfo::TIMER_QT) {
	    if(!t || currentTime < t->u.qt_timer.timeout) // no timer has expired
		break;
	    timerList->take();			// unlink from list
	    t->u.qt_timer.timeout += t->u.qt_timer.interval;
	    if(t->u.qt_timer.timeout < currentTime)
		t->u.qt_timer.timeout = currentTime + t->u.qt_timer.interval;
	    insertTimer(t);			// relink timer
	    if(t->u.qt_timer.interval.tv_usec > 0 || t->u.qt_timer.interval.tv_sec > 0)
		n_act++;
	    QTimerEvent e(t->id);
	    QApplication::sendEvent(t->obj, &e);	// send event
	}
    }
#endif
    return n_act;
}

//
// Main timer functions for starting and killing timers
//
int qStartTimer(int interval, QObject *obj)
{
    if (!timerList)				// initialize timer data
	initTimers();
    TimerInfo *t = new TimerInfo;		// create timer
    t->obj = obj;
    t->pending = TRUE;
    Q_CHECK_PTR( t );
#ifdef Q_OS_MACX
    if(!qt_is_gui_used) {
	t->type = TimerInfo::TIMER_QT;
	t->u.qt_timer.interval.tv_sec  = interval/1000;
	t->u.qt_timer.interval.tv_usec = (interval%1000)*1000;
	timeval currentTime;
	getTime(currentTime);
	t->u.qt_timer.timeout = currentTime + t->u.qt_timer.interval;
    } else
#endif
    if(!interval) {
	t->type = TimerInfo::TIMER_ZERO;
	zero_timer_count++;
    } else {
	t->type = TimerInfo::TIMER_MAC;
	EventTimerInterval mint = (((EventTimerInterval)interval) / 1000);
	if(InstallEventLoopTimer(GetMainEventLoop(), mint, mint, 
				 timerUPP, t, &t->u.mac_timer)) {
	    delete t;
	    return 0;
	}
    }
    static int serial_id = 666;
    t->id = serial_id++;
    t->pending = FALSE;
    insertTimer(t);
    return t->id;
}

bool qKillTimer(int id)
{
    if(!timerList || id <= 0)
	return FALSE;				// not init'd or invalid timer
    register TimerInfo *t = timerList->first();
    while(t && (t->id != id)) // find timer info in list
	t = timerList->next();
    if (t)					// id found
	return killTimer(t);
    return FALSE; // id not found
}

bool qKillTimer(QObject *obj)
{
    if ( !timerList )				// not initialized
	return FALSE;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// check all timers
	if (t->obj == obj) {			// object found
	    killTimer(t);
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


typedef QPtrList<QSockNot> QSNList;
typedef QPtrListIterator<QSockNot> QSNListIt;

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
    if(mac_select_timer) {
	RemoveEventLoopTimer(mac_select_timer);
	mac_select_timer = NULL;
    }
    if(mac_select_timerUPP) {
	DisposeEventLoopTimerUPP(mac_select_timerUPP);
	mac_select_timerUPP = NULL;
    }
    if(mac_context_timerUPP) {
	DisposeEventLoopTimerUPP(mac_context_timerUPP);
	mac_context_timerUPP = NULL;
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
	if(qt_is_gui_used && !mac_select_timer) {
	    if(!mac_select_timerUPP)
		mac_select_timerUPP = NewEventLoopTimerUPP(QApplication::qt_select_timer_callbk);
	    InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0.1,
				  mac_select_timerUPP, (void *)qApp, &mac_select_timer);
	}
    } else {					// disable notifier
	if ( list == 0 ) {
	    if(sn_highest == -1 && mac_select_timer) {
		RemoveEventLoopTimer(mac_select_timer);
		mac_select_timer = NULL;
	    }
	    return FALSE;			// no such fd set
	}
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
	if(sn_highest == -1 && mac_select_timer) {
	    RemoveEventLoopTimer(mac_select_timer);
	    mac_select_timer = NULL;
	}
    }
    return TRUE;
}

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
bool qt_set_socket_handler(int, int, QObject *, bool)
{
    return FALSE;
}
//#warning "need to implement sockets on mac9"
#endif

static int qt_mac_do_select(timeval *tm) 
{
    int highest = -1;
    if ( qt_preselect_handler ) {
	QVFuncList::Iterator end = qt_preselect_handler->end();
	for ( QVFuncList::Iterator it = qt_preselect_handler->begin();
	      it != end; ++it )
	    (**it)();
    }
#ifdef Q_OS_MACX
    if ( sn_highest >= 0 ) {			// has socket notifier(s)
	highest = sn_highest;
	if ( sn_read )
	    app_readfds = sn_readfds;
	else
	    FD_ZERO( &app_readfds );
	if ( sn_write )
	    app_writefds = sn_writefds;
	if ( sn_except )
	    app_exceptfds = sn_exceptfds;
    } else {
	FD_ZERO(&app_readfds);
    }
#ifdef Q_OS_MACX
    if(!qt_is_gui_used) {
	FD_SET(qt_thread_pipe[0], &app_readfds);
	highest = QMAX(highest, qt_thread_pipe[0]);
    }
#endif
    int nsel = select(highest + 1, (&app_readfds), (sn_write  ? &app_writefds  : 0),
		       (sn_except ? &app_exceptfds : 0), tm);

#else
    //#warning "need to implement sockets on mac9"
#endif
    if (qt_postselect_handler) {
	QVFuncList::Iterator end = qt_postselect_handler->end();
	for (QVFuncList::Iterator it = qt_postselect_handler->begin();
	      it != end; ++it)
	    (**it)();
    }
#ifdef Q_OS_MACX
    if (nsel == -1) {
	if (errno == EINTR || errno == EAGAIN) {
	    errno = 0;
	}
    } else if (nsel > 0 && highest >= 0) {
	if(qt_is_gui_used) 
	    qt_event_request_updates();
#ifdef Q_OS_MACX
	else if(FD_ISSET( qt_thread_pipe[0], &app_readfds)) {
	    char c;
	    ::read(qt_thread_pipe[0], &c, 1);
	}
#endif
	sn_activate();
    }
    return nsel;
#else
    //#warning "need to implement sockets on mac9"
#endif
}

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
    PostEventToQueue(GetMainEventQueue(), sel, kEventPriorityStandard);
    ReleaseEvent(sel);
}

bool QApplication::processNextEvent(bool canWait)
{
#if 0
    //TrackDrag says you may not use the EventManager things..
    if(qt_mac_in_drag) {
	qWarning("Whoa! Cannot process events whilst dragging!");
	return FALSE;
    }
#endif
    int	   nevents = 0;
#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif
    emit guiThreadAwake();
#ifdef QMAC_LAME_TIME_LIMITED
    static bool first = FALSE;
    if(!first) {
	first = TRUE;
	QDate dt = QDate::currentDate();
	if(dt.year() != 2001) {
	    const char *out_str = "Sorry, your evaluation has expired.\n" 
				  "Please contact sales@trolltech.com to continue using Qt/Mac\n"
	    fprintf(stderr, str);
	    if(qt_is_gui_used) {
		QTimer tb;
		QObject::connect(&tb, SIGNAL(timeout()), qApp, SLOT(quit()));
		tb.start(4000);
		if(QMessageBox::critical(NULL, "Evaluation over", "Sorry, your evaluation has expired."
					 "Please contact sales@trolltech.com to continue using Qt/Mac"))
		    qApp->quit();
	    } else {
		qApp->quit();
	    }
	}
    }
#endif

    if(qt_is_gui_used) {
	if(qt_replay_event) {	//ick
	    EventRef ev = qt_replay_event;
	    qt_replay_event = NULL;
	    SendEventToWindow(ev, (WindowPtr)qt_mac_safe_pdev->handle());
	    ReleaseEvent(ev);
	}
	sendPostedEvents();
	qt_activate_timers(TimerInfo::TIMER_ZERO); //try to send null timers..

	EventRef event;
	do {
	    do {
		if(ReceiveNextEvent( 0, 0, QMAC_EVENT_NOWAIT, TRUE, &event))
		    break;
		if(!SendEventToEventTarget(event, GetEventDispatcherTarget()))
		    nevents++;
		ReleaseEvent(event);
	    } while(GetNumEventsInQueue(GetMainEventQueue()));
	    sendPostedEvents();
	} while(GetNumEventsInQueue(GetMainEventQueue()));
    }

    if(quit_now || app_exit_loop) {
#if defined(QT_THREAD_SUPPORT)
	qApp->unlock(FALSE);
#endif
	return FALSE;
    }

    sendPostedEvents();
    if(!qt_is_gui_used) {
	timeval *tm = qt_wait_timer();
	if (!canWait) { 		// no time to wait
	    static timeval zerotm;
	    if (!tm)
		tm = &zerotm;
	    tm->tv_sec  = 0;	
	    tm->tv_usec = 0;
	}
	nevents += qt_mac_do_select(tm);
	nevents += qt_activate_timers();
    } else if(canWait && !zero_timer_count) {
	RunApplicationEventLoop();
    }

#if defined(QT_THREAD_SUPPORT)
    qApp->unlock(FALSE);
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
//terminator
{   0, MAP_KEY(0) } };

static key_sym keyscan_syms[] = { //real scan codes
{ 122, MAP_KEY(Qt::Key_F1) },
{ 120, MAP_KEY(Qt::Key_F2) },
{ 99, MAP_KEY(Qt::Key_F3) },
{ 118, MAP_KEY(Qt::Key_F4) },
{ 96, MAP_KEY(Qt::Key_F5) },
{ 97, MAP_KEY(Qt::Key_F6) },
{ 98, MAP_KEY(Qt::Key_F7) },
{ 100, MAP_KEY(Qt::Key_F8) },
{ 101, MAP_KEY(Qt::Key_F9) },
{ 109, MAP_KEY(Qt::Key_F10) },
{ 103, MAP_KEY(Qt::Key_F11) },
{ 111, MAP_KEY(Qt::Key_F12) },
{   0, MAP_KEY(0) } };

static int get_key(int modif, int key, int scan)
{
#ifdef DEBUG_KEY_MAPS
    qDebug("**Mapping key: %d", key);
#endif

    //general cases..
    if(key >= '0' && key <= '9') {
#ifdef DEBUG_KEY_MAPS
	qDebug("%d: General case Qt::Key_%c", __LINE__, key);
#endif
	return (key - '0') + Qt::Key_0;
    }

    if((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')) {
	char tup = toupper(key);
#ifdef DEBUG_KEY_MAPS
	qDebug("%d: General case Qt::Key_%c %d", __LINE__, tup, (tup - 'A') + Qt::Key_A);
#endif
	return (tup - 'A') + Qt::Key_A;
    }

    for(int i = 0; key_syms[i].qt_code; i++) {
	if(key_syms[i].mac_code == key) {
	    /* To work like Qt/X11 we issue Backtab when Shift + Tab are pressed */
	    if(key_syms[i].qt_code == Qt::Key_Tab && (modif & Qt::ShiftButton)) {
#ifdef DEBUG_KEY_MAPS
		qDebug("%d: got key: Qt::Key_Backtab", __LINE__);
#endif
		return Qt::Key_Backtab;
	    }

#ifdef DEBUG_KEY_MAPS
	    qDebug("%d: got key: %s", __LINE__, key_syms[i].desc);
#endif
	    return key_syms[i].qt_code;
	}
    }

    //last ditch try to match the scan code
    for(int i = 0; keyscan_syms[i].qt_code; i++) {
	if(keyscan_syms[i].mac_code == scan) {
#ifdef DEBUG_KEY_MAPS
	    qDebug("%d: got key: %s", __LINE__, keyscan_syms[i].desc);
#endif
	    return keyscan_syms[i].qt_code;
	}
    }

    //oh well
#ifdef DEBUG_KEY_MAPS
    qDebug("Unknown case.. %s:%d %d %d", __FILE__, __LINE__, key, scan);
#endif
    return Qt::Key_unknown;
}

static bool mouse_down_unhandled = FALSE;
bool QApplication::do_mouse_down(Point *pt)
{
    QWidget *widget;
    short windowPart = qt_mac_find_window(pt->h, pt->v, &widget);
    mouse_down_unhandled = FALSE;
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
    if(windowPart == inMenuBar) {
	QMacBlockingFunction block;
	MenuSelect(*pt); //allow menu tracking
	return FALSE;
    } else
#endif
    if(!widget) {
	mouse_down_unhandled = TRUE;
	return FALSE;
    } else if(windowPart == inContent) {
	return TRUE; //just return and let the event loop process
    } else if(windowPart != inGoAway && windowPart != inCollapseBox) {
	widget->raise();
	if(widget->isTopLevel() && !widget->isDesktop() && !widget->isPopup() &&
	   (widget->isModal() || !widget->inherits("QDockWindow")))
	    widget->setActiveWindow();
    }
    if(windowPart == inGoAway || windowPart == inToolbarButton || 
       windowPart == inCollapseBox || windowPart == inZoomIn || windowPart == inZoomOut) {
	QMacBlockingFunction block;
	if(!TrackBox((WindowPtr)widget->handle(), *pt, windowPart))
	    return FALSE;
    }

    switch(windowPart) {
    case inStructure:
    case inDesk:
	break;
    case inGoAway:
	if(TrackBox( (WindowPtr)widget->handle(), *pt, windowPart))
	    widget->close();
	break;
    case inToolbarButton: { //hide toolbars thing
	if(const QObjectList *chldrn = widget->children()) {
	    int h = 0;
	    for(QObjectListIt it(*chldrn); it.current(); ++it) {
		if(it.current()->isWidgetType() && it.current()->inherits("QDockArea")) {
		    QWidget *w = (QWidget *)it.current();
		    if(w->width() < w->height()) //only do horizontal orientations
			continue;
		    int oh = w->sizeHint().height();
		    if(oh < 0)
			oh = 0;
		    if(QObjectList *l = w->queryList("QToolBar")) {
			for(QObjectListIt it2(*l); it2.current(); ++it2) {
			    QWidget *t = (QWidget *)(*it2);
			    if(t->topLevelWidget() == widget) {
				if(t->isVisible())
				    t->hide();
				else
				    t->show();
			    }
			}
			delete l;
		    }
		    sendPostedEvents();
		    int nh = w->sizeHint().height();
		    if(nh < 0)
			nh = 0;
		    if(oh != nh)
			h += (oh - nh);
		}
	    }
	    if(h)
		widget->resize(widget->width(), widget->height() - h);
	}
	break; }
    case inDrag: {
	{
	    QMacBlockingFunction block;
	    DragWindow((WindowPtr)widget->handle(), *pt, 0);
	}
	QPoint np, op(widget->crect.x(), widget->crect.y());
	{
	    QMacSavedPortInfo savedInfo(widget);
	    Point p = { 0, 0 };
	    LocalToGlobal(&p);
	    np = QPoint(p.h, p.v);
	}
	if(np != op) {
	    widget->crect = QRect(np, widget->crect.size());
	    QMoveEvent qme(np, op);
	}
	break; }
    case inGrow: {
	Rect limits;
	SetRect(&limits, -2, 0, 0, 0 );
	if(QWExtra   *extra = widget->extraData()) 
	    SetRect(&limits, extra->minw, extra->minh,
		    extra->maxw < QWIDGETSIZE_MAX ? extra->maxw : QWIDGETSIZE_MAX,
		    extra->maxh < QWIDGETSIZE_MAX ? extra->maxh : QWIDGETSIZE_MAX);
	int growWindowSize;
	{
	    QMacBlockingFunction block;
	    growWindowSize = GrowWindow((WindowPtr)widget->handle(), 
					 *pt, limits.left == -2 ? NULL : &limits);
	}
	if(growWindowSize) {
	    // nw/nh might not match the actual size if setSizeIncrement is used
	    int nw = LoWord(growWindowSize);
	    int nh = HiWord(growWindowSize);
	    if(nw != widget->width() || nh != widget->height()) {
		if(nw < desktop()->width() && nw > 0 && nh < desktop()->height() && nh > 0)
			widget->resize(nw, nh);
	    }
	}
	break;
    }
    case inCollapseBox:
	widget->showMinimized();
	break;
    case inZoomIn:
	if(TrackBox((WindowPtr)widget->handle(), *pt, windowPart)) 
	    widget->showNormal();
	break;
    case inZoomOut:
	if(TrackBox((WindowPtr)widget->handle(), *pt, windowPart))
	    widget->showMaximized();
	break;
    default:
	qDebug("Unhandled case in mouse_down.. %d", windowPart);
	break;
    }
    return FALSE;
}

static bool wakeup_pending = FALSE;
void QApplication::wakeUpGuiThread()
{
#ifdef Q_OS_MACX
    if(!qt_is_gui_used) {
	char c = 0;
	::write(qt_thread_pipe[1], &c, 1);
    }
#endif
    if(wakeup_pending)
	return;
    wakeup_pending = TRUE;
    EventRef upd = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestWakeup, GetCurrentEventTime(),
		kEventAttributeUserEvent, &upd);
    PostEventToQueue(GetMainEventQueue(), upd, kEventPriorityHigh);
    ReleaseEvent(upd);
}

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal(QWidget *widget)
{
    if (!qt_modal_stack) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	Q_CHECK_PTR(qt_modal_stack);
    }
    qt_modal_stack->insert(0, widget);
    app_do_modal = TRUE;
}


void qt_leave_modal(QWidget *widget)
{
    if(qt_modal_stack && qt_modal_stack->removeRef(widget)) {
	if(qt_modal_stack->isEmpty()) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	}
    }
    app_do_modal = qt_modal_stack != 0;
}


static bool qt_try_modal(QWidget *widget, EventRef event)
{
   if(qApp->activePopupWidget())
	return TRUE;

    QWidget *modal=0, *top=QApplication::activeModalWidget();

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if (widget->testWFlags(Qt::WShowModal))	// widget is modal
	modal = widget;
    if (!top || modal == top)			// don't block event
	return TRUE;

    while (groupLeader && !groupLeader->testWFlags(Qt::WGroupLeader))
	groupLeader = groupLeader->parentWidget();

    if (groupLeader) {
	// Does groupLeader have a child in qt_modal_stack?
	bool unrelated = TRUE;
	modal = qt_modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while (p && p != groupLeader && !p->testWFlags(Qt::WGroupLeader)) {
		p = p->parentWidget();
	    }
	    modal = qt_modal_stack->next();
	    if (p == groupLeader) unrelated = FALSE;
	}

	if (unrelated)
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

    if (!top->parentWidget() && (block_event || paint_event))
	top->raise();

    return !block_event;
}

//context menu hack
static bool request_context_pending = FALSE;
QMAC_PASCAL void
QApplication::qt_context_timer_callbk(EventLoopTimerRef r, void *d)
{
    QWidget *w = (QWidget *)d;
    EventLoopTimerRef otc = mac_context_timer;
    RemoveEventLoopTimer(mac_context_timer);
    mac_context_timer = NULL;
    if(r != otc || w != qt_button_down || 
       request_context_pending || QMacBlockingFunction::blocking())
	return;
    request_context_pending = TRUE;

    EventRef ctx = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestContext, GetCurrentEventTime(),
		kEventAttributeUserEvent, &ctx );
    SetEventParameter(ctx, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), ctx, kEventPriorityStandard);
    ReleaseEvent(ctx);
}

QMAC_PASCAL OSStatus
QApplication::globalEventProcessor(EventHandlerCallRef er, EventRef event, void *data)
{
    QuitApplicationEventLoop();

    QApplication *app = (QApplication *)data;
    if (app->macEventFilter(event)) //someone else ate it
	return noErr; 
    QWidget *widget = NULL;

    /*Only certain event don't remove the context timer (the left hold context menu),
      otherwise we just turn it off. Similarly we assume all events are handled and in
      the code below we set it to false when we know we didn't handle it, this will let
      rogue events through (shouldn't really happen, but better safe than sorry) */
    bool remove_context_timer = TRUE, handled_event=TRUE;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass)
    {
    case kEventClassQt:
	remove_context_timer = FALSE;
	if(ekind == kEventQtRequestPropagateWidgetUpdates) {
	    for(QValueList<WId>::Iterator it = request_updates_pending_list.begin();
		it != request_updates_pending_list.end(); ++it) {
		QWidget *widget = QWidget::find((*it));
		if(widget && widget->extra && widget->extra->has_dirty_area) {
		    widget->extra->has_dirty_area = FALSE;
		    QRegion r = widget->extra->dirty_area;
		    widget->extra->dirty_area = QRegion();
		    QRegion cr = widget->clippedRegion();
		    if(!widget->isTopLevel()) {
			QPoint point(posInWindow(widget));
			cr.translate(-point.x(), -point.y());
		    }
		    r &= cr;
		    if(!r.isEmpty()) 
			widget->repaint(r, TRUE);
		}
	    }
	    request_updates_pending_list.clear();
	} else if(ekind == kEventQtRequestPropagateWindowUpdates) {
	    request_updates_pending = FALSE;
	    QApplication::sendPostedEvents();
	    if(QWidgetList *list   = qApp->topLevelWidgets()) {
		for (QWidget *widget = list->first(); widget; widget = list->next()) {
		    if (!widget->isHidden()) 
			widget->propagateUpdates();
		}
		delete list;
	    } 
	} else if(ekind == kEventQtRequestWakeup) {
	    wakeup_pending = FALSE; 	    //do nothing else , we just woke up!
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	} else if(ekind == kEventQtRequestMenubarUpdate) {
	    request_menubarupdate_pending = FALSE;
	    QMenuBar::macUpdateMenuBar();
#endif
	} else if(ekind == kEventQtRequestSelect) {
	    request_select_pending = FALSE;
	    timeval tm;
	    memset(&tm, '\0', sizeof(tm));
	    qt_mac_do_select(&tm);
	} else if(ekind == kEventQtRequestContext) {
	    if(request_context_pending) {
		request_context_pending = FALSE;
		//figure out which widget to send it to
		QPoint where = QCursor::pos();
		QWidget *widget = NULL;
		GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
				  sizeof(widget), NULL, &widget);
		if(!widget) {
		    if(qt_button_down)
			widget = qt_button_down;
		    else
			widget = QApplication::widgetAt( where.x(), where.y(), true );
		}
		if ( widget ) {
		    QPoint plocal(widget->mapFromGlobal( where ));
		    QContextMenuEvent qme( QContextMenuEvent::Mouse, plocal, where, 0 );
		    QApplication::sendEvent( widget, &qme );
		    if(qme.isAccepted()) { //once this happens the events before are pitched
			if(qt_button_down && mouse_button_state) {
			    QMouseEvent qme( QEvent::MouseButtonRelease, plocal, where,
					     mouse_button_state, mouse_button_state );
			    QApplication::sendSpontaneousEvent( qt_button_down, &qme );
			}
			qt_button_down = NULL;
			mouse_button_state = 0;
			qt_mac_dblclick.active = FALSE;
#ifdef DEBUG_MOUSE_MAPS
			qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__,
			       mouse_button_state);
#endif
		    }
		} else {
		    handled_event = FALSE;
		}
	    }
	} else if(ekind == kEventQtRequestTimer) {
	    if(!timerList)
		break;
	    TimerInfo *t;
	    GetEventParameter(event, kEventParamTimer, typeTimerInfo, NULL, sizeof(t), NULL, &t);
	    if ( t && t->pending ) {
		t->pending = FALSE;
		QTimerEvent e( t->id );
		QApplication::sendEvent( t->obj, &e );	// send event
	    }
	} else {
	    handled_event = FALSE;
	}
	break;
    case kEventClassMouse:
    {
	if( (ekind == kEventMouseDown && mouse_button_state ) ||
	    (ekind == kEventMouseUp && !mouse_button_state) ) {
#ifdef DEBUG_MOUSE_MAPS
	    qDebug("Dropping mouse event.. %d %d", ekind == kEventMouseDown, mouse_button_state);
#endif
	    break;
	}
#ifdef DEBUG_MOUSE_MAPS
	else qDebug("Handling mouse: %d", ekind == kEventMouseDown);
#endif
	int keys;
	QEvent::Type etype = QEvent::None;
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
	    etype = QEvent::MouseButtonPress;
	    if(qt_mac_dblclick.active) {
		if(qt_mac_dblclick.use_qt_time_limit) {
		    EventTime now = GetEventTime(event);
		    if(qt_mac_dblclick.last_time != -2 &&
		       now - qt_mac_dblclick.last_time <= doubleClickInterval())
			etype = QEvent::MouseButtonDblClick;
		} else {
		    UInt32 count;
		    GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL,
				      sizeof(count), NULL, &count);
		    if(!(count % 2) && qt_mac_dblclick.last_modifiers == keys &&
		       qt_mac_dblclick.last_button == button)
			etype = QEvent::MouseButtonDblClick;
		}
		if(etype == QEvent::MouseButtonDblClick)
		    qt_mac_dblclick.active = FALSE;
	    }
	    after_state = button;
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
	    wheel_delta = mdelt * 120;
	    state = after_state;
	    break;
	}
	}
	//figure out which widget to send it to
	if(ekind != kEventMouseDown && qt_button_down)
	    widget = qt_button_down;
	else if(mac_mouse_grabber)
	    widget = mac_mouse_grabber;
	else
	    widget = QApplication::widgetAt(where.h, where.v, true);

	//set the cursor up
	const QCursor *n = NULL;
	if(widget) { //only over the app, do we set a cursor..
	    if(cursorStack) {
		n = app_cursor;
	    } else {
		for(QWidget *p = widget; p; p = p->parentWidget()) {
		    if(p->extra && p->extra->curs) {
			n = p->extra->curs;
			break;
		    }
		}
	    }
	}
	if(!n)
	    n = &arrowCursor; //I give up..
	qt_mac_set_cursor(n, &where);

	//This mouse button state stuff looks like this on purpose
	//although it looks hacky it is VERY intentional..
	if (widget && app_do_modal && !qt_try_modal(widget, event)) {
	    mouse_button_state = after_state;
#ifdef DEBUG_MOUSE_MAPS
	    qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__, mouse_button_state);
#endif
	    return 1;
	}

	//handle popup's first
	QWidget *popupwidget = NULL;
	if(app->inPopupMode()) {
	    QWidget *clt;
	    qt_mac_find_window(where.h, where.v, &clt);
	    if(clt && clt->isPopup())
		popupwidget = clt;
	    if(!popupwidget)
		popupwidget = activePopupWidget();
	    QMacSavedPortInfo savedInfo(popupwidget);
	    Point gp = where;
	    GlobalToLocal(&gp); //now map it to the window
	    popupwidget = qt_recursive_match(popupwidget, gp.h, gp.v);
	}
	bool special_close = FALSE;
	if(popupwidget) {
	    qt_closed_popup = FALSE;
	    QPoint p(where.h, where.v);
	    QPoint plocal(popupwidget->mapFromGlobal(p));
	    bool was_context = FALSE;
	    if(ekind == kEventMouseDown &&
	       ((button == QMouseEvent::RightButton) ||
		(button == QMouseEvent::LeftButton && (keys & Qt::ControlButton)))) {
		QContextMenuEvent cme(QContextMenuEvent::Mouse, plocal, p, keys );
		QApplication::sendSpontaneousEvent(popupwidget, &cme);
		was_context = cme.isAccepted();
	    }
	    if(!was_context) {
		if(wheel_delta) {
		    QWheelEvent qwe(plocal, p, wheel_delta, state | keys);
		    QApplication::sendSpontaneousEvent(popupwidget, &qwe);
		} else {
		    QMouseEvent qme(etype, plocal, p, button, state | keys);
		    QApplication::sendSpontaneousEvent(popupwidget, &qme);
		}
		if(app->activePopupWidget() != popupwidget && qt_closed_popup)
		    special_close = TRUE;
	    }
	    if(special_close) { 	    //We will resend this event later, so just return
		if(widget != popupwidget) { // We've already sent the event to the correct widget
		    qt_replay_event = CopyEvent(event);
		} else {
		    mouse_button_state = after_state;
#ifdef DEBUG_MOUSE_MAPS
		    qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__,
			   mouse_button_state);
#endif
		}
		break;
	    }
	}

	if(ekind == kEventMouseDown) {
	    if(!app->do_mouse_down(&where)) {
		if(mouse_down_unhandled) {
		    handled_event = FALSE;
		    break;
		}
		mouse_button_state = 0;
#ifdef DEBUG_MOUSE_MAPS
		qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__, mouse_button_state);
#endif
		return 0;
	    } else if(QWidget* w = widget) {
		while(w->focusProxy())
		    w = w->focusProxy();
		QWidget *tlw = w->topLevelWidget();
		tlw->raise();
		if(tlw->isTopLevel() && !tlw->isDesktop() && !tlw->isPopup() &&
		   (tlw->isModal() || !tlw->inherits("QDockWindow")))
		    tlw->setActiveWindow();
	    }
	}
	mouse_button_state = after_state;
#ifdef DEBUG_MOUSE_MAPS
	qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__, mouse_button_state);
#endif

	switch(ekind) {
	case kEventMouseDragged:
	case kEventMouseMoved:
	{
	    if ((QWidget *)qt_mouseover != widget) {
#ifdef DEBUG_MOUSE_MAPS
		qDebug("Entering: %s (%s), Leaving %s (%s)",
		       widget ? widget->className() : "none", widget ? widget->name() : "",
		       qt_mouseover ? qt_mouseover->className() : "none",
		       qt_mouseover ? qt_mouseover->name() : "");
#endif
		qt_dispatchEnterLeave(widget, qt_mouseover);
		qt_mouseover = widget;
	    }
	    break;
	}
	case kEventMouseDown:
	    if(button == QMouseEvent::LeftButton && !mac_context_timer) {
		remove_context_timer = FALSE;
		if(!mac_context_timerUPP)
		    mac_context_timerUPP = NewEventLoopTimerUPP(qt_context_timer_callbk);
		InstallEventLoopTimer(GetMainEventLoop(), 2, 0, mac_context_timerUPP,
				      widget, &mac_context_timer);
	    }
	    qt_button_down = widget;
	    break;
	case kEventMouseUp:
	    qt_button_down = NULL;
	    break;
	}

	//finally send the event to the widget if its not the popup
	if (widget && widget != popupwidget) {
	    if(ekind == kEventMouseDown || ekind == kEventMouseWheelMoved) {
		if(popupwidget) //guess we close the popup...
		    popupwidget->close();

		QWidget* w = widget;
		while (w->focusProxy())
		    w = w->focusProxy();
		int fp = (ekind == kEventMouseDown) ? QWidget::ClickFocus : QWidget::WheelFocus;
		if(w->focusPolicy() & fp) {
		    QFocusEvent::setReason(QFocusEvent::Mouse);
		    w->setFocus();
		    QFocusEvent::resetReason();
		}
	    }

	    QPoint p(where.h, where.v );
	    QPoint plocal(widget->mapFromGlobal(p));
	    bool was_context = FALSE;
	    if(ekind == kEventMouseDown &&
	       ((button == QMouseEvent::RightButton) ||
		(button == QMouseEvent::LeftButton && (keys & Qt::ControlButton)))) {
		QContextMenuEvent cme(QContextMenuEvent::Mouse, plocal, p, keys );
		QApplication::sendSpontaneousEvent(widget, &cme);
		was_context = cme.isAccepted();
	    }
#ifdef DEBUG_MOUSE_MAPS
	    char *event_desc = NULL;
	    if(was_context) {
		event_desc = "Context Menu";
	    } else if(etype == QEvent::MouseButtonDblClick) {
		event_desc = "Double Click";
	    } else {
		switch(ekind) {
		case kEventMouseDown: event_desc = "MouseButtonPress"; break;
		case kEventMouseUp: event_desc = "MouseButtonRelease"; break;
		case kEventMouseDragged:
		case kEventMouseMoved: event_desc = "MouseMove"; break;
		case kEventMouseWheelMoved: event_desc = "MouseWheelMove"; break;
		}
	    }
	    qDebug("%d %d (%d %d) - Would send (%s) event to %s %s (%d %d %d)", p.x(), p.y(),
		   plocal.x(), plocal.y(), event_desc, widget->name(), widget->className(),
		   button, state|keys, wheel_delta);
#endif
	    if(!was_context) {
		if(etype == QEvent::MouseButtonPress) {
		    qt_mac_dblclick.active = TRUE;
		    qt_mac_dblclick.last_modifiers = keys;
		    qt_mac_dblclick.last_button = button;
		    qt_mac_dblclick.last_time = GetEventTime(event);
		}
		if(wheel_delta) {
		    QWheelEvent qwe(plocal, p, wheel_delta, state | keys);
		    QApplication::sendSpontaneousEvent(widget, &qwe);
		    if(!qwe.isAccepted() && focus_widget && focus_widget != widget) {
			QWheelEvent qwe2(focus_widget->mapFromGlobal(p), p, 
					  wheel_delta, state | keys);
			QApplication::sendSpontaneousEvent(focus_widget, &qwe2);
	    }
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
		    QMouseEvent qme(etype, plocal, p, button, state | keys);
		    QApplication::sendSpontaneousEvent(widget, &qme);
		}
	    }
	} else {
	    handled_event = FALSE;
	}
	break;
    }
    case kEventClassTextInput:
	handled_event = FALSE;
	if(ekind == kEventTextInputUnicodeForKeyEvent && (widget=focus_widget)) {
	    EventRef key_ev;
	    GetEventParameter(event, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL,
			      sizeof(key_ev), NULL, &key_ev);
	    char chr;
	    GetEventParameter(key_ev, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(chr), NULL, &chr);
	    UInt32 keyc;
	    GetEventParameter(key_ev, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyc), NULL, &keyc);
	    if(get_key(0, chr, keyc) == Qt::Key_unknown) {
		QIMEvent imstart(QEvent::IMStart, QString::null, -1);
		QApplication::sendSpontaneousEvent(widget, &imstart);
		if(imstart.isAccepted()) { //doesn't want the event
		    handled_event = TRUE;
		    UInt32 unilen;
		    GetEventParameter(event, kEventParamTextInputSendText, typeUnicodeText,
				      NULL, 0, &unilen, NULL);
		    UniChar *unicode = (UniChar*)NewPtr(unilen);
		    GetEventParameter(event, kEventParamTextInputSendText, typeUnicodeText,
				      NULL, unilen, NULL, unicode);
		    QString text((QChar*)unicode, unilen / 2);
		    DisposePtr((char*)unicode);
		    QIMEvent imend(QEvent::IMEnd, text, 1);
		    QApplication::sendSpontaneousEvent(widget, &imend);
		}
	    }
	    if(!handled_event)
		return -1;
	}
	break;
    case kEventClassKeyboard: {
	UInt32 modif;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL,
			  sizeof(modif), NULL, &modif);

	/*unfortunatly modifiers changed event looks quite different, so I have a separate
	  code path */
	if(ekind == kEventRawKeyModifiersChanged) {
	    int changed_modifiers = keyboard_modifiers_state ^ modif,
		   last_modifiers = keyboard_modifiers_state,
			modifiers = get_modifiers(last_modifiers);
	    keyboard_modifiers_state = modif;
	    if( mac_keyboard_grabber )
		widget = mac_keyboard_grabber;
	    else if(focus_widget)
		widget = focus_widget;
	    if(!widget || (app_do_modal && !qt_try_modal(widget, event) ))
		break;
	    static key_sym key_modif_syms[] = {
		{ shiftKeyBit, MAP_KEY(Qt::Key_Shift) },
		{ rightShiftKeyBit, MAP_KEY(Qt::Key_Shift) }, //???
		{ controlKeyBit, MAP_KEY(Qt::Key_Control) },
		{ rightControlKeyBit, MAP_KEY(Qt::Key_Control) }, //???
		{ cmdKeyBit, MAP_KEY(Qt::Key_Meta) },
		{ optionKeyBit, MAP_KEY(Qt::Key_Super_L) },
		{ rightOptionKeyBit, MAP_KEY(Qt::Key_Super_R) },
		{ alphaLockBit, MAP_KEY(Qt::Key_CapsLock) },
		{   0, MAP_KEY(0) } };
	    for(int i = cmdKeyBit; i <= rightControlKeyBit; i++) {
		if(!(changed_modifiers & (1 << i)))
		    continue;
		QEvent::Type etype = QEvent::KeyPress;
		if(last_modifiers & (1 << i))
		    etype = QEvent::KeyRelease;
		int key = 0;
		for(int x = 0; key_modif_syms[x].mac_code; x++) {
		    if(key_modif_syms[x].mac_code == i) {
#ifdef DEBUG_KEY_MAPS
			qDebug("got modifier changed: %s", key_modif_syms[x].desc);
#endif
			key = key_modif_syms[x].qt_code;
			break;
		    }
		}
		if(!key) {
#ifdef DEBUG_KEY_MAPS
		    qDebug("could not get modifier changed: %d", i);
#endif
		    continue;
		}
#ifdef DEBUG_KEY_MAPS
		qDebug("KeyEvent (modif): Sending %s to %s::%s: %d - %d",
		       etype == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
		       widget ? widget->className() : "none", widget ? widget->name() : "",
		       key, modifiers);
#endif
		QKeyEvent ke(etype, key, 0, modifiers, "", FALSE, 0);
		QApplication::sendSpontaneousEvent(widget,&ke);
	    }
	    break;
	}

	int modifiers = get_modifiers(modif);
	UInt32 keyc;
	GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyc), NULL, &keyc);
	//map it into qt keys
	UInt32 state = 0L;
	char chr = KeyTranslate((void *)GetScriptManagerVariable(smUnicodeScript),
		   (modif & (shiftKey|rightShiftKey|alphaLock)) | keyc, &state);
	if(!chr)
	    break;
	int mychar=get_key(modifiers, chr, keyc);
	static QTextCodec *c = NULL;
	if(!c)
	    c = QTextCodec::codecForName("Apple Roman");
       	QString mystr = c->toUnicode(&chr, 1);
	//now get the real ascii value
	UInt32 tmp_mod = 0L;
	if(modifiers & Qt::ShiftButton)
	    tmp_mod |= shiftKey;
	if(modifiers & Qt::ControlButton)
	    tmp_mod |= controlKey;
	if(modif & alphaLock)
	    tmp_mod |= alphaLock;
	chr = KeyTranslate((void *)GetScriptManagerVariable(smUnicodeScript),
			   tmp_mod | keyc, &state);

	QEvent::Type etype = (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress;
	if(mac_keyboard_grabber)
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;
	if(widget) {
	    if (app_do_modal && !qt_try_modal(widget, event))
		return 1;

	    bool key_event = TRUE;
	    if(etype == QEvent::KeyPress && !mac_keyboard_grabber) {
		QKeyEvent aa(QEvent::AccelOverride, mychar, chr, modifiers,
			     mystr, ekind == kEventRawKeyRepeat, mystr.length());
		aa.ignore();
		QApplication::sendSpontaneousEvent(widget, &aa);
		if (!aa.isAccepted()) {
		    QKeyEvent a(QEvent::Accel, mychar, chr, modifiers, 
				mystr, ekind == kEventRawKeyRepeat, mystr.length());
		    a.ignore();
		    QApplication::sendSpontaneousEvent(widget->topLevelWidget(), &a);
		    if (a.isAccepted()) {
#ifdef DEBUG_KEY_MAPS
		qDebug("KeyEvent: %s::%s consumed Accel: %04x %c %s %d",
		       widget ? widget->className() : "none", widget ? widget->name() : "",
		       mychar, chr, mystr.latin1(), ekind == kEventRawKeyRepeat);
#endif
			key_event = FALSE;
		    } else {
			HICommand hic;
			if(IsMenuKeyEvent(NULL, event, kNilOptions,
					  &hic.menu.menuRef, &hic.menu.menuItemIndex)) {
			    hic.attributes = kHICommandFromMenu;
			    if(GetMenuItemCommandID(hic.menu.menuRef, hic.menu.menuItemIndex,
						    &hic.commandID))
			       qDebug("Shouldn't happen.. %s:%d", __FILE__, __LINE__);
#if !defined(QMAC_QMENUBAR_NO_NATIVE) //In native menubar mode we offer the event to the menubar...
			    if(QMenuBar::activateCommand(hic.commandID) ||
			       QMenuBar::activate(hic.menu.menuRef, hic.menu.menuItemIndex,
						  FALSE, TRUE)) {
#ifdef DEBUG_KEY_MAPS
				qDebug("KeyEvent: Consumed by Menubar(1)");
#endif

				key_event = FALSE;
			    } else
#endif
			    if(!ProcessHICommand(&hic)) {
#ifdef DEBUG_KEY_MAPS
				qDebug("KeyEvent: Consumed by an HICommand(1)");
#endif
				key_event = FALSE;
			    }
			}
		    }
		}
	    }
	    if(key_event) {
		//Find out if someone else wants the event, namely
		//is it of use to text services? If so we won't bother
		//with a QKeyEvent.
		if(!CallNextEventHandler(er, event)) {
		    handled_event = TRUE;
		    break;
		}
		if(modifiers & Qt::AltButton) {
		    mystr = QString();
		    chr = 0;
		}
#ifdef DEBUG_KEY_MAPS
		qDebug("KeyEvent: Sending %s to %s::%s: %04x %c %s %d",
		       etype == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
		       widget ? widget->className() : "none", widget ? widget->name() : "",
		       mychar, chr, mystr.latin1(), ekind == kEventRawKeyRepeat);
#endif
		QKeyEvent ke(etype,mychar, chr, modifiers,
			     mystr, ekind == kEventRawKeyRepeat, mystr.length());
		QApplication::sendSpontaneousEvent(widget,&ke);
	    }
	} else if(etype == QEvent::KeyPress) {
#ifdef DEBUG_KEY_MAPS
	    qDebug("KeyEvent: No widget could be found to accept the KeyPress");
#endif
	    HICommand hic;
	    if(IsMenuKeyEvent(NULL, event, kNilOptions,
			      &hic.menu.menuRef, &hic.menu.menuItemIndex)) {
		hic.attributes = kHICommandFromMenu;
		if(GetMenuItemCommandID(hic.menu.menuRef, hic.menu.menuItemIndex,
					&hic.commandID))
		    qDebug("Shouldn't happen.. %s:%d", __FILE__, __LINE__);
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
		if(QMenuBar::activateCommand(hic.commandID) ||
		   QMenuBar::activate(hic.menu.menuRef, hic.menu.menuItemIndex, FALSE, TRUE)) {
#ifdef DEBUG_KEY_MAPS
		    qDebug("KeyEvent: Consumed by Menubar(2)");
#endif
		} else
#endif
		    if(!ProcessHICommand(&hic)) {
#ifdef DEBUG_KEY_MAPS
			qDebug("KeyEvent: Consumed by an HICommand(2)");
#endif
			handled_event = FALSE;
		    }
	    } else {
		handled_event = FALSE;
	    }
	} else {
	    handled_event = FALSE;
	}
	break; }
    case kEventClassWindow: {
	WindowRef wid;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
			  sizeof(WindowRef), NULL, &wid);
	widget = QWidget::find((WId)wid);
	if(!widget) {
	    if(ekind == kEventWindowShown )
		unhandled_dialogs.insert((void *)wid, (void *)1);
	    else if(ekind == kEventWindowHidden)
		unhandled_dialogs.remove((void *)wid);
	    handled_event = FALSE;
	    break;
	}

	if(ekind == kEventWindowUpdate) {
	    remove_context_timer = FALSE;
	    widget->propagateUpdates();
	} else if(ekind == kEventWindowBoundsChanged) {
	    UInt32 flags;
	    GetEventParameter(event, kEventParamAttributes, typeUInt32, NULL,
			      sizeof(flags), NULL, &flags);
	    Rect nr;
	    GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, NULL,
			      sizeof(nr), NULL, &nr);
	    if((flags & kWindowBoundsChangeOriginChanged)) {
		int ox = widget->crect.x(), oy = widget->crect.y();
		int nx = nr.left, ny = nr.top;
		if(nx != ox ||  ny != oy) {
		    widget->crect.setRect(nx, ny, widget->width(), widget->height());
		    QMoveEvent qme(widget->crect.topLeft(), QPoint( ox, oy));
		    QApplication::sendSpontaneousEvent(widget, &qme);
		}
	    }
	    if((flags & kWindowBoundsChangeSizeChanged)) {
		int nw = nr.right - nr.left, nh = nr.bottom - nr.top;
		if(widget->width() != nw || widget->height() != nh) {
		    widget->resize(nw, nh);
		    if(widget->isVisible())
			widget->propagateUpdates();
		}
	    }
	} else if(ekind == kEventWindowActivated) {
	    if(QApplication::app_style) {
		//I shouldn't have to do this, but the StyleChanged isn't happening as I expected
		//so this is in for now, FIXME!
		QEvent ev(QEvent::Style);
		QApplication::sendSpontaneousEvent(QApplication::app_style, &ev);
	    }

	    if(app_do_modal && !qt_try_modal(widget, event))
		return 1;

	    if(widget) {
		widget->raise();
		QWidget *tlw = widget->topLevelWidget();
		if(tlw->isTopLevel() && !tlw->isPopup() && (tlw->isModal() || 
							    !tlw->testWFlags(WStyle_Tool)))
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
	    app->setActiveWindow(NULL);
	} else {
	    handled_event = FALSE;
	}
	break; }
    case kEventClassApplication:
	if(ekind == kEventAppActivated) {
	    app->clipboard()->loadScrap(FALSE);
	    if(qt_clipboard) { //manufacture an event so the clipboard can see if it has changed
		QEvent ev(QEvent::Clipboard);
		QApplication::sendSpontaneousEvent(qt_clipboard, &ev);
	    }
	} else if(ekind == kEventAppDeactivated) {
	    while(app->inPopupMode())
		app->activePopupWidget()->close();
	    app->clipboard()->saveScrap();
	    app->setActiveWindow(NULL);
	} else {
	    handled_event = FALSE;
	}
	break;
    case kEventClassMenu:
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	if(ekind == kEventMenuOpening) {
	    Boolean first;
	    GetEventParameter(event, kEventParamMenuFirstOpen, typeBoolean,
			      NULL, sizeof(first), NULL, &first);
	    if(first) {
		MenuRef mr;
		GetEventParameter(event, kEventParamDirectObject, typeMenuRef,
				  NULL, sizeof(mr), NULL, &mr);
		QMenuBar::macUpdatePopup(mr);
	    }
	} else if(ekind == kEventMenuTargetItem) {
	    MenuRef mr;
	    GetEventParameter(event, kEventParamDirectObject, typeMenuRef,
			      NULL, sizeof(mr), NULL, &mr);
	    MenuItemIndex idx;
	    GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex,
			      NULL, sizeof(idx), NULL, &idx);
	    QMenuBar::activate(mr, idx, TRUE);
	} else {
	    handled_event = FALSE;
	}
#else
	handled_event = FALSE;
#endif
	break;
    case kAppearanceEventClass:
	if(ekind == kAEAppearanceChanged && QApplication::app_style) {
	    QEvent ev(QEvent::Style);
	    QApplication::sendSpontaneousEvent(QApplication::app_style, &ev);
	} else {
	    handled_event = FALSE;
	}
	break;
    case kEventClassAppleEvent:
	handled_event = FALSE;
	if(ekind == kEventAppleEvent) {
	    OSType aeID, aeClass;
	    GetEventParameter(event, kEventParamAEEventClass, typeType,
			      NULL, sizeof(aeClass), NULL, &aeClass);
	    GetEventParameter(event, kEventParamAEEventID, typeType,
			      NULL, sizeof(aeID), NULL, &aeID);
	    if(aeClass == kCoreEventClass) {
		switch(aeID) {
		case kAEQuitApplication:
		    handled_event = TRUE;
		    app->closeAllWindows();
		    app->quit();
		    break;
		default:
		    break;
		}
	    }
	}
	break;
    case kEventClassCommand:
	if(ekind == kEventCommandProcess) {
	    UInt32 keyc;
	    HICommand cmd;
	    GetEventParameter(event, kEventParamDirectObject, typeHICommand,
			      NULL, sizeof(cmd), NULL, &cmd);
	    bool gotmod = !GetEventParameter(event, kEventParamKeyModifiers, typeUInt32,
					     NULL, sizeof(keyc), NULL, &keyc);

#if !defined(QMAC_QMENUBAR_NO_NATIVE) //offer it to the menubar..
	    if(!QMenuBar::activateCommand(cmd.commandID))
		QMenuBar::activate(cmd.menu.menuRef, cmd.menu.menuItemIndex, FALSE, gotmod && keyc);
#else
	    if(cmd.commandID == kHICommandQuit) {
		qApp->closeAllWindows();
		qApp->quit();
	    } else if(cmd.commandID == kHICommandAbout) {
		qmessageBox::aboutQt(NULL);
	    } else {
		handled_event = FALSE;
	    }
#endif
	} else {
	    handled_event = FALSE;
	}
	break;
    }

    // ok we clear all QtRequestContext events from the queue
    if(remove_context_timer) {
	if(mac_context_timer) {
	    RemoveEventLoopTimer(mac_context_timer);
	    mac_context_timer = NULL;
	}
	if(request_context_pending) {
	    request_context_pending = FALSE;
	    EventRef er;
	    const EventTypeSpec eventspec = { kEventClassQt, kEventQtRequestContext };
	    for(;;) {
		OSStatus ret = ReceiveNextEvent(1, &eventspec, QMAC_EVENT_NOWAIT, TRUE, &er);
		if(ret == eventLoopTimedOutErr || ret == eventLoopQuitErr)
		    break;
		ReleaseEvent(er);
	    }
	}
    }

    if(!handled_event) //let the event go through
	return CallNextEventHandler(er, event);
    return noErr; //we eat the event
}

void QApplication::processEvents(int maxtime)
{
    QTime now;
    QTime start = QTime::currentTime();
    while (!quit_now && processNextEvent(FALSE)) {
	now = QTime::currentTime();
	if (start.msecsTo(now) > maxtime)
	    break;
    }
}

extern uint qGlobalPostedEventsCount();

bool QApplication::hasPendingEvents()
{
    return qGlobalPostedEventsCount() || GetNumEventsInQueue(GetMainEventQueue());
}

/*!
  This virtual function is only implemented under Macintosh.

  If you create an application that inherits QApplication and
  reimplement this function, you get direct access to all Carbon Events
  that are received from the MacOS.

  Return TRUE if you want to stop the event from being processed.
  Return FALSE for normal event dispatching.
*/
bool QApplication::macEventFilter(EventRef)
{
    return FALSE;
}

void QApplication::openPopup(QWidget *popup)
{
    if (!popupWidgets) {			// create list
	popupWidgets = new QWidgetList;
	Q_CHECK_PTR(popupWidgets);
	if (!activeBeforePopup)
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
    }
    popupWidgets->append( popup );		// add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason( QFocusEvent::Popup );
    if (popup->focusWidget())
	popup->focusWidget()->setFocus();
    else
	popup->setFocus();
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
    qt_mac_dblclick.use_qt_time_limit = 1;
    mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time; //FIXME: What is the default value on the Mac?
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
    case UI_FadeMenu:
	fade_menu = enable;
	if( !enable )
	    break;
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeTooltip:
	fade_tooltip = enable;
	if( !enable )
	    break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
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
	if ( QColor::numBitPlanes() < 16 )
	    return FALSE;
	return fade_menu;
    case UI_AnimateCombo:
	return animate_combo;
    case UI_AnimateTooltip:
	return animate_tooltip;
    case UI_FadeTooltip:
	if ( QColor::numBitPlanes() < 16 )
	    return FALSE;
	return fade_tooltip;
    default:
	return animate_ui;
    }
}

void QApplication::flush()
{
//    sendPostedEvents();
    if(qApp) {
	if(QWidgetList *list = qApp->topLevelWidgets()) {
	    for ( QWidget *widget = list->first(); widget; widget = list->next() ) {
		if(widget->isVisible()) {
		    widget->propagateUpdates();
		    QMacSavedPortInfo::flush(widget);
		}
	    }
	    delete list;
	}
    }
}

bool QApplication::qt_mac_apply_settings()
{
    QSettings settings;

    /*
      Qt settings.  This is how they are written into the datastream.
      /qt/Palette/ *             - QPalette
      /qt/font                   - QFont
      /qt/libraryPath            - QStringList
      /qt/style                  - QString
      /qt/doubleClickInterval    - int
      /qt/cursorFlashTime        - int
      /qt/wheelScrollLines       - int
      /qt/colorSpec              - QString
      /qt/defaultCodec           - QString
      /qt/globalStrut            - QSize
      /qt/GUIEffects             - QStringList
      /qt/Font Substitutions/ *  - QStringList
      /qt/Font Substitutions/... - QStringList
    */

    QString str;
    QStringList strlist;
    int i, num;
    QPalette pal(QApplication::palette());
    strlist = settings.readListEntry("/qt/Palette/active");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Active, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }
    strlist = settings.readListEntry("/qt/Palette/inactive");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Inactive, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }
    strlist = settings.readListEntry("/qt/Palette/disabled");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Disabled, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }

    if ( pal != QApplication::palette())
	QApplication::setPalette(pal, TRUE);

    QFont font(QApplication::font());
    // read new font
    str = settings.readEntry("/qt/font");
    if (! str.isNull() && ! str.isEmpty()) {
	font.fromString(str);

	if (font != QApplication::font())
	    QApplication::setFont(font, TRUE);
    }

    // read library (ie. plugin) path list
    QStringList pathlist = settings.readListEntry("/qt/libraryPath", ':');
    if (! pathlist.isEmpty()) {
	QStringList::ConstIterator it = pathlist.begin();
	while (it != pathlist.end())
	    QApplication::addLibraryPath(*it++);
    }

    // read new QStyle
    QString stylename = settings.readEntry("/qt/style");
    if (! stylename.isNull() && ! stylename.isEmpty()) {
	QStyle *style = QStyleFactory::create(stylename);
	if (style)
	    QApplication::setStyle(style);
	else
	    stylename = "default";
    } else
	stylename = "default";

    num = settings.readNumEntry("/qt/doubleClickInterval",
				QApplication::doubleClickInterval());
    if(num != QApplication::doubleClickInterval())
	QApplication::setDoubleClickInterval(num);

    num = settings.readNumEntry("/qt/cursorFlashTime",
				QApplication::cursorFlashTime());
    QApplication::setCursorFlashTime(num);

    num = settings.readNumEntry("/qt/wheelScrollLines",
				QApplication::wheelScrollLines());
    QApplication::setWheelScrollLines(num);

    QString colorspec = settings.readEntry("/qt/colorSpec",
					   "default");
    if (colorspec == "normal")
	QApplication::setColorSpec(QApplication::NormalColor);
    else if (colorspec == "custom")
	QApplication::setColorSpec(QApplication::CustomColor);
    else if (colorspec == "many")
	QApplication::setColorSpec(QApplication::ManyColor);
    else if (colorspec != "default")
	colorspec = "default";

    QString defaultcodec = settings.readEntry("/qt/defaultCodec",
					      "none");
    if (defaultcodec != "none") {
	QTextCodec *codec = QTextCodec::codecForName(defaultcodec);
	if (codec)
	    qApp->setDefaultCodec(codec);
    }

    QStringList strut = settings.readListEntry("/qt/globalStrut");
    if (! strut.isEmpty()) {
	if (strut.count() == 2) {
	    QSize sz(strut[0].toUInt(), strut[1].toUInt());

	    if (sz.isValid())
		QApplication::setGlobalStrut(sz);
	}
    }

    QStringList effects = settings.readListEntry("/qt/GUIEffects");
    if (! effects.isEmpty()) {
	if ( effects.contains("none") )
	    QApplication::setEffectEnabled( Qt::UI_General, FALSE);
	if ( effects.contains("general") )
	    QApplication::setEffectEnabled( Qt::UI_General, TRUE );
	if ( effects.contains("animatemenu") )
	    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, TRUE );
	if ( effects.contains("fademenu") )
	    QApplication::setEffectEnabled( Qt::UI_FadeMenu, TRUE );
	if ( effects.contains("animatecombo") )
	    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, TRUE );
	if ( effects.contains("animatetooltip") )
	    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, TRUE );
	if ( effects.contains("fadetooltip") )
	    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, TRUE );
    } else
	QApplication::setEffectEnabled( Qt::UI_General, FALSE);

    QStringList fontsubs =
	settings.entryList("/qt/Font Substitutions");
    if (!fontsubs.isEmpty()) {
	QStringList subs;
	QString fam, skey;
	QStringList::Iterator it = fontsubs.begin();
	while (it != fontsubs.end()) {
	    fam = (*it++).latin1();
	    skey = "/qt/Font Substitutions/" + fam;
	    subs = settings.readListEntry(skey);
	    QFont::insertSubstitutions(fam, subs);
	}
    }

    qt_resolve_symlinks =
	settings.readBoolEntry("/qt/resolveSymlinks", TRUE);

    return TRUE;
}

bool QMacBlockingFunction::block = FALSE;
QMacBlockingFunction::QMacBlockingFunction()
{
    if(block)
	qWarning("QMacBlockingFunction is a non-recursive function");
    block = TRUE;
    startTimer(1);
}

void QMacBlockingFunction::timerEvent(QTimerEvent *)
{
    if(qt_activate_timers(TimerInfo::TIMER_ZERO))
	QApplication::flush();
}

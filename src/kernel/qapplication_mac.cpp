/****************************************************************************
** $Id$
**
** Implementation of Mac startup routines and event handling
**
** Created : 001018
**
** Copyrigght (C) 1992-2000 Trolltech AS.  All rights reserved.
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
#ifndef QT_NO_STYLE_AQUA
#include "qaquastyle.h"
#endif

#if !defined(QMAC_QMENUBAR_NO_NATIVE)
#include "qmenubar.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


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

//for qt_mac.h
QPaintDevice *qt_mac_safe_pdev = 0;
QPtrList<QMacSavedPortInfo> QMacSavedPortInfo::gports;
#ifdef QT_THREAD_SUPPORT
QMutex *qt_mac_port_mutex = NULL;
#endif

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static int mouse_button_state = 0;
static bool	app_do_modal	= FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;		// stack of modal widgets
static char    *appName;                        // application name
static Qt::HANDLE currentCursor;                  //current cursor
QWidget	       *qt_button_down	 = 0;		// widget got last button-down
QWidget        *qt_mouseover = 0;
QPtrDict<void> unhandled_dialogs;             //all unhandled dialogs (ie mac file dialog)
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif
static EventLoopTimerRef mac_select_timer = NULL;
static EventLoopTimerUPP mac_select_timerUPP = NULL;
static EventHandlerRef app_proc_handler = NULL;
static EventHandlerUPP app_proc_handlerUPP = NULL;
//popup variables
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;

/*****************************************************************************
  External functions
 *****************************************************************************/
// Paint event clipping magic - qpainter_mac.cpp
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping(QPaintDevice *dev);
extern void qt_mac_set_cursor(const QCursor *); //Cursor switching - qcursor_mac.cpp
QCString p2qstring(const unsigned char *); //qglobal.cpp

//special case popup handlers - look where these are used, they are very hacky,
//and very special case, if you plan on using these variables be VERY careful!!
static bool qt_closed_popup = FALSE;
static EventRef qt_replay_event = NULL;

//Input Method stuff
class QMacInputMethod {
    bool composing;
    QWidget *widget;
    TSMDocumentID id;
public:
    QMacInputMethod() : composing(FALSE), widget(NULL), id(0) { }
    ~QMacInputMethod() { endCompose(); }

    QWidget *getWidget() const { return composing ? widget : NULL; }
    void endCompose(bool fix=TRUE);
    void startCompose(QWidget *);
};
void QMacInputMethod::endCompose(bool fix)
{
    if(!composing)
	return;
    if(widget && fix) {
	FixTSMDocument(id); //finish what they were doing
	QIMEvent event(QEvent::IMEnd, QString::null, -1);
	QApplication::sendSpontaneousEvent(widget, &event);
    }
    widget = NULL;

    if(id) {
	DeleteTSMDocument(id);
	DeactivateTSMDocument(id);
	id = 0;
    }
}
void QMacInputMethod::startCompose(QWidget *w)
{
    if(composing && w == widget)
	return;
    endCompose();
    composing = TRUE;
    widget = w;
    OSType doc = kTextService;
    OSErr err = NewTSMDocument(1, &doc, &id, (long int)w);
    if(err != noErr) {
	qDebug("%s:%d %d", __FILE__, __LINE__, err);
    } else {
	UseInputWindow(id, true); //inline
	err = ActivateTSMDocument(id);
	if(err != noErr)
	    qDebug("%s:%d %d", __FILE__, __LINE__, err);
	else
	    qDebug("all systems go..");
    }
}
static QMacInputMethod qt_app_im;

void qt_mac_destroy_widget(QWidget *w)
{
    if(qt_button_down == w)
	qt_button_down = NULL;
    if(qt_mouseover == w)
	qt_mouseover = NULL;
    if(w == qt_app_im.getWidget())
	qt_app_im.endCompose(FALSE);
}

bool qt_nograb()				// application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}

//pre/post select callbacks
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
    typeQWidget = 1,  /* QWidget *  */
    typeTimerInfo = 2, /* TimerInfo * */
    //params
    kEventParamTimer = 'qtim',     /* typeTimerInfo */
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    //events
    kEventQtRequestPropagate = 10,
    kEventQtRequestSelect = 11,
    kEventQtRequestContext = 12,
#ifndef QMAC_QMENUBAR_NO_NATIVE
    kEventQtRequestMenubarUpdate = 13,
#endif
    kEventQtRequestTimer = 14
};
void qt_event_request_updates(QWidget *w, QRegion &r)
{
    w->createExtra();
    if(w->extra->has_dirty_area) {
	w->extra->dirty_area |= r;	
	return;
    }

    w->extra->has_dirty_area = TRUE;
    w->extra->dirty_area = r;

    EventRef upd = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestPropagate, GetCurrentEventTime(),
		kEventAttributeUserEvent, &upd);
    SetEventParameter(upd, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue( GetCurrentEventQueue(), upd, kEventPriorityStandard );
}
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
    PostEventToQueue( GetCurrentEventQueue(), upd, kEventPriorityHigh );
}
#endif

static const int non_gui_event_count = 4;
static EventTypeSpec events[] = {
    /* Since non-gui Qt is a subset of gui qt app I put the non-GUI
       events at the top and only pass those in as part of the event
       handler, if you add more to the top you must increase the 
       non_gui_event_count
    */
    { kEventClassQt, kEventQtRequestMenubarUpdate },
    { kEventClassQt, kEventQtRequestSelect },
    { kEventClassQt, kEventQtRequestContext },
    { kEventClassQt, kEventQtRequestTimer },
    { kEventClassQt, kEventQtRequestPropagate },

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

    { kEventClassKeyboard, kEventRawKeyUp },
    { kEventClassKeyboard, kEventRawKeyDown },
    { kEventClassKeyboard, kEventRawKeyRepeat },

    { kEventClassApplication, kEventAppActivated },
    { kEventClassApplication, kEventAppDeactivated },

    { kEventClassMenu, kEventMenuOpening },
    { kEventClassMenu, kEventMenuTargetItem },

    { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
    { kEventClassTextInput, kEventTextInputUpdateActiveInputArea },
    { kEventClassTextInput, kEventTextInputOffsetToPos },
    { kEventClassTextInput, kEventTextInputShowHideBottomWindow },

    { kEventClassCommand, kEventCommandProcess },
    { kAppearanceEventClass, kAEAppearanceChanged }
};

/* platform specific implementations */
void qt_init( int* argcptr, char **argv, QApplication::Type )
{
    // Get command line params
    int argc = *argcptr;
    int i, j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
#if defined(QT_DEBUG)
	if ( arg == "-nograb" )
	    appNoGrab = !appNoGrab;
	else
#endif // QT_DEBUG
#ifdef Q_WS_MACX
	//just ignore it, this seems to be passed from the finder (no clue what it does) FIXME
	    if( arg.left(5) == "-psn_"); 
	else
#endif
	    argv[j++] = argv[i];
    }
    *argcptr = j;

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

    qApp->setName( appName );
    if ( qt_is_gui_used ) {
#ifdef Q_WS_MACX
	ProcessSerialNumber psn;
	GetCurrentProcess(&psn);
	SetFrontProcess(&psn);
#endif
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	QMenuBar::initialize();
#endif
	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();

	{ //create an empty widget on startup and this can be used for a port anytime
	    QWidget *tlw = new QWidget(NULL, "empty_widget", Qt::WDestructiveClose);
	    tlw->hide();
	    qt_mac_safe_pdev = tlw;
	}
#if defined(QT_THREAD_SUPPORT)
	qt_mac_port_mutex = new QMutex(TRUE);
#endif
	RegisterAppearanceClient();
    }

    if(!app_proc_handler) {
	app_proc_handlerUPP = NewEventHandlerUPP(QApplication::globalEventProcessor);
	InstallEventHandler( GetApplicationEventTarget(), app_proc_handlerUPP,
			     qt_is_gui_used ? GetEventTypeCount(events) : non_gui_event_count, 
			     events, (void *)qApp, &app_proc_handler);
    }

#ifndef QT_NO_STYLE_AQUA
    if(qt_is_gui_used)
	QAquaStyle::appearanceChanged();
#endif
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

    if(qt_mac_safe_pdev) {
        delete qt_mac_safe_pdev;
        qt_mac_safe_pdev = NULL;
    }
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

typedef QPtrList<QCursor> QCursorList;
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
		if(x>=wx && y>=wy && x<=wx2 && y<=wy2) 
		    return recursive_match(curwidg,x-wx,y-wy);
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
    if( qt_is_gui_used )
	qApp->unlock();
#endif
    enter_loop();
    return quit_code;
}

/* timer code */
struct TimerInfo {
    QObject *obj;
    bool pending;

    enum { TIMER_ZERO, TIMER_MAC } type;
    EventLoopTimerRef mac_timer;
    int id;
};
static int zero_timer_count = 0;
typedef QPtrList<TimerInfo> TimerList;	// list of TimerInfo structs
static TimerList *timerList	= 0;		// timer list
static EventLoopTimerUPP timerUPP = NULL;       //UPP

/* timer call back */
QMAC_PASCAL static void qt_activate_timers(EventLoopTimerRef, void *data)
{
    TimerInfo *tmr = ((TimerInfo *)data);
    if(tmr->pending)
	return;
    tmr->pending = TRUE;
    EventRef tmr_ev = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestTimer, GetCurrentEventTime(),
		kEventAttributeUserEvent, &tmr_ev );
    SetEventParameter(tmr_ev, kEventParamTimer, typeTimerInfo, sizeof(tmr), &tmr);
    PostEventToQueue( GetCurrentEventQueue(), tmr_ev, kEventPriorityStandard );
}

//central cleanup
QMAC_PASCAL static Boolean find_timer_event(EventRef event, void *d)
{
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
	RemoveEventLoopTimer(t->mac_timer);
	if(t->pending) {
	    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
	    FlushSpecificEventsFromQueue(GetCurrentEventQueue(), fnc, (void *)t);
	    DisposeEventComparatorUPP(fnc);
	}
    } else {
	zero_timer_count--;
    }
    return remove ? timerList->remove() : TRUE;
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

static int activateNullTimers()
{
    if(!zero_timer_count)
	return 0;

    int ret = 0;
    for(register TimerInfo *t = timerList->first();
	ret != zero_timer_count && t; t = timerList->next()) {
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
    t->obj = obj;
    t->pending = TRUE;
    Q_CHECK_PTR( t );
    if(interval == 0) {
	t->type = TimerInfo::TIMER_ZERO;
	t->mac_timer = NULL;
	zero_timer_count++;
    } else {
	t->type = TimerInfo::TIMER_MAC;
	EventTimerInterval mint = (((EventTimerInterval)interval) / 1000);
	if(InstallEventLoopTimer(GetMainEventLoop(), mint, mint, timerUPP, t, &t->mac_timer) ) {
	    delete t;
	    return 0;
	}

    }
    static int serial_id = 666;
    t->id = serial_id++;
    timerList->append(t);
    t->pending = FALSE;
    return t->id;
}

bool qKillTimer( int id )
{
    if ( !timerList || id <= 0)
	return FALSE;				// not init'd or invalid timer
    register TimerInfo *t = timerList->first();
    while ( t && (t->id != id) ) // find timer info in list
	t = timerList->next();
    if ( t )					// id found
	return killTimer(t);
    return FALSE; // id not found
}

bool qKillTimer( QObject *obj )
{
    if ( !timerList )				// not initialized
	return FALSE;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
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

	if(!mac_select_timer) {
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
    //TrackDrag says you may not use the EventManager things..
    extern bool qt_mac_in_drag; //qdnd_mac.cpp
    if(qt_mac_in_drag) {
	qWarning("Whoa! Cannot process events whilst dragging!");
	return FALSE;
    }

    //ok to carry on
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif

    if(qt_is_gui_used && qt_replay_event) {	//ick
	EventRef ev = qt_replay_event;
	qt_replay_event = NULL;
	SendEventToWindow(ev, (WindowPtr)qt_mac_safe_pdev->handle());
	ReleaseEvent(ev);
    }

    sendPostedEvents();
    activateNullTimers(); //try to send null timers..

    EventRef event;
    OSStatus ret;
    do {
	do {
	    ret = ReceiveNextEvent( 0, 0, QMAC_EVENT_NOWAIT, TRUE, &event );
	    if(ret != noErr)
		break;
	    if((qt_is_gui_used && SendEventToWindow(event, (WindowPtr)qt_mac_safe_pdev->handle())) ||
	       (!qt_is_gui_used && SendEventToApplication(event)))
		nevents++;
	    ReleaseEvent(event);
	} while(GetNumEventsInQueue(GetCurrentEventQueue()));
	sendPostedEvents();
    } while(GetNumEventsInQueue(GetCurrentEventQueue()));

    if( quit_now || app_exit_loop ) {
#if defined(QT_THREAD_SUPPORT)
	qApp->unlock( FALSE );
#endif
	return FALSE;
    }

    if(canWait && !zero_timer_count) {
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

static int get_key(int key, int scan)
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


bool QApplication::do_mouse_down( Point *pt )
{
    WindowPtr wp;
    short windowPart;
    windowPart = FindWindow( *pt, &wp );
    QWidget *widget = QWidget::find( (WId)wp );
    bool in_widget = FALSE;

    switch( windowPart ) {
    case inDesk:
	break;
    case inGoAway:
	if( widget ) {
	    widget->close();
	} else {
	    qWarning("Close for unknown widget");
	}
	break;
    case 13: { //hide toolbars thing
	if(widget) {
	    if(const QObjectList *chldrn = widget->children()) {
		for(QObjectListIt it(*chldrn); it.current(); ++it) {
		    if(it.current()->isWidgetType() && it.current()->inherits("QDockArea")) {
			QWidget *w = (QWidget *)it.current();
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
			int nh = w->sizeHint().height();
			if(nh < 0)
			    nh = 0;
			if(oh != nh)
			    widget->resize(widget->width(), widget->height() - (oh - nh));
		    }
		}
	    }
	}
	break; }
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
	SetRect( &limits, -2, 0, 0, 0 );

	if( widget ) {
	    int wstrut = 0, hstrut = 0;
	    if(widget->fstrut_dirty)
		widget->updateFrameStrut();
	    if(QTLWExtra *tlextra = widget->topData()) {
		wstrut = tlextra->fleft + tlextra->fright;
		hstrut = tlextra->ftop + tlextra->fbottom;
	    }
	    if(QWExtra   *extra = widget->extraData()) 
		SetRect( &limits, extra->minw+wstrut, extra->minh+hstrut,
			 extra->maxw+wstrut < QWIDGETSIZE_MAX ? extra->maxw+wstrut : QWIDGETSIZE_MAX,
			 extra->maxh+hstrut < QWIDGETSIZE_MAX ? extra->maxh+hstrut : QWIDGETSIZE_MAX);
	}
	int growWindowSize = GrowWindow( wp, *pt, limits.left == -2 ? NULL : &limits);
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
QApplication::globalEventProcessor(EventHandlerCallRef er, EventRef event, void *data)
{
    QApplication *app = (QApplication *)data;
    if ( app->macEventFilter( event ) ) //someone else ate it
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
	if(ekind == kEventQtRequestPropagate) {
	    QWidget *widget = NULL;
	    GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
			      sizeof(widget), NULL, &widget);
	    if(widget && widget->extra && widget->extra->has_dirty_area) {
		widget->extra->has_dirty_area = FALSE;
		if(!widget->extra->dirty_area.isEmpty())
		    widget->repaint(widget->extra->dirty_area);
	    } else {
		request_updates_pending = FALSE;
		QApplication::sendPostedEvents();
		if(QWidgetList *list   = qApp->topLevelWidgets()) {
		    for ( QWidget     *widget = list->first(); widget; widget = list->next() ) {
			if ( !widget->isHidden() )
			    widget->propagateUpdates();
		    }
		    delete list;
		}
	    } 
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	} else if(ekind == kEventQtRequestMenubarUpdate) {
	    request_menubarupdate_pending = FALSE;
	    QMenuBar::macUpdateMenuBar();
#endif
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
		    if(qt_button_down && mouse_button_state) {
			QMouseEvent qme( QEvent::MouseButtonRelease, plocal, where, 
					 mouse_button_state, mouse_button_state );
			QApplication::sendSpontaneousEvent( qt_button_down, &qme );
		    }
		    qt_button_down = NULL;
		    mouse_button_state = 0;
		} 
	    } else {
		handled_event = FALSE;
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
	    if(!(count % 2))
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
	    wheel_delta = mdelt * 120;
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
		QApplication::sendSpontaneousEvent( popupwidget, &cme );
		was_context = cme.isAccepted();
	    }
	    if(!was_context) {
		if(wheel_delta) {
		    QWheelEvent qwe( plocal, p, wheel_delta, state | keys);
		    QApplication::sendSpontaneousEvent( popupwidget, &qwe);
		} else {
		    QMouseEvent qme( etype, plocal, p, button | keys, state | keys );
		    QApplication::sendSpontaneousEvent( popupwidget, &qme );
		}
		if(app->activePopupWidget() != popupwidget && qt_closed_popup)
		    special_close = TRUE;
	    }
	    if(special_close) { 	    //We will resend this event later, so just return
		qt_replay_event = CopyEvent(event);
		return noErr;
	    }
	}

	//figure out which widget to send it to
	if( ekind != kEventMouseDown && qt_button_down )
	    widget = qt_button_down;
	else if( mac_mouse_grabber )
	    widget = mac_mouse_grabber;
	else
	    widget = QApplication::widgetAt( where.h, where.v, true );

	//This mouse button state stuff looks like this on purpose
	//although it looks hacky it is VERY intentional..
	if ( widget && app_do_modal && !qt_try_modal(widget, event) ) {
	    mouse_button_state = after_state;
	    return 1;
	}

	if(ekind == kEventMouseDown) {
	    if(QWidget* w = widget) {
		while ( w->focusProxy() )
		    w = w->focusProxy();
		if(ekind == kEventMouseDown) {
		    if(QWidget *tlw = w->topLevelWidget()) {
			tlw->raise();
			if(tlw->isTopLevel() && !tlw->isDesktop() && !tlw->isPopup() && 
			   (tlw->isModal() || !tlw->inherits("QDockWindow")))
			    tlw->setActiveWindow();
		    }
		}
	    }
	    if(!app->do_mouse_down( &where )) {
		mouse_button_state = 0;
		return 0;
	    }
	}
	mouse_button_state = after_state;

	switch(ekind) {
	case kEventMouseDragged:
	case kEventMouseMoved:
	{
	    //set the cursor up
	    const QCursor *n = NULL;
	    if(!widget) //not over the app, don't set a cursor..
		;
	    else if(widget->extra && widget->extra->curs)
		n = widget->extra->curs;
	    else if(cursorStack)
		n = app_cursor;
	    if(!n)
		n = &arrowCursor; //I give up..
	    if(currentCursor != n->handle()) {
		currentCursor = n->handle();
		qt_mac_set_cursor(n);
	    }
	    if ( qt_mouseover != widget ) {
#ifdef DEBUG_MOUSE_MAPS
		qDebug("Entering: %s (%s), Leaving %s (%s)", 
		       widget ? widget->className() : "none", widget ? widget->name() : "",
		       qt_mouseovert ? qt_mouseover->className() : "none", 
		       qt_mouseover ? qt_mouseover->name() : "");
#endif
		qt_dispatchEnterLeave( widget, qt_mouseover );
		qt_mouseover = widget;
	    }
	    break;
	}
	case kEventMouseDown:
	    qt_button_down = widget;
	    break;
	case kEventMouseUp:
	    qt_button_down = NULL;
	    break;
	}

	//finally send the event to the widget if its not the popup
	if ( widget && widget != popupwidget ) {
	    if(ekind == kEventMouseDown || ekind == kEventMouseWheelMoved) {
		if(popupwidget) //guess we close the popup...
		    popupwidget->close();

		QWidget* w = widget;
		while ( w->focusProxy() )
		    w = w->focusProxy();
		int fp = (ekind == kEventMouseDown) ? QWidget::ClickFocus : QWidget::WheelFocus;
		if ( w->focusPolicy() & fp) {
		    QFocusEvent::setReason( QFocusEvent::Mouse);
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
		QApplication::sendSpontaneousEvent( widget, &cme );
		was_context = cme.isAccepted();
	    }
	    if(!was_context) {
#ifdef DEBUG_MOUSE_MAPS
		char *desc = NULL;
		switch(ekind) {
		case kEventMouseDown: desc = "MouseButtonPress"; break;
		case kEventMouseUp: desc = "MouseButtonRelease"; break;
		case kEventMouseDragged: case kEventMouseMoved: desc = "MouseMove"; break;
		case kEventMouseWheelMoved: desc = "MouseWheelMove"; break;
		}
		qDebug("%d %d (%d %d) - Would send (%s) event to %s %s (%d %d %d)", p.x(), p.y(),
		       plocal.x(), plocal.y(), desc, widget->name(), widget->className(),
		       button|keys, state|keys, wheel_delta);
#endif
		if(wheel_delta) {
		    QWheelEvent qwe( plocal, p, wheel_delta, state | keys);
		    QApplication::sendSpontaneousEvent( widget, &qwe);
		    if(!qwe.isAccepted() && focus_widget && focus_widget != widget) {
			QWheelEvent qwe2( focus_widget->mapFromGlobal( p ), p, 
					  wheel_delta, state | keys );
			QApplication::sendSpontaneousEvent( focus_widget, &qwe2 );
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
		    QMouseEvent qme( etype, plocal, p, button | keys, state | keys );
		    QApplication::sendSpontaneousEvent( widget, &qme );
		}
		if(etype == QEvent::MouseButtonPress && 
		   button == QMouseEvent::LeftButton && !mac_trap_context) {
		    remove_context_timer = FALSE;
		    InstallEventLoopTimer(GetMainEventLoop(), 2, 0,
					  NewEventLoopTimerUPP(qt_trap_context_mouse), widget,
					  &mac_trap_context);
		}
	    }
	} else {
	    handled_event = FALSE;
	}
	break;
    }
    case kEventClassTextInput:
#if 0 //no need for text input since it doesn't really work yet..
	if(!(widget=focus_widget))
	    return 1; //no use to me!
	if(ekind == kEventTextInputShowHideBottomWindow) {
	    Boolean tmp = false;
	    GetEventParameter(event, kEventParamTextInputSendShowHide, typeBoolean, NULL, sizeof(tmp), NULL, &tmp);
	    qDebug("%d", tmp);

	    tmp = false;
	    SetEventParameter(event, kEventParamTextInputReplyShowHide, typeBoolean, sizeof(tmp), &tmp);
	} else if(ekind == kEventTextInputUnicodeForKeyEvent) {
	    QIMEvent imstart(QEvent::IMStart, QString::null, -1);
	    QApplication::sendSpontaneousEvent(widget, &imstart);
	    if(!imstart.isAccepted()) //doesn't want the event
		return 1;
	    EventRef key_ev;
	    GetEventParameter(event, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL,
			      sizeof(key_ev), NULL, &key_ev);
	    UniChar unicode;
	    GetEventParameter(key_ev, kEventParamKeyUnicodes, typeUnicodeText, NULL,
			      sizeof(unicode), NULL, &unicode);
	    QString text = QChar(unicode);
	    QIMEvent imend(QEvent::IMEnd, text, 1);
	    QApplication::sendSpontaneousEvent(widget, &imend);
	} else {
	    handled_event = FALSE;
	}
#else
	handled_event = FALSE;
#endif
	break;

    case kEventClassKeyboard: {
	char chr;
	GetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(chr), NULL, &chr);
	if(!chr)
	    break;
	UInt32 modif;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modif), NULL, &modif);
	int modifiers = get_modifiers(modif);
	UInt32 keyc;
	GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyc), NULL, &keyc);
	int mychar=get_key(chr, keyc);

	static QTextCodec *c = NULL;
	if(!c)
	    c = QTextCodec::codecForName("Apple Roman");
       	QString mystr = c->toUnicode(&chr, 1);
	QEvent::Type etype = (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress;
	if( mac_keyboard_grabber )
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;
	if(widget) {
	    if ( app_do_modal && !qt_try_modal(widget, event) )
		return 1;

	    bool key_event = TRUE;
	    if(etype == QEvent::KeyPress && !mac_keyboard_grabber) {
		QKeyEvent aa(QEvent::AccelOverride, mychar, chr, modifiers, mystr, ekind == kEventRawKeyRepeat,
			     mystr.length());
		aa.ignore();
		QApplication::sendSpontaneousEvent( widget, &aa );
		if ( !aa.isAccepted() ) {
		    QKeyEvent a(QEvent::Accel, mychar, chr, modifiers, mystr, ekind == kEventRawKeyRepeat,
				mystr.length());
		    a.ignore();
		    QApplication::sendSpontaneousEvent( widget->topLevelWidget(), &a );
		    if ( a.isAccepted() ) {
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
			       QMenuBar::activate(hic.menu.menuRef, hic.menu.menuItemIndex)) 
				key_event = FALSE;
			    else
#endif
			    if(!ProcessHICommand(&hic)) 
				key_event = FALSE;
			}
		    }
		}
	    }
	    if(key_event) {
		if((modifiers & Qt::ControlButton) && mychar == Key_Space) { //eat it
		    if(etype == QEvent::KeyPress) {
			QIMEvent event(QEvent::IMStart, QString::null, -1);
			QApplication::sendSpontaneousEvent(widget, &event);
			if(event.isAccepted())
			    qt_app_im.startCompose(widget);
		    }
		} else {
		    if(modifiers & (Qt::ControlButton | Qt::AltButton)) {
			mystr = QString();
			chr = 0;
		    } 
		    QKeyEvent ke(etype,mychar, chr, modifiers,
				 mystr, ekind == kEventRawKeyRepeat, mystr.length());
		    QApplication::sendSpontaneousEvent(widget,&ke);
		}
	    }
	} else {
	    handled_event = FALSE;
	}
	break; }
    case kEventClassWindow: {
	WindowRef wid;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
			  sizeof(WindowRef), NULL, &wid);
	widget = QWidget::find( (WId)wid );

	if(!widget) {
	    if(ekind == kEventWindowShown )
		unhandled_dialogs.insert((void *)wid, (void *)1);
	    else if(ekind == kEventWindowHidden)
		unhandled_dialogs.remove((void *)wid);
#if 0
	    else if(!unhandled_dialogs.find((void *)wid))
		qWarning("Couldn't find EventClassWindow widget for %d %d", (int)wid, ekind);
	    else
		return 1;
#endif
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
		if(nx != ox ||  ny != oy) {
		    widget->crect.setRect( nx, ny, widget->width(), widget->height() );
		    QMoveEvent qme( widget->crect.topLeft(), QPoint( ox, oy) );
		    QApplication::sendSpontaneousEvent( widget, &qme );
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
#ifndef QT_NO_STYLE_AQUA
	    //I shouldn't have to do this, but the StyleChanged isn't happening as I expected
	    //so this is in for now, FIXME!
	    QAquaStyle::appearanceChanged();
#endif
	    if( app_do_modal && !qt_try_modal(widget, event) )
		return 1;

	    if(widget) {
		widget->raise();
		QWidget *tlw = widget->topLevelWidget();
		if(tlw->isTopLevel() && !tlw->isPopup() && (tlw->isModal() || !tlw->testWFlags( WStyle_Tool )))
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
	    while(app->inPopupMode())
		app->activePopupWidget()->close();
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
	}
#else
	handled_event = FALSE;
#endif
	break;
    case kAppearanceEventClass:
#ifndef QT_NO_STYLE_AQUA
	if(ekind == kAEAppearanceChanged) 
	    QAquaStyle::appearanceChanged();
	else
	    handled_event = FALSE;
#else
	handled_event = FALSE;
#endif
	break;
    case kEventClassCommand:
	if(ekind == kEventCommandProcess) {
	    HICommand cmd;
	    GetEventParameter(event, kEventParamDirectObject, typeHICommand, 
			      NULL, sizeof(cmd), NULL, &cmd);
#if !defined(QMAC_QMENUBAR_NO_NATIVE) //offer it to the menubar..
	    if(!QMenuBar::activateCommand(cmd.commandID) && 
	       !QMenuBar::activate(cmd.menu.menuRef, cmd.menu.menuItemIndex)) 
		handled_event = FALSE;
#else
	    if(cmd.commandID == kHICommandQuit)
		qApp->closeAllWindows();
	    else if(cmd.commandID == kHICommandAbout)
		QMessageBox::aboutQt(NULL);
	    else
		handled_event = FALSE;
#endif
	}
	break;
    }

    // ok we clear all QtRequestContext events from the queue
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

    if(!handled_event) //let the event go through
	return CallNextEventHandler(er, event);
    return noErr; //we eat the event
}

void QApplication::processEvents( int maxtime)
{
    QTime now;
    QTime start = QTime::currentTime();
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

/*!
  This virtual function is only implemented under Macintosh.

  If you create an application that inherits QApplication and
  reimplement this function, you get direct access to all Carbon Events
  that are received from the MacOS.

  Return TRUE if you want to stop the event from being processed, or
  return FALSE for normal event dispatching.
*/

bool QApplication::macEventFilter( EventRef )
{
    return FALSE;
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
    if(QWidgetList *list = qApp->topLevelWidgets()) {
	for ( QWidget *widget = list->first(); widget; widget = list->next() ) {
	    widget->propagateUpdates();
	    QMacSavedPortInfo::flush(widget);
	}
	delete list;
    }
}

#if 0
#if 1
#include <stdlib.h>
#else
#define free(x) 
void *malloc(int x) {
    static char *foo = NULL;
    static int off = 0;
    if(!foo)
	foo = (char *)malloc(1000000);
    char *ret = foo + off;
    off += x;
    return (void *)ret;
}
#endif
void* operator new[](size_t size) { return malloc(size); }
void* operator new(size_t size) { return malloc(size); }
void operator delete[](void *p) { free(p); }
void operator delete[](void *p, size_t) { free(p); }
void operator delete(void *p) { free(p); }
void operator delete(void *p, size_t) { free(p); }
#endif

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
#include <qeventloop.h>
#include "qmessagebox.h"

//#define QMAC_LAME_TIME_LIMITED
#ifdef QMAC_LAME_TIME_LIMITED
#  include <qtimer.h>
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
extern bool qt_tryAccelEvent( QWidget*, QKeyEvent* ); // def in qaccel.cpp
static QGuardedPtr<QWidget> qt_mouseover;
static QPtrDict<void> unhandled_dialogs;        //all unhandled dialogs (ie mac file dialog)
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif
static EventLoopTimerRef mac_context_timer = NULL;
static EventLoopTimerUPP mac_context_timerUPP = NULL;
static DMExtendedNotificationUPP mac_display_changeUPP = NULL;
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
extern void qt_mac_set_cursor(const QCursor *, const Point *); //Cursor switching - qcursor_mac.cpp
extern bool qt_mac_is_macsheet(QWidget *); //qwidget_mac.cpp
QCString p2qstring(const unsigned char *); //qglobal.cpp

//special case popup handlers - look where these are used, they are very hacky,
//and very special case, if you plan on using these variables be VERY careful!!
static bool qt_closed_popup = FALSE;
EventRef qt_replay_event = NULL;

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

/* Event masks */
// internal Qt types
const UInt32 kEventClassQt = 'cute';
enum {
    //types
    typeQWidget = 1,  /* QWidget *  */
    typeTimerInfo = 2, /* TimerInfo * */
    typeQEventLoop = 3, /* QEventLoop * */
    //params
    kEventParamTimer = 'qtim',     /* typeTimerInfo */
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    kEventParamQEventLoop = 'qlop', /* typeQEventLoop */
    //events
    kEventQtRequestPropagateWindowUpdates = 10,
    kEventQtRequestPropagateWidgetUpdates = 11,
    kEventQtRequestSelect = 12,
    kEventQtRequestContext = 13,
#ifndef QMAC_QMENUBAR_NO_NATIVE
    kEventQtRequestMenubarUpdate = 14,
#endif
    kEventQtRequestTimer = 15,
    kEventQtRequestWakeup = 16,
    kEventQtRequestShowSheet = 17
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
static bool request_select_pending = FALSE;
void qt_event_request_select(QEventLoop *loop) {
    if(request_select_pending)
	return;
    request_select_pending = TRUE;

    EventRef sel = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestSelect, GetCurrentEventTime(),
		kEventAttributeUserEvent, &sel);
    SetEventParameter(sel, kEventParamQEventLoop, typeQEventLoop, sizeof(loop), &loop);
    PostEventToQueue(GetMainEventQueue(), sel, kEventPriorityStandard);
    ReleaseEvent(sel);
}
void qt_event_request_showsheet(QWidget *w)
{
    EventRef ctx = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestShowSheet, GetCurrentEventTime(),
		kEventAttributeUserEvent, &ctx );
    SetEventParameter(ctx, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), ctx, kEventPriorityStandard);
    ReleaseEvent(ctx);
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
static bool request_wakeup_pending = FALSE;
void qt_event_request_wakeup()
{
    if(request_wakeup_pending)
	return;
    request_wakeup_pending = TRUE;
    EventRef upd = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestWakeup, GetCurrentEventTime(),
		kEventAttributeUserEvent, &upd);
    PostEventToQueue(GetMainEventQueue(), upd, kEventPriorityHigh);
    ReleaseEvent(upd);
}
void qt_event_request_timer(TimerInfo *tmr)
{
    EventRef tmr_ev = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestTimer, GetCurrentEventTime(),
		kEventAttributeUserEvent, &tmr_ev );
    SetEventParameter(tmr_ev, kEventParamTimer, typeTimerInfo, sizeof(tmr), &tmr);
    PostEventToQueue(GetMainEventQueue(), tmr_ev, kEventPriorityStandard);
    ReleaseEvent(tmr_ev);
}
TimerInfo *qt_event_get_timer(EventRef event)
{
    if(GetEventClass(event) != kEventClassQt || GetEventKind(event) != kEventQtRequestTimer)
	return NULL; //short circuit our tests..
    TimerInfo *t;
    GetEventParameter(event, kEventParamTimer, typeTimerInfo, NULL, sizeof(t), NULL, &t);
    return t;
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
    { kEventClassQt, kEventQtRequestShowSheet },
    { kEventClassQt, kEventQtRequestContext },
#ifndef QMAC_QMENUBAR_NO_NATIVE
    { kEventClassQt, kEventQtRequestMenubarUpdate },
#endif
    { kEventClassQt, kEventQtRequestPropagateWindowUpdates },
    { kEventClassQt, kEventQtRequestPropagateWidgetUpdates },

    { kEventClassWindow, kEventWindowUpdate },
    { kEventClassWindow, kEventWindowDrawContent },
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

    { kEventClassKeyboard, kEventRawKeyModifiersChanged },
    { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
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
#else
	DisableMenuCommand(NULL, kHICommandQuit);
#endif
	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();
#if defined(QT_THREAD_SUPPORT)
	QThread::initialize();
#endif

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
{ controlKey, MAP_KEY(Qt::MetaButton) },
{ rightControlKey, MAP_KEY(Qt::MetaButton) },
{ cmdKey, MAP_KEY(Qt::ControlButton) },
{ optionKey, MAP_KEY(Qt::AltButton) },
{ rightOptionKey, MAP_KEY(Qt::AltButton) },
{ kEventKeyModifierNumLockMask, MAP_KEY(Qt::Keypad) },
{   0, MAP_KEY(0) } };
static int get_modifiers(int key, bool from_mouse=FALSE)
{
#if !defined(DEBUG_KEY_MAPS) || defined(DEBUG_MOUSE_MAPS)
    Q_UNUSED(from_mouse);
#endif
#ifdef DEBUG_KEY_MAPS
#ifndef DEBUG_MOUSE_MAPS
	    if(!from_mouse)
#endif
		qDebug("**Mapping modifier: %d (0x%04x) -- %d", key, key, from_mouse);
#endif
    int ret = 0;
    for(int i = 0; modifier_syms[i].qt_code; i++) {
	if(key & modifier_syms[i].mac_code) {
#ifdef DEBUG_KEY_MAPS
#ifndef DEBUG_MOUSE_MAPS
	    if(!from_mouse)
#endif
		qDebug("%d: got modifier: %s", from_mouse, modifier_syms[i].desc);
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
    qDebug("**Mapping key: %d (0x%04x) - %d (0x%04x)", key, key, scan, scan);
#endif

    //special case for clear key
    if(key == kClearCharCode && scan == 0x47) {
#ifdef DEBUG_KEY_MAPS
	qDebug("%d: got key: Qt::Key_Clear", __LINE__);
#endif
	return Qt::Key_Clear;
    }

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

bool QApplication::do_mouse_down(Point *pt, bool *mouse_down_unhandled)
{
    QWidget *widget;
    short windowPart = qt_mac_find_window(pt->h, pt->v, &widget);
    if(mouse_down_unhandled)
	(*mouse_down_unhandled) = FALSE;
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
    if(windowPart == inMenuBar) {
	QMacBlockingFunction block;
	MenuSelect(*pt); //allow menu tracking
	return FALSE;
    } else
#endif
    if(!widget) {
	if(mouse_down_unhandled)
	    (*mouse_down_unhandled) = TRUE;
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
    case inGoAway: {
	widget->close();
	break; }
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
	widget->showNormal();
	break;
    case inZoomOut:
	widget->showMaximized();
	break;
    default:
	qDebug("Unhandled case in mouse_down.. %d", windowPart);
	break;
    }
    return FALSE;
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
    // a bit of a hack: use WStyle_Tool as a general ignore-modality
    // allow tool windows; disallow tear off popups
    if (widget->testWFlags(Qt::WStyle_Tool) && widget->inherits( "QPopupMenu"))
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
#if 0 //This is really different than Qt behaves, but it is correct for Aqua, what do I do? -Sam
    if(block_event && qt_mac_is_macsheet(top)) {
	for(QWidget *w = top->parentWidget(); w; w = w->parentWidget()) {
	    w = w->topLevelWidget();
	    if(w == widget || w->isModal())
		return FALSE;
	}
	return TRUE;
    }
#endif
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

bool qt_mac_send_event(QEventLoop::ProcessEventsFlags flags, EventRef event, WindowPtr pt)
{
    if(flags != QEventLoop::AllEvents ) {
	UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
	if(flags & QEventLoop::ExcludeUserInput) {
	    switch(eclass) {
	    case kEventClassQt:
		if(ekind == kEventQtRequestContext)
		    return FALSE;
		break;
	    case kEventClassMouse:
	    case kEventClassKeyboard:
		return FALSE;
	    }
	}
	if(flags & QEventLoop::ExcludeSocketNotifiers) {
	    switch(eclass) {
	    case kEventClassQt:
		if(ekind == kEventQtRequestSelect)
		    return FALSE;
		break;
	    }
	}
    }
    if(pt)
	return !SendEventToWindow(event, pt);
    return !SendEventToEventTarget(event, GetEventDispatcherTarget());
}

QMAC_PASCAL OSStatus
QApplication::globalEventProcessor(EventHandlerCallRef er, EventRef event, void *data)
{
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
	} else if(ekind == kEventQtRequestShowSheet) {
	    QWidget *widget = NULL;
	    GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
			      sizeof(widget), NULL, &widget);
	    if(widget) 
		ShowSheetWindow((WindowPtr)widget->hd, (WindowPtr)widget->parentWidget()->hd);
	} else if(ekind == kEventQtRequestWakeup) {
	    request_wakeup_pending = FALSE; 	    //do nothing else, we just woke up!
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	} else if(ekind == kEventQtRequestMenubarUpdate) {
	    request_menubarupdate_pending = FALSE;
	    QMenuBar::macUpdateMenuBar();
#endif
	} else if(ekind == kEventQtRequestSelect) {
	    request_select_pending = FALSE;
	    QEventLoop *l = NULL;
	    if(GetEventParameter(event, kEventParamQEventLoop, typeQEventLoop, NULL, sizeof(l), NULL, &l))
		l = app->eventLoop();
	    timeval tm;
	    memset(&tm, '\0', sizeof(tm));
	    l->macHandleSelect(&tm);
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
	    TimerInfo *t;
	    GetEventParameter(event, kEventParamTimer, typeTimerInfo, NULL, sizeof(t), NULL, &t);
	    app->eventLoop()->macHandleTimer(t);
	} else {
	    handled_event = FALSE;
	}
	break;
    case kEventClassMouse:
    {
#ifdef DEBUG_MOUSE_MAPS
	const char *edesc = NULL;
	switch(ekind) {
	case kEventMouseDown: edesc = "MouseButtonPress"; break;
	case kEventMouseUp: edesc = "MouseButtonRelease"; break;
	case kEventMouseDragged: case kEventMouseMoved: edesc = "MouseMove"; break;
	case kEventMouseWheelMoved: edesc = "MouseWheelMove"; break;
	}
#endif
	if( (ekind == kEventMouseDown && mouse_button_state ) ||
	    (ekind == kEventMouseUp && !mouse_button_state) ) {
#ifdef DEBUG_MOUSE_MAPS
	    qDebug("**** Dropping mouse event.. %s %d %p **** ",
		   edesc, mouse_button_state, (QWidget*)qt_button_down);
#endif
	    break;
	}
#ifdef DEBUG_MOUSE_MAPS
	else if(ekind == kEventMouseDown || ekind == kEventMouseUp) {
	    qDebug("Handling mouse: %s", edesc);
	}
#endif
	QEvent::Type etype = QEvent::None;
	UInt32 modifiers;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL,
			  sizeof(modifiers), NULL, &modifiers);
	int keys = get_modifiers(modifiers, TRUE);
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
	if(!QMacBlockingFunction::blocking()) { //set the cursor up
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
	}

	//This mouse button state stuff looks like this on purpose
	//although it looks hacky it is VERY intentional..
	if (widget && app_do_modal && !qt_try_modal(widget, event)) {
	    mouse_button_state = after_state;
	    if(ekind == kEventMouseDown && qt_mac_is_macsheet(activeModalWidget())) {
		activeModalWidget()->parentWidget()->setActiveWindow(); //sheets have a parent
		if(!app->do_mouse_down(&where, NULL))
		    mouse_button_state = 0;
	    }
#ifdef DEBUG_MOUSE_MAPS
	    qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__, mouse_button_state);
#endif
	    break;
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
		(button == QMouseEvent::LeftButton && (modifiers & controlKey)))) {
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
	    bool mouse_down_unhandled;
	    if(!app->do_mouse_down(&where, &mouse_down_unhandled)) {
		if(mouse_down_unhandled) {
		    handled_event = FALSE;
		    break;
		}
		mouse_button_state = 0;
#ifdef DEBUG_MOUSE_MAPS
		qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__, mouse_button_state);
#endif
		break;
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
		qDebug("Entering: %p - %s (%s), Leaving %s (%s)", widget,
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
		(button == QMouseEvent::LeftButton && (modifiers & controlKey)))) {
		QContextMenuEvent cme(QContextMenuEvent::Mouse, plocal, p, keys );
		QApplication::sendSpontaneousEvent(widget, &cme);
		was_context = cme.isAccepted();
	    }
#ifdef DEBUG_MOUSE_MAPS
	    const char *event_desc = edesc;
	    if(was_context)
		event_desc = "Context Menu";
	    else if(etype == QEvent::MouseButtonDblClick)
		event_desc = "Double Click";
	    qDebug("%d %d (%d %d) - Would send (%s) event to %p %s %s (%d %d %d)", p.x(), p.y(),
		   plocal.x(), plocal.y(), event_desc, widget, widget->name(),
		   widget->className(), button, state|keys, wheel_delta);
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
		{ controlKeyBit, MAP_KEY(Qt::Key_Meta) },
		{ rightControlKeyBit, MAP_KEY(Qt::Key_Meta) }, //???
		{ cmdKeyBit, MAP_KEY(Qt::Key_Control) },
		{ optionKeyBit, MAP_KEY(Qt::Key_Alt) },
		{ rightOptionKeyBit, MAP_KEY(Qt::Key_Alt) }, //???
		{ alphaLockBit, MAP_KEY(Qt::Key_CapsLock) },
		{ kEventKeyModifierNumLockBit, MAP_KEY(Qt::Key_NumLock) },
		{   0, MAP_KEY(0) } };
	    for(int i = 0; i <= 32; i++) { //just check each bit
		if(!(changed_modifiers & (1 << i)))
		    continue;
		QEvent::Type etype = QEvent::KeyPress;
		if(last_modifiers & (1 << i))
		    etype = QEvent::KeyRelease;
		int key = 0;
		for(uint x = 0; key_modif_syms[x].mac_code; x++) {
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

	UInt32 keyc;
	GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyc), NULL, &keyc);
	static UInt32 state = 0L;
	char chr = KeyTranslate((void *)GetScriptVariable(smCurrentScript, smKCHRCache),
		   (modif & (kEventKeyModifierNumLockMask|shiftKey|rightShiftKey|alphaLock)) | keyc, &state);
	if(!chr)
	    break;

	//map it into qt keys
	QString mystr;
	int modifiers = get_modifiers(modif), mychar=get_key(modifiers, chr, keyc);
	if(modifiers & (Qt::ControlButton | Qt::AltButton | Qt::MetaButton)) {
	    chr = 0;
	} else {  	//now get the real ascii value
	    UInt32 tmp_mod = 0L;
	    static UInt32 tmp_state = 0L;
	    if(modifiers & Qt::ShiftButton)
		tmp_mod |= shiftKey;
	    if(modifiers & Qt::MetaButton)
		tmp_mod |= controlKey;
	    if(modifiers & Qt::ControlButton)
		tmp_mod |= cmdKey;
	    if(modif & alphaLock)
		tmp_mod |= alphaLock;
	    if(modifiers & Qt::AltButton)
		tmp_mod |= optionKey;
	    if(modifiers & Qt::Keypad)
		tmp_mod |= kEventKeyModifierNumLockMask;
	    chr = KeyTranslate((void *)GetScriptManagerVariable(smUnicodeScript),
			       tmp_mod | keyc, &tmp_state);

	    static QTextCodec *c = NULL;
	    if(!c)
		c = QTextCodec::codecForName("Apple Roman");
	    mystr = c->toUnicode(&chr, 1);
	}

	QEvent::Type etype = (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress;
	if(mac_keyboard_grabber)
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;
	if(widget) {
	    if(app_do_modal && !qt_try_modal(widget, event))
		break;

	    bool key_event = TRUE;
	    if(etype == QEvent::KeyPress && !mac_keyboard_grabber) {
		QKeyEvent a(etype, mychar, chr, modifiers,
			     mystr, ekind == kEventRawKeyRepeat, mystr.length());
		if(qt_tryAccelEvent(widget, &a)) {
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
	    if(key_event) {
		//Find out if someone else wants the event, namely
		//is it of use to text services? If so we won't bother
		//with a QKeyEvent.
		if(!CallNextEventHandler(er, event)) {
		    handled_event = TRUE;
		    break;
		}
#ifdef DEBUG_KEY_MAPS
		qDebug("KeyEvent: Sending %s to %s::%s: %04x '%c' (%s) %d%s",
		       etype == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
		       widget ? widget->className() : "none", widget ? widget->name() : "",
		       mychar, chr, mystr.latin1(), modifiers,
		       ekind == kEventRawKeyRepeat ? " Repeat" : "");
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
	remove_context_timer = FALSE;

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
	} else if(widget->isDesktop()) {
	    handled_event = FALSE;
	    break;
	}

	if(ekind == kEventWindowUpdate || ekind == kEventWindowDrawContent) {
	    remove_context_timer = FALSE;
	    widget->propagateUpdates(ekind == kEventWindowUpdate);
	} else if(ekind == kEventWindowBoundsChanged) {
	    handled_event = FALSE;
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
		break;

	    if(widget) {
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
	    if(widget && widget == active_window)
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

	    WindowPtr wp = ActiveNonFloatingWindow();
	    if(wp && !unhandled_dialogs.find((void *)wp)) {
		if(QWidget *tmp_w = QWidget::find((WId)wp))
		    app->setActiveWindow(tmp_w);
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
		case kAEQuitApplication: {
		    QCloseEvent ev;
		    QApplication::sendSpontaneousEvent(app, &ev);
		    if(ev.isAccepted()) {
			handled_event = TRUE;
			app->quit();
		    }
		    break; }
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
#endif
	    {
		if(cmd.commandID == kHICommandQuit) {
		    QCloseEvent ev;
		    QApplication::sendSpontaneousEvent(app, &ev);
		    HiliteMenu(0);
		    if(ev.isAccepted()) 
			app->quit();
		} else if(cmd.commandID == kHICommandAbout) {
		    QMessageBox::aboutQt(NULL);
		    HiliteMenu(0);
		} else {
#if !defined(QMAC_QMENUBAR_NO_NATIVE) //offer it to the menubar..
		    bool by_accel = gotmod && keyc;
		    if(!QMenuBar::activate(cmd.menu.menuRef, cmd.menu.menuItemIndex, FALSE, by_accel) && by_accel)
#endif
			handled_event = FALSE;
		}
	    }
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
		OSStatus ret = ReceiveNextEvent(1, &eventspec, kEventDurationNoWait, TRUE, &er);
		if(ret == eventLoopTimedOutErr || ret == eventLoopQuitErr)
		    break;
		ReleaseEvent(er);
	    }
	}
    }

    if(!handled_event) //let the event go through
	return CallNextEventHandler(er, event);
    QuitApplicationEventLoop();
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
	(*activeBeforePopup) = focus_widget ? focus_widget : active_window;
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
    if(popup == qt_button_down) {
	mouse_button_state = 0;
	qt_button_down = NULL;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	// restore the former active window immediately, although
	// we'll get a focusIn later
	if ( *activeBeforePopup ) {
	    active_window = (*activeBeforePopup)->topLevelWidget();
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    (*activeBeforePopup)->setFocus();
	    QFocusEvent::resetReason();
	} else {
	    active_window = 0;
	}
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

    // read library (ie. plugin) path list
    QString libpathkey =
	QString("/qt/%1.%2/libraryPath").arg( QT_VERSION >> 16 ).arg( (QT_VERSION & 0xff00 ) >> 8 );
    QStringList pathlist = settings.readListEntry(libpathkey, ':');
    if (! pathlist.isEmpty()) {
	QStringList::ConstIterator it = pathlist.begin();
	while (it != pathlist.end())
	    QApplication::addLibraryPath(*it++);
    }

    QString defaultcodec = settings.readEntry("/qt/defaultCodec", "none");
    if (defaultcodec != "none") {
	QTextCodec *codec = QTextCodec::codecForName(defaultcodec);
	if (codec)
	    qApp->setDefaultCodec(codec);
    }

    qt_resolve_symlinks = settings.readBoolEntry("/qt/resolveSymlinks", TRUE);

    if(qt_is_gui_used) {
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
		pal.setColor(QPalette::Inactive, (QColorGroup::ColorRole) i, QColor(strlist[i]));
	}
	strlist = settings.readListEntry("/qt/Palette/disabled");
	if (strlist.count() == QColorGroup::NColorRoles) {
	    for (i = 0; i < QColorGroup::NColorRoles; i++)
		pal.setColor(QPalette::Disabled, (QColorGroup::ColorRole) i, QColor(strlist[i]));
	}
	if ( pal != QApplication::palette())
	    QApplication::setPalette(pal, TRUE);

	QFont font(QApplication::font());     // read new font
	str = settings.readEntry("/qt/font");
	if (! str.isNull() && ! str.isEmpty()) {
	    font.fromString(str);
	    if (font != QApplication::font())
		QApplication::setFont(font, TRUE);
	}

	// read new QStyle
	QString stylename = settings.readEntry("/qt/style");
	if (! stylename.isNull() && ! stylename.isEmpty()) {
	    QStyle *style = QStyleFactory::create(stylename);
	    if (style)
		QApplication::setStyle(style);
	    else
		stylename = "default";
	} else {
	    stylename = "default";
	}

	num = settings.readNumEntry("/qt/doubleClickInterval",QApplication::doubleClickInterval());
	if(num != QApplication::doubleClickInterval())
	    QApplication::setDoubleClickInterval(num);

	num = settings.readNumEntry("/qt/cursorFlashTime", QApplication::cursorFlashTime());
	QApplication::setCursorFlashTime(num);

	num = settings.readNumEntry("/qt/wheelScrollLines", QApplication::wheelScrollLines());
	QApplication::setWheelScrollLines(num);

	QString colorspec = settings.readEntry("/qt/colorSpec", "default");
	if (colorspec == "normal")
	    QApplication::setColorSpec(QApplication::NormalColor);
	else if (colorspec == "custom")
	    QApplication::setColorSpec(QApplication::CustomColor);
	else if (colorspec == "many")
	    QApplication::setColorSpec(QApplication::ManyColor);
	else if (colorspec != "default")
	    colorspec = "default";

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
	} else {
	    QApplication::setEffectEnabled( Qt::UI_General, FALSE);
	}

	QStringList fontsubs = settings.entryList("/qt/Font Substitutions");
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
    }
    return TRUE;
}

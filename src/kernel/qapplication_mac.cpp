/****************************************************************************
**
** Implementation of Mac startup routines and event handling.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// NOT REVISED
#include "qglobal.h"
#include "qt_mac.h"

#include "qapplication.h"
#include "private/qapplication_p.h"
#include "private/qcolor_p.h"
#include "qwidget.h"
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
#include "qhash.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qpaintdevicemetrics.h"
#include "qcursor.h"
#include "qsettings.h"
#include "qstylefactory.h"
#include "qstyle.h"
#include "qeventloop.h"
#include "qmessagebox.h"
#include "qdesktopwidget.h"
#include "qmime.h"
#include "qevent.h"

#ifndef QT_NO_MAINWINDOW
# include "qmainwindow.h"
#endif

#if !defined(QMAC_QMENUBAR_NO_NATIVE)
#  include "qmenubar.h"
#endif

#if defined(QT_ACCESSIBILITY_SUPPORT)
#  include "qaccessible.h"
#endif

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif

#ifdef Q_WS_MACX
#  include "qdir.h"
#  include <unistd.h>
#  include <sys/time.h>
#  include <sys/select.h>
#elif defined(Q_WS_MAC9)
typedef int timeval;
#endif

#include <string.h>
#include "private/qwidget_p.h"
#define d d_func()
#define q q_func()


/*****************************************************************************
  QApplication debug facilities
 *****************************************************************************/
//#define DEBUG_EVENTS [like EventDebug but more specific to Qt]
//#define DEBUG_DROPPED_EVENTS
//#define DEBUG_KEY_MAPS
//#define DEBUG_MOUSE_MAPS
//#define DEBUG_MODAL_EVENTS
//#define DEBUG_PLATFORM_SETTINGS

#define QMAC_SPEAK_TO_ME
#ifdef QMAC_SPEAK_TO_ME
#include "qvariant.h"
#include "qregexp.h"
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
#if defined(QT_TABLET_SUPPORT)
static int tablet_button_state = 0;
#endif
static int mouse_button_state = 0;
static int keyboard_modifiers_state = 0;
static bool	app_do_modal	= FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;		// stack of modal widgets
extern bool qt_mac_in_drag; //qdnd_mac.cpp
extern bool qt_resolve_symlinks; // from qapplication.cpp
extern bool qt_tab_all_widgets; // from qapplication.cpp
static char    *appName = NULL;                        // application name
bool qt_scrollbar_jump_to_pos = FALSE;
QGuardedPtr<QWidget> qt_button_down;		// widget got last button-down
extern bool qt_tryAccelEvent(QWidget*, QKeyEvent*); // def in qaccel.cpp
static QGuardedPtr<QWidget> qt_mouseover;
static QHash<WindowRef, int> unhandled_dialogs;        //all unhandled dialogs (ie mac file dialog)
static enum { QT_MAC_OFFTHESPOT, QT_MAC_ONTHESPOT } qt_mac_input_spot = QT_MAC_ONTHESPOT;
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif
static EventLoopTimerRef mac_context_timer = NULL;
static EventLoopTimerUPP mac_context_timerUPP = NULL;
static DMExtendedNotificationUPP mac_display_changeUPP = NULL;
static EventHandlerRef app_proc_handler = NULL;
static EventHandlerUPP app_proc_handlerUPP = NULL;
//popup variables
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;

class QExtraWidget : public QWidget
{
public:
    inline QWExtra* extraData();
    inline QTLWExtra* topData();
};
inline QWExtra* QExtraWidget::extraData() { return d->extraData(); }
inline QTLWExtra* QExtraWidget::topData() { return d->topData(); }

/*****************************************************************************
  External functions
 *****************************************************************************/
// Paint event clipping magic - qpainter_mac.cpp
extern QString cfstring2qstring(CFStringRef); //qglobal.cpp
extern void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping(QPaintDevice *dev);
extern void qt_mac_set_cursor(const QCursor *, const Point *); //qcursor_mac.cpp
extern bool qt_mac_is_macsheet(QWidget *, bool =FALSE); //qwidget_mac.cpp
extern QString qt_mac_get_global_setting(QString key, QString val, QString file=QString::null); //qsettings_mac.cpp
QByteArray p2qstring(const unsigned char *); //qglobal.cpp
void qt_mac_command_set_enabled(UInt32, bool); //qmenubar_mac.cpp

/* Unicode input entry magic */
class QTSMDocumentWrapper
{
    QGuardedPtr<QWidget> act;
    TSMDocumentID id;
public:
    QTSMDocumentWrapper() {
	InterfaceTypeList itl = { kUnicodeDocument };
	NewTSMDocument(1, itl, &id, (long)this);
    }
    ~QTSMDocumentWrapper() {
	DeleteTSMDocument(id);
    }
    TSMDocumentID document() const { return id; }

    void setInputWidget(QWidget *w) { act = w; }
    QWidget *inputWidget() const { return act; }
};
static QHash<int, QTSMDocumentWrapper> *qt_mac_tsm_dict=NULL;
void qt_mac_unicode_init(QWidget *w) {
    if(!qt_mac_tsm_dict)
	qt_mac_tsm_dict = new QHash<int, QTSMDocumentWrapper>();
    else if(qt_mac_tsm_dict->contains((int)w->handle()))
	return;
    qt_mac_tsm_dict->insert((int)w->handle(), QTSMDocumentWrapper());
}
void qt_mac_unicode_cleanup(QWidget *w) {
    if(w && qt_mac_tsm_dict && w->isTopLevel())
	qt_mac_tsm_dict->remove((int)w->handle());
}
static QTSMDocumentWrapper *qt_mac_get_document_id(QWidget *w)
{
    if(!w || !qt_mac_tsm_dict ||
       !qt_mac_tsm_dict->contains((int)w->handle()))
	return 0;
    return &(*qt_mac_tsm_dict)[(int)w->handle()];
}
void qt_mac_unicode_reset_input(QWidget *w) {
    if(QTSMDocumentWrapper *doc = qt_mac_get_document_id(w)) {
	if(doc->inputWidget() && doc->inputWidget() != w) {
	    FixTSMDocument(doc->document());
	    doc->setInputWidget(NULL);
	}
    }
}

/* Resolution change magic */
static QMAC_PASCAL void qt_mac_display_change_callbk(void *, SInt16 msg, void *)
{
    if(msg == kDMNotifyEvent) {
	if(QDesktopWidget *dw = qApp->desktop()) {
	    QResizeEvent re(dw->size(), dw->size());
	    QApplication::sendEvent(dw, &re);
	}
    }
}

#ifdef DEBUG_PLATFORM_SETTINGS
static void qt_mac_debug_palette(const QPalette &pal, const QPalette &pal2, const char *where)
{
    const char *groups[] = { "Disabled", "Active", "Inactive" };
    const char *roles[] = { "Foreground", "Button", "Light", "Midlight", "Dark", "Mid",
			    "Text", "BrightText", "ButtonText", "Base", "Background", "Shadow",
			    "Highlight", "HighlightedText", "Link", "LinkVisited" };
    if(where)
	qDebug("qt-internal: %s", where);
    for(int grp = 0; grp < QPalette::NColorGroups; grp++) {
	for(int role = 0; role < QPalette::NColorRoles; role++) {
	    QBrush b = pal.brush((QPalette::ColorGroup)grp, (QPalette::ColorRole)role);
	    qDebug("  %s::%s %d::%d::%d [%p]%s", groups[grp], roles[role], b.color().red(),
		   b.color().green(), b.color().blue(), b.pixmap(),
		   pal2.brush((QPalette::ColorGroup)grp, (QPalette::ColorRole)role) != b ? " (*)" : "");
	}
    }

}
#else
#define qt_mac_debug_palette(x, y, z)
#endif

/* lookup widget helper function */
static short qt_mac_find_window(int x, int y, QWidget **w=NULL)
{
    Point p;
    p.h = x;
    p.v = y;
    WindowPtr wp;
    WindowPartCode wpc;
    OSStatus err = FindWindowOfClass(&p, kAllWindowClasses, &wp, &wpc);
    if(err != noErr) {
	if(w)
	    (*w) = NULL;
	return wpc;
    }
#if !defined(QMAC_NO_FAKECURSOR) && QT_MACOSX_VERSION < 0x1020
    if(wp && !unhandled_dialogs.contains(wp)) {
	QWidget *tmp_w = QWidget::find((WId)wp);
	if(tmp_w && !strcmp(tmp_w->className(),"QMacCursorWidget")) {
	    tmp_w->hide();
	    wpc = qt_mac_find_window(x, y, w);
	    tmp_w->show();
	    return wpc;
	}
    }
#endif
    if(w) {
	if(wp && !unhandled_dialogs.contains(wp)) {
	    *w = QWidget::find((WId)wp);
#if 0
	    if(!*w)
		qWarning("Qt: qt_mac_find_window: Couldn't find %d",(int)wp);
#endif
	} else {
	    *w = NULL;
	}
    }
    return wpc;
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

void qt_mac_update_os_settings()
{
    if(!qApp)
	return;
    if(!QApplication::startingUp())
	qApp->style().polish(qApp);
    { //focus mode
	/* I just reverse engineered this, I'm not so sure how well it will hold up but it works as of 10.2.3 */
	bool ok;
	int i = qt_mac_get_global_setting("AppleKeyboardUIMode", "0").toInt(&ok);
	qt_tab_all_widgets = !ok || (i & 0x2);
    }
    { //focus mode
	/* I just reverse engineered this, I'm not so sure how well it will hold up but it works as of 10.2.3 */
	QString paging = qt_mac_get_global_setting("AppleScrollerPagingBehavior", "FALSE");
	qt_scrollbar_jump_to_pos = (paging == "TRUE");
    }

#ifdef DEBUG_PLATFORM_SETTINGS
    qDebug("qt_mac_update_os_settings *********************************************************************");
#endif
    { //setup the global peltte
	QColor qc;
	RGBColor c;
	QPalette pal = QApplication::palette();
	if(!GetThemeBrushAsColor(-3, 32, true, &c))
	    pal.setBrush(QPalette::Active, QPalette::Highlight,
			 QColor(c.red / 256, c.green / 256, c.blue / 256));
	if(!GetThemeBrushAsColor(-4, 32, true, &c)) {
	    pal.setBrush(QPalette::Inactive, QPalette::Highlight,
			 QColor(c.red / 256, c.green / 256, c.blue / 256));
	    pal.setBrush(QPalette::Disabled, QPalette::Highlight,
			 QColor(c.red / 256, c.green / 256, c.blue / 256));
	}
	if(!GetThemeBrushAsColor(kThemeBrushButtonActiveDarkShadow, 32, true, &c))
	    pal.setBrush(QPalette::Active, QPalette::Shadow,
			 QColor(c.red / 256, c.green / 256, c.blue / 256));
	if(!GetThemeBrushAsColor(kThemeBrushButtonInactiveDarkShadow, 32, true, &c)) {
	    pal.setBrush(QPalette::Inactive, QPalette::Shadow,
			 QColor(c.red / 256, c.green / 256, c.blue / 256));
	    pal.setBrush(QPalette::Disabled, QPalette::Shadow,
			 QColor(c.red / 256, c.green / 256, c.blue / 256));
	}
	if(!GetThemeTextColor(kThemeTextColorDialogActive, 32, true, &c)) {
	    qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
	    pal.setColor(QPalette::Active, QPalette::Text, qc);
	    pal.setColor(QPalette::Active, QPalette::Foreground, qc);
	    pal.setColor(QPalette::Active, QPalette::HighlightedText, qc);
	}
	if(!GetThemeTextColor(kThemeTextColorDialogInactive, 32, true, &c)) {
	    qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
	    pal.setColor(QPalette::Inactive, QPalette::Text, qc);
	    pal.setColor(QPalette::Inactive, QPalette::Foreground, qc);
	    pal.setColor(QPalette::Inactive, QPalette::HighlightedText, qc);
	    pal.setColor(QPalette::Disabled, QPalette::Text, qc);
	    pal.setColor(QPalette::Disabled, QPalette::Foreground, qc);
	    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, qc);
	}
	if(!(pal == QApplication::palette())) {
	    QApplication::setPalette(pal);
#ifdef DEBUG_PLATFORM_SETTINGS
	    qt_mac_debug_palette(pal, QApplication::palette(), "Global Palette");
#endif
	}
    }
    { //setup the global font
	Str255 f_name;
	SInt16 f_size;
	Style f_style;
	GetThemeFont(kThemeApplicationFont, smSystemScript, f_name, &f_size, &f_style);
	QApplication::setFont(QFont(p2qstring(f_name), f_size,
				    (f_style & ::bold) ? QFont::Bold : QFont::Normal,
				    (bool)(f_style & ::italic)));
    }
    { //setup the fonts
	struct {
	    const char *qt_class;
	    short font_key;
	} mac_widget_fonts[] = {
	    { "QPushButton", kThemePushButtonFont },
	    { "QListView", kThemeViewsFont },
	    { "QListBox", kThemeViewsFont },
	    { "QTitleBar", kThemeWindowTitleFont },
	    { "QMenuBar", kThemeMenuTitleFont },
	    { "QPopupMenu", kThemeMenuItemFont },
	    { "QHeader", kThemeSmallSystemFont },
	    { "QTipLabel", kThemeSmallSystemFont },
	    { "QMessageBoxLabel", kThemeEmphasizedSystemFont },
	    { "QLabel", kThemeLabelFont },
	    { NULL, 0 } };
	Str255 f_name;
	SInt16 f_size;
	Style f_style;
	for(int i = 0; mac_widget_fonts[i].qt_class; i++) {
	    GetThemeFont(mac_widget_fonts[i].font_key, smSystemScript, f_name, &f_size, &f_style);
	    QFont fnt(p2qstring(f_name), f_size, (f_style & ::bold) ? QFont::Bold : QFont::Normal,
		      (bool)(f_style & ::italic));
	    bool set_font = TRUE;
	    if(QApplication::app_fonts) {
		if(QFont *oldfnt = QApplication::app_fonts->find(mac_widget_fonts[i].qt_class))
		    set_font = !(fnt == *oldfnt);
	    }
	    if(set_font) {
		QApplication::setFont(fnt, TRUE, mac_widget_fonts[i].qt_class);
#ifdef DEBUG_PLATFORM_SETTINGS
		qDebug("qt-internal: Font for %s [%s::%d::%d::%d]", mac_widget_fonts[i].qt_class,
		       fnt.family().latin1(), fnt.pointSize(), fnt.bold(), fnt.italic());
#endif
	    }
	}
    }
    { //setup the palette
	struct {
	    const char *qt_class;
	    ThemeBrush active, inactive;
	} mac_widget_colours[] = {
	    { "QToolButton", kThemeTextColorBevelButtonActive, kThemeTextColorBevelButtonInactive },
	    { "QButton", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive },
	    { "QHeader", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive },
	    { "QComboBox", kThemeTextColorPopupButtonActive, kThemeTextColorPopupButtonInactive },
	    { "QListView", kThemeTextColorListView, kThemeTextColorDialogInactive },
	    { "QListBox", kThemeTextColorListView, kThemeTextColorDialogInactive },
	    { "QMessageBoxLabel", kThemeTextColorAlertActive, kThemeTextColorAlertInactive },
	    { "QTabBar", kThemeTextColorTabFrontActive, kThemeTextColorTabFrontInactive },
	    { "QLabel", kThemeTextColorPlacardActive, kThemeTextColorPlacardInactive },
	    { "QGroupBox", kThemeTextColorPlacardActive, kThemeTextColorPlacardInactive },
	    { "QPopupMenu", kThemeTextColorPopupLabelActive, kThemeTextColorPopupLabelInactive },
	    { NULL, 0, 0 } };
	QColor qc;
	RGBColor c;
	QPalette apppal = QApplication::palette();
	for(int i = 0; mac_widget_colours[i].qt_class; i++) {
	    QPalette pal = apppal;
	    if(!GetThemeTextColor(mac_widget_colours[i].active, 32, true, &c)) {
		qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
		pal.setColor(QPalette::Active, QPalette::Text, qc);
		pal.setColor(QPalette::Active, QPalette::Foreground, qc);
		pal.setColor(QPalette::Active, QPalette::HighlightedText, qc);
	    }
	    if(!GetThemeTextColor(mac_widget_colours[i].inactive, 32, true, &c)) {
		qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
		pal.setColor(QPalette::Inactive, QPalette::Text, qc);
		pal.setColor(QPalette::Disabled, QPalette::Text, qc);
		pal.setColor(QPalette::Inactive, QPalette::Foreground, qc);
		pal.setColor(QPalette::Disabled, QPalette::Foreground, qc);
		pal.setColor(QPalette::Inactive, QPalette::HighlightedText, qc);
		pal.setColor(QPalette::Disabled, QPalette::HighlightedText, qc);
	    }
	    if(!strcmp(mac_widget_colours[i].qt_class, "QPopupMenu")) { //special
		GetThemeTextColor(kThemeTextColorMenuItemActive, 32, true, &c);
		pal.setBrush(QPalette::ButtonText, QColor(c.red / 256, c.green / 256, c.blue / 256));
		GetThemeTextColor(kThemeTextColorMenuItemSelected, 32, true, &c);
		pal.setBrush(QPalette::HighlightedText, QColor(c.red / 256, c.green / 256, c.blue / 256));
		GetThemeTextColor(kThemeTextColorMenuItemDisabled, 32, true, &c);
		pal.setBrush(QPalette::Disabled, QPalette::Text,
			     QColor(c.red / 256, c.green / 256, c.blue / 256));
	    } else if(!strcmp(mac_widget_colours[i].qt_class, "QButton") ||
		      !strcmp(mac_widget_colours[i].qt_class, "QHeader")) { //special
		pal.setColor(QPalette::Disabled, QPalette::ButtonText,
			     pal.color(QPalette::Disabled, QPalette::Text));
		pal.setColor(QPalette::Inactive, QPalette::ButtonText,
			     pal.color(QPalette::Inactive, QPalette::Text));
		pal.setColor(QPalette::Active, QPalette::ButtonText,
			     pal.color(QPalette::Active, QPalette::Text));
	    }
	    bool set_palette = TRUE;
	    if(QApplication::app_palettes) {
		if(QPalette *oldpal = QApplication::app_palettes->find(mac_widget_colours[i].qt_class))
		    set_palette = !(pal == *oldpal);
	    }
	    if(set_palette && pal != apppal) {
		QApplication::setPalette(pal, TRUE, mac_widget_colours[i].qt_class);
#ifdef DEBUG_PLATFORM_SETTINGS
		qt_mac_debug_palette(pal, apppal, QString("Palette for ") + mac_widget_colours[i].qt_class);
#endif
	    }
	}
    }
#ifdef DEBUG_PLATFORM_SETTINGS
    qDebug("qt_mac_update_os_settings END !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#endif
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
    kEventQtRequestShowSheet = 17,
    kEventQtRequestActivate = 18,
    kEventQtRequestSocketAct = 19
};
static EventRef request_updates_pending = NULL;
void qt_event_request_updates()
{
    if(request_updates_pending) {
	if(IsEventInQueue(GetMainEventQueue(), request_updates_pending))
	    return;
#ifdef DEBUG_DROPPED_EVENTS
	qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(NULL, kEventClassQt, kEventQtRequestPropagateWindowUpdates,
		GetCurrentEventTime(), kEventAttributeUserEvent, &request_updates_pending);
    PostEventToQueue(GetMainEventQueue(), request_updates_pending, kEventPriorityHigh);
    ReleaseEvent(request_updates_pending);
}
static EventRef request_select_pending = NULL;
void qt_event_request_select(QEventLoop *loop) {
    if(request_select_pending) {
	if(IsEventInQueue(GetMainEventQueue(), request_select_pending))
	    return;
#ifdef DEBUG_DROPPED_EVENTS
	qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(NULL, kEventClassQt, kEventQtRequestSelect, GetCurrentEventTime(),
		kEventAttributeUserEvent, &request_select_pending);
    SetEventParameter(request_select_pending,
		      kEventParamQEventLoop, typeQEventLoop, sizeof(loop), &loop);
    PostEventToQueue(GetMainEventQueue(), request_select_pending, kEventPriorityStandard);
    ReleaseEvent(request_select_pending);
}
static EventRef request_sockact_pending = NULL;
void qt_event_request_sockact(QEventLoop *loop) {
    if(request_sockact_pending) {
	if(IsEventInQueue(GetMainEventQueue(), request_sockact_pending))
	    return;
#ifdef DEBUG_DROPPED_EVENTS
	qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(NULL, kEventClassQt, kEventQtRequestSocketAct, GetCurrentEventTime(),
		kEventAttributeUserEvent, &request_sockact_pending);
    SetEventParameter(request_sockact_pending,
		      kEventParamQEventLoop, typeQEventLoop, sizeof(loop), &loop);
    PostEventToQueue(GetMainEventQueue(), request_sockact_pending, kEventPriorityStandard);
    ReleaseEvent(request_sockact_pending);
}
void qt_event_request_showsheet(QWidget *w)
{
    EventRef ctx = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestShowSheet, GetCurrentEventTime(),
		kEventAttributeUserEvent, &ctx);
    SetEventParameter(ctx, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), ctx, kEventPriorityStandard);
    ReleaseEvent(ctx);
}

static QValueList<WId> request_updates_pending_list;
void qt_event_request_updates(QWidget *w, const QRegion &r, bool subtract)
{
    QWExtra *extra = ((QExtraWidget*)w)->extraData();
    if(subtract) {
	if(extra->has_dirty_area) {
	    extra->dirty_area -= r;
	    if(extra->dirty_area.isEmpty()) {
		request_updates_pending_list.remove(w->winId());
		extra->has_dirty_area = FALSE;
	    }
	}
	return;
    } else if(extra->has_dirty_area) {
	extra->dirty_area |= r;
	return;
    }
    extra->has_dirty_area = TRUE;
    extra->dirty_area = r;
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
static EventRef request_wakeup_pending = NULL;
void qt_event_request_wakeup()
{
    if(request_wakeup_pending) {
	if(IsEventInQueue(GetMainEventQueue(), request_wakeup_pending))
	    return;
#ifdef DEBUG_DROPPED_EVENTS
	qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(NULL, kEventClassQt, kEventQtRequestWakeup, GetCurrentEventTime(),
		kEventAttributeUserEvent, &request_wakeup_pending);
    PostEventToQueue(GetMainEventQueue(), request_wakeup_pending,
		     kEventPriorityHigh);
    ReleaseEvent(request_wakeup_pending);
}
static EventRef request_activate_pending = NULL;
void qt_event_request_activate(QWidget *w)
{
    if(request_activate_pending) {
	if(IsEventInQueue(GetMainEventQueue(), request_activate_pending))
	    return;
#ifdef DEBUG_DROPPED_EVENTS
	qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(NULL, kEventClassQt, kEventQtRequestActivate, GetCurrentEventTime(),
		kEventAttributeUserEvent, &request_activate_pending);
    SetEventParameter(request_activate_pending, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), request_activate_pending, kEventPriorityHigh);
    ReleaseEvent(request_activate_pending);
}
void qt_event_request_timer(TimerInfo *tmr)
{
    EventRef tmr_ev = NULL;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestTimer, GetCurrentEventTime(),
		kEventAttributeUserEvent, &tmr_ev);
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
static EventRef request_menubarupdate_pending = NULL;
void qt_event_request_menubarupdate()
{
    if(request_menubarupdate_pending) {
	if(IsEventInQueue(GetMainEventQueue(), request_menubarupdate_pending))
	    return;
#ifdef DEBUG_DROPPED_EVENTS
	qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(NULL, kEventClassQt, kEventQtRequestMenubarUpdate, GetCurrentEventTime(),
		kEventAttributeUserEvent, &request_menubarupdate_pending);
    PostEventToQueue(GetMainEventQueue(), request_menubarupdate_pending, kEventPriorityHigh);
    ReleaseEvent(request_menubarupdate_pending);
}
#endif

static EventTypeSpec events[] = {
    { kEventClassQt, kEventQtRequestTimer },
    { kEventClassQt, kEventQtRequestWakeup },
    { kEventClassQt, kEventQtRequestSelect },
    { kEventClassQt, kEventQtRequestShowSheet },
    { kEventClassQt, kEventQtRequestContext },
    { kEventClassQt, kEventQtRequestActivate },
#ifndef QMAC_QMENUBAR_NO_NATIVE
    { kEventClassQt, kEventQtRequestMenubarUpdate },
#endif
    { kEventClassQt, kEventQtRequestPropagateWindowUpdates },
    { kEventClassQt, kEventQtRequestPropagateWidgetUpdates },
    { kEventClassQt, kEventQtRequestSocketAct },

    { kEventClassWindow, kEventWindowInit },
    { kEventClassWindow, kEventWindowDispose },
    { kEventClassWindow, kEventWindowUpdate },
    { kEventClassWindow, kEventWindowDrawContent },
    { kEventClassWindow, kEventWindowActivated },
    { kEventClassWindow, kEventWindowDeactivated },
    { kEventClassWindow, kEventWindowShown },
    { kEventClassWindow, kEventWindowHidden },
    { kEventClassWindow, kEventWindowBoundsChanged },
    { kEventClassWindow, kEventWindowExpanded },

    { kEventClassMouse, kEventMouseWheelMoved },
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseMoved },

    { kEventClassApplication, kEventAppActivated },
    { kEventClassApplication, kEventAppDeactivated },

    { kEventClassMenu, kEventMenuOpening },
    { kEventClassMenu, kEventMenuClosed },
    { kEventClassMenu, kEventMenuTargetItem },

    { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
    { kEventClassTextInput, kEventTextInputOffsetToPos },
    { kEventClassTextInput, kEventTextInputUpdateActiveInputArea },
    { kEventClassKeyboard, kEventRawKeyModifiersChanged },
    { kEventClassKeyboard, kEventRawKeyRepeat },
    { kEventClassKeyboard, kEventRawKeyUp },
    { kEventClassKeyboard, kEventRawKeyDown },

    { kEventClassCommand, kEventCommandProcess },
    { kEventClassAppleEvent, kEventAppleEvent },
    { kAppearanceEventClass, kAEAppearanceChanged }
};

void qt_init_app_proc_handler()
{
    InstallEventHandler(GetApplicationEventTarget(), app_proc_handlerUPP,
			GetEventTypeCount(events), events, (void *)qApp,
			&app_proc_handler);
}

void qt_release_app_proc_handler()
{
    if(app_proc_handler) {
	RemoveEventHandler(app_proc_handler);
	app_proc_handler = NULL;
    }
}

bool qt_sendSpontaneousEvent(QObject *obj, QEvent *event)
{
    return QKernelApplication::sendSpontaneousEvent(obj, event);
}

/* platform specific implementations */
void qt_init(QApplicationPrivate *priv, QApplication::Type)
{
    if(qt_is_gui_used) {
	ProcessSerialNumber psn;
	if(GetCurrentProcess(&psn) == noErr) {
	    if(!mac_display_changeUPP) {
		mac_display_changeUPP = NewDMExtendedNotificationUPP(qt_mac_display_change_callbk);
		DMRegisterExtendedNotifyProc(mac_display_changeUPP, NULL, 0, &psn);
	    }
#ifdef Q_WS_MACX
	    SetFrontProcess(&psn);
#endif
	}
    }

    char **argv = d->argv;

    // Get command line params
    if(int argc = priv->argc) {
	int i, j = 1;
	for(i=1; i < argc; i++) {
	    if(argv[i] && *argv[i] != '-') {
		argv[j++] = argv[i];
		continue;
	    }
	    QByteArray arg(argv[i]);
#if defined(QT_DEBUG)
	    if(arg == "-nograb")
		appNoGrab = !appNoGrab;
	    else
#endif // QT_DEBUG
		if(arg == "-inputstyle") {
		    if(++i < argc) {
			QByteArray s = QByteArray(argv[i]).lower();
			if(s == "onthespot")
			    qt_mac_input_spot = QT_MAC_ONTHESPOT;
			else if(s == "offthespot")
			    qt_mac_input_spot = QT_MAC_OFFTHESPOT;
			else
			    qDebug("Qt: internal: Misunderstood input style '%s'", s.constData());
		    }
		} else
#ifdef Q_WS_MACX
		    //just ignore it, this seems to be passed from the finder (no clue what it does) FIXME
		    if(arg.left(5) == "-psn_");
		    else
#endif
			argv[j++] = argv[i];
	}
	priv->argc = j;
	// Set application name
	char *p = strrchr(argv[0], '/');
	appName = p ? p + 1 : argv[0];
#ifdef Q_WS_MACX
	if(qt_is_gui_used && argv[0] && *argv[0] != '/')
	    qWarning("Qt: QApplication: Warning argv[0] == '%s' is relative.\n"
		     "In order to dispatch events correctly Mac OS X may "
		     "require applications to be run with the *full* path to the "
		     "executable.", argv[0]);

	//special hack to change working directory to a resource fork when running from finder
	if(p && !QDir::isRelativePath(p) && QDir::currentDirPath() == "/") {
	    QString path = argv[0];
	    int rfork = path.findRev(QString("/") + appName + ".app/");
	    if(rfork != -1)
		QDir::setCurrent(path.left(rfork+1));
	}
    }
#endif

    QMacMime::initialize();

    if(appName)
	qApp->setName(appName);
    if(qt_is_gui_used) {
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	QMenuBar::initialize();
#else
	qt_mac_command_set_enabled(kHICommandQuit, FALSE);
#endif
	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::initialize();
#endif

#if defined(QT_THREAD_SUPPORT)
	qt_mac_port_mutex = new QMutex(TRUE);
#endif
	RegisterAppearanceClient();
	if(QApplication::desktopSettingsAware())
	    qt_mac_update_os_settings();

	if(!app_proc_handler) {
	    app_proc_handlerUPP = NewEventHandlerUPP(QApplication::globalEventProcessor);
	    qt_init_app_proc_handler();
	}

	if(QApplication::app_style) {
	    QEvent ev(QEvent::Style);
	    qt_sendSpontaneousEvent(QApplication::app_style, &ev);
	}
    }
    QApplication::qt_mac_apply_settings();
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    qt_release_app_proc_handler();
    if(app_proc_handlerUPP) {
	DisposeEventHandlerUPP(app_proc_handlerUPP);
	app_proc_handlerUPP = NULL;
    }
    QPixmapCache::clear();
    if(qt_is_gui_used) {
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::cleanup();
#endif
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
    if(qt_mac_tsm_dict) {
	delete qt_mac_tsm_dict;
	qt_mac_tsm_dict = NULL;
    }
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/
void qt_updated_rootinfo()
{
}

bool qt_wstate_iconified(WId)
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


/*!
  Returns the version of the Macintosh operating system that is running.

  Note: that this function is implemented for the Macintosh version
  of Qt only.
*/
Qt::MacintoshVersion QApplication::macVersion()
{
    return qt_macver;
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/
typedef QPtrList<QCursor> QCursorList;
static QCursorList *cursorStack = 0;
void QApplication::setOverrideCursor(const QCursor &cursor, bool replace)
{
    if(!cursorStack) {
	cursorStack = new QCursorList;
	cursorStack->setAutoDelete(TRUE);
    }
    app_cursor = new QCursor(cursor);
    if(replace)
	cursorStack->removeLast();
    cursorStack->append(app_cursor);

    if(qApp && qApp->activeWindow()) {
	Point mouse_pos;
	QPoint qmp(QCursor::pos());
	mouse_pos.h = qmp.x();
	mouse_pos.v = qmp.y();
	qt_mac_set_cursor(app_cursor, &mouse_pos);
    }
}

void QApplication::restoreOverrideCursor()
{
    if(!cursorStack)				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    if(cursorStack->isEmpty()) {
	delete cursorStack;
	cursorStack = NULL;
    }

    if(qApp && qApp->activeWindow()) {
	Point mouse_pos;
	QPoint qmp(QCursor::pos());
	mouse_pos.h = qmp.x();
	mouse_pos.v = qmp.y();

	const QCursor def(Qt::ArrowCursor);
	const QCursor *n = app_cursor ? app_cursor : &def;
	qt_mac_set_cursor(n, &mouse_pos);
    }
}

#endif


QWidget *qt_recursive_match(QWidget *widg, int x, int y)
{
    // Keep looking until we find ourselves in a widget with no kiddies
    // where the x,y is
    if(!widg)
	return 0;

    QObjectList chldrn=widg->children();
    for(int i = chldrn.size() - 1; i >= 0; --i) {
	QObject *obj = chldrn.at(i);
	if(obj->isWidgetType()) {
	    QWidget *curwidg=(QWidget *)obj;
	    if(curwidg->isVisible() && !curwidg->isTopLevel()) {
		int wx=curwidg->x(), wy=curwidg->y();
		int wx2=wx+curwidg->width(), wy2=wy+curwidg->height();
		if(x>=wx && y>=wy && x<=wx2 && y<=wy2) {
		    if(!curwidg->testWFlags(Qt::WMouseNoMask)) {
			QWExtra *extra = ((QExtraWidget*)curwidg)->extraData();
			if(extra && !extra->mask.isEmpty() && !extra->mask.contains(QPoint(x-wx, y-wy)))
			    continue;
		    }
		    return qt_recursive_match(curwidg,x-wx,y-wy);
		}
	    }
	}
    }
    // If we get here, it's within a widget that has children, but isn't in any
    // of the children
    return widg;
}

QWidget *QApplication::widgetAt(int x, int y, bool child)
{
    //find the tld
    QWidget *widget;
    qt_mac_find_window(x, y, &widget);
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
		qDebug("Qt: internal: **Mapping modifier: %d (0x%04x) -- %d", key, key, from_mouse);
#endif
    int ret = 0;
    for(int i = 0; modifier_syms[i].qt_code; i++) {
	if(key & modifier_syms[i].mac_code) {
#ifdef DEBUG_KEY_MAPS
#ifndef DEBUG_MOUSE_MAPS
	    if(!from_mouse)
#endif
		qDebug("Qt: internal: %d: got modifier: %s", from_mouse, modifier_syms[i].desc);
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
{ '^', MAP_KEY(Qt::Key_AsciiCircum) },
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
    int popup_close_count = 0;
    short windowPart = qt_mac_find_window(pt->h, pt->v, &widget);
    if(inPopupMode() && widget != activePopupWidget()) {
	while(inPopupMode()) {
	    activePopupWidget()->close();
	    popup_close_count++;
	    if(windowPart == inContent)
		break;
	}
    }

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
	return !popup_close_count; //just return and let the event loop process
    } else if(windowPart != inGoAway && windowPart != inCollapseBox) {
	bool set_active = TRUE;
	if(windowPart == inZoomIn || windowPart == inZoomOut || windowPart == inDrag || windowPart == inGrow)
	    set_active = !(GetCurrentKeyModifiers() & cmdKey);
	if(set_active) {
	    widget->raise();
	    if(widget->isTopLevel() && !widget->isDesktop() && !widget->isPopup() &&
	       (widget->isModal() || !widget->inherits("QDockWindow")))
		widget->setActiveWindow();
	}
    }
    if(windowPart == inGoAway || windowPart == inCollapseBox ||
       windowPart == inZoomIn || windowPart == inZoomOut) {
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
	int h = 0;
	QObjectList chldrn = widget->children();
	for(int i = 0; i < chldrn.size(); i++) {
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType() && obj->inherits("QDockArea")) {
		QWidget *w = (QWidget *)obj;
#ifndef QT_NO_MAINWINDOW
		if(widget->inherits("QMainWindow") && ((QMainWindow*)widget)->topDock() != (QDockArea*)w)
		    continue; //bleh
#endif
		if(w->width() < w->height()) //only do horizontal orientations
		    continue;
		int oh = w->sizeHint().height();
		if(oh < 0)
		    oh = 0;
		if(w->isVisible())
		    w->hide();
		else
		    w->show();
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
	SetRect(&limits, -2, 0, 0, 0);
	if(QWExtra *extra = ((QExtraWidget*)widget)->extraData())
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
    case inCollapseBox: {
	widget->showMinimized();
	QEvent e(QEvent::Hide);
	QApplication::sendSpontaneousEvent(widget, &e);
	break; }
    case inZoomIn:
	widget->showNormal();
	break;
    case inZoomOut:
	widget->showMaximized();
	break;
    default:
	qDebug("Qt: internal: Unhandled case in mouse_down.. %d", windowPart);
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
#ifdef DEBUG_MODAL_EVENTS
    qDebug("Entering modal state with %s::%s::%p (%d)", widget->className(), widget->name(),
	   widget, qt_modal_stack ? (int)qt_modal_stack->count() : -1);
#endif
    if(!qt_modal_stack) {			// create modal stack
	qt_modal_stack = new QWidgetList;
    }
    qt_modal_stack->insert(0, widget);
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
    if(!app_do_modal)
	qt_event_request_menubarupdate();
#endif
    app_do_modal = TRUE;
}


void qt_leave_modal(QWidget *widget)
{
    if(qt_modal_stack && qt_modal_stack->remove(widget)) {
#ifdef DEBUG_MODAL_EVENTS
	qDebug("Leaving modal state with %s::%s::%p (%d)", widget->className(), widget->name(),
	       widget, qt_modal_stack->count());
#endif
	if(qt_modal_stack->isEmpty()) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	}
    }
#ifdef DEBUG_MODAL_EVENTS
    else qDebug("Failure to remove %s::%s::%p -- %p", widget->className(), widget->name(), widget, qt_modal_stack);
#endif
    app_do_modal = (qt_modal_stack != 0);
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
    if(!app_do_modal)
	qt_event_request_menubarupdate();
#endif
}

QWidget *qt_tryModalHelperMac( QWidget * top ) {
    if(top && qt_mac_is_macsheet(top) && !MacIsWindowVisible((WindowPtr)top->handle())) {
	if(WindowPtr wp = GetFrontWindowOfClass(kSheetWindowClass, true)) {
	    if(QWidget *sheet = QWidget::find((WId)wp))
		top = sheet;
	}
    }
    return top;
}

static bool qt_try_modal(QWidget *widget, EventRef event)
{
    QWidget * top = 0;

    if ( qt_tryModalHelper( widget, &top ) )
	return TRUE;

    bool block_event  = FALSE;
    bool paint_event = FALSE;

    UInt32 ekind = GetEventKind(event), eclass=GetEventClass(event);
    switch(eclass) {
    case kEventClassMouse:
	block_event = (ekind != kEventMouseMoved);
	break;
    case kEventClassKeyboard:
	block_event = TRUE;
	break;
    case kEventClassWindow:
	paint_event = (ekind == kEventWindowUpdate);
	break;
    }

    if(!top->parentWidget() && (block_event || paint_event))
	top->raise();
#if 0 //This is really different than Qt behaves, but it is correct for Aqua, what do I do? -Sam
    if(block_event && qt_mac_is_macsheet(top)) {
	for(QWidget *w = top->parentWidget(); w; w = w->parentWidget()) {
	    w = w->topLevelWidget();
	    if(w == widget || w->isModal()) {
#ifdef DEBUG_MODAL_EVENTS
		qDebug("%s:%d -- modal (FALSE)", __FILE__, __LINE__);
#endif
		return FALSE;
	    }
	}
#ifdef DEBUG_MODAL_EVENTS
	qDebug("%s:%d -- special mac-sheet (TRUE)", __FILE__, __LINE__);
#endif
	return TRUE;
    }
#endif

#ifdef DEBUG_MODAL_EVENTS
    qDebug("%s:%d -- final decision! (%s)", __FILE__, __LINE__, block_event ? "FALSE" : "TRUE");
#endif
    return !block_event;
}

//context menu hack
static EventRef request_context_pending = NULL;
static void qt_event_request_context(QWidget *w=NULL, EventRef *where=NULL)
{
    if(!where)
	where = &request_context_pending;
    if(*where || QMacBlockingFunction::blocking())
	return;
    CreateEvent(NULL, kEventClassQt, kEventQtRequestContext, GetCurrentEventTime(),
		kEventAttributeUserEvent, where);
    if(w)
	SetEventParameter(*where, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), *where, kEventPriorityStandard);
    ReleaseEvent(*where);
}
static EventRef request_context_hold_pending = NULL;
QMAC_PASCAL void
QApplication::qt_context_timer_callbk(EventLoopTimerRef r, void *w)
{
    QWidget *widg = (QWidget *)w;
    EventLoopTimerRef otc = mac_context_timer;
    RemoveEventLoopTimer(mac_context_timer);
    mac_context_timer = NULL;
    if(r == otc && widg == qt_button_down)
	qt_event_request_context(widg, &request_context_hold_pending);
}

bool qt_mac_send_event(QEventLoop::ProcessEventsFlags flags, EventRef event, WindowPtr pt)
{
    if(flags != QEventLoop::AllEvents) {
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
		if(ekind == kEventQtRequestSelect || ekind == kEventQtRequestSocketAct)
		    return FALSE;
		break;
	    }
	}
    }
    if(pt && SendEventToWindow(event, pt) != eventNotHandledErr)
	return TRUE;
    return !SendEventToEventTarget(event, GetEventDispatcherTarget());
}

QMAC_PASCAL OSStatus
QApplication::globalEventProcessor(EventHandlerCallRef er, EventRef event, void *data)
{
    QApplication *app = (QApplication *)data;
    if(app->macEventFilter(er, event)) //someone else ate it
	return noErr;
    QGuardedPtr<QWidget> widget;

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
		if(!widget)
		    continue;
		QWExtra *extra = ((QExtraWidget*)widget)->extraData();
		if(extra && extra->has_dirty_area) {
		    extra->has_dirty_area = FALSE;
		    QRegion r = extra->dirty_area;
		    extra->dirty_area = QRegion();
		    QRegion cr = widget->clippedRegion();
		    if(!widget->isTopLevel()) {
			QPoint point(posInWindow(widget));
			cr.translate(-point.x(), -point.y());
		    }
		    r &= cr;
		    if(!r.isEmpty())
			widget->repaint(r);
		}
	    }
	    request_updates_pending_list.clear();
	} else if(ekind == kEventQtRequestPropagateWindowUpdates) {
	    request_updates_pending = NULL;
	    QApplication::sendPostedEvents();
	    QWidgetList tlws = qApp->topLevelWidgets();
	    for(int i = 0; i < tlws.size(); i++) {
		QWidget *tlw = tlws.at(i);
		if(!tlw->isHidden())
		    tlw->propagateUpdates();
	    }
	} else if(ekind == kEventQtRequestShowSheet) {
	    QWidget *widget = NULL;
	    GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
			      sizeof(widget), NULL, &widget);
	    if(widget)
		ShowSheetWindow((WindowPtr)widget->hd, (WindowPtr)widget->parentWidget()->hd);
	} else if(ekind == kEventQtRequestWakeup) {
	    request_wakeup_pending = NULL; 	    //do nothing else, we just woke up!
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	} else if(ekind == kEventQtRequestMenubarUpdate) {
	    request_menubarupdate_pending = NULL;
	    QMenuBar::macUpdateMenuBar();
#endif
	} else if(ekind == kEventQtRequestSelect) {
	    request_select_pending = NULL;
	    QEventLoop *l = NULL;
	    if(GetEventParameter(event, kEventParamQEventLoop, typeQEventLoop, NULL, sizeof(l), NULL, &l))
		l = app->eventLoop();
	    timeval tm;
	    memset(&tm, '\0', sizeof(tm));
	    l->macHandleSelect(&tm);
	} else if(ekind == kEventQtRequestSocketAct) {
	    request_sockact_pending = NULL;
	    QEventLoop *l = NULL;
	    if(GetEventParameter(event, kEventParamQEventLoop, typeQEventLoop, NULL, sizeof(l), NULL, &l))
		l = app->eventLoop();
	    l->activateSocketNotifiers();
	} else if(ekind == kEventQtRequestActivate) {
	    request_activate_pending = NULL;
	    QWidget *widget = NULL;
	    GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
			      sizeof(widget), NULL, &widget);
	    if(widget)
		widget->setActiveWindow();
	} else if(ekind == kEventQtRequestContext) {
	    bool send = FALSE;
	    if((send = (event == request_context_hold_pending)))
		request_context_hold_pending = NULL;
	    else if((send = (event == request_context_pending)))
		request_context_pending = NULL;
	    if(send) {
		//figure out which widget to send it to
		QPoint where = QCursor::pos();
		QWidget *widget = NULL;
		GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
				  sizeof(widget), NULL, &widget);
		if(!widget) {
		    if(qt_button_down)
			widget = qt_button_down;
		    else
			widget = QApplication::widgetAt(where.x(), where.y(), true);
		}
		if(widget) {
		    QPoint plocal(widget->mapFromGlobal(where));
		    QContextMenuEvent qme(QContextMenuEvent::Mouse, plocal, where, 0);
		    QApplication::sendEvent(widget, &qme);
		    if(qme.isAccepted()) { //once this happens the events before are pitched
			if(qt_button_down && mouse_button_state) {
			    QMouseEvent qme(QEvent::MouseButtonRelease, plocal, where,
					     mouse_button_state, mouse_button_state);
			    QApplication::sendSpontaneousEvent(qt_button_down, &qme);
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
#if defined(DEBUG_MOUSE_MAPS) || defined(DEBUG_DROPPED_EVENTS)
	const char *edesc = NULL;
	switch(ekind) {
	case kEventMouseDown: edesc = "MouseButtonPress"; break;
	case kEventMouseUp: edesc = "MouseButtonRelease"; break;
	case kEventMouseDragged: case kEventMouseMoved: edesc = "MouseMove"; break;
	case kEventMouseWheelMoved: edesc = "MouseWheelMove"; break;
	}
#endif
	if((ekind == kEventMouseDown && mouse_button_state) ||
	    (ekind == kEventMouseUp && !mouse_button_state)) {
#if defined(DEBUG_MOUSE_MAPS) || defined(DEBUG_DROPPED_EVENTS)
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
	if(app->inPopupMode()) {
	    QWidget *clt;
	    qt_mac_find_window(where.h, where.v, &clt);
	    if(clt && clt->isPopup())
		widget = clt;
	    if(!widget)
		widget = activePopupWidget();
	    QMacSavedPortInfo savedInfo(widget);
	    Point gp = where;
	    GlobalToLocal(&gp); //now map it to the window
	    widget = qt_recursive_match(widget, gp.h, gp.v);
	} else {
	    if(ekind != kEventMouseDown && qt_button_down)
		widget = qt_button_down;
	    else if(mac_mouse_grabber)
		widget = mac_mouse_grabber;
	    else
		widget = QApplication::widgetAt(where.h, where.v, true);
	}
	if(!QMacBlockingFunction::blocking()) { //set the cursor up
	    const QCursor *n = NULL;
	    if(widget) { //only over the app, do we set a cursor..
		if(cursorStack) {
		    n = app_cursor;
		} else {
		    for(QWidget *p = widget; p; p = p->parentWidget()) {
			QWExtra *extra = ((QExtraWidget*)p)->extraData();
			if(extra && extra->curs) {
			    n = extra->curs;
			    break;
			}
		    }
		}
	    }
	    const QCursor def(Qt::ArrowCursor);
	    if(!n) n = &def; //I give up..
	    qt_mac_set_cursor(n, &where);
	}

	//This mouse button state stuff looks like this on purpose
	//although it looks hacky it is VERY intentional..
	if(widget && app_do_modal && !qt_try_modal(widget, event)) {
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

#if defined(QT_TABLET_SUPPORT)
	UInt32 tabletEventType;
	GetEventParameter(event, kEventParamTabletEventType, typeUInt32, NULL,
			  sizeof(tabletEventType), NULL, &tabletEventType);
	if (tabletEventType == kEventTabletPoint) {
	    TabletPointRec tabletPointRec;
	    GetEventParameter(event, kEventParamTabletPointRec, typeTabletPointRec, NULL,
			      sizeof(tabletPointRec), NULL, &tabletPointRec);
	    QEvent::Type t = QEvent::TabletMove; //default
	    int new_tablet_button_state = tabletPointRec.buttons ? 1 : 0;
            if (new_tablet_button_state != tablet_button_state)
		if (new_tablet_button_state)
		    t = QEvent::TabletPress;
		else
		    t = QEvent::TabletRelease;
	    tablet_button_state = new_tablet_button_state;
	    if (widget) {
		int dev = QTabletEvent::Stylus;
		int tiltX = ((int)tabletPointRec.tiltX)/(32767/60); // 32K -> 60
		int tiltY = ((int)tabletPointRec.tiltY)/(32767/60); // 32K -> 60
		int pressure = (int)tabletPointRec.pressure >> 8; // 32K -> 255
		QPair<int,int> uId( (int)tabletPointRec.deviceID, (int)tabletPointRec.vendor1 );
		QPoint p(where.h, where.v);
		QPoint plocal(widget->mapFromGlobal(p));
		QTabletEvent e( t, plocal, p, dev, pressure, tiltX, tiltY, uId );
		QApplication::sendSpontaneousEvent(widget, &e);
		if (e.isAccepted())
		    break;
	    }
	}
#endif

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
	    if((QWidget *)qt_mouseover != widget) {
#ifdef DEBUG_MOUSE_MAPS
		qDebug("Entering: %p - %s (%s), Leaving %s (%s)", (QWidget*)widget,
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

	if(widget) {
	    QPoint p(where.h, where.v);
	    QPoint plocal(widget->mapFromGlobal(p));
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
			s.replace(QRegExp(QString::fromLatin1("(\\&|\\<[^\\>]*\\>)")), "");
			SpeechChannel ch;
			NewSpeechChannel(NULL, &ch);
			SpeakText(ch, s.latin1(), s.length());
		    }
		}
#endif
		int macButton = button;
		if(button == QMouseEvent::LeftButton && (modifiers & controlKey))
		    macButton = QMouseEvent::RightButton;
		QMouseEvent qme(etype, plocal, p, macButton, state | keys);
		QApplication::sendSpontaneousEvent(widget, &qme);
	    }
	    if(ekind == kEventMouseDown &&
	       ((button == QMouseEvent::RightButton) ||
		(button == QMouseEvent::LeftButton && (modifiers & controlKey))))
		qt_event_request_context();

#ifdef DEBUG_MOUSE_MAPS
	    const char *event_desc = edesc;
	    if(etype == QEvent::MouseButtonDblClick)
		event_desc = "Double Click";
	    qDebug("%d %d (%d %d) - Would send (%s) event to %p %s %s (%d %d %d)", p.x(), p.y(),
		   plocal.x(), plocal.y(), event_desc, (QWidget*)widget, widget ? widget->name() : "*Unknown*",
		   widget ? widget->className() : "*Unknown*", button, state|keys, wheel_delta);
#endif
	} else {
	    handled_event = FALSE;
	}
	break;
    }
    case kEventClassTextInput:
	handled_event = FALSE;
	if(!(widget=focus_widget)) {
	    handled_event = FALSE;
	} else if(ekind == kEventTextInputOffsetToPos) {
	    if(qt_mac_input_spot != QT_MAC_ONTHESPOT) {
		handled_event = FALSE;
		break;
	    }

	    if(qt_mac_get_document_id(widget)) {
		QPoint mp(widget->mapToGlobal(QPoint(0, 0)));
		Point pt;
		pt.h = mp.x();
		pt.v = mp.y();
		SetEventParameter(event, kEventParamTextInputReplyPoint, typeQDPoint,
				  sizeof(pt), &pt);
		handled_event = TRUE;
	    }
	} else if(ekind == kEventTextInputUpdateActiveInputArea) {
	    if(qt_mac_input_spot != QT_MAC_ONTHESPOT) {
		handled_event = FALSE;
		break;
	    }

	    long refcon;
	    GetEventParameter(event, kEventParamTextInputSendRefCon, typeLongInteger, NULL,
			      sizeof(refcon), NULL, &refcon);
	    if(QTSMDocumentWrapper *doc = (QTSMDocumentWrapper*)refcon) {
		UInt32 unilen;
		GetEventParameter(event, kEventParamTextInputSendText, typeUnicodeText,
				  NULL, 0, &unilen, NULL);
		UniChar *unicode = (UniChar*)NewPtr(unilen);
		GetEventParameter(event, kEventParamTextInputSendText, typeUnicodeText,
				  NULL, unilen, NULL, unicode);
		QString text((QChar*)unicode, unilen / sizeof(UniChar));
		DisposePtr((char*)unicode);
		if(doc->inputWidget()) {
		    long fixed_length;
		    GetEventParameter(event, kEventParamTextInputSendFixLen, typeLongInteger, NULL,
				      sizeof(fixed_length), NULL, &fixed_length);
		    if(fixed_length == -1 || fixed_length == (long)unilen) {
			QIMEvent imend(QEvent::IMEnd, text, text.length());
			QApplication::sendSpontaneousEvent(doc->inputWidget(), &imend);
			if(imend.isAccepted()) {
			    doc->setInputWidget(NULL);
			    handled_event = TRUE;
			}
		    } else {
			if(fixed_length > 0) {
			    QIMEvent imend(QEvent::IMEnd, text.left(fixed_length / sizeof(UniChar)),
					   fixed_length / sizeof(UniChar));
			    QApplication::sendSpontaneousEvent(doc->inputWidget(), &imend);
			    if(imend.isAccepted()) {
				handled_event = TRUE;
				QIMEvent imstart(QEvent::IMStart, text.mid(fixed_length / sizeof(UniChar)),
						 (fixed_length - text.length()) / sizeof(UniChar));
				QApplication::sendSpontaneousEvent(doc->inputWidget(), &imstart);
				if(imstart.isAccepted())
				    handled_event = TRUE;
			    }
			} else {
			    QIMEvent imcompose(QEvent::IMCompose, text, text.length(), 0);
			    QApplication::sendSpontaneousEvent(doc->inputWidget(), &imcompose);
			    if(imcompose.isAccepted())
				handled_event = TRUE;
			}
		    }
		} else {
		    QIMEvent imstart(QEvent::IMStart, text, text.length());
		    QApplication::sendSpontaneousEvent(widget, &imstart);
		    if(imstart.isAccepted()) {
			handled_event = TRUE;
			doc->setInputWidget(widget);
			QIMEvent imcompose(QEvent::IMCompose, text, text.length(), 0);
			QApplication::sendSpontaneousEvent(doc->inputWidget(), &imcompose);
		    }
		}
	    }
	} else if(ekind == kEventTextInputUnicodeForKeyEvent) {
	    EventRef key_ev;
	    GetEventParameter(event, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL,
			      sizeof(key_ev), NULL, &key_ev);
	    QString text;
	    UInt32 unilen;
	    if(GetEventParameter(key_ev, kEventParamKeyUnicodes, typeUnicodeText, NULL, 0, &unilen, NULL) == noErr) {
		UniChar *unicode = (UniChar*)NewPtr(unilen);
		GetEventParameter(key_ev, kEventParamKeyUnicodes, typeUnicodeText, NULL, unilen, NULL, unicode);
		text = QString((QChar*)unicode, unilen / sizeof(UniChar));
		DisposePtr((char*)unicode);
	    }
	    char chr;
	    GetEventParameter(key_ev, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(chr), NULL, &chr);
	    if(!chr || (text.length() > 0 && (text.length() > 1 || text.at(0) != QChar(chr)))) {
		QIMEvent imstart(QEvent::IMStart, QString::null, -1);
		QApplication::sendSpontaneousEvent(widget, &imstart);
		if(imstart.isAccepted()) { //wants the event
		    handled_event = TRUE;
		    QIMEvent imend(QEvent::IMEnd, text, 1);
		    QApplication::sendSpontaneousEvent(widget, &imend);
		}
	    }
	}
	if(!handled_event) //just bail now
	    return eventNotHandledErr;
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
	    if(mac_keyboard_grabber)
		widget = mac_keyboard_grabber;
	    else if(focus_widget)
		widget = focus_widget;
	    if(!widget || (app_do_modal && !qt_try_modal(widget, event)))
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
		QKeyEvent ke(etype, key, modifiers, "", FALSE);
		QApplication::sendSpontaneousEvent(widget,&ke);
	    }
	    break;
	}

	UInt32 keyc;
	GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyc), NULL, &keyc);
	static UInt32 state = 0L;
	char chr = KeyTranslate((void *)GetScriptVariable(smCurrentScript, smKCHRCache),
		   (modif & (kEventKeyModifierNumLockMask|shiftKey|rightShiftKey|alphaLock)) | keyc, &state);
	if(!chr) {
	    if(CallNextEventHandler(er, event) == noErr)
		handled_event = TRUE;
	    break;
	}

	//map it into qt keys
	QString mystr;

#ifdef DEBUG_KEY_MAPS
	qDebug("************ Mapping modifiers and key ***********");
#endif
	int modifiers = get_modifiers(modif), mychar=get_key(modifiers, chr, keyc);
#ifdef DEBUG_KEY_MAPS
	qDebug("------------ Mapping modifiers and key -----------");
#endif
	if(modifiers & (Qt::AltButton | Qt::ControlButton)) {
	    if(chr & (1 << 7))
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
	}
	/* I don't know why the str is only filled in in RawKeyDown - but it does seem to be on X11
	   is this a bug on X11? --Sam ### */
	QEvent::Type etype = (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress;
	if(etype == QEvent::KeyPress) {
	    UInt32 unilen;
	    if(GetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, NULL, 0, &unilen, NULL) == noErr && unilen == 2) {
		UniChar *unicode = (UniChar*)NewPtr(unilen);
		GetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, NULL, unilen, NULL, unicode);
		mystr = QString((QChar*)unicode, unilen / sizeof(UniChar));
		DisposePtr((char *)unicode);
	    } else if(chr) {
		static QTextCodec *c = NULL;
		if(!c)
		    c = QTextCodec::codecForName("Apple Roman");
		mystr = c->toUnicode(&chr, 1);
	    }
	}

	if(mac_keyboard_grabber)
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;
	if(widget) {
	    if(app_do_modal && !qt_try_modal(widget, event))
		break;

	    bool key_event = TRUE;
	    if(etype == QEvent::KeyPress && !mac_keyboard_grabber) {
		/* We offer the accelerator a text representation of chr, this is because the Mac
		   actually flips the keyboard when things like alt are pressed, but that doesn't
		   really mean that accelerators should be mapped to the new key (or things could get
		   quite broken). */
		QString accel_str;
		if(chr) {
		    static QTextCodec *c = NULL;
		    if(!c)
			c = QTextCodec::codecForName("Apple Roman");
		    accel_str = c->toUnicode(&chr, 1);
		}
		QKeyEvent accel_ev(QEvent::AccelOverride, mychar, modifiers,
				   accel_str, ekind == kEventRawKeyRepeat,
				   QMAX(1, accel_str.length()));
		if(qt_tryAccelEvent(widget, &accel_ev)) {
#ifdef DEBUG_KEY_MAPS
		    qDebug("KeyEvent: %s::%s consumed Accel: %04x %c %s %d",
			   widget ? widget->className() : "none", widget ? widget->name() : "",
			   mychar, chr, mystr.latin1(), ekind == kEventRawKeyRepeat);
#endif
		    key_event = FALSE;
		} else {
		    accel_ev.t = QEvent::AccelOverride;
		    QApplication::sendSpontaneousEvent(widget, &accel_ev);
		    if(accel_ev.isAccepted()) {
#ifdef DEBUG_KEY_MAPS
			qDebug("KeyEvent: %s::%s overrode Accel: %04x %c %s %d",
			       widget ? widget->className() : "none", widget ? widget->name() : "",
			       mychar, chr, mystr.latin1(), ekind == kEventRawKeyRepeat);
#endif
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
				if(0 && !ProcessHICommand(&hic)) {
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
		if(CallNextEventHandler(er, event) == noErr) {
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
		/* This is actually wrong - but unfortunatly it is the best that can be
		   done for now because of the Control/Meta mapping problems */
		if(modifiers & (Qt::ControlButton | Qt::MetaButton)) {
		    chr = 0;
		    mystr = "";
		}
		QKeyEvent ke(etype, mychar, modifiers,
			     mystr, ekind == kEventRawKeyRepeat,
			     QMAX(1, mystr.length()));
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
		    qDebug("Qt: internal: Unexpected condition reached. %s:%d", __FILE__, __LINE__);
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
	    if(ekind == kEventWindowShown)
		unhandled_dialogs.insert(wid, 1);
	    else if(ekind == kEventWindowHidden)
		unhandled_dialogs.remove(wid);
	    handled_event = FALSE;
	    break;
	} else if(widget->isDesktop()) {
	    handled_event = FALSE;
	    break;
	}

	if(ekind == kEventWindowUpdate || ekind == kEventWindowDrawContent) {
	    remove_context_timer = FALSE;
	    widget->propagateUpdates(ekind == kEventWindowUpdate);
	} else if(ekind == kEventWindowDispose) {
	    qt_mac_unicode_cleanup(widget);
	} else if(ekind == kEventWindowExpanded) {
	    QShowEvent qse;
	    QApplication::sendSpontaneousEvent(widget, &qse);
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
		    QMoveEvent qme(widget->crect.topLeft(), QPoint(ox, oy));
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
	} else if(ekind == kEventWindowHidden) {
	} else if(ekind == kEventWindowShown) {
	    if(!widget->testWFlags(WType_Popup))
		widget->topLevelWidget()->setActiveWindow();
	} else if(ekind == kEventWindowActivated) {
	    if(QApplication::app_style) {
		//I shouldn't have to do this, but the StyleChanged isn't happening as I expected
		//so this is in for now, FIXME!
		QEvent ev(QEvent::Style);
		QApplication::sendSpontaneousEvent(QApplication::app_style, &ev);
	    }

	    if(QTSMDocumentWrapper *doc = qt_mac_get_document_id(widget))
		ActivateTSMDocument(doc->document());
	    if(app_do_modal && !qt_try_modal(widget, event))
		break;

	    if(widget && widget->topLevelWidget()->isVisible()) {
		QWidget *tlw = widget->topLevelWidget();
		if(tlw->isTopLevel() && !tlw->isPopup() && (tlw->isModal() ||
							    !tlw->testWFlags(WStyle_Tool))) {
		    bool just_send_event = FALSE;
		    {
			WindowActivationScope scope;
			if(GetWindowActivationScope((WindowRef)wid, &scope) == noErr &&
			   scope == kWindowActivationScopeIndependent) {
			    if(GetFrontWindowOfClass(kAllWindowClasses, true) != wid)
				just_send_event = TRUE;
			}
		    }
		    if(just_send_event) {
			QEvent e(QEvent::WindowActivate);
			QApplication::sendSpontaneousEvent(widget, &e);
		    } else {
			app->setActiveWindow(tlw);
		    }
		}
		if(widget->focusWidget())
		    widget->focusWidget()->setFocus();
		else
		    widget->setFocus();
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
		QMenuBar::macUpdateMenuBar();
#endif
	    }
	} else if(ekind == kEventWindowDeactivated) {
	    if(QTSMDocumentWrapper *doc = qt_mac_get_document_id(widget))
		DeactivateTSMDocument(doc->document());
	    if(widget && active_window == widget)
		app->setActiveWindow(NULL);
	} else {
	    handled_event = FALSE;
	}
	break; }
    case kEventClassApplication:
	if(ekind == kEventAppActivated) {
	    if(QApplication::desktopSettingsAware())
		qt_mac_update_os_settings();
	    app->clipboard()->loadScrap(FALSE);
	    if(qt_clipboard) { //manufacture an event so the clipboard can see if it has changed
		QEvent ev(QEvent::Clipboard);
		QApplication::sendSpontaneousEvent(qt_clipboard, &ev);
	    }
	    if(!app->activeWindow()) {
		WindowPtr wp = ActiveNonFloatingWindow();
		if(wp && !unhandled_dialogs.contains(wp)) {
		    if(QWidget *tmp_w = QWidget::find((WId)wp))
			app->setActiveWindow(tmp_w);
		}
	    }
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
	    QMenuBar::macUpdateMenuBar();
#endif
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
	if(ekind == kEventMenuOpening || ekind == kEventMenuClosed) {
	    MenuRef mr;
	    GetEventParameter(event, kEventParamDirectObject, typeMenuRef,
			      NULL, sizeof(mr), NULL, &mr);
	    if(ekind == kEventMenuOpening) {
		Boolean first;
		GetEventParameter(event, kEventParamMenuFirstOpen, typeBoolean,
				  NULL, sizeof(first), NULL, &first);
		if(first && !QMenuBar::macUpdatePopup(mr))
		    handled_event = FALSE;
	    }
	    if(handled_event) {
		if(!QMenuBar::macUpdatePopupVisible(mr, ekind == kEventMenuOpening))
		    handled_event = FALSE;
	    }
	} else if(ekind == kEventMenuTargetItem) {
	    MenuRef mr;
	    GetEventParameter(event, kEventParamDirectObject, typeMenuRef,
			      NULL, sizeof(mr), NULL, &mr);
	    MenuItemIndex idx;
	    GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex,
			      NULL, sizeof(idx), NULL, &idx);
	    if(!QMenuBar::activate(mr, idx, TRUE))
		handled_event = FALSE;
	} else {
	    handled_event = FALSE;
	}
#else
	handled_event = FALSE;
#endif
	break;
    case kAppearanceEventClass:
	if(ekind == kAEAppearanceChanged) {
	    if(QApplication::desktopSettingsAware())
		qt_mac_update_os_settings();
	    if(QApplication::app_style) {
		QEvent ev(QEvent::Style);
		QApplication::sendSpontaneousEvent(QApplication::app_style, &ev);
	    }
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
		    if(!qt_modal_state() || IsMenuCommandEnabled(NULL, kHICommandQuit)) {
			QCloseEvent ev;
			QApplication::sendSpontaneousEvent(app, &ev);
			if(ev.isAccepted()) {
			    handled_event = TRUE;
			    app->quit();
			}
		    }
		    break; }
		default:
		    break;
		}
	    }
	    if(!handled_event) {
		EventRecord erec;
		if(!ConvertEventRefToEventRecord(event, &erec))
		    qDebug("Qt: internal: WH0A, unexpected condition reached. %s:%d", __FILE__, __LINE__);
		else if(AEProcessAppleEvent(&erec) == noErr)
		    handled_event = TRUE;
	    }
	}
	break;
    case kEventClassCommand:
	if(ekind == kEventCommandProcess) {
	    HICommand cmd;
	    GetEventParameter(event, kEventParamDirectObject, typeHICommand,
			      NULL, sizeof(cmd), NULL, &cmd);
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
		    bool by_accel = FALSE;
#if QT_MACOSX_VERSION >= 0x1020
		    UInt32 command_flags;
		    if(!GetEventParameter(event, kEventParamMenuContext, typeUInt32,
					  NULL, sizeof(command_flags), NULL, &command_flags)) {
			by_accel = (command_flags & kMenuContextKeyMatching);
		    } else
#endif
		    {
			UInt32 keyc;
			by_accel = !GetEventParameter(event, kEventParamKeyModifiers, typeUInt32,
						      NULL, sizeof(keyc), NULL, &keyc) && keyc;
		    }
		    if(by_accel || !QMenuBar::activate(cmd.menu.menuRef, cmd.menu.menuItemIndex, FALSE, by_accel))
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
	if(request_context_hold_pending) {
	    RemoveEventFromQueue(GetMainEventQueue(), request_context_hold_pending);
	    ReleaseEvent(request_context_hold_pending);
	    request_context_hold_pending = NULL;
	}
    }

#ifdef DEBUG_EVENTS
    qDebug("%shandled event %c%c%c%c %d", handled_event ? "(*) " : "",
	   char(eclass >> 24), char((eclass >> 16) & 255), char((eclass >> 8) & 255),
	   char(eclass & 255), (int)ekind);
#endif
    if(!handled_event) //let the event go through
	return eventNotHandledErr;
#ifdef QMAC_USE_APPLICATION_EVENT_LOOP
    QuitApplicationEventLoop();
#endif
    return noErr; //we eat the event
}

/*!
  This virtual function is only implemented under Macintosh.

  If you create an application that inherits QApplication and
  reimplement this function, you get direct access to all Carbon Events
  that are received from the MacOS.

  Return TRUE if you want to stop the event from being processed.
  Return FALSE for normal event dispatching.
*/
bool QApplication::macEventFilter(EventHandlerCallRef, EventRef)
{
    return FALSE;
}

void QApplication::openPopup(QWidget *popup)
{
    if(!popupWidgets) {			// create list
	popupWidgets = new QWidgetList;
    }
    popupWidgets->append(popup);		// add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason(QFocusEvent::Popup);
    if(popup->focusWidget())
	popup->focusWidget()->setFocus();
    else
	popup->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup(QWidget *popup)
{
    if(!popupWidgets)
	return;

    popupWidgets->remove(popup);
    if(popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if(popup == qt_button_down) {
	mouse_button_state = 0;
	qt_button_down = NULL;
    }
    if(popupWidgets->isEmpty()) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( active_window ) {
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    if ( active_window->focusWidget() )
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	    QFocusEvent::resetReason();
	}
    } else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	active_window = popupWidgets->last();
	QFocusEvent::setReason(QFocusEvent::Popup);
	if(active_window->focusWidget())
	    active_window->focusWidget()->setFocus();
	else
	    active_window->setFocus();
	QFocusEvent::resetReason();
    }
}

void  QApplication::setCursorFlashTime(int msecs)
{
    cursor_flash_time = msecs;
}

int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

void QApplication::setDoubleClickInterval(int ms)
{
    qt_mac_dblclick.use_qt_time_limit = 1;
    mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time; //FIXME: What is the default value on the Mac?
}

void QApplication::setWheelScrollLines(int n)
{
    wheel_scroll_lines = n;
}

int QApplication::wheelScrollLines()
{
    return wheel_scroll_lines;
}

void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
    switch (effect) {
    case UI_FadeMenu:
	fade_menu = enable;
	if(!enable)
	    break;
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeTooltip:
	fade_tooltip = enable;
	if(!enable)
	    break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateToolBox:
	animate_toolbox = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }
}

bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
    if ( QColor::numBitPlanes() < 16 || !animate_ui )
	return FALSE;

    switch(effect) {
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
    case UI_AnimateToolBox:
	return animate_toolbox;
    default:
	return animate_ui;
    }
}

void QApplication::flush()
{
//    sendPostedEvents();
    if(qApp) {
	QWidgetList tlws = qApp->topLevelWidgets();
	for(int i = 0; i < tlws.size(); i++) {
	    QWidget *tlw = tlws.at(i);
	    if(tlw->isVisible()) {
		for(QValueList<WId>::Iterator it = request_updates_pending_list.begin();
		    it != request_updates_pending_list.end(); ++it) {
		    QWidget *widget = QWidget::find((*it));
		    if(!widget)
			continue;
		    QWExtra *extra = ((QExtraWidget*)widget)->extraData();
		    if(extra && extra->has_dirty_area && widget->topLevelWidget() == tlw) {
			extra->has_dirty_area = FALSE;
			QRegion r = extra->dirty_area;
			extra->dirty_area = QRegion();
			QRegion cr = widget->clippedRegion();
			if(!widget->isTopLevel()) {
			    QPoint point(posInWindow(widget));
			    cr.translate(-point.x(), -point.y());
			}
			r &= cr;
			if(!r.isEmpty())
			    widget->repaint(r);

			it = request_updates_pending_list.remove(it);
			if(it == request_updates_pending_list.end())
			    break;
		    }
		}
		tlw->propagateUpdates();
		QMacSavedPortInfo::flush(tlw);
	    }
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
	QString("/qt/%1.%2/libraryPath").arg(QT_VERSION >> 16).arg((QT_VERSION & 0xff00) >> 8);
    QStringList pathlist = settings.readListEntry(libpathkey, ':');
    if(!pathlist.isEmpty()) {
	QStringList::ConstIterator it = pathlist.begin();
	while(it != pathlist.end())
	    QApplication::addLibraryPath(*it++);
    }

    QString defaultcodec = settings.readEntry("/qt/defaultCodec", "none");
    if(defaultcodec != "none") {
	QTextCodec *codec = QTextCodec::codecForName(defaultcodec);
	if(codec)
	    qApp->setDefaultCodec(codec);
    }

    qt_resolve_symlinks = settings.readBoolEntry("/qt/resolveSymlinks", TRUE);

    if(qt_is_gui_used) {
	QString str;
	QStringList strlist;
	int i, num;
	QPalette pal(QApplication::palette());
	strlist = settings.readListEntry("/qt/Palette/active");
	if(strlist.count() == QPalette::NColorRoles) {
	    for(i = 0; i < QPalette::NColorRoles; i++)
		pal.setColor(QPalette::Active, (QPalette::ColorRole) i,
			     QColor(strlist[i]));
	}
	strlist = settings.readListEntry("/qt/Palette/inactive");
	if(strlist.count() == QPalette::NColorRoles) {
	    for(i = 0; i < QPalette::NColorRoles; i++)
		pal.setColor(QPalette::Inactive, (QPalette::ColorRole) i, QColor(strlist[i]));
	}
	strlist = settings.readListEntry("/qt/Palette/disabled");
	if(strlist.count() == QPalette::NColorRoles) {
	    for(i = 0; i < QPalette::NColorRoles; i++)
		pal.setColor(QPalette::Disabled, (QPalette::ColorRole) i, QColor(strlist[i]));
	}
	if(pal != QApplication::palette())
	    QApplication::setPalette(pal, TRUE);

	QFont font(QApplication::font());     // read new font
	str = settings.readEntry("/qt/font");
	if(!str.isNull() && !str.isEmpty()) {
	    font.fromString(str);
	    if(font != QApplication::font())
		QApplication::setFont(font, TRUE);
	}

	// read new QStyle
	QString stylename = settings.readEntry("/qt/style");
	if(! stylename.isNull() && ! stylename.isEmpty()) {
	    QStyle *style = QStyleFactory::create(stylename);
	    if(style)
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
	if(colorspec == "normal")
	    QApplication::setColorSpec(QApplication::NormalColor);
	else if(colorspec == "custom")
	    QApplication::setColorSpec(QApplication::CustomColor);
	else if(colorspec == "many")
	    QApplication::setColorSpec(QApplication::ManyColor);
	else if(colorspec != "default")
	    colorspec = "default";

	QStringList strut = settings.readListEntry("/qt/globalStrut");
	if(!strut.isEmpty()) {
	    if(strut.count() == 2) {
		QSize sz(strut[0].toUInt(), strut[1].toUInt());
		if(sz.isValid())
		    QApplication::setGlobalStrut(sz);
	    }
	}

	QStringList effects = settings.readListEntry("/qt/GUIEffects");
	if(!effects.isEmpty()) {
	    if(effects.contains("none"))
		QApplication::setEffectEnabled(Qt::UI_General, FALSE);
	    if(effects.contains("general"))
		QApplication::setEffectEnabled(Qt::UI_General, TRUE);
	    if(effects.contains("animatemenu"))
		QApplication::setEffectEnabled(Qt::UI_AnimateMenu, TRUE);
	    if(effects.contains("fademenu"))
		QApplication::setEffectEnabled(Qt::UI_FadeMenu, TRUE);
	    if(effects.contains("animatecombo"))
		QApplication::setEffectEnabled(Qt::UI_AnimateCombo, TRUE);
	    if(effects.contains("animatetooltip"))
		QApplication::setEffectEnabled(Qt::UI_AnimateTooltip, TRUE);
	    if(effects.contains("fadetooltip"))
		QApplication::setEffectEnabled(Qt::UI_FadeTooltip, TRUE);
	    if(effects.contains("animatetoolbox"))
		QApplication::setEffectEnabled(Qt::UI_AnimateToolBox, TRUE);
	} else {
	    QApplication::setEffectEnabled(Qt::UI_General, FALSE);
	}

	QStringList fontsubs = settings.entryList("/qt/Font Substitutions");
	if(!fontsubs.isEmpty()) {
	    QStringList subs;
	    QString fam, skey;
	    QStringList::Iterator it = fontsubs.begin();
	    while(it != fontsubs.end()) {
		fam = (*it++).latin1();
		skey = "/qt/Font Substitutions/" + fam;
		subs = settings.readListEntry(skey);
		QFont::insertSubstitutions(fam, subs);
	    }
	}
    }
    return TRUE;
}

/*!
    \enum Qt::MacintoshVersion

    \value MV_Unknown Version cannot be detected
    \value MV_9 Mac OS 9
    \value MV_10_DOT_3 Mac OS X 10.3
    \value MV_10_DOT_2 Mac OS X 10.2
    \value MV_10_DOT_1 Mac OS X 10.1
    \value MV_10_DOT_0 Mac OS X 10.0

    \value MV_CHEETAH 10.0 Codename
    \value MV_PUMA    10.1 Codename
    \value MV_JAGUAR  10.2 Codename
    \value MV_PANTHER 10.3 Codename
*/


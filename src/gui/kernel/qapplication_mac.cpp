/****************************************************************************
**
** Implementation of Mac startup routines and event handling.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"
#include "qbitarray.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qdatastream.h"
#include "qdatetime.h"
#include "qdesktopwidget.h"
#include "qdockarea.h"
#include "qevent.h"
#include "qpointer.h"
#include "qhash.h"
#include "qmessagebox.h"
#include "qmime.h"
#include "qpaintdevicemetrics.h"
#include "qpaintengine_mac.h"
#include "qpixmapcache.h"
#include "qsessionmanager.h"
#include "qsettings.h"
#include "qsocketnotifier.h"
#include "qstyle.h"
#include "qstylefactory.h"
#include "qtextcodec.h"
#include "qvariant.h"
#include "qwidget.h"
#include "qmenubar.h"

#include "private/qapplication_p.h"
#include "private/qcolor_p.h"
#include "private/qguieventloop_p.h"
#include "private/qwidget_p.h"

#ifndef QT_NO_MAINWINDOW
# include "qmainwindow.h"
#endif



#if defined(QT_ACCESSIBILITY_SUPPORT)
#  include "qaccessible.h"
#endif

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif

#include "qdir.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>


#include <string.h>
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
QMutex *qt_mac_port_mutex = 0;
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
static bool        app_do_modal        = false;        // modal mode
extern QWidgetList *qt_modal_stack;                // stack of modal widgets
extern bool qt_mac_in_drag; //qdnd_mac.cpp
extern bool qt_resolve_symlinks; // from qapplication.cpp
extern bool qt_tab_all_widgets; // from qapplication.cpp
bool qt_mac_app_fullscreen = false;
bool qt_scrollbar_jump_to_pos = false;
static bool qt_mac_collapse_on_dblclick = true;
QPointer<QWidget> qt_button_down;                // widget got last button-down
extern bool qt_tryAccelEvent(QWidget*, QKeyEvent*); // def in qaccel.cpp
static QPointer<QWidget> qt_mouseover;
static QHash<WindowRef, int> unhandled_dialogs;        //all unhandled dialogs (ie mac file dialog)
static enum { QT_MAC_OFFTHESPOT, QT_MAC_ONTHESPOT } qt_mac_input_spot = QT_MAC_ONTHESPOT;
#if defined(QT_DEBUG)
static bool        appNoGrab        = false;        // mouse/keyboard grabbing
#endif
static bool qt_mac_press_and_hold_context = false;
static EventLoopTimerRef mac_context_timer = 0;
static EventLoopTimerUPP mac_context_timerUPP = 0;
static DMExtendedNotificationUPP mac_display_changeUPP = 0;
static EventHandlerRef app_proc_handler = 0;
static EventHandlerUPP app_proc_handlerUPP = 0;
static AEEventHandlerUPP app_proc_ae_handlerUPP = NULL;
//popup variables
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool            popupCloseDownMode = false;

class QETWidget : public QWidget
{
public:
    inline QWExtra* extraData();
    inline QTLWExtra* topData();
};
inline QWExtra* QETWidget::extraData() { return d->extraData(); }
inline QTLWExtra* QETWidget::topData() { return d->topData(); }

/*****************************************************************************
  External functions
 *****************************************************************************/
extern bool qt_mac_is_macdrawer(QWidget *); //qwidget_mac.cpp
extern WindowPtr qt_mac_window_for(HIViewRef); //qwidget_mac.cpp
extern QWidget *qt_mac_find_window(WindowPtr); //qwidget_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const Point *); //qcursor_mac.cpp
extern bool qt_mac_is_macsheet(QWidget *); //qwidget_mac.cpp
extern QString qt_mac_get_global_setting(QString key, QString val, QString file=QString::null); //qsettings_mac.cpp
extern QString pstring2qstring(const unsigned char *); //qglobal.cpp
extern void qt_mac_command_set_enabled(MenuRef, UInt32, bool); //qmenu_mac.cpp

/* Unicode input entry magic */
class QTSMDocumentWrapper
{
    QPointer<QWidget> act;
    TSMDocumentID id;
public:
    QTSMDocumentWrapper() {
        InterfaceTypeList itl = { kUnicodeDocument };
        NewTSMDocument(1, itl, &id, (long)this);
    }
    ~QTSMDocumentWrapper() {
        DeleteTSMDocument(id);
    }
    inline bool operator==(const QTSMDocumentWrapper &wrap) const {  return (act == wrap.act && wrap.id == id); }
    inline bool operator!=(const QTSMDocumentWrapper &wrap) const {  return !operator==(wrap); }
    TSMDocumentID document() const { return id; }

    void setInputWidget(QWidget *w) { act = w; }
    QWidget *inputWidget() const { return act; }
};
static QHash<WindowPtr, QTSMDocumentWrapper *> qt_mac_tsm_hash;
void qt_mac_unicode_init(QWidget *w)
{
    qt_mac_tsm_hash.ensure_constructed();
    WindowPtr window = qt_mac_window_for((HIViewRef)w->winId());
    if(!qt_mac_tsm_hash.contains(window))
        qt_mac_tsm_hash.insert(window, new QTSMDocumentWrapper());
}
void qt_mac_unicode_cleanup(QWidget *w)
{
    if(w && w->isTopLevel()) {
        qt_mac_tsm_hash.ensure_constructed();
        delete qt_mac_tsm_hash.take(qt_mac_window_for((HIViewRef)w->winId()));
    }
}
static QTSMDocumentWrapper *qt_mac_get_document_id(QWidget *w)
{
    if(!w)
        return 0;
    return qt_mac_tsm_hash.value(qt_mac_window_for((HIViewRef)w->winId()));
}
void qt_mac_unicode_reset_input(QWidget *w)
{
    if(QTSMDocumentWrapper *doc = qt_mac_get_document_id(w)) {
        if(doc->inputWidget() && doc->inputWidget() != w) {
            FixTSMDocument(doc->document());
            doc->setInputWidget(0);
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

//find widget (and part) at a given point
static short qt_mac_window_at(int x, int y, QWidget **w=0)
{
    Point p;
    p.h = x;
    p.v = y;
    WindowPtr wp;
    WindowPartCode wpc;
    OSStatus err = FindWindowOfClass(&p, kAllWindowClasses, &wp, &wpc);
    if(err != noErr) {
        if(w)
            (*w) = 0;
        return wpc;
    }
    if(w) {
        if(wp && !unhandled_dialogs.contains(wp)) {
            *w = qt_mac_find_window(wp);
#if 0
            if(!*w)
                qWarning("Qt: qt_mac_window_at: Couldn't find %d",(int)wp);
#endif
        } else {
            *w = 0;
        }
    }
    return wpc;
}

void qt_mac_set_press_and_hold_context(bool b) { qt_mac_press_and_hold_context = b; } //backdoor to enable press and hold

void qt_mac_secure_keyboard(bool b)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if(b) {
        if (EnableSecureEventInput)
            EnableSecureEventInput();
    } else {
        if (DisableSecureEventInput)
            DisableSecureEventInput();
    }
#else
    Q_UNUSED(b);
#endif
}

bool qt_nograb()                                // application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return false;
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
    if(!QApplication::startingUp()) {
        static bool needToPolish = true;
        if (needToPolish) {
            qApp->style().polish(qApp);
            needToPolish = false;
        }
    }
    { //focus mode
        /* I just reverse engineered this, I'm not so sure how well it will hold up but it works as of 10.2.3 */
        bool ok;
        int i = qt_mac_get_global_setting("AppleKeyboardUIMode", "0").toInt(&ok);
        qt_tab_all_widgets = !ok || (i & 0x2);
    }
    { //paging mode
        /* I just reverse engineered this, I'm not so sure how well it will hold up but it works as of 10.2.3 */
        QString paging = qt_mac_get_global_setting("AppleScrollerPagingBehavior", "FALSE");
        qt_scrollbar_jump_to_pos = (paging == "TRUE");
    }
    { //collapse
	/* I just reverse engineered this, I'm not so sure how well it will hold up but it works as of 10.3.3 */
	QString collapse = qt_mac_get_global_setting("AppleMiniaturizeOnDoubleClick", "TRUE");
	qt_mac_collapse_on_dblclick = (collapse == "TRUE");
    }

#ifdef DEBUG_PLATFORM_SETTINGS
    qDebug("qt_mac_update_os_settings *********************************************************************");
#endif
    { //setup the global peltte
        QColor qc;
        RGBColor c;
        QPalette pal = QApplication::palette();
        if(!GetThemeBrushAsColor(kThemeBrushPrimaryHighlightColor, 32, true, &c))
            pal.setBrush(QPalette::Active, QPalette::Highlight,
                         QColor(c.red / 256, c.green / 256, c.blue / 256));
        if(!GetThemeBrushAsColor(kThemeBrushSecondaryHighlightColor, 32, true, &c)) {
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
        if(pal != QApplication::palette()) {
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
        QApplication::setFont(QFont(pstring2qstring(f_name), f_size,
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
            { "QMenu", kThemeMenuItemFont },
            { "Q3MenuBar", kThemeMenuTitleFont },
            { "Q3PopupMenu", kThemeMenuItemFont },
            { "QHeader", kThemeSmallSystemFont },
            { "QTipLabel", kThemeSmallSystemFont },
            { "QMessageBoxLabel", kThemeEmphasizedSystemFont },
            { "QLabel", kThemeSystemFont },
            { 0, 0 } };
        Str255 f_name;
        SInt16 f_size;
        Style f_style;
        for(int i = 0; mac_widget_fonts[i].qt_class; i++) {
            GetThemeFont(mac_widget_fonts[i].font_key, smSystemScript, f_name, &f_size, &f_style);
            QFont fnt(pstring2qstring(f_name), f_size, (f_style & ::bold) ? QFont::Bold : QFont::Normal,
                      (bool)(f_style & ::italic));
            bool set_font = true;
            extern QHash<QByteArray, QFont> app_fonts;  //qapplication.cpp
            if(!app_fonts.isEmpty()) {
                QHash<QByteArray, QFont>::ConstIterator it = app_fonts.find(mac_widget_fonts[i].qt_class);
                if (it != app_fonts.constEnd())
                    set_font = (fnt != *it);
            }
            if(set_font) {
                QApplication::setFont(fnt, mac_widget_fonts[i].qt_class);
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
        } mac_widget_colors[] = {
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
            { "QMenu", kThemeTextColorPopupLabelActive, kThemeTextColorPopupLabelInactive },
            { "Q3PopupMenu", kThemeTextColorPopupLabelActive, kThemeTextColorPopupLabelInactive },
            { 0, 0, 0 } };
        QColor qc;
        RGBColor c;
        QPalette apppal = QApplication::palette();
        for(int i = 0; mac_widget_colors[i].qt_class; i++) {
            QPalette pal = apppal;
            if(!GetThemeTextColor(mac_widget_colors[i].active, 32, true, &c)) {
                qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
                pal.setColor(QPalette::Active, QPalette::Text, qc);
                pal.setColor(QPalette::Active, QPalette::Foreground, qc);
                pal.setColor(QPalette::Active, QPalette::HighlightedText, qc);
            }
            if(!GetThemeTextColor(mac_widget_colors[i].inactive, 32, true, &c)) {
                qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
                pal.setColor(QPalette::Inactive, QPalette::Text, qc);
                pal.setColor(QPalette::Disabled, QPalette::Text, qc);
                pal.setColor(QPalette::Inactive, QPalette::Foreground, qc);
                pal.setColor(QPalette::Disabled, QPalette::Foreground, qc);
                pal.setColor(QPalette::Inactive, QPalette::HighlightedText, qc);
                pal.setColor(QPalette::Disabled, QPalette::HighlightedText, qc);
            }
            if(!strcmp(mac_widget_colors[i].qt_class, "QMenu") || 
               !strcmp(mac_widget_colors[i].qt_class, "Q3PopupMenu")) {
                GetThemeTextColor(kThemeTextColorMenuItemActive, 32, true, &c);
                pal.setBrush(QPalette::ButtonText, QColor(c.red / 256, c.green / 256, c.blue / 256));
                GetThemeTextColor(kThemeTextColorMenuItemSelected, 32, true, &c);
                pal.setBrush(QPalette::HighlightedText, QColor(c.red / 256, c.green / 256, c.blue / 256));
                GetThemeTextColor(kThemeTextColorMenuItemDisabled, 32, true, &c);
                pal.setBrush(QPalette::Disabled, QPalette::Text,
                             QColor(c.red / 256, c.green / 256, c.blue / 256));
            } else if(!strcmp(mac_widget_colors[i].qt_class, "QButton") ||
                      !strcmp(mac_widget_colors[i].qt_class, "QHeader")) { //special
                pal.setColor(QPalette::Disabled, QPalette::ButtonText,
                             pal.color(QPalette::Disabled, QPalette::Text));
                pal.setColor(QPalette::Inactive, QPalette::ButtonText,
                             pal.color(QPalette::Inactive, QPalette::Text));
                pal.setColor(QPalette::Active, QPalette::ButtonText,
                             pal.color(QPalette::Active, QPalette::Text));
            }
            bool set_palette = true;
            extern QHash<QByteArray, QPalette> app_palettes; //qapplication.cpp
            if(!app_palettes.isEmpty()) {
                QHash<QByteArray, QPalette>::ConstIterator it = app_palettes.find(mac_widget_colors[i].qt_class);
                if (it != app_palettes.constEnd())
                    set_palette = (pal != *it);
            }
            if(set_palette && pal != apppal) {
                QApplication::setPalette(pal, mac_widget_colors[i].qt_class);
#ifdef DEBUG_PLATFORM_SETTINGS
                qt_mac_debug_palette(pal, apppal, QString("Palette for ") + mac_widget_colors[i].qt_class);
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
    //AE types
    typeAEClipboardChanged = 1,
    //types
    typeQWidget = 1,  /* QWidget *  */
    typeMacTimerInfo = 2, /* MacTimerInfo * */
    typeQGuiEventLoop = 3, /* QGuiEventLoop * */
    //params
    kEventParamMacTimer = 'qtim',     /* typeMacTimerInfo */
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    kEventParamQGuiEventLoop = 'qlop', /* typeQGuiEventLoop */
    //events
    kEventQtRequestSelect = 12,
    kEventQtRequestContext = 13,
    kEventQtRequestMenubarUpdate = 14,
    kEventQtRequestTimer = 15,
    kEventQtRequestWakeup = 16,
    kEventQtRequestShowSheet = 17,
    kEventQtRequestActivate = 18,
    kEventQtRequestSocketAct = 19
};
static void qt_mac_event_release(EventRef &event)
{
    ReleaseEvent(event);
    event = 0;
}
static void qt_mac_event_release(QWidget *w, EventRef &event)
{
    if (event) {
        if (IsEventInQueue(GetMainEventQueue(), event))  {
            QWidget *widget = 0;
            GetEventParameter(event, kEventParamQWidget, typeQWidget, 0, sizeof(widget), 0, &widget);
            if (w == widget) {
                RemoveEventFromQueue(GetMainEventQueue(), event);
                qt_mac_event_release(event);
            }
        }
    }
}

static bool qt_event_remove(EventRef &event)
{
    if (event) {
        if (IsEventInQueue(GetMainEventQueue(), event))  {
            RemoveEventFromQueue(GetMainEventQueue(), event);
            qt_mac_event_release(event);
        }
        return true;
    }
    return false;
}

/* socket notifiers */
static EventRef request_select_pending = 0;
void qt_event_request_select(QGuiEventLoop *loop) {
    if(request_select_pending) {
        if(IsEventInQueue(GetMainEventQueue(), request_select_pending))
            return;
#ifdef DEBUG_DROPPED_EVENTS
        qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(0, kEventClassQt, kEventQtRequestSelect, GetCurrentEventTime(),
                kEventAttributeUserEvent, &request_select_pending);
    SetEventParameter(request_select_pending,
                      kEventParamQGuiEventLoop, typeQGuiEventLoop, sizeof(loop), &loop);
    PostEventToQueue(GetMainEventQueue(), request_select_pending, kEventPriorityStandard);
    ReleaseEvent(request_select_pending);
}
static EventRef request_sockact_pending = 0;
void qt_event_request_sockact(QGuiEventLoop *loop) {
    if(request_sockact_pending) {
        if(IsEventInQueue(GetMainEventQueue(), request_sockact_pending))
            return;
#ifdef DEBUG_DROPPED_EVENTS
        qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(0, kEventClassQt, kEventQtRequestSocketAct, GetCurrentEventTime(),
                kEventAttributeUserEvent, &request_sockact_pending);
    SetEventParameter(request_sockact_pending,
                      kEventParamQGuiEventLoop, typeQGuiEventLoop, sizeof(loop), &loop);
    PostEventToQueue(GetMainEventQueue(), request_sockact_pending, kEventPriorityStandard);
    ReleaseEvent(request_sockact_pending);
}

/* sheets */
static EventRef request_showsheet_pending = 0;
void qt_event_request_showsheet(QWidget *w)
{
    Q_ASSERT(qt_mac_is_macsheet(w));
    qt_event_remove(request_showsheet_pending);
    CreateEvent(0, kEventClassQt, kEventQtRequestShowSheet, GetCurrentEventTime(),
                kEventAttributeUserEvent, &request_showsheet_pending);
    SetEventParameter(request_showsheet_pending, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), request_showsheet_pending, kEventPriorityStandard);
    ReleaseEvent(request_showsheet_pending);
}

/* wakeup */
static EventRef request_wakeup_pending = 0;
void qt_event_request_wakeup()
{
    if(request_wakeup_pending) {
        if(IsEventInQueue(GetMainEventQueue(), request_wakeup_pending))
            return;
#ifdef DEBUG_DROPPED_EVENTS
        qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(0, kEventClassQt, kEventQtRequestWakeup, GetCurrentEventTime(),
                kEventAttributeUserEvent, &request_wakeup_pending);
    PostEventToQueue(GetMainEventQueue(), request_wakeup_pending,
                     kEventPriorityHigh);
    ReleaseEvent(request_wakeup_pending);
}

/* activation */
static struct {
    QPointer<QWidget> widget;
    EventRef event;
    EventLoopTimerRef timer;
    EventLoopTimerUPP timerUPP;
} request_activate_pending = { 0, 0, 0, 0 };
bool qt_event_remove_activate()
{
    if(request_activate_pending.timer) {
        RemoveEventLoopTimer(request_activate_pending.timer);
        request_activate_pending.timer = 0;
    }
    if(request_activate_pending.event)
        qt_mac_event_release(request_activate_pending.event);
    return true;
}
QMAC_PASCAL void qt_event_activate_timer_callbk(EventLoopTimerRef r, void *)
{
    EventLoopTimerRef otc = request_activate_pending.timer;
    qt_event_remove_activate();
    if(r == otc && !request_activate_pending.widget.isNull()) {
        CreateEvent(0, kEventClassQt, kEventQtRequestActivate, GetCurrentEventTime(),
                    kEventAttributeUserEvent, &request_activate_pending.event);
        PostEventToQueue(GetMainEventQueue(), request_activate_pending.event, kEventPriorityHigh);
        ReleaseEvent(request_activate_pending.event);
    }
}
void qt_event_request_activate(QWidget *w)
{
    if(w == request_activate_pending.widget)
        return;

    /* We put these into a timer because due to order of events being sent we need to be sure this
       comes from inside of the event loop */
    qt_event_remove_activate();
    if(!request_activate_pending.timerUPP)
        request_activate_pending.timerUPP = NewEventLoopTimerUPP(qt_event_activate_timer_callbk);
    request_activate_pending.widget = w;
    InstallEventLoopTimer(GetMainEventLoop(), 0, 0, request_activate_pending.timerUPP, 0, &request_activate_pending.timer);
}

/* timers */
void qt_event_request_timer(MacTimerInfo *tmr)
{
    EventRef tmr_ev = 0;
    CreateEvent(0, kEventClassQt, kEventQtRequestTimer, GetCurrentEventTime(),
                kEventAttributeUserEvent, &tmr_ev);
    SetEventParameter(tmr_ev, kEventParamMacTimer, typeMacTimerInfo, sizeof(tmr), &tmr);
    PostEventToQueue(GetMainEventQueue(), tmr_ev, kEventPriorityStandard);
    ReleaseEvent(tmr_ev);
}
MacTimerInfo *qt_event_get_timer(EventRef event)
{
    if(GetEventClass(event) != kEventClassQt || GetEventKind(event) != kEventQtRequestTimer)
        return 0; //short circuit our tests..
    MacTimerInfo *t;
    GetEventParameter(event, kEventParamMacTimer, typeMacTimerInfo, 0, sizeof(t), 0, &t);
    return t;
}

/* menubars */
static EventRef request_menubarupdate_pending = 0;
void qt_event_request_menubarupdate()
{
    if(request_menubarupdate_pending) {
        if(IsEventInQueue(GetMainEventQueue(), request_menubarupdate_pending))
            return;
#ifdef DEBUG_DROPPED_EVENTS
        qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(0, kEventClassQt, kEventQtRequestMenubarUpdate, GetCurrentEventTime(),
                kEventAttributeUserEvent, &request_menubarupdate_pending);
    PostEventToQueue(GetMainEventQueue(), request_menubarupdate_pending, kEventPriorityHigh);
    ReleaseEvent(request_menubarupdate_pending);
}

//context menu
static EventRef request_context_pending = 0;
static void qt_event_request_context(QWidget *w=0, EventRef *where=0)
{
    if(!where)
        where = &request_context_pending;
    if(*where)
        return;
    CreateEvent(0, kEventClassQt, kEventQtRequestContext, GetCurrentEventTime(),
                kEventAttributeUserEvent, where);
    if(w)
        SetEventParameter(*where, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), *where, kEventPriorityStandard);
    ReleaseEvent(*where);
}
static EventRef request_context_hold_pending = 0;
QMAC_PASCAL void
QApplication::qt_context_timer_callbk(EventLoopTimerRef r, void *data)
{
    QWidget *w = (QWidget *)data;
    EventLoopTimerRef otc = mac_context_timer;
    RemoveEventLoopTimer(mac_context_timer);
    mac_context_timer = 0;
    if(r == otc && w == qt_button_down)
        qt_event_request_context(w, &request_context_hold_pending);
}

/* clipboard */
void qt_event_send_clipboard_changed()
{
#if 0
    AppleEvent ae;
    if(AECreateAppleEvent(kEventClassQt, typeAEClipboardChanged, 0, kAutoGenerateReturnID, kAnyTransactionID, &ae) != noErr)
        qDebug("Can't happen!!");
    AppleEvent reply;
    AESend(&ae, &reply,  kAENoReply, kAENormalPriority, kAEDefaultTimeout, 0, 0);
#endif
}

/* events that hold pointers to widgets, must be cleaned up like this */
void qt_mac_event_release(QWidget *w)
{
    if (w) {
        // cleanup show sheet pending
        qt_mac_event_release(w, request_showsheet_pending);
    }
}

/* watched events */
static EventTypeSpec events[] = {
    { kEventClassQt, kEventQtRequestTimer },
    { kEventClassQt, kEventQtRequestWakeup },
    { kEventClassQt, kEventQtRequestSelect },
    { kEventClassQt, kEventQtRequestShowSheet },
    { kEventClassQt, kEventQtRequestContext },
    { kEventClassQt, kEventQtRequestActivate },
    { kEventClassQt, kEventQtRequestMenubarUpdate },
    { kEventClassQt, kEventQtRequestSocketAct },

    { kEventClassWindow, kEventWindowInit },
    { kEventClassWindow, kEventWindowDispose },
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
        app_proc_handler = 0;
    }
}

bool qt_sendSpontaneousEvent(QObject *obj, QEvent *event)
{
    return QCoreApplication::sendSpontaneousEvent(obj, event);
}

/* platform specific implementations */
void qt_init(QApplicationPrivate *priv, QApplication::Type)
{
    if(qt_is_gui_used) {
        ProcessSerialNumber psn;
        if(GetCurrentProcess(&psn) == noErr) {
            if(!mac_display_changeUPP) {
                mac_display_changeUPP = NewDMExtendedNotificationUPP(qt_mac_display_change_callbk);
                DMRegisterExtendedNotifyProc(mac_display_changeUPP, 0, 0, &psn);
            }
#ifdef Q_WS_MAC
            SetFrontProcess(&psn);
#endif
        }
    }

    char **argv = priv->argv;

    // Get command line params
    if(int argc = priv->argc) {
        int i, j = 1;
        QString passed_psn;
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
                        QString s = QString(argv[i]).toLower();
                        if(s == "onthespot")
                            qt_mac_input_spot = QT_MAC_ONTHESPOT;
                        else if(s == "offthespot")
                            qt_mac_input_spot = QT_MAC_OFFTHESPOT;
                        else
                            qDebug("Qt: internal: Misunderstood input style '%s'", s.latin1());
                    }
                } else if(arg.left(5) == "-psn_") {
                    passed_psn = arg.mid(6);
                } else {
                    argv[j++] = argv[i];
                }
        }
        priv->argc = j;

        if(qt_is_gui_used && argv[0] && *argv[0] != '/')
            qWarning("Qt: QApplication: Warning argv[0] == '%s' is relative.\n"
                     "In order to dispatch events correctly Mac OS X may "
                     "require applications to be run with the *full* path to the "
                     "executable.", argv[0]);
        //special hack to change working directory (for an app bundle) when running from finder
        if(!passed_psn.isNull() && QDir::currentDirPath() == "/") {
            QCFType<CFURLRef> bundleURL(CFBundleCopyBundleURL(CFBundleGetMainBundle()));
            QString qbundlePath = QCFString(CFURLCopyFileSystemPath(bundleURL,
                                            kCFURLPOSIXPathStyle));
            if(qbundlePath.endsWith(".app"))
                QDir::setCurrent(qbundlePath.section('/', 0, -2));
        }
    }

    QMacMime::initialize();

    qApp->setObjectName(qAppName());
    if(qt_is_gui_used) {
        QColor::initialize();
        QFont::initialize();
        QCursor::initialize();
#if !defined(QMAC_NO_COREGRAPHICS)
        QCoreGraphicsPaintEngine::initialize();
#endif
        QQuickDrawPaintEngine::initialize();
#if defined(QT_ACCESSIBILITY_SUPPORT)
        QAccessible::initialize();
#endif

#if defined(QT_THREAD_SUPPORT)
        qt_mac_port_mutex = new QMutex(true);
#endif
        RegisterAppearanceClient();
        if(QApplication::desktopSettingsAware())
            qt_mac_update_os_settings();

        if(!app_proc_handler) {
            app_proc_handlerUPP = NewEventHandlerUPP(QApplication::globalEventProcessor);
            qt_init_app_proc_handler();
        }

        if(!app_proc_ae_handlerUPP) {
            app_proc_ae_handlerUPP = AEEventHandlerUPP(QApplication::globalAppleEventProcessor);
            AEInstallEventHandler(typeWildCard, typeWildCard, app_proc_ae_handlerUPP, (long)qApp, true);
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
        app_proc_handlerUPP = 0;
    }
    if(app_proc_ae_handlerUPP) {
        AERemoveEventHandler(typeWildCard, typeWildCard, app_proc_ae_handlerUPP, true);
        DisposeAEEventHandlerUPP(app_proc_ae_handlerUPP);
        app_proc_ae_handlerUPP = NULL;
    }
    QPixmapCache::clear();
    if(qt_is_gui_used) {
#if defined(QT_ACCESSIBILITY_SUPPORT)
        QAccessible::cleanup();
#endif
        QQuickDrawPaintEngine::cleanup();
        QFont::cleanup();
        QColor::cleanup();
        if(qt_mac_safe_pdev) {
            delete qt_mac_safe_pdev;
            qt_mac_safe_pdev = 0;
        }
    }

    qDeleteAll(qt_mac_tsm_hash.constBegin(), qt_mac_tsm_hash.constEnd());
    qt_mac_tsm_hash.clear();
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/
void qt_updated_rootinfo()
{
}

bool qt_wstate_iconified(WId)
{
    return false;
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
void QApplication::setOverrideCursor(const QCursor &cursor, bool replace)
{
    if (replace && !qApp->d->cursor_list.isEmpty())
        qApp->d->cursor_list.replace(0, cursor);
    else
        qApp->d->cursor_list.prepend(cursor);

    if(qApp && qApp->activeWindow()) {
        Point mouse_pos;
        QPoint qmp(QCursor::pos());
        mouse_pos.h = qmp.x();
        mouse_pos.v = qmp.y();
        qt_mac_set_cursor(&qApp->d->cursor_list.first(), &mouse_pos);
    }
}

void QApplication::restoreOverrideCursor()
{
    if (qApp->d->cursor_list.isEmpty())
        return;
    qApp->d->cursor_list.removeFirst();

    if(qApp && qApp->activeWindow()) {
        Point mouse_pos;
        QPoint qmp(QCursor::pos());
        mouse_pos.h = qmp.x();
        mouse_pos.v = qmp.y();

        const QCursor def(Qt::ArrowCursor);
        qt_mac_set_cursor(qApp->d->cursor_list.isEmpty() ? &def : &qApp->d->cursor_list.first(), &mouse_pos);
    }
}
#endif

QWidget *qt_recursive_match(QWidget *widget, int x, int y)
{
    HIViewRef child;
    const HIPoint pt = CGPointMake(x, y);
    HIViewGetSubviewHit((HIViewRef)widget->winId(), &pt, true, &child);
    return child ? QWidget::find((WId)child) : widget;
}

QWidget *QApplication::widgetAt_sys(int x, int y)
{
    //find the tld
    QWidget *widget;
    qt_mac_window_at(x, y, &widget);
    if(!widget)
        return 0;

    //find the child
    QPoint p = widget->mapFromGlobal(QPoint(x, y));
    widget = qt_recursive_match(widget, p.x(), p.y());
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
static int get_modifiers(int key, bool from_mouse=false)
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

/*!
    \internal
*/
bool QApplication::do_mouse_down(Point *pt, bool *mouse_down_unhandled)
{
    QWidget *widget;
    int popup_close_count = 0;
    short windowPart = qt_mac_window_at(pt->h, pt->v, &widget);
    if(inPopupMode() && widget != activePopupWidget()) {
        while(inPopupMode()) {
            activePopupWidget()->close();
            popup_close_count++;
            if(windowPart == inContent)
                break;
        }
    }

    if(mouse_down_unhandled)
        (*mouse_down_unhandled) = false;
    if(windowPart == inMenuBar) {
        QMacBlockingFunction block;
        MenuSelect(*pt); //allow menu tracking
        return false;
    } else if(!widget) {
        if(mouse_down_unhandled)
            (*mouse_down_unhandled) = true;
        return false;
    } else if(windowPart != inGoAway && windowPart != inCollapseBox) {
        bool set_active = true;
        if(windowPart == inZoomIn || windowPart == inZoomOut || windowPart == inDrag || windowPart == inGrow)
            set_active = !(GetCurrentKeyModifiers() & cmdKey);
        if(set_active) {
            widget->raise();
            if(widget->isTopLevel() && !widget->isDesktop() && !widget->isPopup() &&
               !qt_mac_is_macsheet(widget) && (widget->isModal() || !::qt_cast<QDockWindow *>(widget)))
                widget->setActiveWindow();
        }
    }
    if(windowPart == inContent)
        return !popup_close_count; //just return and let the event loop process

    WindowPtr window = qt_mac_window_for((HIViewRef)widget->winId());
    if(windowPart == inGoAway || windowPart == inCollapseBox ||
       windowPart == inZoomIn || windowPart == inZoomOut) {
        QMacBlockingFunction block;
        if(!TrackBox(window, *pt, windowPart))
            return false;
    }

    switch(windowPart) {
    case inStructure:
    case inDesk:
        break;
    case inGoAway:
        widget->d->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
        break;
    case inToolbarButton: { //hide toolbars thing
        int h = 0;
        QObjectList chldrn = widget->children();
#ifndef QT_NO_MAINWINDOW
        QMainWindow *mw = ::qt_cast<QMainWindow *>(widget);
#endif
        for(int i = 0; i < chldrn.size(); i++) {
            QObject *obj = chldrn.at(i);
            QDockArea *area = ::qt_cast<QDockArea *>(obj);
            if (area) {
#ifndef QT_NO_MAINWINDOW
                if(mw && mw->topDock() != area)
                    continue;
#endif
                if(area->width() < area->height()) //only do horizontal orientations
                    continue;
                int oh = area->sizeHint().height();
                if(oh < 0)
                    oh = 0;
                if(area->isVisible())
                    area->hide();
                else
                    area->show();
                sendPostedEvents();
                int nh = area->sizeHint().height();
                if(nh < 0)
                    nh = 0;
                if(oh != nh)
                    h += (oh - nh);
            }
        }
        if(h)
            widget->resize(widget->width(), widget->height() - h);
        break; }
    case inProxyIcon: {
        QIconDragEvent e;
        QApplication::sendSpontaneousEvent(widget, &e);
        if(e.isAccepted())
            break;
        //fall through if not accepted
        }
    case inDrag: {
        {
            QMacBlockingFunction block;
            DragWindow(window, *pt, 0);
        }
        QPoint np, op(widget->data->crect.x(), widget->data->crect.y());
        {
            QMacSavedPortInfo savedInfo(widget);
            Point p = { 0, 0 };
            LocalToGlobal(&p);
            np = QPoint(p.h, p.v);
        }
        if(np != op) {
            widget->data->crect = QRect(np, widget->data->crect.size());
            QMoveEvent qme(np, op);
        }
        break; }
    case inGrow: {
        Rect limits;
        SetRect(&limits, -2, 0, 0, 0);
        if(QWExtra *extra = ((QETWidget*)widget)->extraData())
            SetRect(&limits, extra->minw, extra->minh,
                    extra->maxw < 32767 ? extra->maxw : 32767,
                    extra->maxh < 32767 ? extra->maxh : 32767);
        int growWindowSize;
        {
            QMacBlockingFunction block;
            growWindowSize = GrowWindow(window, *pt, limits.left == -2 ? 0 : &limits);
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
        widget->setWindowState(widget->windowState() | WindowMinimized);
        //we send a hide to be like X11/Windows
        QEvent e(QEvent::Hide);
        QApplication::sendSpontaneousEvent(widget, &e);
        break; }
    case inZoomIn:
        widget->setWindowState(widget->windowState() & ~WindowMaximized);
        break;
    case inZoomOut:
        widget->setWindowState(widget->windowState() | WindowMaximized);
        break;
    default:
        qDebug("Qt: internal: Unhandled case in mouse_down.. %d", windowPart);
        break;
    }
    return false;
}


bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal(QWidget *widget)
{
#ifdef DEBUG_MODAL_EVENTS
    qDebug("Entering modal state with %s::%s::%p (%d)", widget->className(), widget->objectName().local8Bit(),
           widget, qt_modal_stack ? (int)qt_modal_stack->count() : -1);
#endif
    if(!qt_modal_stack) {                        // create modal stack
        qt_modal_stack = new QWidgetList;
    }
    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }

    qt_dispatchEnterLeave(0, qt_mouseover);
    qt_mouseover = 0;

    qt_modal_stack->insert(0, widget);
    if(!app_do_modal)
        qt_event_request_menubarupdate();
    app_do_modal = true;
}


void qt_leave_modal(QWidget *widget)
{
    if(qt_modal_stack && qt_modal_stack->removeAll(widget)) {
#ifdef DEBUG_MODAL_EVENTS
        qDebug("Leaving modal state with %s::%s::%p (%d)", widget->className(), widget->objectName().local8Bit(),
               widget, qt_modal_stack->count());
#endif
        if(qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
            QPoint p(QCursor::pos());
            app_do_modal = false;
            QWidget* w = QApplication::widgetAt(p.x(), p.y());
            qt_dispatchEnterLeave(w, qt_mouseover); // send synthetic enter event
            qt_mouseover = w? w->winId() : 0;
        }
    }
#ifdef DEBUG_MODAL_EVENTS
    else qDebug("Failure to remove %s::%s::%p -- %p", widget->className(), widget->objectName().local8Bit(), widget, qt_modal_stack);
#endif
    app_do_modal = (qt_modal_stack != 0);
    if(!app_do_modal)
        qt_event_request_menubarupdate();

    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }
}

QWidget *qt_tryModalHelperMac(QWidget * top) {
    if(top && qt_mac_is_macsheet(top) && !IsWindowVisible(qt_mac_window_for((HIViewRef)top->winId()))) {
        if(WindowPtr wp = GetFrontWindowOfClass(kSheetWindowClass, true)) {
            if(QWidget *sheet = qt_mac_find_window(wp))
                top = sheet;
        }
    }
    return top;
}

static bool qt_try_modal(QWidget *widget, EventRef event)
{
    QWidget * top = 0;

    if (qt_tryModalHelper(widget, &top))
        return true;

    bool block_event  = false;
    bool paint_event = false;

    UInt32 ekind = GetEventKind(event), eclass=GetEventClass(event);
    switch(eclass) {
    case kEventClassMouse:
        block_event = true;
        break;
    case kEventClassKeyboard:
        block_event = true;
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
                qDebug("%s:%d -- modal (false)", __FILE__, __LINE__);
#endif
                return false;
            }
        }
#ifdef DEBUG_MODAL_EVENTS
        qDebug("%s:%d -- special mac-sheet (true)", __FILE__, __LINE__);
#endif
        return true;
    }
#endif

#ifdef DEBUG_MODAL_EVENTS
    qDebug("%s:%d -- final decision! (%s)", __FILE__, __LINE__, block_event ? "false" : "true");
#endif
    return !block_event;
}

bool qt_mac_send_event(QEventLoop::ProcessEventsFlags flags, EventRef event, WindowPtr pt)
{
    if(flags != QEventLoop::AllEvents) {
        UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
        if(flags & QEventLoop::ExcludeUserInput) {
            switch(eclass) {
            case kEventClassQt:
                if(ekind == kEventQtRequestContext)
                    return false;
                break;
            case kEventClassMouse:
            case kEventClassKeyboard:
                return false;
            }
        }
        if(flags & QEventLoop::ExcludeSocketNotifiers) {
            switch(eclass) {
            case kEventClassQt:
                if(ekind == kEventQtRequestSelect || ekind == kEventQtRequestSocketAct)
                    return false;
                break;
            }
        }
    }
    if(pt && SendEventToWindow(event, pt) != eventNotHandledErr)
        return true;
    return !SendEventToEventTarget(event, GetEventDispatcherTarget());
}

OSStatus
QApplication::globalEventProcessor(EventHandlerCallRef er, EventRef event, void *data)
{
    QApplication *app = (QApplication *)data;
    if(app->macEventFilter(er, event)) //someone else ate it
        return noErr;
    QPointer<QWidget> widget;

    /*Only certain event don't remove the context timer (the left hold context menu),
      otherwise we just turn it off. Similarly we assume all events are handled and in
      the code below we set it to false when we know we didn't handle it, this will let
      rogue events through (shouldn't really happen, but better safe than sorry) */
    bool remove_context_timer = true, handled_event=true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass)
    {
    case kEventClassQt:
        remove_context_timer = false;
        if(ekind == kEventQtRequestShowSheet) {
            request_showsheet_pending = 0;
            QWidget *widget = 0;
            GetEventParameter(event, kEventParamQWidget, typeQWidget, 0,
                              sizeof(widget), 0, &widget);
            if(widget) {
                WindowPtr window = qt_mac_window_for((HIViewRef)widget->winId());
                bool just_show = false;
                if(!qt_mac_is_macsheet(widget)
                   || ShowSheetWindow(window, qt_mac_window_for((HIViewRef)widget->parentWidget()->winId())) != noErr) {
                    qWarning("Qt: QWidget: Unable to show as sheet %s::%s", widget->className(), widget->objectName().local8Bit());
                    just_show = true;
                }
                if(just_show) //at least the window will be visible, but the sheet flag doesn't work sadly (probalby too many sheets)
                    ShowHide(window, true);
            }
        } else if(ekind == kEventQtRequestWakeup) {
            request_wakeup_pending = 0;             //do nothing else, we just woke up!
        } else if(ekind == kEventQtRequestMenubarUpdate) {
            request_menubarupdate_pending = 0;
            QMenuBar::macUpdateMenuBar();
        } else if(ekind == kEventQtRequestSelect) {
            request_select_pending = 0;
            QGuiEventLoop *l = 0;
            GetEventParameter(event, kEventParamQGuiEventLoop, typeQGuiEventLoop, 0, sizeof(l), 0, &l);
            timeval tm;
            memset(&tm, '\0', sizeof(tm));
            l->d->eventloopSelect(QEventLoop::AllEvents, &tm);
        } else if(ekind == kEventQtRequestSocketAct) {
            request_sockact_pending = 0;
            QGuiEventLoop *l = 0;
            GetEventParameter(event, kEventParamQGuiEventLoop, typeQGuiEventLoop, 0, sizeof(l), 0, &l);
            l->activateSocketNotifiers();
        } else if(ekind == kEventQtRequestActivate) {
            request_activate_pending.event = 0;
            if(request_activate_pending.widget) {
                QWidget *w = request_activate_pending.widget;
                request_activate_pending.widget = 0;
                w->setActiveWindow();
            }
        } else if(ekind == kEventQtRequestContext) {
            bool send = false;
            if((send = (event == request_context_hold_pending)))
                request_context_hold_pending = 0;
            else if((send = (event == request_context_pending)))
                request_context_pending = 0;
            if(send) {
                //figure out which widget to send it to
                QPoint where = QCursor::pos();
                QWidget *widget = 0;
                GetEventParameter(event, kEventParamQWidget, typeQWidget, 0,
                                  sizeof(widget), 0, &widget);
                if(!widget) {
                    if(qt_button_down)
                        widget = qt_button_down;
                    else
                        widget = QApplication::widgetAt(where.x(), where.y());
                }
                if(widget) {
                    QPoint plocal(widget->mapFromGlobal(where));
                    QContextMenuEvent qme(QContextMenuEvent::Mouse, plocal, where, 0);
                    QApplication::sendEvent(widget, &qme);
                    if(qme.isAccepted()) { //once this happens the events before are pitched
#if 0
                        if(qt_button_down && mouse_button_state) {
                            QMouseEvent qme(QEvent::MouseButtonRelease, plocal, where,
                                             mouse_button_state, mouse_button_state);
                            QApplication::sendSpontaneousEvent(qt_button_down, &qme);
                        }
                        mouse_button_state = 0;
#endif
                        qt_button_down = 0;
                        qt_mac_dblclick.active = false;
#ifdef DEBUG_MOUSE_MAPS
                        qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__,
                               mouse_button_state);
#endif
                    }
                } else {
                    handled_event = false;
                }
            }
        } else if(ekind == kEventQtRequestTimer) {
            MacTimerInfo *t;
            GetEventParameter(event, kEventParamMacTimer, typeMacTimerInfo, 0, sizeof(t), 0, &t);
            if(t && t->pending) {
                t->pending = false;
                QTimerEvent e(t->id);
                QApplication::sendEvent(t->obj, &e);        // send event
            }
        } else {
            handled_event = false;
        }
        break;
    case kEventClassMouse:
    {
       Point where;
        GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 0,
                          sizeof(where), 0, &where);
        if(ekind == kEventMouseMoved && qt_mac_app_fullscreen &&
            QApplication::desktop()->screenNumber(QPoint(where.h, where.v)) ==
            QApplication::desktop()->primaryScreen()) {
            if(where.v <= 0)
                ShowMenuBar();
            else if(qt_mac_window_at(where.h, where.v, 0) != inMenuBar)
                HideMenuBar();
        }

#if defined(DEBUG_MOUSE_MAPS)
        const char *edesc = 0;
        switch(ekind) {
        case kEventMouseDown: edesc = "MouseButtonPress"; break;
        case kEventMouseUp: edesc = "MouseButtonRelease"; break;
        case kEventMouseDragged: case kEventMouseMoved: edesc = "MouseMove"; break;
        case kEventMouseWheelMoved: edesc = "MouseWheelMove"; break;
        }
        if(ekind == kEventMouseDown || ekind == kEventMouseUp)
            qDebug("Handling mouse: %s", edesc);
#endif
        QEvent::Type etype = QEvent::None;
        UInt32 modifiers;
        GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 0,
                          sizeof(modifiers), 0, &modifiers);
        int keys = get_modifiers(modifiers, true);
        int button=QEvent::NoButton, state=0, wheel_delta=0, after_state=mouse_button_state;
        if(ekind == kEventMouseDown || ekind == kEventMouseUp) {
            EventMouseButton mb;
            GetEventParameter(event, kEventParamMouseButton, typeMouseButton, 0,
                              sizeof(mb), 0, &mb);

            if(mb == kEventMouseButtonPrimary)
                button = QMouseEvent::LeftButton;
            else if(mb == kEventMouseButtonSecondary)
                button = QMouseEvent::RightButton;
            else
                button = QMouseEvent::MidButton;
        }

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
                    GetEventParameter(event, kEventParamClickCount, typeUInt32, 0,
                                      sizeof(count), 0, &count);
                    if(!(count % 2) && qt_mac_dblclick.last_modifiers == keys &&
                       qt_mac_dblclick.last_button == button)
                        etype = QEvent::MouseButtonDblClick;
                }
                if(etype == QEvent::MouseButtonDblClick)
                    qt_mac_dblclick.active = false;
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
            GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, 0,
                              sizeof(mdelt), 0, &mdelt);
            wheel_delta = mdelt * 120;
            state = after_state;
            break;
        }
        }
        //figure out which widget to send it to
        if(app->inPopupMode()) {
            QWidget *clt;
            qt_mac_window_at(where.h, where.v, &clt);
            if(clt && clt->isPopup())
                widget = clt;
            if(!widget)
                widget = activePopupWidget();
            QMacSavedPortInfo savedInfo(widget);
            Point gp = where;
            GlobalToLocal(&gp); //now map it to the window
            widget = qt_recursive_match(widget, gp.h, gp.v);
        } else {
            if(ekind != kEventMouseDown && qt_button_down) {
                widget = qt_button_down;
            } else if(mac_mouse_grabber) {
                widget = mac_mouse_grabber;
            } else {
                widget = QApplication::widgetAt(where.h, where.v);
                if(ekind == kEventMouseUp) {
                    short part = qt_mac_window_at(where.h, where.v);
                    if(part == inDrag) {
                        UInt32 count;
                        GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL,
                                          sizeof(count), NULL, &count);
                        if(count == 2 && qt_mac_collapse_on_dblclick) {
                            mouse_button_state = 0;
#ifdef DEBUG_MOUSE_MAPS
                            qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__, mouse_button_state);
#endif

                            widget->setWindowState(widget->windowState() | WindowMinimized);
                            //we send a hide to be like X11/Windows
                            QEvent e(QEvent::Hide);
                            QApplication::sendSpontaneousEvent(widget, &e);
                            break;
                        }
                    }
                }
            }
        }
        if(!unhandled_dialogs.contains(FrontWindow())) { //set the cursor up
            QCursor cursor(Qt::ArrowCursor);
            QWidget *cursor_widget = widget;
            if(cursor_widget && cursor_widget == qt_button_down && ekind == kEventMouseUp)
                cursor_widget = QApplication::widgetAt(where.h, where.v);
            if(cursor_widget) { //only over the app, do we set a cursor..
                if(!qApp->d->cursor_list.isEmpty()) {
                    cursor = qApp->d->cursor_list.first();
                } else {
                    for(; cursor_widget; cursor_widget = cursor_widget->parentWidget()) {
                        QWExtra *extra = ((QETWidget*)cursor_widget)->extraData();
                        if(extra && extra->curs) {
                            cursor = *extra->curs;
                            break;
                        }
                    }
                }
            }
            qt_mac_set_cursor(&cursor, &where);
        }

        //This mouse button state stuff looks like this on purpose
        //although it looks hacky it is VERY intentional..
        if(widget && app_do_modal && !qt_try_modal(widget, event)) {
            mouse_button_state = after_state;
            if(ekind == kEventMouseDown && qt_mac_is_macsheet(activeModalWidget())) {
                activeModalWidget()->parentWidget()->setActiveWindow(); //sheets have a parent
                if(!app->do_mouse_down(&where, 0))
                    mouse_button_state = 0;
            }
#ifdef DEBUG_MOUSE_MAPS
            qDebug("%s:%d Mouse_button_state = %d", __FILE__, __LINE__, mouse_button_state);
#endif
            break;
        }

#if defined(QT_TABLET_SUPPORT)
        UInt32 tabletEventType;
        GetEventParameter(event, kEventParamTabletEventType, typeUInt32, 0,
                          sizeof(tabletEventType), 0, &tabletEventType);
        if (tabletEventType == kEventTabletPoint) {
            TabletPointRec tabletPointRec;
            GetEventParameter(event, kEventParamTabletPointRec, typeTabletPointRec, 0,
                              sizeof(tabletPointRec), 0, &tabletPointRec);
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
                QPair<int,int> uId((int)tabletPointRec.deviceID, (int)tabletPointRec.vendor1);
                QPoint p(where.h, where.v);
                QPoint plocal(widget->mapFromGlobal(p));
                QTabletEvent e(t, plocal, p, dev, pressure, tiltX, tiltY, uId);
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
                    handled_event = false;
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
                if(tlw->isTopLevel() && !tlw->isDesktop() && !tlw->isPopup() && !qt_mac_is_macsheet(tlw) &&
                   (tlw->isModal() || !::qt_cast<QDockWindow *>(tlw)))
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
                       widget ? widget->className() : "none", widget ? widget->objectName().local8Bit() : "",
                       qt_mouseover ? qt_mouseover->className() : "none",
                       qt_mouseover ? qt_mouseover->objectName().local8Bit() : "");
#endif
                qt_dispatchEnterLeave(widget, qt_mouseover);
                qt_mouseover = widget;
            }
            break;
        }
        case kEventMouseDown:
            if(button == QMouseEvent::LeftButton && !mac_context_timer && qt_mac_press_and_hold_context) {
                remove_context_timer = false;
                if(!mac_context_timerUPP)
                    mac_context_timerUPP = NewEventLoopTimerUPP(qt_context_timer_callbk);
                InstallEventLoopTimer(GetMainEventLoop(), 2, 0, mac_context_timerUPP,
                                      widget, &mac_context_timer);
            }
            qt_button_down = widget;
            break;
        case kEventMouseUp:
            qt_button_down = 0;
            break;
        }

        if(widget) {
            QPoint p(where.h, where.v);
            QPoint plocal(widget->mapFromGlobal(p));
            if(etype == QEvent::MouseButtonPress) {
                qt_mac_dblclick.active = true;
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
                        NewSpeechChannel(0, &ch);
                        SpeakText(ch, s.latin1(), s.length());
                    }
                }
#endif
                int macButton = button;
                static bool macButtonModified = false;
                if(ekind == kEventMouseDown &&
                   button == QMouseEvent::LeftButton &&
                   (modifiers & controlKey)) {
                    macButton = QMouseEvent::RightButton;
                    macButtonModified = true;
                }
                if(ekind == kEventMouseUp && macButtonModified) {
                    macButtonModified = false;
                    macButton = QMouseEvent::RightButton;
                }
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
                   plocal.x(), plocal.y(), event_desc, (QWidget*)widget, widget ? widget->objectName().local8Bit() : "*Unknown*",
                   widget ? widget->className() : "*Unknown*", button, state|keys, wheel_delta);
#endif
        } else {
            handled_event = false;
        }
        break;
    }
    case kEventClassTextInput:
        handled_event = false;
        if(!(widget=focus_widget)) {
            handled_event = false;
        } else if(ekind == kEventTextInputOffsetToPos) {
            if(qt_mac_input_spot != QT_MAC_ONTHESPOT) {
                handled_event = false;
                break;
            }

            if(qt_mac_get_document_id(widget)) {
                QRect mr(widget->microFocusHint());
                QPoint mp(widget->mapToGlobal(QPoint(mr.topLeft())));
                Point pt;
                pt.h = mp.x();
                pt.v = mp.y() + mr.height();
                SetEventParameter(event, kEventParamTextInputReplyPoint, typeQDPoint,
                                  sizeof(pt), &pt);
                handled_event = true;
            }
        } else if(ekind == kEventTextInputUpdateActiveInputArea) {
            if(qt_mac_input_spot != QT_MAC_ONTHESPOT) {
                handled_event = false;
                break;
            }

            long refcon;
            GetEventParameter(event, kEventParamTextInputSendRefCon, typeLongInteger, 0,
                              sizeof(refcon), 0, &refcon);
            if(QTSMDocumentWrapper *doc = (QTSMDocumentWrapper*)refcon) {
                UInt32 unilen;
                GetEventParameter(event, kEventParamTextInputSendText, typeUnicodeText,
                                  0, 0, &unilen, 0);
                UniChar *unicode = (UniChar*)NewPtr(unilen);
                GetEventParameter(event, kEventParamTextInputSendText, typeUnicodeText,
                                  0, unilen, 0, unicode);
                QString text((QChar*)unicode, unilen / sizeof(UniChar));
                DisposePtr((char*)unicode);
                if(doc->inputWidget()) {
                    long fixed_length;
                    GetEventParameter(event, kEventParamTextInputSendFixLen, typeLongInteger, 0,
                                      sizeof(fixed_length), 0, &fixed_length);
                    if(fixed_length == -1 || fixed_length == (long)unilen) {
                        QIMEvent imend(QEvent::IMEnd, text, text.length());
                        QApplication::sendSpontaneousEvent(doc->inputWidget(), &imend);
                        if(imend.isAccepted()) {
                            doc->setInputWidget(0);
                            handled_event = true;
                        }
                    } else {
                        if(fixed_length > 0) {
                            QIMEvent imend(QEvent::IMEnd, text.left(fixed_length / sizeof(UniChar)),
                                           fixed_length / sizeof(UniChar));
                            QApplication::sendSpontaneousEvent(doc->inputWidget(), &imend);
                            if(imend.isAccepted()) {
                                handled_event = true;
                                QIMEvent imstart(QEvent::IMStart, text.mid(fixed_length / sizeof(UniChar)),
                                                 (fixed_length - text.length()) / sizeof(UniChar));
                                QApplication::sendSpontaneousEvent(doc->inputWidget(), &imstart);
                                if(imstart.isAccepted())
                                    handled_event = true;
                            }
                        } else {
                            QIMEvent imcompose(QEvent::IMCompose, text, text.length(), 0);
                            QApplication::sendSpontaneousEvent(doc->inputWidget(), &imcompose);
                            if(imcompose.isAccepted())
                                handled_event = true;
                        }
                    }
                } else {
                    QIMEvent imstart(QEvent::IMStart, text, text.length());
                    QApplication::sendSpontaneousEvent(widget, &imstart);
                    if(imstart.isAccepted()) {
                        handled_event = true;
                        doc->setInputWidget(widget);
                        QIMEvent imcompose(QEvent::IMCompose, text, text.length(), 0);
                        QApplication::sendSpontaneousEvent(doc->inputWidget(), &imcompose);
                    }
                }
            }
        } else if(ekind == kEventTextInputUnicodeForKeyEvent) {
            EventRef key_ev;
            GetEventParameter(event, kEventParamTextInputSendKeyboardEvent, typeEventRef, 0,
                              sizeof(key_ev), 0, &key_ev);
            QString text;
            UInt32 unilen;
            if(GetEventParameter(key_ev, kEventParamKeyUnicodes, typeUnicodeText, 0, 0, &unilen, 0) == noErr) {
                UniChar *unicode = (UniChar*)NewPtr(unilen);
                GetEventParameter(key_ev, kEventParamKeyUnicodes, typeUnicodeText, 0, unilen, 0, unicode);
                text = QString((QChar*)unicode, unilen / sizeof(UniChar));
                DisposePtr((char*)unicode);
            }
            unsigned char chr;
            GetEventParameter(key_ev, kEventParamKeyMacCharCodes, typeChar, 0, sizeof(chr), 0, &chr);
            if(!chr || chr >= 128 || (text.length() > 0 && (text.length() > 1 || text.at(0) != QChar(chr)))) {
                QIMEvent imstart(QEvent::IMStart, QString::null, -1);
                QApplication::sendSpontaneousEvent(widget, &imstart);
                if(imstart.isAccepted()) { //wants the event
                    handled_event = true;
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
        GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 0,
                          sizeof(modif), 0, &modif);

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
                       widget ? widget->className() : "none", widget ? widget->objectName().local8Bit() : "",
                       key, modifiers);
#endif
                QKeyEvent ke(etype, key, modifiers, "", false);
                QApplication::sendSpontaneousEvent(widget,&ke);
            }
            break;
        }

        UInt32 keyc;
        GetEventParameter(event, kEventParamKeyCode, typeUInt32, 0, sizeof(keyc), 0, &keyc);
        static UInt32 state = 0L;
        char chr = KeyTranslate((void *)GetScriptVariable(smCurrentScript, smKCHRCache),
                   (modif & (kEventKeyModifierNumLockMask|shiftKey|rightShiftKey|alphaLock)) | keyc, &state);
        if(!chr) {
            if(CallNextEventHandler(er, event) == noErr)
                handled_event = true;
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
        } else {          //now get the real ascii value
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
            if(GetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, 0, 0, &unilen, 0) == noErr && unilen == 2) {
                UniChar *unicode = (UniChar*)NewPtr(unilen);
                GetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, 0, unilen, 0, unicode);
                mystr = QString((QChar*)unicode, unilen / sizeof(UniChar));
                DisposePtr((char *)unicode);
            } else if(chr) {
                static QTextCodec *c = 0;
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

            bool key_event = true;
            if(etype == QEvent::KeyPress && !mac_keyboard_grabber) {
                /* We offer the accelerator a text representation of chr, this is because the Mac
                   actually flips the keyboard when things like alt are pressed, but that doesn't
                   really mean that accelerators should be mapped to the new key (or things could get
                   quite broken). */
                QString accel_str;
                if(chr) {
                    static QTextCodec *c = 0;
                    if(!c)
                        c = QTextCodec::codecForName("Apple Roman");
                    accel_str = c->toUnicode(&chr, 1);
                }
                QKeyEvent accel_ev(QEvent::AccelOverride, mychar, modifiers,
                                   accel_str, ekind == kEventRawKeyRepeat,
                                   qMax(1, accel_str.length()));
                if(qt_tryAccelEvent(widget, &accel_ev)) {
#ifdef DEBUG_KEY_MAPS
                    qDebug("KeyEvent: %s::%s consumed Accel: %04x %c %s %d",
                           widget ? widget->className() : "none", widget ? widget->objectName().local8Bit() : "",
                           mychar, chr, mystr.latin1(), ekind == kEventRawKeyRepeat);
#endif
                    key_event = false;
                } else {
                    if(accel_ev.isAccepted()) {
#ifdef DEBUG_KEY_MAPS
                        qDebug("KeyEvent: %s::%s overrode Accel: %04x %c %s %d",
                               widget ? widget->className() : "none", widget ? widget->objectName().local8Bit() : "",
                               mychar, chr, mystr.latin1(), ekind == kEventRawKeyRepeat);
#endif
                    }
                }
            }
            if(key_event) {
                //Find out if someone else wants the event, namely
                //is it of use to text services? If so we won't bother
                //with a QKeyEvent.
                if(CallNextEventHandler(er, event) == noErr) {
                    handled_event = true;
                    break;
                }
#ifdef DEBUG_KEY_MAPS
                qDebug("KeyEvent: Sending %s to %s::%s: %04x '%c' (%s) %d%s",
                       etype == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
                       widget ? widget->className() : "none", widget ? widget->objectName().local8Bit() : "",
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
                             qMax(1, mystr.length()));
                QApplication::sendSpontaneousEvent(widget,&ke);
            }
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassWindow: {
        remove_context_timer = false;

        WindowRef wid;
        GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
                          sizeof(WindowRef), 0, &wid);
        widget = qt_mac_find_window(wid);
        if(!widget) {
            if(ekind == kEventWindowShown)
                unhandled_dialogs.insert(wid, 1);
            else if(ekind == kEventWindowHidden)
                unhandled_dialogs.remove(wid);
            handled_event = false;
            break;
        } else if(widget->isDesktop()) {
            handled_event = false;
            break;
        }

        if(ekind == kEventWindowDispose) {
            qt_mac_unicode_cleanup(widget);
        } else if(ekind == kEventWindowExpanded) {
            widget->setWindowState((widget->windowState() & ~WindowMinimized) | WindowActive);
            QShowEvent qse;
            QApplication::sendSpontaneousEvent(widget, &qse);
        } else if(ekind == kEventWindowBoundsChanged) {
            handled_event = false;
            UInt32 flags;
            GetEventParameter(event, kEventParamAttributes, typeUInt32, 0,
                              sizeof(flags), 0, &flags);
            Rect nr;
            GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, 0,
                              sizeof(nr), 0, &nr);
            if((flags & kWindowBoundsChangeOriginChanged)) {
                int ox = widget->data->crect.x(), oy = widget->data->crect.y();
                int nx = nr.left, ny = nr.top;
                if(nx != ox ||  ny != oy) {
                    widget->data->crect.setRect(nx, ny, widget->width(), widget->height());
                    QMoveEvent qme(widget->data->crect.topLeft(), QPoint(ox, oy));
                    QApplication::sendSpontaneousEvent(widget, &qme);
                }
            }
            if((flags & kWindowBoundsChangeSizeChanged)) {
                int nw = nr.right - nr.left, nh = nr.bottom - nr.top;
                if(widget->width() != nw || widget->height() != nh)
                    widget->resize(nw, nh);
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
                if(tlw->isTopLevel() && !tlw->isPopup() && !qt_mac_is_macdrawer(tlw) &&
                   (tlw->isModal() || !tlw->testWFlags(WStyle_Tool))) {
                    bool just_send_event = false;
                    {
                        WindowActivationScope scope;
                        if(GetWindowActivationScope((WindowRef)wid, &scope) == noErr &&
                           scope == kWindowActivationScopeIndependent) {
                            if(GetFrontWindowOfClass(kAllWindowClasses, true) != wid)
                                just_send_event = true;
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
                QMenuBar::macUpdateMenuBar();
            }
        } else if(ekind == kEventWindowDeactivated) {
            if(QTSMDocumentWrapper *doc = qt_mac_get_document_id(widget))
                DeactivateTSMDocument(doc->document());
            if(widget && active_window == widget)
                app->setActiveWindow(0);
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassApplication:
        if(ekind == kEventAppActivated) {
            if(QApplication::desktopSettingsAware())
                qt_mac_update_os_settings();
            app->clipboard()->loadScrap(false);
            if(qt_clipboard) { //manufacture an event so the clipboard can see if it has changed
                QEvent ev(QEvent::Clipboard);
                QApplication::sendSpontaneousEvent(qt_clipboard, &ev);
            }
            if(!app->activeWindow()) {
                WindowPtr wp = ActiveNonFloatingWindow();
                if(wp && !unhandled_dialogs.contains(wp)) {
                    if(QWidget *tmp_w = qt_mac_find_window(wp))
                        app->setActiveWindow(tmp_w);
                }
            }
            QMenuBar::macUpdateMenuBar();
        } else if(ekind == kEventAppDeactivated) {
            while(app->inPopupMode())
                app->activePopupWidget()->close();
            app->clipboard()->saveScrap();
            app->setActiveWindow(0);
        } else {
            handled_event = false;
        }
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
            handled_event = false;
        }
        break;
    case kEventClassAppleEvent:
        handled_event = false;
        if(ekind == kEventAppleEvent) {
            EventRecord erec;
            if(!ConvertEventRefToEventRecord(event, &erec))
                qDebug("Qt: internal: WH0A, unexpected condition reached. %s:%d", __FILE__, __LINE__);
            else if(AEProcessAppleEvent(&erec) != noErr)
                handled_event = false;
        } else {
            handled_event = false;
        }
        break;
    case kEventClassCommand:
        if(ekind == kEventCommandProcess) {
            HICommand cmd;
            GetEventParameter(event, kEventParamDirectObject, typeHICommand,
                              0, sizeof(cmd), 0, &cmd);
            if(cmd.commandID == kHICommandQuit) {
                HiliteMenu(0);
                if (!qt_modal_state()) {
                    QCloseEvent ev;
                    QApplication::sendSpontaneousEvent(app, &ev);
                    if(ev.isAccepted())
                        app->quit();
                } else {
                    QApplication::beep();
                }
            } else if(cmd.commandID == kHICommandAbout) {
                QMessageBox::aboutQt(0);
                HiliteMenu(0);
            }
        } else {
            handled_event = false;
        }
        break;
    }

    // ok we clear all QtRequestContext events from the queue
    if(remove_context_timer) {
        if(mac_context_timer) {
            RemoveEventLoopTimer(mac_context_timer);
            mac_context_timer = 0;
        }
        if(request_context_hold_pending) {
            RemoveEventFromQueue(GetMainEventQueue(), request_context_hold_pending);
            ReleaseEvent(request_context_hold_pending);
            request_context_hold_pending = 0;
        }
    }

#ifdef DEBUG_EVENTS
    qDebug("%shandled event %c%c%c%c %d", handled_event ? "(*) " : "",
           char(eclass >> 24), char((eclass >> 16) & 255), char((eclass >> 8) & 255),
           char(eclass & 255), (int)ekind);
#endif
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}

OSStatus QApplication::globalAppleEventProcessor(const AppleEvent *ae, AppleEvent *, long handlerRefcon)
{
    QApplication *app = (QApplication *)handlerRefcon;
    bool handled_event=false;
    OSType aeID=typeWildCard, aeClass=typeWildCard;
    AEGetAttributePtr(ae, keyEventClassAttr, typeType, 0, &aeClass, sizeof(aeClass), 0);
    AEGetAttributePtr(ae, keyEventIDAttr, typeType, 0, &aeID, sizeof(aeID), 0);
    if(aeClass == kCoreEventClass) {
        switch(aeID) {
        case kAEQuitApplication: {
            if(!qt_modal_state() && IsMenuCommandEnabled(NULL, kHICommandQuit)) {
                QCloseEvent ev;
                QApplication::sendSpontaneousEvent(app, &ev);
                if(ev.isAccepted()) {
                    handled_event = true;
                    app->quit();
                }
            } else {
                QApplication::beep();  // Sorry, you can't quit right now.
            }
            break; }
        case kAEOpenDocuments: {
            AEDescList docs;
            if(AEGetParamDesc(ae, keyDirectObject, typeAEList, &docs) == noErr) {
                long cnt = 0;
                AECountItems(&docs, &cnt);
                UInt8 *str_buffer = NULL;
                for(int i = 0; i < cnt; i++) {
                    FSRef ref;
                    if(AEGetNthPtr(&docs, i+1, typeFSRef, 0, 0, &ref, sizeof(ref), 0) != noErr)
                        continue;
                    if(!str_buffer)
                        str_buffer = (UInt8 *)malloc(1024);
                    FSRefMakePath(&ref, str_buffer, 1024);
                    QFileOpenEvent ev(QString::fromUtf8((const char *)str_buffer));
                    QApplication::sendSpontaneousEvent(app, &ev);
                }
                if(str_buffer)
                    free(str_buffer);
            }
            break; }
        default:
            break;
        }
    }
#ifdef DEBUG_EVENTS
    qDebug("Qt: internal: %shandled Apple event! %c%c%c%c %c%c%c%c", handled_event ? "(*)" : "",
           char(aeID >> 24), char((aeID >> 16) & 255), char((aeID >> 8) & 255),char(aeID & 255),
           char(aeClass >> 24), char((aeClass >> 16) & 255), char((aeClass >> 8) & 255),char(aeClass & 255));
#endif
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}

/*!
  This virtual function is only implemented under Macintosh.

  If you create an application that inherits QApplication and
  reimplement this function, you get direct access to all Carbon Events
  that are received from the MacOS.

  Return true if you want to stop the event from being processed.
  Return false for normal event dispatching.
*/
bool QApplication::macEventFilter(EventHandlerCallRef, EventRef)
{
    return false;
}

/*!
    \internal
*/
void QApplication::openPopup(QWidget *popup)
{
    if(!popupWidgets)                        // create list
        popupWidgets = new QWidgetList;
    popupWidgets->append(popup);                // add to end of list

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

/*!
    \internal
*/
void QApplication::closePopup(QWidget *popup)
{
    if(!popupWidgets)
        return;

    popupWidgets->removeAll(popup);
    if(popup == popupOfPopupButtonFocus) {
        popupButtonFocus = 0;
        popupOfPopupButtonFocus = 0;
    }
    if(popup == qt_button_down) {
        mouse_button_state = 0;
        qt_button_down = 0;
    }
    if(popupWidgets->isEmpty()) {                // this was the last popup
        popupCloseDownMode = true;                // control mouse events
        delete popupWidgets;
        popupWidgets = 0;
        if (active_window) {
            QFocusEvent::setReason(QFocusEvent::Popup);
            if (active_window->focusWidget())
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
    if (QColor::numBitPlanes() < 16 || !animate_ui)
        return false;

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

/*!
    \internal
*/
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
        QTextCodec *codec = QTextCodec::codecForName(defaultcodec.latin1());
        if(codec)
            qApp->setDefaultCodec(codec);
    }

    qt_resolve_symlinks = settings.readBoolEntry("/qt/resolveSymlinks", true);

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
            QApplication::setPalette(pal);

        QFont font(QApplication::font());     // read new font
        str = settings.readEntry("/qt/font");
        if(!str.isNull() && !str.isEmpty()) {
            font.fromString(str);
            if(font != QApplication::font())
                QApplication::setFont(font);
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
                QApplication::setEffectEnabled(Qt::UI_General, false);
            if(effects.contains("general"))
                QApplication::setEffectEnabled(Qt::UI_General, true);
            if(effects.contains("animatemenu"))
                QApplication::setEffectEnabled(Qt::UI_AnimateMenu, true);
            if(effects.contains("fademenu"))
                QApplication::setEffectEnabled(Qt::UI_FadeMenu, true);
            if(effects.contains("animatecombo"))
                QApplication::setEffectEnabled(Qt::UI_AnimateCombo, true);
            if(effects.contains("animatetooltip"))
                QApplication::setEffectEnabled(Qt::UI_AnimateTooltip, true);
            if(effects.contains("fadetooltip"))
                QApplication::setEffectEnabled(Qt::UI_FadeTooltip, true);
            if(effects.contains("animatetoolbox"))
                QApplication::setEffectEnabled(Qt::UI_AnimateToolBox, true);
        } else {
            QApplication::setEffectEnabled(Qt::UI_General, false);
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
    return true;
}

#if defined(Q_OS_MAC) && defined(QT_COMPAT)
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
#endif


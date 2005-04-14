/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include "qdockwidget.h"
#include "qevent.h"
#include "qhash.h"
#include "qmenubar.h"
#include "qmessagebox.h"
#include "qmime.h"
#include "qpixmapcache.h"
#include "qpointer.h"
#include "qsessionmanager.h"
#include "qsettings.h"
#include "qsocketnotifier.h"
#include "qstyle.h"
#include "qstylefactory.h"
#include "qtextcodec.h"
#include "qtoolbar.h"
#include "qvariant.h"
#include "qwidget.h"
#include "qcolormap.h"
#include <private/qmacinputcontext_p.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qcursor_p.h>

#include "private/qapplication_p.h"
#include "private/qcolor_p.h"
#include "private/qwidget_p.h"
#include "qeventdispatcher_mac_p.h"

#ifndef QT_NO_ACCESSIBILITY
#  include "qaccessible.h"
#endif

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif

#if !defined(QT_NO_DEBUG)
#include <qdebug.h>
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
#include "qregexp.h"
#endif

//for qt_mac.h
QPaintDevice *qt_mac_safe_pdev = 0;
QList<QMacWindowChangeEvent*> *QMacWindowChangeEvent::change_events = 0;

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static struct {
    bool use_qt_time_limit;
    int last_modifiers, last_button;
    EventTime last_time;
    bool active;
} qt_mac_dblclick = { false, 0, 0, -2, 0 };
static bool qt_mac_use_qt_scroller_lines = false;

// tablet structure
static QTabletEvent::PointerType currPointerType = QTabletEvent::UnknownPointer;
static QTabletEvent::TabletDevice currTabletDevice = QTabletEvent::NoDevice;
static qint64 tabletUniqueID = 0;
static UInt32 tabletCaps = 0;

static int tablet_button_state = 0;
bool qt_mac_eat_unicode_key = false;
static bool app_do_modal = false;       // modal mode
extern QWidgetList *qt_modal_stack;     // stack of modal widgets
extern bool qt_mac_in_drag;             // from qdnd_mac.cpp
extern bool qt_tab_all_widgets;         // from qapplication.cpp
extern bool qt_app_has_font;
bool qt_mac_app_fullscreen = false;
bool qt_scrollbar_jump_to_pos = false;
static bool qt_mac_collapse_on_dblclick = true;
QPointer<QWidget> qt_button_down;                // widget got last button-down
static QPointer<QWidget> qt_mouseover;
static QHash<WindowRef, int> unhandled_dialogs;        //all unhandled dialogs (ie mac file dialog)
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
extern bool qt_mac_can_clickThrough(const QWidget *); //qwidget_mac.cpp
extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
extern WindowPtr qt_mac_window_for(const QWidget*); //qwidget_mac.cpp
extern QWidget *qt_mac_find_window(WindowPtr); //qwidget_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const QPoint &); //qcursor_mac.cpp
extern bool qt_mac_is_macsheet(const QWidget *); //qwidget_mac.cpp
extern QString qt_mac_from_pascal_string(const Str255); //qglobal.cpp
extern void qt_mac_command_set_enabled(MenuRef, UInt32, bool); //qmenu_mac.cpp

/* Resolution change magic */
static void qt_mac_display_change_callbk(void *, SInt16 msg, void *)
{
    if(msg == kDMNotifyEvent && qApp) {
        if(QDesktopWidget *dw = qApp->desktop()) {
            QResizeEvent *re = new QResizeEvent(dw->size(), dw->size());
            QApplication::postEvent(dw, re);
        }
    }
}

#ifdef DEBUG_PLATFORM_SETTINGS
static void qt_mac_debug_palette(const QPalette &pal, const QPalette &pal2, const QString &where)
{
    const char *const groups[] = {"Active", "Disabled", "Inactive" };
    const char *const roles[] = { "Foreground", "Button", "Light", "Midlight", "Dark", "Mid",
                            "Text", "BrightText", "ButtonText", "Base", "Background", "Shadow",
                            "Highlight", "HighlightedText", "Link", "LinkVisited" };
    if(!where.isNull())
        qDebug("qt-internal: %s", where.toLatin1().constData());
    for(int grp = 0; grp < QPalette::NColorGroups; grp++) {
        for(int role = 0; role < QPalette::NColorRoles; role++) {
            QBrush b = pal.brush((QPalette::ColorGroup)grp, (QPalette::ColorRole)role);
            QPixmap pm = b.texture();
            qDebug("  %s::%s %d::%d::%d [%p]%s", groups[grp], roles[role], b.color().red(),
                   b.color().green(), b.color().blue(), pm.isNull() ? 0 : &pm,
                   pal2.brush((QPalette::ColorGroup)grp, (QPalette::ColorRole)role) != b ? " (*)" : "");
        }
    }

}
#else
#define qt_mac_debug_palette(x, y, z)
#endif

//raise a notification
static NMRecPtr qt_mac_notification = 0;
void qt_mac_send_notification()
{
    if(qt_mac_notification)
        return;

    //only if we are inactive
    ProcessSerialNumber mine, front;
    if(GetCurrentProcess(&mine) == noErr && GetFrontProcess(&front) == noErr) {
        Boolean same;
        if(SameProcess(&mine, &front, &same) == noErr && same)
            return;
    }

    //send it
    qt_mac_notification = (NMRecPtr)malloc(sizeof(NMRec));
    memset(qt_mac_notification, '\0', sizeof(NMRec));
    qt_mac_notification->nmMark = 1; //non-zero magic number
    qt_mac_notification->qType = nmType;
}
void qt_mac_cancel_notification()
{
     if(!qt_mac_notification)
	return;
     NMRemove(qt_mac_notification);
     free(qt_mac_notification);
     qt_mac_notification = 0;
}

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

void qt_mac_set_app_icon(const QPixmap &pixmap)
{
    if(pixmap.isNull()) {
        RestoreApplicationDockTileImage();
    } else {
        QPixmap scaled_pixmap = pixmap.scaled(40, 40);
        CGImageRef ir = (CGImageRef)scaled_pixmap.macCGHandle();
        SetApplicationDockTileImage(ir);
    }
}

Q_GUI_EXPORT void qt_mac_set_press_and_hold_context(bool b) { qt_mac_press_and_hold_context = b; }

Q_GUI_EXPORT void qt_mac_secure_keyboard(bool b)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if(b) {
        SInt32 (*EnableSecureEventInput_ptr)() = EnableSecureEventInput; // workaround for gcc warning
        if (EnableSecureEventInput_ptr)
            (*EnableSecureEventInput_ptr)();
    } else {
        SInt32 (*DisableSecureEventInput_ptr)() = DisableSecureEventInput;
        if (DisableSecureEventInput_ptr)
            (*DisableSecureEventInput_ptr)();
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

void qt_mac_update_os_settings()
{
    if(!qApp)
        return;
    if(!QApplication::startingUp()) {
        static bool needToPolish = true;
        if (needToPolish) {
            QApplication::style()->polish(qApp);
            needToPolish = false;
        }
    }
    //focus mode
    /* First worked as of 10.2.3 */
    QSettings appleSettings(QLatin1String("apple.com"));
    QVariant appleValue = appleSettings.value(QLatin1String("AppleKeyboardUIMode"), 0);
    qt_tab_all_widgets = (appleValue.toInt() & 0x2);
    //paging mode
    /* First worked as of 10.2.3 */
    appleValue = appleSettings.value(QLatin1String("AppleScrollerPagingBehavior"), false);
    qt_scrollbar_jump_to_pos = appleValue.toBool();
    //collapse
    /* First worked as of 10.3.3 */
    appleValue = appleSettings.value(QLatin1String("AppleMiniaturizeOnDoubleClick"), true);
    qt_mac_collapse_on_dblclick = appleValue.toBool();

#ifdef DEBUG_PLATFORM_SETTINGS
    qDebug("qt_mac_update_os_settings *********************************************************************");
#endif
    { //setup the global palette
        QColor qc;
        RGBColor c;
        (void) QApplication::style();  // trigger creation of application style and system palettes
        QPalette pal = *QApplicationPrivate::sys_pal;
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
        QApplicationPrivate::setSystemPalette(pal);
#ifdef DEBUG_PLATFORM_SETTINGS
        qt_mac_debug_palette(pal, QApplication::palette(), "Global Palette");
#endif
    }
    if(!qt_app_has_font) {
        //setup the global font
        Str255 f_name;
        SInt16 f_size;
        Style f_style;
        GetThemeFont(kThemeApplicationFont, smSystemScript, f_name, &f_size, &f_style);
        QFont fnt(qt_mac_from_pascal_string(f_name), f_size,
                  (f_style & ::bold) ? QFont::Bold : QFont::Normal,
                  (bool)(f_style & ::italic));
#ifdef DEBUG_PLATFORM_SETTINGS
        qDebug("qt-internal: Font for Application [%s::%d::%d::%d]",
               fnt.family().toLatin1().constData(), fnt.pointSize(), fnt.bold(), fnt.italic());
#endif
        QApplication::setFont(fnt);
    }
    { //setup the fonts
        struct {
            const char *const qt_class;
            short font_key;
        } mac_widget_fonts[] = {
            { "QPushButton", kThemePushButtonFont },
            { "QListView", kThemeViewsFont },
            { "QListBox", kThemeViewsFont },
            { "QTitleBar", kThemeWindowTitleFont },
            { "QMenuBar", kThemeMenuTitleFont },
            { "QMenu", kThemeMenuItemFont },
            { "QComboMenuItem", kThemeSystemFont },
            { "QHeaderView", kThemeSmallSystemFont },
            { "Q3Header", kThemeSmallSystemFont },
            { "QTipLabel", kThemeSmallSystemFont },
            { "QLabel", kThemeSystemFont },
            { "QToolButton", kThemeSmallSystemFont },
            { "QMenuItem", kThemeMenuItemCmdKeyFont },  // It doesn't exist, but its unique.
            { "QComboLineEdit", kThemeViewsFont },  // It doesn't exist, but its unique.
            { 0, 0 } };
        Str255 f_name;
        SInt16 f_size;
        Style f_style;
        for(int i = 0; mac_widget_fonts[i].qt_class; i++) {
            GetThemeFont(mac_widget_fonts[i].font_key, smSystemScript, f_name, &f_size, &f_style);
            QFont fnt(qt_mac_from_pascal_string(f_name), f_size, (f_style & ::bold) ? QFont::Bold : QFont::Normal,
                      (bool)(f_style & ::italic));
            bool set_font = true;
            extern QHash<QByteArray, QFont> *qt_app_fonts_hash(); // qapplication.cpp
            QHash<QByteArray, QFont> *hash = qt_app_fonts_hash();
            if(!hash->isEmpty()) {
                QHash<QByteArray, QFont>::const_iterator it
                                        = hash->find(mac_widget_fonts[i].qt_class);
                if (it != hash->constEnd())
                    set_font = (fnt != *it);
            }
            if(set_font) {
                QApplication::setFont(fnt, mac_widget_fonts[i].qt_class);
#ifdef DEBUG_PLATFORM_SETTINGS
                qDebug("qt-internal: Font for %s [%s::%d::%d::%d]", mac_widget_fonts[i].qt_class,
                       fnt.family().toLatin1().constData(), fnt.pointSize(), fnt.bold(), fnt.italic());
#endif
            }
        }
    }
    { //setup the palette
        struct {
            const char *const qt_class;
            ThemeBrush active, inactive;
        } mac_widget_colors[] = {
            { "QToolButton", kThemeTextColorBevelButtonActive, kThemeTextColorBevelButtonInactive },
            { "QAbstractButton", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive },
            { "QHeaderView", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive },
            { "Q3Header", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive },
            { "QComboBox", kThemeTextColorPopupButtonActive, kThemeTextColorPopupButtonInactive },
            { "QListView", kThemeTextColorListView, kThemeTextColorDialogInactive },
            { "QListBox", kThemeTextColorListView, kThemeTextColorDialogInactive },
            { "QMessageBoxLabel", kThemeTextColorAlertActive, kThemeTextColorAlertInactive },
            { "QTabBar", kThemeTextColorTabFrontActive, kThemeTextColorTabFrontInactive },
            { "QLabel", kThemeTextColorPlacardActive, kThemeTextColorPlacardInactive },
            { "QGroupBox", kThemeTextColorPlacardActive, kThemeTextColorPlacardInactive },
            { "QMenu", kThemeTextColorPopupLabelActive, kThemeTextColorPopupLabelInactive },
            { 0, 0, 0 } };
        QColor qc;
        RGBColor c;
        for(int i = 0; mac_widget_colors[i].qt_class; i++) {
            QPalette pal;
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
            if(!strcmp(mac_widget_colors[i].qt_class, "QMenu")) {
                GetThemeTextColor(kThemeTextColorMenuItemActive, 32, true, &c);
                pal.setBrush(QPalette::ButtonText, QColor(c.red / 256, c.green / 256, c.blue / 256));
                GetThemeTextColor(kThemeTextColorMenuItemSelected, 32, true, &c);
                pal.setBrush(QPalette::HighlightedText, QColor(c.red / 256, c.green / 256, c.blue / 256));
                GetThemeTextColor(kThemeTextColorMenuItemDisabled, 32, true, &c);
                pal.setBrush(QPalette::Disabled, QPalette::Text,
                             QColor(c.red / 256, c.green / 256, c.blue / 256));
            } else if(!strcmp(mac_widget_colors[i].qt_class, "QAbstractButton")
                      || !strcmp(mac_widget_colors[i].qt_class, "QHeaderView")
                      || !strcmp(mac_widget_colors[i].qt_class, "Q3Header")) { //special
                pal.setColor(QPalette::Disabled, QPalette::ButtonText,
                             pal.color(QPalette::Disabled, QPalette::Text));
                pal.setColor(QPalette::Inactive, QPalette::ButtonText,
                             pal.color(QPalette::Inactive, QPalette::Text));
                pal.setColor(QPalette::Active, QPalette::ButtonText,
                             pal.color(QPalette::Active, QPalette::Text));
            }
            bool set_palette = true;
            extern QHash<QByteArray, QPalette> *qt_app_palettes_hash(); //qapplication.cpp
            QHash<QByteArray, QPalette> *phash = qt_app_palettes_hash();
            if(!phash->isEmpty()) {
                QHash<QByteArray, QPalette>::const_iterator it
                                    = phash->find(mac_widget_colors[i].qt_class);
                if (it != phash->constEnd())
                    set_palette = (pal != *it);
            }
            if(set_palette) {
                QApplication::setPalette(pal, mac_widget_colors[i].qt_class);
#ifdef DEBUG_PLATFORM_SETTINGS
                qt_mac_debug_palette(pal, QApplication::palette(), QString("Palette for ") + mac_widget_colors[i].qt_class);
#endif
            }
        }
    }
#ifdef DEBUG_PLATFORM_SETTINGS
    qDebug("qt_mac_update_os_settings END !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#endif
}

static void qt_mac_event_release(EventRef &event)
{
    ReleaseEvent(event);
    event = 0;
}
static void qt_mac_event_release(QWidget *w, EventRef &event)
{
    if (event) {
        QWidget *widget = 0;
        if(GetEventParameter(event, kEventParamQWidget, typeQWidget, 0, sizeof(widget), 0, &widget) == noErr
           && w == widget) {
            if (IsEventInQueue(GetMainEventQueue(), event))
                RemoveEventFromQueue(GetMainEventQueue(), event);
            qt_mac_event_release(event);
        }
    }
}

static bool qt_mac_event_remove(EventRef &event)
{
    if (event) {
        if (IsEventInQueue(GetMainEventQueue(), event))
            RemoveEventFromQueue(GetMainEventQueue(), event);
        qt_mac_event_release(event);
        return true;
    }
    return false;
}

/* socket notifiers */
static EventRef request_select_pending = 0;
void qt_event_request_select(QEventDispatcherMac *loop) {
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
                      kEventParamQEventDispatcherMac, typeQEventDispatcherMac, sizeof(loop), &loop);
    PostEventToQueue(GetMainEventQueue(), request_select_pending, kEventPriorityStandard);
}
static EventRef request_sockact_pending = 0;
void qt_event_request_sockact(QEventDispatcherMac *loop) {
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
                      kEventParamQEventDispatcherMac, typeQEventDispatcherMac, sizeof(loop), &loop);
    PostEventToQueue(GetMainEventQueue(), request_sockact_pending, kEventPriorityStandard);
}

/* sheets */
static EventRef request_showsheet_pending = 0;
void qt_event_request_showsheet(QWidget *w)
{
    Q_ASSERT(qt_mac_is_macsheet(w));
    qt_mac_event_remove(request_showsheet_pending);
    CreateEvent(0, kEventClassQt, kEventQtRequestShowSheet, GetCurrentEventTime(),
                kEventAttributeUserEvent, &request_showsheet_pending);
    SetEventParameter(request_showsheet_pending, kEventParamQWidget, typeQWidget, sizeof(w), &w);
    PostEventToQueue(GetMainEventQueue(), request_showsheet_pending, kEventPriorityStandard);
}

/* window changing. This is a hack around Apple's missing functionality, pending the toolbox
   team fix. --Sam */
static EventRef request_window_change_pending = 0;
void qt_event_request_window_change()
{
    if(request_window_change_pending) {
        if(IsEventInQueue(GetMainEventQueue(), request_window_change_pending))
            return;
#ifdef DEBUG_DROPPED_EVENTS
        qDebug("%s:%d Whoa, we dropped an event on the floor!", __FILE__, __LINE__);
#endif
    }

    CreateEvent(0, kEventClassQt, kEventQtRequestWindowChange, GetCurrentEventTime(),
                kEventAttributeUserEvent, &request_window_change_pending);
    PostEventToQueue(GetMainEventQueue(), request_window_change_pending,
                     kEventPriorityHigh);
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
void qt_event_activate_timer_callbk(EventLoopTimerRef r, void *)
{
    EventLoopTimerRef otc = request_activate_pending.timer;
    qt_event_remove_activate();
    if(r == otc && !request_activate_pending.widget.isNull()) {
        const QWidget *tlw = request_activate_pending.widget->window();
        if(tlw->isVisible() && !(tlw->windowType() == Qt::Desktop) && !(tlw->windowType() == Qt::Popup) && !(tlw->windowType() == Qt::Tool)) {
            CreateEvent(0, kEventClassQt, kEventQtRequestActivate, GetCurrentEventTime(),
                        kEventAttributeUserEvent, &request_activate_pending.event);
            PostEventToQueue(GetMainEventQueue(), request_activate_pending.event, kEventPriorityHigh);
        }
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
    MacTimerInfo *t = 0;
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
}
static EventRef request_context_hold_pending = 0;

void QApplicationPrivate::createEventDispatcher()
{
    if (q->type() != QApplication::Tty)
        eventDispatcher = new QEventDispatcherMac(q);
    else
        eventDispatcher = new QEventDispatcherUNIX(q);
}

void
QApplicationPrivate::qt_context_timer_callbk(EventLoopTimerRef r, void *data)
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

/* app menu */
static QMenu *qt_mac_dock_menu = 0;
Q_GUI_EXPORT void qt_mac_set_dock_menu(QMenu *menu)
{
    qt_mac_dock_menu = menu;
    SetApplicationDockTileMenu(menu->macMenu());
}

/* events that hold pointers to widgets, must be cleaned up like this */
void qt_mac_event_release(QWidget *w)
{
    if (w) {
        qt_mac_event_release(w, request_showsheet_pending);
        qt_mac_event_release(w, request_context_pending);
        if(w == qt_mac_dock_menu) {
            qt_mac_dock_menu = 0;
            SetApplicationDockTileMenu(0);
        }
    }
}

/* watched apple events */
struct QMacAppleEventTypeSpec {
    AEEventClass mac_class;
    AEEventID mac_id;
} app_apple_events[] = {
    { kCoreEventClass, kAEQuitApplication },
    { kCoreEventClass, kAEOpenDocuments }
};

/* watched events */
static EventTypeSpec app_events[] = {
    { kEventClassQt, kEventQtRequestTimer },
    { kEventClassQt, kEventQtRequestWindowChange },
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

    { kEventClassTablet, kEventTabletProximity },

    { kEventClassApplication, kEventAppActivated },
    { kEventClassApplication, kEventAppDeactivated },

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
                        GetEventTypeCount(app_events), app_events, (void *)qApp,
                        &app_proc_handler);
}

static QString qt_guiAppName()
{
    static QString appName;
    if (appName.isEmpty()) {
        ProcessSerialNumber psn;
        if (qt_is_gui_used && GetCurrentProcess(&psn) == noErr) {
            QCFString cfstr;
            CopyProcessName(&psn, &cfstr);
            appName = cfstr;
        } else {
            appName = qAppName();
        }
    }
    return appName;
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
void qt_init(QApplicationPrivate *priv, int)
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
                if(arg.left(5) == "-psn_") {
                    passed_psn = arg.mid(6);
                } else {
                    argv[j++] = argv[i];
                }
        }
        priv->argc = j;

        if(qt_is_gui_used && argv[0] && *argv[0] != '/' && QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)
            qWarning("Qt: QApplication: Warning argv[0] == '%s' is relative.\n"
                     "In order to dispatch events correctly Mac OS X may "
                     "require applications to be run with the *full* path to the "
                     "executable.", argv[0]);
        //special hack to change working directory (for an app bundle) when running from finder
        if(!passed_psn.isNull() && QDir::currentPath() == "/") {
            QCFType<CFURLRef> bundleURL(CFBundleCopyBundleURL(CFBundleGetMainBundle()));
            QString qbundlePath = QCFString(CFURLCopyFileSystemPath(bundleURL,
                                            kCFURLPOSIXPathStyle));
            if(qbundlePath.endsWith(".app"))
                QDir::setCurrent(qbundlePath.section('/', 0, -2));
        }
    }

    QMacMime::initialize();

    qApp->setObjectName(qt_guiAppName());
    if(qt_is_gui_used) {
        QColormap::initialize();
        QFont::initialize();
        QCursorData::initialize();
#if !defined(QMAC_NO_COREGRAPHICS)
        QCoreGraphicsPaintEngine::initialize();
#endif
        QQuickDrawPaintEngine::initialize();
#ifndef QT_NO_ACCESSIBILITY
        QAccessible::initialize();
#endif
        QMacInputContext::initialize();
        QApplicationPrivate::inputContext = new QMacInputContext;

#if defined(QT_THREAD_SUPPORT)
        qt_mac_port_mutex = new QMutex(true);
#endif
        RegisterAppearanceClient();
        if(QApplication::desktopSettingsAware())
            qt_mac_update_os_settings();

        if(!app_proc_handler) {
            app_proc_handlerUPP = NewEventHandlerUPP(QApplicationPrivate::globalEventProcessor);
            qt_init_app_proc_handler();
        }

        if(!app_proc_ae_handlerUPP) {
            app_proc_ae_handlerUPP = AEEventHandlerUPP(QApplicationPrivate::globalAppleEventProcessor);
            for(uint i = 0; i < sizeof(app_apple_events) / sizeof(QMacAppleEventTypeSpec); ++i)
                AEInstallEventHandler(app_apple_events[i].mac_class, app_apple_events[i].mac_id,
                                      app_proc_ae_handlerUPP, (long)qApp, true);
        }

        if(QApplicationPrivate::app_style) {
            QEvent ev(QEvent::Style);
            qt_sendSpontaneousEvent(QApplicationPrivate::app_style, &ev);
        }
    }
    if(QApplication::desktopSettingsAware())
        QApplicationPrivate::qt_mac_apply_settings();
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
        for(uint i = 0; i < sizeof(app_apple_events) / sizeof(QMacAppleEventTypeSpec); ++i)
            AERemoveEventHandler(app_apple_events[i].mac_class, app_apple_events[i].mac_id,
                                 app_proc_ae_handlerUPP, true);
        DisposeAEEventHandlerUPP(app_proc_ae_handlerUPP);
        app_proc_ae_handlerUPP = NULL;
    }
    QPixmapCache::clear();
    if(qt_is_gui_used) {
#ifndef QT_NO_ACCESSIBILITY
        QAccessible::cleanup();
#endif
        QMacInputContext::cleanup();
        QQuickDrawPaintEngine::cleanup();
        QCursorData::cleanup();
        QFont::cleanup();
        QColormap::cleanup();
        if(qt_mac_safe_pdev) {
            delete qt_mac_safe_pdev;
            qt_mac_safe_pdev = 0;
        }
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
    return false;
}

/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/
extern QWidget * mac_mouse_grabber;
extern QWidget * mac_keyboard_grabber;

#ifdef QT3_SUPPORT
void QApplication::setMainWidget(QWidget *mainWidget)
{
    QApplicationPrivate::main_widget = mainWidget;
    if (QApplicationPrivate::main_widget && windowIcon().isNull()
	&& QApplicationPrivate::main_widget->testAttribute(Qt::WA_SetWindowIcon))
        setWindowIcon(QApplicationPrivate::main_widget->windowIcon());
}
#endif
#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/
void QApplication::setOverrideCursor(const QCursor &cursor)
{
    qApp->d->cursor_list.prepend(cursor);

    if(qApp && qApp->activeWindow())
        qt_mac_set_cursor(&qApp->d->cursor_list.first(), QCursor::pos());
}

void QApplication::restoreOverrideCursor()
{
    if (qApp->d->cursor_list.isEmpty())
        return;
    qApp->d->cursor_list.removeFirst();

    if(qApp && qApp->activeWindow()) {
        const QCursor def(Qt::ArrowCursor);
        qt_mac_set_cursor(qApp->d->cursor_list.isEmpty() ? &def : &qApp->d->cursor_list.first(), QCursor::pos());
    }
}
#endif

QWidget *QApplication::topLevelAt(const QPoint &p)
{
    QWidget *widget;
    qt_mac_window_at(p.x(), p.y(), &widget);
    return widget;
}

QWidget *QApplicationPrivate::widgetAt_sys(int x, int y)
{
    QWidget *widget = QApplication::topLevelAt(x, y);
    if(!widget)
        return 0;

    HIViewRef child;
    const QPoint qpt = widget->mapFromGlobal(QPoint(x, y));
    const HIPoint pt = CGPointMake(qpt.x(), qpt.y());
    if(HIViewGetSubviewHit((HIViewRef)widget->winId(), &pt, true, &child) == noErr && child)
        widget = QWidget::find((WId)child);
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

struct mac_enum_mapper
{
    int mac_code;
    int qt_code;
#if defined(DEBUG_KEY_MAPS) || defined(DEBUG_MOUSE_MAPS)
#   define MAP_MAC_ENUM(x) x, #x
    const char *desc;
#else
#   define MAP_MAC_ENUM(x) x
#endif
};

//modifiers
static mac_enum_mapper modifier_symbols[] = {
    { shiftKey, MAP_MAC_ENUM(Qt::ShiftModifier) },
    { rightShiftKeyBit, MAP_MAC_ENUM(Qt::ShiftModifier) },
    { controlKey, MAP_MAC_ENUM(Qt::MetaModifier) },
    { rightControlKey, MAP_MAC_ENUM(Qt::MetaModifier) },
    { cmdKey, MAP_MAC_ENUM(Qt::ControlModifier) },
    { optionKey, MAP_MAC_ENUM(Qt::AltModifier) },
    { rightOptionKey, MAP_MAC_ENUM(Qt::AltModifier) },
    { kEventKeyModifierNumLockMask, MAP_MAC_ENUM(Qt::KeypadModifier) },
    { 0, MAP_MAC_ENUM(0) }
};
static Qt::KeyboardModifiers get_modifiers(int keys, bool from_mouse=false)
{
#if !defined(DEBUG_KEY_MAPS) || defined(DEBUG_MOUSE_MAPS)
    Q_UNUSED(from_mouse);
#endif
#ifdef DEBUG_KEY_MAPS
#ifndef DEBUG_MOUSE_MAPS
            if(!from_mouse)
#endif
                qDebug("Qt: internal: **Mapping modifiers: %d (0x%04x) -- %d", keys, keys, from_mouse);
#endif
    Qt::KeyboardModifiers ret = Qt::NoModifier;
    for(int i = 0; modifier_symbols[i].qt_code; i++) {
        if(keys & modifier_symbols[i].mac_code) {
#ifdef DEBUG_KEY_MAPS
#ifndef DEBUG_MOUSE_MAPS
            if(!from_mouse)
#endif
                qDebug("Qt: internal: %d: got modifier: %s", from_mouse, modifier_symbols[i].desc);
#endif
            ret |= Qt::KeyboardModifier(modifier_symbols[i].qt_code);
        }
    }
    return ret;
}
void qt_mac_send_modifiers_changed(quint32 modifiers, QObject *object)
{
    static quint32 cachedModifiers = 0;
    quint32 lastModifiers = cachedModifiers,
          changedModifiers = lastModifiers ^ modifiers;
    cachedModifiers = modifiers;

    //check the bits
    static mac_enum_mapper modifier_key_symbols[] = {
        { shiftKeyBit, MAP_MAC_ENUM(Qt::Key_Shift) },
        { rightShiftKeyBit, MAP_MAC_ENUM(Qt::Key_Shift) }, //???
        { controlKeyBit, MAP_MAC_ENUM(Qt::Key_Meta) },
        { rightControlKeyBit, MAP_MAC_ENUM(Qt::Key_Meta) }, //???
        { cmdKeyBit, MAP_MAC_ENUM(Qt::Key_Control) },
        { optionKeyBit, MAP_MAC_ENUM(Qt::Key_Alt) },
        { rightOptionKeyBit, MAP_MAC_ENUM(Qt::Key_Alt) }, //???
        { alphaLockBit, MAP_MAC_ENUM(Qt::Key_CapsLock) },
        { kEventKeyModifierNumLockBit, MAP_MAC_ENUM(Qt::Key_NumLock) },
        {   0, MAP_MAC_ENUM(0) } };
    for(int i = 0; i <= 32; i++) { //just check each bit
        if(!(changedModifiers & (1 << i)))
            continue;
        QEvent::Type etype = QEvent::KeyPress;
        if(lastModifiers & (1 << i))
            etype = QEvent::KeyRelease;
        int key = 0;
        for(uint x = 0; modifier_key_symbols[x].mac_code; x++) {
            if(modifier_key_symbols[x].mac_code == i) {
#ifdef DEBUG_KEY_MAPS
                qDebug("got modifier changed: %s", modifier_key_symbols[x].desc);
#endif
                key = modifier_key_symbols[x].qt_code;
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
        qDebug("KeyEvent (modif): Sending %s to %s::%s: %d - 0x%08x",
               etype == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
               object ? object->metaObject()->className() : "none",
               object ? object->objectName().toLatin1().constData() : "",
               key, (int)modifiers);
#endif
        QKeyEvent ke(etype, key, get_modifiers(modifiers ^ (1 << i)), "", false);
        qt_sendSpontaneousEvent(object, &ke);
    }
}

//mouse buttons
static mac_enum_mapper mouse_symbols[] = {
    { kEventMouseButtonPrimary, MAP_MAC_ENUM(Qt::LeftButton) },
    { kEventMouseButtonSecondary, MAP_MAC_ENUM(Qt::RightButton) },
    { kEventMouseButtonTertiary, MAP_MAC_ENUM(Qt::MidButton) },
    { 0, MAP_MAC_ENUM(0) }
};
static Qt::MouseButtons get_buttons(int buttons)
{
#ifdef DEBUG_MOUSE_MAPS
    qDebug("Qt: internal: **Mapping buttons: %d (0x%04x)", buttons, buttons);
#endif
    Qt::MouseButtons ret = Qt::NoButton;
    for(int i = 0; mouse_symbols[i].qt_code; i++) {
        if(buttons & (0x01<<(mouse_symbols[i].mac_code-1))) {
#ifdef DEBUG_MOUSE_MAPS
            qDebug("Qt: internal: got button: %s", mouse_symbols[i].desc);
#endif
            ret |= Qt::MouseButtons(mouse_symbols[i].qt_code);
        }
    }
    return ret;
}
static Qt::MouseButton get_button(EventMouseButton button)
{
#ifdef DEBUG_MOUSE_MAPS
    qDebug("Qt: internal: **Mapping button: %d (0x%04x)", button, button);
#endif
    Qt::MouseButtons ret = 0;
    for(int i = 0; mouse_symbols[i].qt_code; i++) {
        if(button == mouse_symbols[i].mac_code) {
#ifdef DEBUG_MOUSE_MAPS
            qDebug("Qt: internal: got button: %s", mouse_symbols[i].desc);
#endif
            return Qt::MouseButton(mouse_symbols[i].qt_code);
        }
    }
    return Qt::NoButton;
}


//keyboard keys (non-modifiers)
static mac_enum_mapper keyboard_symbols[] = {
    { kHomeCharCode, MAP_MAC_ENUM(Qt::Key_Home) },
    { kEnterCharCode, MAP_MAC_ENUM(Qt::Key_Enter) },
    { kEndCharCode, MAP_MAC_ENUM(Qt::Key_End) },
    { kBackspaceCharCode, MAP_MAC_ENUM(Qt::Key_Backspace) },
    { kTabCharCode, MAP_MAC_ENUM(Qt::Key_Tab) },
    { kPageUpCharCode, MAP_MAC_ENUM(Qt::Key_PageUp) },
    { kPageDownCharCode, MAP_MAC_ENUM(Qt::Key_PageDown) },
    { kReturnCharCode, MAP_MAC_ENUM(Qt::Key_Return) },
    { kEscapeCharCode, MAP_MAC_ENUM(Qt::Key_Escape) },
    { kLeftArrowCharCode, MAP_MAC_ENUM(Qt::Key_Left) },
    { kRightArrowCharCode, MAP_MAC_ENUM(Qt::Key_Right) },
    { kUpArrowCharCode, MAP_MAC_ENUM(Qt::Key_Up) },
    { kDownArrowCharCode, MAP_MAC_ENUM(Qt::Key_Down) },
    { kHelpCharCode, MAP_MAC_ENUM(Qt::Key_Help) },
    { kDeleteCharCode, MAP_MAC_ENUM(Qt::Key_Delete) },
//ascii maps, for debug
    { ':', MAP_MAC_ENUM(Qt::Key_Colon) },
    { ';', MAP_MAC_ENUM(Qt::Key_Semicolon) },
    { '<', MAP_MAC_ENUM(Qt::Key_Less) },
    { '=', MAP_MAC_ENUM(Qt::Key_Equal) },
    { '>', MAP_MAC_ENUM(Qt::Key_Greater) },
    { '?', MAP_MAC_ENUM(Qt::Key_Question) },
    { '@', MAP_MAC_ENUM(Qt::Key_At) },
    { ' ', MAP_MAC_ENUM(Qt::Key_Space) },
    { '!', MAP_MAC_ENUM(Qt::Key_Exclam) },
    { '"', MAP_MAC_ENUM(Qt::Key_QuoteDbl) },
    { '#', MAP_MAC_ENUM(Qt::Key_NumberSign) },
    { '$', MAP_MAC_ENUM(Qt::Key_Dollar) },
    { '%', MAP_MAC_ENUM(Qt::Key_Percent) },
    { '&', MAP_MAC_ENUM(Qt::Key_Ampersand) },
    { '\'', MAP_MAC_ENUM(Qt::Key_Apostrophe) },
    { '(', MAP_MAC_ENUM(Qt::Key_ParenLeft) },
    { ')', MAP_MAC_ENUM(Qt::Key_ParenRight) },
    { '*', MAP_MAC_ENUM(Qt::Key_Asterisk) },
    { '+', MAP_MAC_ENUM(Qt::Key_Plus) },
    { ',', MAP_MAC_ENUM(Qt::Key_Comma) },
    { '-', MAP_MAC_ENUM(Qt::Key_Minus) },
    { '.', MAP_MAC_ENUM(Qt::Key_Period) },
    { '/', MAP_MAC_ENUM(Qt::Key_Slash) },
    { '[', MAP_MAC_ENUM(Qt::Key_BracketLeft) },
    { ']', MAP_MAC_ENUM(Qt::Key_BracketRight) },
    { '\\', MAP_MAC_ENUM(Qt::Key_Backslash) },
    { '_', MAP_MAC_ENUM(Qt::Key_Underscore) },
    { '`', MAP_MAC_ENUM(Qt::Key_QuoteLeft) },
    { '{', MAP_MAC_ENUM(Qt::Key_BraceLeft) },
    { '}', MAP_MAC_ENUM(Qt::Key_BraceRight) },
    { '|', MAP_MAC_ENUM(Qt::Key_Bar) },
    { '~', MAP_MAC_ENUM(Qt::Key_AsciiTilde) },
    { '^', MAP_MAC_ENUM(Qt::Key_AsciiCircum) },
    {   0, MAP_MAC_ENUM(0) }
};

static mac_enum_mapper keyscan_symbols[] = { //real scan codes
    { 122, MAP_MAC_ENUM(Qt::Key_F1) },
    { 120, MAP_MAC_ENUM(Qt::Key_F2) },
    { 99,  MAP_MAC_ENUM(Qt::Key_F3) },
    { 118, MAP_MAC_ENUM(Qt::Key_F4) },
    { 96,  MAP_MAC_ENUM(Qt::Key_F5) },
    { 97,  MAP_MAC_ENUM(Qt::Key_F6) },
    { 98,  MAP_MAC_ENUM(Qt::Key_F7) },
    { 100, MAP_MAC_ENUM(Qt::Key_F8) },
    { 101, MAP_MAC_ENUM(Qt::Key_F9) },
    { 109, MAP_MAC_ENUM(Qt::Key_F10) },
    { 103, MAP_MAC_ENUM(Qt::Key_F11) },
    { 111, MAP_MAC_ENUM(Qt::Key_F12) },
    {   0, MAP_MAC_ENUM(0) }
};

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

    for(int i = 0; keyboard_symbols[i].qt_code; i++) {
        if(keyboard_symbols[i].mac_code == key) {
            /* To work like Qt/X11 we issue Backtab when Shift + Tab are pressed */
            if(keyboard_symbols[i].qt_code == Qt::Key_Tab && (modif & Qt::ShiftModifier)) {
#ifdef DEBUG_KEY_MAPS
                qDebug("%d: got key: Qt::Key_Backtab", __LINE__);
#endif
                return Qt::Key_Backtab;
            }

#ifdef DEBUG_KEY_MAPS
            qDebug("%d: got key: %s", __LINE__, keyboard_symbols[i].desc);
#endif
            return keyboard_symbols[i].qt_code;
        }
    }

    //last ditch try to match the scan code
    for(int i = 0; keyscan_symbols[i].qt_code; i++) {
        if(keyscan_symbols[i].mac_code == scan) {
#ifdef DEBUG_KEY_MAPS
            qDebug("%d: got key: %s", __LINE__, keyscan_symbols[i].desc);
#endif
            return keyscan_symbols[i].qt_code;
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
bool QApplicationPrivate::do_mouse_down(const QPoint &pt, bool *mouse_down_unhandled)
{
    //find the widget/part
    QWidget *widget = 0;
    short windowPart = qt_mac_window_at(pt.x(), pt.y(), &widget);

    //close down the popups
    int popup_close_count = 0;
    if(inPopupMode() && widget != q->activePopupWidget()) {
        while(inPopupMode()) {
            q->activePopupWidget()->close();
            ++popup_close_count;
            if(windowPart == inContent)
                break;
        }
    }

    //handle the down
    if(mouse_down_unhandled)
        (*mouse_down_unhandled) = false;
    if(windowPart == inMenuBar) {
        Point mac_pt;
        mac_pt.h = pt.x();
        mac_pt.v = pt.y();
        QMacBlockingFunction block;
        MenuSelect(mac_pt); //allow menu tracking
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
            if(!widget->isActiveWindow() && widget->isWindow() && !(widget->windowType() == Qt::Desktop)
               && !(widget->windowType() == Qt::Popup) && !qt_mac_is_macsheet(widget)
               && (widget->isModal() || !::qobject_cast<QDockWidget *>(widget))) {
                widget->activateWindow();
                if(windowPart == inContent) {
                    HIViewRef child;
                    const HIPoint hiPT = CGPointMake(pt.x() - widget->geometry().x(), pt.y() - widget->geometry().y());
                    if(HIViewGetSubviewHit((HIViewRef)widget->winId(), &hiPT, true, &child) == noErr && child) {
                        if(!qt_mac_can_clickThrough(QWidget::find((WId)child))) {
                            if(mouse_down_unhandled)
                                (*mouse_down_unhandled) = true;
                            return false;
                        }
                    }
                }
            }
        }
    }
    if(windowPart == inContent)
        return !popup_close_count; //just return and let the event loop process

    WindowPtr window = qt_mac_window_for(widget);
    if(windowPart == inGoAway || windowPart == inCollapseBox ||
       windowPart == inZoomIn || windowPart == inZoomOut
       || windowPart == inToolbarButton) {
        Point mac_pt;
        mac_pt.h = pt.x();
        mac_pt.v = pt.y();
        QMacBlockingFunction block;
        if(!TrackBox(window, mac_pt, windowPart))
            return false;
    }

    switch(windowPart) {
    case inStructure:
    case inDesk:
        break;
    case inGoAway:
        widget->d->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
        break;
    case inToolbarButton: {
        QToolBarChangeEvent ev(!(GetCurrentKeyModifiers() & cmdKey));
        QApplication::sendSpontaneousEvent(widget, &ev);
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
            Point mac_pt;
            mac_pt.h = pt.x();
            mac_pt.v = pt.y();
            QMacBlockingFunction block;
            DragWindow(window, mac_pt, 0);
        }
        QPoint np, op(widget->data->crect.x(), widget->data->crect.y());
        {
            QMacSavedPortInfo savedInfo(widget);
            Point p = { 0, 0 };
            LocalToGlobal(&p);
            np = QPoint(p.h, p.v);
        }
        if(np != op)
            widget->data->crect = QRect(np, widget->data->crect.size());
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
            Point mac_pt;
            mac_pt.h = pt.x();
            mac_pt.v = pt.y();
            QMacBlockingFunction block;
            growWindowSize = GrowWindow(window, mac_pt, limits.left == -2 ? 0 : &limits);
        }
        if(growWindowSize) {
            // nw/nh might not match the actual size if setSizeIncrement is used
            int nw = LoWord(growWindowSize);
            int nh = HiWord(growWindowSize);
            if(nw != widget->width() || nh != widget->height()) {
                if(nw < q->desktop()->width() && nw > 0 && nh < q->desktop()->height() && nh > 0)
                    widget->resize(nw, nh);
            }
        }
        break;
    }
    case inCollapseBox: {
        widget->setWindowState(widget->windowState() | Qt::WindowMinimized);
        //we send a hide to be like X11/Windows
        QEvent e(QEvent::Hide);
        QApplication::sendSpontaneousEvent(widget, &e);
        break; }
    case inZoomIn:
        widget->setWindowState(widget->windowState() & ~Qt::WindowMaximized);
        break;
    case inZoomOut:
        widget->setWindowState(widget->windowState() | Qt::WindowMaximized);
        break;
    default:
        qDebug("Qt: internal: Unhandled case in mouse_down.. %d", windowPart);
        break;
    }
    return false;
}


bool QApplicationPrivate::modalState()
{
    return app_do_modal;
}

void QApplicationPrivate::enterModal(QWidget *widget)
{
#ifdef DEBUG_MODAL_EVENTS
    qDebug("Entering modal state with %s::%s::%p (%d)", widget->metaObject()->className(), widget->objectName().local8Bit(),
           widget, qt_modal_stack ? (int)qt_modal_stack->count() : -1);
#endif
    if(!qt_modal_stack) {                        // create modal stack
        qt_modal_stack = new QWidgetList;
    }
    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }

    dispatchEnterLeave(0, qt_mouseover);
    qt_mouseover = 0;

    qt_modal_stack->insert(0, widget);
    if(!app_do_modal)
        qt_event_request_menubarupdate();
    app_do_modal = true;
}


void QApplicationPrivate::leaveModal(QWidget *widget)
{
    if(qt_modal_stack && qt_modal_stack->removeAll(widget)) {
#ifdef DEBUG_MODAL_EVENTS
        qDebug("Leaving modal state with %s::%s::%p (%d)", widget->metaObject()->className(), widget->objectName().local8Bit(),
               widget, qt_modal_stack->count());
#endif
        if(qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
            QPoint p(QCursor::pos());
            app_do_modal = false;
            QWidget* w = QApplication::widgetAt(p.x(), p.y());
            dispatchEnterLeave(w, qt_mouseover); // send synthetic enter event
            qt_mouseover = w;
        }
    }
#ifdef DEBUG_MODAL_EVENTS
    else qDebug("Failure to remove %s::%s::%p -- %p", widget->metaObject()->className(), widget->objectName().local8Bit(), widget, qt_modal_stack);
#endif
    app_do_modal = (qt_modal_stack != 0);
    if(!app_do_modal)
        qt_event_request_menubarupdate();

    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }
}

QWidget *QApplicationPrivate::tryModalHelperMac(QWidget *top) {
    if(top && qt_mac_is_macsheet(top) && !IsWindowVisible(qt_mac_window_for(top))) {
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

    if (QApplicationPrivate::tryModalHelper(widget, &top))
        return true;

    bool block_event  = false;
    bool paint_event = false;

    UInt32 ekind = GetEventKind(event), eclass=GetEventClass(event);
    switch(eclass) {
    case kEventClassMouse:
        if(!top->isActiveWindow())
            top->activateWindow();
        block_event = true;
        break;
    case kEventClassKeyboard:
        block_event = true;
        break;
    case kEventClassWindow:
        paint_event = (ekind == kEventWindowUpdate);
        break;
    }

    if(top->isWindow() && (block_event || paint_event))
        top->raise();
#if 0 //This is really different than Qt behaves, but it is correct for Aqua, what do I do? -Sam
    if(block_event && qt_mac_is_macsheet(top)) {
        for(QWidget *w = top->parentWidget(); w; w = w->parentWidget()) {
            w = w->window();
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
QApplicationPrivate::globalEventProcessor(EventHandlerCallRef er, EventRef event, void *data)
{
    QApplication *app = (QApplication *)data;
    long result;
    if (app->filterEvent(&event, &result))
        return result;
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
                WindowPtr window = qt_mac_window_for(widget);
                bool just_show = false;
                if(!qt_mac_is_macsheet(widget)
                   || ShowSheetWindow(window, qt_mac_window_for(widget->parentWidget())) != noErr) {
                    qWarning("Qt: QWidget: Unable to show as sheet %s::%s", widget->metaObject()->className(),
                             widget->objectName().toLocal8Bit().constData());
                    just_show = true;
                }
                if(just_show) //at least the window will be visible, but the sheet flag doesn't work sadly (probalby too many sheets)
                    ShowHide(window, true);
            }
        } else if(ekind == kEventQtRequestWindowChange) {
            qt_mac_event_release(request_window_change_pending);
            QMacWindowChangeEvent::exec();
        } else if(ekind == kEventQtRequestMenubarUpdate) {
            qt_mac_event_release(request_menubarupdate_pending);
            QMenuBar::macUpdateMenuBar();
        } else if(ekind == kEventQtRequestSelect) {
            qt_mac_event_release(request_select_pending);
            QEventDispatcherMac *l = 0;
            GetEventParameter(event, kEventParamQEventDispatcherMac, typeQEventDispatcherMac, 0, sizeof(l), 0, &l);
            timeval tm;
            memset(&tm, '\0', sizeof(tm));
            l->d->doSelect(QEventLoop::AllEvents, &tm);
        } else if(ekind == kEventQtRequestSocketAct) {
            qt_mac_event_release(request_sockact_pending);
            QEventDispatcherMac *l = 0;
            GetEventParameter(event, kEventParamQEventDispatcherMac, typeQEventDispatcherMac, 0, sizeof(l), 0, &l);
            l->activateSocketNotifiers();
        } else if(ekind == kEventQtRequestActivate) {
            qt_mac_event_release(request_activate_pending.event);
            if(request_activate_pending.widget) {
                QWidget *tlw = request_activate_pending.widget->window();
                request_activate_pending.widget = 0;
                tlw->activateWindow();
                SelectWindow((WindowPtr)tlw->handle());
            }
        } else if(ekind == kEventQtRequestContext) {
            bool send = false;
            if((send = (event == request_context_hold_pending)))
                qt_mac_event_release(request_context_hold_pending);
            else if((send = (event == request_context_pending)))
                qt_mac_event_release(request_context_pending);
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
                    QContextMenuEvent qme(QContextMenuEvent::Mouse, plocal, where);
                    QApplication::sendEvent(widget, &qme);
                    if(qme.isAccepted()) { //once this happens the events before are pitched
                        qt_button_down = 0;
                        qt_mac_dblclick.active = false;
                    }
                } else {
                    handled_event = false;
                }
            }
        } else if(ekind == kEventQtRequestTimer) {
            MacTimerInfo *t = 0;
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
    case kEventClassTablet:
        switch (ekind) {
        case kEventTabletProximity:
            // Get the current point of the device and its unique ID.
            TabletProximityRec proxRec;
            GetEventParameter(event, kEventParamTabletProximityRec, typeTabletProximityRec, 0,
                              sizeof(proxRec), 0, &proxRec);
            tabletUniqueID = proxRec.uniqueID;
            tabletCaps = proxRec.capabilityMask;
            // Defined in some non-existent Wacom.h
            // EUnknown = 0, EPen = 1, ECursor = 2, EEraser = 3
            switch (proxRec.pointerType) {
                case 0:
                default:
                    currPointerType = QTabletEvent::UnknownPointer;
                    break;
                case 1:
                    currPointerType = QTabletEvent::Pen;
                    break;
                case 2:
                    currPointerType = QTabletEvent::Cursor;
                    break;
                case 3:
                    currPointerType = QTabletEvent::Eraser;
                    break;
            }

            // Defined in the "EN0056-NxtGenImpGuideX"
            // on Wacom's Developer Website (www.wacomeng.com)
            switch (proxRec.vendorPointerType & 0x0006) {
            case 0x0002:
                if ((proxRec.vendorPointerType & 0x0F06) != 0x902)
                    currTabletDevice = QTabletEvent::Stylus;
                else
                    currTabletDevice = QTabletEvent::Airbrush;
                break;
            case 0x0004:
                currTabletDevice = QTabletEvent::FourDMouse;
                break;
            case 0x0006:
                currTabletDevice = QTabletEvent::Puck;
                break;
            }
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
        Qt::KeyboardModifiers modifiers;
        {
            UInt32 mac_modifiers = 0;
            GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 0,
                              sizeof(mac_modifiers), 0, &mac_modifiers);
            modifiers = get_modifiers(mac_modifiers, true);
        }
        Qt::MouseButtons buttons;
        {
            UInt32 mac_buttons = 0;
            GetEventParameter(event, kEventParamMouseChord, typeUInt32, 0,
                              sizeof(mac_buttons), 0, &mac_buttons);
            buttons = get_buttons(mac_buttons);
        }
        int wheel_delta=0;
        if(ekind == kEventMouseWheelMoved) {
            long int mdelt = 0;
            GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, 0,
                              sizeof(mdelt), 0, &mdelt);
            wheel_delta = mdelt * 120;
        }

        Qt::MouseButton button = Qt::NoButton;
        if(ekind == kEventMouseDown || ekind == kEventMouseUp) {
            EventMouseButton mac_button = 0;
            GetEventParameter(event, kEventParamMouseButton, typeMouseButton, 0,
                              sizeof(mac_button), 0, &mac_button);
            button = get_button(mac_button);
        }

        switch(ekind) {
        case kEventMouseDown:
        {
            etype = QEvent::MouseButtonPress;
            if(qt_mac_dblclick.active) {
                if(qt_mac_dblclick.use_qt_time_limit) {
                    EventTime now = GetEventTime(event);
                    if(qt_mac_dblclick.last_time != -2 &&
                       now - qt_mac_dblclick.last_time <= ((double)QApplicationPrivate::mouse_double_click_time)/1000)
                        etype = QEvent::MouseButtonDblClick;
                } else {
                    UInt32 count = 0;
                    GetEventParameter(event, kEventParamClickCount, typeUInt32, 0,
                                      sizeof(count), 0, &count);
                    if(!(count % 2) && qt_mac_dblclick.last_modifiers == modifiers &&
                       qt_mac_dblclick.last_button == button)
                        etype = QEvent::MouseButtonDblClick;
                }
                if(etype == QEvent::MouseButtonDblClick)
                    qt_mac_dblclick.active = false;
            }
            break;
        }
        case kEventMouseUp:
            etype = QEvent::MouseButtonRelease;
            break;
        case kEventMouseDragged:
        case kEventMouseMoved:
            etype = QEvent::MouseMove;
            break;
        }
        //figure out which widget to send it to
        if(app->d->inPopupMode()) {
            QWidget *popup = qApp->activePopupWidget();
            if (qt_button_down && qt_button_down->window() == popup) {
                widget = qt_button_down;
            } else {
                QPoint pos = popup->mapFromGlobal(QPoint(where.h, where.v));
                widget = popup->childAt(pos);
            }
            if(!widget)
                widget = popup;
        } else {
            if(mac_mouse_grabber) {
                widget = mac_mouse_grabber;
            } else if(ekind != kEventMouseDown && qt_button_down) {
                widget = qt_button_down;
            } else {
                {
                    WindowPtr window = 0;
                    if(GetEventParameter(event, kEventParamWindowRef, typeWindowRef, 0,
                                         sizeof(window), 0, &window) != noErr)
                        FindWindowOfClass(&where, kAllWindowClasses, &window, 0);
                    if(window) {
                        HIViewRef hiview;
                        if(HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &hiview) == noErr) {
                            widget = QWidget::find((WId)hiview);;
                            if (widget) {
                                // Make sure we didn't pass over a widget with a "fake hole" in it.
                                QPoint pos = widget->mapFromGlobal(QPoint(where.h, where.v));
                                QWidget *otherWidget = widget->childAt(pos);
                                if (otherWidget && otherWidget->testAttribute(Qt::WA_MouseNoMask))
                                    widget = otherWidget;
                            }
                        }
                    }
                }
                if(!widget) //fallback
                    widget = QApplication::widgetAt(where.h, where.v);
                if(ekind == kEventMouseUp && widget) {
                    short part = qt_mac_window_at(where.h, where.v);
                    if(part == inDrag) {
                        UInt32 count = 0;
                        GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL,
                                          sizeof(count), NULL, &count);
                        if(count == 2 && qt_mac_collapse_on_dblclick) {
                            widget->setWindowState(widget->windowState() | Qt::WindowMinimized);
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
                        if(extra && extra->curs && cursor_widget->isEnabled()) {
                            cursor = *extra->curs;
                            break;
                        }
                    }
                }
            }
            qt_mac_set_cursor(&cursor, QPoint(where.h, where.v));
        }

        //This mouse button state stuff looks like this on purpose
        //although it looks hacky it is VERY intentional..
        if(widget && app_do_modal && !qt_try_modal(widget, event)) {
            if(ekind == kEventMouseDown && qt_mac_is_macsheet(QApplication::activeModalWidget())) {
                QApplication::activeModalWidget()->parentWidget()->activateWindow(); //sheets have a parent
                app->d->do_mouse_down(QPoint(where.h, where.v), 0);
            }
            break;
        }

        UInt32 tabletEventType = 0;
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
                int tiltX = ((int)tabletPointRec.tiltX)/(32767/64); // 32K -> 60
                int tiltY = ((int)tabletPointRec.tiltY)/(-32767/64); // 32K -> 60
                HIPoint hiPoint;
                GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, 0, sizeof(HIPoint), 0, &hiPoint);
                QPointF hiRes(hiPoint.x, hiPoint.y);
                QPoint global(where.h, where.v);
                QPoint local(widget->mapFromGlobal(global));
                int z = 0;
                qreal rotation = 0.0;
                qreal tp = 0.0;
                // Again from the Wacom.h header
                if (tabletCaps & 0x0200)     // Z-axis
                    z = tabletPointRec.absZ;

                if (tabletCaps & 0x0800)  // Tangental pressure
                    tp = tabletPointRec.tangentialPressure / 32767.0;

                if (tabletCaps & 0x2000) // Rotation
                    rotation = qreal(tabletPointRec.rotation) / 64.0;

                QTabletEvent e(t, local, global, hiRes, currTabletDevice, currPointerType,
                               qreal(tabletPointRec.pressure / qreal(0xffff)), tiltX, tiltY,
                               tp, rotation, z, modifiers, tabletUniqueID);
                QApplication::sendSpontaneousEvent(widget, &e);
                if (e.isAccepted())
                    break;
            }
        }

        if(ekind == kEventMouseDown) {
            bool mouse_down_unhandled;
            if(!app->d->do_mouse_down(QPoint(where.h, where.v), &mouse_down_unhandled)) {
                if(mouse_down_unhandled) {
                    handled_event = false;
                    break;
                }
                break;
            }
       }

        switch(ekind) {
        case kEventMouseDragged:
        case kEventMouseMoved:
            if((QWidget *)qt_mouseover != widget) {
#ifdef DEBUG_MOUSE_MAPS
                qDebug("Entering: %p - %s (%s), Leaving %s (%s)", (QWidget*)widget,
                       widget ? widget->metaObject()->className() : "none",
                       widget ? widget->objectName().toLocal8Bit().constData() : "",
                       qt_mouseover ? qt_mouseover->metaObject()->className() : "none",
                       qt_mouseover ? qt_mouseover->objectName().toLocal8Bit().constData() : "");
#endif
                QApplicationPrivate::dispatchEnterLeave(widget, qt_mouseover);
                qt_mouseover = widget;
            }
            break;
        case kEventMouseDown:
            if(button == Qt::LeftButton && !mac_context_timer && qt_mac_press_and_hold_context) {
                remove_context_timer = false;
                if(!mac_context_timerUPP)
                    mac_context_timerUPP = NewEventLoopTimerUPP(QApplicationPrivate::qt_context_timer_callbk);
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
                qt_mac_dblclick.last_modifiers = modifiers;
                qt_mac_dblclick.last_button = button;
                qt_mac_dblclick.last_time = GetEventTime(event);
            }
            if(wheel_delta) {
                EventMouseWheelAxis axis;
                GetEventParameter(event, kEventParamMouseWheelAxis, typeMouseWheelAxis, 0,
                                  sizeof(axis), 0, &axis);
                QWheelEvent qwe(plocal, p, wheel_delta, buttons, modifiers,
                                axis == kEventMouseWheelAxisX ? Qt::Horizontal : Qt::Vertical);
                QApplication::sendSpontaneousEvent(widget, &qwe);
                if(!qwe.isAccepted() && QApplicationPrivate::focus_widget && QApplicationPrivate::focus_widget != widget) {
                    QWheelEvent qwe2(QApplicationPrivate::focus_widget->mapFromGlobal(p), p,
                                     wheel_delta, buttons, modifiers,
                                     axis == kEventMouseWheelAxisX ? Qt::Horizontal : Qt::Vertical);
                    QApplication::sendSpontaneousEvent(QApplicationPrivate::focus_widget, &qwe2);
                }
            } else {
#ifdef QMAC_SPEAK_TO_ME
                if(etype == QMouseEvent::MouseButtonDblClick && (modifiers & Qt::AltModifier)) {
                    QVariant v = widget->property("text");
                    if(!v.isValid()) v = widget->property("windowTitle");
                    if(v.isValid()) {
                        QString s = v.toString();
                        s.replace(QRegExp(QString::fromLatin1("(\\&|\\<[^\\>]*\\>)")), "");
                        SpeechChannel ch;
                        NewSpeechChannel(0, &ch);
                        SpeakText(ch, s.toLatin1().constData(), s.length());
                    }
                }
#endif
                Qt::MouseButton buttonToSend = button;
                static bool lastButtonTranslated = false;
                if(ekind == kEventMouseDown &&
                   button == Qt::LeftButton && (modifiers & Qt::MetaModifier)) {
                    buttonToSend = Qt::RightButton;
                    lastButtonTranslated = true;
                } else if(ekind == kEventMouseUp && lastButtonTranslated) {
                    buttonToSend = Qt::RightButton;
                    lastButtonTranslated = false;
                }
                QMouseEvent qme(etype, plocal, p, buttonToSend, buttons, modifiers);
                QApplication::sendSpontaneousEvent(widget, &qme);
            }
            if(ekind == kEventMouseDown &&
               ((button == Qt::RightButton) ||
                (button == Qt::LeftButton && (modifiers & Qt::MetaModifier))))
                qt_event_request_context();

#ifdef DEBUG_MOUSE_MAPS
            const char *event_desc = edesc;
            if(etype == QEvent::MouseButtonDblClick)
                event_desc = "Double Click";
            qDebug("%d %d (%d %d) - Would send (%s) event to %p %s %s (%d 0x%08x 0x%08x %d)", p.x(), p.y(),
                   plocal.x(), plocal.y(), event_desc, (QWidget*)widget,
                   widget ? widget->objectName().toLocal8Bit().constData() : "*Unknown*",
                   widget ? widget->metaObject()->className() : "*Unknown*",
                   button, (int)buttons, (int)modifiers, wheel_delta);
#endif
        } else {
            handled_event = false;
        }
        break;
    }
    case kEventClassKeyboard: {
        // unfortunatly modifiers changed event looks quite different, so I have a separate
        // code path
        if(ekind == kEventRawKeyModifiersChanged) {
            //figure out changed modifiers, wish Apple would just send a delta
            UInt32 modifiers = 0;
            GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 0,
                              sizeof(modifiers), 0, &modifiers);
            //find which widget to send to
            if(mac_keyboard_grabber)
                widget = mac_keyboard_grabber;
            else if (app->d->inPopupMode())
                widget = (app->activePopupWidget()->focusWidget() ?
                          app->activePopupWidget()->focusWidget() : app->activePopupWidget());
            else if(QApplicationPrivate::focus_widget)
                widget = QApplicationPrivate::focus_widget;
            if(!widget || (app_do_modal && !qt_try_modal(widget, event)))
                break;
            qt_mac_send_modifiers_changed(modifiers, widget);
            break;
        }

        //get modifiers
        Qt::KeyboardModifiers modifiers;
        {
            UInt32 mac_modifiers = 0;
            GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 0,
                              sizeof(mac_modifiers), 0, &mac_modifiers);
#ifdef DEBUG_KEY_MAPS
            qDebug("************ Mapping modifiers and key ***********");
#endif
            modifiers = get_modifiers(mac_modifiers, false);
#ifdef DEBUG_KEY_MAPS
            qDebug("------------ Mapping modifiers and key -----------");
#endif
        }

        //get keycode
        UInt32 keycode = 0;
        GetEventParameter(event, kEventParamKeyCode, typeUInt32, 0, sizeof(keycode), 0, &keycode);

        //get mac mapping
        static UInt32 tmp_unused_state = 0L;
        char translatedChar = KeyTranslate((void *)GetScriptVariable(smCurrentScript, smKCHRCache),
                                           (GetCurrentEventKeyModifiers() &
                                            (kEventKeyModifierNumLockMask|shiftKey|
                                             rightShiftKey|alphaLock)) | keycode,
                                           &tmp_unused_state);
        if(!translatedChar) {
            qt_mac_eat_unicode_key = false;
            CallNextEventHandler(er, event);
            handled_event = qt_mac_eat_unicode_key;
            break;
        }

        //map it into qt keys
        QString asString;
        int asChar=get_key(modifiers, translatedChar, keycode);
        if(modifiers & (Qt::AltModifier | Qt::ControlModifier)) {
            if(translatedChar & (1 << 7)) //high ascii
                translatedChar = 0;
        } else {          //now get the real ascii value
            UInt32 tmp_mod = 0L;
            static UInt32 tmp_state = 0L;
            if(modifiers & Qt::ShiftModifier)
                tmp_mod |= shiftKey;
            if(modifiers & Qt::MetaModifier)
                tmp_mod |= controlKey;
            if(modifiers & Qt::ControlModifier)
                tmp_mod |= cmdKey;
            if(GetCurrentEventKeyModifiers() & alphaLock) //no Qt mapper
                tmp_mod |= alphaLock;
            if(modifiers & Qt::AltModifier)
                tmp_mod |= optionKey;
            if(modifiers & Qt::KeypadModifier)
                tmp_mod |= kEventKeyModifierNumLockMask;
            translatedChar = KeyTranslate((void *)GetScriptManagerVariable(smUnicodeScript),
                                          tmp_mod | keycode, &tmp_state);
        }
        /* I don't know why the str is only filled in in RawKeyDown - but it does seem to be on X11
           is this a bug on X11? --Sam */
        QEvent::Type etype = (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress;
        if(etype == QEvent::KeyPress) {
            UInt32 unilen = 0;
            if(GetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, 0, 0, &unilen, 0) == noErr && unilen == 2) {
                UniChar *unicode = (UniChar*)NewPtr(unilen);
                GetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, 0, unilen, 0, unicode);
                asString = QString((QChar*)unicode, unilen / sizeof(UniChar));
                DisposePtr((char *)unicode);
            } else if(translatedChar) {
                static QTextCodec *c = 0;
                if(!c)
                    c = QTextCodec::codecForName("Apple Roman");
                asString = c->toUnicode(&translatedChar, 1);
            }
        }

        if(mac_keyboard_grabber)
            widget = mac_keyboard_grabber;
        else if (app->d->inPopupMode())
            widget = (app->activePopupWidget()->focusWidget() ?
                      app->activePopupWidget()->focusWidget() : app->activePopupWidget());
        else if(QApplicationPrivate::focus_widget)
            widget = QApplicationPrivate::focus_widget;
        if(widget) {
            if(app_do_modal && !qt_try_modal(widget, event))
                break;

            bool key_event = true;
#if defined(QT3_SUPPORT) && !defined(QT_NO_ACCEL)
            if(etype == QEvent::KeyPress && !mac_keyboard_grabber
               && static_cast<QApplicationPrivate*>(qApp->d_ptr)->use_compat()) {
                /* We offer the shortcut a text representation of chr, this is because the Mac
                   actually flips the keyboard when things like alt are pressed, but that doesn't
                   really mean that shortcuts should be mapped to the new key (or things could get
                   quite broken). */
                QString accel_str;
                if(translatedChar) {
                    static QTextCodec *c = 0;
                    if(!c)
                        c = QTextCodec::codecForName("Apple Roman");
                    accel_str = c->toUnicode(&translatedChar, 1);
                }
                QKeyEvent accel_ev(QEvent::ShortcutOverride, asChar, modifiers,
                                   accel_str, ekind == kEventRawKeyRepeat,
                                   qMax(1, accel_str.length()));
                if(static_cast<QApplicationPrivate*>(qApp->d_ptr)->qt_tryAccelEvent(widget, &accel_ev)) {
#ifdef DEBUG_KEY_MAPS
                    qDebug("KeyEvent: %s::%s consumed Accel: %04x %c %s %d",
                           widget ? widget->metaObject()->className() : "none",
                           widget ? widget->objectName().toLatin1().constData() : "",
                           asChar, translatedChar, asString.toLatin1().constData(), ekind == kEventRawKeyRepeat);
#endif
                    key_event = false;
                } else {
                    if(accel_ev.isAccepted()) {
#ifdef DEBUG_KEY_MAPS
                        qDebug("KeyEvent: %s::%s overrode Accel: %04x %c %s %d",
                               widget ? widget->metaObject()->className() : "none",
                               widget ? widget->objectName().toLatin1().constData() : "",
                               asChar, translatedChar, asString.toLatin1().constData(), ekind == kEventRawKeyRepeat);
#endif
                    }
                }
            }
#endif // QT3_SUPPORT && !QT_NO_ACCEL
            if(key_event) {
                //Find out if someone else wants the event, namely
                //is it of use to text services? If so we won't bother
                //with a QKeyEvent.
                qt_mac_eat_unicode_key = false;
                CallNextEventHandler(er, event);
                if(qt_mac_eat_unicode_key) {
                    handled_event = true;
                    break;
                }
#ifdef DEBUG_KEY_MAPS
                qDebug("KeyEvent: Sending %s to %s::%s: %04x '%c' (%s) 0x%08x%s",
                       etype == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
                       widget ? widget->metaObject()->className() : "none",
                       widget ? widget->objectName().toLatin1().constData() : "",
                       asChar, translatedChar, asString.toLatin1().constData(), int(modifiers),
                       ekind == kEventRawKeyRepeat ? " Repeat" : "");
#endif
                /* This is actually wrong - but unfortunatly it is the best that can be
                   done for now because of the Control/Meta mapping problems */
                if(modifiers & (Qt::ControlModifier | Qt::MetaModifier)) {
                    translatedChar = 0;
                    asString = "";
                }
                QKeyEvent ke(etype, asChar, modifiers,
                             asString, ekind == kEventRawKeyRepeat,
                             qMax(1, asString.length()));
                QApplication::sendSpontaneousEvent(widget,&ke);
            }
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassWindow: {
        remove_context_timer = false;

        WindowRef wid = 0;
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
        } else if((widget->windowType() == Qt::Desktop)) {
            handled_event = false;
            break;
        }

        if(ekind == kEventWindowDispose) {
        } else if(ekind == kEventWindowExpanded) {
            widget->setWindowState((widget->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            QShowEvent qse;
            QApplication::sendSpontaneousEvent(widget, &qse);
        } else if(ekind == kEventWindowBoundsChanged) {
            //implicitly removes the maximized bit
            if((widget->windowState() & Qt::WindowMaximized) &&
               IsWindowInStandardState((WindowPtr)widget->handle(), 0, 0))
                widget->setWindowState(widget->windowState() & ~Qt::WindowMaximized);

            handled_event = false;
            UInt32 flags = 0;
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
#if 0
            if(!(widget->windowType() == Qt::Popup))
                widget->activateWindow();
#endif
        } else if(ekind == kEventWindowActivated) {
            if(QApplicationPrivate::app_style) {
                QEvent ev(QEvent::Style);
                QApplication::sendSpontaneousEvent(QApplicationPrivate::app_style, &ev);
            }

            if(app_do_modal && !qt_try_modal(widget, event))
                break;

            if(widget && widget->window()->isVisible()) {
                QWidget *tlw = widget->window();
		if(tlw->isWindow() && !(tlw->windowType() == Qt::Popup)
                   && !qt_mac_is_macdrawer(tlw)
                   && (!tlw->parentWidget() || tlw->isModal()
                       || !(tlw->windowType() == Qt::Tool))) {
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
                    widget->focusWidget()->setFocus(Qt::ActiveWindowFocusReason);
                else
                    widget->setFocus(Qt::ActiveWindowFocusReason);
                QMenuBar::macUpdateMenuBar();
            }
        } else if(ekind == kEventWindowDeactivated) {
            if(widget && QApplicationPrivate::active_window == widget)
                app->setActiveWindow(0);
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassApplication:
        if(ekind == kEventAppActivated) {
            qt_mac_cancel_notification();

            if(QApplication::desktopSettingsAware())
                qt_mac_update_os_settings();
            app->clipboard()->loadScrap(false);
            if(qt_clipboard) { //manufacture an event so the clipboard can see if it has changed
                QEvent ev(QEvent::Clipboard);
                QApplication::sendSpontaneousEvent(qt_clipboard, &ev);
            }
            if(app) {
                QEvent ev(QEvent::ApplicationActivated);
                QApplication::sendSpontaneousEvent(app, &ev);
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
            while(app->d->inPopupMode())
                app->activePopupWidget()->close();
            app->clipboard()->saveScrap();
            if(app) {
                QEvent ev(QEvent::ApplicationDeactivated);
                QApplication::sendSpontaneousEvent(app, &ev);
            }
            app->setActiveWindow(0);
        } else {
            handled_event = false;
        }
        break;
    case kAppearanceEventClass:
        if(ekind == kAEAppearanceChanged) {
            if(QApplication::desktopSettingsAware())
                qt_mac_update_os_settings();
            if(QApplicationPrivate::app_style) {
                QEvent ev(QEvent::Style);
                QApplication::sendSpontaneousEvent(QApplicationPrivate::app_style, &ev);
            }
        } else {
            handled_event = false;
        }
        break;
    case kEventClassAppleEvent:
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
            handled_event = false;
            if(!cmd.menu.menuRef && GetApplicationDockTileMenu()) {
                EventRef copy = CopyEvent(event);
                HICommand copy_cmd;
                GetEventParameter(event, kEventParamDirectObject, typeHICommand,
                                  0, sizeof(copy_cmd), 0, &copy_cmd);
                copy_cmd.menu.menuRef = GetApplicationDockTileMenu();
                SetEventParameter(copy, kEventParamDirectObject, typeHICommand, sizeof(copy_cmd), &copy_cmd);
                if(SendEventToMenu(copy, copy_cmd.menu.menuRef) == noErr)
                    handled_event = true;
            }
            if(!handled_event) {
                if(cmd.commandID == kHICommandQuit) {
                    handled_event = true;
                    HiliteMenu(0);
                    if (!QApplicationPrivate::modalState()) {
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
                    handled_event = true;
                }
            }
        }
        break;
    }

    // ok we clear all QtRequestContext events from the queue
    if(remove_context_timer) {
        if(mac_context_timer) {
            RemoveEventLoopTimer(mac_context_timer);
            mac_context_timer = 0;
        }
        if(request_context_hold_pending)
            qt_mac_event_remove(request_context_hold_pending);
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

OSStatus QApplicationPrivate::globalAppleEventProcessor(const AppleEvent *ae, AppleEvent *, long handlerRefcon)
{
    QApplication *app = (QApplication *)handlerRefcon;
    bool handled_event=false;
    OSType aeID=typeWildCard, aeClass=typeWildCard;
    AEGetAttributePtr(ae, keyEventClassAttr, typeType, 0, &aeClass, sizeof(aeClass), 0);
    AEGetAttributePtr(ae, keyEventIDAttr, typeType, 0, &aeID, sizeof(aeID), 0);
    if(aeClass == kCoreEventClass) {
        switch(aeID) {
        case kAEQuitApplication: {
            if(!QApplicationPrivate::modalState() && IsMenuCommandEnabled(NULL, kHICommandQuit)) {
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
    \fn bool QApplication::macEventFilter(EventHandlerCallRef caller, EventRef event)

    \warning This virtual function is only implemented under Mac OS X.

    If you create an application that inherits QApplication and
    reimplement this function, you get direct access to all Carbon
    Events that are received from Mac OS X with this function being
    called with the \a caller and the \a event.

    Return true if you want to stop the event from being processed.
    Return false for normal event dispatching. The default
    implementation returns false.
*/
bool QApplication::macEventFilter(EventHandlerCallRef, EventRef)
{
    return false;
}

/*!
    \internal
*/
void QApplicationPrivate::openPopup(QWidget *popup)
{
    if(!QApplicationPrivate::popupWidgets)                        // create list
        QApplicationPrivate::popupWidgets = new QWidgetList;
    QApplicationPrivate::popupWidgets->append(popup);                // add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    if(popup->focusWidget()) {
        popup->focusWidget()->setFocus(Qt::PopupFocusReason);
    } else if (QApplicationPrivate::popupWidgets->count() == 1) { // this was the first popup
        if (QWidget *fw = QApplication::focusWidget()) {
            QFocusEvent e(QEvent::FocusOut, Qt::PopupFocusReason);
            q->sendEvent(fw, &e);
        }
    }
}

/*!
    \internal
*/
void QApplicationPrivate::closePopup(QWidget *popup)
{
    if(!QApplicationPrivate::popupWidgets)
        return;

    QApplicationPrivate::popupWidgets->removeAll(popup);
    if(popup == popupOfPopupButtonFocus) {
        popupButtonFocus = 0;
        popupOfPopupButtonFocus = 0;
    }
    if(popup == qt_button_down)
        qt_button_down = 0;
    if(QApplicationPrivate::popupWidgets->isEmpty()) {  // this was the last popup
        popupCloseDownMode = true;                      // control mouse events
        delete QApplicationPrivate::popupWidgets;
        QApplicationPrivate::popupWidgets = 0;
        if (QApplicationPrivate::active_window) {
            if (QWidget *fw = QApplicationPrivate::active_window->focusWidget()) {
                if (fw != QApplication::focusWidget()) {
                    fw->setFocus(Qt::PopupFocusReason);
                } else {
                    QFocusEvent e(QEvent::FocusIn, Qt::PopupFocusReason);
                    q->sendEvent(fw, &e);
                }
            }
        }
    } else {
        // popups are not focus-handled by the window system (the
        // first popup grabbed the keyboard), so we have to do that
        // manually: A popup was closed, so the previous popup gets
        // the focus.
        QApplicationPrivate::active_window = QApplicationPrivate::popupWidgets->last();
        if (QWidget *fw = QApplicationPrivate::active_window->focusWidget())
            fw->setFocus(Qt::PopupFocusReason);
    }
}

void  QApplication::setCursorFlashTime(int msecs)
{
    QApplicationPrivate::cursor_flash_time = msecs;
}

int QApplication::cursorFlashTime()
{
    return QApplicationPrivate::cursor_flash_time;
}

void QApplication::setDoubleClickInterval(int ms)
{
    qt_mac_dblclick.use_qt_time_limit = true;
    QApplicationPrivate::mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    if(!qt_mac_dblclick.use_qt_time_limit) { //get it from the system
        QSettings appleSettings(QLatin1String("apple.com"));
        /* First worked as of 10.3.3 */
        double dci = appleSettings.value(QLatin1String("com.apple.mouse.doubleClickThreshold"), 0.5).toDouble();
        return int(dci * 1000);
    }
    return QApplicationPrivate::mouse_double_click_time;
}

void QApplication::setKeyboardInputInterval(int ms)
{
    QApplicationPrivate::keyboard_input_time = ms;
}

int QApplication::keyboardInputInterval()
{
    // FIXME: get from the system
    return QApplicationPrivate::keyboard_input_time;
}

void QApplication::setWheelScrollLines(int n)
{
    qt_mac_use_qt_scroller_lines = true;
    QApplicationPrivate::wheel_scroll_lines = n;
}

int QApplication::wheelScrollLines()
{
    if(!qt_mac_use_qt_scroller_lines) {
        /* First worked as of 10.3.3 */
        QSettings appleSettings(QLatin1String("apple.com"));
        double scroll = appleSettings.value(QLatin1String("com.apple.scrollwheel.scaling"),
                                           (QApplicationPrivate::wheel_scroll_lines)).toDouble();
        return scroll ? int(3 * scroll) : 1;
    }
    return QApplicationPrivate::wheel_scroll_lines;
}

void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
    switch (effect) {
    case Qt::UI_FadeMenu:
        QApplicationPrivate::fade_menu = enable;
        if(!enable)
            break;
    case Qt::UI_AnimateMenu:
        QApplicationPrivate::animate_menu = enable;
        break;
    case Qt::UI_FadeTooltip:
        QApplicationPrivate::fade_tooltip = enable;
        if(!enable)
            break;
    case Qt::UI_AnimateTooltip:
        QApplicationPrivate::animate_tooltip = enable;
        break;
    case Qt::UI_AnimateCombo:
        QApplicationPrivate::animate_combo = enable;
        break;
    case Qt::UI_AnimateToolBox:
        QApplicationPrivate::animate_toolbox = enable;
        break;
    default:
        QApplicationPrivate::animate_ui = enable;
        break;
    }
}

bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
    if (QColormap::instance().depth() < 16 || !QApplicationPrivate::animate_ui)
        return false;

    switch(effect) {
    case Qt::UI_AnimateMenu:
        return QApplicationPrivate::animate_menu;
    case Qt::UI_FadeMenu:
        return QApplicationPrivate::fade_menu;
    case Qt::UI_AnimateCombo:
        return QApplicationPrivate::animate_combo;
    case Qt::UI_AnimateTooltip:
        return QApplicationPrivate::animate_tooltip;
    case Qt::UI_FadeTooltip:
        return QApplicationPrivate::fade_tooltip;
    case Qt::UI_AnimateToolBox:
        return QApplicationPrivate::animate_toolbox;
    default:
        break;
    }
    return QApplicationPrivate::animate_ui;
}

/*!
    \internal
*/
bool QApplicationPrivate::qt_mac_apply_settings()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));

    /*
      Qt settings.  This is how they are written into the datastream.
      Palette/ *             - QPalette
      font                   - QFont
      libraryPath            - QStringList
      style                  - QString
      doubleClickInterval    - int
      cursorFlashTime        - int
      wheelScrollLines       - int
      colorSpec              - QString
      defaultCodec           - QString
      globalStrut/width      - int
      globalStrut/height     - int
      GUIEffects             - QStringList
      Font Substitutions/ *  - QStringList
      Font Substitutions/... - QStringList
    */

    // read library (ie. plugin) path list
    QString libpathkey =
        QString(QLatin1String("%1.%2/libraryPath"))
                    .arg(QT_VERSION >> 16)
                    .arg((QT_VERSION & 0xff00) >> 8);
    QStringList pathlist = settings.value(libpathkey).toString().split(QLatin1Char(':'));
    if(!pathlist.isEmpty()) {
        QStringList::ConstIterator it = pathlist.begin();
        while(it != pathlist.end())
            QApplication::addLibraryPath(*it++);
    }

    QString defaultcodec = settings.value(QLatin1String("defaultCodec"), QVariant(QLatin1String("none"))).toString();
    if(defaultcodec != QLatin1String("none")) {
        QTextCodec *codec = QTextCodec::codecForName(defaultcodec.toLatin1().constData());
        if(codec)
            QTextCodec::setCodecForTr(codec);
    }

    if(qt_is_gui_used) {
        QString str;
        QStringList strlist;
        int num;

        // read new palette
        int i;
        QPalette pal(QApplication::palette());
        strlist = settings.value(QLatin1String("Palette/active")).toStringList();
        if (strlist.count() == QPalette::NColorRoles) {
            for (i = 0; i < QPalette::NColorRoles; i++)
                pal.setColor(QPalette::Active, (QPalette::ColorRole) i,
                            QColor(strlist[i]));
        }
        strlist = settings.value(QLatin1String("Palette/inactive")).toStringList();
        if (strlist.count() == QPalette::NColorRoles) {
            for (i = 0; i < QPalette::NColorRoles; i++)
                pal.setColor(QPalette::Inactive, (QPalette::ColorRole) i,
                            QColor(strlist[i]));
        }
        strlist = settings.value(QLatin1String("Palette/disabled")).toStringList();
        if (strlist.count() == QPalette::NColorRoles) {
            for (i = 0; i < QPalette::NColorRoles; i++)
                pal.setColor(QPalette::Disabled, (QPalette::ColorRole) i,
                            QColor(strlist[i]));
        }

        if(pal != QApplication::palette())
            QApplication::setPalette(pal);

        // read new font
        QFont font(QApplication::font());
        str = settings.value(QLatin1String("font")).toString();
        if (!str.isEmpty()) {
            font.fromString(str);
            if (font != QApplication::font())
                QApplication::setFont(font);
        }

        // read new QStyle
        QString stylename = settings.value(QLatin1String("style")).toString();
        if(! stylename.isNull() && ! stylename.isEmpty()) {
            QStyle *style = QStyleFactory::create(stylename);
            if(style)
                QApplication::setStyle(style);
            else
                stylename = QLatin1String("default");
        } else {
            stylename = QLatin1String("default");
        }

        num = settings.value(QLatin1String("doubleClickInterval"),
                            QApplication::doubleClickInterval()).toInt();
        QApplication::setDoubleClickInterval(num);

        num = settings.value(QLatin1String("cursorFlashTime"),
                            QApplication::cursorFlashTime()).toInt();
        QApplication::setCursorFlashTime(num);

        num = settings.value(QLatin1String("wheelScrollLines"),
                            QApplication::wheelScrollLines()).toInt();
        QApplication::setWheelScrollLines(num);

        QString colorspec = settings.value(QLatin1String("colorSpec"),
                                            QVariant(QLatin1String("default"))).toString();
        if (colorspec == QLatin1String("normal"))
            QApplication::setColorSpec(QApplication::NormalColor);
        else if (colorspec == QLatin1String("custom"))
            QApplication::setColorSpec(QApplication::CustomColor);
        else if (colorspec == QLatin1String("many"))
            QApplication::setColorSpec(QApplication::ManyColor);
        else if (colorspec != QLatin1String("default"))
            colorspec = QLatin1String("default");

        int w = settings.value(QLatin1String("globalStrut/width")).toInt();
        int h = settings.value(QLatin1String("globalStrut/height")).toInt();
        QSize strut(w, h);
        if (strut.isValid())
            QApplication::setGlobalStrut(strut);

        QStringList effects = settings.value(QLatin1String("GUIEffects")).toStringList();
        if(!effects.isEmpty()) {
            if(effects.contains(QLatin1String("none")))
                QApplication::setEffectEnabled(Qt::UI_General, false);
            if(effects.contains(QLatin1String("general")))
                QApplication::setEffectEnabled(Qt::UI_General, true);
            if(effects.contains(QLatin1String("animatemenu")))
                QApplication::setEffectEnabled(Qt::UI_AnimateMenu, true);
            if(effects.contains(QLatin1String("fademenu")))
                QApplication::setEffectEnabled(Qt::UI_FadeMenu, true);
            if(effects.contains(QLatin1String("animatecombo")))
                QApplication::setEffectEnabled(Qt::UI_AnimateCombo, true);
            if(effects.contains(QLatin1String("animatetooltip")))
                QApplication::setEffectEnabled(Qt::UI_AnimateTooltip, true);
            if(effects.contains(QLatin1String("fadetooltip")))
                QApplication::setEffectEnabled(Qt::UI_FadeTooltip, true);
            if(effects.contains(QLatin1String("animatetoolbox")))
                QApplication::setEffectEnabled(Qt::UI_AnimateToolBox, true);
        } else {
            QApplication::setEffectEnabled(Qt::UI_General, false);
        }

        settings.beginGroup(QLatin1String("Font Substitutions"));
        QStringList fontsubs = settings.childGroups();
        if (!fontsubs.isEmpty()) {
            QStringList::Iterator it = fontsubs.begin();
            for (; it != fontsubs.end(); ++it) {
                QString fam = (*it).toLatin1();
                QStringList subs = settings.value(fam).toStringList();
                QFont::insertSubstitutions(fam, subs);
            }
        }
        settings.endGroup();
    }

    settings.endGroup();
    return true;
}

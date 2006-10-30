/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
#include "qlayout.h"
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
#include "qdir.h"
#include "qtooltip.h"
#include "qdebug.h"
#include "private/qmacinputcontext_p.h"
#include "private/qpaintengine_mac_p.h"
#include "private/qcursor_p.h"
#include "private/qapplication_p.h"
#include "private/qcolor_p.h"
#include "private/qwidget_p.h"
#include "private/qkeymapper_p.h"
#include "qeventdispatcher_mac_p.h"

#ifndef QT_NO_ACCESSIBILITY
#  include "qaccessible.h"
#endif

#ifndef QT_NO_THREAD
#  include "qmutex.h"
#endif

#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>

/*****************************************************************************
  QApplication debug facilities
 *****************************************************************************/
//#define DEBUG_EVENTS [like EventDebug but more specific to Qt]
//#define DEBUG_DROPPED_EVENTS
//#define DEBUG_MOUSE_MAPS
//#define DEBUG_MODAL_EVENTS
//#define DEBUG_PLATFORM_SETTINGS

#define QMAC_SPEAK_TO_ME
#ifdef QMAC_SPEAK_TO_ME
#include "qregexp.h"
#endif

#ifndef kThemeBrushAlternatePrimaryHighlightColor
#define kThemeBrushAlternatePrimaryHighlightColor -5
#endif

//for qt_mac.h
QPaintDevice *qt_mac_safe_pdev = 0;
QList<QMacWindowChangeEvent*> *QMacWindowChangeEvent::change_events = 0;

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static struct {
    bool use_qt_time_limit;
    QWidget *last_widget;
    int last_x, last_y;
    int last_modifiers, last_button;
    EventTime last_time;
} qt_mac_dblclick = { false, 0, -1, -1, 0, 0, -2 };

struct qt_mac_enum_mapper
{
    int mac_code;
    int qt_code;
#if defined(DEBUG_MOUSE_MAPS)
#   define QT_MAC_MAP_ENUM(x) x, #x
    const char *desc;
#else
#   define QT_MAC_MAP_ENUM(x) x
#endif
};

// tablet structure
static QTabletEvent::PointerType currPointerType = QTabletEvent::UnknownPointer;
static QTabletEvent::TabletDevice currTabletDevice = QTabletEvent::NoDevice;
static qint64 tabletUniqueID = 0;
static UInt32 tabletCaps = 0;

static int tablet_button_state = 0;
static bool app_do_modal = false;       // modal mode
extern QWidgetList *qt_modal_stack;     // stack of modal widgets
extern bool qt_mac_in_drag;             // from qdnd_mac.cpp
extern bool qt_tab_all_widgets;         // from qapplication.cpp
extern bool qt_app_has_font;
bool qt_mac_app_fullscreen = false;
bool qt_scrollbar_jump_to_pos = false;
static bool qt_mac_collapse_on_dblclick = true;
extern int qt_antialiasing_threshold; // from qapplication.cpp
QPointer<QWidget> qt_button_down;                // widget got last button-down
static QPointer<QWidget> qt_mouseover;
#if defined(QT_DEBUG)
static bool        appNoGrab        = false;        // mouse/keyboard grabbing
#endif
static bool qt_mac_press_and_hold_context = false;
static EventLoopTimerRef mac_context_timer = 0;
static EventLoopTimerUPP mac_context_timerUPP = 0;
static EventHandlerRef app_proc_handler = 0;
static EventHandlerUPP app_proc_handlerUPP = 0;
static AEEventHandlerUPP app_proc_ae_handlerUPP = NULL;

/*****************************************************************************
  External functions
 *****************************************************************************/
extern Qt::KeyboardModifiers qt_mac_get_modifiers(int keys); //qkeymapper_mac.cpp
extern bool qt_mac_can_clickThrough(const QWidget *); //qwidget_mac.cpp
extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
extern WindowPtr qt_mac_window_for(const QWidget*); //qwidget_mac.cpp
extern QWidget *qt_mac_find_window(WindowPtr); //qwidget_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const QPoint &); //qcursor_mac.cpp
extern bool qt_mac_is_macsheet(const QWidget *); //qwidget_mac.cpp
extern QString qt_mac_from_pascal_string(const Str255); //qglobal.cpp
extern void qt_mac_command_set_enabled(MenuRef, UInt32, bool); //qmenu_mac.cpp

/* Resolution change magic */
static void qt_mac_display_change_callbk(CGDirectDisplayID, CGDisplayChangeSummaryFlags flags, void *)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
    const bool resized = flags & kCGDisplayDesktopShapeChangedFlag;
#else
    Q_UNUSED(flags);
    const bool resized = true;
#endif
    if(resized && qApp) {
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
    const char *const roles[] = { "WindowText", "Button", "Light", "Midlight", "Dark", "Mid",
                            "Text", "BrightText", "ButtonText", "Base", "Window", "Shadow",
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
        if(wp) {
            *w = qt_mac_find_window(wp);
#if 0
            if(!*w)
                qWarning("QApplication: qt_mac_window_at: Couldn't find %d",(int)wp);
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
        CGImageRef img = (CGImageRef)pixmap.macCGHandle();
        SetApplicationDockTileImage(img);
        CGImageRelease(img);
    }
}

Q_GUI_EXPORT void qt_mac_set_press_and_hold_context(bool b) { qt_mac_press_and_hold_context = b; }

//mouse buttons
static qt_mac_enum_mapper qt_mac_mouse_symbols[] = {
    { kEventMouseButtonPrimary, QT_MAC_MAP_ENUM(Qt::LeftButton) },
    { kEventMouseButtonSecondary, QT_MAC_MAP_ENUM(Qt::RightButton) },
    { kEventMouseButtonTertiary, QT_MAC_MAP_ENUM(Qt::MidButton) },
    { 0, QT_MAC_MAP_ENUM(0) }
};
static Qt::MouseButtons qt_mac_get_buttons(int buttons)
{
#ifdef DEBUG_MOUSE_MAPS
    qDebug("Qt: internal: **Mapping buttons: %d (0x%04x)", buttons, buttons);
#endif
    Qt::MouseButtons ret = Qt::NoButton;
    for(int i = 0; qt_mac_mouse_symbols[i].qt_code; i++) {
        if(buttons & (0x01<<(qt_mac_mouse_symbols[i].mac_code-1))) {
#ifdef DEBUG_MOUSE_MAPS
            qDebug("Qt: internal: got button: %s", qt_mac_mouse_symbols[i].desc);
#endif
            ret |= Qt::MouseButtons(qt_mac_mouse_symbols[i].qt_code);
        }
    }
    return ret;
}
static Qt::MouseButton qt_mac_get_button(EventMouseButton button)
{
#ifdef DEBUG_MOUSE_MAPS
    qDebug("Qt: internal: **Mapping button: %d (0x%04x)", button, button);
#endif
    Qt::MouseButtons ret = 0;
    for(int i = 0; qt_mac_mouse_symbols[i].qt_code; i++) {
        if(button == qt_mac_mouse_symbols[i].mac_code) {
#ifdef DEBUG_MOUSE_MAPS
            qDebug("Qt: internal: got button: %s", qt_mac_mouse_symbols[i].desc);
#endif
            return Qt::MouseButton(qt_mac_mouse_symbols[i].qt_code);
        }
    }
    return Qt::NoButton;
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

    // Anti-aliasing threshold
    appleValue = appleSettings.value(QLatin1String("AppleAntiAliasingThreshold"));
    if (appleValue.isValid())
        qt_antialiasing_threshold = appleValue.toInt();

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
            pal.setColor(QPalette::Active, QPalette::WindowText, qc);
            pal.setColor(QPalette::Active, QPalette::HighlightedText, qc);
        }
        if(!GetThemeTextColor(kThemeTextColorDialogInactive, 32, true, &c)) {
            qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
            pal.setColor(QPalette::Inactive, QPalette::Text, qc);
            pal.setColor(QPalette::Inactive, QPalette::WindowText, qc);
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText, qc);
            pal.setColor(QPalette::Disabled, QPalette::Text, qc);
            pal.setColor(QPalette::Disabled, QPalette::WindowText, qc);
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
        struct FontMap {
            FontMap(const char *qc, short fk) : qt_class(qc), font_key(fk) { }
            const char *const qt_class;
            short font_key;
        } mac_widget_fonts[] = {
            FontMap("QPushButton", kThemePushButtonFont),
            FontMap("QListView", kThemeViewsFont),
            FontMap("QListBox", kThemeViewsFont),
            FontMap("QTitleBar", kThemeWindowTitleFont),
            FontMap("QMenuBar", kThemeMenuTitleFont),
            FontMap("QMenu", kThemeMenuItemFont),
            FontMap("QComboMenuItem", kThemeSystemFont),
            FontMap("QHeaderView", kThemeSmallSystemFont),
            FontMap("Q3Header", kThemeSmallSystemFont),
            FontMap("QTipLabel", kThemeSmallSystemFont),
            FontMap("QLabel", kThemeSystemFont),
            FontMap("QToolButton", kThemeSmallSystemFont),
            FontMap("QMenuItem", kThemeMenuItemCmdKeyFont),  // It doesn't exist, but its unique.
            FontMap("QComboLineEdit", kThemeViewsFont),  // It doesn't exist, but its unique.
            FontMap("QMiniPushButton", kThemeMiniSystemFont),  // It doesn't exist, but its unique.
            FontMap(0, 0) };
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
    QApplicationPrivate::initializeWidgetPaletteHash();
#ifdef DEBUG_PLATFORM_SETTINGS
    qDebug("qt_mac_update_os_settings END !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#endif
}

void QApplicationPrivate::initializeWidgetPaletteHash()
{
    { //setup the palette
        struct PaletteMap {
            inline PaletteMap(const char *qc, ThemeBrush a, ThemeBrush i) :
                qt_class(qc), active(a), inactive(i) { }
            const char *const qt_class;
            ThemeBrush active, inactive;
        } mac_widget_colors[] = {
            PaletteMap("QToolButton", kThemeTextColorBevelButtonActive, kThemeTextColorBevelButtonInactive),
            PaletteMap("QAbstractButton", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive),
            PaletteMap("QHeaderView", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive),
            PaletteMap("Q3Header", kThemeTextColorPushButtonActive, kThemeTextColorPushButtonInactive),
            PaletteMap("QComboBox", kThemeTextColorPopupButtonActive, kThemeTextColorPopupButtonInactive),
            PaletteMap("QAbstractItemView", kThemeTextColorListView, kThemeTextColorDialogInactive),
            PaletteMap("QMessageBoxLabel", kThemeTextColorAlertActive, kThemeTextColorAlertInactive),
            PaletteMap("QTabBar", kThemeTextColorTabFrontActive, kThemeTextColorTabFrontInactive),
            PaletteMap("QLabel", kThemeTextColorPlacardActive, kThemeTextColorPlacardInactive),
            PaletteMap("QGroupBox", kThemeTextColorPlacardActive, kThemeTextColorPlacardInactive),
            PaletteMap("QMenu", kThemeTextColorPopupLabelActive, kThemeTextColorPopupLabelInactive),
            PaletteMap("QTextEdit", 0, 0),
            PaletteMap("QTextControl", 0, 0),
            PaletteMap("QToolTip", 0, 0),
            PaletteMap("QLineEdit", 0, 0),
            PaletteMap(0, 0, 0) };
        QColor qc;
        RGBColor c;
        for(int i = 0; mac_widget_colors[i].qt_class; i++) {
            QPalette pal;
            if (mac_widget_colors[i].active != 0) {
                if(!GetThemeTextColor(mac_widget_colors[i].active, 32, true, &c)) {
                    qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
                    pal.setColor(QPalette::Active, QPalette::Text, qc);
                    pal.setColor(QPalette::Active, QPalette::WindowText, qc);
                    pal.setColor(QPalette::Active, QPalette::HighlightedText, qc);
                }
                if(!GetThemeTextColor(mac_widget_colors[i].inactive, 32, true, &c)) {
                    qc = QColor(c.red / 256, c.green / 256, c.blue / 256);
                    pal.setColor(QPalette::Inactive, QPalette::Text, qc);
                    pal.setColor(QPalette::Disabled, QPalette::Text, qc);
                    pal.setColor(QPalette::Inactive, QPalette::WindowText, qc);
                    pal.setColor(QPalette::Disabled, QPalette::WindowText, qc);
                    pal.setColor(QPalette::Inactive, QPalette::HighlightedText, qc);
                    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, qc);
                }
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
            } else if (!strcmp(mac_widget_colors[i].qt_class, "QAbstractItemView")) {
                if(!GetThemeBrushAsColor(kThemeBrushAlternatePrimaryHighlightColor, 32, true, &c))
                    pal.setBrush(QPalette::Active, QPalette::Highlight,
                            QColor(c.red / 256, c.green / 256, c.blue / 256));
                GetThemeTextColor(kThemeTextColorMenuItemSelected, 32, true, &c);
                pal.setBrush(QPalette::Active, QPalette::HighlightedText, QColor(c.red / 256, c.green / 256, c.blue / 256));
#if 1
                pal.setBrush(QPalette::Inactive, QPalette::Text,
                              pal.brush(QPalette::Active, QPalette::Text));
                pal.setBrush(QPalette::Inactive, QPalette::HighlightedText,
                              pal.brush(QPalette::Active, QPalette::Text));
#endif
            } else if (!strcmp(mac_widget_colors[i].qt_class, "QTextEdit")
                       || !strcmp(mac_widget_colors[i].qt_class, "QTextControl")) {
                pal.setBrush(QPalette::Inactive, QPalette::Text,
                              pal.brush(QPalette::Active, QPalette::Text));
                pal.setBrush(QPalette::Inactive, QPalette::HighlightedText,
                              pal.brush(QPalette::Active, QPalette::Text));
            } else if (!strcmp(mac_widget_colors[i].qt_class, "QLineEdit")) {
                pal.setBrush(QPalette::Disabled, QPalette::Base,
                             pal.brush(QPalette::Active, QPalette::Base));
            } else if (!strcmp(mac_widget_colors[i].qt_class, "QToolTip")) {
                pal.setBrush(QPalette::Window, QColor(255, 255, 199));
                QToolTip::setPalette(pal);
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
Q_GUI_EXPORT void qt_event_request_window_change()
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
bool qt_event_remove_window_change()
{
    if(request_window_change_pending) {
        if (IsEventInQueue(GetMainEventQueue(), request_window_change_pending))
            RemoveEventFromQueue(GetMainEventQueue(), request_window_change_pending);
        qt_mac_event_release(request_window_change_pending);
        return true;
    }
    return false;
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
        Qt::WindowType wt = tlw->windowType();
        if(tlw->isVisible()
               && ((wt != Qt::Desktop && wt != Qt::Popup && wt != Qt::Tool) || tlw->isModal())) {
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
    Q_Q(QApplication);
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
    AESend(&ae, &reply, kAENoReply, kAENormalPriority, kAEDefaultTimeout, 0, 0);
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

    { kEventClassWindow, kEventWindowActivated },
    { kEventClassWindow, kEventWindowDeactivated },

    { kEventClassMouse, kEventMouseWheelMoved },
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseMoved },

    { kEventClassTablet, kEventTabletProximity },

    { kEventClassApplication, kEventAppActivated },
    { kEventClassApplication, kEventAppDeactivated },

//     { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
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

QString QApplicationPrivate::appName() const
{
    static QString applName;
    if (applName.isEmpty()) {
        ProcessSerialNumber psn;
        if (qt_is_gui_used && GetCurrentProcess(&psn) == noErr) {
            QCFString cfstr;
            CopyProcessName(&psn, &cfstr);
            applName = cfstr;
        } else {
            applName = QCoreApplicationPrivate::appName();
        }
    }
    return applName;
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
        CGDisplayRegisterReconfigurationCallback(qt_mac_display_change_callbk, 0);
        ProcessSerialNumber psn;
        if(GetCurrentProcess(&psn) == noErr) {
            TransformProcessType(&psn, kProcessTransformToForegroundApplication);
            SetFrontProcess(&psn);
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
            if(arg == QLatin1String("-nograb"))
                appNoGrab = !appNoGrab;
            else
#endif // QT_DEBUG
                if(arg.left(5) == QLatin1String("-psn_")) {
                    passed_psn = arg.mid(6);
                } else {
                    argv[j++] = argv[i];
                }
        }
        if(j < priv->argc) {
            priv->argv[j] = 0;
            priv->argc = j;
        }

        //special hack to change working directory (for an app bundle) when running from finder
        if(!passed_psn.isNull() && QDir::currentPath() == QLatin1String("/")) {
            QCFType<CFURLRef> bundleURL(CFBundleCopyBundleURL(CFBundleGetMainBundle()));
            QString qbundlePath = QCFString(CFURLCopyFileSystemPath(bundleURL,
                                            kCFURLPOSIXPathStyle));
            if(qbundlePath.endsWith(".app"))
                QDir::setCurrent(qbundlePath.section('/', 0, -2));
        }
    }

    QMacPasteboardMime::initialize();

    qApp->setObjectName(priv->appName());
    if(qt_is_gui_used) {
        QColormap::initialize();
        QFont::initialize();
        QCursorData::initialize();
        QCoreGraphicsPaintEngine::initialize();
#ifndef QT_NO_ACCESSIBILITY
        QAccessible::initialize();
#endif
        QMacInputContext::initialize();
        QApplicationPrivate::inputContext = new QMacInputContext;

#ifndef __LP64__
        RegisterAppearanceClient();
#endif
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
                                      app_proc_ae_handlerUPP, SRefCon(qApp), true);
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
    CGDisplayRemoveReconfigurationCallback(qt_mac_display_change_callbk, 0);
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
    qApp->d_func()->cursor_list.prepend(cursor);

    if(qApp && qApp->activeWindow())
        qt_mac_set_cursor(&qApp->d_func()->cursor_list.first(), QCursor::pos());
}

void QApplication::restoreOverrideCursor()
{
    if (qApp->d_func()->cursor_list.isEmpty())
        return;
    qApp->d_func()->cursor_list.removeFirst();

    if(qApp && qApp->activeWindow()) {
        const QCursor def(Qt::ArrowCursor);
        qt_mac_set_cursor(qApp->d_func()->cursor_list.isEmpty() ? &def : &qApp->d_func()->cursor_list.first(), QCursor::pos());
    }
}
#endif

QWidget *QApplication::topLevelAt(const QPoint &p)
{
    QWidget *widget;
    qt_mac_window_at(p.x(), p.y(), &widget);
    return widget;
}

static QWidget *qt_mac_recursive_widgetAt(QWidget *widget, int x, int y)
{
    if(!widget)
	return 0;
    const QObjectList kids = widget->children();
    for(int i = kids.size()-1; i >= 0; --i) {
	if(QWidget *kid = qobject_cast<QWidget*>(kids.at(i))) {
	    if(kid->isVisible() && !kid->isTopLevel() &&
               !kid->testAttribute(Qt::WA_TransparentForMouseEvents)) {
		const int wx=kid->x(), wy=kid->y(),
                         wx2=wx+kid->width(), wy2=wy+kid->height();
		if(x >= wx && y >= wy && x < wx2 && y < wy2) {
                    const QRegion mask = kid->mask();
		    if(!mask.isEmpty() && !mask.contains(QPoint(x-wx, y-wy)))
			continue;
		    return qt_mac_recursive_widgetAt(kid, x-wx, y-wy);
		}
	    }
	}
    }
    return widget;
}

void QApplication::beep()
{
#ifndef __LP64__
    SysBeep(0);
#endif
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/


bool QApplicationPrivate::modalState()
{
    return app_do_modal;
}

void QApplicationPrivate::enterModal_sys(QWidget *widget)
{
#ifdef DEBUG_MODAL_EVENTS
    Q_ASSERT(widget);
    qDebug("Entering modal state with %s::%s::%p (%d)", widget->metaObject()->className(), widget->objectName().toLocal8Bit().constData(),
           widget, qt_modal_stack ? (int)qt_modal_stack->count() : -1);
#endif
    if(!qt_modal_stack)
        qt_modal_stack = new QWidgetList;

    dispatchEnterLeave(0, qt_mouseover);
    qt_mouseover = 0;

    qt_modal_stack->insert(0, widget);
    if(!app_do_modal)
        qt_event_request_menubarupdate();
    app_do_modal = true;
}

void QApplicationPrivate::leaveModal_sys(QWidget *widget)
{
    if(qt_modal_stack && qt_modal_stack->removeAll(widget)) {
#ifdef DEBUG_MODAL_EVENTS
        qDebug("Leaving modal state with %s::%s::%p (%d)", widget->metaObject()->className(), widget->objectName().toLocal8Bit().constData(),
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
    else qDebug("Failure to remove %s::%s::%p -- %p", widget->metaObject()->className(), widget->objectName().toLocal8Bit().constData(), widget, qt_modal_stack);
#endif
    app_do_modal = (qt_modal_stack != 0);
    if(!app_do_modal)
        qt_event_request_menubarupdate();
}

QWidget *QApplicationPrivate::tryModalHelper_sys(QWidget *top) {
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

    UInt32 /*ekind = GetEventKind(event), */eclass=GetEventClass(event);
    switch(eclass) {
    case kEventClassMouse:
        if(!top->isActiveWindow())
            top->activateWindow();
        block_event = true;
        break;
    case kEventClassKeyboard:
        block_event = true;
        break;
    }

    if((!QApplication::activeWindow() || QApplicationPrivate::isBlockedByModal(QApplication::activeWindow())) &&
       top->isWindow() && block_event)
        top->raise();

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
                if(ekind == kEventQtRequestSelect)
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
                bool just_show = !qt_mac_is_macsheet(widget);
                if(!just_show) {
                    OSStatus err = ShowSheetWindow(window, qt_mac_window_for(widget->parentWidget()));
                    if(err != noErr)
                        qWarning("Qt: QWidget: Unable to show as sheet %s::%s [%ld]", widget->metaObject()->className(),
                                 widget->objectName().toLocal8Bit().constData(), long(err));
                    just_show = true;
                }
                if(just_show) //at least the window will be visible, but the sheet flag doesn't work sadly (probalby too many sheets)
                    ShowHide(window, true);
            }
        } else if(ekind == kEventQtRequestWindowChange) {
            qt_mac_event_release(request_window_change_pending);
            QMacWindowChangeEvent::exec(false);
        } else if(ekind == kEventQtRequestMenubarUpdate) {
            qt_mac_event_release(request_menubarupdate_pending);
            QMenuBar::macUpdateMenuBar();
        } else if(ekind == kEventQtRequestSelect) {
            qt_mac_event_release(request_select_pending);
            QEventDispatcherMac *l = 0;
            GetEventParameter(event, kEventParamQEventDispatcherMac, typeQEventDispatcherMac, 0, sizeof(l), 0, &l);
            timeval tm;
            memset(&tm, '\0', sizeof(tm));
            l->d_func()->doSelect(QEventLoop::AllEvents, &tm);
        } else if(ekind == kEventQtRequestActivate) {
            qt_mac_event_release(request_activate_pending.event);
            if(request_activate_pending.widget) {
                QWidget *tlw = request_activate_pending.widget->window();
                request_activate_pending.widget = 0;
                tlw->activateWindow();
                SelectWindow(qt_mac_window_for(tlw));
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
                        qt_mac_dblclick.last_widget = 0;
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
            switch (proxRec.vendorPointerType & 0x0F06) {
            case 0x0802:
                currTabletDevice = QTabletEvent::Stylus;
                break;
            case 0x0902:
                currTabletDevice = QTabletEvent::Airbrush;
                break;
            case 0x0004:
                currTabletDevice = QTabletEvent::FourDMouse;
                break;
            case 0x0006:
                currTabletDevice = QTabletEvent::Puck;
                break;
            case 0x0804:
                currTabletDevice = QTabletEvent::RotationStylus;
                break;
            default:
                currTabletDevice = QTabletEvent::NoDevice;
            }
            QTabletEvent tabletProximity(proxRec.enterProximity ? QEvent::TabletEnterProximity : QEvent::TabletLeaveProximity,
                                         QPoint(), QPoint(), QPointF(), currTabletDevice, currPointerType, 0, 0, 0, 0, 0, 0,
                                         0, tabletUniqueID);
            QApplication::sendSpontaneousEvent(qApp, &tabletProximity);
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
            modifiers = qt_mac_get_modifiers(mac_modifiers);
        }
        Qt::MouseButtons buttons;
        {
            UInt32 mac_buttons = 0;
            GetEventParameter(event, kEventParamMouseChord, typeUInt32, 0,
                              sizeof(mac_buttons), 0, &mac_buttons);
            buttons = qt_mac_get_buttons(mac_buttons);
        }
        int wheel_delta=0;
        if(ekind == kEventMouseWheelMoved) {
            int mdelt = 0;
            GetEventParameter(event, kEventParamMouseWheelDelta, typeSInt32, 0,
                              sizeof(mdelt), 0, &mdelt);
            wheel_delta = mdelt * 120;
        }

        Qt::MouseButton button = Qt::NoButton;
        if(ekind == kEventMouseDown || ekind == kEventMouseUp) {
            EventMouseButton mac_button = 0;
            GetEventParameter(event, kEventParamMouseButton, typeMouseButton, 0,
                              sizeof(mac_button), 0, &mac_button);
            button = qt_mac_get_button(mac_button);
        }

        switch(ekind) {
        case kEventMouseDown:
            etype = QEvent::MouseButtonPress;
            break;
        case kEventMouseUp:
            etype = QEvent::MouseButtonRelease;
            break;
        case kEventMouseDragged:
        case kEventMouseMoved:
            etype = QEvent::MouseMove;
            break;
        }
        //figure out which widget to send it to
        if(app->d_func()->inPopupMode()) {
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
        if(qt_mac_find_window((FrontWindow()))) { //set the cursor up
            QCursor cursor(Qt::ArrowCursor);
            QWidget *cursor_widget = widget;
            if(cursor_widget && cursor_widget == qt_button_down && ekind == kEventMouseUp)
                cursor_widget = QApplication::widgetAt(where.h, where.v);
            if(cursor_widget) { //only over the app, do we set a cursor..
                if(!qApp->d_func()->cursor_list.isEmpty()) {
                    cursor = qApp->d_func()->cursor_list.first();
                } else {
                    for(; cursor_widget; cursor_widget = cursor_widget->parentWidget()) {
                        QWExtra *extra = cursor_widget->d_func()->extraData();
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
            if(ekind == kEventMouseDown && qt_mac_is_macsheet(QApplication::activeModalWidget()))
                QApplication::activeModalWidget()->parentWidget()->activateWindow(); //sheets have a parent
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

                if (tabletCaps & 0x0800)  // Tangential pressure
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
            if(!widget) {
                const short windowPart = qt_mac_window_at(where.h, where.v, 0);
                if(windowPart == inMenuBar) {
                    QMacBlockingFunction block;
                    MenuSelect(where); //allow menu tracking
                }
            } else if(!(GetCurrentKeyModifiers() & cmdKey)) {
                QWidget *window = widget->window();
                window->raise();
                if(window->windowType() != Qt::Desktop
                   && window->windowType() != Qt::Popup && !qt_mac_is_macsheet(window)
                   && (window->isModal() || !::qobject_cast<QDockWidget *>(window))) {
                    const bool wasActive = window->isActiveWindow();
                    window->activateWindow();
                    if(!wasActive && !qt_mac_can_clickThrough(widget)) {
                        handled_event = false;
                        break;
                    }
                }
            }

            if(qt_mac_dblclick.last_widget &&
               qt_mac_dblclick.last_x != -1 && qt_mac_dblclick.last_y != -1 &&
               QRect(qt_mac_dblclick.last_x-2, qt_mac_dblclick.last_y-2, 4, 4).contains(QPoint(where.h, where.v))) {
                if(qt_mac_dblclick.use_qt_time_limit) {
                    EventTime now = GetEventTime(event);
                    if(qt_mac_dblclick.last_time != -2 && qt_mac_dblclick.last_widget == widget &&
                       now - qt_mac_dblclick.last_time <= ((double)QApplicationPrivate::mouse_double_click_time)/1000 &&
                       qt_mac_dblclick.last_button == button)
                        etype = QEvent::MouseButtonDblClick;
                } else {
                    UInt32 count = 0;
                    GetEventParameter(event, kEventParamClickCount, typeUInt32, 0,
                                      sizeof(count), 0, &count);
                    if(!(count % 2) && qt_mac_dblclick.last_modifiers == modifiers &&
                       qt_mac_dblclick.last_widget == widget && qt_mac_dblclick.last_button == button)
                        etype = QEvent::MouseButtonDblClick;
                }
                if(etype == QEvent::MouseButtonDblClick)
                    qt_mac_dblclick.last_widget = 0;
            }
            if(etype != QEvent::MouseButtonDblClick) {
                qt_mac_dblclick.last_x = where.h;
                qt_mac_dblclick.last_y = where.v;
            } else {
                qt_mac_dblclick.last_x = qt_mac_dblclick.last_y = -1;
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
                qt_mac_dblclick.last_widget = widget;
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
                    if(!qwe2.isAccepted())
                        handled_event = false;
                }
            } else {
#ifdef QMAC_SPEAK_TO_ME
                const int speak_keys = Qt::AltModifier | Qt::ShiftModifier;
		if(etype == QMouseEvent::MouseButtonDblClick && ((modifiers & speak_keys) == speak_keys)) {
                    QVariant v = widget->property("displayText");
                    if(!v.isValid()) v = widget->property("text");
                    if(!v.isValid()) v = widget->property("windowTitle");
                    if(v.isValid()) {
                        QString s = v.toString();
                        s.replace(QRegExp(QString::fromLatin1("(\\&|\\<[^\\>]*\\>)")), "");
                        SpeechChannel ch;
                        NewSpeechChannel(0, &ch);
                        SpeakText(ch, s.toLatin1().constData(), s.length());
                        DisposeSpeechChannel(ch);
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
                if(!qme.isAccepted())
                    handled_event = false;
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
    case kEventClassTextInput:
    case kEventClassKeyboard: {
        EventRef key_event = event;
        if(eclass == kEventClassTextInput) {
            Q_ASSERT(ekind == kEventTextInputUnicodeForKeyEvent);
            OSStatus err = GetEventParameter(event, kEventParamTextInputSendKeyboardEvent, typeEventRef, 0,
                                             sizeof(key_event), 0, &key_event);
            Q_ASSERT(err == noErr);
            Q_UNUSED(err);
        }
        const UInt32 key_ekind = GetEventKind(key_event);
        Q_ASSERT(GetEventClass(key_event) == kEventClassKeyboard);

        if(key_ekind == kEventRawKeyDown)
            qt_keymapper_private()->updateKeyMap(er, key_event, data);
        if(mac_keyboard_grabber)
            widget = mac_keyboard_grabber;
        else if (app->activePopupWidget())
            widget = (app->activePopupWidget()->focusWidget() ?
                      app->activePopupWidget()->focusWidget() : app->activePopupWidget());
        else if(QApplication::focusWidget())
            widget = QApplication::focusWidget();
        else
            widget = app->activeWindow();

        if (!widget) {
            // Darn, I need to update tho modifier state, even though
            // Qt itself isn't getting them, otherwise the keyboard state get inconsistent.
            if (key_ekind == kEventRawKeyModifiersChanged) {
                UInt32 modifiers = 0;
                GetEventParameter(key_event, kEventParamKeyModifiers, typeUInt32, 0,
                                  sizeof(modifiers), 0, &modifiers);
                extern void qt_mac_send_modifiers_changed(quint32 modifiers, QObject *object); // qkeymapper_mac.cpp
                // Just send it to the qApp for the time being.
                qt_mac_send_modifiers_changed(modifiers, qApp);
            }
            handled_event = false;
            break;
        }

        if(app_do_modal && !qt_try_modal(widget, key_event))
            break;
        handled_event = qt_keymapper_private()->translateKeyEvent(widget, er, key_event, data, widget == mac_keyboard_grabber);
        break; }
    case kEventClassWindow: {
        remove_context_timer = false;

        WindowRef wid = 0;
        GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
                          sizeof(WindowRef), 0, &wid);
        widget = qt_mac_find_window(wid);
        if(ekind == kEventWindowActivated) {
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
                if(QWidget *tmp_w = qt_mac_find_window(wp))
                    app->setActiveWindow(tmp_w);
            }
            QMenuBar::macUpdateMenuBar();
        } else if(ekind == kEventAppDeactivated) {
            while(app->d_func()->inPopupMode())
                app->activePopupWidget()->close();
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
                    bool handle_quit = true;
                    if(QApplicationPrivate::modalState()) {
                        int visible = 0;
                        const QWidgetList tlws = QApplication::topLevelWidgets();
                        for(int i = 0; i < tlws.size(); ++i) {
                            if(tlws.at(i)->isVisible())
                                ++visible;
                        }
                        handle_quit = (visible <= 1);
                    }
                    if(handle_quit) {
                        QCloseEvent ev;
                        QApplication::sendSpontaneousEvent(app, &ev);
                        if(ev.isAccepted())
                            app->quit();
                    } else {
                        QApplication::beep();
                    }
                } else if(cmd.commandID == kHICommandSelectWindow) {
                    if((GetCurrentKeyModifiers() & cmdKey))
                        handled_event = true;
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
    Q_Q(QApplication);
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
    Q_Q(QApplication);
    if(!QApplicationPrivate::popupWidgets)
        return;

    QApplicationPrivate::popupWidgets->removeAll(popup);
    if(popup == qt_button_down)
        qt_button_down = 0;
    if(QApplicationPrivate::popupWidgets->isEmpty()) {  // this was the last popup
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
        QWidget* aw = QApplicationPrivate::popupWidgets->last();
        if (QWidget *fw = aw->focusWidget())
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
        double dci = appleSettings.value(QLatin1String("com/apple/mouse/doubleClickThreshold"), 0.5).toDouble();
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
    QApplicationPrivate::wheel_scroll_lines = n;
}

int QApplication::wheelScrollLines()
{
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
        QStringList fontsubs = settings.childKeys();
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

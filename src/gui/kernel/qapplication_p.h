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

#ifndef QAPPLICATION_P_H
#define QAPPLICATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp, qcolor_x11.cpp, qfiledialog.cpp
// and many other.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include "qfont.h"
#include "qcursor.h"
#include "qmutex.h"
#include "qtranslator.h"
#include "qshortcutmap_p.h"

#include <private/qcoreapplication_p.h>
#include "qapplication.h"
#include "qbasictimer.h"

class QWidget;
class QObject;
class QClipboard;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
class QInputContext;

extern Q_GUI_EXPORT bool qt_modal_state();
extern Q_GUI_EXPORT void qt_enter_modal(QWidget*);
extern Q_GUI_EXPORT void qt_leave_modal(QWidget*);

extern bool qt_is_gui_used;
#ifndef QT_NO_CLIPBOARD
extern QClipboard *qt_clipboard;
#endif

#if defined (Q_OS_WIN32) || defined (Q_OS_CYGWIN)
extern QSysInfo::WinVersion qt_winver;
enum { QT_TABLET_NPACKETQSIZE = 128 };
# ifdef Q_OS_TEMP
  extern DWORD qt_cever;
# endif
#elif defined (Q_OS_MAC)
extern QSysInfo::MacVersion qt_macver;
#endif

extern void qt_dispatchEnterLeave(QWidget*, QWidget*);
extern bool qt_tryModalHelper(QWidget *, QWidget ** = 0);

#ifndef QT_NO_TABLET_SUPPORT
struct TabletDeviceData
{
    int minPressure;
    int maxPressure;
    int minX, maxX, minY, maxY;
#ifdef Q_WS_X11
    int deviceType;
    enum {
        TOTAL_XINPUT_EVENTS = 64
    };
    void *device;
    int eventCount;
    long unsigned int eventList[TOTAL_XINPUT_EVENTS]; // XEventClass is in fact a long unsigned int

    int xinput_motion;
    int xinput_key_press;
    int xinput_key_release;
    int xinput_button_press;
    int xinput_button_release;
#endif
};

typedef QList<TabletDeviceData> TabletDeviceDataList;
TabletDeviceDataList *qt_tablet_devices();
#endif

#ifdef QT_COMPAT
extern "C" {
    typedef bool (*Ptrqt_tryAccelEvent)(QWidget *w, QKeyEvent *e);
    typedef bool (*Ptrqt_tryComposeUnicode)(QWidget *w, QKeyEvent *e);
    typedef bool (*Ptrqt_dispatchAccelEvent)(QWidget *w, QKeyEvent *e);
}
#endif

class QApplicationPrivate : public QCoreApplicationPrivate
{
    Q_DECLARE_PUBLIC(QApplication)
public:
    QApplicationPrivate(int &argc, char **argv, QApplication::Type type);
    ~QApplicationPrivate() {}

    bool lastMousePressAccepted;

    void createEventDispatcher();

#ifndef QT_NO_SESSIONMANAGER
    QSessionManager *session_manager;
    QString session_id;
    QString session_key;
    bool is_session_restored;
#endif

#ifndef QT_NO_CURSOR
    QList<QCursor> cursor_list;
#endif

    QBasicTimer toolTipWakeUp, toolTipFallAsleep;
    QPoint toolTipPos, toolTipGlobalPos;
    QPointer<QWidget> toolTipWidget;
    QShortcutMap shortcutMap;

#ifdef QT_COMPAT
    bool qt_compat_used;
    bool qt_compat_resolved;
    Ptrqt_tryAccelEvent qt_tryAccelEvent;
    Ptrqt_tryComposeUnicode qt_tryComposeUnicode;
    Ptrqt_dispatchAccelEvent qt_dispatchAccelEvent;

    bool use_compat() {
        return qt_tryAccelEvent
               && qt_tryComposeUnicode
               && qt_dispatchAccelEvent;
    }
#endif
#ifdef Q_WS_X11
    static QInputContext *inputContext;
#endif

    static Qt::MouseButtons mouse_buttons;
    static Qt::KeyboardModifiers modifier_buttons;

    static QSize     app_strut;
    static QWidgetList *popupWidgets;
    static QStyle *app_style;
    static int app_cspec;
#ifndef QT_NO_PALETTE
    static QPalette *app_pal;
#endif
    static QFont *app_font;
    static QWidget *main_widget;
    static QWidget *focus_widget;
    static QWidget *active_window;
    static QPixmap *app_icon;
    static bool obey_desktop_settings;
    static int  cursor_flash_time;
    static int  mouse_double_click_time;
    static int  wheel_scroll_lines;

    static bool animate_ui;
    static bool animate_menu;
    static bool animate_tooltip;
    static bool animate_combo;
    static bool fade_menu;
    static bool fade_tooltip;
    static bool animate_toolbox;
    static bool widgetCount; // Coupled with -widgetcount switch

#ifdef Q_WS_MAC
    bool do_mouse_down(Point *, bool *);
    static OSStatus globalEventProcessor(EventHandlerCallRef,  EventRef, void *);
    static OSStatus globalAppleEventProcessor(const AppleEvent *, AppleEvent *, long);
    static void qt_context_timer_callbk(EventLoopTimerRef, void *);
    static bool qt_mac_apply_settings();
#endif
#ifdef Q_WS_QWS
    QPointer<QWSManager> last_manager;
#endif
};

#endif // QAPPLICATION_P_H

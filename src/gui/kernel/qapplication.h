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

#ifndef QAPPLICATION_H
#define QAPPLICATION_H

#ifndef QT_H
#include "qcoreapplication.h"
#include "qpoint.h"
#include "qsize.h"
#ifdef QT_INCLUDE_COMPAT
# include "qdesktopwidget.h"
#endif
#ifdef QT_COMPAT
# include "qwidget.h"
# include "qpalette.h"
#endif
#ifdef Q_WS_QWS
# include "qrgb.h"
#endif
#endif // QT_H

class QSessionManager;
class QDesktopWidget;
class QStyle;
class QEventLoop;
template <typename T> class QList;
#if defined(Q_WS_QWS)
class QWSDecoration;
#endif


class QApplication;
class QApplicationPrivate;
#define qApp (static_cast<QApplication *>(QCoreApplication::instance()))

class Q_GUI_EXPORT QApplication : public QCoreApplication
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QApplication)
public:
    QApplication(int &argc, char **argv);
    QApplication(int &argc, char **argv, bool GUIenabled);
    enum Type { Tty, GuiClient, GuiServer };
    QApplication(int &argc, char **argv, Type);
#if defined(Q_WS_X11)
    QApplication(Display* dpy, Qt::HANDLE visual = 0, Qt::HANDLE cmap = 0);
    QApplication(Display *dpy, int argc, char **argv,
                  Qt::HANDLE visual = 0, Qt::HANDLE cmap= 0);
#endif
    virtual ~QApplication();

    Type type() const;

#ifndef QT_NO_STYLE
    static QStyle  &style();
    static void            setStyle(QStyle*);
    static QStyle*  setStyle(const QString&);
#endif
    enum ColorSpec { NormalColor=0, CustomColor=1, ManyColor=2 };
    static int colorSpec();
    static void setColorSpec(int);

#ifdef QT_COMPAT
    enum ColorMode { NormalColors = NormalColor, CustomColors = CustomColor };
    static inline QT_COMPAT ColorMode colorMode() { return (ColorMode)colorSpec(); }
    static inline QT_COMPAT void setColorMode(ColorMode mode) { setColorSpec((ColorSpec)mode); }
#endif

#ifndef QT_NO_CURSOR
    static QCursor  *overrideCursor();
    static void             setOverrideCursor(const QCursor &, bool replace=false);
    static void             restoreOverrideCursor();
#endif
#ifndef QT_NO_PALETTE
    static QPalette  palette();
    static QPalette  palette(const QWidget *);
    static QPalette  palette(const char *className);
    static void             setPalette(const QPalette &, const char* className = 0);
#endif
    static QFont     font(const QWidget* = 0);
    static void             setFont(const QFont &, const char* className = 0);
    static QFontMetrics fontMetrics();

    static void setWindowIcon(const QPixmap &);
    static const QPixmap &windowIcon();


    QWidget            *mainWidget()  const;
    virtual void     setMainWidget(QWidget *);
    virtual void     polish(QWidget *);

    static QWidgetList allWidgets();
    static QWidgetList topLevelWidgets();

    static QDesktopWidget   *desktop();

    static QWidget     *activePopupWidget();
    static QWidget     *activeModalWidget();
#ifndef QT_NO_CLIPBOARD
    static QClipboard  *clipboard();
#endif
    QWidget               *focusWidget() const;
    QWidget               *activeWindow() const;

    static QWidget *widgetAt(int x, int y);
    static inline QWidget *widgetAt(const QPoint &p) { return widgetAt(p.x(), p.y()); }
    static QWidget *topLevelAt(int x,  int y);
    static inline QWidget *topLevelAt(const QPoint &p) { return topLevelAt(p.x(), p.y()); }

    static void syncX();
    static void beep();

    static void      setDesktopSettingsAware(bool);
    static bool      desktopSettingsAware();

    static void      setCursorFlashTime(int);
    static int       cursorFlashTime();

    static void      setDoubleClickInterval(int);
    static int       doubleClickInterval();
#ifndef QT_NO_WHEELEVENT
    static void      setWheelScrollLines(int);
    static int       wheelScrollLines();
#endif
    static void             setGlobalStrut(const QSize &);
    static QSize     globalStrut();

    static void setStartDragTime(int ms);
    static int startDragTime();
    static void setStartDragDistance(int l);
    static int startDragDistance();

    static void setReverseLayout(bool b);
    static bool reverseLayout();

    static Qt::Alignment horizontalAlignment(Qt::Alignment align);

    static bool            isEffectEnabled(Qt::UIEffect);
    static void            setEffectEnabled(Qt::UIEffect, bool enable = true);

#if defined(Q_WS_MAC)
    virtual bool     macEventFilter(EventHandlerCallRef, EventRef);
#endif
#if defined(Q_WS_X11)
    virtual bool     x11EventFilter(XEvent *);
    virtual int             x11ClientMessage(QWidget*, XEvent*, bool passive_only);
    int              x11ProcessEvent(XEvent*);
#endif
#if defined(Q_WS_QWS)
    virtual bool     qwsEventFilter(QWSEvent *);
    int              qwsProcessEvent(QWSEvent*);
    void             qwsSetCustomColors(QRgb *colortable, int start, int numColors);
#ifndef QT_NO_QWS_MANAGER
    static QWSDecoration &qwsDecoration();
    static void      qwsSetDecoration(QWSDecoration *);
#endif
#endif

#ifdef QT_COMPAT
#if defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN)
    static QT_COMPAT Qt::WindowsVersion winVersion() { return (Qt::WindowsVersion)QSysInfo::WindowsVersion; }
#endif
#if defined(Q_OS_MAC)
    static QT_COMPAT Qt::MacintoshVersion macVersion() { return (Qt::MacintoshVersion)QSysInfo::MacintoshVersion; }
#endif
#endif
#if defined(Q_WS_WIN)
    void             winFocus(QWidget *, bool);
    static void             winMouseButtonUp();
#endif

#ifndef QT_NO_SESSIONMANAGER
    // session management
    bool             isSessionRestored() const;
    QString         sessionId() const;
    QString         sessionKey() const;
    virtual void     commitData(QSessionManager& sm);
    virtual void     saveState(QSessionManager& sm);
#endif
#if defined(Q_WS_X11)
    static void create_xim();
    static void close_xim();
    static bool x11_apply_settings();
#endif

    int exec();
    bool notify(QObject *, QEvent *);


signals:
    void             lastWindowClosed();

public slots:
    void             closeAllWindows();
    void             aboutQt();

protected:
#if defined(Q_WS_QWS)
    void setArgs(int, char **);
#endif
    bool event(QEvent *);
    bool compressEvent(QEvent *, QObject *receiver, QPostEventList *);

#ifdef QT_COMPAT
public:
    inline static QT_COMPAT bool hasGlobalMouseTracking() {return true;}
    inline static QT_COMPAT void setGlobalMouseTracking(bool) {};
    inline static QT_COMPAT void flushX() { flush(); }
#ifndef QT_NO_PALETTE
    // obsolete functions
    static inline QT_COMPAT void setWinStyleHighlightColor(const QColor &c) {
        QPalette p(palette());
        p.setColor(QPalette::Highlight, c);
        setPalette(p);
    }
    static inline QT_COMPAT const QColor &winStyleHighlightColor()
        { return palette().color(QPalette::Active, QPalette::Highlight); }
    static inline QT_COMPAT void setPalette(const QPalette &pal, bool, const char* className = 0)
        { setPalette(pal, className); };
#endif // QT_NO_PALETTE
    static inline QT_COMPAT void setFont(const QFont &font, bool, const char* className = 0)
        { setFont(font, className); }

    static inline QT_COMPAT QWidget *widgetAt(int x, int y, bool child)
        { QWidget *w = widgetAt(x, y); return child ? w : (w ? w->topLevelWidget() : 0); }
    static inline QT_COMPAT QWidget *widgetAt(const QPoint &p, bool child)
        { QWidget *w = widgetAt(p); return child ? w : (w ? w->topLevelWidget() : 0); }

#endif // QT_COMPAT
private:
    static QWidget *widgetAt_sys(int x, int y);
    bool notify_helper(QObject *receiver, QEvent * e);

    void construct(Type);
    void initialize();
    void process_cmdline();
#if defined(Q_WS_QWS)
    static QWidget *findChildWidget(const QWidget *p, const QPoint &pos);
    static QWidget *findWidget(const QObjectList&, const QPoint &, bool rec);
#endif

#if defined(Q_WS_MAC)
    bool do_mouse_down(Point *, bool *);
    static OSStatus globalEventProcessor(EventHandlerCallRef,  EventRef, void *);
    static OSStatus globalAppleEventProcessor(const AppleEvent *, AppleEvent *, long);
    static void qt_context_timer_callbk(EventLoopTimerRef, void *);
    static void qt_select_timer_callbk(EventLoopTimerRef, void *);
    static bool qt_mac_apply_settings();
    friend class QMacInputMethod;
    friend OSStatus qt_window_event(EventHandlerCallRef, EventRef, void *);
    friend void qt_mac_update_os_settings();
    friend bool qt_set_socket_handler(int, int, QObject *, bool);
    friend void qt_mac_destroy_widget(QWidget *);
    friend void qt_init(QApplicationPrivate *priv, QApplication::Type);
#endif

    static QStyle   *app_style;
    static int             app_cspec;
#ifndef QT_NO_PALETTE
    static QPalette *app_pal;
#endif
    static QFont    *app_font;
    static QWidget  *main_widget;
    static QWidget  *focus_widget;
    static QWidget  *active_window;
    static QPixmap    *app_icon;
    static bool             obey_desktop_settings;
    static int             cursor_flash_time;
    static int             mouse_double_click_time;
    static int             wheel_scroll_lines;

    static bool             animate_ui;
    static bool             animate_menu;
    static bool             animate_tooltip;
    static bool             animate_combo;
    static bool             fade_menu;
    static bool             fade_tooltip;
    static bool             animate_toolbox;
    static bool             widgetCount; // Coupled with -widgetcount switch

#if defined(Q_WS_X11) && !defined (QT_NO_STYLE)
    static void x11_initialize_style();
#endif

    static QSize     app_strut;

    static QWidgetList *popupWidgets;
    bool             inPopupMode() const;
    void             closePopup(QWidget *popup);
    void             openPopup(QWidget *popup);
public:
    void setActiveWindow(QWidget* act);
private:

    // ### the next 2 friends should go away
    friend class QEventLoop;
    friend class QEvent;
    friend class QWidget;
    friend class QWidgetPrivate;
    friend class QETWidget;
    friend class QDialog;
    friend class QAccelManager;
    friend class QTranslator;
    friend class QShortcut;
    friend class QAction;
#if defined(Q_WS_QWS)
    friend class QInputContext;
#endif
private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QApplication(const QApplication &);
    QApplication &operator=(const QApplication &);
#endif
};

inline QWidget *QApplication::mainWidget() const
{
    return main_widget;
}

inline QWidget *QApplication::focusWidget() const
{
    return focus_widget;
}

inline QWidget *QApplication::activeWindow() const
{
    return active_window;
}

inline bool QApplication::inPopupMode() const
{
    return popupWidgets != 0;
}

inline QSize QApplication::globalStrut()
{
    return app_strut;
}

inline Qt::Alignment QApplication::horizontalAlignment(Qt::Alignment align)
{
    align &= Qt::AlignHorizontal_Mask;
    if (align == Qt::AlignAuto) {
        if (reverseLayout())
            align = Qt::AlignRight;
        else
            align = Qt::AlignLeft;
    }
    return align;
}

#endif // QAPPLICATION_H


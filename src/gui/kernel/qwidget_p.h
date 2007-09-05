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

#ifndef QWIDGET_P_H
#define QWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qwidget.h"
#include "private/qobject_p.h"
#include "QtCore/qrect.h"
#include "QtCore/qlocale.h"
#include "QtGui/qregion.h"
#include "QtGui/qsizepolicy.h"
#include "QtGui/qstyle.h"

#ifdef Q_WS_WIN
#include "QtCore/qt_windows.h"
#include <private/qdnd_p.h>
#endif // Q_WS_WIN

#ifdef Q_WS_X11
#include "QtGui/qx11info_x11.h"
#endif

#if defined(Q_WS_QWS)
#include "QtGui/qinputcontext.h"
#include "QtGui/qscreen_qws.h"
#endif

#if defined(Q_WS_QWS) || defined(Q_WS_X11)
//#define Q_WIDGET_USE_DIRTYLIST
#endif

#ifndef Q_WS_MAC
#define Q_WIDGET_CACHE_OPAQUEREGIONS
#endif

//#define Q_WIN_USE_QT_UPDATE_EVENT
//#define Q_RATE_LIMIT_PAINTING
//#define Q_FLATTEN_EXPOSE

// Extra QWidget data
//  - to minimize memory usage for members that are seldom used.
//  - top-level widgets have extra extra data to reduce cost further
#if defined(Q_WS_QWS)
class QWSManager;
#endif
#if defined(Q_WS_MAC)
class QCoreGraphicsPaintEnginePrivate;
#endif
class QPaintEngine;
class QPixmap;
class QWidgetBackingStore;

class QStyle;

struct QTLWExtra {
    QString caption; // widget caption
    QString iconText; // widget icon text
    QString role; // widget role
    QIcon *icon; // widget icon
    QPixmap *iconPixmap;
    short incw, inch; // size increments
     // frame strut, don't use these directly, use QWidgetPrivate::frameStrut() instead.
    QRect frameStrut;
    uint opacity : 8;
    uint posFromMove : 1;
#ifndef Q_WS_MAC
    QWidgetBackingStore *backingStore;
#endif
#if defined(Q_WS_WIN)
    ulong savedFlags; // Save window flags while showing fullscreen
#else
    Qt::WindowFlags savedFlags; // Save widget flags while showing fullscreen
#endif
    short basew, baseh; // base sizes
#if defined(Q_WS_X11)
    WId parentWinId; // parent window Id (valid after reparenting)
    uint embedded : 1; // window is embedded in another Qt application
    uint spont_unmapped: 1; // window was spontaneously unmapped
    uint dnd : 1; // DND properties installed
    uint validWMState : 1; // is WM_STATE valid?
    uint waitingForMapNotify : 1; // show() has been called, haven't got the MapNotify yet
    QPoint fullScreenOffset;
    QBitmap *iconMask;
#endif
#if defined(Q_WS_MAC)
    quint32 wattr;
    quint32 wclass;
    WindowGroupRef group;
    uint resizer : 4;
    uint isSetGeometry : 1;
    uint isMove : 1;
#endif
#if defined(Q_WS_QWS) && !defined (QT_NO_QWS_MANAGER)
    QWSManager *qwsManager;
#endif
#if defined Q_WS_QWS
    bool inPaintTransaction;
#endif
#if defined(Q_WS_WIN)
    HICON winIconBig; // internal big Windows icon
    HICON winIconSmall; // internal small Windows icon
#endif
    QRect normalGeometry; // used by showMin/maximized/FullScreen
    QWindowSurface *windowSurface;
};

struct QWExtra {
    qint32 minw, minh; // minimum size
    qint32 maxw, maxh; // maximum size
    QPointer<QWidget> focus_proxy;
#ifndef QT_NO_CURSOR
    QCursor *curs;
#endif
    QTLWExtra *topextra; // only useful for TLWs
#if defined(Q_WS_WIN)
    QOleDropTarget *dropTarget; // drop target
    QList<QPointer<QWidget> > oleDropWidgets;
#endif
#if defined(Q_WS_X11)
    WId xDndProxy; // XDND forwarding to embedded windows
#endif
    QRegion mask; // widget mask

//bit flags at the end to improve packing
#if defined(Q_WS_WIN)
    uint shown_mode : 8; // widget show mode
#ifndef QT_NO_DIRECT3D
    uint had_paint_on_screen : 1;
    uint had_no_system_bg : 1;
    uint had_auto_fill_bg : 1;
#endif
#endif
#if defined(Q_WS_X11)
    uint compress_events : 1;
#endif
    uint explicitMinSize : 2;
    uint autoFillBackground : 1;

    QPointer<QStyle> style;
    QString styleSheet;
};

class Q_GUI_EXPORT QWidgetPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWidget)

public:
    explicit QWidgetPrivate(int version = QObjectPrivateVersion);
    ~QWidgetPrivate();

    QWExtra *extraData() const;
    QTLWExtra *topData() const;
    QTLWExtra *maybeTopData() const;
#ifndef Q_WS_MAC
    QWidgetBackingStore *maybeBackingStore() const;
#endif
#ifdef Q_WS_QWS
    void setMaxWindowState_helper();
#endif
    void init(QWidget *desktopWidget, Qt::WindowFlags f);
    void create_sys(WId window, bool initializeWindow, bool destroyOldWindow);
    void createRecursively();
    void createWinId(WId id = 0);

    void createTLExtra();
    void createExtra();
    void deleteExtra();
    void createSysExtra();
    void deleteSysExtra();
    void createTLSysExtra();
    void deleteTLSysExtra();
    void updateSystemBackground();
    void propagatePaletteChange();

    void setPalette_helper(const QPalette &);
    void resolvePalette();

#ifdef Q_WS_WIN
    void setMask_sys(const QRegion &);
#endif

#ifdef Q_WS_MAC
    void macUpdateSizeAttribute();
    void macUpdateHideOnSuspend();
    void macUpdateOpaqueSizeGrip();
    void macUpdateIgnoreMouseEvents();
    void macUpdateMetalAttribute();
    void macUpdateIsOpaque();
#endif

    void raise_sys();
    void lower_sys();
    void stackUnder_sys(QWidget *);

    void setFocus_sys();

    void setFont_helper(const QFont &);
    void resolveFont();

    void setLayoutDirection_helper(Qt::LayoutDirection);
    void resolveLayoutDirection();

    void setLocale_helper(const QLocale &l);
    void resolveLocale();

    void setStyle_helper(QStyle *newStyle, bool propagate, bool metalHack = false);
    void inheritStyle();

    bool isBackgroundInherited() const;

    void setUpdatesEnabled_helper(bool );

    void paintBackground(QPainter *, const QRect &, bool asRoot = true) const;
    enum DrawWidgetFlags {
        DrawAsRoot = 0x01,
        DrawPaintOnScreen = 0x02,
        DrawRecursive = 0x04,
        DrawInvisible = 0x08,
        DontSubtractOpaqueChildren = 0x10
    };
    void drawWidget(QPaintDevice *pdev, const QRegion &rgn, const QPoint &offset, int flags = DrawAsRoot | DrawRecursive);

#ifdef Q_WS_MAC
    void render_helper(QWidget *widget, QPaintDevice *result, const QPoint &offset,
                       const QRect &rect, QWidget::RenderFlags renderFlags);
#endif

    QRect clipRect() const;
    QRegion clipRegion() const;
    void subtractOpaqueChildren(QRegion &rgn, const QRegion &clipRgn, const QPoint &offset, int startIdx = 0) const;
    void subtractOpaqueSiblings(QRegion &rgn, const QPoint &offset) const;
    void updateIsOpaque();
    bool isOpaque() const;
    bool hasBackground() const;
    bool paintOnScreen() const;

#ifdef Q_WIDGET_CACHE_OPAQUEREGIONS
    QRegion getOpaqueRegion() const;
    QRegion getOpaqueChildren() const;
    void setDirtyOpaqueRegion();

#if defined(Q_WIDGET_USE_DIRTYLIST) || (QT_VERSION >= 0x040400)
    QRegion getOpaqueSiblings() const;
#endif

    QRegion opaqueChildren;
    bool dirtyOpaqueChildren;
#endif

    enum CloseMode {
        CloseNoEvent,
        CloseWithEvent,
        CloseWithSpontaneousEvent
    };
    bool close_helper(CloseMode mode);

    bool compositeEvent(QEvent *e);
    void setWindowIcon_helper();
    void setWindowIcon_sys(bool forceReset = false);

    void focusInputContext();

#if defined(Q_WS_X11)
    void setWindowRole(const char *role);
    void sendStartupMessage(const char *message) const;
#endif

#if defined (Q_WS_WIN)
    void reparentChildren();
#endif

    void scrollChildren(int dx, int dy);

#ifndef Q_WS_MAC
    void dirtyWidget_sys(const QRegion &rgn);
    void cleanWidget_sys(const QRegion& rgn);
    void moveRect(const QRect &, int dx, int dy);
    void scrollRect(const QRect &, int dx, int dy);
    void invalidateBuffer(const QRegion &);
    bool isOverlapped(const QRect&) const;
# if defined(Q_WS_X11) || defined(Q_WS_WIN)
    QRegion dirtyOnScreen;
# endif
#endif

    void reparentFocusWidgets(QWidget *oldtlw);

    static int pointToRect(const QPoint &p, const QRect &r);
    QRect fromOrToLayoutItemRect(const QRect &rect, int sign) const;

    void setWinId(WId);
    void showChildren(bool spontaneous);
    void hideChildren(bool spontaneous);
    void setParent_sys(QWidget *parent, Qt::WindowFlags);
    void scroll_sys(int dx, int dy);
    void scroll_sys(int dx, int dy, const QRect &r);
    void deactivateWidgetCleanup();
    void setGeometry_sys(int, int, int, int, bool);
#ifdef Q_WS_MAC
    void setGeometry_sys_helper(int, int, int, int, bool);
#endif
    void show_recursive();
    void show_helper();
    void show_sys();
    void hide_sys();
    void hide_helper();
    void _q_showIfNotHidden();

    void setEnabled_helper(bool);
    void registerDropSite(bool);
#ifdef Q_WS_WIN
    QOleDropTarget *registerOleDnd(QWidget *widget);
    void unregisterOleDnd(QWidget *widget, QOleDropTarget *target);
#endif
    static void adjustFlags(Qt::WindowFlags &flags, QWidget *w = 0);

    void updateFrameStrut();
    QRect frameStrut() const;

    void setWindowIconText_sys(const QString &cap);
    void setWindowIconText_helper(const QString &cap);
    void setWindowTitle_sys(const QString &cap);

#ifdef Q_OS_WIN
    void grabMouseWhileInWindow();
#endif

#ifndef QT_NO_CURSOR
    void setCursor_sys(const QCursor &cursor);
    void unsetCursor_sys();
#endif

#ifdef Q_WS_MAC
    void setWindowModified_sys(bool b);
    void createWindow_sys();
    void determineWindowClass();
    void initWindowPtr();
    void transferChildren();
#endif
    void setWindowTitle_helper(const QString &cap);

    bool setMinimumSize_helper(int &minw, int &minh);
    bool setMaximumSize_helper(int &maxw, int &maxh);
    void setConstraints_sys();
    void updateGeometry_helper(bool forceUpdate);

    void getLayoutItemMargins(int *left, int *top, int *right, int *bottom) const;
    void setLayoutItemMargins(int left, int top, int right, int bottom);
    void setLayoutItemMargins(QStyle::SubElement element, const QStyleOption *opt = 0);

#if defined(Q_WS_QWS)
    void moveSurface(QWindowSurface *surface, const QPoint &offset);

    QRegion localRequestedRegion() const;
    QRegion localAllocatedRegion() const;

    void blitToScreen(const QRegion &globalrgn);
#ifndef QT_NO_CURSOR
    void updateCursor() const;
#endif

    QScreen* getScreen() const;

    friend class QWSManager;
    friend class QWSManagerPrivate;
    friend class QDecoration;
#endif

    static int instanceCounter; // Current number of widget instances
    static int maxInstances; // Maximum number of widget instances

#ifdef QT_KEYPAD_NAVIGATION
    static QPointer<QWidget> editingWidget;
#endif

    QWidgetData data;

    QWExtra *extra;
    QWidget *focus_next;
    QWidget *focus_prev;
    QWidget *focus_child;
#ifndef QT_NO_ACTION
    QList<QAction*> actions;
#endif
    QLayout *layout;
#if !defined(QT_NO_IM)
    QPointer<QInputContext> ic;
#endif
    // All widgets are initially added into the uncreatedWidgets set. Once
    // they receive a window id they are removed and added to the mapper
    static QWidgetMapper *mapper;
    static QWidgetSet *uncreatedWidgets;

    short leftmargin, topmargin, rightmargin, bottommargin;

    signed char leftLayoutItemMargin;
    signed char topLayoutItemMargin;
    signed char rightLayoutItemMargin;
    signed char bottomLayoutItemMargin;

    // ### TODO: reorganize private/extra/topextra to save memory
    QPointer<QWidget> compositeChildGrab;
#ifndef QT_NO_TOOLTIP
    QString toolTip;
#endif
#ifndef QT_NO_STATUSTIP
    QString statusTip;
#endif
#ifndef QT_NOWHATSTHIS
    QString whatsThis;
#endif
    QString accessibleName, accessibleDescription;

    QPalette::ColorRole fg_role : 8;
    QPalette::ColorRole bg_role : 8;
    uint high_attributes[3]; // the low ones are in QWidget::widget_attributes
    Qt::HANDLE hd;
#ifdef Q_RATE_LIMIT_PAINTING
    int timerId;
#endif
#if defined(Q_WIDGET_USE_DIRTYLIST) || defined(Q_RATE_LIMIT_PAINTING)
    QRegion dirty;
#endif
#if defined(Q_WS_X11)
    QX11Info xinfo;
    Qt::HANDLE picture;
#endif
#if defined(Q_WS_MAC)
    enum PaintChildrenOPs {
        PC_None = 0x00,
        PC_Now = 0x01,
        PC_NoPaint = 0x04,
        PC_Later = 0x10
    };
    EventHandlerRef window_event;
    bool qt_mac_dnd_event(uint, DragRef);
    void toggleDrawers(bool);
    //mac event functions
    static bool qt_create_root_win();
    static void qt_clean_root_win();
    static bool qt_recreate_root_win();
    static bool qt_mac_update_sizer(QWidget *, int);
    static OSStatus qt_window_event(EventHandlerCallRef er, EventRef event, void *);
    static OSStatus qt_widget_event(EventHandlerCallRef er, EventRef event, void *);
    static bool qt_widget_rgn(QWidget *, short, RgnHandle, bool);

    //these are here just for code compat (HIViews)
    QRegion clp;
    Qt::HANDLE qd_hd;
    uint clp_serial : 8;
    inline QRegion clippedRegion(bool = true) { return clp; }
    inline uint clippedSerial(bool =true) { return clp_serial; }
    uint needWindowChange : 1;
#endif

#if defined(Q_WS_X11) || defined (Q_WS_WIN) || defined(Q_WS_MAC)
    void setWSGeometry(bool dontShow=false);

    inline QPoint mapToWS(const QPoint &p) const
    { return p - data.wrect.topLeft(); }

    inline QPoint mapFromWS(const QPoint &p) const
    { return p + data.wrect.topLeft(); }

    inline QRect mapToWS(const QRect &r) const
    { QRect rr(r); rr.translate(-data.wrect.topLeft()); return rr; }

    inline QRect mapFromWS(const QRect &r) const
    { QRect rr(r); rr.translate(data.wrect.topLeft()); return rr; }
#endif

    QPaintEngine *extraPaintEngine;

    mutable const QMetaObject *polished;

    void setModal_sys();
    QSizePolicy size_policy;
    QLocale locale;
};

inline QWExtra *QWidgetPrivate::extraData() const
{
    return extra;
}

inline QTLWExtra *QWidgetPrivate::topData() const
{
    const_cast<QWidgetPrivate *>(this)->createTLExtra();
    return extra->topextra;
}

inline QTLWExtra *QWidgetPrivate::maybeTopData() const
{
    return extra ? extra->topextra : 0;
}

#ifndef Q_WS_MAC
inline QWidgetBackingStore *QWidgetPrivate::maybeBackingStore() const
{
    Q_Q(const QWidget);
    QTLWExtra *x = q->window()->d_func()->maybeTopData();
    return x ? x->backingStore : 0;
}
#endif

#endif // QWIDGET_P_H

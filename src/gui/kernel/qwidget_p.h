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

#include <private/qobject_p.h>

#include <qrect.h>
#include <qregion.h>
#include <qsizepolicy.h>
#include <qwidget.h>

#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif // Q_WS_WIN

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

// Extra QWidget data
//  - to minimize memory usage for members that are seldom used.
//  - top-level widgets have extra extra data to reduce cost further

class QWSManager;
#if defined(Q_WS_WIN)
class QOleDropTarget;
#endif
#if defined(Q_WS_MAC)
class QCoreGraphicsPaintEnginePrivate;
#endif
class QPaintEngine;
class QPixmap;

#ifndef QT_NO_STYLE
class QStyle;
#endif

struct QTLWExtra {
#ifndef QT_NO_WIDGET_TOPEXTRA
    QString  caption;                                // widget caption
    QString  iconText;                                // widget icon text
    QString  role;                                // widget role
    QPixmap *icon;                                // widget icon
#endif
    short    incw, inch;                        // size increments
    // frame strut
    ulong    fleft, fright, ftop, fbottom;
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    uint     opacity : 8;                        // Stores opacity level on Windows/Mac OS X.
#endif
#if defined(Q_WS_WIN)
    ulong savedFlags;                                // Save window flags while showing fullscreen
#else
    Qt::WFlags savedFlags;                        // Save widget flags while showing fullscreen
#endif
    short    basew, baseh;                        // base sizes
#if defined(Q_WS_X11)
    WId  parentWinId;                                // parent window Id (valid after reparenting)
    uint     embedded : 1;                        // window is embedded in another Qt application
    uint     spont_unmapped: 1;                        // window was spontaneously unmapped
    uint     reserved: 1;                        // reserved
    uint     dnd : 1;                                // DND properties installed
    uint     uspos : 1;                                // User defined position
    uint     ussize : 1;                        // User defined size
#endif
#if defined(Q_WS_MAC)
    WindowGroupRef group;
    uint     is_moved: 1;
    uint     resizer : 4;
#endif
#if defined(Q_WS_QWS) && !defined (QT_NO_QWS_MANAGER)
    QRegion decor_allocated_region;                // decoration allocated region
    QWSManager *qwsManager;
#endif
#if defined(Q_WS_WIN)
    HICON    winIcon;                                // internal Windows icon
#endif
    QRect    normalGeometry;                        // used by showMin/maximized/FullScreen
};


// dear user: you can see this struct, but it is internal. do not touch.

struct QWExtra {
    Q_INT32  minw, minh;                        // minimum size
    Q_INT32  maxw, maxh;                        // maximum size
    QPointer<QWidget> focus_proxy;
#ifndef QT_NO_CURSOR
    QCursor *curs;
#endif
    QTLWExtra *topextra;                        // only useful for TLWs
#if defined(Q_WS_WIN)
    QOleDropTarget *dropTarget;                        // drop target
    uint shown_mode : 8;                        // widget show mode
#endif
#if defined(Q_WS_X11)
    WId xDndProxy;                                // XDND forwarding to embedded windows
    uint children_use_dnd : 1;
    uint compress_events : 1;
#endif
    QRegion mask;                                // widget mask
#ifndef QT_NO_STYLE
    QStyle* style;
#endif
    QSizePolicy size_policy;
};

class Q_GUI_EXPORT QWidgetPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWidget)

public:
    QWidgetPrivate(int version = QObjectPrivateVersion);
    ~QWidgetPrivate();

    QWExtra        *extraData() const;
    QTLWExtra        *topData() const;

    void init(Qt::WFlags f);

    void createTLExtra();
    void createExtra();
    void deleteExtra();
    void createSysExtra();
    void deleteSysExtra();
    void createTLSysExtra();
    void deleteTLSysExtra();
    void updateSystemBackground();
    void propagatePaletteChange();

#ifndef QT_NO_PALETTE
    void setPalette_helper(const QPalette &);
    void resolvePalette();
#endif

    void setFont_sys(QFont *f = 0);
    void setFont_helper(const QFont &);
    void resolveFont();

    void setLayoutDirection_helper(Qt::LayoutDirection);
    void resolveLayoutDirection();

    bool isForegroundInherited() const;
    bool isBackgroundInherited() const;
    bool isTransparent() const;
    void updateInheritedBackground(bool force = false);
    void updatePropagatedBackground(const QRegion * = 0);

    QRect clipRect() const;

    enum CloseMode {
        CloseNoEvent,
        CloseWithEvent,
        CloseWithSpontaneousEvent
    };
    bool close_helper(CloseMode mode);

    bool compositeEvent(QEvent *e);
    void setWindowIcon_sys(const QPixmap &pixmap);

#if defined(Q_WS_X11)
    void focusInputContext();
    void unfocusInputContext();
    void checkChildrenDnd();
    void removePendingPaintEvents();
    QRegion invalidated_region;

    void setWindowRole(const char *role);
#endif

#if defined (Q_WS_WIN)
    void reparentChildren();
#endif

    void reparentFocusWidgets(QWidget *oldtlw);

    static int pointToRect(const QPoint &p, const QRect &r);

    void setWinId(WId);
    void showChildren(bool spontaneous);
    void hideChildren(bool spontaneous);
    void setParent_sys(QWidget *parent, Qt::WFlags);
    void deactivateWidgetCleanup();
    void setGeometry_sys(int, int, int, int, bool);
    void show_recursive();
    void show_helper();
    void show_sys();
    void hide_sys();
    void hide_helper();
    void setEnabled_helper(bool);
    void updateFrameStrut() const;

#if defined(Q_WS_QWS)
    void updateOverlappingChildren() const;
    void setChildrenAllocatedDirty();
    void setChildrenAllocatedDirty(const QRegion &r, const QWidget *dirty=0);
    bool isAllocatedRegionDirty() const;
    void updateRequestedRegion(const QPoint &gpos);
    QRegion requestedRegion() const;
    QRegion allocatedRegion() const;
    QRegion paintableRegion() const;

#ifndef QT_NO_CURSOR
    void updateCursor(const QRegion &r) const;
#endif
    // used to accumulate dirty region when children moved/resized.
    QRegion dirtyChildren;
    bool isSettingGeometry;
    friend class QWSManager;
    friend class QWSManagerPrivate;
    friend class QDecoration;
    friend class QWSPaintEngine;
#endif

    static int instanceCounter;  // Current number of widget instances
    static int maxInstances;     // Maximum number of widget instances

    QWidgetData data;

    QWExtra *extra;
    QWidget *focus_next;
    QWidget *focus_child;
    QList<QAction*> actions;
#ifndef QT_NO_LAYOUT
    QLayout *layout;
#endif
#if defined(Q_WS_X11) && !defined(QT_NO_IM)
    QPointer<QInputContext> ic;
#endif
    static QWidgetMapper *mapper;

    int leftmargin, topmargin, rightmargin, bottommargin;
    // ### TODO: reorganize private/extra/topextra to save memory
    QPointer<QWidget> compositeChildGrab;
    QString toolTip, statusTip, whatsThis;
    QString accessibleName, accessibleDescription;

#ifndef QT_NO_PALETTE
    QPalette::ColorRole fg_role : 8;
    QPalette::ColorRole bg_role : 8;
#endif
    uint high_attributes[1]; // the low ones are in QWidget::widget_attributes
    Qt::HANDLE hd;
#if defined(Q_WS_X11)
    QX11Info xinfo;
    Qt::HANDLE xft_hd;
#endif
#if defined(Q_WS_MAC)
    enum PaintChildrenOPs {
        PC_None = 0x00,
        PC_Now = 0x01,
        PC_NoPaint = 0x04,
        PC_Later = 0x10
    };
    uint    macDropEnabled : 1;
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
    uint clp_serial : 8;
    inline QRegion clippedRegion(bool = true) { return clp; }
    inline uint clippedSerial(bool =true) { return clp_serial; }
    CGContextRef cg_hd;
#endif

#if defined(Q_WS_X11) || defined (Q_WS_WIN) || defined(Q_WS_MAC)
    void setWSGeometry();
    inline QPoint mapToWS(const QPoint &p) const { return p - data.wrect.topLeft(); }
    inline QPoint mapFromWS(const QPoint &p) const { return p + data.wrect.topLeft(); }
    inline QRect mapToWS(const QRect &r) const { QRect rr(r); rr.translate(-data.wrect.topLeft()); return rr; }
    inline QRect mapFromWS(const QRect &r) const { QRect rr(r); rr.translate(data.wrect.topLeft()); return rr; }
#endif

    mutable const QMetaObject *polished;
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

#endif

/****************************************************************************
**
** Definition of some Qt private functions.
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
//
#ifndef QT_H
#include "qobject_p.h"
#endif // QT_H


// Extra QWidget data
//  - to minimize memory usage for members that are seldom used.
//  - top-level widgets have extra extra data to reduce cost further

class QWSManager;
#if defined(Q_WS_WIN)
class QOleDropTarget;
#endif
#if defined(Q_WS_MAC)
class QMacDndExtra;
class QMacCGExtra;
#endif

struct QTLWExtra {
#ifndef QT_NO_WIDGET_TOPEXTRA
    QString  caption;				// widget caption
    QString  iconText;				// widget icon text
    QString  role;				// widget role
    QPixmap *icon;				// widget icon
#endif
    short    incw, inch;			// size increments
    // frame strut
    ulong    fleft, fright, ftop, fbottom;
#if defined( Q_WS_WIN ) || defined( Q_WS_MAC )
    uint     opacity : 8;			// Stores opacity level on Windows/Mac OS X.
#endif
#if defined(Q_WS_WIN)
    ulong savedFlags;				// Save window flags while showing fullscreen
#else
    Qt::WFlags savedFlags;			// Save widget flags while showing fullscreen
#endif
    short    basew, baseh;			// base sizes
#if defined(Q_WS_X11)
    WId  parentWinId;				// parent window Id (valid after reparenting)
    uint     embedded : 1;			// window is embedded in another Qt application
    uint     spont_unmapped: 1;			// window was spontaneously unmapped
    uint     reserved: 1;			// reserved
    uint     dnd : 1;				// DND properties installed
    uint     uspos : 1;				// User defined position
    uint     ussize : 1;			// User defined size
    void    *xic;				// XIM Input Context
#endif
#if defined(Q_WS_MAC)
    WindowGroupRef group;
    uint     is_moved: 1;
    uint     resizer : 4;
#endif
#if defined(Q_WS_QWS) && !defined ( QT_NO_QWS_MANAGER )
    QRegion decor_allocated_region;		// decoration allocated region
    QWSManager *qwsManager;
#endif
#if defined(Q_WS_WIN)
    HICON    winIcon;				// internal Windows icon
#endif
    QRect    normalGeometry;			// used by showMin/maximized/FullScreen
};


// dear user: you can see this struct, but it is internal. do not touch.

struct QWExtra {
    Q_INT16  minw, minh;			// minimum size
    Q_INT16  maxw, maxh;			// maximum size
    QPointer<QWidget> focus_proxy;
#ifndef QT_NO_CURSOR
    QCursor *curs;
#endif
    QTLWExtra *topextra;			// only useful for TLWs
#if defined(Q_WS_WIN)
    QOleDropTarget *dropTarget;			// drop target
    uint shown_mode : 8;			// widget show mode
#endif
#if defined(Q_WS_X11)
    WId xDndProxy;				// XDND forwarding to embedded windows
#endif
#if defined(Q_WS_MAC)
    QRegion clip_saved, clip_sibs, clip_children;
    QMacDndExtra *macDndExtra;
    QRegion dirty_area;
    uint clip_dirty : 1, clip_serial : 15;
    uint child_dirty : 1, child_serial : 15;
    uint has_dirty_area:1;
#endif // Q_WS_MAC
#if defined(Q_WS_X11)
    uint children_use_dnd : 1;
    uint compress_events : 1;
#endif
#if defined(Q_WS_QWS) || defined(Q_WS_MAC)
    QRegion mask;				// widget mask
#endif
#ifndef QT_NO_STYLE
    QStyle* style;
#endif
    QRect micro_focus_hint;			// micro focus hint
    QSizePolicy size_policy;
};

class QWidgetPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC( QWidget );

public:
    QWidgetPrivate() :
	QObjectPrivate(), extra(0), focus_child(0)
#ifndef QT_NO_LAYOUT
        ,layout(0)
#endif
#ifndef QT_NO_PALETTE
        ,fg_role(QPalette::Foreground),
	bg_role(QPalette::Background)
#endif
    {high_attributes[0] = 0;}
    ~QWidgetPrivate();

    QWExtra	*extraData() const;
    QTLWExtra	*topData() const;

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

    void setFont_syshelper( QFont *f = 0 );
    void setFont_helper(const QFont &);
    void resolveFont();


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

#if defined(Q_WS_X11)
    void createInputContext();
    void destroyInputContext();
    void focusInputContext();
    void checkChildrenDnd();
    void removePendingPaintEvents();
    QRegion invalidated_region;

    void setWindowRole(const char *role);
#endif

    QWExtra *extra;
    QWidget *focus_next;
    QWidget *focus_child;
#ifndef QT_NO_LAYOUT
    QLayout *layout;
#endif

    // ### TODO: reorganize private/extra/topextra to safe memory
    QPointer<QWidget> compositeChildGrab;
    QString toolTip, statusTip, whatsThis;

#ifndef QT_NO_PALETTE
    QPalette::ColorRole fg_role : 8;
    QPalette::ColorRole bg_role : 8;
#endif
    uint high_attributes[1]; // the low ones are in QWidget::widget_attributes
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

#if defined (Q_WS_X11) || defined (Q_WS_QWS)
extern int qt_widget_tlw_gravity;
#endif





#endif

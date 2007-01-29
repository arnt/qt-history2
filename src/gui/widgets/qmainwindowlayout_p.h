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

#ifndef QDYNAMICMAINWINDOWLAYOUT_P_H
#define QDYNAMICMAINWINDOWLAYOUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmainwindow.h"

#ifndef QT_NO_MAINWINDOW

#include "QtGui/qlayout.h"
#include "QtCore/qvector.h"
#include "QtCore/qset.h"
#include "private/qlayoutengine_p.h"

#include "qdockarealayout_p.h"
#include "qtoolbararealayout_p.h"

class QToolBar;
class QWidgetAnimator;
class QTabBar;
class QRubberBand;

/* This data structure represents the state of all the tool-bars and dock-widgets. It's value based
   so it can be easilly copied into a temporary variable. All operations are performed without moving
   any widgets. Only when we are sure we have the desired state, we call apply(), which moves the
   widgets.
*/

class QMainWindowLayoutState
{
public:
    QRect rect;
    QMainWindow *mainWindow;

    QMainWindowLayoutState(QMainWindow *win);

#ifndef QT_NO_TOOLBAR
    QToolBarAreaLayout toolBarAreaLayout;
#endif

#ifndef QT_NO_DOCKWIDGET
    QDockAreaLayout dockAreaLayout;
#else
    QLayoutItem *centralWidgetItem;
    QRect centralWidgetRect;
#endif

    void apply(bool animated);
    void deleteAllLayoutItems();
    void deleteCentralWidgetItem();

    QSize sizeHint() const;
    QSize minimumSize() const;
    void fitLayout();

    QLayoutItem *itemAt(int index, int *x) const;
    QLayoutItem *takeAt(int index, int *x);
    QList<int> indexOf(QWidget *widget) const;
    QLayoutItem *item(QList<int> path);
    QRect itemRect(QList<int> path) const;
    QRect gapRect(QList<int> path) const; // ### get rid of this, use itemRect() instead

    void setCentralWidget(QWidget *widget);
    QWidget *centralWidget() const;

    QList<int> gapIndex(QWidget *widget, const QPoint &pos, bool dockNestingEnabled) const;
    bool insertGap(QList<int> path, QLayoutItem *item);
    void remove(QList<int> path);
    void clear();
    bool isValid() const;

    QLayoutItem *plug(QList<int> path);
    QLayoutItem *unplug(QList<int> path);

    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream, const QMainWindowLayoutState &oldState);
};

class Q_AUTOTEST_EXPORT QMainWindowLayout : public QLayout
{
    Q_OBJECT

public:
    QMainWindowLayoutState layoutState, savedState;

    explicit QMainWindowLayout(QMainWindow *mainwindow);
    ~QMainWindowLayout();

    // status bar

    QLayoutItem *statusbar;

#ifndef QT_NO_STATUSBAR
    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *sb);
#endif

    // central widget

    QWidget *centralWidget() const;
    void setCentralWidget(QWidget *cw);

    // toolbars

#ifndef QT_NO_TOOLBAR
    void addToolBarBreak(Qt::ToolBarArea area);
    void insertToolBarBreak(QToolBar *before);
    void removeToolBarBreak(QToolBar *before);

    void addToolBar(Qt::ToolBarArea area, QToolBar *toolbar, bool needAddChildWidget = true);
    void insertToolBar(QToolBar *before, QToolBar *toolbar);
    Qt::ToolBarArea toolBarArea(QToolBar *toolbar) const;
    bool toolBarBreak(QToolBar *toolBar) const;
    void getStyleOptionInfo(QStyleOptionToolBar *option, QToolBar *toolBar) const;
#endif

    // dock widgets

#ifndef QT_NO_DOCKWIDGET
    bool dockNestingEnabled;

    void setCorner(Qt::Corner corner, Qt::DockWidgetArea area);
    Qt::DockWidgetArea corner(Qt::Corner corner) const;
    void addDockWidget(Qt::DockWidgetArea area,
                       QDockWidget *dockwidget,
                       Qt::Orientation orientation);
    void splitDockWidget(QDockWidget *after,
                         QDockWidget *dockwidget,
                         Qt::Orientation orientation);
    void tabifyDockWidget(QDockWidget *first, QDockWidget *second);
    Qt::DockWidgetArea dockWidgetArea(QDockWidget *dockwidget) const;
    void raise(QDockWidget *widget);
    void setVerticalTabsEnabled(bool enabled);
    bool verticalTabsEnabled() const;

#ifndef QT_NO_TABBAR
    QTabBar *getTabBar();
    QSet<QTabBar*> usedTabBars;
    QList<QTabBar*> unusedTabBars;
#endif

    // separators

    QList<int> movingSeparator;
    QPoint movingSeparatorOrigin, movingSeparatorPos;
    QTimer *separatorMoveTimer;
    QVector<QLayoutStruct> separatorMoveCache;

    bool startSeparatorMove(const QPoint &pos);
    bool separatorMove(const QPoint &pos);
    bool endSeparatorMove(const QPoint &pos);
    void keepSize(QDockWidget *w);
#endif // QT_NO_DOCKWIDGET

    // save/restore

    enum { // sentinel values used to validate state data
        VersionMarker = 0xff,
    };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream);

    // QLayout interface

    void addItem(QLayoutItem *item);
    void setGeometry(const QRect &r);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    int count() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    mutable QSize szHint;
    mutable QSize minSize;
    void invalidate();

    // animations

    QWidgetAnimator *widgetAnimator;
    bool animationEnabled;
    QList<int> currentGapPos;
    QRect currentGapRect;
    QWidget *pluggingWidget;
    QRubberBand *gapIndicator;

    typedef QInternal::DockPosition DockPosition;

    QList<int> hover(QLayoutItem *widgetItem, const QPoint &mousePos);
    bool plug(QLayoutItem *widgetItem);
    QLayoutItem *unplug(QWidget *widget);
    void revert(QLayoutItem *widgetItem);
    void updateGapIndicator();
    void paintDropIndicator(QPainter *p, QWidget *widget, const QRegion &clip);
    void applyState(QMainWindowLayoutState &newState, bool animate = true);
    void restore();

private slots:
    void animationFinished(QWidget *widget);
    void allAnimationsFinished();
#ifndef QT_NO_DOCKWIDGET
    void doSeparatorMove();
#ifndef QT_NO_TABBAR
    void tabChanged();
#endif
#endif
};

#endif // QT_NO_MAINWINDOW

static inline int pick(Qt::Orientation o, const QPoint &pos)
{ return o == Qt::Horizontal ? pos.x() : pos.y(); }

static inline int pick(Qt::Orientation o, const QSize &size)
{ return o == Qt::Horizontal ? size.width() : size.height(); }

static inline int &rpick(Qt::Orientation o, QPoint &pos)
{ return o == Qt::Horizontal ? pos.rx() : pos.ry(); }

static inline int &rpick(Qt::Orientation o, QSize &size)
{ return o == Qt::Horizontal ? size.rwidth() : size.rheight(); }

static inline QSizePolicy::Policy pick(Qt::Orientation o, const QSizePolicy &policy)
{ return o == Qt::Horizontal ? policy.horizontalPolicy() : policy.verticalPolicy(); }

static inline int perp(Qt::Orientation o, const QPoint &pos)
{ return o == Qt::Vertical ? pos.x() : pos.y(); }

static inline int perp(Qt::Orientation o, const QSize &size)
{ return o == Qt::Vertical ? size.width() : size.height(); }

static inline int &rperp(Qt::Orientation o, QPoint &pos)
{ return o == Qt::Vertical ? pos.rx() : pos.ry(); }

static inline int &rperp(Qt::Orientation o, QSize &size)
{ return o == Qt::Vertical ? size.rwidth() : size.rheight(); }

// the rest of QMainWindow uses this instead...
static inline int pick_perp(Qt::Orientation o, const QPoint &p)
{ return o == Qt::Vertical ? p.x() : p.y(); }
static inline int pick_perp(Qt::Orientation o, const QSize &s)
{ return o == Qt::Vertical ? s.width() : s.height(); }

#endif // QDYNAMICMAINWINDOWLAYOUT_P_H

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

#ifndef QDOCKAREALAYOUT_P_H
#define QDOCKAREALAYOUT_P_H

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

#include "QtCore/qlist.h"
#include "QtCore/qrect.h"
#include "QtCore/qpair.h"
#include "QtCore/qlist.h"
#include "QtGui/qlayout.h"

#ifndef QT_NO_DOCKWIDGET

class QLayoutItem;
class QWidget;
class QLayoutItem;
class QDockAreaLayoutInfo;
class QDockWidget;
class QMainWindow;
class QWidgetAnimator;
class QMainWindowLayout;
struct QLayoutStruct;
class QTabBar;

struct QDockAreaLayoutItem
{
    QDockAreaLayoutItem(QLayoutItem *_widgetItem = 0);
    QDockAreaLayoutItem(QDockAreaLayoutInfo *_subinfo);
    QDockAreaLayoutItem(const QDockAreaLayoutItem &other);
    ~QDockAreaLayoutItem();

    QDockAreaLayoutItem &operator = (const QDockAreaLayoutItem &other);

    bool skip() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize sizeHint() const;
    bool expansive(Qt::Orientation o) const;

    QLayoutItem *widgetItem;
    QDockAreaLayoutInfo *subinfo;
    int pos;
    int size;
    bool gap;
    bool keep_size;
};

class Q_AUTOTEST_EXPORT QDockAreaLayoutInfo
{
public:
    QDockAreaLayoutInfo();
    QDockAreaLayoutInfo(int _sep, QInternal::DockPosition _dockPos, Qt::Orientation _o,
                        int tbhape, QMainWindow *window);

    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize sizeHint() const;
    QSize size() const;

    bool insertGap(QList<int> path, QLayoutItem *dockWidgetItem);
    QLayoutItem *plug(QList<int> path);
    QLayoutItem *unplug(QList<int> path);
    enum TabMode { NoTabs, AllowTabs, ForceTabs };
    QList<int> gapIndex(const QPoint &pos, bool nestingEnabled,
                            TabMode tabMode) const;
    void remove(QList<int> path);
    void unnest(int index);
    void split(int index, Qt::Orientation orientation, QLayoutItem *dockWidgetItem);
    void tab(int index, QLayoutItem *dockWidgetItem);
    QDockAreaLayoutItem &item(QList<int> path);
    QDockAreaLayoutInfo *info(QList<int> path);
    QDockAreaLayoutInfo *info(QWidget *widget);

    enum { // sentinel values used to validate state data
        SequenceMarker = 0xfc,
        TabMarker = 0xfa,
        WidgetMarker = 0xfb
    };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream, QList<QDockWidget*> &widgets);

    void fitItems();
    bool expansive(Qt::Orientation o) const;
    int changeSize(int index, int size, bool below);
    QRect itemRect(int index) const;
    QRect itemRect(QList<int> path) const;
    QRect separatorRect(int index) const;
    QRect separatorRect(QList<int> path) const;

    void clear();
    bool isEmpty() const;
    QList<int> findSeparator(const QPoint &pos) const;
    int next(int idx) const;
    int prev(int idx) const;

    QList<int> indexOf(QWidget *widget) const;

    void apply(bool animate);

    void paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip,
                            const QPoint &mouse) const;
    QRegion separatorRegion() const;
    int separatorMove(int index, int delta, QVector<QLayoutStruct> *cache);

    QLayoutItem *itemAt(int *x, int index) const;
    QLayoutItem *takeAt(int *x, int index);
    void deleteAllLayoutItems();

    QMainWindowLayout *mainWindowLayout() const;

    int sep;
    QInternal::DockPosition dockPos;
    Qt::Orientation o;
    QRect rect;
    QMainWindow *mainWindow;
    QList<QDockAreaLayoutItem> item_list;

#ifndef QT_NO_TABBAR
    quintptr currentTabId() const;
    void setCurrentTab(QWidget *widget);
    void setCurrentTabId(quintptr id);
    QRect tabContentRect() const;
    bool tabbed;
    QTabBar *tabBar;
    QSize tabBarMin, tabBarHint;
    int tabBarShape;
    bool tabBarVisible;

    void updateTabBar() const;
    void setTabBarShape(int shape);
    QSize tabBarMinimumSize() const;
    QSize tabBarSizeHint() const;

    QSet<QTabBar*> usedTabBars() const;
#endif // QT_NO_TABBAR
};

class Q_AUTOTEST_EXPORT QDockAreaLayout
{
public:
    enum { EmptyDropAreaSize = 80 }; // when a dock area is empty, how "wide" is it?

    Qt::DockWidgetArea corners[4]; // use a Qt::Corner for indexing
    QRect rect;
    QLayoutItem *centralWidgetItem;
    QMainWindow *mainWindow;
    QRect centralWidgetRect;
    QDockAreaLayout(QMainWindow *win);
    QDockAreaLayoutInfo docks[4];
    int sep; // separator extent

    bool isValid() const;

    enum { DockWidgetStateMarker = 0xfd };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream, const QList<QDockWidget*> &widgets);

    QList<int> indexOf(QDockWidget *dockWidget) const;
    QList<int> gapIndex(const QPoint &pos) const;
    QList<int> findSeparator(const QPoint &pos) const;

    QDockAreaLayoutItem &item(QList<int> path);
    QDockAreaLayoutInfo *info(QList<int> path);
    const QDockAreaLayoutInfo *info(QList<int> path) const;
    QDockAreaLayoutInfo *info(QWidget *widget);
    QRect itemRect(QList<int> path) const;
    QRect separatorRect(int index) const;
    QRect separatorRect(QList<int> path) const;

    bool insertGap(QList<int> path, QLayoutItem *dockWidgetItem);
    QLayoutItem *plug(QList<int> path);
    QLayoutItem *unplug(QList<int> path);
    void remove(QList<int> path);

    void fitLayout();

    void clear();

    QSize sizeHint() const;
    QSize minimumSize() const;

    void addDockWidget(QInternal::DockPosition pos, QDockWidget *dockWidget, Qt::Orientation orientation);
    void splitDockWidget(QDockWidget *after, QDockWidget *dockWidget,
                         Qt::Orientation orientation);
    void tabifyDockWidget(QDockWidget *first, QDockWidget *second);

    void apply(bool animate);

    void paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip,
                            const QPoint &mouse) const;
    QRegion separatorRegion() const;
    int separatorMove(QList<int> separator, const QPoint &origin, const QPoint &dest,
                        QVector<QLayoutStruct> *cache);

    QLayoutItem *itemAt(int *x, int index) const;
    QLayoutItem *takeAt(int *x, int index);
    void deleteAllLayoutItems();

    void getGrid(QVector<QLayoutStruct> *ver_struct_list,
                    QVector<QLayoutStruct> *hor_struct_list);
    void setGrid(QVector<QLayoutStruct> *ver_struct_list,
                    QVector<QLayoutStruct> *hor_struct_list);

    QRect gapRect(QList<int> path) const;

    void keepSize(QDockWidget *w);

    QSet<QTabBar*> usedTabBars() const;
};

#endif // QT_NO_QDOCKWIDGET

#endif // QDOCKAREALAYOUT_P_H

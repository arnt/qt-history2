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

#ifndef QTOOLBARAREALAYOUT_P_H
#define QTOOLBARAREALAYOUT_P_H

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

#include <QList>
#include <QSize>
#include <QRect>

class QToolBar;
class QLayoutItem;
class QMainWindow;
class QStyleOptionToolBar;

class QToolBarAreaLayoutItem
{
public:
    QToolBarAreaLayoutItem(QLayoutItem *item = 0)
        : widgetItem(item), pos(0), size(-1), gap(false) {}

    bool skip() const;
    QSize minimumSize() const;
    QSize sizeHint() const;

    QLayoutItem *widgetItem;
    int pos;
    int size;
    bool gap;
};

class QToolBarAreaLayoutLine
{
public:
    QToolBarAreaLayoutLine(Qt::Orientation orientation);

    QSize sizeHint() const;
    QSize minimumSize() const;

    void fitLayout();
    bool skip() const;

    QRect rect;
    Qt::Orientation o;

    QList<QToolBarAreaLayoutItem> toolBarItems;
};

class QToolBarAreaLayoutInfo
{
public:
    enum { EmptyDockAreaSize = 80 }; // when a dock area is empty, how "wide" is it?

    QToolBarAreaLayoutInfo(QInternal::DockPosition pos = QInternal::TopDock);

    QList<QToolBarAreaLayoutLine> lines;

    QSize sizeHint() const;
    QSize minimumSize() const;

    void fitLayout();

    void insertToolBar(QToolBar *before, QToolBar *toolBar);
    void removeToolBar(QToolBar *toolBar);
    void insertToolBarBreak(QToolBar *before);
    void removeToolBarBreak(QToolBar *before);

    QList<int> gapIndex(const QPoint &pos) const;
    bool insertGap(QList<int> path, QLayoutItem *item);
    void clear();
    QRect itemRect(QList<int> path) const;
    QRect appendLineDropRect() const;

    QRect rect;
    Qt::Orientation o;
    QInternal::DockPosition dockPos;
};

class QToolBarAreaLayout
{
public:
    enum { // sentinel values used to validate state data
        ToolBarStateMarker = 0xfe,
        ToolBarStateMarkerEx = 0xfc
    };

    QRect rect;
    QMainWindow *mainWindow;
    QToolBarAreaLayoutInfo docks[4];

    QToolBarAreaLayout(QMainWindow *win);

    QRect fitLayout();

    QSize minimumSize(const QSize &centerMin) const;
    QSize sizeHint(const QSize &centerHint) const;
    void apply(bool animate);

    QLayoutItem *itemAt(int *x, int index) const;
    QLayoutItem *takeAt(int *x, int index);
    void deleteAllLayoutItems();

    void insertToolBar(QToolBar *before, QToolBar *toolBar);
    void removeToolBar(QToolBar *toolBar);
    void addToolBar(QInternal::DockPosition pos, QToolBar *toolBar);
    void insertToolBarBreak(QToolBar *before);
    void removeToolBarBreak(QToolBar *before);
    void addToolBarBreak(QInternal::DockPosition pos);

    QInternal::DockPosition findToolBar(QToolBar *toolBar) const;
    bool toolBarBreak(QToolBar *toolBar) const;

    void getStyleOptionInfo(QStyleOptionToolBar *option, QToolBar *toolBar) const;

    QList<int> indexOf(QToolBar *toolBar) const;
    QList<int> gapIndex(const QPoint &pos) const;
    bool insertGap(QList<int> path, QLayoutItem *item);
    void remove(QList<int> path);
    void clear();
    QToolBarAreaLayoutItem &item(QList<int> path);
    QRect itemRect(QList<int> path) const;
    QLayoutItem *plug(QList<int> path);
    QLayoutItem *unplug(QList<int> path);

    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream, const QList<QToolBar*> &toolBars);
    bool isEmpty() const;
};

#endif // QTOOLBARAREALAYOUT_P_H

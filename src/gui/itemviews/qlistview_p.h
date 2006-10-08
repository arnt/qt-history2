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

#ifndef QLISTVIEW_P_H
#define QLISTVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractitemview_p.h"
#include "qrubberband.h"
#include "qbitarray.h"
#include "qbsptree_p.h"
#include <limits.h>

#ifndef QT_NO_LISTVIEW

class QListViewItem
{
    friend class QListViewPrivate;
    friend class QStaticListViewPrivate;
    friend class QDynamicListViewPrivate;
public:
    inline QListViewItem()
        : x(-1), y(-1), w(0), h(0), indexHint(-1), visited(0xffff) {}
    inline QListViewItem(const QListViewItem &other)
        : x(other.x), y(other.y), w(other.w), h(other.h),
          indexHint(other.indexHint), visited(other.visited) {}
    inline QListViewItem(QRect r, int i)
        : x(r.x()), y(r.y()), w(qMin(r.width(), SHRT_MAX)), h(qMin(r.height(), SHRT_MAX)),
          indexHint(i), visited(0xffff) {}
    inline bool operator==(const QListViewItem &other) const {
        return (x == other.x && y == other.y && w == other.w && h == other.h &&
                indexHint == other.indexHint); }
    inline bool operator!=(const QListViewItem &other) const
        { return !(*this == other); }
    inline bool isValid() const
        { return (x > -1) && (y > -1) && (w > 0) && (h > 0) && (indexHint > -1); }
    inline void invalidate()
        { x = -1; y = -1; w = 0; h = 0; }
    inline void resize(const QSize &size)
        { w = qMin(size.width(), SHRT_MAX); h = qMin(size.height(), SHRT_MAX); }
    inline void move(const QPoint &position)
        { x = position.x(); y = position.y(); }
    inline int width() const { return w; }
    inline int height() const { return h; }
private:
    inline QRect rect() const
        { return QRect(x, y, w, h); }
    int x, y;
    short w, h;
    mutable int indexHint;
    uint visited;
};

struct LayoutInformation
{
    QRect bounds;
    QSize grid;
    int spacing;
    int first;
    int last;
    bool wrap;
    QListView::Flow flow;
};

class QStaticListViewPrivate
{
    friend class QListViewPrivate;
public:
    QVector<int> flowPositions;
    QVector<int> segmentPositions;
    QVector<int> segmentStartRows;

    QSize contentsSize;

    // used when laying out in batches
    int batchStartRow;
    int batchSavedDeltaSeg;
    int batchSavedPosition;

    bool wrap;
    int spacing;
    QSize gridSize;

    bool doBatchedItemLayout(const LayoutInformation &info, int max);

    QPoint initStaticLayout(const LayoutInformation &info);
    void doStaticLayout(const LayoutInformation &info);
    void intersectingStaticSet(const QRect &area) const;

    int itemIndex(const QListViewItem &item) const;

    int perItemScrollingPageSteps(int length, int bounds) const;

    int perItemScrollToValue(int index, int value, int height,
                             QAbstractItemView::ScrollHint hint,
                             Qt::Orientation orientation) const;

    QRect mapToViewport(const QRect &rect, const QSize &viewportSize,
                        Qt::ScrollBarPolicy horizontalPolicy,
                        bool rightToLeft) const;

    QListViewItem indexToListViewItem(const QModelIndex &index) const;

    void scrollContentsBy(int &dx, int &dy, bool wrap, QListView::Flow flow);

    int verticalPerItemValue(int itemIndex, int verticalValue, int areaHeight,
                       bool above, bool below, QListView::ScrollHint hint) const;
    int horizontalPerItemValue(int itemIndex, int horizontalValue, int areaWidth,
                       bool leftOf, bool rightOf, QListView::ScrollHint hint) const;

    QListViewPrivate *dd;
    QListView *qq;
};

class QDynamicListViewPrivate
{
    friend class QListViewPrivate;
public:
    QBspTree tree;
    QVector<QListViewItem> items;
    QBitArray moved;

    QSize contentsSize;

    QVector<QModelIndex> draggedItems; // indices to the tree.itemVector
    mutable QPoint draggedItemsPos;

    // used when laying out in batches
    int batchStartRow;
    int batchSavedDeltaSeg;

    QRect elasticBand;

    bool wrap;
    int spacing;
    QSize gridSize;

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    bool doBatchedItemLayout(const LayoutInformation &info, int max);

    void initBspTree(const QSize &contents);
    QPoint initDynamicLayout(const LayoutInformation &info);
    void doDynamicLayout(const LayoutInformation &info);
    void intersectingDynamicSet(const QRect &area) const;

    static void addLeaf(QVector<int> &leaf, const QRect &area,
                        uint visited, QBspTree::Data data);

    void insertItem(int index);
    void removeItem(int index);
    void moveItem(int index, const QPoint &dest);

    int itemIndex(const QListViewItem &item) const;

    void createItems(int to);
    void drawItems(QPainter *painter, const QVector<QModelIndex> &indexes) const;
    QRect itemsRect(const QVector<QModelIndex> &indexes) const;

    QPoint draggedItemsDelta() const;
    QRect draggedItemsRect() const;

    QPoint snapToGrid(const QPoint &pos) const;

    void scrollElasticBandBy(int dx, int dy);

    QListViewItem indexToListViewItem(const QModelIndex &index) const;

    QListViewPrivate *dd;
    QListView *qq;
};

class QListViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QListView)
public:
    QDynamicListViewPrivate dynamicListView;
    QStaticListViewPrivate staticListView;

    QListViewPrivate();
    ~QListViewPrivate() {}

    void clear();
    void prepareItemsLayout();

    bool doItemsLayout(int num);
    void doItemsLayout(const QRect &bounds, const QModelIndex &first, const QModelIndex &last);

    inline void intersectingSet(const QRect &area, bool doLayout = true) const {
        if (doLayout) executePostedLayout();
        QRect a = (q_func()->isRightToLeft() ? flipX(area.normalized()) : area.normalized());
        if (movement == QListView::Static) staticListView.intersectingStaticSet(a);
        else dynamicListView.intersectingDynamicSet(a);
    }

    inline int batchStartRow() const
        { return (movement == QListView::Static
          ? staticListView.batchStartRow : dynamicListView.batchStartRow); }

    inline QSize contentsSize() const
        { return (movement == QListView::Static
          ? staticListView.contentsSize : dynamicListView.contentsSize); }
    inline void setContentsSize(int w, int h)
        { staticListView.contentsSize = QSize(w, h);
          dynamicListView.contentsSize = QSize(w, h); }

    inline void setGridSize(const QSize &size)
        { staticListView.gridSize = size;
          dynamicListView.gridSize = size; }
    inline QSize gridSize() const
        { return (movement == QListView::Static
          ? staticListView.gridSize : dynamicListView.gridSize); }

    inline void setWrapping(bool wrap)
        { staticListView.wrap = wrap;
          dynamicListView.wrap = wrap; }

    inline bool isWrapping() const
        { return (movement == QListView::Static
          ? staticListView.wrap : dynamicListView.wrap); }

    inline void setSpacing(int space)
        { staticListView.spacing = space;
          dynamicListView.spacing = space; }

    inline int spacing() const
        { return (movement == QListView::Static
          ? staticListView.spacing : dynamicListView.spacing); }

    inline int flipX(int x) const
        { return qMax(viewport->width(), contentsSize().width()) - x; }
    inline QPoint flipX(const QPoint &p) const
        { return QPoint(flipX(p.x()), p.y()); }
    inline QRect flipX(const QRect &r) const
        { return QRect(flipX(r.x()) - r.width(), r.y(), r.width(), r.height()); }

    inline QRect viewItemRect(const QListViewItem &item) const
        { if (q_func()->isRightToLeft()) return flipX(item.rect()); return item.rect(); }

    int itemIndex(const QListViewItem &item) const;
    QListViewItem indexToListViewItem(const QModelIndex &index) const;
    inline QModelIndex listViewItemToIndex(const QListViewItem &item) const
        { return model->index(itemIndex(item), column, root); }

    QRect mapToViewport(const QRect &rect) const;

    QModelIndex closestIndex(const QPoint &target, const QVector<QModelIndex> &candidates) const;
    QSize itemSize(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    bool selectionAllowed(const QModelIndex &index) const
        { if (movement == QListView::Static) return index.isValid(); return true; }

    QStyleOptionViewItemV2 viewOptionsV2() const;
    int horizontalScrollToValue(const QModelIndex &index, const QRect &rect, QListView::ScrollHint hint) const;
    int verticalScrollToValue(const QModelIndex &index, const QRect &rect, QListView::ScrollHint hint) const;

    //int spacing;
    QListView::Flow flow;
    QListView::Movement movement;
    QListView::ResizeMode resizeMode;
    QListView::LayoutMode layoutMode;
    QListView::ViewMode viewMode;

    // the properties controlling the
    // icon- or list-view modes
    enum ModeProperties {
        Wrap = 1,
        Spacing = 2,
        GridSize = 4,
        Flow = 8,
        Movement = 16,
        ResizeMode = 32
    };

    uint modeProperties : 8;

    QRect layoutBounds;

    // used for intersecting set
    mutable QVector<QModelIndex> intersectVector;

    // timers
    QBasicTimer batchLayoutTimer;

    // used for hidden items
    QVector<int> hiddenRows;

    int column;
    bool uniformItemSizes;
    mutable QSize cachedItemSize;
    int batchSize;
    bool wrapItemText;
};

#endif // QT_NO_LISTVIEW

#endif // QLISTVIEW_P_H

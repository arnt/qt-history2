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

#ifndef QLISTVIEW_P_H
#define QLISTVIEW_P_H

#include <private/qabstractitemview_p.h>
#include <qrubberband.h>

template <class T>
class QBinTree
{
public:
    struct Node
    {
        enum Type { None = 0, VerticalPlane = 1, HorizontalPlane = 2, Both = 3 };
        inline Node() : pos(0), type(None) {}
        uint pos : 30;
        uint type : 2;
    };

    struct Data
    {
        Data(void *p) : ptr(p) {}
        Data(int n) : i(n) {}
        union {
            void *ptr;
            int i;
        };
    };

    typedef void callback(QVector<int> &leaf, const QRect &area, uint visited, QBinTree::Data data);

    inline QBinTree() : depth_(6), visited(0) {}

    void create(int n);
    void destroy();
    static void insert(QVector<int> &leaf, const QRect &area, uint visited, QBinTree::Data data);
    static void remove(QVector<int> &leaf, const QRect &area, uint visited, QBinTree::Data data);

    inline void climbTree(const QRect &rect, callback *function, QBinTree::Data data);

    inline void init(const QRect &area, typename QBinTree::Node::Type type);
    inline void reserve(int size);

    inline int itemCount() const;
    inline const T &const_item(int idx) const;
    inline T &item(int idx);
    inline T *itemPtr(int idx);

    inline void setItemPosition(int x, int y, int idx);

    inline void appendItem(T &item);

    inline void insertItem(T &item, const QRect &rect, int idx);
    inline void removeItem(const QRect &rect, int idx);
    inline void moveItem(const QPoint &dest, const QRect &rect, int idx);

    inline int leafCount() const;
    inline const QVector<int> &const_leaf(int idx) const;
    inline QVector<int> &leaf(int idx);
    inline void clearLeaf(int idx);

    inline int nodeCount() const;
    inline const Node &node(int idx) const;
    inline int parentIndex(int idx) const;
    inline int firstChildIndex(int idx) const;

private:
    void climbTree(const QRect &rect, callback *function, QBinTree::Data data, int index);
    void init(const QRect &area, int depth, typename QBinTree::Node::Type type, int index);

    uint depth_ : 8;
    mutable uint visited : 16;
    QVector<T> itemVector;
    QVector<Node> nodeVector;
    mutable QVector< QVector<int> > leafVector; // the leaves are just indices into the itemVector
};

template <class T>
void QBinTree<T>::climbTree(const QRect &rect, callback *function, QBinTree::Data data)
{
    ++visited;
    climbTree(rect, function, data, 0);
}

template <class T>
void QBinTree<T>::init(const QRect &area, typename QBinTree::Node::Type type)
{
    init(area, depth_, type, 0);
}

template <class T>
void QBinTree<T>::reserve(int size)
{
    itemVector.reserve(size);
}

template <class T>
int QBinTree<T>::itemCount() const
{
    return itemVector.count();
}

template <class T>
const T &QBinTree<T>::const_item(int idx) const
{
    return itemVector[idx];
}

template <class T>
T &QBinTree<T>::item(int idx)
{
    return itemVector[idx];
}

template <class T>
T *QBinTree<T>::itemPtr(int idx)
{
    return &itemVector[idx];
}

template <class T>
void QBinTree<T>::setItemPosition(int x, int y, int idx)
{
    item(idx).x = x;
    item(idx).y = y;
}

template <class T>
void QBinTree<T>::appendItem(T &item)
{
    itemVector.append(item);
}

template <class T>
void QBinTree<T>::insertItem(T &item, const QRect &rect, int idx)
{
    itemVector.insert(idx + 1, 1, item); // insert after idx
    climbTree(rect, &insert, reinterpret_cast<void *>(idx), 0);
}

template <class T>
void QBinTree<T>::removeItem(const QRect &rect, int idx)
{
    climbTree(rect, &remove, reinterpret_cast<void *>(idx), 0);
    itemVector.remove(idx, 1);
}

template <class T>
void QBinTree<T>::moveItem(const QPoint &dest, const QRect &rect, int idx)
{
    climbTree(rect, &remove, reinterpret_cast<void *>(idx), 0);
    item(idx).x = dest.x();
    item(idx).y = dest.y();
    climbTree(QRect(dest, rect.size()), &insert, reinterpret_cast<void *>(idx), 0);
}

template <class T>
int QBinTree<T>::leafCount() const
{
    return leafVector.count();
}

template <class T>
const QVector<int> &QBinTree<T>::const_leaf(int idx) const
{
    return leafVector.at(idx);
}

template <class T>
QVector<int> &QBinTree<T>::leaf(int idx)
{
    return leafVector[idx];
}

template <class T>
void QBinTree<T>::clearLeaf(int idx) { leafVector[idx].clear(); }

template <class T>
int QBinTree<T>::nodeCount() const
{
    return nodeVector.count();
}

template <class T>
const typename QBinTree<T>::Node &QBinTree<T>::node(int idx) const
{
    return nodeVector[idx];
}

template <class T>
int QBinTree<T>::parentIndex(int idx) const
{
    return (idx & 1) ? ((idx - 1) / 2) : ((idx - 2) / 2);
}

template <class T>
int QBinTree<T>::firstChildIndex(int idx) const
{
    return ((idx * 2) + 1);
}

class QListViewItem
{
public:
    inline QListViewItem()
        : x(-1), y(-1), w(-1), h(-1), indexHint(-1), visited(0xffff) {}
    inline QListViewItem(const QListViewItem &other)
        : x(other.x), y(other.y), w(other.w), h(other.h),
          indexHint(other.indexHint), visited(other.visited) {}
    inline QListViewItem(QRect r, int i)
        : x(r.x()), y(r.y()), w(r.width()), h(r.height()),
          indexHint(i), visited(0xffff) {}
    inline bool operator==(const QListViewItem &other) const {
        return (x == other.x && y == other.y && w == other.w && h == other.h &&
                indexHint == other.indexHint); }
    inline bool operator!=(const QListViewItem &other) const
        { return !(*this == other); }
    inline QRect rect()
        { return QRect(x, y, w, h); }
    inline bool isValid()
        { return (x > -1) && (y > -1) && (w > -1) && (h > -1) && (indexHint > -1); }

    int x, y;
    short w, h;
    mutable int indexHint;
    uint visited : 16;
};

class QListViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QListView)
public:
    QListViewPrivate();
    ~QListViewPrivate() {}

    void init();
    void prepareItemsLayout();
    void intersectingDynamicSet(const QRect &area) const;
    void intersectingStaticSet(const QRect &area) const;
    void createItems(int to);
    void drawItems(QPainter *painter, const QVector<QModelIndex> &indexes) const;
    QRect itemsRect(const QVector<QModelIndex> &indexes) const;

    QListViewItem indexToListViewItem(const QModelIndex &index) const;
    inline QModelIndex listViewItemToIndex(const QListViewItem item) const
    { return q_func()->model()->index(itemIndex(item), 0, q_func()->root()); }
    int itemIndex(const QListViewItem item) const;
    static void addLeaf(QVector<int> &leaf, const QRect &area,
                        uint visited, QBinTree<QListViewItem>::Data data);
    void createStaticRow(int &x, int &y, int &dy, int &wraps, int i,
                         const QRect &bounds, int spacing, int delta);
    void createStaticColumn(int &x, int &y, int &dx, int &wraps, int i,
                            const QRect &bounds, int spacing, int delta);
    void initStaticLayout(int &x, int &y, int first, const QRect &bounds, int spacing);

    void insertItem(int index, QListViewItem &item);
    void removeItem(int index);
    void moveItem(int index, const QPoint &dest);

    QPoint snapToGrid(const QPoint &pos) const;
    QRect mapToViewport(const QRect &rect) const;
    QPoint draggedItemsDelta() const;
    QRect draggedItemsRect() const;

    QModelIndex closestIndex(const QPoint &target, const QVector<QModelIndex> &candidates) const;

    bool wrap;
    int spacing;
    QSize gridSize;
    QListView::Flow flow;
    QListView::Movement movement;
    QListView::IconSize iconSize;
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
        IconSize = 32,
        ResizeMode = 64
    };

    uint modeProperties : 8;
    
    int layoutStart;
    int translate;
    QRect layoutBounds;
    QSize contentsSize;

    // used for intersecting set
    mutable QVector<QModelIndex> intersectVector;

    // used when items are movable
    QBinTree<QListViewItem> tree;

    // used when items are static
    QVector<int> xposVector;
    QVector<int> yposVector;
    QVector<int> wrapVector;
    int layoutWraps;
    int layoutTimer;

    // used when dragging
    QVector<QModelIndex> draggedItems; // indices to the tree.itemVector
    mutable QPoint draggedItemsPos;
    QRubberBand *rubberBand;

    // used for hidden items
    QVector<int> hiddenRows;
};

#endif //QLISTVIEW_P_H

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
class BinTree
{
public:
    struct Node
    {
        enum Type { None = 0, VerticalPlane = 1, HorizontalPlane = 2, Both = 3 };
        inline Node() : pos(0), type(None) {}
        uint pos : 30;
        uint type : 2;
    };

    typedef void callback(QVector<int> &leaf, const QRect &area, uint visited, void *data);

    inline BinTree() : depth_(6), visited(0) {}

    void create(int n);
    void destroy();
    static void insert(QVector<int> &leaf, const QRect &area, uint visited, void *data);
    static void remove(QVector<int> &leaf, const QRect &area, uint visited, void *data);

    inline void climbTree(const QRect &rect, callback *function, void *data);

    inline void init(const QRect &area, typename BinTree::Node::Type type);
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
    void climbTree(const QRect &rect, callback *function, void *data, int index);
    void init(const QRect &area, int depth, typename BinTree::Node::Type type, int index);

    uint depth_ : 8;
    mutable uint visited : 16;
    QVector<T> itemVector;
    QVector<Node> nodeVector;
    mutable QVector< QVector<int> > leafVector; // the leaves are just indices into the itemVector
};

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

    inline bool operator==(const QListViewItem &other) const
    {
        return (x == other.x && y == other.y && w == other.w && h == other.h &&
                indexHint == other.indexHint);
    }
    inline bool operator!=(const QListViewItem &other) const { return !(*this == other); }

    inline QRect rect() { return QRect(x, y, w, h); }
    inline bool isValid() { return (x > -1) && (y > -1) && (w > -1) && (h > -1) && (indexHint > -1); }

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
    static void addLeaf(QVector<int> &leaf,
                        const QRect &area, uint visited, void *data);
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

    bool wrap;
    int spacing;
    QSize gridSize;
    QListView::Flow flow;
    QListView::Movement movement;
    QListView::IconSize iconSize;
    QListView::ResizeMode resizeMode;
    QListView::LayoutMode layoutMode;
    QListView::ViewMode viewMode; 

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
    BinTree<QListViewItem> tree;
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
};

#endif //QLISTVIEW_P_H

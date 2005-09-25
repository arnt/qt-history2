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

#ifndef QBSPTREE_P_H
#define QBSPTREE_P_H

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

#include <qvector.h>
#include <qrect.h>

template <class T>
class QBspTree
{
public:

    struct Node
    {
        enum Type { None = 0, VerticalPlane = 1, HorizontalPlane = 2, Both = 3 };
        inline Node() : pos(0), type(None) {}
        uint pos : 30;
        uint type : 2;
    };
    typedef typename Node::Type NodeType;

    struct Data
    {
        Data(void *p) : ptr(p) {}
        Data(int n) : i(n) {}
        union {
            void *ptr;
            int i;
        };
    };
    typedef typename QBspTree::Data QBspTreeData;
    typedef void callback(QVector<int> &leaf, const QRect &area, uint visited, QBspTreeData data);
    
    static void insert(QVector<int> &leaf, const QRect &area, uint visited, QBspTreeData data);
    static void remove(QVector<int> &leaf, const QRect &area, uint visited, QBspTreeData data);

    QBspTree() : depth(6), visited(0) {}

    // tree functions
    void create(int n);
    void destroy();

    void climbTree(const QRect &rect, callback *function, QBspTreeData data);
    void climbTree(const QRect &rect, callback *function, QBspTreeData data, int index);

    void init(const QRect &area, NodeType type);
    void init(const QRect &area, int depth, NodeType type, int index);

    inline void reserve(int size);

    // item functions
    inline int itemCount() const;
    inline const T &item(int idx) const;
    inline T &item(int idx);
    inline T *itemPtr(int idx);

    inline void appendItem(T &item);
    inline void insertItem(T &item, const QRect &rect, int idx);
    inline void removeItem(const QRect &rect, int idx);

    // leaf functions
    inline int leafCount() const;
    inline QVector<int> &leaf(int idx);

    // node functions
    inline int nodeCount() const;
    inline const Node &node(int idx) const;
    inline int parentIndex(int idx) const;
    inline int firstChildIndex(int idx) const;

private:
    uint depth : 8;
    mutable uint visited : 16;
    QVector<T> items;
    QVector<Node> nodes;
    mutable QVector< QVector<int> > leaves; // the leaves are just indices into the items
};

template <class T>
void QBspTree<T>::create(int n)
{
    // simple heuristics to find the best tree depth
    int c;
    for (c = 0; n; ++c)
        n = n / 10;
    depth = c << 1;
    nodes.resize((1 << depth) - 1); // resize to number of nodes
    leaves.resize(1 << depth); // resize to number of leaves
}

template <class T>
void QBspTree<T>::destroy()
{
    leaves.clear();
    nodes.clear();
    items.clear();
}

template <class T>
void QBspTree<T>::insert(QVector<int> &leaf, const QRect &, uint, QBspTreeData data)
{
    leaf.append(data.i);
}

template <class T>
void QBspTree<T>::remove(QVector<int> &leaf, const QRect &, uint, QBspTreeData data)
{
    int i = leaf.indexOf(data.i);
    if (i != -1)
        leaf.remove(i);
}

template <class T>
void QBspTree<T>::climbTree(const QRect &rect, callback *function, QBspTreeData data)
{
    ++visited;
    climbTree(rect, function, data, 0);
}

template <class T>
void QBspTree<T>::climbTree(const QRect &area, callback *function, QBspTreeData data, int index)
{
    int tvs = nodeCount(); // the number of non-leaf-nodes
    if (index >= tvs) { // the index points to a leaf
        if (tvs > 0)
            function(leaf(index - tvs), area, visited, data);
        return;
    }

    typename Node::Type t = (typename Node::Type) node(index).type;

    int pos = node(index).pos;
    int idx = firstChildIndex(index);
    if (t == Node::VerticalPlane) {
        if (area.left() < pos)
            climbTree(area, function, data, idx); // back
        if (area.right() >= pos)
            climbTree(area, function, data, idx + 1); // front
    } else {
        if (area.top() < pos)
            climbTree(area, function, data, idx); // back
        if (area.bottom() >= pos)
            climbTree(area, function, data, idx + 1); // front
    }
}

template <class T>
void QBspTree<T>::init(const QRect &area, NodeType type)
{
    init(area, depth, type, 0);
}

template <class T>
void QBspTree<T>::init(const QRect &area, int depth, NodeType type, int index)
{
    typename Node::Type t = Node::None; // t should never have this value
    if (type == Node::Both) // if both planes are specified, use 2d bsp
        t = (depth & 1) ? Node::HorizontalPlane : Node::VerticalPlane;
    else
        t = type;
    QPoint center = area.center();
    nodes[index].pos = (t == Node::VerticalPlane ? center.x() : center.y());
    nodes[index].type = t;

    QRect front = area;
    QRect back = area;

    if (t == Node::VerticalPlane) {
        front.setLeft(center.x());
        back.setRight(center.x() - 1); // front includes the center
    } else { // t == Node::HorizontalPlane
        front.setTop(center.y());
        back.setBottom(center.y() - 1);
    }

    int idx = firstChildIndex(index);
    if (--depth) {
        init(back, depth, type, idx);
        init(front, depth, type, idx + 1);
    }
}

template <class T>
void QBspTree<T>::reserve(int size)
{
    items.reserve(size);
}

template <class T>
int QBspTree<T>::itemCount() const
{
    return items.count();
}

template <class T>
const T &QBspTree<T>::item(int idx) const
{
    return items.at(idx);
}

template <class T>
T &QBspTree<T>::item(int idx)
{
    return items[idx];
}

template <class T>
T *QBspTree<T>::itemPtr(int idx)
{
    return &items[idx];
}

template <class T>
void QBspTree<T>::appendItem(T &item)
{
    items.append(item);
}

template <class T>
void QBspTree<T>::insertItem(T &item, const QRect &rect, int idx)
{
    items.insert(idx + 1, 1, item); // insert after idx
    climbTree(rect, &insert, Data(idx), 0);
}

template <class T>
void QBspTree<T>::removeItem(const QRect &rect, int idx)
{
    climbTree(rect, &remove, Data(idx), 0);
    items.remove(idx, 1);
}

template <class T>
int QBspTree<T>::leafCount() const
{
    return leaves.count();
}

template <class T>
QVector<int> &QBspTree<T>::leaf(int idx)
{
    return leaves[idx];
}

template <class T>
int QBspTree<T>::nodeCount() const
{
    return nodes.count();
}

template <class T>
const typename QBspTree<T>::Node &QBspTree<T>::node(int idx) const
{
    return nodes.at(idx);
}

template <class T>
int QBspTree<T>::parentIndex(int idx) const
{
    return (idx & 1) ? ((idx - 1) / 2) : ((idx - 2) / 2);
}

template <class T>
int QBspTree<T>::firstChildIndex(int idx) const
{
    return ((idx * 2) + 1);
}

#endif // QBSPTREE_P_H

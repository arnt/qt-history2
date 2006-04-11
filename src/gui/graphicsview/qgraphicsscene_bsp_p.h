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

class QGraphicsSceneBspTree
{
public:

    struct Node
    {
        enum Type { None = 0, VerticalPlane = 1, HorizontalPlane = 2, Both = 3 };
        inline Node() : pos(0), type(None) {}
        qreal pos;
        Type type;
    };
    typedef Node::Type NodeType;

    struct Data
    {
        Data(void *p) : ptr(p) {}
        Data(int n) : i(n) {}
        union {
            void *ptr;
            int i;
        };
    };
    typedef QGraphicsSceneBspTree::Data QGraphicsSceneBspTreeData;
    typedef void callback(QVector<int> &leaf, const QRectF &area, uint visited, QGraphicsSceneBspTreeData data);

    QGraphicsSceneBspTree();

    void create(int n, int d = -1);
    void destroy();

    inline void init(const QRectF &area, NodeType type) { init(area, depth, type, 0); }

    void climbTree(const QRectF &rect, callback *function, QGraphicsSceneBspTreeData data);

    inline int leafCount() const { return leaves.count(); }
    inline QVector<int> &leaf(int i) { return leaves[i]; }
    inline void insertLeaf(const QRectF &r, int i) { climbTree(r, &insert, i, 0); }
    inline void removeLeaf(const QRectF &r, int i) { climbTree(r, &remove, i, 0); }

protected:
    void init(const QRectF &area, int depth, NodeType type, int index);
    void climbTree(const QRectF &rect, callback *function, QGraphicsSceneBspTreeData data, int index);

    inline int parentIndex(int i) const { return (i & 1) ? ((i - 1) / 2) : ((i - 2) / 2); }
    inline int firstChildIndex(int i) const { return ((i * 2) + 1); }

    static void insert(QVector<int> &leaf, const QRectF &area, uint visited, QGraphicsSceneBspTreeData data);
    static void remove(QVector<int> &leaf, const QRectF &area, uint visited, QGraphicsSceneBspTreeData data);

private:
    uint depth;
    mutable uint visited;
    QVector<Node> nodes;
    mutable QVector< QVector<int> > leaves; // the leaves are just indices into the items
};

#endif // QBSPTREE_P_H

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

#ifndef QTREEVIEW_P_H
#define QTREEVIEW_P_H

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

#include <private/qabstractitemview_p.h>

struct QTreeViewItem
{
    QTreeViewItem() : expanded(false), total(0), level(0), height(0) {}
    QModelIndex index; // we remove items whenever the indexes are invalidated (make persistent ?)
    uint expanded : 1;
    uint total : 30; // total number of children visible (+ hidden children)
    uint level : 16; // indentation
    uint height : 16; // row height
};

class QTreeViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeView)
public:

    QTreeViewPrivate()
        : QAbstractItemViewPrivate(),
          header(0), indent(20), itemHeight(-1),
          uniformRowHeights(false), rootDecoration(true), itemsExpandable(true), reexpand(-1) { }

    ~QTreeViewPrivate() {}
    void initialize();

    void expand(int item);
    void collapse(int item);
    void layout(int item);

    int pageUp(int item) const;
    int pageDown(int item) const;

    inline int above(int item) const
        { return (--item < 0 ? 0 : item); }
    inline int below(int item) const
        { return (++item >= viewItems.count() ? viewItems.count() - 1 : item); }

    inline int height(int item) const {
        if (uniformRowHeights) return itemHeight;
        if (viewItems.at(item).height == 0 && viewItems.at(item).index.isValid())
            viewItems[item].height = q_func()->indexRowSizeHint(viewItems.at(item).index);
        return viewItems.at(item).height;
    }

    int indentation(int item) const;
    int coordinate(int item) const;
    int item(int coordinate) const;

    int viewIndex(const QModelIndex &index) const;
    QModelIndex modelIndex(int i) const;

    int itemAt(int value) const;
    int topItemDelta(int value, int iheight) const;
    int columnAt(int x) const;

    void relayout(const QModelIndex &parent);
    void reexpandChildren(const QModelIndex &parent);

    void updateVerticalScrollbar();
    void updateHorizontalScrollbar();

    int itemDecorationAt(const QPoint &pos) const;

    void select(int start, int stop, QItemSelectionModel::SelectionFlags command);

    QHeaderView *header;
    int indent;

    mutable QVector<QTreeViewItem> viewItems;
    int itemHeight; // this is just a number; contentsHeight() / numItems
    bool uniformRowHeights; // used when all rows have the same height
    bool rootDecoration;
    bool itemsExpandable;

    // used for drawing
    int left;
    int right;
    int current;

    // used when expanding and closing items
    QVector<QPersistentModelIndex> expandedIndexes;
    int reexpand;

    // used when hiding and showing items
    QVector<QPersistentModelIndex> hiddenIndexes;

    // used for hidden items
    int hiddenItemsCount;
};

#endif // QTREEVIEW_P_H

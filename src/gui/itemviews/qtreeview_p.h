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

#include <private/qabstractitemview_p.h>

struct QTreeViewItem
{
    QTreeViewItem() : open(false), hidden(false), total(0), level(0) {}
    QModelIndex index;
    uint open : 1;
    uint hidden : 1;
    uint total : 30; // total number of children visible
    uint level : 16; // indentation
};

class QTreeViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeView)
public:

    QTreeViewPrivate()
        : QAbstractItemViewPrivate(), header(0), indent(20), itemHeight(-1) { }

    ~QTreeViewPrivate() {}

    void open(int item, bool update);
    void close(int item, bool update);
    void layout(int item);

    int pageUp(int item) const;
    int pageDown(int item) const;
    int above(int item) const;
    int below(int item) const;
    int first() const;
    int last() const;

    int indentation(int item) const;
    int coordinate(int item) const;
    int item(int coordinate) const;

    int viewIndex(const QModelIndex &index) const;
    QModelIndex modelIndex(int i) const;

    int itemAt(int value) const;
    int coordinateAt(int value, int iheight) const;
    int columnAt(int x) const;

    void relayout(const QModelIndex &parent);

    QHeaderView *header;
    int indent;

    QVector<QTreeViewItem> items;
    int itemHeight; // this is just a number; contentsHeight() / numItems
    bool rootDecoration;

    // used for drawing
    int left;
    int right;
    int current;

    // used when opening and closing items
    QVector<QModelIndex> opened;
};

#endif

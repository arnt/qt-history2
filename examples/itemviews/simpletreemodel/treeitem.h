/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>

class TreeItem
{
public:
    TreeItem(QStringList data, TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();

private:
    int childRow(TreeItem *item) const;

    QList<TreeItem*> childItems;
    QStringList itemData;
    TreeItem *parentItem;
};

#endif

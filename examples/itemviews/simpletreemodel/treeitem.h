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

    void appendChildItem(TreeItem *child);
    TreeItem *childItem(int i);
    int childCount() const;
    int childRow(TreeItem *child) const;

    int columnCount() const;
    int row() const;
    QVariant data(int column) const;
    TreeItem *parent();

private:
    QList<TreeItem*> childItems;
    QStringList itemData;
    TreeItem *parentItem;
};

#endif

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

#ifndef QTREEWIDGET_P_H
#define QTREEWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//


#include <QtCore/qabstractitemmodel.h>
#include <QtCore/QPair.h>

class QTreeWidgetItem;
class QTreeWidgetItemIterator;

class QTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class QTreeWidget;
    friend class QTreeWidgetItem;
    friend class QTreeWidgetItemIterator;

public:
    QTreeModel(int columns = 0, QObject *parent = 0);
    ~QTreeModel();

    void clear();
    void setColumnCount(int columns);

    QTreeWidgetItem *item(const QModelIndex &index) const;

    QModelIndex index(QTreeWidgetItem *item, int column) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order);
    static bool itemLessThan(const QPair<QTreeWidgetItem*,int> &left,
                             const QPair<QTreeWidgetItem*,int> &right);
    static bool itemGreaterThan(const QPair<QTreeWidgetItem*,int> &left,
                                const QPair<QTreeWidgetItem*,int> &right);

    void insertInTopLevel(int row, QTreeWidgetItem *item);

    void insertListInTopLevel(int row, const QList<QTreeWidgetItem*> &items);

    // dnd
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    QMimeData *internalMimeData()  const;

    // used by the iterator
    static QTreeWidgetItem* nextSibling(const QTreeWidgetItem* item);
    static QTreeWidgetItem* previousSibling(const QTreeWidgetItem* item);
    static void ensureValidIterator(QTreeWidgetItemIterator *iterator, const QTreeWidgetItem *itemToBeRemoved);
    static QTreeWidgetItem* walk(const QTreeWidgetItem *current);

protected:
    void emitDataChanged(QTreeWidgetItem *item, int column);
    void beginInsertItems(QTreeWidgetItem *parent, int row, int count);
    void beginRemoveItems(QTreeWidgetItem *parent, int row, int count);
    void sortItems(QList<QTreeWidgetItem*> *items, int column, Qt::SortOrder order);

private:
    QList<QTreeWidgetItem*> tree;
    QTreeWidgetItem *header;
    Qt::SortOrder sorting;

    // A cache must be mutable if get-functions should have const modifiers
    mutable QModelIndexList cachedIndexes;
    QList<QTreeWidgetItemIterator*> iterators;
};

#endif /*QTREEWIDGET_P_H*/

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

#include "qsortingproxymodel.h"
#include <qalgorithms.h>
#include <qstack.h>
#include <qsize.h>
#include <qdebug.h>

typedef bool(*LessThan)(const QModelIndex &left, const QModelIndex &right);

/*!
  \class QSortingProxyModel
  \brief The QSortingProxyModel class provides support for sorting data
  that is passed between another model and a view.

*/

/*!
    Constructs a sorting proxy model with the given \a parent.
*/

QSortingProxyModel::QSortingProxyModel(QObject *parent)
    : QProxyModel(parent),  row_iid(false), sort_column(-1), sort_order(Qt::Ascending)
{
}

/*!
    Destroys the sorting proxy model.
*/
QSortingProxyModel::~QSortingProxyModel()
{
}

/*!
    \reimpl
*/
void QSortingProxyModel::setModel(QAbstractItemModel *model)
{
    if (this->model()) {        
        // disconnect QSortingProxyModel signals

        disconnect(this->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        
        disconnect(this->model(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsInserted()));
        
        disconnect(this->model(), SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(columnsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsInserted()));
        
        disconnect(this->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsRemoved()));
        
        disconnect(this->model(), SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsRemoved()));
        
        disconnect(this->model(), SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
        disconnect(this->model(), SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));
    }

    QProxyModel::setModel(model);

    if (this->model()) {
        // disconnect QProxyModel signals        

        disconnect(this->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
        disconnect(this->model(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                   this, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SIGNAL(rowsInserted(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SIGNAL(rowsRemoved(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                   this, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(columnsInserted(QModelIndex,int,int)),
                   this, SIGNAL(columnsInserted(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   this, SIGNAL(columnsRemoved(QModelIndex,int,int)));
        disconnect(this->model(), SIGNAL(modelReset()), this, SIGNAL(modelReset()));
        disconnect(this->model(), SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
        
        // connect QSortingProxyModel signals
        
        connect(this->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));    
        
        connect(this->model(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(this->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted()));
        
        connect(this->model(), SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
        connect(this->model(), SIGNAL(columnsInserted(QModelIndex,int,int)),
                this, SLOT(sourceColumnsInserted()));
        
        connect(this->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(this->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved()));
        
        connect(this->model(), SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
        connect(this->model(), SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceColumnsRemoved()));

        connect(this->model(), SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
        connect(this->model(), SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));
    }
}

/*!
    \reimpl
*/
QModelIndex QSortingProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex source_index = sourceIndex(row, column, parent);
    return createIndex(row, column, p_id(source_index));
}

/*!
  \reimpl
*/
QModelIndex QSortingProxyModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        QModelIndex source_child = id_to_source_index_map.value(child.internalPointer());
        QModelIndex source_parent = model()->parent(source_child);
        if (source_parent.isValid())
            return proxyIndex(source_parent);
    }
    return QModelIndex();
}

/*!
  \reimpl
*/
int QSortingProxyModel::rowCount(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    return model()->rowCount(source_parent);
}

/*!
  \reimpl
*/
int QSortingProxyModel::columnCount(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    return model()->columnCount(source_parent);
}

/*!
  \reimpl
*/
bool QSortingProxyModel::hasChildren(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    return model()->hasChildren(source_parent);
}

/*!
  \reimpl
*/
QVariant QSortingProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        QModelIndex source_index = id_to_source_index_map.value(index.internalPointer());
        return model()->data(source_index, role);
    }
    return QVariant();
}

/*!
  \reimpl
*/
bool QSortingProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        QModelIndex source_index = id_to_source_index_map.value(index.internalPointer());
        return model()->setData(source_index, value, role);
    }
    return false;
}

/*!
  \reimpl
*/
QMimeData *QSortingProxyModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList source_indexes;
    for (int i = 0; i < indexes.count(); ++i)
        source_indexes << id_to_source_index_map.value(indexes.at(i).internalPointer());
    return model()->mimeData(source_indexes);
}

/*!
  \reimpl
*/
bool QSortingProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent)
{
    QModelIndex source_index = sourceIndex(row, 0, parent);
    return model()->dropMimeData(data, action, source_index.row(), column, source_index.parent());
}

/*!
  \reimpl
*/
bool QSortingProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    return model()->insertRows(row, count, source_parent);
}

/*!
  \reimpl
*/
bool QSortingProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    return model()->insertColumns(column, count, source_parent); // we don't change the columns
}

/*!
  \reimpl
*/
bool QSortingProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    QModelIndex source_index = sourceIndex(row, 0, parent);
    return model()->removeRows(source_index.row(), count, source_index.parent());
}

/*!
  \reimpl
*/
bool QSortingProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    return model()->removeColumns(column, count, source_parent); // we don't change the columns
}

/*!
  \reimpl
*/
void QSortingProxyModel::fetchMore(const QModelIndex &parent)
{
    QModelIndex source_parent = id_to_source_index_map.value(parent.internalPointer());
    model()->fetchMore(source_parent);
}

/*!
  \reimpl
*/
bool QSortingProxyModel::canFetchMore(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    return model()->canFetchMore(source_parent);
}

/*!
  \reimpl
*/
Qt::ItemFlags QSortingProxyModel::flags(const QModelIndex &index) const
{
    QModelIndex source_index;
    if (index.isValid())
        source_index = id_to_source_index_map.value(index.internalPointer());
    return model()->flags(source_index);
}

/*!
  \reimpl
*/
void QSortingProxyModel::sort(int column, Qt::SortOrder order)
{
    sort_column = column;
    sort_order = order;
    QStack<QModelIndex> source_parent_stack;
    source_parent_stack.push(QModelIndex());
    QList<QModelIndex> source_children;
    LessThan compare = (order == Qt::AscendingOrder ? &lessThan : &greaterThan);
    while (!source_parent_stack.isEmpty()) {
        QModelIndex source_parent = source_parent_stack.pop();
        for (int row = 0; row < model()->rowCount(source_parent); ++row) {
            QModelIndex source_index = model()->index(row, column, source_parent);
            source_parent_stack.push(source_index);
            source_children.append(source_index);
        }
        qSort(source_children.begin(), source_children.end(), compare);
        int source_column_count = model()->columnCount(source_parent);
        for (int proxy_row = 0; proxy_row < source_children.count(); ++proxy_row) {
            int source_row = source_children.at(proxy_row).row();
            QModelIndex source_index = model()->index(source_row, 0, source_parent);
            void* source_iid = p_id(source_index);
            int old_proxy_row = id_to_proxy_row_map.value(source_iid, -1);
            for (int source_column = 0; source_column < source_column_count; ++source_column) {
                source_index = model()->index(source_row, source_column, source_parent);
                // some models have the same id for all items in a row
                row_iid = (source_iid == p_id(source_index));
                source_iid = p_id(source_index);
                // we need the old row to change the persistent index (if it exists)
                id_to_proxy_row_map.insert(p_id(source_index), proxy_row);
                QModelIndex from = createIndex(old_proxy_row, source_column, source_iid);
                QModelIndex to = createIndex(proxy_row, source_column, source_iid);
                changePersistentIndex(from, to);
            }
        }
        source_children.clear();
    }
    emit layoutChanged();
}

/*!
  \reimpl
*/
QModelIndex QSortingProxyModel::buddy(const QModelIndex &index) const
{
    if (index.isValid()) {
        QModelIndex source_index = id_to_source_index_map.value(index.internalPointer());
        QModelIndex source_buddy = model()->buddy(source_index);
        return proxyIndex(source_buddy);
    }
    return QModelIndex();
}

/*!
  \reimpl
*/
QModelIndexList QSortingProxyModel::match(const QModelIndex &start, int role,
                                          const QVariant &value, int hits,
                                          Qt::MatchFlags flags) const
{
    QModelIndex source_start = id_to_source_index_map.value(start.internalPointer());
    QModelIndexList result = model()->match(source_start, role, value, hits, flags);
    for (int i = 0; i < result.count(); ++i)
        result[i] = proxyIndex(result.at(i));
    return result;
}

/*!
  \reimpl
*/
QSize QSortingProxyModel::span(const QModelIndex &index) const
{
    QModelIndex source_index = id_to_source_index_map.value(index.internalPointer());
    return model()->span(source_index);
}

/*!
  Clears the sorting proxy model, removing all map.
*/
void QSortingProxyModel::clear()
{
    id_to_source_index_map.clear();
    id_to_proxy_row_map.clear();
    row_iid = false;
    sort_column = -1;
    sort_order = Qt::Ascending;
    reset();
}

// protected

/*!
  Returns true if the data in the item in \a left is less than
  the data in \a right; other wise returns false.
*/
bool QSortingProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right)
{
    QVariant leftValue = left.model()->data(left, Qt::DisplayRole);
    QVariant rightValue = right.model()->data(right, Qt::DisplayRole);
    return leftValue.toString() < rightValue.toString();
}

/*!
  Returns true if the data in the item in \a left is greater than
  the data in \a right; other wise returns false.
*/
bool QSortingProxyModel::greaterThan(const QModelIndex &left, const QModelIndex &right)
{
    return lessThan(right, left);
}

/*!
  Returns the model index in the QSortingProxyModel given
  the \a source_index from the source model.
*/
QModelIndex QSortingProxyModel::proxyIndex(const QModelIndex &source_index) const
{
    if (!source_index.isValid())
        return QModelIndex();
    int proxy_row = id_to_proxy_row_map.value(p_id(source_index), source_index.row());
    int proxy_column = source_index.column();
    void* proxy_id = p_id(source_index);
    QModelIndex proxy_index = createIndex(proxy_row, proxy_column, proxy_id);
    if (proxy_row == source_index.row()) // was not found in the map
        id_to_proxy_row_map.insert(proxy_id, proxy_row);
    return proxy_index;
}

/*!
  Returns the model index in the source model given
  the \a row, \a column and \a parent model index in
  the source model.
*/
QModelIndex QSortingProxyModel::sourceIndex(int row, int column, const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = id_to_source_index_map.value(parent.internalPointer());
    QModelIndex source_index;
    QList<void*> proxy_ids = id_to_proxy_row_map.keys(row);
    for (int i = 0; i < proxy_ids.count(); ++i) {
        void *proxy_id = proxy_ids.at(i); // ### uses ids directly
        QModelIndex candidate_index = id_to_source_index_map.value(proxy_id);
        if (candidate_index.isValid()
            && candidate_index.parent() == source_parent
            && (candidate_index.column() == column || row_iid)) {
            source_index = candidate_index;
            break;
        }
    }
    if (!source_index.isValid()) {
        source_index = model()->index(row, column, source_parent);
        id_to_source_index_map.insert(p_id(source_index), source_index); // ###
    }
    return source_index;
}

// slots called by the source model

/*!
  \internal
*/
void QSortingProxyModel::sourceDataChanged(const QModelIndex &source_top_left,
                                           const QModelIndex &source_bottom_right)
{
    emit dataChanged(proxyIndex(source_top_left), proxyIndex(source_bottom_right));
}

/*!
  \internal
*/
void QSortingProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &source_parent,
                                                     int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    QModelIndex source_index = model()->index(start, 0, source_parent);
    QModelIndex proxy_index = proxyIndex(source_index);
    int proxy_start = proxy_index.row() > 0 ? proxy_index.row() : start;
    int proxy_end = proxy_start + (end - start);
    beginInsertRows(proxy_parent, proxy_start, proxy_end); // emits signal
}

/*!
  \internal
*/
void QSortingProxyModel::sourceRowsInserted()
{
    clearAndSort();
    endInsertRows(); // emits signal
}

/*!
  \internal
*/
void QSortingProxyModel::sourceColumnsAboutToBeInserted(const QModelIndex &source_parent,
                                                        int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    beginInsertColumns(proxy_parent, start, end); // emits signal
}   

/*!
  \internal
*/
void QSortingProxyModel::sourceColumnsInserted()
{
    endInsertColumns(); // emits signal
}

/*!
  \internal
*/
void QSortingProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &source_parent,
                                                    int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    QModelIndex source_index = model()->index(start, 0, source_parent);
    QModelIndex proxy_index = proxyIndex(source_index);
    int proxy_start = proxy_index.row();
    int proxy_end = proxy_start + (end - start);
    beginRemoveRows(proxy_parent, proxy_start, proxy_end); // emits signal
    id_to_proxy_row_map.remove(proxy_index.internalPointer());
    id_to_source_index_map.remove(proxy_index.internalPointer());
}

/*!
  \internal
*/
void QSortingProxyModel::sourceRowsRemoved()
{
    clearAndSort();
    endRemoveRows(); // emits signal
}

/*!
  \internal
*/
void QSortingProxyModel::sourceColumnsAboutToBeRemoved(const QModelIndex &source_parent,
                                                       int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    beginRemoveColumns(proxy_parent, start, end); // emits signal
}

/*!
  \internal
*/
void QSortingProxyModel::sourceColumnsRemoved()
{
    endRemoveColumns(); // emits signal
}

/*!
  \internal
*/
void QSortingProxyModel::sourceModelReset()
{
    clear();
}

/*!
  \internal
*/
void QSortingProxyModel::sourceLayoutChanged()
{
    // we have no other way of handling this
    clear();
}

/*!
  \internal
*/
void QSortingProxyModel::clearAndSort()
{
    id_to_source_index_map.clear();
    id_to_proxy_row_map.clear();
    if (sort_column != -1)
        sort(sort_column, sort_order);
}

/*!
  \internal
*/
void *QSortingProxyModel::p_id(const QModelIndex &source_index) const
{
    if (source_index.internalPointer())
        return source_index.internalPointer();
    return reinterpret_cast<void*>(source_index.row());
}

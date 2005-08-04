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
#include <private/qsortingproxymodel_p.h>

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
    : QAbstractItemModel(*new QSortingProxyModelPrivate, parent),
      sort_column(-1), sort_order(Qt::Ascending)
{
    Q_D(QSortingProxyModel);
    setModel(&d->empty);
}

/*!
    \internal
*/
QSortingProxyModel::QSortingProxyModel(QSortingProxyModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(QSortingProxyModel);
    setModel(&d->empty);
}

/*!
    Destroys the sorting proxy model.
*/
QSortingProxyModel::~QSortingProxyModel()
{

}

/*!
    Sets the given \a model to be processed by the proxy model.
*/
void QSortingProxyModel::setModel(QAbstractItemModel *model)
{
    Q_D(QSortingProxyModel);
    if (d->model && d->model != &d->empty) {
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        disconnect(d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                   this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

        disconnect(d->model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsInserted()));

        disconnect(d->model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsInserted()));

        disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsRemoved()));

        disconnect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsRemoved()));

        disconnect(d->model, SIGNAL(modelReset()), this, SLOT(clearAndSort()));
        disconnect(d->model, SIGNAL(layoutChanged()), this, SLOT(clearAndSort()));
    }

    if (model) {
        d->model = model;
        
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        connect(d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

        connect(d->model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted()));

        connect(d->model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                this, SLOT(sourceColumnsInserted()));

        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved()));

        connect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceColumnsRemoved()));

        connect(d->model, SIGNAL(modelReset()), this, SLOT(clearAndSort()));
        connect(d->model, SIGNAL(layoutChanged()), this, SLOT(clearAndSort()));

    } else {
        d->model = &d->empty;
    }
}

/*!
    Returns the model that contains the data that is available through the
    sorting proxy model.
*/
QAbstractItemModel *QSortingProxyModel::model() const
{
    Q_D(const QSortingProxyModel);
    return d->model;
}

/*!
    \reimpl
*/
QModelIndex QSortingProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    void *parent_node = 0;
    if (parent.isValid())
        parent_node = proxy_to_source.find(parent); // ### slow
    QModelIndex proxy_index = createIndex(row, column, parent_node);

    if (!proxy_to_source.contains(proxy_index)) {
        QModelIndex source_parent = proxy_to_source.value(parent);
        QModelIndex source_index = model()->index(row, column, source_parent);
        proxy_to_source.insert(proxy_index, source_index);
    }

    return proxy_index;
}

/*!
  \reimpl
*/
QModelIndex QSortingProxyModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        QModelIndex source_child = proxy_to_source.value(child);
        QModelIndex source_parent = model()->parent(source_child);
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
        source_parent = proxy_to_source.value(parent);
    return model()->rowCount(source_parent);
}

/*!
  \reimpl
*/
int QSortingProxyModel::columnCount(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    return model()->columnCount(source_parent);
}

/*!
  \reimpl
*/
bool QSortingProxyModel::hasChildren(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    return model()->hasChildren(source_parent);
}

/*!
  \reimpl
*/
QVariant QSortingProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        QModelIndex source_index = proxy_to_source.value(index);
        //qDebug() << "*key*" << index;
        //qDebug() << "*val*" << source_index;
        Q_ASSERT(source_index.isValid());
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
        QModelIndex source_index = proxy_to_source.value(index);
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
        source_indexes << proxy_to_source.value(indexes.at(i));
    return model()->mimeData(source_indexes);
}

/*!
  \reimpl
*/
bool QSortingProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent)
{
    QModelIndex source_index = sourceIndex(row, column, parent);
    return model()->dropMimeData(data, action, source_index.row(), source_index.column(),
                                 source_index.parent());
}

/*!
  \reimpl
*/
bool QSortingProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    QModelIndex source_index = sourceIndex(row, 0, parent);
    return model()->insertRows(source_index.row(), count, source_index.parent());
}

/*!
  \reimpl
*/
bool QSortingProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    QModelIndex source_index = sourceIndex(0, column, parent);
    return model()->insertColumns(source_index.column(), count, source_index.parent());
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
    QModelIndex source_index = sourceIndex(0, column, parent);
    return model()->removeColumns(source_index.column(), count, source_index.parent());
}

/*!
  \reimpl
*/
void QSortingProxyModel::fetchMore(const QModelIndex &parent)
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    model()->fetchMore(source_parent);
}

/*!
  \reimpl
*/
bool QSortingProxyModel::canFetchMore(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    return model()->canFetchMore(source_parent);
}

/*!
  \reimpl
*/
Qt::ItemFlags QSortingProxyModel::flags(const QModelIndex &index) const
{
    QModelIndex source_index;
    if (index.isValid())
        source_index = proxy_to_source.value(index);
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
            if (model()->hasChildren(source_index))
                source_parent_stack.push(source_index);
            source_children.append(source_index);
        }
        
        qSort(source_children.begin(), source_children.end(), compare);

        QModelIndex proxy_parent = proxy_to_source.key(source_parent); // ### slow
        void *parent_node = 0;
        if (proxy_parent.isValid())
            parent_node = proxy_to_source.find(proxy_parent);

        int source_column_count = model()->columnCount(source_parent);
        for (int proxy_row = 0; proxy_row < source_children.count(); ++proxy_row) {
            int source_row = source_children.at(proxy_row).row();
            for (int source_column = 0; source_column < source_column_count; ++source_column) {
                QModelIndex source_index = model()->index(source_row, source_column, source_parent);
                Q_ASSERT(source_index.isValid());
                QModelIndex old_proxy_index = proxy_to_source.key(source_index);
                QModelIndex new_proxy_index = createIndex(proxy_row, source_column, parent_node);
                if (old_proxy_index.isValid()) {
                    changePersistentIndex(old_proxy_index, new_proxy_index);
                    proxy_to_source.remove(old_proxy_index);
                }
                proxy_to_source.insert(new_proxy_index, source_index);
                qDebug() << "#" << model()->data(source_index).toString() << proxy_row;
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
        QModelIndex source_index = proxy_to_source.value(index);
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
    QModelIndex source_start = proxy_to_source.value(start);
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
    QModelIndex source_index = proxy_to_source.value(index);
    return model()->span(source_index);
}

/*!
  Clears the sorting proxy model, removing all map.
*/
void QSortingProxyModel::clear()
{
    proxy_to_source.clear();
    sort_column = -1;
    sort_order = Qt::AscendingOrder;
    reset();
}

// protected

/*!
  Returns true if the data in the item in \a source_left is less than
  the data in \a source_right; other wise returns false.
*/
bool QSortingProxyModel::lessThan(const QModelIndex &source_left,
                                  const QModelIndex &source_right)
{
    QVariant leftValue = source_left.model()->data(source_left, Qt::DisplayRole);
    QVariant rightValue = source_right.model()->data(source_right, Qt::DisplayRole);
    return leftValue.toString() < rightValue.toString();
}

/*!
  Returns true if the data in the item in \a source_left is greater than
  the data in \a source_right; other wise returns false.
*/
bool QSortingProxyModel::greaterThan(const QModelIndex &source_left,
                                     const QModelIndex &source_right)
{
    return lessThan(source_right, source_left);
}

/*!
  Returns the model index in the QSortingProxyModel given
  the \a source_index from the source model.
*/
QModelIndex QSortingProxyModel::proxyIndex(const QModelIndex &source_index) const
{
    if (!source_index.isValid())
        return QModelIndex();

    QModelIndex proxy_index = proxy_to_source.key(source_index); // ### slow
    if (proxy_index.isValid())
        return proxy_index;

    // not found in the map; create the index and insert it into the map
    QModelIndex source_parent = source_index.parent();
    QModelIndex proxy_parent;
    if (source_parent.isValid())
        proxy_parent = proxy_to_source.key(source_parent); // ### slow
    
    void *parent_node = 0;
    if (proxy_parent.isValid())
        parent_node = proxy_to_source.find(proxy_parent); // ### slow
    proxy_index = createIndex(source_index.row(), source_index.column(), parent_node);
    
    proxy_to_source.insert(proxy_index, source_index);
    return proxy_index;
}

/*!
  Returns the model index in the source model given
  the \a row, \a column and \a parent model index in
  the source model.
*/
QModelIndex QSortingProxyModel::sourceIndex(int row, int column, const QModelIndex &parent) const
{
    void *parent_node = 0;
    if (parent.isValid())
        parent_node = proxy_to_source.find(parent); // ### slow

    QModelIndex proxy_index = createIndex(row, column, parent_node);
    QModelIndex source_index = proxy_to_source.value(proxy_index);

    if (!source_index.isValid()) {
        QModelIndex source_parent = proxy_to_source.value(parent);
        source_index = model()->index(row, column, source_parent);
        proxy_to_source.insert(proxy_index, source_index);
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
void QSortingProxyModel::sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end)
{
    // FIXME: map rows and columns for headers too
    emit headerDataChanged(orientation, start, end);
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
    proxy_to_source.remove(proxy_index);
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
void QSortingProxyModel::clearAndSort()
{
    proxy_to_source.clear();
    if (sort_column != -1)
        sort(sort_column, sort_order);
}

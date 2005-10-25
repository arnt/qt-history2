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

#include "qfilteringproxymodel.h"
#include <private/qfilteringproxymodel_p.h>
#include <qbitarray.h>
#include <qdebug.h>

/*!
    \class QFilteringProxyModel
    \brief The QFilteringProxyModel class provides support for filtering data
    that is passed between another model and a view.
    \since 4.1
    \ingroup model-view

    \sa QProxyModel, QAbstractItemModel, {Model/View Programming}
*/

/*!
    Constructs a filterting proxy model with the given \a parent.
*/
QFilteringProxyModel::QFilteringProxyModel(QObject *parent)
    : QMappingProxyModel(*new QFilteringProxyModelPrivate, parent)
{

}

/*!
    \internal
*/
QFilteringProxyModel::QFilteringProxyModel(QFilteringProxyModelPrivate &dd, QObject *parent)
    : QMappingProxyModel(dd, parent)
{

}

/*!
    Destroys the filtering proxy model.
*/
QFilteringProxyModel::~QFilteringProxyModel()
{

}

/*!
  Clears the filtering proxy model, removing the mapping.
*/
void QFilteringProxyModel::clear()
{
    Q_D(QFilteringProxyModel);
    d->filtered_count.clear();
    QMappingProxyModel::clear();
}

/*!
    \reimp
*/
QModelIndex QFilteringProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const QFilteringProxyModel);
    if (d->filtered_count.isEmpty() || !d->filtered_count.contains(parent))
        mapChildren(parent); // filter and map the children of parent, including proxy_index
    if (d->proxy_to_source.isEmpty()) // nothing was mapped
        return QModelIndex();
    void *parent_node = 0;
    if (parent.isValid())
        parent_node = d->proxy_to_source.find(parent); // ### slow
    //Q_ASSERT(row >= 0 && row < rowCount(parent));
    //Q_ASSERT(column >= 0 && column < columnCount(parent));
    return createIndex(row, column, parent_node);
}

/*!
    \reimp
*/
int QFilteringProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QFilteringProxyModel);
    if (d->filtered_count.isEmpty() || !d->filtered_count.contains(parent))
        mapChildren(parent); // filter and map the children of parent, including proxy_index
    if (d->proxy_to_source.isEmpty()) // nothing was mapped
        return 0;
    return QMappingProxyModel::rowCount(parent) - d->filtered_count.value(parent).first;
}

/*!
    \reimp
*/
int QFilteringProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const QFilteringProxyModel);
    if (d->filtered_count.isEmpty() || !d->filtered_count.contains(parent))
        mapChildren(parent); // filter and map the children of parent, including proxy_index
    if (d->proxy_to_source.isEmpty()) // nothing was mapped
        return 0;
    return QMappingProxyModel::columnCount(parent) - d->filtered_count.value(parent).second;
}

/*!
  \internal
*/
void QFilteringProxyModel::mapChildren(const QModelIndex &parent) const
{
    Q_D(const QFilteringProxyModel);

    QModelIndex source_parent;
    void *parent_node = 0;
    if (parent.isValid()) {
        parent_node = d->proxy_to_source.find(parent); // ### slow
        source_parent = d->proxy_to_source.value(parent);
    }

    int proxy_row = 0;
    int filtered_rows_count = 0;
    int source_row_count = d->model->rowCount(source_parent);
    int source_column_count = d->model->columnCount(source_parent);
    int filtered_columns_count = 0;
    QBitArray filtered_columns(source_column_count);
    for (int source_row = 0; source_row < source_row_count; ++source_row) {
        if (filterRow(source_row, source_parent)) {
            ++filtered_rows_count;
        } else {
            int proxy_column = 0;
            for (int source_column = 0; source_column < source_column_count; ++source_column) {
                if (proxy_row == 0 && filterColumn(source_column, source_parent)) {
                    filtered_columns.setBit(source_column);
                    ++filtered_columns_count;
                } else if (!filtered_columns.testBit(source_column)) {
                    QModelIndex source_index = d->model->index(source_row, source_column,
                                                               source_parent);
                    QModelIndex proxy_index = createIndex(proxy_row, proxy_column, parent_node);
                    Q_ASSERT(proxy_index.isValid());
                    Q_ASSERT(source_index.isValid());
                    //Q_ASSERT(!proxy_to_source.contains(proxy_index));
                    d->proxy_to_source.insert(proxy_index, source_index);
                    ++proxy_column;
                }
            }
            ++proxy_row;
        }
    }
    d->filtered_count.insert(parent, QPair<int,int>(filtered_rows_count, filtered_columns_count));
}

/*!
  Returns true if the value in the item in the row indicated by
  the given \a source_row and \a source_parent should be removed from the model.
  The default implementation returns false.
*/
bool QFilteringProxyModel::filterRow(int /*source_row*/, const QModelIndex &/*source_parent*/) const
{
    return false;
}

/*!
  Returns true if the value in the item in the column indicated by
  the given \a source_column and \a source_parent should be removed from the model.
  The default implementation returns false.
*/
bool QFilteringProxyModel::filterColumn(int /*source_column*/,  const QModelIndex &/*source_parent*/) const
{
    return false;
}

/*!
  \internal
*/
void QFilteringProxyModel::sourceLayoutChanged()
{
    Q_D(QFilteringProxyModel);
    d->filtered_count.clear();
    d->proxy_to_source.clear();
}

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
#include <qdebug.h>

/*!
    \class QFilteringProxyModel
    \brief The QFilteringProxyModel class provides support for filtering data
    that is passed between another model and a view.

    \since 4.1
*/

/*!
  \enum QFilteringProxyModel::FilterMode

  \valie FilterRows
  \value FilterColumns
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
  \property QFilteringProxyModel::filterMode
  \brief wether to filter rows or columns

*/
QFilteringProxyModel::FilterMode QFilteringProxyModel::filterMode() const
{
    Q_D(const QFilteringProxyModel);
    return d->mode;
}

void QFilteringProxyModel::setFilterMode(FilterMode mode)
{
    Q_D(QFilteringProxyModel);
    d->mode = mode;
    reset();
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
    if (d->mode == FilterColumns)
        return QMappingProxyModel::rowCount(parent);
    if (d->filtered_count.isEmpty() || !d->filtered_count.contains(parent))
        mapChildren(parent); // filter and map the children of parent, including proxy_index
    if (d->proxy_to_source.isEmpty()) // nothing was mapped
        return 0;
    return QMappingProxyModel::rowCount(parent) - d->filtered_count.value(parent);
}

/*!
    \reimp
*/
int QFilteringProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const QFilteringProxyModel);
    if (d->mode == FilterRows)
        return QMappingProxyModel::columnCount(parent);
    if (d->filtered_count.isEmpty() || !d->filtered_count.contains(parent))
        mapChildren(parent); // filter and map the children of parent, including proxy_index
    if (d->proxy_to_source.isEmpty()) // nothing was mapped
        return 0;
    return QMappingProxyModel::columnCount(parent) - d->filtered_count.value(parent);
}

/*!
  \internal
*/
void QFilteringProxyModel::mapChildren(const QModelIndex &parent) const
{
    Q_D(const QFilteringProxyModel);
    if (d->mode == FilterRows)
        d->filterRows(parent);
    else
        d->filterColumns(parent);
}

/*!
  Returns true if the value in the item in the row indicated by
  the given \a source_row and \a source_parent should be removed from the model.
  The default implementation returns false.
*/
bool QFilteringProxyModel::filterRow(int source_row, const QModelIndex &source_parent) const
{    
    return false;
}

/*!
  Returns true if the value in the item in the column indicated by
  the given \a source_column and \a source_parent should be removed from the model.
  The default implementation returns false.
*/
bool QFilteringProxyModel::filterColumn(int source_column,  const QModelIndex &source_parent) const
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

void QFilteringProxyModelPrivate::filterRows(const QModelIndex &parent) const
{
    Q_Q(const QFilteringProxyModel);
    
    QModelIndex source_parent;
    void *parent_node = 0;
    if (parent.isValid()) {
        parent_node = proxy_to_source.find(parent); // ### slow
        source_parent = proxy_to_source.value(parent);
    }

    int proxy_row = 0;
    int filtered_rows_count = 0;
    int source_row_count = model->rowCount(source_parent);
    int source_column_count = model->columnCount(source_parent);
    for (int source_row = 0; source_row < source_row_count; ++source_row) {
        if (q->filterRow(source_row, source_parent)) {
            ++filtered_rows_count;
        } else {
            for (int source_column = 0; source_column < source_column_count; ++source_column) {
                QModelIndex source_index = model->index(source_row, source_column, source_parent);
                QModelIndex proxy_index = q->createIndex(proxy_row, source_column, parent_node);
                Q_ASSERT(proxy_index.isValid());
                Q_ASSERT(source_index.isValid());
                //Q_ASSERT(!proxy_to_source.contains(proxy_index)); 
                proxy_to_source.insert(proxy_index, source_index);
            }
            ++proxy_row;
        }
    }
    filtered_count.insert(parent, filtered_rows_count);
}

void QFilteringProxyModelPrivate::filterColumns(const QModelIndex &parent) const
{
    Q_Q(const QFilteringProxyModel);
    
    QModelIndex source_parent;
    void *parent_node = 0;
    if (parent.isValid()) {
        parent_node = proxy_to_source.find(parent); // ### slow
        source_parent = proxy_to_source.value(parent);
    }

    int proxy_column = 0;
    int filtered_columns_count = 0;
    int source_row_count = model->rowCount(source_parent);
    int source_column_count = model->columnCount(source_parent);
    for (int source_column = 0; source_column < source_column_count; ++source_column) {
        if (q->filterColumn(source_column, source_parent)) {
            ++filtered_columns_count;
        } else {
            for (int source_row = 0; source_row < source_row_count; ++source_row) {
                QModelIndex source_index = model->index(source_row, source_column, source_parent);
                QModelIndex proxy_index = q->createIndex(source_row, proxy_column, parent_node);
                Q_ASSERT(proxy_index.isValid());
                Q_ASSERT(source_index.isValid());
                //Q_ASSERT(!proxy_to_source.contains(proxy_index)); FIXME
                proxy_to_source.insert(proxy_index, source_index);
            }
            ++proxy_column;
        }
    }
    filtered_count.insert(parent, filtered_columns_count);
}

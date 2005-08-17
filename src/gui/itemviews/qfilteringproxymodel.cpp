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
#include <qdebug.h>

/*!
    Constructs a filterting proxy model with the given \a parent.
*/
QFilteringProxyModel::QFilteringProxyModel(QObject *parent)
    : QMappingProxyModel(parent)
{

}

/*!
    Destroys the mapping proxy model.
*/
QFilteringProxyModel::~QFilteringProxyModel()
{

}

/*!
  Clears the sorting proxy model, removing all map.
*/
void QFilteringProxyModel::clear()
{
    filtered_row_count.clear();
    QMappingProxyModel::clear();    
}


/*!
    \reimp
*/
QModelIndex QFilteringProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!filtered_row_count.contains(parent))
        mapChildren(parent); // filter and map the children of parent, including proxy_index
    void *parent_node = 0;
    if (parent.isValid())
        parent_node = proxy_to_source.find(parent); // ### slow
    //Q_ASSERT(row >= 0 && row < rowCount(parent));
    return createIndex(row, column, parent_node);
}

/*!
    \reimp
*/
int QFilteringProxyModel::rowCount(const QModelIndex &parent) const
{
    if (!filtered_row_count.contains(parent))
        mapChildren(parent); // filter and map the children of parent, including proxy_index
    return QMappingProxyModel::rowCount(parent) - filtered_row_count.value(parent);
}

/*!
  \internal
*/
void QFilteringProxyModel::mapChildren(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    void *parent_node = 0;
    if (parent.isValid()) {
        parent_node = proxy_to_source.find(parent); // ### slow
        source_parent = proxy_to_source.value(parent);
    }

    int proxy_row = 0;
    int filtered_count = 0;
    int source_row_count = model()->rowCount(source_parent);
    int source_column_count = model()->columnCount(source_parent);
    for (int source_row = 0; source_row < source_row_count; ++source_row) {
        if (filterRow(source_row, source_parent)) {
            ++filtered_count;
        } else {
            for (int source_column = 0; source_column < source_column_count; ++source_column) {
                QModelIndex source_index = model()->index(source_row, source_column,
                                                          source_parent);
                QModelIndex proxy_index = createIndex(proxy_row, source_column, parent_node);
                Q_ASSERT(proxy_index.isValid());
                Q_ASSERT(source_index.isValid());
                //Q_ASSERT(!proxy_to_source.contains(proxy_index)); FIXME
                proxy_to_source.insert(proxy_index, source_index);
            }
            ++proxy_row;
        }
    }
    filtered_row_count.insert(parent, filtered_count);
}

/*!
  \internal
*/
void QFilteringProxyModel::sourceLayoutChanged()
{
    filtered_row_count.clear();
    proxy_to_source.clear();
}

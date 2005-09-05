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
#include <qdebug.h>
#include <private/qsortingproxymodel_p.h>

/*!
  \class QSortingProxyModel
  \brief The QSortingProxyModel class provides support for sorting data
  that is passed between another model and a view.

  \ingroup model-view

  \sa QProxyModel, QAbstractItemModel, {Model/View Programming}
*/

/*!
    Constructs a sorting proxy model with the given \a parent.
*/

QSortingProxyModel::QSortingProxyModel(QObject *parent)
    : QMappingProxyModel(*new QSortingProxyModelPrivate, parent)
{

}

/*!
    \internal
*/
QSortingProxyModel::QSortingProxyModel(QSortingProxyModelPrivate &dd, QObject *parent)
    : QMappingProxyModel(dd, parent)
{

}

/*!
    Destroys the sorting proxy model.
*/
QSortingProxyModel::~QSortingProxyModel()
{

}

/*!
  \reimp
*/
void QSortingProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_D(QSortingProxyModel);
    d->sort_column = column;
    d->sort_order = order;

    QStack<QModelIndex> source_parent_stack;
    source_parent_stack.push(QModelIndex());

    QList<QModelIndex> source_children;
    Compare *compare = (order == Qt::AscendingOrder ? d->less : d->greater);

    while (!source_parent_stack.isEmpty()) {

        QModelIndex source_parent = source_parent_stack.pop();
        for (int row = 0; row < model()->rowCount(source_parent); ++row) {
            QModelIndex source_index = model()->index(row, column, source_parent);
            if (model()->hasChildren(source_index))
                source_parent_stack.push(source_index);
            source_children.append(source_index);
        }

        qSort(source_children.begin(), source_children.end(), *compare);

        QModelIndex proxy_parent = proxy_to_source.key(source_parent); // ### slow
        void *parent_node = 0;
        if (proxy_parent.isValid())
            parent_node = proxy_to_source.find(proxy_parent); // get the QMap node, used as uid in the proxy index

        // for each proxy_row, go through the source_column (same as proxy columns) and update the mapping
        int source_column_count = model()->columnCount(source_parent);
        int proxy_row_count = source_children.count();
        for (int proxy_row = 0; proxy_row < proxy_row_count; ++proxy_row) {
            int source_row = source_children.at(proxy_row).row();
            for (int source_column = 0; source_column < source_column_count; ++source_column) {
                QModelIndex source_index = model()->index(source_row, source_column,
                                                          source_parent);
                Q_ASSERT(source_index.isValid());
                QModelIndex old_proxy_index = proxy_to_source.key(source_index);
                QModelIndex new_proxy_index = createIndex(proxy_row, source_column, parent_node);
                if (old_proxy_index.isValid()) {
                    changePersistentIndex(old_proxy_index, new_proxy_index);
                    proxy_to_source.remove(old_proxy_index);
                }
                Q_ASSERT(new_proxy_index.isValid());
                Q_ASSERT(source_index.isValid());
                proxy_to_source.insert(new_proxy_index, source_index);
            }
        }
        
        source_children.clear();
    }

    emit layoutChanged();
}

/*!
  Clears the sorting proxy model, removing all map.
*/
void QSortingProxyModel::clear()
{
    Q_D(QSortingProxyModel);
    d->sort_column = -1;
    d->sort_order = Qt::AscendingOrder;
    QMappingProxyModel::clear();    
}

/*!
  Sets the given \a function to be used as the < operator when sorting;
*/
void QSortingProxyModel::setLessThan(Compare *function)
{
    d_func()->less = function;
}

/*!
  Sets the given \a function to be used as the > operator when sorting;
*/
void QSortingProxyModel::setGreaterThan(Compare *function)
{
    d_func()->greater = function;
}

// protected

/*!
  \internal
*/
void QSortingProxyModel::sourceLayoutChanged()
{
    Q_D(QSortingProxyModel);

    QMappingProxyModel::sourceLayoutChanged();
    if (d->sort_column != -1)
        sort(d->sort_column, d->sort_order);
}

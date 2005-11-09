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
  \since 4.1
  \class QSortingProxyModel
  \brief The QSortingProxyModel class provides support for sorting data
  that is passed between another model and a view.

  \ingroup model-view

  Sorting proxy models apply mappings between a source model and a proxy model to expose
  a sorted items to other model/view components.

  QSortingProxyModel is designed to be used without subclassing, and provides two functions
  to allow the sorting algorithm to be customized:

  \list
  \o setLessThan() is used to specify the function to be used as a < (less than) operator
     when comparing data referenced by two model indexes. This function is used when the
     sorting model needs to sort data in ascending order.
  \o setGreaterThan() is used to specify the function to be used as a > (greater than)
     operator when comparing data referenced by two model indexes. This function is used
     when the sorting model needs to sort data in descending order.
  \endlist

  You must call \e both setLessThan() and setGreaterThan() functions if you want to change
  the default sorting behavior. If you only redefine one sorting function, the arrangement
  of sorted items for ascending sort order will not be the reverse of the arrangement for
  descending sort order.

  The clear() function is used to disable sorting and reset the sort order. It does not
  reset the sorting functions to the default ones provided by this class.

  \sa QFilteringProxyModel, QAbstractProxyModel, QAbstractItemModel, {Model/View Programming}
*/

/*!
    \typedef QSortingProxyModel::Compare

    Defines the type of function to be used when comparing items of
    data from the model. Comparison functions must have a signature in
    the following form:

    \code
    bool(*Compare)(const QModelIndex &left, const QModelIndex &right);
    \endcode
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
    Compare compare = (order == Qt::AscendingOrder ? d->less : d->greater);

    QModelIndexList persistent_from;
    QModelIndexList persistent_to;
    
    QModelIndexList map_from;
    QModelIndexList map_to;

    while (!source_parent_stack.isEmpty()) {

        QModelIndex source_parent = source_parent_stack.pop();
        for (int row = 0; row < sourceModel()->rowCount(source_parent); ++row) {
            QModelIndex source_index = sourceModel()->index(row, column, source_parent);
            if (sourceModel()->hasChildren(source_index))
                source_parent_stack.push(source_index);
            source_children.append(source_index);
        }

        qSort(source_children.begin(), source_children.end(), compare);

        QModelIndex proxy_parent = d->proxy_to_source.key(source_parent); // ### slow
        void *parent_node = 0;
        if (proxy_parent.isValid())
            parent_node = d->proxy_to_source.find(proxy_parent); // get the QMap node, used as uid in the proxy index

        // for each proxy_row, go through the source_column (same as proxy columns) and update the mapping
        int source_column_count = sourceModel()->columnCount(source_parent);
        int proxy_row_count = source_children.count();
        
        for (int proxy_row = 0; proxy_row < proxy_row_count; ++proxy_row) {
            int source_row = source_children.at(proxy_row).row();
            for (int source_column = 0; source_column < source_column_count; ++source_column) {
                QModelIndex source_index = sourceModel()->index(source_row, source_column,
                                                                source_parent);
                Q_ASSERT(source_index.isValid());
                QModelIndex old_proxy_index = d->proxy_to_source.key(source_index);
                QModelIndex new_proxy_index = createIndex(proxy_row, source_column, parent_node);
                if (old_proxy_index.isValid()) {                    
                    persistent_from.append(old_proxy_index);
                    persistent_to.append(new_proxy_index);
                    d->proxy_to_source.remove(old_proxy_index);
                }
                Q_ASSERT(new_proxy_index.isValid());
                Q_ASSERT(source_index.isValid());
                map_from.append(new_proxy_index);
                map_to.append(source_index);
            }
        }

        // do the mapping afterwards to avoid problems with looking up new and old indexes
        for (int i = 0; i < map_from.count(); ++i)
            d->proxy_to_source.insert(map_from.at(i), map_to.at(i));

        map_from.clear();
        map_to.clear();

        source_children.clear();
    }

    // this may be slow if we have many persistent indexes
    changePersistentIndexList(persistent_from, persistent_to);

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
void QSortingProxyModel::setLessThan(Compare function)
{
    d_func()->less = function;
}

/*!
  Sets the given \a function to be used as the > operator when sorting;
*/
void QSortingProxyModel::setGreaterThan(Compare function)
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

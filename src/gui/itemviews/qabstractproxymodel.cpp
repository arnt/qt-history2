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

#include "qabstractproxymodel.h"
#include "qitemselectionmodel.h"
#include <private/qabstractproxymodel_p.h>

/*!
    \since 4.1
    \class QAbstractProxyModel
    \brief The QAbstractProxyModel class provides a base class for proxy item models
    that can do sorting, filtering or other data processing tasks.
    \ingroup model-view

    This class defines the standard interface that proxy models must use to be able to
    interoperate correctly with other model/view components. It is not supposed to be
    instantiated directly.

    All standard proxy models are derived from the QAbstractProxyModel class. If you
    need to create a new proxy model class, it is usually better to subclass an existing
    class that provides the closest behavior to the one you want to provide. Proxy models
    that filter or sort items of data from a source model should be created by using or
    subclassing QSortFilterProxyModel.

    \sa QSortFilterProxyModel, QAbstractItemModel, {Model/View Programming}
*/


/*!
    Constructs a proxy model with the given \a parent.
*/

QAbstractProxyModel::QAbstractProxyModel(QObject *parent)
    :QAbstractItemModel(*new QAbstractProxyModelPrivate, parent)
{
    Q_D(QAbstractProxyModel);
    setSourceModel(&d->empty);
}

/*!
    \internal
*/

QAbstractProxyModel::QAbstractProxyModel(QAbstractProxyModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(QAbstractProxyModel);
    setSourceModel(&d->empty);
}

/*!
    Destroys the proxy model.
*/
QAbstractProxyModel::~QAbstractProxyModel()
{

}

/*!
    Sets the given \a sourceModel to be processed by the proxy model.
*/
void QAbstractProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    Q_D(QAbstractProxyModel);
    if (sourceModel)
        d->model = sourceModel;        
    else
        d->model = &d->empty;
}

/*!
    Returns the model that contains the data that is available through the proxy model.
*/
QAbstractItemModel *QAbstractProxyModel::sourceModel() const
{
    Q_D(const QAbstractProxyModel);
    return d->model;
}

/*!
    \reimp
 */
bool QAbstractProxyModel::submit()
{
    Q_D(QAbstractProxyModel);
    return d->model->submit();
}

/*!
    \reimp
 */
void QAbstractProxyModel::revert()
{
    Q_D(QAbstractProxyModel);
    d->model->revert();
}


/*!
  \fn QModelIndex QSortFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const

  Reimplement this method to map proxy indexes to source indexes. 
*/

/*!
  \fn QModelIndex mapFromSource(const QModelIndex &sourceIndex) const

  Reimplement this method to map source indexes to proxy indexes. 
*/

/*!
  Reimplement this method to map proxy selections to source selections. 
 */
QItemSelection QAbstractProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    QModelIndexList proxyIndexes = proxySelection.indexes();
    QItemSelection sourceSelection;
    for (int i = 0; i < proxyIndexes.size(); ++i) 
        sourceSelection << QItemSelectionRange(mapToSource(proxyIndexes.at(i)));
    return sourceSelection;
}

/*!
  Reimplement this method to map source selections to proxy selections. 
*/
QItemSelection QAbstractProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    QModelIndexList sourceIndexes = sourceSelection.indexes();
    QItemSelection proxySelection;
    for (int i = 0; i < sourceIndexes.size(); ++i) 
        proxySelection << QItemSelectionRange(mapFromSource(sourceIndexes.at(i)));
    return proxySelection;
}

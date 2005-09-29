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
#include <private/qabstractproxymodel_p.h>

/*!
    \since 4.1
    \class QAbstractProxyModel
    \brief The QAbstractProxyModel class provides a base class for proxy item models
    that can do sorting, filtering or other data processing tasks.
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

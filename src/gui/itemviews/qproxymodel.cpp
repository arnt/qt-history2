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

#include "qproxymodel.h"
#include <private/qproxymodel_p.h>
#include <qsize.h>
#include <qstringlist.h>

/*!
    \class QProxyModel
    \brief The QProxyModel class provides support for filtering and sorting data
    that is passed between another model and a view.

    Proxy models provide a standard model interface 

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel

*/

/*!
    Constructs a proxy model with the given \a parent.
*/

QProxyModel::QProxyModel(QObject *parent)
    : QAbstractItemModel(*new QProxyModelPrivate, parent)
{
    Q_D(QProxyModel);
    setModel(&d->empty);
    disconnect(this, SIGNAL(reset()), this, SLOT(resetPersistentIndexes()));
}

/*!
    \internal
*/
QProxyModel::QProxyModel(QProxyModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(QProxyModel);
    setModel(&d->empty);
    disconnect(this, SIGNAL(reset()), this, SLOT(resetPersistentIndexes()));
}

/*!
    Destroys the proxy model.
*/
QProxyModel::~QProxyModel()
{
}

/*!
    Sets the given \a model to be processed by the proxy model.
*/
void QProxyModel::setModel(QAbstractItemModel *model)
{
    Q_D(QProxyModel);
    if (d->model && d->model != &d->empty) {
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
        disconnect(d->model, SIGNAL(headerDataChanged(Orientation,int,int)),
                   this, SIGNAL(headerDataChanged(Orientation,int,int)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SIGNAL(rowsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                   this, SIGNAL(columnsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(reset()), this, SIGNAL(reset()));
    }

    if (model) {
        d->model = model;
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(headerDataChanged(Orientation,int,int)),
                this, SIGNAL(headerDataChanged(Orientation,int,int)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SIGNAL(rowsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                this, SIGNAL(columnsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(reset()), this, SIGNAL(reset()));
    } else {
        d->model = &d->empty;
    }
}

/*!
    Returns the model that contains the data that is available through the
    proxy model.
*/
QAbstractItemModel *QProxyModel::model() const
{
    Q_D(const QProxyModel);
    return d->model;
}

/*!
    Returns the model index with the given \a row, \a column, and \a parent.

    \sa QAbstractItemModel::index()
*/
QModelIndex QProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const QProxyModel);
    return d->model->index(row, column, parent);    
}

/*!
    Returns the model index that corresponds to the parent of the given \a child
    index.
*/
QModelIndex QProxyModel::parent(const QModelIndex &child) const
{
    Q_D(const QProxyModel);
    return d->model->parent(child);
}

/*!
    Returns the number of rows for the given \a parent.

    \sa QAbstractItemModel::rowCount()
*/
int QProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QProxyModel);
    return d->model->rowCount(parent);
}

/*!
    Returns the number of columns for the given \a parent.

    \sa QAbstractItemModel::columnCount()
*/
int QProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const QProxyModel);
    return d->model->columnCount(parent);
}

/*!
    Returns true if the item corresponding to the \a parent index has child
    items; otherwise returns false.

    \sa QAbstractItemModel::hasChildren()
*/
bool QProxyModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const QProxyModel);
    return d->model->hasChildren(parent);
}

/*!
    Returns the data stored in the item with the given \a index under the
    specified \a role.
*/
QVariant QProxyModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QProxyModel);
    return d->model->data(index, role);
}

/*!
    Sets the \a role data for the item at \a index to \a value.
    Returns true if successful; otherwise returns false.

    The base class implementation returns false. This function and
    data() must be reimplemented for editable models.

    \sa data() itemData() QAbstractItemModel::setData()
*/
bool QProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(const QProxyModel);
    return d->model->setData(index, value, role);
}

/*!
    Returns the data stored in the \a section of the header with specified
    \a orientation under the given \a role.
*/
QVariant QProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const QProxyModel);
    return d->model->headerData(section, orientation, role);
}

/*!
    Sets the \a role data in the \a section of the header with the specified
    \a orientation to the \a value given.

    \sa QAbstractItemModel::setHeaderData()
*/
bool QProxyModel::setHeaderData(int section, Qt::Orientation orientation,
                                const QVariant &value, int role)
{
    Q_D(const QProxyModel);
    return d->model->setHeaderData(section, orientation, value, role);
}

QStringList QProxyModel::mimeTypes() const
{
    Q_D(const QProxyModel);
    return d->model->mimeTypes();
}

QMimeData *QProxyModel::mimeData(const QModelIndexList &indexes) const
{
    Q_D(const QProxyModel);
    return d->model->mimeData(indexes);
}

bool QProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                               int row, const QModelIndex &parent)
{
    Q_D(const QProxyModel);
    return d->model->dropMimeData(data, action, row, parent);
}

Qt::DropActions QProxyModel::supportedDropActions() const
{
    Q_D(const QProxyModel);
    return d->model->supportedDropActions();
}

/*!
    Inserts \a count rows into the model, creating new items as children of
    the given \a parent. The new rows are inserted before the \a row
    specified. If the \a parent item has no children, a single column is
    created to contain the required number of rows.

    Returns true if the rows were successfully inserted; otherwise
    returns false.

    \sa QAbstractItemModel::insertRows()*/
bool QProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_D(const QProxyModel);
    return d->model->insertRows(row, count, parent);
}

/*!
    Inserts \a count columns into the model, creating new items as children of
    the given \a parent. The new columns are inserted before the \a column
    specified. If the \a parent item has no children, a single row is created
    to contain the required number of columns.

    Returns true if the columns were successfully inserted; otherwise
    returns false.

    \sa QAbstractItemModel::insertColumns()
*/
bool QProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(const QProxyModel);
    return d->model->insertColumns(column, count, parent);
}

/*!
    Fetches more child items of the given \a parent. This function is used by views
    to tell the model that they can display more data than the model has provided.

    \sa QAbstractItemModel::fetchMore()
*/
void QProxyModel::fetchMore(const QModelIndex &parent)
{
    Q_D(const QProxyModel);
    d->model->fetchMore(parent);
}

/*!
    Returns the item flags for the given \a index.

    \sa Qt::flags()
*/
Qt::ItemFlags QProxyModel::flags(const QModelIndex &index) const
{
    Q_D(const QProxyModel);
    return d->model->flags(index);
}

/*!
    Sorts the child items in the specified \a column of the given \a parent
    according to the sort order defined by \a order.

    \sa QAbstractItemModel::sort()
*/
void QProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_D(QProxyModel);
    d->model->sort(column, order);
}

/*!
    Returns a list of model indexes that each contain the given \a value for
    the \a role specified. The search begins at the \a start index and is
    performed according to the specified \a flags. The search continues until
    the number of matching data items equals \a hits, the last row is reached,
    or the search reaches \a start again, depending on whether \c MatchWrap is
    specified in \a flags.

    \sa QAbstractItemModel::match()
*/
QModelIndexList QProxyModel::match(const QModelIndex &start, int role,
                                   const QVariant &value,
                                   int hits, QAbstractItemModel::MatchFlags flags) const
{
    Q_D(const QProxyModel);
    return d->model->match(start, role, value, hits, flags);
}

/*!
    Returns the size of the item that corresponds to the specified \a index.
*/
QSize QProxyModel::span(const QModelIndex &index) const
{
    Q_D(const QProxyModel);
    return d->model->span(index);
}

/*!
 */
bool QProxyModel::submit()
{
    Q_D(QProxyModel);
    return d->model->submit();
}

/*!
 */
void QProxyModel::revert()
{
    Q_D(QProxyModel);
    d->model->revert();
}


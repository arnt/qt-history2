/****************************************************************************
 **
 ** Implementation of QSqlModel class.
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is part of the sql module of the Qt GUI Toolkit.
 ** EDITIONS: FREE, ENTERPRISE
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include "qsqlmodel.h"

#include <qsqldriver.h>

#include "qsqlmodel_p.h"

#define d d_func()

#define QSQL_PREFETCH 15 // ### make this configurable

void QSqlModelPrivate::prefetch()
{
    int limit = lastRowIndex + QSQL_PREFETCH;
    if (!query.seek(lastRowIndex)) {
        d->error = query.lastError();
        return;
    }
    while (lastRowIndex < limit) {
        if (!query.next()) {
            // hooray - found the end of the result set
            bottom = QModelIndex(lastRowIndex, bottom.column());
            atEnd = true;
            return;
        }
        ++lastRowIndex;
    }
    bottom = QModelIndex(lastRowIndex, bottom.column());
}

QSqlModel::QSqlModel(QObject *parent)
    : QAbstractItemModel(*new QSqlModelPrivate, parent)
{
}

QSqlModel::QSqlModel(QSqlModelPrivate &dd, QObject *parent = 0)
    : QAbstractItemModel(dd, parent)
{
}

QSqlModel::~QSqlModel()
{
}

/*! \reimp
 */
void QSqlModel::fetchMore()
{
    if (d->atEnd)
        return;
    QModelIndex idx = QModelIndex(d->bottom.row(), 0);
    d->prefetch();
    if (idx.row() != d->bottom.row())
        emit contentsInserted(idx, d->bottom);
}

/*! \reimp
 */
int QSqlModel::rowCount(const QModelIndex &) const
{
    return d->lastRowIndex + 1;
}

/*! \reimp
 */
int QSqlModel::columnCount(const QModelIndex &) const
{
    return d->rec.count();
}

/*! \reimp
    Returns the value for the specified \a item and \a role.

    For items with type QModelIndex::HorizontalHeader, the name of the database field
    will be returned. This can be overridden by setData().

    For items with type QModelIndex::VerticalHeader, the index of the row will be returned.

    An invalid QVariant is returned if \a item is out of bounds or an error occured.

    \sa setData(), lastError()
 */
QVariant QSqlModel::data(const QModelIndex &item, int role) const
{

    if (item.type() == QModelIndex::HorizontalHeader) {
        QVariant val = d->headers.value(item.column());
        if (val.type() != QVariant::Invalid)
            return val;
        return d->rec.fieldName(item.column());
    }
    if (item.type() == QModelIndex::VerticalHeader)
        return QString::number(item.row());

    QVariant v;
    if (role & ~(Display | Edit))
        return v;

    if (!d->rec.isGenerated(item.column()))
        return v;
    QModelIndex dItem = dataIndex(item);
    if (!d->query.seek(dItem.row())) {
        d->error = d->query.lastError();
        return v;
    }

    return d->query.value(dItem.column());
}


/*!
    Resets the model and sets the data provider to \a query. Note that the query
    has to be active and may not be forwardOnly.

    \sa query(), QSqlQuery::isActive(), QSqlQuery::setForwardOnly().
 */
void QSqlModel::setQuery(const QSqlQuery &query)
{
    d->error = QSqlError();
    d->query = query;
    d->rec = query.record();
    d->colOffsets.resize(d->rec.count());
    memset(d->colOffsets.data(), 0, d->colOffsets.size() * sizeof(int));
    if (d->bottom.isValid())
        emit contentsRemoved(index(0, 0, 0), d->bottom);
    if (!query.isActive() || query.isForwardOnly()) {
        d->atEnd = true;
        d->lastRowIndex = 0;
        d->bottom = QModelIndex();
        if (query.isForwardOnly())
            d->error = QSqlError("Forward-only queries cannot be used in a data model",
                                 QString(), QSqlError::Connection);
        else
            d->error = query.lastError();
        return;
    }
    if (d->query.driver()->hasFeature(QSqlDriver::QuerySize)) {
        d->atEnd = true;
        d->lastRowIndex = d->query.size();
        d->bottom = QModelIndex(d->lastRowIndex, d->rec.count() - 1);
    } else {
        d->lastRowIndex = 0;
        d->bottom = QModelIndex(0, d->rec.count() - 1);
//        d->prefetch();
    }
    emit contentsInserted(index(0, 0, 0), d->bottom);
}

/*! \reimp
    This method can be used to set the caption for the horizontal header of a column to \a value.

    returns true if \a role is QAbstractItemModel::Display and \a index is of type
    QModelIndex::HorizontalHeader and points to a valid column, otherwise false.

    Note that this method can not be used to modify actual values in the database since the model
    is read-only.

    \sa data()
 */
bool QSqlModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (role != Display || index.type() != QModelIndex::HorizontalHeader || index.row() <= 0
        || index.column() < 0)
        return false;

    if (d->headers.size() <= index.row())
        d->headers.resize(qMax(index.row() + 1, 16));
    d->headers[index.row()] = value;
    return true;
}

/*!
    Returns the QSqlQuery that is associated with this model.

    \sa setQuery()
 */
const QSqlQuery QSqlModel::query() const
{
    return d->query;
}

/*!
    Returns information about the last error that occurred on the database. See \a QSqlError for
    more information.
 */
QSqlError QSqlModel::lastError() const
{
    return d->error;
}

/*!
   Protected function which allows derived classes to set the value of the last \a error that
   occurred on the database.

   \sa lastError()
 */
void QSqlModel::setLastError(const QSqlError &error)
{
    d->error = error;
}

/*!
    Returns the field information about \a column or an invalid QSqlField if \a column is
    out of bounds.
 */
QSqlField QSqlModel::field(int column) const
{
    return d->rec.field(column);
}

/*! \reimp
    Inserts \a count columns at position \a column. The paramter \a parent should always be
    an invalid QModelIndex, since the model does not support parent-child relations.

    To populate the newly inserted columns with data, QSqlModel::data() has to be reimplemented.
    Example:

    \code
    QVariant MyModel::data(const QModelIndex &item, int role) const
    {
        if (item.column() == myNewlyInsertedColumn) {
            return "My calculated value";
        }
        return QSqlModel::data(item, role);
    }
    \endcode

    Returns false if \a column is out of bounds, otherwise true.
 */
bool QSqlModel::insertColumn(int column, const QModelIndex &parent, int count)
{
    if (count <= 0 || parent.isValid() || column < 0 || column > d->rec.count())
        return false;

    for (int c = 0; c < count; ++c) {
        QSqlField field;
        field.setReadOnly(true);
        d->rec.insert(column, field);
        d->rec.setGenerated(column, false);
        if (d->colOffsets.size() < d->rec.count()) {
            int nVal = d->colOffsets.isEmpty() ? 0 : d->colOffsets[d->colOffsets.size() - 1];
            d->colOffsets.append(nVal);
            Q_ASSERT(d->colOffsets.size() >= d->rec.count());
        }
        for (int i = column + 1; i < d->colOffsets.count(); ++i)
            ++d->colOffsets[i];
    }
    emit contentsInserted(QModelIndex(0, column), QModelIndex(rowCount(), column + count));
    return true;
}

/*! \reimp
    Removes \a count columns from position \a column. The paramter \a parent should always be
    an invalid QModelIndex, since the model does not support parent-child relations.

    Note that removing columns does not affect the underlying QSqlQuery, it will just hide the
    columns.

    Returns true if the columns could be removed, otherwise false.
 */
bool QSqlModel::removeColumn(int column, const QModelIndex &parent, int count)
{
    if (count <= 0 || parent.isValid() || column < 0 || column >= d->rec.count())
        return false;

    int i;
    for (i = 0; i < count; ++i)
        d->rec.remove(column);
    for (i = column; i < d->colOffsets.count(); ++i)
        d->colOffsets[i] -= count;
    emit contentsRemoved(QModelIndex(0, column), QModelIndex(rowCount(), column + count));
    return true;
}

/*!
    Returns the index of the value in the database result set for \a item in case the
    row and column of an item in the model does not map to the same row and column in the
    database result set.

    Returns an invalid model index if \a item is out of bounds or if \a item does not point
    to a value in the result set.
 */
QModelIndex QSqlModel::dataIndex(const QModelIndex &item) const
{
    if (item.column() < 0 || item.column() >= d->rec.count()
        || !d->rec.isGenerated(item.column()))
        return QModelIndex();
    return QModelIndex(item.row(), item.column() - d->colOffsets[item.column()],
                       item.data(), item.type());
}

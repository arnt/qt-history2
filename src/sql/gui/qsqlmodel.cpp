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
#include <qsqlfield.h>

#include "qsqlmodel_p.h"

#define d d_func()
#define q q_func()

#define QSQL_PREFETCH 15 // ### make this configurable

void QSqlModelPrivate::prefetch(int limit)
{
    if (atEnd || limit <= bottom.row())
        return;

    QModelIndex oldBottom = q->createIndex(bottom.row(), 0);

    // try to seek directly
    if (query.seek(limit)) {
        bottom = q->createIndex(limit, bottom.column());
    } else {
        // fetch as far as we can
        if (!query.last())
            error = query.lastError();
        else
            bottom = q->createIndex(query.at(), bottom.column());
        atEnd = true; // this is the end.
    }
    if (bottom.row() > oldBottom.row())
        emit q->rowsInserted(QModelIndex(), oldBottom.row(), bottom.row());
}

/*!
    \class QSqlModel
    \brief The QSqlModel class provides a read only data model for
           SQL result sets.

    \ingroup database
    \mainclass
    \module sql

    QSqlModel is a data model that provides data from a QSqlQuery.
    The QSqlQuery has to be valid and may not be forward-only.

    \code
    QSqlModel model;
    model.setQuery(QSqlQuery("SELECT * FROM TEST"));
    \endcode

    The model is read-only by default, to make it read-write, it is
    neccessary to reimplement the setData() method.

    QSqlTableModel is a convenience subclass which allows manipulating
    a database table.

    \sa QSqlTableModel QSqlQuery
*/

/*!
    Creates an empty QSqlModel and sets the parent to \a parent.
 */
QSqlModel::QSqlModel(QObject *parent)
    : QAbstractItemModel(*new QSqlModelPrivate, parent)
{
}

/*! \internal
 */
QSqlModel::QSqlModel(QSqlModelPrivate &dd, QObject *parent = 0)
    : QAbstractItemModel(dd, parent)
{
}

/*!
    Destroys the object and frees any allocated resources.
 */
QSqlModel::~QSqlModel()
{
}

/*! \reimp
 */
void QSqlModel::fetchMore()
{
    d->prefetch(d->bottom.row() + QSQL_PREFETCH);
}

/*! \reimp
 */
int QSqlModel::rowCount(const QModelIndex &) const
{
    return d->bottom.row() + 1;
}

/*! \reimp
 */
int QSqlModel::columnCount(const QModelIndex &) const
{
    return d->rec.count();
}

/*!
    \reimp

    Returns the value for the specified \a item and \a role.

    For items with type \c QModelIndex::HorizontalHeader, the name of
    the database field is returned. This can be overridden by
    setData().

    For items with type \c QModelIndex::VerticalHeader, the index of the
    row is returned.

    An invalid QVariant is returned if \a item is out of bounds or if
    an error occured.

    \sa setData(), lastError()
*/
QVariant QSqlModel::data(const QModelIndex &item, int role) const
{

    if (item.type() == QModelIndex::HorizontalHeader) {
        QVariant val = d->headers.value(item.column());
        if (val.isValid())
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
    if (dItem.row() > d->bottom.row())
        const_cast<QSqlModelPrivate *>(d)->prefetch(dItem.row());

    if (!d->query.seek(dItem.row())) {
        d->error = d->query.lastError();
        return v;
    }

    return d->query.value(dItem.column());
}


/*!
    Resets the model and sets the data provider to be the given \a
    query. Note that the query must be active and must not be
    isForwardOnly().

    \sa query(), QSqlQuery::isActive(), QSqlQuery::setForwardOnly()
*/
void QSqlModel::setQuery(const QSqlQuery &query)
{
    d->error = QSqlError();
    d->query = query;
    d->rec = query.record();
    d->colOffsets.resize(d->rec.count());
    memset(d->colOffsets.data(), 0, d->colOffsets.size() * sizeof(int));
    if (d->bottom.isValid())
        emit rowsRemoved(QModelIndex(), 0, d->bottom.row());
    if (!query.isActive() || query.isForwardOnly()) {
        d->atEnd = true;
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
        d->bottom = createIndex(d->query.size(), d->rec.count() - 1);
    } else {
        d->bottom = createIndex(0, d->rec.count() - 1);
    }
    emit rowsInserted(QModelIndex(), 0, d->bottom.row());
}

/*!
    \reimp

    This function is used to set the caption for the horizontal
    header of a column to \a value.

    It returns true if \a role is \c QAbstractItemModel::Display and \a
    index is of type \c QModelIndex::HorizontalHeader and points to a
    valid column; otherwise returns false.

    Note that this function cannot be used to modify values in the
    database since the model is read-only.

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
    Returns information about the last error that occurred on the
    database.
*/
QSqlError QSqlModel::lastError() const
{
    return d->error;
}

/*!
   Protected function which allows derived classes to set the value of
   the last error that occurred on the database to \a error.

   \sa lastError()
*/
void QSqlModel::setLastError(const QSqlError &error)
{
    d->error = error;
}

/*!
   Returns the record containing information about the fields
   that are currently displayed. Returns an empty record if
   the model has not yet been initialized.
*/
QSqlRecord QSqlModel::record() const
{
    return d->rec;
}

/*!
    \reimp

    Inserts \a count columns into the model at position \a column. The
    \a parent parameter must always be an invalid QModelIndex, since
    the model does not support parent-child relationships.

    To populate the newly inserted columns with data, you must
    reimplement QSqlModel::data().

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

    Returns true if \a column is within bounds; otherwise returns false.
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
    emit columnsInserted(parent, column, column + count - 1);
    return true;
}

/*!
    \reimp

    Removes \a count columns from the model starting from position \a
    column. The \a parent parameter must always be an invalid
    QModelIndex, since the model does not support parent-child
    relationships.

    Note that removing columns does not affect the underlying
    QSqlQuery, it will just hide the columns.

    Returns true if the columns were removed; otherwise returns false.
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
    emit columnsRemoved(parent, column, column + count - 1);
    return true;
}

/*!
    Returns the index of the value in the database result set for the
    given \a item for situations where the row and column of an item
    in the model does not map to the same row and column in the
    database result set.

    Returns an invalid model index if \a item is out of bounds or if
    \a item does not point to a value in the result set.
*/
QModelIndex QSqlModel::dataIndex(const QModelIndex &item) const
{
    if (item.column() < 0 || item.column() >= d->rec.count()
        || !d->rec.isGenerated(item.column()))
        return QModelIndex();
    return createIndex(item.row(), item.column() - d->colOffsets[item.column()],
                       item.data(), item.type());
}

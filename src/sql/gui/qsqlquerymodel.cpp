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

#include "qsqlquerymodel.h"

#include <qsqldriver.h>
#include <qsqlfield.h>

#include "qsqlquerymodel_p.h"

#define QSQL_PREFETCH 15 // ### make this configurable

void QSqlQueryModelPrivate::prefetch(int limit)
{
    if (atEnd || limit <= bottom.row())
        return;

    QModelIndex oldBottom = q_ptr->createIndex(bottom.row(), 0);

    // try to seek directly
    if (query.seek(limit)) {
        bottom = q_ptr->createIndex(limit, bottom.column());
    } else {
        // fetch as far as we can
        if (query.last()) {
            bottom = q_ptr->createIndex(query.at(), bottom.column());
        } else {
            error = query.lastError();
            bottom = q_ptr->createIndex(-1, bottom.column());
        }
        atEnd = true; // this is the end.
    }
    if (bottom.row() > oldBottom.row())
        emit q_ptr->rowsInserted(QModelIndex(), oldBottom.row(), bottom.row());
}

QSqlQueryModelPrivate::~QSqlQueryModelPrivate()
{

}

#define d d_func()

/*!
    \class QSqlQueryModel
    \brief The QSqlQueryModel class provides a read only data model for SQL
    result sets.

    \ingroup database
    \module sql

    QSqlQueryModel is a data model that provides data from a QSqlQuery.
    The QSqlQuery has to be valid and may not be forward-only.

    \code
    QSqlQueryModel model;
    model.setQuery("SELECT FORENAME, SURNAME, ID FROM TEST");
    \endcode

    The model is read-only by default, to make it read-write, it is
    neccessary to reimplement the setData() and isEditable() methods.

    QSqlTableModel is a convenience subclass which allows manipulating
    a database table.

    \sa QSqlTableModel QSqlQuery
*/

/*!
    Creates an empty QSqlQueryModel and sets the parent to \a parent.
 */
QSqlQueryModel::QSqlQueryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    d_ptr = new QSqlQueryModelPrivate(this);
}

/*! \internal
 */
QSqlQueryModel::QSqlQueryModel(QSqlQueryModelPrivate &dd, QObject *parent = 0)
    : QAbstractTableModel(parent)
{
    d_ptr = &dd;
}

/*!
    Destroys the object and frees any allocated resources.
 */
QSqlQueryModel::~QSqlQueryModel()
{
    delete d;
}

/*!
    \internal
*/
void QSqlQueryModel::fetchMore()
{
    d->prefetch(d->bottom.row() + QSQL_PREFETCH);
}

/*! \reimp
 */
int QSqlQueryModel::rowCount() const
{
    return d->bottom.row() + 1;
}

/*! \reimp
 */
int QSqlQueryModel::columnCount() const
{
    return d->rec.count();
}

/*!
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
QVariant QSqlQueryModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid())
        return QVariant();

    QVariant v;
    if (role & ~(DisplayRole | EditRole))
        return v;

    if (!d->rec.isGenerated(item.column()))
        return v;
    QModelIndex dItem = dataIndex(item);
    if (dItem.row() > d->bottom.row())
        const_cast<QSqlQueryModelPrivate *>(d)->prefetch(dItem.row());

    if (!d->query.seek(dItem.row())) {
        d->error = d->query.lastError();
        return v;
    }

    return d->query.value(dItem.column());
}

/*!
    Returns the header data for the given \a role in the \a section of the
    header with the specified \a orientation.
*/
QVariant QSqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        QVariant val = d->headers.value(section);
        if (val.isValid())
            return val;
        return d->rec.fieldName(section);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

/*!
    Resets the model and sets the data provider to be the given \a
    query. Note that the query must be active and must not be
    isForwardOnly().

    \sa query(), QSqlQuery::isActive(), QSqlQuery::setForwardOnly()
*/
void QSqlQueryModel::setQuery(const QSqlQuery &query)
{
    if (d->bottom.isValid()) {
        emit rowsRemoved(QModelIndex(), 0, d->bottom.row());
        emit columnsRemoved(QModelIndex(), 0, d->bottom.column());
    }
    d->bottom = QModelIndex();
    d->error = QSqlError();
    d->atEnd = false;
    d->query = query;
    d->rec = query.record();
    d->colOffsets.resize(d->rec.count());
    memset(d->colOffsets.data(), 0, d->colOffsets.size() * sizeof(int));
    if (!query.isActive() || query.isForwardOnly()) {
        d->atEnd = true;
        d->bottom = QModelIndex();
        if (query.isForwardOnly())
            d->error = QSqlError(QLatin1String("Forward-only queries "
                                               "cannot be used in a data model"),
                                 QString(), QSqlError::ConnectionError);
        else
            d->error = query.lastError();
        return;
    }
    if (d->query.driver()->hasFeature(QSqlDriver::QuerySize)) {
        d->atEnd = true;
        d->bottom = createIndex(d->query.size(), d->rec.count() - 1);
    } else {
        d->bottom = createIndex(0, d->rec.count() - 1);
        fetchMore();
    }
    emit rowsInserted(QModelIndex(), 0, d->bottom.row());
    emit columnsInserted(QModelIndex(), 0, d->bottom.column());
}

/*! \overload
    Convenience method, executes the query \a query for the given connection \a db.
    If no database is specified, the default connection is used.
 */
void QSqlQueryModel::setQuery(const QString &query, const QSqlDatabase &db)
{
    setQuery(QSqlQuery(query, db));
}

/*!
    Clears the model and releases all aquired resources
 */
void QSqlQueryModel::clear()
{
    d->error = QSqlError();
    d->atEnd = true;
    d->query.clear();
    d->rec.clear();
    d->colOffsets.clear();
    d->bottom = QModelIndex();
    d->headers.clear();
}

/*!
    Sets the caption for the header with the given \a orientation to the
    specified \a value.

    It returns true if \a role is \c QAbstractItemModel::DisplayRole and
    the \a section refers to a valid section; otherwise returns false.

    Note that this function cannot be used to modify values in the
    database since the model is read-only.

    \sa data()
 */
bool QSqlQueryModel::setHeaderData(int section, Qt::Orientation orientation, int role,
                                   const QVariant &value)
{
    if (role != DisplayRole || orientation != Qt::Horizontal || section < 0)
        return false;

    if (d->headers.size() <= section)
        d->headers.resize(qMax(section + 1, 16));
    d->headers[section] = value;
    return true;
}

/*!
    Returns the QSqlQuery that is associated with this model.

    \sa setQuery()
*/
const QSqlQuery QSqlQueryModel::query() const
{
    return d->query;
}

/*!
    Returns information about the last error that occurred on the
    database.
*/
QSqlError QSqlQueryModel::lastError() const
{
    return d->error;
}

/*!
   Protected function which allows derived classes to set the value of
   the last error that occurred on the database to \a error.

   \sa lastError()
*/
void QSqlQueryModel::setLastError(const QSqlError &error)
{
    d->error = error;
}

/*!
    Returns the record containing information about the fields
    in the database. If \a row points to a valid row, the
    record will be populated with values from that row.

    If the model is not initialized, en empty record will be
    returned.
*/
QSqlRecord QSqlQueryModel::record(int row) const
{
    if (row < 0)
        return d->rec;

    QSqlRecord rec = d->rec;
    for (int i = 0; i < rec.count(); ++i)
        rec.setValue(i, data(createIndex(row, i), EditRole));
    return rec;
}

/*!
    Inserts \a count columns into the model at position \a column. The
    \a parent parameter must always be an invalid QModelIndex, since
    the model does not support parent-child relationships.

    To populate the newly inserted columns with data, you must
    reimplement QSqlQueryModel::data().

    Example:
    \code
    QVariant MyModel::data(const QModelIndex &item, int role) const
    {
        if (item.column() == myNewlyInsertedColumn) {
            return "My calculated value";
        }
        return QSqlQueryModel::data(item, role);
    }
    \endcode

    Returns true if \a column is within bounds; otherwise returns false.
 */
bool QSqlQueryModel::insertColumn(int column, const QModelIndex &parent, int count)
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
    Removes \a count columns from the model starting from position \a
    column. The \a parent parameter must always be an invalid
    QModelIndex, since the model does not support parent-child
    relationships.

    Note that removing columns does not affect the underlying
    QSqlQuery, it will just hide the columns.

    Returns true if the columns were removed; otherwise returns false.
 */
bool QSqlQueryModel::removeColumn(int column, const QModelIndex &parent, int count)
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
QModelIndex QSqlQueryModel::dataIndex(const QModelIndex &item) const
{
    if (item.column() < 0 || item.column() >= d->rec.count()
        || !d->rec.isGenerated(item.column()))
        return QModelIndex();
    return createIndex(item.row(), item.column() - d->colOffsets[item.column()], item.data());
}

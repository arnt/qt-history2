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
    Q_Q(QSqlQueryModel);

    if (atEnd || limit <= bottom.row())
        return;

    QModelIndex oldBottom = q->createIndex(bottom.row(), 0);
    QModelIndex newBottom;

    // try to seek directly
    if (query.seek(limit)) {
        newBottom = q->createIndex(limit, bottom.column());
    } else {
        // fetch as far as we can
        if (query.last()) {
            newBottom = q->createIndex(query.at(), bottom.column());
        } else {
            error = query.lastError();
            newBottom = q->createIndex(-1, bottom.column());
        }
        atEnd = true; // this is the end.
    }
    if (newBottom.row() > oldBottom.row()) {
        emit q->rowsAboutToBeInserted(QModelIndex(), oldBottom.row(), newBottom.row());
        bottom = newBottom;
        emit q->rowsInserted(QModelIndex(), oldBottom.row(), bottom.row());
    }
}

QSqlQueryModelPrivate::~QSqlQueryModelPrivate()
{
}

/*!
    \class QSqlQueryModel
    \brief The QSqlQueryModel class provides a read-only data model for SQL
    result sets.

    \ingroup database
    \module sql

    QSqlQueryModel is a high-level interface for executing SQL
    statements and traversing the result set. It is built on top of
    the lower-level QSqlQuery and can be used to provide data to
    view classes such as QTableView. For example:

    \quotefromfile snippets/sqldatabase/sqldatabase.cpp
    \skipto QSqlQueryModel_snippets
    \skipto QSqlQueryModel *model
    \printuntil show()

    We set the model's query, then we set up the labels displayed in
    the view header.

    QSqlQueryModel can also be used to access a database
    programmatically, without binding it to a view:

    \skipto QSqlQueryModel model;
    \printuntil int salary =

    The code snippet above extracts the \c salary field from record 4 in
    the result set of the query \c{SELECT * from employee}. Assuming
    that \c salary is column 2, we can rewrite the last line as follows:

    \skipto int salary =
    \printline int salary =

    The model is read-only by default. To make it read-write, you
    must subclass it and reimplement setData() and flags(). Another
    option is to use QSqlTableModel, which provides a read-write
    model based on a single database table.

    The \l{sql/querymodel} example illustrates how to use
    QSqlQueryModel to display the result of a query. It also shows
    how to subclass QSqlQueryModel to customize the contents of the
    data before showing it to the user, and how to create a
    read-write model based on QSqlQueryModel.

    \sa QSqlTableModel, QSqlRelationalTableModel, QSqlQuery,
        {Model/View Programming}
*/

/*!
    Creates an empty QSqlQueryModel with the given \a parent.
 */
QSqlQueryModel::QSqlQueryModel(QObject *parent)
    : QAbstractTableModel(*new QSqlQueryModelPrivate, parent)
{
}

/*! \internal
 */
QSqlQueryModel::QSqlQueryModel(QSqlQueryModelPrivate &dd, QObject *parent)
    : QAbstractTableModel(dd, parent)
{
}

/*!
    Destroys the object and frees any allocated resources.

    \sa clear()
*/
QSqlQueryModel::~QSqlQueryModel()
{
}

/*!
    \reimp
*/
void QSqlQueryModel::fetchMore(const QModelIndex &parent)
{
    Q_D(QSqlQueryModel);
    if (parent.isValid())
        return;
    d->prefetch(d->bottom.row() + QSQL_PREFETCH);
}

/*! \reimp
 */
bool QSqlQueryModel::canFetchMore(const QModelIndex &parent) const
{
    Q_D(const QSqlQueryModel);
    return (!parent.isValid() && !d->atEnd);
}

/*! \reimp
 */
int QSqlQueryModel::rowCount(const QModelIndex &) const
{
    Q_D(const QSqlQueryModel);
    return d->bottom.row() + 1;
}

/*! \reimp
 */
int QSqlQueryModel::columnCount(const QModelIndex &) const
{
    Q_D(const QSqlQueryModel);
    return d->rec.count();
}

/*!
    Returns the value for the specified \a item and \a role.

    If \a item is out of bounds or if an error occurred, an invalid
    QVariant is returned.

    \sa lastError()
*/
QVariant QSqlQueryModel::data(const QModelIndex &item, int role) const
{
    Q_D(const QSqlQueryModel);
    if (!item.isValid())
        return QVariant();

    QVariant v;
    if (role & ~(Qt::DisplayRole | Qt::EditRole))
        return v;

    if (!d->rec.isGenerated(item.column()))
        return v;
    QModelIndex dItem = indexInQuery(item);
    if (dItem.row() > d->bottom.row())
        const_cast<QSqlQueryModelPrivate *>(d)->prefetch(dItem.row());

    if (!d->query.seek(dItem.row())) {
        d->error = d->query.lastError();
        return v;
    }

    return d->query.value(dItem.column());
}

/*!
    Returns the header data for the given \a role in the \a section
    of the header with the specified \a orientation.
*/
QVariant QSqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const QSqlQueryModel);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        QVariant val = d->headers.value(section);
        if (val.isValid())
            return val;
        if (d->rec.count() > section)
            return d->rec.fieldName(section);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

/*!
    This virtual function is called whenever the query changes. The
    default implementation does nothing.

    query() returns the new query.

    \sa query(), setQuery()
 */
void QSqlQueryModel::queryChange()
{
    // do nothing
}

/*!
    Resets the model and sets the data provider to be the given \a
    query. Note that the query must be active and must not be
    isForwardOnly().

    \sa query(), QSqlQuery::isActive(), QSqlQuery::setForwardOnly()
*/
void QSqlQueryModel::setQuery(const QSqlQuery &query)
{
    Q_D(QSqlQueryModel);
    QSqlRecord newRec = query.record();
    bool columnsChanged = (newRec != d->rec);
    bool hasQuerySize = d->query.driver()->hasFeature(QSqlDriver::QuerySize);

    if (d->bottom.isValid()) {
        emit rowsAboutToBeRemoved(QModelIndex(), 0, d->bottom.row());
        if (columnsChanged)
            emit columnsAboutToBeRemoved(QModelIndex(), 0, d->bottom.column());
    }

    if (d->colOffsets.size() != newRec.count() || columnsChanged) {
        d->colOffsets.resize(newRec.count());
        memset(d->colOffsets.data(), 0, d->colOffsets.size() * sizeof(int));
    }

    d->bottom = QModelIndex();
    d->error = QSqlError();
    d->atEnd = false;
    d->query = query;
    d->rec = newRec;
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
    QModelIndex newBottom;
    if (hasQuerySize) {
        newBottom = createIndex(d->query.size() - 1, d->rec.count() - 1);
        emit rowsAboutToBeInserted(QModelIndex(), 0, newBottom.row());
        d->atEnd = true;
    } else {
        newBottom = createIndex(0, d->rec.count() - 1);
    }
    if (columnsChanged)
        emit columnsAboutToBeInserted(QModelIndex(), 0, newBottom.column());
    d->bottom = newBottom;

    queryChange();

    if (hasQuerySize) {
        emit rowsInserted(QModelIndex(), 0, d->bottom.row());
    } else {
        // fetchMore does the rowsInserted stuff
        fetchMore();
    }
    if (columnsChanged)
        emit columnsInserted(QModelIndex(), 0, d->bottom.column());
}

/*! \overload

    Executes the query \a query for the given database connection \a
    db. If no database is specified, the default connection is used.

    \sa query(), queryChange()
*/
void QSqlQueryModel::setQuery(const QString &query, const QSqlDatabase &db)
{
    setQuery(QSqlQuery(query, db));
}

/*!
    Clears the model and releases any aquired resource.
*/
void QSqlQueryModel::clear()
{
    Q_D(QSqlQueryModel);
    d->error = QSqlError();
    d->atEnd = true;
    d->query.clear();
    d->rec.clear();
    d->colOffsets.clear();
    d->bottom = QModelIndex();
    d->headers.clear();
}

/*!
    Sets the caption for the header with the given \a orientation to
    the specified \a value. This is useful if the model is used to
    display data in a view (e.g., QTableView).

    Returns true if \a role is \c Qt::DisplayRole and
    the \a section refers to a valid section; otherwise returns
    false.

    Note that this function cannot be used to modify values in the
    database since the model is read-only.

    \sa data()
 */
bool QSqlQueryModel::setHeaderData(int section, Qt::Orientation orientation,
                                   const QVariant &value, int role)
{
    Q_D(QSqlQueryModel);
    if ((role != Qt::EditRole && role != Qt::DisplayRole)
        || orientation != Qt::Horizontal || section < 0)
        return false;

    if (d->headers.size() <= section)
        d->headers.resize(qMax(section + 1, 16));
    d->headers[section] = value;
    return true;
}

/*!
    Returns the QSqlQuery associated with this model.

    \sa setQuery()
*/
QSqlQuery QSqlQueryModel::query() const
{
    Q_D(const QSqlQueryModel);
    return d->query;
}

/*!
    Returns information about the last error that occurred on the
    database.
*/
QSqlError QSqlQueryModel::lastError() const
{
    Q_D(const QSqlQueryModel);
    return d->error;
}

/*!
   Protected function which allows derived classes to set the value of
   the last error that occurred on the database to \a error.

   \sa lastError()
*/
void QSqlQueryModel::setLastError(const QSqlError &error)
{
    Q_D(QSqlQueryModel);
    d->error = error;
}

/*!
    Returns the record containing information about the fields of the
    current query. If \a row is the index of a valid row, the record
    will be populated with values from that row.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSqlRecord::isEmpty()
*/
QSqlRecord QSqlQueryModel::record(int row) const
{
    Q_D(const QSqlQueryModel);
    if (row < 0)
        return d->rec;

    QSqlRecord rec = d->rec;
    for (int i = 0; i < rec.count(); ++i)
        rec.setValue(i, data(createIndex(row, i), Qt::EditRole));
    return rec;
}

/*! \overload

    Returns an empty record containing information about the fields
    of the current query.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSqlRecord::isEmpty()
 */
QSqlRecord QSqlQueryModel::record() const
{
    Q_D(const QSqlQueryModel);
    return d->rec;
}

/*!
    Inserts \a count columns into the model at position \a column. The
    \a parent parameter must always be an invalid QModelIndex, since
    the model does not support parent-child relationships.

    Returns true if \a column is within bounds; otherwise returns false.

    By default, inserted columns are empty. To fill them with data,
    reimplement data() and handle any inserted column separately:

    \quotefromfile snippets/sqldatabase/sqldatabase.cpp
    \skipto QSqlQueryModel_snippets
    \skipto MyModel::data(
    \printuntil /^\}/

    \sa removeColumns()
*/
bool QSqlQueryModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(QSqlQueryModel);
    if (count <= 0 || parent.isValid() || column < 0 || column > d->rec.count())
        return false;

    emit columnsAboutToBeInserted(parent, column, column + count - 1);
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

    Removing columns effectively hides them. It does not affect the
    underlying QSqlQuery.

    Returns true if the columns were removed; otherwise returns false.
 */
bool QSqlQueryModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(QSqlQueryModel);
    if (count <= 0 || parent.isValid() || column < 0 || column >= d->rec.count())
        return false;

    emit columnsAboutToBeRemoved(parent, column, column + count - 1);

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
    given \a item in the model.

    The return value is identical to \a item if no columns or rows
    have been inserted, removed, or moved around.

    Returns an invalid model index if \a item is out of bounds or if
    \a item does not point to a value in the result set.

    \sa QSqlTableModel::indexInQuery(), insertColumns(), removeColumns()
*/
QModelIndex QSqlQueryModel::indexInQuery(const QModelIndex &item) const
{
    Q_D(const QSqlQueryModel);
    if (item.column() < 0 || item.column() >= d->rec.count()
        || !d->rec.isGenerated(item.column()))
        return QModelIndex();
    return createIndex(item.row(), item.column() - d->colOffsets[item.column()],
                       item.internalPointer());
}


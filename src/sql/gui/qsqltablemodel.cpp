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

#include "qsqltablemodel.h"

#include "qhash.h"
#include "qsqlindex.h"
#include "qsqlquery.h"
#include "qsqlrecord.h"

#include "qsqlmodel_p.h"

class QSqlTableModelPrivate: public QSqlModelPrivate
{
    Q_DECLARE_PUBLIC(QSqlTableModel);
public:
    QSqlTableModelPrivate()
        : editIndex(-1), insertIndex(-1), sortColumn(-1),
          sortOrder(Qt::Ascending),
          strategy(QSqlTableModel::OnFieldChange) {}
    void clear();
    QString whereClause() const;
    void setPrimaryValues(int index);
    void clearEditBuffer();
    QSqlRecord record(const QVector<QVariant> &values) const;

    QSqlDatabase db;
    int editIndex;
    int insertIndex;

    int sortColumn;
    Qt::SortOrder sortOrder;

    QSqlTableModel::EditStrategy strategy;

    QSqlQuery editQuery;
    QSqlIndex primaryIndex;
    QString tableName;
    QString filter;

    QSqlRecord editBuffer;

    typedef QHash<int, QVector<QVariant> > CacheHash;
    CacheHash cache;
};

#define d d_func()

/*! \internal
    Populates our record with values
 */
QSqlRecord QSqlTableModelPrivate::record(const QVector<QVariant> &values) const
{
    QSqlRecord r = rec;
    for (int i = 0; i < r.count() && i < values.count(); ++i)
        r.setValue(i, values.at(i));
    return r;
}

void QSqlTableModelPrivate::clear()
{
    editIndex = -1;
    sortColumn = -1;
    sortOrder = Qt::Ascending;
    tableName = QString();
    editQuery.clear();
    editBuffer.clear();
    cache.clear();
    primaryIndex.clear();
    rec.clear();
    filter.clear();
}

void QSqlTableModelPrivate::clearEditBuffer()
{
    editBuffer = d->rec;
}

QString QSqlTableModelPrivate::whereClause() const
{
    QString clause;
    const QSqlRecord r = primaryIndex.isEmpty() ? rec : primaryIndex;
    for (int i = 0; i < r.count(); ++i)
        clause.append(r.fieldName(i)).append(" = ? and ");
    if (!clause.isEmpty()) {
        clause.prepend(" where ");
        // remove tailing "and"
        clause.truncate(clause.length() - 5);
    }
    return clause;
}

void QSqlTableModelPrivate::setPrimaryValues(int index)
{
    if (!query.seek(index)) {
        error = query.lastError();
        return;
    }
    if (primaryIndex.isEmpty()) {
        for (int i = 0; i < rec.count(); ++i)
            editQuery.addBindValue(query.value(i));
    } else {
        for (int i = 0; i < primaryIndex.count(); ++i)
            editQuery.addBindValue(query.value(rec.indexOf(primaryIndex.fieldName(i))));
    }
}

QSqlTableModel::QSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlModel(*new QSqlTableModelPrivate, parent)
{
    d->db = db.isValid() ? db : QSqlDatabase::database();
}

QSqlTableModel::~QSqlTableModel()
{
}

/*!
    Sets the table to \a tableName. Does not select data from the table, but
    fetches its field information.

    To populate the model with the table's data, call select().

    \sa select(), setFilter(), sort()
 */
void QSqlTableModel::setTable(const QString &tableName)
{
    d->tableName = tableName;
    d->rec = d->db.record(tableName);
    d->primaryIndex = d->db.primaryIndex(tableName);
}

/*!
    Returns the name of the currently selected table.
 */
QString QSqlTableModel::tableName() const
{
    return d->tableName;
}

/*!
    Populates the model with data from the table that was set via setTable(), using the
    specified filter and sort condition.

    \sa setTable(), setFilter(), sort()
 */
bool QSqlTableModel::select()
{
    QString query = selectStatement();
    if (query.isEmpty())
        return false;

    cancelChanges();
    QSqlQuery q(query, d->db);
    setQuery(q);
    return q.isActive();
}

/*! \reimp
    Returns the data for the item at position \a idx for the role \a role.
    Returns an invalid variant if \a idx is out of bounds.
 */
QVariant QSqlTableModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid()) // ###hack for breakage in tableview
        return QVariant();

    QModelIndex item = dataIndex(idx);
    if (idx.row() == d->insertIndex) {
        if (item.type() == QModelIndex::VerticalHeader) {
            return "*";
        } else if (item.type() == QModelIndex::View) {
            QVariant val;
            if (item.column() < 0 || item.column() >= d->rec.count())
                return val;
            val = d->editBuffer.value(idx.column());
            if (val.type() == QVariant::Invalid)
                val = QVariant(d->rec.field(item.column()).type());
            return val;
        }
    }

    if (item.type() != QModelIndex::View)
        return QSqlModel::data(item, role);

    switch (d->strategy) {
    case OnFieldChange:
        break;
    case OnRowChange:
        if (d->editIndex == item.row()) {
            QVariant var = d->editBuffer.value(item.column());
            if (var.isValid())
                return var;
        }
        break;
    case OnManualSubmit: {
        QVariant var = d->cache.value(item.row()).value(item.column());
        if (var.isValid())
            return var;
        break; }
    }
    return QSqlModel::data(item, role);
}

/*! \reimp
    Sets the data for the item \a index for the role \a role to \a value.
    Depending on the edit strategy, the value might be applied to the database at once or
    cached in the model.

    Returns true if the value could be set or false on error, for example if \a index is
    out of bounds.

    \sa editStrategy(), data(), submitChanges(), cancelChanges()
 */
bool QSqlTableModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (index.type() != QModelIndex::View)
        return QSqlModel::setData(index, role, value);

    QSqlRecord rec = query().record();
    if (index.column() >= rec.count())
        return false;

    if (index.row() == d->insertIndex) {
        d->editBuffer.setValue(index.column(), value);
        return true;
    }

    switch (d->strategy) {
    case OnFieldChange: {
        d->clearEditBuffer();
        d->editBuffer.setValue(index.column(), value);
        if (update(index.row(), d->editBuffer))
            d->query.exec(d->query.lastQuery()); // ### Refresh
        break;
    }
    case OnRowChange:
        if (d->editIndex != index.row()) {
            if (d->editBuffer.isEmpty())
                d->clearEditBuffer();
            else if (d->editIndex != -1)
                submitChanges();
        }
        d->editBuffer.setValue(index.column(), value);
        d->editIndex = index.row();
        break;
    case OnManualSubmit:
        if (!d->cache.contains(index.row())) {
            QVector<QVariant> vec;
            vec.resize(query().record().count());
            vec[index.column()] = value;
            d->cache[index.row()] = vec;
        } else {
            d->cache[index.row()][index.column()] = value;
        }
        break;
    }
    return true;
}

/*! \reimp
 */
void QSqlTableModel::setQuery(const QSqlQuery &query)
{
    QSqlModel::setQuery(query);
}

/*!
    Returns a SQL statement for inserting the \a values.
    By default, the query will contain placeholders, the values will be set later.
    Returns an empty string on error.

    \sa QSqlQuery::prepare, update()
 */
QString QSqlTableModel::updateStatement(const QSqlRecord &values) const
{
    QString stmt;
    stmt.reserve(0xff); // magic number
    stmt.append("update ").append(d->tableName).append(" set ");

    int i;
    for (i = 0; i < values.count(); ++i) {
        if (values.value(i).isValid()) // ### generated?
            stmt.append(values.fieldName(i)).append(" = ?, ");
    }
    if (stmt.at(stmt.length() - 2) != QLatin1Char(',')) // nothing to update
        return QString();

    stmt.truncate(stmt.length() - 2);

    stmt.append(d->whereClause());

    qDebug("updateStatement: %s", stmt.ascii());
    return stmt;
}

bool QSqlTableModel::update(int row, const QSqlRecord &values)
{
    const QString stmt(updateStatement(values));
    if (stmt.isEmpty() || row < 0 || row >= rowCount())
        return false; // nothing to update
    if (d->editQuery.lastQuery() != stmt) {
        if (!d->editQuery.prepare(stmt)) {
            d->error = d->editQuery.lastError();
            return false;
        }
    }
    for (int i = 0; i < values.count(); ++i) {
        const QVariant v(values.value(i));
        if (v.isValid()) // ### generated?
            d->editQuery.addBindValue(v);
    }
    d->setPrimaryValues(row);
    if (!d->editQuery.exec()) {
        d->error = d->editQuery.lastError();
        return false;
    }
    qDebug("executed: %s", d->editQuery.executedQuery().ascii());
    return true;
}

bool QSqlTableModel::insert(const QSqlRecord &values)
{
    const QString stmt(insertStatement());
    if (stmt.isEmpty())
        return false;

    if (d->editQuery.lastQuery() != stmt) {
        if (!d->editQuery.prepare(stmt)) {
            d->error = d->editQuery.lastError();
            return false;
        }
    }

    for (int i = 0; i < values.count(); ++i)
        d->editQuery.addBindValue(values.value(i)); // ### generated
    if (!d->editQuery.exec()) {
        d->error = d->editQuery.lastError();
        return false;
    }
    qDebug("executed: %s", d->editQuery.executedQuery().ascii());
    return true;
}

/*!
    Submits all pending changes and returns true on success.
    \sa lastError()
 */
bool QSqlTableModel::submitChanges()
{
    switch (d->strategy) {
    case OnFieldChange:
        return true;
    case OnRowChange:
        if (d->editBuffer.isEmpty())
            return true;
        if (!update(d->editIndex, d->editBuffer))
            return false;
        d->query.exec(d->query.lastQuery()); // ### Refresh
        d->clearEditBuffer();
        d->editIndex = -1;
        break;
    case OnManualSubmit:
        QSqlTableModelPrivate::CacheHash::const_iterator i = d->cache.constBegin();
        while (i != d->cache.constEnd()) {
            update(i.key(), d->record(i.value())); // ### error handling
            ++i;
        }
        d->cache.clear();
        d->query.exec(d->query.lastQuery()); // ### Refresh
        break;
    }
    return true;
}


/*!
    \enum QSqlTableModel::EditStrategy

    This enum type describes which strategy to choose when editing values in the database.

    \value OnFieldChange  All changes to the model will be applied immediately to the database.
    \value OnRowChange  Changes will be applied when the current row changes.
    \value OnManualSubmit  All changes will be cached in the model until either submitChanges()
                           or cancelChanges() is invoked.
*/


/*!
    Sets the strategy for editing values in the database to \a strategy.

    \sa EditStrategy, editStrategy()
 */
void QSqlTableModel::setEditStrategy(EditStrategy strategy)
{
    cancelChanges();
    d->strategy = strategy;
}

/*!
    Returns the current edit strategy.

    \sa EditStrategy, setEditStrategy()
 */
QSqlTableModel::EditStrategy QSqlTableModel::editStrategy() const
{
    return d->strategy;
}

/*!
    Cancels all pending changes.
 */
void QSqlTableModel::cancelChanges()
{
    d->editQuery.clear();
    d->editBuffer.clear();
    d->cache.clear();
    d->editIndex = -1;
}

/*!
    Returns the primary key for the current table, or an empty QSqlIndex
    if the table is not set or has no primary key.
 */
QSqlIndex QSqlTableModel::primaryKey() const
{
    return d->primaryIndex;
}

/*!
    Protected method to allow subclasses to set the primary key to \a key.
 */
void QSqlTableModel::setPrimaryKey(const QSqlIndex &key)
{
    d->primaryIndex = key;
}

/*!
    Returns a pointer to the used QSqlDatabase or 0 if no database was set.
 */
QSqlDatabase QSqlTableModel::database() const
{
     return d->db;
}

/*! \reimp
 */
bool QSqlTableModel::isSortable() const
{
    return true;
}

/*! \reimp
    Sorts the data by \a column with the sort order \a order.
    This will immediately select data, use setSort()
    to set a sort order without populating the model with data.

    \sa setSort(), isSortable(), select(), orderByStatement()
 */
void QSqlTableModel::sort(int column, SortOrder order)
{
    d->sortColumn = column;
    d->sortOrder = order;
    select();
}

/*!
    Sets the sort oder for \a column to \a order. This does not
    affect the current data, to refresh the data using the new
    sort order, call select().

    \sa select(), sort(), isSortable(), orderByStatement()
 */
void QSqlTableModel::setSort(int column, SortOrder order)
{
   d->sortColumn = column;
   d->sortOrder = order;
}

/*!
    Returns a SQL 'ORDER BY' statement based on the currently set
    sort order.

    \sa sort()
 */
QString QSqlTableModel::orderByStatement() const
{
    QString s;
    QSqlField f = d->rec.field(d->sortColumn);
    if (!f.isValid())
        return s;
    s.append("ORDER BY ").append(f.name());
    s += d->sortOrder == Qt::Ascending ? " ASC" : " DESC";
    return s;
}

/*!
    Returns the index of the field \a fieldName.
 */
int QSqlTableModel::fieldIndex(const QString &fieldName) const
{
    return d->rec.indexOf(fieldName);
}

QString QSqlTableModel::selectStatement() const
{
    QString query;
    if (d->tableName.isEmpty()) {
        d->error = QSqlError("No tablename given", QString(), QSqlError::Statement);
        return query;
    }

    QSqlRecord rec = d->db.record(d->tableName);
    if (d->rec.isEmpty()) {
        d->error = QSqlError("Unable to find table " + d->tableName, QString(),
                             QSqlError::Statement);
        return query;
    }
    query.append("SELECT ");
    query.append(d->rec.toString()).append(" FROM ").append(d->tableName).append(" ");
    if (!d->filter.isEmpty())
        query.append("WHERE ").append(d->filter).append(" ");
    query.append(orderByStatement());

    return query;
}

bool QSqlTableModel::removeColumn(int column, const QModelIndex &parent)
{
    if (parent.isValid() || column < 0 || column >= d->rec.count())
        return false;
    d->rec.remove(column);
    if (d->query.isActive())
        return select();
    return true;
}

bool QSqlTableModel::removeRow(int row, const QModelIndex &parent)
{
    // ### also handle manual update strategy...?
    if (row < 0 || row >= rowCount() || parent.isValid())
        return false;

    const QString stmt = deleteStatement();
    if (stmt.isEmpty())
        return false; //nothing to update

    if (d->editQuery.lastQuery() != stmt) {
        if (!d->editQuery.prepare(stmt)) {
            d->error = d->editQuery.lastError();
            return false;
        }
    }

    d->setPrimaryValues(row);
    if (!d->editQuery.exec()) {
        d->error = d->editQuery.lastError();
        return false;
    }

    return true;
}

QString QSqlTableModel::deleteStatement() const
{
    QString stmt;
    stmt.reserve(0xff);
    stmt.append("delete from ").append(d->tableName);
    stmt.append(d->whereClause());
    return stmt;
}

QString QSqlTableModel::insertStatement() const
{
    QString stmt;
    stmt.reserve(0xff);
    stmt.append("insert into ").append(d->tableName);
    stmt.append(" (").append(d->rec.toString());
    stmt.append(") values ( ");
    for (int i = 0; i < d->rec.count(); ++i)
        stmt.append("?,");
    stmt[stmt.length() - 1] = ')'; // chop off tailing comma
    return stmt;
}

/*!
    Inserts an empty row at position \a row. Note that \a parent has to be invalid, since
    this model does not support parent-child relations.

    Note that only one row can be inserted at a time, so \a count should always be 1.

    The primeInsert() signal will be emitted, so the newly inserted values can be initialized.

    Returns false if the parameters are out of bounds, otherwise true.

    \sa primeInsert()
 */
bool QSqlTableModel::insertRow(int row, const QModelIndex &parent, int count)
{
    if (count != 1 || row < 0 || row > rowCount() || parent.isValid())
        return false;

    d->insertIndex = row;
    // ### apply dangling changes... ?
    d->clearEditBuffer();
    emit primeInsert(row, d->editBuffer);
#define UGLY_WORKAROUND
#ifdef UGLY_WORKAROUND
    emit contentsRemoved(QModelIndex(row, 0), bottomRight());
    emit contentsInserted(QModelIndex(row, 0), QModelIndex(rowCount() + 1, columnCount() - 1));
#else
    // broken atm
    emit contentsInserted(QModelIndex(row, 0), QModelIndex(row, columnCount() - 1));
#endif
    return true;
}

/*! \reimp
 */
int QSqlTableModel::rowCount(const QModelIndex &parent) const
{
    int rc = QSqlModel::rowCount(parent);
    if (!parent.isValid() && d->insertIndex >= 0)
        ++rc;
    return rc;
}

/*! \reimp
 */
QModelIndex QSqlTableModel::dataIndex(const QModelIndex &it) const
{
    QModelIndex item = QSqlModel::dataIndex(it);
    if (d->insertIndex >= 0 && item.row() >= d->insertIndex)
        return QModelIndex(item.row() - 1, item.column(), item.data(), item.type());
    return item;
}

/*!
    Returns the currently set filter.

    \sa setFilter(), select()
 */
QString QSqlTableModel::filter() const
{
    return d->filter;
}

/*!
    Sets the current filter to \a filter. Note that no new records are selected. To select new
    records, use select(). The \a filter will apply to any subsequent select() calls.

    The filter is a SQL WHERE clause without the keyword 'WHERE', e.g. \c{name='Harry'} which will
    be processed by the DBMS.

    \sa filter(), select()
 */
void QSqlTableModel::setFilter(const QString &filter)
{
    d->filter = filter;
    if (d->query.isActive())
        select();
}


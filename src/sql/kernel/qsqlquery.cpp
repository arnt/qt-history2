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

#include "qsqlquery.h"

//#define QT_DEBUG_SQL

#include "qatomic.h"
#include "qsqlrecord.h"
#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldatabase.h"
#include "private/qsqlnulldriver_p.h"
#include "qvector.h"
#include "qmap.h"

class QSqlQueryPrivate
{
public:
    QSqlQueryPrivate(QSqlResult* result);
    ~QSqlQueryPrivate();
    QSqlResult* sqlResult;
    QAtomic ref;

    Q_GLOBAL_STATIC_WITH_ARGS(QSqlQueryPrivate, nullQueryPrivate, (0))
    Q_GLOBAL_STATIC(QSqlNullDriver, nullDriver)
    Q_GLOBAL_STATIC_WITH_ARGS(QSqlNullResult, nullResult, (nullDriver()))
    static QSqlQueryPrivate* shared_null();
};

QSqlQueryPrivate* QSqlQueryPrivate::shared_null()
{
    QSqlQueryPrivate *null = nullQueryPrivate();
    null->ref.ref();
    return null;
}

/*!
\internal
*/
QSqlQueryPrivate::QSqlQueryPrivate(QSqlResult* result): sqlResult(result)
{
    ref = 1;
    if (!sqlResult)
        sqlResult = nullResult();
}

QSqlQueryPrivate::~QSqlQueryPrivate()
{
    if (sqlResult == nullResult())
        return;
    delete sqlResult;
}

/*!
    \class QSqlQuery
    \brief The QSqlQuery class provides a means of executing and
    manipulating SQL statements.

    \ingroup database
    \mainclass
    \module sql

    QSqlQuery encapsulates the functionality involved in creating,
    navigating and retrieving data from SQL queries which are
    executed on a \l QSqlDatabase. It can be used to execute DML
    (data manipulation language) statements, such as \c SELECT, \c
    INSERT, \c UPDATE and \c DELETE, as well as DDL (data definition
    language) statements, such as \c{CREATE} \c{TABLE}. It can also
    be used to execute database-specific commands which are not
    standard SQL (e.g. \c{SET DATESTYLE=ISO} for PostgreSQL).

    Successfully executed SQL statements set the query's state to
    active; isActive() then returns true. Otherwise the query's state
    is set to inactive. In either case, when executing a new SQL
    statement, the query is positioned on an invalid record; an
    active query must be navigated to a valid record (so that
    isValid() returns true) before values can be retrieved.

    \target QSqlQuery examples

    Navigating records is performed with the following functions:

    \list
    \o next()
    \o previous()
    \o first()
    \o last()
    \o seek()
    \endlist

    These functions allow the programmer to move forward, backward or
    arbitrarily through the records returned by the query. If you only
    need to move forward through the results, e.g. using next() or
    using seek() with a positive offset, you can use setForwardOnly()
    and save a significant amount of memory overhead. Once an active
    query is positioned on a valid record, data can be retrieved using
    value(). All data is transferred from the SQL backend using
    QVariants.

    For example:

    \quotefromfile snippets/sqldatabase/sqldatabase.cpp
    \skipto typical loop
    \skipto QSqlQuery query
    \printuntil }

    To access the data returned by a query, use value(int). Each
    field in the data returned by a \c SELECT statement is accessed
    by passing the field's position in the statement, starting from
    0. This makes using \c{SELECT *} queries inadvisable because the
    order of the fields returned is indeterminate.

    For the sake of efficiency, there are no functions to access a
    field by name (unless you use prepared queries with names, as
    explained below). To convert a field name into an index, use
    record().\l{QSqlRecord::indexOf()}{indexOf()}, for example:

    \skipto field index lookup
    \skipto QSqlQuery query
    \printuntil }

    QSqlQuery supports prepared query execution and the binding of
    parameter values to placeholders. Some databases don't support
    these features, so for those, Qt emulates the required
    functionality. For example, the Oracle and ODBC drivers have
    proper prepared query support, and Qt makes use of it; but for
    databases that don't have this support, Qt implements the feature
    itself, e.g. by replacing placeholders with actual values when a
    query is executed. Use numRowsAffected() to find out how many rows
    were affected by a non-\c SELECT query, and size() to find how
    many were retrieved by a \c SELECT.

    Oracle databases identify placeholders by using a colon-name
    syntax, e.g \c{:name}. ODBC simply uses \c ? characters. Qt
    supports both syntaxes, with the restriction that you can't mix
    them in the same query.

    You can retrieve the values of all the fields in a single variable
    (a map) using boundValues().

    Below we present the same example using each of the four
    different binding approaches, as well as one example of binding
    values to a stored procedure.

    \bold{Named binding using named placeholders:}

    \skipto named with named
    \skipto QSqlQuery
    \printuntil exec()

    \bold{Positional binding using named placeholders:}

    \skipto positional with named
    \skipto QSqlQuery
    \printuntil exec()

    \bold{Binding values using positional placeholders (version 1):}

    \skipto positional 1
    \skipto QSqlQuery
    \printuntil exec()

    \bold{Binding values using positional placeholders (version 2):}

    \skipto positional 2
    \skipto QSqlQuery
    \printuntil exec()

    \bold{Binding values to a stored procedure:}

    This code calls a stored procedure called \c AsciiToInt(), passing
    it a character through its in parameter, and taking its result in
    the out parameter.

    \skipto stored
    \skipto QSqlQuery
    \printuntil boundValue(

    \sa QSqlDatabase, QSqlQueryModel, QSqlTableModel, QVariant
*/

/*!
    Constructs a QSqlQuery object which uses the QSqlResult \a result
    to communicate with a database.
*/

QSqlQuery::QSqlQuery(QSqlResult *result)
{
    d = new QSqlQueryPrivate(result);
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlQuery::~QSqlQuery()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    Constructs a copy of \a other.
*/

QSqlQuery::QSqlQuery(const QSqlQuery& other)
{
    d = other.d;
    d->ref.ref();
}

/*!
    \internal
*/
static void qInit(QSqlQuery *q, const QString& query, QSqlDatabase db)
{
    QSqlDatabase database = db;
    if (!database.isValid())
        database = QSqlDatabase::database(QLatin1String(QSqlDatabase::defaultConnection), false);
    if (database.isValid()) {
        *q = QSqlQuery(database.driver()->createResult());
    }
    if (!query.isEmpty())
        q->exec(query);
}

/*!
    Constructs a QSqlQuery object using the SQL \a query and the
    database \a db. If \a db is not specified, the application's
    default database is used. If \a query is not an empty string, it
    will be executed.

    \sa QSqlDatabase
*/
QSqlQuery::QSqlQuery(const QString& query, QSqlDatabase db)
{
    d = QSqlQueryPrivate::shared_null();
    qInit(this, query, db);
}

/*!
    Constructs a QSqlQuery object using the database \a db.

    \sa QSqlDatabase
*/

QSqlQuery::QSqlQuery(QSqlDatabase db)
{
    d = QSqlQueryPrivate::shared_null();
    qInit(this, QString(), db);
}


/*!
    Assigns \a other to this object.
*/

QSqlQuery& QSqlQuery::operator=(const QSqlQuery& other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Returns true if the query is active and positioned on a valid
    record and the \a field is NULL; otherwise returns false. Note
    that for some drivers, isNull() will not return accurate
    information until after an attempt is made to retrieve data.

    \sa isActive(), isValid(), value()
*/

bool QSqlQuery::isNull(int field) const
{
    if (d->sqlResult->isActive() && d->sqlResult->isValid())
        return d->sqlResult->isNull(field);
    return true;
}

/*!
    Executes the SQL in \a query. Returns true and sets the query
    state to active if the query was successful; otherwise returns
    false. The \a query string must use syntax appropriate for the
    SQL database being queried (for example, standard SQL).

    After the query is executed, the query is positioned on an \e
    invalid record and must be navigated to a valid record before
    data values can be retrieved (for example, using next()).

    Note that the last error for this query is reset when exec() is
    called.

    Example:

    \quotefromfile snippets/sqldatabase/sqldatabase.cpp
    \skipto QSqlQuery_snippets()
    \skipto typical loop
    \skipto QSqlQuery query
    \printuntil }

    \sa isActive() isValid() next() previous() first() last() seek()
*/

bool QSqlQuery::exec(const QString& query)
{
    if (d->ref != 1) {
        bool fo = isForwardOnly();
        *this = QSqlQuery(driver()->createResult());
        setForwardOnly(fo);
    } else {
        d->sqlResult->clear();
        d->sqlResult->setActive(false);
        d->sqlResult->setLastError(QSqlError());
        d->sqlResult->setAt(QSql::BeforeFirstRow);
    }
    d->sqlResult->setQuery(query.trimmed());
    if (!driver()->isOpen() || driver()->isOpenError()) {
        qWarning("QSqlQuery::exec: database not open");
        return false;
    }
    if (query.isEmpty()) {
        qWarning("QSqlQuery::exec: empty query");
        return false;
    }
#ifdef QT_DEBUG_SQL
    qDebug("\n QSqlQuery: %s", query.toLocal8Bit().constData());
#endif
    return d->sqlResult->reset(query);
}

/*!
    Returns the value of field \a index in the current record.

    The fields are numbered from left to right using the text of the
    \c SELECT statement, e.g. in

    \code
        SELECT forename, surname FROM people;
    \endcode

    field 0 is \c forename and field 1 is \c
    surname. Using \c{SELECT *} is not recommended because the order
    of the fields in the query is undefined.

    An invalid QVariant is returned if field \a index does not
    exist, if the query is inactive, or if the query is positioned on
    an invalid record.

    \sa previous() next() first() last() seek() isActive() isValid()
*/

QVariant QSqlQuery::value(int index) const
{
    if (isActive() && isValid() && (index > QSql::BeforeFirstRow))
        return d->sqlResult->data(index);
    qWarning("QSqlQuery::value: not positioned on a valid record");
    return QVariant();
}

/*!
    Returns the current internal position of the query. The first
    record is at position zero. If the position is invalid, the
    function returns QSql::BeforeFirstRow or
    QSql::AfterLastRow, which are special negative values.

    \sa previous() next() first() last() seek() isActive() isValid()
*/

int QSqlQuery::at() const
{
    return d->sqlResult->at();
}

/*!
    Returns the text of the current query being used, or an empty
    string if there is no current query text.

    \sa executedQuery()
*/

QString QSqlQuery::lastQuery() const
{
    return d->sqlResult->lastQuery();
}

/*!
    Returns the database driver associated with the query.
*/

const QSqlDriver *QSqlQuery::driver() const
{
    return d->sqlResult->driver();
}

/*!
    Returns the result associated with the query.
*/

const QSqlResult* QSqlQuery::result() const
{
    return d->sqlResult;
}

/*!
    Retrieves the record at position \a index, if available, and
    positions the query on the retrieved record. The first record is
    at position 0. Note that the query must be in an active state and
    isSelect() must return true before calling this function.

    If \a relative is false (the default), the following rules apply:

    \list
    \o If \a index is negative, the result is positioned before the
    first record and false is returned.
    \o Otherwise, an attempt is made to move to the record at position
    \a index. If the record at position \a index could not be retrieved, the
    result is positioned after the last record and false is returned. If
    the record is successfully retrieved, true is returned.
    \endlist

    If \a relative is true, the following rules apply:

    \list
    \o If the result is currently positioned before the first
    record or on the first record, and \a index is negative, there is no
    change, and false is returned.
    \o If the result is currently located after the last record, and
    \a index is positive, there is no change, and false is returned.
    \o If the result is currently located somewhere in the middle,
    and the relative offset \a index moves the result below zero, the
    result is positioned before the first record and false is
    returned.
    \o Otherwise, an attempt is made to move to the record \a index
    records ahead of the current record (or \a index records behind the
    current record if \a index is negative). If the record at offset \a index
    could not be retrieved, the result is positioned after the last
    record if \a index >= 0, (or before the first record if \a index is
    negative), and false is returned. If the record is successfully
    retrieved, true is returned.
    \endlist

    \sa next() previous() first() last() at() isActive() isValid()
*/
bool QSqlQuery::seek(int index, bool relative)
{
    if (!isSelect() || !isActive())
        return false;
    int actualIdx;
    if (!relative) { // arbitrary seek
        if (index < 0) {
            d->sqlResult->setAt(QSql::BeforeFirstRow);
            return false;
        }
        actualIdx = index;
    } else {
        switch (at()) { // relative seek
        case QSql::BeforeFirstRow:
            if (index > 0)
                actualIdx = index;
            else {
                return false;
            }
            break;
        case QSql::AfterLastRow:
            if (index < 0) {
                d->sqlResult->fetchLast();
                actualIdx = at() + index;
            } else {
                return false;
            }
            break;
        default:
            if ((at() + index) < 0) {
                d->sqlResult->setAt(QSql::BeforeFirstRow);
                return false;
            }
            actualIdx = at() + index;
            break;
        }
    }
    // let drivers optimize
    if (isForwardOnly() && actualIdx < at()) {
        qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
        return false;
    }
    if (actualIdx == (at() + 1) && at() != QSql::BeforeFirstRow) {
        if (!d->sqlResult->fetchNext()) {
            d->sqlResult->setAt(QSql::AfterLastRow);
            return false;
        }
        return true;
    }
    if (actualIdx == (at() - 1)) {
        if (!d->sqlResult->fetchPrevious()) {
            d->sqlResult->setAt(QSql::BeforeFirstRow);
            return false;
        }
        return true;
    }
    if (!d->sqlResult->fetch(actualIdx)) {
        d->sqlResult->setAt(QSql::AfterLastRow);
        return false;
    }
    return true;
}

/*!
    Retrieves the next record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return true before
    calling this function or it will do nothing and return false.

    The following rules apply:

    \list
    \o If the result is currently located before the first
    record, e.g. immediately after a query is executed, an attempt is
    made to retrieve the first record.

    \o If the result is currently located after the last record,
    there is no change and false is returned.

    \o If the result is located somewhere in the middle, an attempt
    is made to retrieve the next record.
    \endlist

    If the record could not be retrieved, the result is positioned after
    the last record and false is returned. If the record is successfully
    retrieved, true is returned.

    \sa previous() first() last() seek() at() isActive() isValid()
*/

bool QSqlQuery::next()
{
    if (!isSelect() || !isActive())
        return false;
    bool b = false;
    switch (at()) {
    case QSql::BeforeFirstRow:
        b = d->sqlResult->fetchFirst();
        return b;
    case QSql::AfterLastRow:
        return false;
    default:
        if (!d->sqlResult->fetchNext()) {
            d->sqlResult->setAt(QSql::AfterLastRow);
            return false;
        }
        return true;
    }
}

/*!
    Retrieves the previous record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return true before
    calling this function or it will do nothing and return false.

    The following rules apply:

    \list
    \o If the result is currently located before the first record,
    there is no change and false is returned.

    \o If the result is currently located after the last record, an
    attempt is made to retrieve the last record.

    \o If the result is somewhere in the middle, an attempt is made
    to retrieve the previous record.
    \endlist

    If the record could not be retrieved, the result is positioned
    before the first record and false is returned. If the record is
    successfully retrieved, true is returned.

    \sa next() first() last() seek() at() isActive() isValid()
*/

bool QSqlQuery::previous()
{
    if (!isSelect() || !isActive())
        return false;
    if (isForwardOnly()) {
        qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
        return false;
    }

    bool b = false;
    switch (at()) {
    case QSql::BeforeFirstRow:
        return false;
    case QSql::AfterLastRow:
        b = d->sqlResult->fetchLast();
        return b;
    default:
        if (!d->sqlResult->fetchPrevious()) {
            d->sqlResult->setAt(QSql::BeforeFirstRow);
            return false;
        }
        return true;
    }
}

/*!
    Retrieves the first record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return true before
    calling this function or it will do nothing and return false.
    Returns true if successful. If unsuccessful the query position is
    set to an invalid position and false is returned.

    \sa next() previous() last() seek() at() isActive() isValid()
*/

bool QSqlQuery::first()
{
    if (!isSelect() || !isActive())
        return false;
    if (isForwardOnly() && at() > QSql::BeforeFirstRow) {
        qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
        return false;
    }
    bool b = false;
    b = d->sqlResult->fetchFirst();
    return b;
}

/*!
    Retrieves the last record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return true before
    calling this function or it will do nothing and return false.
    Returns true if successful. If unsuccessful the query position is
    set to an invalid position and false is returned.

    \sa next() previous() first() seek() at() isActive() isValid()
*/

bool QSqlQuery::last()
{
    if (!isSelect() || !isActive())
        return false;
    bool b = false;
    b = d->sqlResult->fetchLast();
    return b;
}

/*!
    Returns the size of the result (number of rows returned), or -1
    if the size cannot be determined or if the database does not
    support reporting information about query sizes. Note that for
    non-\c SELECT statements (isSelect() returns false), size() will
    return -1. If the query is not active (isActive() returns false),
    -1 is returned.

    To determine the number of rows affected by a non-\c SELECT
    statement, use numRowsAffected().

    \sa isActive() numRowsAffected() QSqlDriver::hasFeature()
*/
int QSqlQuery::size() const
{
    if (isActive() && d->sqlResult->driver()->hasFeature(QSqlDriver::QuerySize))
        return d->sqlResult->size();
    return -1;
}

/*!
    Returns the number of rows affected by the result's SQL statement,
    or -1 if it cannot be determined. Note that for \c SELECT
    statements, the value is undefined; use size() instead. If the
    query is not active (isActive() returns false), -1 is returned.

    \sa size() QSqlDriver::hasFeature()
*/

int QSqlQuery::numRowsAffected() const
{
    if (isActive())
        return d->sqlResult->numRowsAffected();
    return -1;
}

/*!
    Returns error information about the last error (if any) that
    occurred.

    \sa QSqlError
*/

QSqlError QSqlQuery::lastError() const
{
    return d->sqlResult->lastError();
}

/*!
    Returns true if the query is currently positioned on a valid
    record; otherwise returns false.
*/

bool QSqlQuery::isValid() const
{
    return d->sqlResult->isValid();
}

/*!
    Returns true if the query is currently active; otherwise returns
    false.
*/

bool QSqlQuery::isActive() const
{
    return d->sqlResult->isActive();
}

/*!
    Returns true if the current query is a \c SELECT statement;
    otherwise returns false.
*/

bool QSqlQuery::isSelect() const
{
    return d->sqlResult->isSelect();
}

/*!
    Returns true if you can only scroll forward through a result set;
    otherwise returns false.

    \sa setForwardOnly(), next()
*/
bool QSqlQuery::isForwardOnly() const
{
    return d->sqlResult->isForwardOnly();
}

/*!
    Sets forward only mode to \a forward. If \a forward is true, only
    next() and seek() with positive values, are allowed for
    navigating the results. Forward only mode needs far less memory
    since results do not need to be cached.

    Forward only mode is off by default.

    \sa isForwardOnly(), next(), seek()
*/
void QSqlQuery::setForwardOnly(bool forward)
{
    d->sqlResult->setForwardOnly(forward);
}

/*!
    Returns a QSqlRecord containing the field information for the
    current query. If the query points to a valid row (isValid()
    returns true), the record is populated with the row's values.
    An empty record is returned when there is no active query
    (isActive() returns false).

    To retrieve values from a query, value() should be used since
    its index-based lookup is faster.

    In the following example, a \c{SELECT * FROM} query is executed.
    Since the order of the columns is not defined, QSqlRecord::indexOf()
    is used to obtain the index of a column.

    \code
    QSqlQuery q("select * from employees");
    QSqlRecord rec = q.record();

    qDebug() << "Number of columns: " << rec.count();

    int nameCol = rec.indexOf("name"); // index of the field "name"
    while (q.next())
        qDebug() << q.value(nameCol).toString(); // output all names
    \endcode

    \sa value()
*/
QSqlRecord QSqlQuery::record() const
{
    QSqlRecord rec = d->sqlResult->record();

    if (isValid()) {
        for (int i = 0; i < rec.count(); ++i)
            rec.setValue(i, value(i));
    }
    return rec;
}

/*!
    Clears the result set and releases any resources held by the
    query. You should rarely if ever need to call this function.
*/
void QSqlQuery::clear()
{
    *this = QSqlQuery(driver()->createResult());
}

/*!
    Prepares the SQL query \a query for execution. The query may
    contain placeholders for binding values. Both Oracle style
    colon-name (e.g., \c{:surname}), and ODBC style (\c{?})
    placeholders are supported; but they cannot be mixed in the same
    query. See the \l{QSqlQuery examples}{Detailed Description} for examples.

    \sa exec(), bindValue(), addBindValue()
*/
bool QSqlQuery::prepare(const QString& query)
{
    if (d->ref != 1) {
        bool fo = isForwardOnly();
        *this = QSqlQuery(driver()->createResult());
        setForwardOnly(fo);
    } else {
        d->sqlResult->setActive(false);
        d->sqlResult->setLastError(QSqlError());
        d->sqlResult->setAt(QSql::BeforeFirstRow);
    }
    if (!driver()) {
        qWarning("QSqlQuery::prepare: no driver");
        return false;
    }
    if (!driver()->isOpen() || driver()->isOpenError()) {
        qWarning("QSqlQuery::prepare: database not open");
        return false;
    }
    if (query.isEmpty()) {
        qWarning("QSqlQuery::prepare: empty query");
        return false;
    }
#ifdef QT_DEBUG_SQL
    qDebug("\n QSqlQuery::prepare: %s", query.toLocal8Bit().constData());
#endif
    return d->sqlResult->savePrepare(query);
}

/*!
    \overload

    Executes a previously prepared SQL query. Returns true if the
    query executed successfully; otherwise returns false.

    \sa prepare() bindValue() addBindValue() boundValue() boundValues()
*/
bool QSqlQuery::exec()
{
    d->sqlResult->resetBindCount();
    return d->sqlResult->exec();
}

/*!
    Set the placeholder \a placeholder to be bound to value \a val in
    the prepared statement. Note that the placeholder mark (e.g \c{:})
    must be included when specifying the placeholder name. If \a paramType
    is \c QSql::Out or \c QSql::InOut, the placeholder will be
    overwritten with data from the database after the exec() call.

    \sa addBindValue(), prepare(), exec(), boundValue() boundValues()
*/
void QSqlQuery::bindValue(const QString& placeholder, const QVariant& val,
                          QSql::ParamType paramType
)
{
    d->sqlResult->bindValue(placeholder, val, paramType);
}

/*!
    \overload

    Set the placeholder in position \a pos to be bound to value \a val
    in the prepared statement. Field numbering starts at 0. If \a paramType
    is \c QSql::Out or \c QSql::InOut, the placeholder will be
    overwritten with data from the database after the exec() call.
*/
void QSqlQuery::bindValue(int pos, const QVariant& val, QSql::ParamType paramType)
{
    d->sqlResult->bindValue(pos, val, paramType);
}

/*!
    Adds the value \a val to the list of values when using positional
    value binding. The order of the addBindValue() calls determines
    which placeholder a value will be bound to in the prepared query.
    If \a paramType is \c QSql::Out or \c QSql::InOut, the placeholder will
    be overwritten with data from the database after the exec() call.

    \sa bindValue(), prepare(), exec(), boundValue() boundValues()
*/
void QSqlQuery::addBindValue(const QVariant& val, QSql::ParamType paramType)
{
    d->sqlResult->addBindValue(val, paramType);
}

/*!
    Returns the value for the \a placeholder.

    \sa boundValues() bindValue() addBindValue()
*/
QVariant QSqlQuery::boundValue(const QString& placeholder) const
{
    return d->sqlResult->boundValue(placeholder);
}

/*!
    \overload

    Returns the value for the placeholder at position \a pos.
*/
QVariant QSqlQuery::boundValue(int pos) const
{
    return d->sqlResult->boundValue(pos);
}

/*!
    Returns a map of the bound values.

    With named binding, the bound values can be examined in the
    following ways:

    \quotefromfile snippets/sqldatabase/sqldatabase.cpp
    \skipto examine with named binding
    \skipto QMapIterator
    \printuntil }

    With positional binding, the code becomes:

    \skipto examine with positional binding
    \skipto QList
    \printuntil endl;

    \sa boundValue() bindValue() addBindValue()
*/
QMap<QString,QVariant> QSqlQuery::boundValues() const
{
    QMap<QString,QVariant> map;

    const QVector<QVariant> values(d->sqlResult->boundValues());
    for (int i = 0; i < values.count(); ++i)
        map[d->sqlResult->boundValueName(i)] = values.at(i);
    return map;
}

/*!
    Returns the last query that was executed.

    In most cases this function returns the same string as
    lastQuery(). If a prepared query with placeholders is executed on
    a DBMS that does not support it, the preparation of this query is
    emulated. The placeholders in the original query are replaced with
    their bound values to form a new query. This function returns the
    modified query. It is mostly useful for debugging purposes.

    \sa lastQuery()
*/
QString QSqlQuery::executedQuery() const
{
    return d->sqlResult->executedQuery();
}

/*!
    \fn bool QSqlQuery::prev()

    Use previous() instead.
*/

/*!
    Returns the object ID of the most recent inserted row if the
    database supports it.
    An invalid QVariant will be returned if the query did not
    insert any value or if the database does not report the id back.
    If more than one row was touched by the insert, the behavior is
    undefined.

    \sa QSqlDriver::hasFeature()
*/
QVariant QSqlQuery::lastInsertId() const
{
    return d->sqlResult->lastInsertId();
}


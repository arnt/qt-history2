/****************************************************************************
**
** Implementation of QSqlQuery class.
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

#include "qsqlquery.h"

#ifndef QT_NO_SQL

//#define QT_DEBUG_SQL

#include "qatomic.h"
#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldatabase.h"
#include "qsql.h"
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

    static QSqlResult* nullResult();
    static QSqlQueryPrivate* shared_null();
};

QSqlQueryPrivate* QSqlQueryPrivate::shared_null()
{
    static QSqlQueryPrivate null(0);
    ++null.ref;
    return &null;
}

QSqlResult *QSqlQueryPrivate::nullResult()
{
    static QSqlNullDriver nDriver;
    static QSqlNullResult nResult(&nDriver);
    return &nResult;
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
    \class QSqlQuery qsqlquery.h
    \brief The QSqlQuery class provides a means of executing and
    manipulating SQL statements.

    \ingroup database
    \mainclass
    \module sql

    QSqlQuery encapsulates the functionality involved in creating,
    navigating and retrieving data from SQL queries which are executed
    on a \l QSqlDatabase. It can be used to execute DML (data
    manipulation language) statements, e.g. \c SELECT, \c INSERT, \c
    UPDATE and \c DELETE, and also DDL (data definition language)
    statements, e.g. \c{CREATE} \c{TABLE}. It can also be used to
    execute database-specific commands which are not standard SQL
    (e.g. \c{SET DATESTYLE=ISO} for PostgreSQL).

    Successfully executed SQL statements set the query's state to
    active (isActive() returns true); otherwise the query's state is
    set to inactive. In either case, when executing a new SQL
    statement, the query is positioned on an invalid record; an active
    query must be navigated to a valid record (so that isValid()
    returns true) before values can be retrieved.

    Navigating records is performed with the following functions:

    \list
    \i \c next()
    \i \c previous()
    \i \c first()
    \i \c last()
    \i \c \link QSqlQuery::seek() seek\endlink(int)
    \endlist

    These functions allow the programmer to move forward, backward or
    arbitrarily through the records returned by the query. If you only
    need to move forward through the results, e.g. using next() or
    using seek() with a positive offset, you can use setForwardOnly()
    and save a significant amount of memory overhead. Once an active
    query is positioned on a valid record, data can be retrieved using
    value(). All data is transferred from the SQL backend using
    QCoreVariants.

    For example:

    \code
    QSqlQuery query("SELECT name FROM customer");
    while (query.next()) {
        QString name = query.value(0).toString();
        doSomething(name);
    }
    \endcode

    To access the data returned by a query, use the value() method.
    Each field in the data returned by a \c SELECT statement is
    accessed by passing the field's position in the statement,
    starting from 0. This makes using \c{SELECT *} queries inadvisable
    because the order of the fields returned is indeterminate. For the
    sake of efficiency there are no methods to access a field by name,
    unless you use prepared queries with names.

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
    supports both syntaxes (although you can't mix them in the same
    query).

    You can retrieve the values of all the fields in a single variable
    (a map) using boundValues().

    Below we present the same example using each of the four different
    binding approaches.

    <b>Named binding using named placeholders</b>
    \code
    QSqlQuery query;
    query.prepare("INSERT INTO atable (id, forename, surname) "
                   "VALUES (:id, :forename, :surname)");
    query.bindValue(":id", 1001);
    query.bindValue(":forename", "Bart");
    query.bindValue(":surname", "Simpson");
    query.exec();
    \endcode

    <b>Positional binding using named placeholders</b>
    \code
    QSqlQuery query;
    query.prepare("INSERT INTO atable (id, forename, surname) "
                   "VALUES (:id, :forename, :surname)");
    query.bindValue(0, 1001);
    query.bindValue(1, "Bart");
    query.bindValue(2, "Simpson");
    query.exec();
    \endcode

    <b>Binding values using positional placeholders #1</b>
    \code
    QSqlQuery query;
    query.prepare("INSERT INTO atable (id, forename, surname) "
                   "VALUES (?, ?, ?)");
    query.bindValue(0, 1001);
    query.bindValue(1, "Bart");
    query.bindValue(2, "Simpson");
    query.exec();
    \endcode

    <b>Binding values using positional placeholders #2</b>
    \code
    query.prepare("INSERT INTO atable (id, forename, surname) "
                   "VALUES (?, ?, ?)");
    query.addBindValue(1001);
    query.addBindValue("Bart");
    query.addBindValue("Simpson");
    query.exec();
    \endcode

    <b>Binding values to a stored procedure</b>
    This code calls a stored procedure called \c AsciiToInt(), passing
    it a character through its in parameter, and taking its result in
    the out parameter.
    \code
    QSqlQuery query;
    query.prepare("call AsciiToInt(?, ?)");
    query.bindValue(0, "A");
    query.bindValue(1, 0, QSql::Out);
    query.exec();
    int i = query.boundValue(1).toInt(); // i is 65.
    \endcode

    \sa QSqlDatabase QSqlCursor QCoreVariant
*/

/*!
    Creates a QSqlQuery object which uses the QSqlResult \a r to
    communicate with a database.
*/

QSqlQuery::QSqlQuery(QSqlResult * r)
{
    d = new QSqlQueryPrivate(r);
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlQuery::~QSqlQuery()
{
    if (!--d->ref)
        delete d;
}

/*!
    Constructs a copy of \a other.
*/

QSqlQuery::QSqlQuery(const QSqlQuery& other)
{
    d = other.d;
    ++d->ref;
}

/*!
    Creates a QSqlQuery object using the SQL \a query and the database
    \a db. If \a db is 0, (the default), the application's default
    database is used. If \a query is not a null string, it will be
    executed.

    \sa QSqlDatabase
*/
QSqlQuery::QSqlQuery(const QString& query, QSqlDatabase db)
{
    init(query, db);
}

/*! \overload

    Convenience function that takes a latin1 \a query. For
    queries with non-latin1 characters, the QString overload
    must be used. Uses the database \a db or the default
    database if \a db is not valid.
 */
QSqlQuery::QSqlQuery(const char *query, QSqlDatabase db)
{
    init(QString::fromLatin1(query), db);
}


/*!
    Creates a QSqlQuery object using the database \a db. If \a db is
    0, the application's default database is used.

    \sa QSqlDatabase
*/

QSqlQuery::QSqlQuery(QSqlDatabase db)
{
    init(QString(), db);
}

/*! \internal
*/

void QSqlQuery::init(const QString& query, QSqlDatabase db)
{
    d = QSqlQueryPrivate::shared_null();
    QSqlDatabase database = db;
    if (!database.isValid())
        database = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
    if (database.isValid())
        *this = database.driver()->createQuery();
    if (!query.isEmpty())
        exec(query);
}

/*!
    Assigns \a other to the query.
*/

QSqlQuery& QSqlQuery::operator=(const QSqlQuery& other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Returns true if the query is active and positioned on a valid
    record and the \a field is NULL; otherwise returns false. Note
    that for some drivers isNull() will not return accurate
    information until after an attempt is made to retrieve data.

    \sa isActive() isValid() value()
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
    false and sets the query state to inactive. The \a query string
    must use syntax appropriate for the SQL database being queried,
    for example, standard SQL.

    After the query is executed, the query is positioned on an \e
    invalid record, and must be navigated to a valid record before
    data values can be retrieved, e.g. using next().

    Note that the last error for this query is reset when exec() is
    called.

    \sa isActive() isValid() next() previous() first() last() seek()
*/

bool QSqlQuery::exec(const QString& query)
{
    if (d->ref != 1) {
        *this = driver()->createQuery();
    } else {
        d->sqlResult->clear();
        d->sqlResult->setActive(false);
        d->sqlResult->setLastError(QSqlError());
        d->sqlResult->setAt(QSql::BeforeFirst);
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
    qDebug("\n QSqlQuery: %s", query.ascii());
#endif
    return d->sqlResult->reset(query);
}

/*!
    Returns the value of the \a{i}-th field in the query (zero based).

    The fields are numbered from left to right using the text of the
    \c SELECT statement, e.g. in \c{SELECT} \c{forename,} \c{surname}
    \c{FROM} \c{people}, field 0 is \c forename and field 1 is \c
    surname. Using \c{SELECT *} is not recommended because the order
    of the fields in the query is undefined.

    An invalid QCoreVariant is returned if field \a i does not exist, if
    the query is inactive, or if the query is positioned on an invalid
    record.

    \sa previous() next() first() last() seek() isActive() isValid()
*/

QCoreVariant QSqlQuery::value(int i) const
{
    if (isActive() && isValid() && (i > QSql::BeforeFirst))
        return d->sqlResult->data(i);
    qWarning("QSqlQuery::value: not positioned on a valid record");
    return QCoreVariant();
}

/*!
    Returns the current internal position of the query. The first
    record is at position zero. If the position is invalid, a
    QSql::Location will be returned indicating the invalid position.

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

const QSqlDriver* QSqlQuery::driver() const
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
    Retrieves the record at position (offset) \a i, if available, and
    positions the query on the retrieved record. The first record is
    at position 0. Note that the query must be in an active state and
    isSelect() must return true before calling this function.

    If \a relative is false (the default), the following rules apply:

    \list
    \i If \a i is negative, the result is positioned before the
    first record and false is returned.
    \i Otherwise, an attempt is made to move to the record at position
    \a i. If the record at position \a i could not be retrieved, the
    result is positioned after the last record and false is returned. If
    the record is successfully retrieved, true is returned.
    \endlist

    If \a relative is true, the following rules apply:

    \list
    \i If the result is currently positioned before the first
    record or on the first record, and \a i is negative, there is no
    change, and false is returned.
    \i If the result is currently located after the last record, and
    \a i is positive, there is no change, and false is returned.
    \i If the result is currently located somewhere in the middle,
    and the relative offset \a i moves the result below zero, the
    result is positioned before the first record and false is
    returned.
    \i Otherwise, an attempt is made to move to the record \a i
    records ahead of the current record (or \a i records behind the
    current record if \a i is negative). If the record at offset \a i
    could not be retrieved, the result is positioned after the last
    record if \a i >= 0, (or before the first record if \a i is
    negative), and false is returned. If the record is successfully
    retrieved, true is returned.
    \endlist

    \sa next() previous() first() last() at() isActive() isValid()
*/
bool QSqlQuery::seek(int i, bool relative)
{
    if (!isSelect() || !isActive())
        return false;
    beforeSeek();
    int actualIdx;
    if (!relative) { // arbitrary seek
        if (i < 0) {
            d->sqlResult->setAt(QSql::BeforeFirst);
            afterSeek();
            return false;
        }
        actualIdx = i;
    } else {
        switch (at()) { // relative seek
        case QSql::BeforeFirst:
            if (i > 0)
                actualIdx = i;
            else {
                afterSeek();
                return false;
            }
            break;
        case QSql::AfterLast:
            if (i < 0) {
                d->sqlResult->fetchLast();
                actualIdx = at() + i;
            } else {
                afterSeek();
                return false;
            }
            break;
        default:
            if ((at() + i) < 0) {
                d->sqlResult->setAt(QSql::BeforeFirst);
                afterSeek();
                return false;
            }
            actualIdx = at() + i;
            break;
        }
    }
    // let drivers optimize
    if (isForwardOnly() && actualIdx < at()) {
        qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
        afterSeek();
        return false;
    }
    if (actualIdx == (at() + 1) && at() != QSql::BeforeFirst) {
        if (!d->sqlResult->fetchNext()) {
            d->sqlResult->setAt(QSql::AfterLast);
            afterSeek();
            return false;
        }
        afterSeek();
        return true;
    }
    if (actualIdx == (at() - 1)) {
        if (!d->sqlResult->fetchPrevious()) {
            d->sqlResult->setAt(QSql::BeforeFirst);
            afterSeek();
            return false;
        }
        afterSeek();
        return true;
    }
    if (!d->sqlResult->fetch(actualIdx)) {
        d->sqlResult->setAt(QSql::AfterLast);
        afterSeek();
        return false;
    }
    afterSeek();
    return true;
}

/*!
    Retrieves the next record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return true before
    calling this function or it will do nothing and return false.

    The following rules apply:

    \list
    \i If the result is currently located before the first
    record, e.g. immediately after a query is executed, an attempt is
    made to retrieve the first record.

    \i If the result is currently located after the last record,
    there is no change and false is returned.

    \i If the result is located somewhere in the middle, an attempt
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
    beforeSeek();
    bool b = false;
    switch (at()) {
    case QSql::BeforeFirst:
        b = d->sqlResult->fetchFirst();
        afterSeek();
        return b;
    case QSql::AfterLast:
        afterSeek();
        return false;
    default:
        if (!d->sqlResult->fetchNext()) {
            d->sqlResult->setAt(QSql::AfterLast);
            afterSeek();
            return false;
        }
        afterSeek();
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
    \i If the result is currently located before the first record,
    there is no change and false is returned.

    \i If the result is currently located after the last record, an
    attempt is made to retrieve the last record.

    \i If the result is somewhere in the middle, an attempt is made
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

    beforeSeek();
    bool b = false;
    switch (at()) {
    case QSql::BeforeFirst:
        afterSeek();
        return false;
    case QSql::AfterLast:
        b = d->sqlResult->fetchLast();
        afterSeek();
        return b;
    default:
        if (!d->sqlResult->fetchPrevious()) {
            d->sqlResult->setAt(QSql::BeforeFirst);
            afterSeek();
            return false;
        }
        afterSeek();
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
    if (isForwardOnly() && at() > QSql::BeforeFirst) {
        qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
        return false;
    }
    beforeSeek();
    bool b = false;
    b = d->sqlResult->fetchFirst();
    afterSeek();
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
    beforeSeek();
    bool b = false;
    b = d->sqlResult->fetchLast();
    afterSeek();
    return b;
}

/*!
    Returns the size of the result, (number of rows returned), or -1
    if the size cannot be determined or if the database does not
    support reporting information about query sizes. Note that for
    non-\c SELECT statements (isSelect() returns false), size() will
    return -1. If the query is not active (isActive() returns false),
    -1 is returned.

    To determine the number of rows affected by a non-SELECT
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
    Returns true if you can only scroll \e forward through a result
    set; otherwise returns false.

    \sa setForwardOnly()
*/
bool QSqlQuery::isForwardOnly() const
{
    return d->sqlResult->isForwardOnly();
}

/*!
    Sets forward only mode to \a forward. If forward is true only
    next(), and seek() with positive values, are allowed for
    navigating the results. Forward only mode needs far less memory
    since results do not need to be cached.

    Forward only mode is off by default.

    Forward only mode cannot be used with data aware widgets or with
    any of the model/view classes, since they must to be able to
    scroll backward as well as forward.

    \sa isForwardOnly(), next(), seek()
*/
void QSqlQuery::setForwardOnly(bool forward)
{
    d->sqlResult->setForwardOnly(forward);
}

/*!
    Returns a QSqlRecord containing the data for the query's current
    record; the query must be active (isActive() returns true), and there
    must be a current record, i.e. a navigation function (e.g. next())
    must have been called and isValid() must return true. Normally
    data is retrieved field-by-field using value() or boundValue().
*/
QSqlRecord QSqlQuery::record() const
{
    return d->sqlResult->record();
}

/*!
    Protected virtual function called before the internal record
    pointer is moved to a new record. The default implementation does
    nothing.
*/

void QSqlQuery::beforeSeek()
{

}


/*!
    Protected virtual function called after the internal record
    pointer is moved to a new record. The default implementation does
    nothing.
*/

void QSqlQuery::afterSeek()
{

}


/*!
    Clears the result set and releases any resources held by the
    query. You should rarely if ever need to call this function.
*/
void QSqlQuery::clear()
{
    *this = driver()->createQuery();
}

/*!
    Prepares the SQL query \a query for execution. The query may
    contain placeholders for binding values. Both Oracle style
    colon-name (e.g. \c{:surname}), and ODBC style (e.g. \c{?})
    placeholders are supported; but they cannot be mixed in the same
    query. See the \link #details Description\endlink for examples.

    \sa exec(), bindValue(), addBindValue()
*/
bool QSqlQuery::prepare(const QString& query)
{
    if (d->ref != 1) {
        *this = driver()->createQuery();
    } else {
        d->sqlResult->setActive(false);
        d->sqlResult->setLastError(QSqlError());
        d->sqlResult->setAt(QSql::BeforeFirst);
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
    qDebug("\n QSqlQuery::prepare: %s", query.ascii());
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
    must be included when specifying the placeholder name. If \a type
    is \c QSql::Out or \c QSql::InOut, the placeholder will be
    overwritten with data from the database after the exec() call.

    \sa addBindValue(), prepare(), exec(), boundValue() boundValues()
*/
void QSqlQuery::bindValue(const QString& placeholder, const QCoreVariant& val, QSql::ParamType type
)
{
    d->sqlResult->bindValue(placeholder, val, type);
}

/*!
    \overload

    Set the placeholder in position \a pos to be bound to value \a val
    in the prepared statement. Field numbering starts at 0. If \a type
    is \c QSql::Out or \c QSql::InOut, the placeholder will be
    overwritten with data from the database after the exec() call.
*/
void QSqlQuery::bindValue(int pos, const QCoreVariant& val, QSql::ParamType type)
{
    d->sqlResult->bindValue(pos, val, type);
}

/*!
    Adds the value \a val to the list of values when using positional
    value binding. The order of the addBindValue() calls determines
    which placeholder a value will be bound to in the prepared query.
    If \a type is \c QSql::Out or \c QSql::InOut, the placeholder will
    be overwritten with data from the database after the exec() call.

    \sa bindValue(), prepare(), exec(), boundValue() boundValues()
*/
void QSqlQuery::addBindValue(const QCoreVariant& val, QSql::ParamType type)
{
    d->sqlResult->addBindValue(val, type);
}

/*!
    Returns the value for the \a placeholder.

    \sa boundValues() bindValue() addBindValue()
*/
QCoreVariant QSqlQuery::boundValue(const QString& placeholder) const
{
    return d->sqlResult->boundValue(placeholder);
}

/*!
    \overload

    Returns the value for the placeholder at position \a pos.
*/
QCoreVariant QSqlQuery::boundValue(int pos) const
{
    return d->sqlResult->boundValue(pos);
}

/*!
    Returns a map of the bound values.

    The bound values can be examined in the following ways:
    \code
    QSqlQuery query;
    ...
    // Examine the bound values - bound using named binding
    QMap<QString, QCoreVariant>::ConstIterator i;
    QMap<QString, QCoreVariant> vals = query.boundValues();
    for (i = vals.constBegin(); i != vals.constEnd(); ++it)
        qWarning("Placeholder: " + i.key() + ", Value: " + (*i).toString());
    ...

    // Examine the bound values - bound using positional binding
    QList<QCoreVariant>::ConstIterator i;
    QList<QCoreVariant> list = query.boundValues().values();
    int j = 0;
    for (i = list.constBegin(); i != list.constEnd(); ++i)
        qWarning("Placeholder position: %d, Value: " + (*i).toString(), j++);
    ...

    \endcode

    \sa boundValue() bindValue() addBindValue()
*/
QMap<QString,QCoreVariant> QSqlQuery::boundValues() const
{
    QMap<QString,QCoreVariant> map;

    const QVector<QCoreVariant> values(d->sqlResult->boundValues());
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
#endif // QT_NO_SQL

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

#include "qcorevariant.h"
#include "qmap.h"
#include "qregexp.h"
#include "qsqlerror.h"
#include "qsqlfield.h"
#include "qsqlrecord.h"
#include "qsqlresult.h"
#include "qvector.h"
#include "qsqldriver.h"

struct Holder {
    Holder(const QString& hldr = QString(), int index = -1): holderName(hldr), holderPos(index) {}
    bool operator==(const Holder& h) const { return h.holderPos == holderPos && h.holderName == holderName; }
    bool operator!=(const Holder& h) const { return h.holderPos != holderPos || h.holderName != holderName; }
    QString holderName;
    int            holderPos;
};

class QSqlResultPrivate
{
public:
    QSqlResultPrivate(QSqlResult* d)
    : q(d), sqldriver(0), idx(QSql::BeforeFirstRow), active(false),
      isSel(false), forwardOnly(false), bindCount(0), binds(QSqlResult::PositionalBinding)
    {}

    void clearValues()
    {
        values.clear();
        bindCount = 0;
    }

    void resetBindCount()
    {
        bindCount = 0;
    }

    void clearIndex()
    {
        indexes.clear();
        holders.clear();
        types.clear();
    }

    void clear()
    {
        clearValues();
        clearIndex();;
    }

    void positionalToNamedBinding();
    void namedToPositionalBinding();
    QString holderAt(int index) const;

public:
    QSqlResult* q;
    const QSqlDriver* sqldriver;
    int idx;
    QString sql;
    bool active;
    bool isSel;
    QSqlError error;
    bool forwardOnly;

    int bindCount;
    QSqlResult::BindingSyntax binds;

    QString executedQuery;
    QMap<int, QSql::ParamType> types;
    QVector<QCoreVariant> values;
    typedef QMap<QString, int> IndexMap;
    IndexMap indexes;

    typedef QVector<Holder> HolderVector;
    HolderVector holders;
};

QString QSqlResultPrivate::holderAt(int index) const
{
    return indexes.key(index);
}

void QSqlResultPrivate::positionalToNamedBinding()
{
    QRegExp rx(QLatin1String("'[^']*'|\\?"));
    QString q = sql;
    int i = 0, cnt = -1;
    while ((i = rx.indexIn(q, i)) != -1) {
        if (rx.cap(0) == QLatin1String("?"))
            q = q.replace(i, 1, QLatin1String(":f") + QString::number(++cnt));
        i += rx.matchedLength();
    }
    executedQuery = q;
}

void QSqlResultPrivate::namedToPositionalBinding()
{
    QRegExp rx(QLatin1String("'[^']*'|:([a-zA-Z0-9_]+)"));
    QString q = sql;
    int i = 0, cnt = -1;
    while ((i = rx.indexIn(q, i)) != -1) {
        if (rx.cap(1).isEmpty()) {
            i += rx.matchedLength();
        } else {
            // record the index of the placeholder - needed
            // for emulating named bindings with ODBC
            indexes[rx.cap(0)]= ++cnt;
            q = q.replace(i, rx.matchedLength(), QLatin1String("?"));
            ++i;
        }
    }
    executedQuery = q;
}

/*!
    \class QSqlResult
    \brief The QSqlResult class provides an abstract interface for
    accessing data from specific SQL databases.

    \ingroup database
    \module sql

    Normally, you would use QSqlQuery instead of QSqlResult, since
    QSqlQuery provides a generic wrapper for database-specific
    implementations of QSqlResult.

    If you are implementing your own SQL driver (by subclassing
    QSqlDriver), you will need to provide your own QSqlResult
    subclass that implements all the pure virtual functions and other
    virtual functions that you need.

    \sa QSqlDriver
*/

/*!
    \enum QSqlResult::BindingSyntax

    This enum type specifies the different syntaxes for specifying
    placeholders in prepared queries.

    \value PositionalBinding Use the ODBC-style positional syntax, with "?" as placeholders.
    \value NamedBinding Use the Oracle-style syntax with named placeholders (e.g., ":id")
    \omitvalue BindByPosition
    \omitvalue BindByName

    \sa bindingSyntax()
*/

/*!
    Creates a QSqlResult using database driver \a db. The object is
    initialized to an inactive state.

    \sa isActive(), driver()
*/

QSqlResult::QSqlResult(const QSqlDriver *db)
{
    d = new QSqlResultPrivate(this);
    d->sqldriver = db;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlResult::~QSqlResult()
{
    delete d;
}

/*!
    Sets the current query for the result to \a query. You must call
    reset() to execute the query on the database.

    \sa reset(), lastQuery()
*/

void QSqlResult::setQuery(const QString& query)
{
    d->sql = query;
}

/*!
    Returns the current SQL query text, or an empty string if there
    isn't one.

    \sa setQuery()
*/

QString QSqlResult::lastQuery() const
{
    return d->sql;
}

/*!
    Returns the current (zero-based) row position of the result. May
    return the special values QSql::BeforeFirstRow or
    QSql::AfterLastRow.

    \sa setAt(), isValid()
*/
int QSqlResult::at() const
{
    return d->idx;
}


/*!
    Returns true if the result is positioned on a valid record (that
    is, the result is not positioned before the first or after the
    last record); otherwise returns false.

    \sa at()
*/

bool QSqlResult::isValid() const
{
    return d->idx != QSql::BeforeFirstRow && d->idx != QSql::AfterLastRow;
}

/*!
    \fn bool QSqlResult::isNull(int index)

    Returns true if the field at position \a index in the current row
    is null; otherwise returns false.
*/

/*!
    Returns true if the result has records to be retrieved; otherwise
    returns false.
*/

bool QSqlResult::isActive() const
{
    return d->active;
}

/*!
    This function is provided for derived classes to set the
    internal (zero-based) row position to \a index.

    \sa at()
*/

void QSqlResult::setAt(int index)
{
    d->idx = index;
}


/*!
    This function is provided for derived classes to indicate whether
    or not the current statement is a SQL \c SELECT statement. The \a
    select parameter should be true if the statement is a \c SELECT
    statement; otherwise it should be false.

    \sa isSelect()
*/

void QSqlResult::setSelect(bool select)
{
    d->isSel = select;
}

/*!
    Returns true if the current result is from a \c SELECT statement;
    otherwise returns false.

    \sa setSelect()
*/

bool QSqlResult::isSelect() const
{
    return d->isSel;
}

/*!
    Returns the driver associated with the result. This is the object
    that was passed to the constructor.
*/

const QSqlDriver *QSqlResult::driver() const
{
    return d->sqldriver;
}


/*!
    This function is provided for derived classes to set the internal
    active state to \a active.

    \sa isActive()
*/

void QSqlResult::setActive(bool active)
{
    d->active = active;
}

/*!
    This function is provided for derived classes to set the last
    error to \a error.

    \sa lastError()
*/

void QSqlResult::setLastError(const QSqlError &error)
{
    d->error = error;
}


/*!
    Returns the last error associated with the result.
*/

QSqlError QSqlResult::lastError() const
{
    return d->error;
}

/*!
    \fn int QSqlResult::size()

    Returns the size of the \c SELECT result, or -1 if it cannot be
    determined or if the query is not a \c SELECT statement.

    \sa numRowsAffected()
*/

/*!
    \fn int QSqlResult::numRowsAffected()

    Returns the number of rows affected by the last query executed, or
    -1 if it cannot be determined or if the query is a \c SELECT
    statement.

    \sa size()
*/

/*!
    \fn QCoreVariant QSqlResult::data(int index)

    Returns the data for field \a index in the current row as
    a QCoreVariant. This function is only called if the result is in
    an active state and is positioned on a valid record and \a index is
    non-negative. Derived classes must reimplement this function and
    return the value of field \a index, or QCoreVariant() if it cannot be
    determined.
*/

/*!
    \fn  bool QSqlResult::reset(const QString &query)

    Sets the result to use the SQL statement \a query for subsequent
    data retrieval.
    
    Derived classes must reimplement this function and apply the \a
    query to the database. This function is only called after the
    result is set to an inactive state and is positioned before the
    first record of the new result. Derived classes should return
    true if the query was successful and ready to be used, or false
    otherwise.

    \sa setQuery()
*/

/*!
    \fn bool QSqlResult::fetch(int index)

    Positions the result to an arbitrary (zero-based) row \a index.
    
    This function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the
    result to the row \a index, and call setAt() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa isActive(), fetchFirst(), fetchLast(), fetchNext(), fetchPrevious()
*/

/*!
    \fn bool QSqlResult::fetchFirst()

    Positions the result to the first record (row 0) in the result.

    This function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the
    result to the first record, and call setAt() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa fetch(), fetchLast()
*/

/*!
    \fn bool QSqlResult::fetchLast()

    Positions the result to the last record (last row) in the result.

    This function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the
    result to the last record, and call setAt() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa fetch(), fetchFirst()
*/

/*!
    Positions the result to the next available record (row) in the
    result.

    This function is only called if the result is in an active
    state. The default implementation calls fetch() with the next
    index. Derived classes can reimplement this function and position
    the result to the next record in some other way, and call setAt()
    with an appropriate value. Return true to indicate success, or
    false to signify failure.

    \sa fetch(), fetchPrevious()
*/

bool QSqlResult::fetchNext()
{
    return fetch(at() + 1);
}

/*!
    Positions the result to the previous record (row) in the result.

    This function is only called if the result is in an active state.
    The default implementation calls fetch() with the previous index.
    Derived classes can reimplement this function and position the
    result to the next record in some other way, and call setAt()
    with an appropriate value. Return true to indicate success, or
    false to signify failure.
*/

bool QSqlResult::fetchPrevious()
{
    return fetch(at() - 1);
}

/*!
    Returns true if you can only scroll forward through the result
    set; otherwise returns false.

    \sa setForwardOnly()
*/
bool QSqlResult::isForwardOnly() const
{
    return d->forwardOnly;
}

/*!
    Sets forward only mode to \a forward. If \a forward is true, only
    fetchNext() is allowed for navigating the results. Forward only
    mode needs much less memory since results do not have to be
    cached. By default, this feature is disabled.

    \sa isForwardOnly(), fetchNext()
*/
void QSqlResult::setForwardOnly(bool forward)
{
    d->forwardOnly = forward;
}

/*!
    Prepares the given \a query, using the underlying database
    functionality where possible.

    \sa prepare
*/
bool QSqlResult::savePrepare(const QString& query)
{
    if (!driver())
        return false;
    d->clear();
    d->sql = query;
    if (!driver()->hasFeature(QSqlDriver::PreparedQueries))
        return prepare(query);

    if (driver()->hasFeature(QSqlDriver::NamedPlaceholders))
        d->positionalToNamedBinding();
    else
        d->namedToPositionalBinding();
    return prepare(d->executedQuery);
}

/*!
    Prepares the given \a query for execution; the query will normally
    use placeholders so that it can be executed repeatedly.

    \sa exec()
*/
bool QSqlResult::prepare(const QString& query)
{
    QRegExp rx(QLatin1String("'[^']*'|:([a-zA-Z0-9_]+)"));
    int i = 0;
    while ((i = rx.indexIn(query, i)) != -1) {
        if (!rx.cap(1).isEmpty())
            d->holders.append(Holder(rx.cap(0), i));
        i += rx.matchedLength();
    }
    d->sql = query;
    return true; // fake prepares should always succeed
}

/*!
    Executes the query.

    \sa prepare()
*/
bool QSqlResult::exec()
{
    bool ret;
    // fake preparation - just replace the placeholders..
    QString query = lastQuery();
    if (d->binds == NamedBinding) {
        int i;
        QCoreVariant val;
        QString holder;
        for (i = d->holders.count() - 1; i >= 0; --i) {
            holder = d->holders[i].holderName;
            val = d->values[d->indexes[holder]];
            QSqlField f(QLatin1String(""), val.type());
            f.setValue(val);
            query = query.replace(d->holders[i].holderPos,
                                   holder.length(), driver()->formatValue(f));
        }
    } else {
        QString val;
        int i = 0;
        int idx = 0;
        for (idx = 0; idx < d->values.count(); ++idx) {
            i = query.indexOf(QLatin1Char('?'), i);
            if (i == -1)
                continue;
            QCoreVariant var = d->values[idx];
            QSqlField f(QLatin1String(""), var.type());
            if (var.isNull())
                f.clear();
            else
                f.setValue(var);
            val = driver()->formatValue(f);
            query = query.replace(i, 1, driver()->formatValue(f));
            i += val.length();
        }
    }

    // have to retain the original query with placeholders
    QString orig = lastQuery();
    ret = reset(query);
    d->executedQuery = query;
    setQuery(orig);
    d->resetBindCount();
    return ret;
}

/*!
    Binds the value \a val of parameter type \a paramType to position \a index
    in the current record (row).

    \sa addBindValue()
*/
void QSqlResult::bindValue(int index, const QCoreVariant& val, QSql::ParamType paramType)
{
    d->binds = PositionalBinding;
    QString nm(QLatin1String(":f") + QString::number(index));
    d->indexes[nm] = index;
    if (d->values.count() <= index)
        d->values.resize(index + 1);
    d->values[index] = val;
    if (paramType != QSql::In || !d->types.isEmpty())
        d->types[index] = paramType;
}

/*!
    \overload

    Binds the value \a val of parameter type \a paramType to the \a
    placeholder name in the current record (row).
*/
void QSqlResult::bindValue(const QString& placeholder, const QCoreVariant& val,
                           QSql::ParamType paramType)
{
    d->binds = NamedBinding;
    // if the index has already been set when doing emulated named
    // bindings - don't reset it
    int idx = d->indexes.value(placeholder, -1);
    if (idx >= 0) {
        if (d->values.count() <= idx)
            d->values.resize(idx + 1);
        d->values[idx] = val;
    } else {
        d->values.append(val);
        idx = d->values.count() - 1;
        d->indexes[placeholder] = idx;
    }

    if (paramType != QSql::In || !d->types.isEmpty())
        d->types[idx] = paramType;
}

/*!
    Binds the value \a val of parameter type \a paramType to the next
    available position in the current record (row).

    \sa bindValue()
*/
void QSqlResult::addBindValue(const QCoreVariant& val, QSql::ParamType paramType)
{
    d->binds = PositionalBinding;
    bindValue(d->bindCount, val, paramType);
    ++d->bindCount;
}

/*!
    Returns the value bound at position \a index in the current record
    (row).

    \sa bindValue(), boundValues()
*/
QCoreVariant QSqlResult::boundValue(int index) const
{
    return d->values.value(index);
}

/*!
    \overload

    Returns the value bound by the given \a placeholder name in the
    current record (row).

    \sa bindValueType()
*/
QCoreVariant QSqlResult::boundValue(const QString& placeholder) const
{
    int idx = d->indexes.value(placeholder, -1);
    return d->values.value(idx);
}

/*!
    Returns the parameter type for the value bound at position \a index.

    \sa boundValue()
*/
QSql::ParamType QSqlResult::bindValueType(int index) const
{
    return d->types.value(index, QSql::In);
}

/*!
    \overload

    Returns the parameter type for the value bound with the given \a
    placeholder name.
*/
QSql::ParamType QSqlResult::bindValueType(const QString& placeholder) const
{
    return d->types.value(d->indexes.value(placeholder, -1), QSql::In);
}

/*!
    Returns the number of bound values in the result.

    \sa boundValues()
*/
int QSqlResult::boundValueCount() const
{
    return d->values.count();
}

/*!
    Returns a vector of the result's bound values for the current
    record (row).

    \sa boundValueCount()
*/
QVector<QCoreVariant>& QSqlResult::boundValues() const
{
    return d->values;
}

/*!
    Returns the binding syntax used by prepared queries.
*/
QSqlResult::BindingSyntax QSqlResult::bindingSyntax() const
{
    return d->binds;
}

/*!
    Clears the entire result set and releases any associated
    resources.
*/
void QSqlResult::clear()
{
    d->clear();
}

/*!
    Returns the query that was actually executed. This may differ from
    the query that was passed, for example if bound values were used
    with a prepared query and the underlying database doesn't support
    prepared queries.

    \sa exec(), setQuery()
*/
QString QSqlResult::executedQuery() const
{
    return d->executedQuery;
}

void QSqlResult::resetBindCount()
{
    d->resetBindCount();
}

/*!
    Returns the name of the bound value at position \a index in the
    current record (row).

    \sa boundValue()
*/
QString QSqlResult::boundValueName(int index) const
{
    return d->holderAt(index);
}

/*!
    Returns true if at least one of the query's bound values is a \c
    QSql::Out or a \c QSql::InOut; otherwise returns false.

    \sa bindValueType()
*/
bool QSqlResult::hasOutValues() const
{
    if (d->types.isEmpty())
        return false;
    QMap<int, QSql::ParamType>::ConstIterator it;
    for (it = d->types.constBegin(); it != d->types.constEnd(); ++it) {
        if (it.value() != QSql::In)
            return true;
    }
    return false;
}

/*!
    Returns the current record if the query is active; otherwise
    returns an empty QSqlRecord.

    The default implementation always returns an empty QSqlRecord.

    \sa isActive()
*/
QSqlRecord QSqlResult::record() const
{
    return QSqlRecord();
}

/*!
    Returns the object ID of the most recent inserted row if the
    database supports it.
    An invalid QCoreVariant will be returned if the query did not
    insert any value or if the database does not report the id back.
    If more than one row was touched by the insert, the behavior is
    undefined.

    \sa QSqlDriver::hasFeature()
*/
QCoreVariant QSqlResult::lastInsertId() const
{
    return QCoreVariant();
}

/*! \internal
*/
void QSqlResult::virtual_hook(int, void *)
{
}


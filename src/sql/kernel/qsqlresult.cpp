/****************************************************************************
**
** Implementation of QSqlResult class.
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

#include "qmap.h"
#include "qregexp.h"
#include "qsqlresult.h"
#include "qvector.h"
#include "qsqldriver.h"

#ifndef QT_NO_SQL

struct Holder {
    Holder(const QString& hldr = QString(), int pos = -1): holderName(hldr), holderPos(pos) {}
    bool operator==(const Holder& h) const { return h.holderPos == holderPos && h.holderName == holderName; }
    bool operator!=(const Holder& h) const { return h.holderPos != holderPos || h.holderName != holderName; }
    QString holderName;
    int            holderPos;
};

class QSqlResultPrivate
{
public:
    QSqlResultPrivate(QSqlResult* d)
    : q(d), sqldriver(0), idx(QSql::BeforeFirst), active(false),
      isSel(false), forwardOnly(false), bindCount(0), bindm(QSqlResult::BindByPosition)
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
        index.clear();
        holders.clear();
        types.clear();
    }

    void clear()
    {
        clearValues();
        clearIndex();
    }

    void positionalToNamedBinding();
    void namedToPositionalBinding();
    QString holderAt(int pos) const;

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
    QSqlResult::BindMethod bindm;

    QString executedQuery;
    QMap<int, QSql::ParamType> types;
    QVector<QCoreVariant> values;
    typedef QMap<QString, int> IndexMap;
    IndexMap index;

    typedef QVector<Holder> HolderVector;
    HolderVector holders;
};

QString QSqlResultPrivate::holderAt(int pos) const
{
    IndexMap::ConstIterator it;
    for (it = index.begin(); it != index.end(); ++it) {
        if (it.value() == pos)
            return it.key();
    }
    return QString();
}

void QSqlResultPrivate::positionalToNamedBinding()
{
    QRegExp rx("'[^']*'|\\?");
    QString q = sql;
    int i = 0, cnt = -1;
    while ((i = rx.indexIn(q, i)) != -1) {
        if (rx.cap(0) == "?") {
            q = q.replace(i, 1, ":f" + QString::number(++cnt));
        }
        i += rx.matchedLength();
    }
    executedQuery = q;
}

void QSqlResultPrivate::namedToPositionalBinding()
{
    QRegExp rx("'[^']*'|:([a-zA-Z0-9_]+)");
    QString q = sql;
    int i = 0, cnt = -1;
    while ((i = rx.indexIn(q, i)) != -1) {
        if (rx.cap(1).isEmpty()) {
            i += rx.matchedLength();
        } else {
            // record the index of the placeholder - needed
            // for emulating named bindings with ODBC
            index[rx.cap(0)]= ++cnt;
            q = q.replace(i, rx.matchedLength(), "?");
            ++i;
        }
    }
    executedQuery = q;
}

/*!
    \class QSqlResult
    \brief The QSqlResult class provides an abstract interface for
    accessing data from SQL databases.

    \ingroup database
    \module sql

    Normally you would use QSqlQuery instead of QSqlResult since QSqlQuery
    provides a generic wrapper for database-specific implementations of
    QSqlResult.

    If you are implementing your own SQL driver you will need to
    provide your own QSqlResult subclass that implements all the pure
    virtual functions, and the other virtual functions that you need,
    (as well as a QSqlDriver subclass).

    \sa QSql
*/

/*!
    \enum QSqlResult::BindMethod

    \value BindByPosition
    \value BindByName
*/

/*!
    This constructor creates a QSqlResult using database \a db. The
    object is initialized to an inactive state.
*/

QSqlResult::QSqlResult(const QSqlDriver * db)
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
    Sets the current query for the result to \a query. The result must
    be reset() in order to execute the query on the database.
*/

void QSqlResult::setQuery(const QString& query)
{
    d->sql = query;
}

/*!
    Returns the current SQL query text, or an empty string if there
    isn't one.
*/

QString QSqlResult::lastQuery() const
{
    return d->sql;
}

/*!
    Returns the current (zero-based) row position of the result.
*/

int QSqlResult::at() const
{
    return d->idx;
}


/*!
    Returns true if the result is positioned on a valid record (that
    is, the result is not positioned before the first or after the
    last record); otherwise returns false.
*/

bool QSqlResult::isValid() const
{
    return (d->idx != QSql::BeforeFirst && \
            d->idx != QSql::AfterLast) ? true : false;
}

/*!
    \fn bool QSqlResult::isNull(int i)

    Returns true if the field at position \a i in the current row is
    NULL; otherwise returns false.
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
    internal (zero-based) row position to \a at.

    \sa at()
*/

void QSqlResult::setAt(int at)
{
    d->idx = at;
}


/*!
    This function is provided for derived classes to indicate
    whether or not the current statement is a SQL \c SELECT statement.
    The \a s parameter should be true if the statement is a \c SELECT
    statement; otherwise it should be false.
*/

void QSqlResult::setSelect(bool s)
{
    d->isSel = s;
}

/*!
    Returns true if the current result is from a \c SELECT statement;
    otherwise returns false.
*/

bool QSqlResult::isSelect() const
{
    return d->isSel;
}

/*!
    Returns the driver associated with the result.
*/

const QSqlDriver* QSqlResult::driver() const
{
    return d->sqldriver;
}


/*!
    This function is provided for derived classes to set the internal
    active state to the value of \a a.

    \sa isActive()
*/

void QSqlResult::setActive(bool a)
{
    d->active = a;
}

/*!
    This function is provided for derived classes to set the last
    error to the value of \a e.

    \sa lastError()
*/

void QSqlResult::setLastError(const QSqlError& e)
{
    d->error = e;
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

    Returns the size of the result or -1 if it cannot be determined or
    if the query is not a \c SELECT statement.

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
    \fn QCoreVariant QSqlResult::data(int i)

    Returns the data for field \a i (zero-based) in the current row as
    a QCoreVariant. This function is only called if the result is in
    an active state and is positioned on a valid record and \a i is
    non-negative. Derived classes must reimplement this function and
    return the value of field \a i, or QCoreVariant() if it cannot be
    determined.
*/

/*!
    \fn  bool QSqlResult::reset(const QString& query)

    Sets the result to use the SQL statement \a query for subsequent
    data retrieval. Derived classes must reimplement this function and
    apply the \a query to the database. This function is only called
    after the result is set to an inactive state and is positioned
    before the first record of the new result. Derived classes should
    return true if the query was successful and ready to be used,
    or false otherwise.
*/

/*!
    \fn bool QSqlResult::fetch(int i)

    Positions the result to an arbitrary (zero-based) row \a i. This
    function is only called if the result is in an active state. Derived
    classes must reimplement this function and position the result to the
    row \a i, and call setAt() with an appropriate value. Return true
    to indicate success, or false to signify failure.
*/

/*!
    \fn bool QSqlResult::fetchFirst()

    Positions the result to the first record (row 0) in the result.
    This function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the
    result to the first record, and call setAt() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.
*/

/*!
    \fn bool QSqlResult::fetchLast()

    Positions the result to the last record (last row) in the result.
    This function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the
    result to the last record, and call setAt() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.
*/

/*!
    Positions the result to the next available record (row) in the
    result. This function is only called if the result is in an active
    state. The default implementation calls fetch() with the next
    index. Derived classes can reimplement this function and position
    the result to the next record in some other way, and call setAt()
    with an appropriate value. Return true to indicate success, or
    false to signify failure.
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
    result to the next record in some other way, and call setAt() with
    an appropriate value. Return true to indicate success, or false to
    signify failure.
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
    Sets forward only mode to \a forward. If forward is true only
    fetchNext() is allowed for navigating the results. Forward only
    mode needs much less memory since results do not have to be cached.
    forward only mode is off by default.

    \sa isForwardOnly() fetchNext()
*/
void QSqlResult::setForwardOnly(bool forward)
{
    d->forwardOnly = forward;
}

/*!
    Prepares the given \a query, using the underlying database
    functionality where possible.
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
    use placeholders so that it can be repeatedly executed.
*/
bool QSqlResult::prepare(const QString& query)
{
    QRegExp rx("'[^']*'|:([a-zA-Z0-9_]+)");
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
    if (d->bindm == BindByName) {
        int i;
        QCoreVariant val;
        QString holder;
        for (i = d->holders.count() - 1; i >= 0; --i) {
            holder = d->holders[i].holderName;
            val = d->values[d->index[holder]];
            QSqlField f("", val.type());
            f.setValue(val);
            query = query.replace(d->holders[i].holderPos,
                                   holder.length(), driver()->formatValue(f));
        }
    } else {
        QString val;
        int i = 0;
        int idx = 0;
        for (idx = 0; idx < d->values.count(); ++idx) {
            i = query.indexOf('?', i);
            if (i == -1)
                continue;
            QCoreVariant var = d->values[idx];
            QSqlField f("", var.type());
            if (var.isNull())
                f.clear();
            else
                f.setValue(var);
            val = driver()->formatValue(f);
            query = query.replace(i, 1, driver()->formatValue(f));
            i += val.length();
        }
    }
    // have to retain the original query w/placeholders..
    QString orig = lastQuery();
    ret = reset(query);
    d->executedQuery = query;
    setQuery(orig);
    d->resetBindCount();
    return ret;
}

/*!
    \overload

    Binds the value \a val of parameter type \a tp to the \a
    placeholder name in the current record (row).
*/
void QSqlResult::bindValue(const QString& placeholder, const QCoreVariant& val, QSql::ParamType tp)
{
    d->bindm = BindByName;
    // if the index has already been set when doing emulated named
    // bindings - don't reset it
    int idx = d->index.value(placeholder, -1);
    if (idx >= 0) {
        if (d->values.count() <= idx)
            d->values.resize(idx + 1);
        d->values[idx] = val;
    } else {
        d->values.append(val);
        idx = d->values.count() - 1;
        d->index[placeholder] = idx;
    }

    if (tp != QSql::In || !d->types.isEmpty())
        d->types[idx] = tp;
}

/*!
    Binds the value \a val of parameter type \a tp to position \a pos
    in the current record (row).
*/
void QSqlResult::bindValue(int pos, const QCoreVariant& val, QSql::ParamType tp)
{
    d->bindm = BindByPosition;
    QString nm(":f" + QString::number(pos));
    d->index[nm] = pos;
    if (d->values.count() <= pos)
        d->values.resize(pos + 1);
    d->values[pos] = val;
    if (tp != QSql::In || !d->types.isEmpty())
        d->types[pos] = tp;
}

/*!
    Binds the value \a val of parameter type \a tp to the next
    available position in the current record (row).
*/
void QSqlResult::addBindValue(const QCoreVariant& val, QSql::ParamType tp)
{
    d->bindm = BindByPosition;
    bindValue(d->bindCount, val, tp);
    ++d->bindCount;
}

/*!
    \overload

    Returns the value bound by the given  \a placeholder name in the
    current record (row).
*/
QCoreVariant QSqlResult::boundValue(const QString& placeholder) const
{
    int idx = d->index.value(placeholder, -1);
    if (idx < 0)
        return QCoreVariant();
    return d->values.at(idx);
}

/*!
    Returns the value bound at position \a pos in the current record
    (row).
*/
QCoreVariant QSqlResult::boundValue(int pos) const
{
    if (pos < 0 || pos >= d->values.count())
        return QCoreVariant();
    return d->values.at(pos);
}

/*!
    \overload

    Returns the parameter type for the value bound with the given \a
    placeholder name.
*/
QSql::ParamType QSqlResult::bindValueType(const QString& placeholder) const
{
    return d->types.value(d->index.value(placeholder, -1), QSql::In);
}

/*!
    Returns the parameter type for the value bound at position \a pos.
*/
QSql::ParamType QSqlResult::bindValueType(int pos) const
{
    if (pos < 0 || pos >= d->values.count())
        return QSql::In;
    return d->types.value(pos, QSql::In);
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
    Returns the bind method used for prepared queries, either \c
    BindByPosition or \c BindByName.
*/
QSqlResult::BindMethod QSqlResult::bindMethod() const
{
    return d->bindm;
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
    Returns the name of the bound value at position \a pos in the
    current record (row).
*/
QString QSqlResult::boundValueName(int pos) const
{
    return d->holderAt(pos);
}

/*!
    Returns true if at least one of the query's bound values is a
    \c QSql::Out or a \c QSql::InOut; otherwise returns false.
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
    The base class implementation does nothing and returns an empty
    QSqlRecord.
*/
QSqlRecord QSqlResult::record() const
{
    return QSqlRecord();
}


#endif // QT_NO_SQL

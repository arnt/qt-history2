/****************************************************************************
**
** Implementation of QSqlResult class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
    Holder( const QString& hldr = QString(), int pos = -1 ): holderName( hldr ), holderPos( pos ) {}
    bool operator==( const Holder& h ) const { return h.holderPos == holderPos && h.holderName == holderName; }
    bool operator!=( const Holder& h ) const { return h.holderPos != holderPos || h.holderName != holderName; }
    QString holderName;
    int	    holderPos;
};

class QSqlResultPrivate
{
public:    
    QSqlResultPrivate(QSqlResult* d)
    : q(d), sqldriver(0), idx(QSql::BeforeFirst), active(FALSE), 
      isSel(FALSE), forwardOnly(FALSE), bindCount(0), bindm(QSqlResult::BindByPosition)
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
    QString holderAt( int pos ) const;
    
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
    QVector<QVariant> values;
    typedef QMap<QString, int> IndexMap;
    IndexMap index;
    
    typedef QVector<Holder> HolderVector;
    HolderVector holders;
};

QString QSqlResultPrivate::holderAt( int pos ) const
{
    IndexMap::ConstIterator it;
    for ( it = index.begin(); it != index.end(); ++it ) {
	if ( it.value() == pos )
	    return it.key();
    }
    return QString();
}

void QSqlResultPrivate::positionalToNamedBinding()
{
    QRegExp rx("'[^']*'|\\?");
    QString q = sql;
    int i = 0, cnt = -1;
    while ( (i = rx.search( q, i )) != -1 ) {
	if ( rx.cap(0) == "?" ) {
	    q = q.replace( i, 1, ":f" + QString::number(++cnt) );
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
    while ( (i = rx.search( q, i )) != -1 ) {
	if ( rx.cap(1).isEmpty() ) {
	    i += rx.matchedLength();
	} else {
	    // record the index of the placeholder - needed
	    // for emulating named bindings with ODBC
	    index[ rx.cap(0) ]= ++cnt;
	    q = q.replace( i, rx.matchedLength(), "?" );
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

    \sa QSql
*/


/*!
    Protected constructor which creates a QSqlResult using database \a
    db. The object is initialized to an inactive state.
*/

QSqlResult::QSqlResult( const QSqlDriver * db )
{
    d = new QSqlResultPrivate( this );
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

void QSqlResult::setQuery( const QString& query )
{
    d->sql = query;
}

/*!
    Returns the current SQL query text, or an empty QString if there is none.
*/

QString QSqlResult::lastQuery() const
{
    return d->sql;
}

/*!
    Returns the current (zero-based) position of the result.
*/

int QSqlResult::at() const
{
    return d->idx;
}


/*!
    Returns TRUE if the result is positioned on a valid record (that
    is, the result is not positioned before the first or after the
    last record); otherwise returns FALSE.
*/

bool QSqlResult::isValid() const
{
    return ( d->idx != QSql::BeforeFirst && \
	    d->idx != QSql::AfterLast ) ? TRUE : FALSE;
}

/*!
    \fn bool QSqlResult::isNull( int i )

    Returns TRUE if the field at position \a i is NULL; otherwise
    returns FALSE.
*/


/*!
    Returns TRUE if the result has records to be retrieved; otherwise
    returns FALSE.
*/

bool QSqlResult::isActive() const
{
    return d->active;
}

/*!
    Protected function provided for derived classes to set the
    internal (zero-based) result index to \a at.

    \sa at()
*/

void QSqlResult::setAt( int at )
{
    d->idx = at;
}


/*!
    Protected function provided for derived classes to indicate
    whether or not the current statement is a SQL SELECT statement.
    The \a s parameter should be TRUE if the statement is a SELECT
    statement, or FALSE otherwise.
*/

void QSqlResult::setSelect( bool s )
{
    d->isSel = s;
}

/*!
    Returns TRUE if the current result is from a SELECT statement;
    otherwise returns FALSE.
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
    Protected function provided for derived classes to set the
    internal active state to the value of \a a.

    \sa isActive()
*/

void QSqlResult::setActive( bool a )
{
    d->active = a;
}

/*!
    Protected function provided for derived classes to set the last
    error to the value of \a e.

    \sa lastError()
*/

void QSqlResult::setLastError( const QSqlError& e )
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

    Returns the size of the result or -1 if it cannot be determined.
*/

/*!
    \fn int QSqlResult::numRowsAffected()

    Returns the number of rows affected by the last query executed.
*/

/*!
    \fn QVariant QSqlResult::data( int i )

    Returns the data for field \a i (zero-based) as a QVariant. This
    function is only called if the result is in an active state and is
    positioned on a valid record and \a i is non-negative.
    Derived classes must reimplement this function and return the value
    of field \a i, or QVariant() if it cannot be determined.
*/

/*!
    \fn  bool QSqlResult::reset( const QString& query )

    Sets the result to use the SQL statement \a query for subsequent
    data retrieval. Derived classes must reimplement this function and
    apply the \a query to the database. This function is called only
    after the result is set to an inactive state and is positioned
    before the first record of the new result. Derived classes should
    return TRUE if the query was successful and ready to be used,
    or FALSE otherwise.
*/

/*!
    \fn bool QSqlResult::fetch( int i )

    Positions the result to an arbitrary (zero-based) index \a i. This
    function is only called if the result is in an active state. Derived
    classes must reimplement this function and position the result to the
    index \a i, and call setAt() with an appropriate value. Return TRUE
    to indicate success, or FALSE to signify failure.
*/

/*!
    \fn bool QSqlResult::fetchFirst()

    Positions the result to the first record in the result. This
    function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the result
    to the first record, and call setAt() with an appropriate value.
    Return TRUE to indicate success, or FALSE to signify failure.
*/

/*!
    \fn bool QSqlResult::fetchLast()

    Positions the result to the last record in the result. This
    function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the result
    to the last record, and call setAt() with an appropriate value.
    Return TRUE to indicate success, or FALSE to signify failure.
*/

/*!
    Positions the result to the next available record in the result.
    This function is only called if the result is in an active state.
    The default implementation calls fetch() with the next index.
    Derived classes can reimplement this function and position the result
    to the next record in some other way, and call setAt() with an
    appropriate value. Return TRUE to indicate success, or FALSE to
    signify failure.
*/

bool QSqlResult::fetchNext()
{
    return fetch( at() + 1 );
}

/*!
    Positions the result to the previous available record in the
    result. This function is only called if the result is in an active
    state. The default implementation calls fetch() with the previous
    index. Derived classes can reimplement this function and position the
    result to the next record in some other way, and call setAt() with
    an appropriate value. Return TRUE to indicate success, or FALSE to
    signify failure.
*/

bool QSqlResult::fetchPrev()
{
    return fetch( at() - 1 );
}

/*!
    Returns TRUE if you can only scroll forward through a result set;
    otherwise returns FALSE.
*/
bool QSqlResult::isForwardOnly() const
{
    return d->forwardOnly;
}

/*!
    Sets forward only mode to \a forward. If forward is TRUE only
    fetchNext() is allowed for navigating the results. Forward only
    mode needs far less memory since results do not have to be cached.
    forward only mode is off by default.

    \sa fetchNext()
*/
void QSqlResult::setForwardOnly( bool forward )
{
    d->forwardOnly = forward;
}

bool QSqlResult::savePrepare( const QString& query )
{
    if ( !driver() )
	return FALSE;
    d->clear();
    d->sql = query;
    if ( !driver()->hasFeature( QSqlDriver::PreparedQueries ) )
	return prepare( query );

    if ( driver()->hasFeature( QSqlDriver::NamedPlaceholders ) )
	d->positionalToNamedBinding();
    else
	d->namedToPositionalBinding();
    return prepare( d->executedQuery );
}

bool QSqlResult::prepare( const QString& query )
{
    QRegExp rx("'[^']*'|:([a-zA-Z0-9_]+)");    
    int i = 0;
    while ( (i = rx.search( query, i )) != -1 ) {
	if ( !rx.cap(1).isEmpty() )
	    d->holders.append( Holder( rx.cap(0), i ) );
	i += rx.matchedLength();
    }
    d->sql = query;
    return TRUE; // fake prepares should always succeed
}

bool QSqlResult::exec()
{
    bool ret;
    // fake preparation - just replace the placeholders..
    QString query = lastQuery();
    if ( d->bindm == BindByName ) {
	int i;
	QVariant val;
	QString holder;
	for ( i = d->holders.count() - 1; i >= 0; --i ) {
	    holder = d->holders[ i ].holderName;
	    val = d->values[ d->index[ holder ] ];
	    QSqlField f( "", val.type() );
	    f.setValue( val );
	    query = query.replace( d->holders[ i ].holderPos,
				   holder.length(), driver()->formatValue( &f ) );
	}
    } else {
	QString val;
	int i = 0;
	int idx = 0;
	for ( idx = 0; idx < d->values.count(); ++idx ) {
	    i = query.indexOf( '?', i );
	    if ( i == -1 )
		continue;
	    QVariant var = d->values[ idx ];
	    QSqlField f( "", var.type() );
	    if ( var.isNull() )
		f.clear();
	    else
		f.setValue( var );
	    val = driver()->formatValue( &f );
	    query = query.replace( i, 1, driver()->formatValue( &f ) );
	    i += val.length();
	}
    }
    // have to retain the original query w/placeholders..
    QString orig = lastQuery();
    ret = reset( query );
    d->executedQuery = query;
    setQuery( orig );
    d->resetBindCount();
    return ret;
}

void QSqlResult::bindValue( const QString& placeholder, const QVariant& val, QSql::ParamType tp )
{
    d->bindm = BindByName;
    // if the index has already been set when doing emulated named
    // bindings - don't reset it
    int idx = d->index.value( placeholder, -1 );
    if ( idx >= 0 ) {
	if ( d->values.count() <= idx )
	    d->values.resize( idx + 1 );
	d->values[ idx ] = val;
    } else {
	d->values.append( val );
	idx = d->values.count() - 1;
	d->index[ placeholder ] = idx;
    }
    
    if ( tp != QSql::In || !d->types.isEmpty() )
	d->types[ idx ] = tp;
}

void QSqlResult::bindValue( int pos, const QVariant& val, QSql::ParamType tp )
{
    d->bindm = BindByPosition;
    QString nm( ":f" + QString::number( pos ) );
    d->index[ nm ] = pos;
    if ( d->values.count() <= pos )
	d->values.resize( pos + 1 );
    d->values[ pos ] = val;
    if ( tp != QSql::In || !d->types.isEmpty() )
	d->types[ pos ] = tp;
}

void QSqlResult::addBindValue( const QVariant& val, QSql::ParamType tp )
{
    d->bindm = BindByPosition;
    bindValue( d->bindCount, val, tp );
    ++d->bindCount;
}

QVariant QSqlResult::boundValue( const QString& placeholder ) const
{
    int idx = d->index.value( placeholder, -1 );
    if ( idx < 0 )
	return QVariant();
    return d->values.at( idx );
}

QVariant QSqlResult::boundValue( int pos ) const
{
    if ( pos < 0 || pos >= d->values.count() )
	return QVariant();
    return d->values.at( pos );
}

QSql::ParamType QSqlResult::bindValueType( const QString& placeholder ) const
{
    return d->types.value( d->index.value( placeholder, -1 ), QSql::In );
}

QSql::ParamType QSqlResult::bindValueType( int pos ) const
{
    if ( pos < 0 || pos >= d->values.count() )
	return QSql::In;
    return d->types.value( pos, QSql::In );
}

int QSqlResult::boundValueCount() const
{
    return d->values.count();
}

QVector<QVariant>& QSqlResult::boundValues() const
{
    return d->values;
}

QSqlResult::BindMethod QSqlResult::bindMethod() const
{
    return d->bindm;
}

void QSqlResult::clear()
{
    d->clear();
}

QString QSqlResult::executedQuery() const
{
    return d->executedQuery;
}

void QSqlResult::resetBindCount()
{
    d->resetBindCount();
}

QString QSqlResult::boundValueName( int pos ) const
{
    return d->holderAt( pos );
}

bool QSqlResult::hasOutValues() const
{
    if ( d->types.isEmpty() )
	return FALSE;
    QMap<int, QSql::ParamType>::ConstIterator it;
    for ( it = d->types.constBegin(); it != d->types.constEnd(); ++it ) {
	if ( it.value() != QSql::In )
	    return TRUE;
    }
    return FALSE;
}

QSqlRecord QSqlResult::record() const
{
    return QSqlRecord();
}


#endif // QT_NO_SQL

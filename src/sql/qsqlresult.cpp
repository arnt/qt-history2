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

struct Param {
    Param( const QVariant& v = QVariant(), QSql::ParameterType t = QSql::In ): value( v ), typ( t ) {}
    QVariant value;
    QSql::ParameterType typ;
    Q_DUMMY_COMPARISON_OPERATOR(Param)
};

struct Holder {
    Holder( const QString& hldr = QString::null, int pos = -1 ): holderName( hldr ), holderPos( pos ) {}
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
    }

    void clear()
    {
	clearValues();
	clearIndex();
    }

    void positionalToNamedBinding();
    void namedToPositionalBinding();
    
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
    QMap<int, QString> index;
    typedef QMap<QString, Param> ValueMap;
    ValueMap values;
    QString executedQuery;

    typedef QVector<Holder> HolderVector;
    HolderVector holders;
};

void QSqlResultPrivate::positionalToNamedBinding()
{
    QRegExp rx("'[^']*'|\\?");
    QString q = sql;
    int i = 0, cnt = 0;
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
    int i = 0, cnt = 0;
    while ( (i = rx.search( q, i )) != -1 ) {
	if ( rx.cap(1).isEmpty() ) {
	    i += rx.matchedLength();
	} else {
	    // record the index of the placeholder - needed
	    // for emulating named bindings with ODBC
	    index[ ++cnt ]= rx.cap(0);
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
    Returns the current SQL query text, or QString::null if there is none.
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
    d->sql = query;
    if ( !driver() )
	return FALSE;
    if ( !driver()->hasFeature( QSqlDriver::PreparedQueries ) )
	return TRUE;

    if ( driver()->hasFeature( QSqlDriver::NamedPlaceholders ) )
	d->positionalToNamedBinding();
    else
	d->namedToPositionalBinding();
    return TRUE;
}

bool QSqlResult::prepare( const QString& query )
{
    /*
	int i = 0;
	while ( (i = rx.search( q, i )) != -1 ) {
	    if ( !rx.cap(1).isEmpty() )
		d->sqlResult->extension()->holders.append( Holder( rx.cap(0), i ) );
	    i += rx.matchedLength();
	}
	return TRUE; // fake prepares should always succeed
    */

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
	for ( i = (int)d->holders.count() - 1; i >= 0; --i ) {
	    holder = d->holders[ i ].holderName;
	    val = d->values[ holder ].value;
	    QSqlField f( "", val.type() );
	    f.setValue( val );
	    query = query.replace( d->holders[ i ].holderPos,
				   holder.length(), driver()->formatValue( &f ) );
	}
    } else {
	QMap<int, QString>::ConstIterator it;
	QString val;
	int i = 0;
	for ( it = d->index.begin();
	it != d->index.end(); ++it ) {
	    i = query.find( '?', i );
	    if ( i > -1 ) {
		QSqlField f( "", d->values[ it.data() ].value.type() );
		if ( d->values[ it.data() ].value.isNull() )
		    f.setNull();
		else
		    f.setValue( d->values[ it.data() ].value );
		val = driver()->formatValue( &f );
		query = query.replace( i, 1, driver()->formatValue( &f ) );
		i += val.length();
	    }
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

void QSqlResult::bindValue( const QString& placeholder, const QVariant& val, QSql::ParameterType tp )
{
    d->bindm = BindByName;
    // if the index has already been set when doing emulated named
    // bindings - don't reset it
    if ( d->index[ d->values.count() ].isEmpty() ) {
	d->index[ d->values.count() ] = placeholder;
    }
    d->values[ placeholder ] = Param( val, tp );
}

void QSqlResult::bindValue( int pos, const QVariant& val, QSql::ParameterType tp )
{
    d->bindm = BindByPosition;
    QString nm = QString::number( pos );
    d->index[ pos ] = nm;
    d->values[ nm ] = Param( val, tp );
}

void QSqlResult::addBindValue( const QVariant& val, QSql::ParameterType tp )
{
    d->bindm = BindByPosition;
    bindValue( d->bindCount, val, tp );
    ++d->bindCount;
}

QVariant QSqlResult::parameterValue( const QString& holder ) const
{
    return d->values[ holder ].value;
}

QVariant QSqlResult::parameterValue( int pos ) const
{
    return d->values[ d->index[ pos ] ].value;
}

QVariant QSqlResult::boundValue( const QString& placeholder ) const
{
    return d->values[ placeholder ].value;
}

QVariant QSqlResult::boundValue( int pos ) const
{
    return d->values[ d->index[ pos ] ].value;
}

QSql::ParameterType QSqlResult::boundValueType( const QString& placeholder ) const
{
    return d->values[ placeholder ].typ;
}

QSql::ParameterType QSqlResult::boundValueType( int pos ) const
{
    return d->values[ d->index[ pos ] ].typ;
}

int QSqlResult::boundValueCount() const
{
    return d->values.count();
}

QMap<QString, QVariant> QSqlResult::boundValues() const
{
    QMap<QString, Param>::ConstIterator it;
    QMap<QString, QVariant> m;
    if ( d->bindm == BindByName ) {
	for ( it = d->values.begin(); it != d->values.end(); ++it )
	    m.insert( it.key(), it.data().value );
    } else {
	QString key, tmp, fmt;
	fmt.sprintf( "%%0%dd", QString::number( d->values.count()-1 ).length() );
	for ( it = d->values.begin(); it != d->values.end(); ++it ) {
	    tmp.sprintf( fmt.ascii(), it.key().toInt() );
	    m.insert( tmp, it.data().value );
	}
    }
    return m;
}

QSqlResult::BindMethod QSqlResult::bindMethod() const
{
    return d->bindm;
}

#endif // QT_NO_SQL

/****************************************************************************
**
** Implementation of QSqlQuery class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsqlquery.h"

#ifndef QT_NO_SQL

//#define QT_DEBUG_SQL

#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldatabase.h"
#include "qsql.h"
#include "qregexp.h"
#include "private/qsqlextension_p.h"


/*!
\internal
*/
QSqlResultShared::QSqlResultShared( QSqlResult* result ): sqlResult(result)
{
    if ( result )
	connect( result->driver(), SIGNAL(destroyed()), this, SLOT(slotResultDestroyed()) );
}

/*!
\internal
*/
QSqlResultShared::~QSqlResultShared()
{
    delete sqlResult;
}

/*!
\internal

In case a plugin gets unloaded the pointer to the sqlResult gets invalid
*/
void QSqlResultShared::slotResultDestroyed()
{
    delete sqlResult;
    sqlResult = 0;
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
    statements, e.g. \c{CREATE TABLE}. It can also be used to
    execute database-specific commands which are not standard SQL
    (e.g. \c{SET DATESTYLE=ISO} for PostgreSQL).

    Successfully executed SQL statements set the query's state to
    active (isActive() returns TRUE); otherwise the query's state is
    set to inactive. In either case, when executing a new SQL
    statement, the query is positioned on an invalid record; an active
    query must be navigated to a valid record (so that isValid()
    returns TRUE) before values can be retrieved.

    Navigating records is performed with the following functions:

    \list
    \i \c next()
    \i \c prev()
    \i \c first()
    \i \c last()
    \i \c \link QSqlQuery::seek() seek\endlink(int)
    \endlist

    These functions allow the programmer to move forward, backward or
    arbitrarily through the records returned by the query. Once an
    active query is positioned on a valid record, data can be
    retrieved using value(). All data is transferred from the SQL
    backend using QVariants.

    For example:

    \code
    QSqlQuery query( "SELECT name FROM customer" );
    while ( query.next() ) {
	QString name = query.value(0).toString();
	doSomething( name );
    }
    \endcode

    To access the data returned by a query, use the value() method.
    Each field in the data returned by a SELECT statement is accessed
    by passing the field's position in the statement, starting from 0.
    For the sake of efficiency there are no methods to access a field
    by name. (The \l QSqlCursor class provides a higher level
    interface for that generates SQL automatically and through which
    fields are accessed by name.)

    QSqlQuery supports prepared query execution and binding of
    parameter values to placeholders. Note that only input values may
    be bound. Be aware that not all databases support these
    features. Currently only the Oracle and ODBC drivers have proper
    prepared query support, but the rest of the drivers support this
    by emulating the missing features (the placeholders are simply
    replaced with the actual value when the query is executed). It is
    also important to know that different databases use different
    placeholder marks for value binding. Oracle uses a \c : character
    followed by a placeholder name, while ODBC only uses a \c ? 
    character to identify a placeholder. In an attempt to make this
    database independant we substitute the markers if you try to use
    ODBC markers in a query to an Oracle database and vice versa. Note
    that you can't mix the different bind styles by binding some
    values using named placeholders and some using positional
    placeholders.
    
    Example:

    \code
    // Named binding using named placeholders
    QSqlQuery q;
    q.prepare( "insert into mytable (id, name, lastname) values (:id, :name, :lname)" );
    q.bindValue( ":id", 0 );
    q.bindValue( ":name", "Testname" );
    q.bindValue( ":lname", "Lastname" );
    q.exec();

    // Positional binding using named placeholders
    QSqlQuery q;
    q.prepare( "insert into mytable (id, name, lastname) values (:id, :name, :lname)" );
    q.bindValue( 0, 0 );
    q.bindValue( 1, "Testname" );
    q.bindValue( 2, "Lastname" );
    q.exec();

    // Binding values using positional placeholders
    QSqlQuery q;
    q.prepare( "insert into mytable (id, name, lastname) values (?, ?, ?)" );
    q.bindValue( 0, 0 );
    q.bindValue( 1, "Testname" );
    q.bindValue( 2, "Lastname" );
    q.exec();
    
    // or alternatively
    q.prepare( "insert into mytable (id, name, lastname) values (?, ?, ?)" );
    q.addBindValue( 0 );
    q.addBindValue( "Testname" );
    q.addBindValue( "Lastname" );
    q.exec();
    \endcode
    
    \sa QSqlDatabase QSqlCursor QVariant
*/

/*!
    Creates a QSqlQuery object which uses the QSqlResult \a r to
    communicate with a database.
*/

QSqlQuery::QSqlQuery( QSqlResult * r )
{
    d = new QSqlResultShared( r );
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlQuery::~QSqlQuery()
{
    if (d->deref()) {
	delete d;
    }
}

/*!
    Constructs a copy of \a other.
*/

QSqlQuery::QSqlQuery( const QSqlQuery& other )
    : d(other.d)
{
    d->ref();
}

/*!
    Creates a QSqlQuery object using the SQL \a query and the database
    \a db. If \a db is 0, (the default), the application's default
    database is used.

    \sa QSqlDatabase
*/
QSqlQuery::QSqlQuery( const QString& query, QSqlDatabase* db )
{
    d = new QSqlResultShared( 0 );
    QSqlDatabase* database = db;
    if ( !database )
	database = QSqlDatabase::database();
    if ( database )
	*this = database->driver()->createQuery();
    if ( !query.isNull() )
	exec( query );
}

/*!
    Assigns \a other to the query.
*/

QSqlQuery& QSqlQuery::operator=( const QSqlQuery& other )
{
    other.d->ref();
    deref();
    d = other.d;
    return *this;
}

/*!
    Returns TRUE if the query is active and positioned on a valid
    record and the \a field is NULL; otherwise returns FALSE. Note
    that, for some drivers, isNull() will not return accurate
    information until after an attempt is made to retrieve data.

    \sa isActive() isValid() value()
*/

bool QSqlQuery::isNull( int field ) const
{
    if ( !d->sqlResult )
	return FALSE;
    if ( d->sqlResult->isActive() && d->sqlResult->isValid() )
	return d->sqlResult->isNull( field );
    return FALSE;
}

/*!
    Executes the SQL in \a query. Returns TRUE and sets the query
    state to active if the query was successful; otherwise returns
    FALSE and sets the query state to inactive. The \a query string
    must use syntax appropriate for the SQL database being queried,
    for example, standard SQL.

    After the query is executed, the query is positioned on an \e
    invalid record, and must be navigated to a valid record before
    data values can be retrieved, e.g. using next().

    Note that the last error for this query is reset when exec() is
    called.

    \sa isActive() isValid() next() prev() first() last() seek()
*/

bool QSqlQuery::exec ( const QString& query )
{
    if ( !d->sqlResult )
	return FALSE;
    d->sqlResult->setActive( FALSE );
    d->sqlResult->setLastError( QSqlError() );
    d->sqlResult->setAt( QSql::BeforeFirst );
    if ( !driver() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::exec: no driver" );
#endif
	return FALSE;
    }
    if ( d->count > 1 )
	*this = driver()->createQuery();
    d->sqlResult->setQuery( query.stripWhiteSpace() );
    if ( !driver()->isOpen() || driver()->isOpenError() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::exec: database not open" );
#endif
	return FALSE;
    }
    if ( query.isNull() || query.length() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::exec: empty query" );
#endif
	return FALSE;
    }
#ifdef QT_DEBUG_SQL
    qDebug( "\n QSqlQuery: " + query );
#endif
    return d->sqlResult->reset( query );
}

/*!
    Returns the value of the \a{i}-th field in the query (zero based).

    The fields are numbered from left to right using the text of the
    \c SELECT statement, e.g. in "SELECT forename, surname FROM
    people", field 0 is forename and field 1 is surname. Using
    \c{SELECT *} is not recommended because the order of the fields in
    the query is undefined.

    An invalid QVariant is returned if field \a i does not exist, if
    the query is inactive, or if the query is positioned on an invalid
    record.

    \sa prev() next() first() last() seek() isActive() isValid()
*/

QVariant QSqlQuery::value( int i ) const
{
    if ( !d->sqlResult )
	return QVariant();
    if ( isActive() && isValid() && ( i > QSql::BeforeFirst ) ) {
	return d->sqlResult->data( i );
    }
    return QVariant();
}

/*!
    Returns the current internal position of the query. The first
    record is at position zero. If the position is invalid, a
    QSql::Location will be returned indicating the invalid position.

    \sa isValid()
*/

int QSqlQuery::at() const
{
    if ( !d->sqlResult )
	return QSql::BeforeFirst;
    return d->sqlResult->at();
}

/*!
    Returns the text of the current query being used, or QString::null
    if there is no current query text.
*/

QString QSqlQuery::lastQuery() const
{
    if ( !d->sqlResult )
	return QString::null;
    return d->sqlResult->lastQuery();
}

/*!
    Returns the database driver associated with the query.
*/

const QSqlDriver* QSqlQuery::driver() const
{
    if ( !d->sqlResult )
	return 0;
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
    isSelect() must return TRUE before calling this function.

    If \a relative is FALSE (the default), the following rules apply:

    \list
    \i If \a i is negative, the result is positioned before the
    first record and FALSE is returned.
    \i Otherwise, an attempt is made to move to the record at position
    \a i. If the record at position \a i could not be retrieved, the
    result is positioned after the last record and FALSE is returned. If
    the record is successfully retrieved, TRUE is returned.
    \endlist

    If \a relative is TRUE, the following rules apply:

    \list
    \i If the result is currently positioned before the first
    record or on the first record, and \a i is negative, there is no
    change, and FALSE is returned.
    \i If the result is currently located after the last record, and
    \a i is positive, there is no change, and FALSE is returned.
    \i If the result is currently located somewhere in the middle,
    and the relative offset \a i moves the result below zero, the
    result is positioned before the first record and FALSE is
    returned.
    \i Otherwise, an attempt is made to move to the record \a i
    records ahead of the current record (or \a i records behind the
    current record if \a i is negative). If the record at offset \a i
    could not be retrieved, the result is positioned after the last
    record if \a i >= 0, (or before the first record if \a i is
    negative), and FALSE is returned. If the record is successfully
    retrieved, TRUE is returned.
    \endlist
*/
bool QSqlQuery::seek( int i, bool relative )
{
    if ( !isSelect() || !isActive() )
	return FALSE;
    beforeSeek();
    checkDetach();
    int actualIdx;
    if ( !relative ) { // arbitrary seek
	if ( i < 0 ) {
	    d->sqlResult->setAt( QSql::BeforeFirst );
	    afterSeek();
	    return FALSE;
	}
	actualIdx = i;
    } else {
	switch ( at() ) { // relative seek
	case QSql::BeforeFirst:
	    if ( i > 0 )
		actualIdx = i;
	    else {
		afterSeek();
		return FALSE;
	    }
	    break;
	case QSql::AfterLast:
	    if ( i < 0 ) {
		d->sqlResult->fetchLast();
		actualIdx = at() + i;
	    } else {
		afterSeek();
		return FALSE;
	    }
	    break;
	default:
	    if ( ( at() + i ) < 0  ) {
		d->sqlResult->setAt( QSql::BeforeFirst );
		afterSeek();
		return FALSE;
	    }
	    actualIdx = at() + i;
	    break;
	}
    }
    // let drivers optimize
    if ( isForwardOnly() && actualIdx < at() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query" );
#endif
	afterSeek();
	return FALSE;
    }
    if ( actualIdx == ( at() + 1 ) ) {
	if ( !d->sqlResult->fetchNext() ) {
	    d->sqlResult->setAt( QSql::AfterLast );
	    afterSeek();
	    return FALSE;
	}
	afterSeek();
	return TRUE;
    }
    if ( actualIdx == ( at() - 1 ) ) {
	if ( !d->sqlResult->fetchPrev() ) {
	    d->sqlResult->setAt( QSql::BeforeFirst );
	    afterSeek();
	    return FALSE;
	}
	afterSeek();
	return TRUE;
    }
    if ( !d->sqlResult->fetch( actualIdx ) ) {
	d->sqlResult->setAt( QSql::AfterLast );
	afterSeek();
	return FALSE;
    }
    afterSeek();
    return TRUE;
}

/*!
    Retrieves the next record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return TRUE before
    calling this function or it will do nothing and return FALSE.

    The following rules apply:

    \list
    \i If the result is currently located before the first
    record, e.g. immediately after a query is executed, an attempt is
    made to retrieve the first record.

    \i If the result is currently located after the last record,
    there is no change and FALSE is returned.

    \i If the result is located somewhere in the middle, an attempt
    is made to retrieve the next record.
    \endlist

    If the record could not be retrieved, the result is positioned after
    the last record and FALSE is returned. If the record is successfully
    retrieved, TRUE is returned.

    \sa at() isValid()
*/

bool QSqlQuery::next()
{
    if ( !isSelect() || !isActive() )
	return FALSE;
    beforeSeek();
    checkDetach();
    bool b = FALSE;
    switch ( at() ) {
    case QSql::BeforeFirst:
	b = d->sqlResult->fetchFirst();
	afterSeek();
	return b;
    case QSql::AfterLast:
	afterSeek();
	return FALSE;
    default:
	if ( !d->sqlResult->fetchNext() ) {
	    d->sqlResult->setAt( QSql::AfterLast );
	    afterSeek();
	    return FALSE;
	}
	afterSeek();
	return TRUE;
    }
}

/*!
    Retrieves the previous record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return TRUE before
    calling this function or it will do nothing and return FALSE.

    The following rules apply:

    \list
    \i If the result is currently located before the first record,
    there is no change and FALSE is returned.

    \i If the result is currently located after the last record, an
    attempt is made to retrieve the last record.

    \i If the result is somewhere in the middle, an attempt is made
    to retrieve the previous record.
    \endlist

    If the record could not be retrieved, the result is positioned
    before the first record and FALSE is returned. If the record is
    successfully retrieved, TRUE is returned.

    \sa at()
*/

bool QSqlQuery::prev()
{
    if ( !isSelect() || !isActive() )
	return FALSE;
    if ( isForwardOnly() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query" );
#endif
	return FALSE;
    }

    beforeSeek();
    checkDetach();
    bool b = FALSE;
    switch ( at() ) {
    case QSql::BeforeFirst:
	afterSeek();
	return FALSE;
    case QSql::AfterLast:
	b = d->sqlResult->fetchLast();
	afterSeek();
	return b;
    default:
	if ( !d->sqlResult->fetchPrev() ) {
	    d->sqlResult->setAt( QSql::BeforeFirst );
	    afterSeek();
	    return FALSE;
	}
	afterSeek();
	return TRUE;
    }
}

/*!
    Retrieves the first record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return TRUE before
    calling this function or it will do nothing and return FALSE.
    Returns TRUE if successful. If unsuccessful the query position is
    set to an invalid position and FALSE is returned.
*/

bool QSqlQuery::first()
{
    if ( !isSelect() || !isActive() )
	return FALSE;
    if ( isForwardOnly() && at() > QSql::BeforeFirst ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query" );
#endif
	return FALSE;
    }
    beforeSeek();
    checkDetach();
    bool b = FALSE;
    b = d->sqlResult->fetchFirst();
    afterSeek();
    return b;
}

/*!
    Retrieves the last record in the result, if available, and
    positions the query on the retrieved record. Note that the result
    must be in an active state and isSelect() must return TRUE before
    calling this function or it will do nothing and return FALSE.
    Returns TRUE if successful. If unsuccessful the query position is
    set to an invalid position and FALSE is returned.
*/

bool QSqlQuery::last()
{
    if ( !isSelect() || !isActive() )
	return FALSE;
    beforeSeek();
    checkDetach();
    bool b = FALSE;
    b = d->sqlResult->fetchLast();
    afterSeek();
    return b;
}

/*!
    Returns the size of the result, (number of rows returned), or -1
    if the size cannot be determined or the database does not support
    reporting information about query sizes. Note that for non-SELECT
    statements (isSelect() returns FALSE), size() will return -1. If
    the query is not active (isActive() returns FALSE), -1 is
    returned.

    To determine the number of rows affected by a non-SELECT
    statement, use numRowsAffected().

    \sa isActive() numRowsAffected() QSqlDriver::hasFeature()
*/
int QSqlQuery::size() const
{
    if ( !d->sqlResult )
	return -1;
    if ( isActive() && d->sqlResult->driver()->hasFeature( QSqlDriver::QuerySize ) )
	return d->sqlResult->size();
    return -1;
}

/*!
    Returns the number of rows affected by the result's SQL statement,
    or -1 if it cannot be determined. Note that for SELECT statements,
    this value will be the same as size(). If the query is not active
    (isActive() returns FALSE), -1 is returned.

    \sa size() QSqlDriver::hasFeature()
*/

int QSqlQuery::numRowsAffected() const
{
    if ( !d->sqlResult )
	return -1;
    if ( isActive() )
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
    if ( !d->sqlResult )
	return QSqlError();
    return d->sqlResult->lastError();
}

/*!
    Returns TRUE if the query is currently positioned on a valid
    record; otherwise returns FALSE.
*/

bool QSqlQuery::isValid() const
{
    if ( !d->sqlResult )
	return FALSE;
    return d->sqlResult->isValid();
}

/*!
    Returns TRUE if the query is currently active; otherwise returns
    FALSE.
*/

bool QSqlQuery::isActive() const
{
    if ( !d->sqlResult )
	return FALSE;
    return d->sqlResult->isActive();
}

/*!
    Returns TRUE if the current query is a \c SELECT statement;
    otherwise returns FALSE.
*/

bool QSqlQuery::isSelect() const
{
    if ( !d->sqlResult )
	return FALSE;
    return d->sqlResult->isSelect();
}

/*! Returns TRUE when you can only scroll forward through a result set
    otherwise FALSE
*/
bool QSqlQuery::isForwardOnly() const
{
    if ( !d->sqlResult )
	return FALSE;
    return d->sqlResult->isForwardOnly();
}

/*! Sets forward only mode to \a forward. If forward is TRUE only next() and
    seek() with positive values are allowed for navigating the results.
    Forward only mode needs far less memory since results do not have to be cached.
    Forward only mode is off by default.
    Note that it is not possible to use forward only mode with data aware widgets
    like QDataTable since they need to be able to scroll backward.
    \sa next(), seek()
*/
void QSqlQuery::setForwardOnly( bool forward )
{
    if ( d->sqlResult ) 
	d->sqlResult->setForwardOnly( forward );
}

/*!
  \internal
*/

void QSqlQuery::deref()
{
    if ( d->deref() ) {
	delete d;
	d = 0;
    }
}

/*!
  \internal
*/

bool QSqlQuery::checkDetach()
{
    if ( d->count > 1 && d->sqlResult ) {
	QString sql = d->sqlResult->lastQuery();
	*this = driver()->createQuery();
	exec( sql );
	return TRUE;
    }
    return FALSE;
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

// XXX: Hack to keep BCI - remove in 4.0. QSqlExtension should be
// removed, and the prepare(), exec() etc. fu's should be
// made virtual members of QSqlQuery/QSqlResult

/*!
    Prepares the SQL query \a query for execution. The query may
    contain placeholders for binding values. Note that placeholder
    markers are usually database dependent.

    \sa exec(), bindValue(), addBindValue()
*/
bool QSqlQuery::prepare( const QString& query )
{
    if ( !d->sqlResult || !d->sqlResult->extension() )
	return FALSE;
    d->sqlResult->setActive( FALSE );
    d->sqlResult->setLastError( QSqlError() );
    d->sqlResult->setAt( QSql::BeforeFirst );
    if ( !driver() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::prepare: no driver" );
#endif
	return FALSE;
    }
    if ( d->count > 1 )
	*this = driver()->createQuery();
    d->sqlResult->setQuery( query.stripWhiteSpace() );
    if ( !driver()->isOpen() || driver()->isOpenError() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::prepare: database not open" );
#endif
	return FALSE;
    }
    if ( query.isNull() || query.length() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlQuery::prepare: empty query" );
#endif
	return FALSE;
    }
#ifdef QT_DEBUG_SQL
    qDebug( "\n QSqlQuery: " + query );
#endif
    d->sqlResult->extension()->clearIndex();
    if ( driver()->hasFeature( QSqlDriver::PreparedQueries ) ) {
	// below we substitute Oracle placeholders with ODBC ones and
	// vice versa to make this db independant
	QString q = query;
	int i = 0, cnt = 0;
	if ( driver()->hasFeature( QSqlDriver::NamedPlaceholders ) ) {
	    QRegExp rx("'[^']*'|\\?");
	    while ( (i = rx.search( q, i )) != -1 ) {
		if ( rx.cap(0) == "?" ) {
		    q = q.replace( i, 1, ":f" + QString::number(cnt) );
		    cnt++;
		}
		i += rx.matchedLength();
	    }
	} else if ( driver()->hasFeature( QSqlDriver::PositionalPlaceholders ) ) {
	    QRegExp rx("'[^']*'|:([a-zA-Z0-9_]+)");
	    while ( (i = rx.search( q, i )) != -1 ) {
		if ( rx.cap(1).isEmpty() ) {
		    i += rx.matchedLength();
		} else {
		    // record the index of the placeholder - needed
		    // for emulating named bindings with ODBC
		    d->sqlResult->extension()->index[ cnt ]= rx.cap(0);
		    q = q.replace( i, rx.matchedLength(), "?" );
		    i++;
		    cnt++;
		}
	    }
	}
	return d->sqlResult->extension()->prepare( q );
    } else {
	return TRUE; // fake prepares should always succeed
    }
}

/*!
    \overload

    Executes a previously prepared SQL query. Returns TRUE if the
    query is executed successfully; otherwise returns FALSE.

    \sa prepare(), bindValue(), addBindValue()
*/
bool QSqlQuery::exec()
{
    bool ret;
    if ( !d->sqlResult || !d->sqlResult->extension() )
	return FALSE;
    if ( driver()->hasFeature( QSqlDriver::PreparedQueries ) ) {
	ret = d->sqlResult->extension()->exec();
    } else {
	// fake preparation - just replace the placeholders..
	QString query = d->sqlResult->lastQuery();
	if ( d->sqlResult->extension()->bindMethod() == QSqlExtension::BindByName ) {
	    QMap<QString, QVariant>::ConstIterator it;
	    for ( it = d->sqlResult->extension()->values.begin();
		  it != d->sqlResult->extension()->values.end(); ++it ) {
		QSqlField f( "", it.data().type() );
		if ( it.data().isNull() )
		    f.setNull();
		else
		    f.setValue( it.data() );
		query = query.replace( it.key(), driver()->formatValue( &f ) ); 
	    }
	} else {
	    QMap<int, QString>::ConstIterator it;
	    QString val;
	    int i = 0;
	    for ( it = d->sqlResult->extension()->index.begin();
		  it != d->sqlResult->extension()->index.end(); ++it ) {
		i = query.find( '?', i );
		if ( i > -1 ) {
		    QSqlField f( "", d->sqlResult->extension()->values[ it.data() ].type() );
		    if ( d->sqlResult->extension()->values[ it.data() ].isNull() )
			f.setNull();
		    else
			f.setValue( d->sqlResult->extension()->values[ it.data() ] );
		    val = driver()->formatValue( &f );
		    query = query.replace( i, 1, driver()->formatValue( &f ) );
		    i += val.length();
		}
	    }
	}
	// have to retain the original query w/placeholders..
	QString orig = d->sqlResult->lastQuery();
	ret = exec( query );
	d->sqlResult->setQuery( orig );
    }
    d->sqlResult->extension()->clearValues(); // no need for the placeholder values anymore
    return ret;
}

/*!
    Set the placeholder \a placeholder to be bound to value \a val in
    the prepared statement. Note that the placeholder mark (e.g \c{:})
    should be included when specifying the placeholder name.
    Placeholder values are cleared after the query has been executed.

    \sa addBindValue(), prepare(), exec()
*/
void QSqlQuery::bindValue( const QString& placeholder, const QVariant& val )
{
    if ( !d->sqlResult || !d->sqlResult->extension() )
	return;
    d->sqlResult->extension()->bindValue( placeholder, val );
}

/*!
    \overload

    Set the placeholder in position \a pos to be bound to value \a val
    in the prepared statement. Field numbering starts at 0.
    Placeholder values are cleared after the query has been executed.

    \sa addBindValue(), prepare(), exec()
*/
void QSqlQuery::bindValue( int pos, const QVariant& val )
{
    if ( !d->sqlResult || !d->sqlResult->extension() )
	return;
    d->sqlResult->extension()->bindValue( pos, val );
}

/*!
    Adds the value \a val to the list of values when using positional
    value binding. The order of the addBindValue() calls determines
    which placeholder a value will be bound to in the prepared query.
    Placeholder values are cleared after the query has been executed.

    \sa bindValue(), prepare(), exec()
*/
void QSqlQuery::addBindValue( const QVariant& val )
{
    if ( !d->sqlResult || !d->sqlResult->extension() )
	return;
    d->sqlResult->extension()->addBindValue( val );
}
#endif // QT_NO_SQL

/****************************************************************************
**
** Implementation of QSqlResult class
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

#include "qsqlresult.h"

#ifndef QT_NO_SQL

class QSqlResultPrivate
{
public:
    const QSqlDriver*   sqldriver;
    int             idx;
    QString         sql;
    bool            active;
    bool            isSel;
    QSqlError	    error;
};

/*! \class QSqlResult qsqlresult.h

  \brief QSqlResult provides an abstract interface for accessing data from SQL databases

  \module sql

  QSqlResult provides an abstract interface for accessing data from SQL databases.
  Normally you would use QSqlQuery instead of QSqlResult since QSqlQuery
  provides a generic wrapper for database-specific implementations of
  QSqlResult.

  \sa QSql

*/


/*! Protected constructor which creates a QSqlResult using database \a
    db.  The object is initialized to an inactive state.

*/

QSqlResult::QSqlResult( const QSqlDriver * db )
{
    d = new QSqlResultPrivate();
    d->sqldriver = db;
    d->idx = BeforeFirst;
    d->isSel = FALSE;
    d->active = FALSE;
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlResult::~QSqlResult()
{
    delete d;
}

/*!  Sets the current query for the result to \a query.  The result
must be reset() in order to execute the query on the database.
*/

void QSqlResult::setQuery( const QString& query )
{
    d->sql = query;
}

/*!  Returns the current SQL query text, or QString::null if there is none.
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


/*! Returns TRUE if the result is positioned on a valid record (that
    is, the result is not positioned before the first or after the
    last record); otherwise returns FALSE.

*/

bool QSqlResult::isValid() const
{
    return ( d->idx != BeforeFirst && \
	    d->idx != AfterLast ) ? TRUE : FALSE;
}

/*! \fn bool QSqlResult::isNull( int i )
  Returns TRUE if the field at position \a i is null, otherwise returns FALSE.
*/


/*! Returns TRUE if the result has records to be retrieved, otherwise
 returns FALSE.

*/

bool QSqlResult::isActive() const
{
    return d->active;
}

/*! Protected function provided for derived classes to set the internal
    (zero-based) result index to \a at.

    \sa at()

*/

void QSqlResult::setAt( int at )
{
    d->idx = at;
}


/*!  Protected function provided for derived classes to indicate whether
  or not the current statement is an SQL SELECT statement.
*/

void QSqlResult::setSelect( bool s )
{
    d->isSel = s;
}

/*!  Returns TRUE if the current result is from a SELECT statement,
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


/*! Protected function provided for derived classes to set the internal
    active state to the value of \a a.

  \sa isActive()

*/

void QSqlResult::setActive( bool a )
{
    d->active = a;
}

/*! Protected function provided for derived classes to set the last
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

/*! \fn int QSqlResult::size()
    Returns the size of the result or -1 if it cannot be determined.
*/

/*! \fn int QSqlResult::numRowsAffected()

  Returns the number of rows affected by the last query executed.
*/

/*! \fn QVariant QSqlResult::data( int i )

    Returns the data for field \a i (zero-based) as a QVariant.  This
    function is only called if the result is in an active state and is
    positioned on a valid record and the field \a i is non-negative.
    Derived classes must reimplement this function and return the value
    of field \a i, or QVariant() if it cannot be determined.

*/

/*! \fn  bool QSqlResult::reset ( const QString& query )

    Sets the result to use the SQL statement \a query for subsequent
    data retrieval.  Derived classes must reimplement this function and
    apply the \a query to the database.  This function is called only
    after the result is set to an inactive state and is positioned
    before the first record of the new result.  Derived classes should
    return TRUE if the query was successful and ready to be used,
    FALSE otherwise.

*/

/*! \fn bool QSqlResult::fetch( int i )

    Positions the result to an arbitrary (zero-based) index \a i.  This
    function is only called if the result is in an active state.  Derived
    classes must reimplement this function and position the result to the
    index \a i, and call setAt() with an appropriate value.  Return TRUE
    to indicate success, FALSE for failure.

*/

/*! \fn bool QSqlResult::fetchFirst()

    Positions the result to the first record in the result.  This
    function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the result
    to the first record, and call setAt() with an appropriate value.
    Return TRUE to indicate success, FALSE for failure.

*/

/*! \fn bool QSqlResult::fetchLast()

    Positions the result to the last record in the result.  This
    function is only called if the result is in an active state.
    Derived classes must reimplement this function and position the result
    to the last record, and call setAt() with an appropriate value.
    Return TRUE to indicate success, FALSE for failure.

*/

/*! Positions the result to the next available record in the result.
    This function is only called if the result is in an active state.
    The default implementation calls fetch() with the next index.
    Derived classes can reimplement this function and position the result
    to the next record in some other way, and call setAt() with an
    appropriate value.  Return TRUE to indicate success, FALSE for
    failure.

*/

bool QSqlResult::fetchNext()
{
    return fetch( at() + 1 );
}

/*! Positions the result to the previous available record in the
    result.  This function is only called if the result is in an active
    state.  The default implementation calls fetch() with the previous
    index.  Derived classes can reimplement this function and position the
    result to the next record in some other way, and call setAt() with
    an appropriate value.  Return TRUE to indicate success, FALSE for
    failure.

*/

bool QSqlResult::fetchPrev()
{
    return fetch( at() - 1 );
}

#endif // QT_NO_SQL

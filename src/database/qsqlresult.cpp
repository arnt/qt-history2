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

  \brief QSqlResult provides an abstract interface for accessing data via SQL

  \module database

  QSqlResult provides an abstract interface for accessing data via SQL.  Normally, this
  class is not used directly.  Rather, QSql should be used instead,  as a wrapper for database-specific
  implementations of QSqlResult.

  \sa QSql

*/


/*! Protected constructor which creates a QSqlResult using database \a db.  The object
    is initialized to an inactive state.

*/

QSqlResult::QSqlResult( const QSqlDriver * db )
{
    d = new QSqlResultPrivate();
    d->sqldriver = db;
    d->idx = QSqlResult::BeforeFirst;
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

/*!  Returns the current SQL query, or QString::null if there is none.
*/

QString QSqlResult::query() const
{
    return d->sql;
}

/*!
  Returns the current position of the result.

*/

int QSqlResult::at() const
{
    return d->idx;
}


/*! Returns TRUE if the result is positioned on a valid record (that is, the
    result is not positioned before the first or after the last record).

*/

bool QSqlResult::isValid() const
{
    return ( d->idx != BeforeFirst && \
	    d->idx != AfterLast ) ? TRUE : FALSE;
}

/*! Returns TRUE if the result has records to be retrieved.

*/

bool QSqlResult::isActive() const
{
    return d->active;
}

/*! Protected method provided for derived classes to set the internal result
    index to \a at.

    \sa at()

*/

void QSqlResult::setAt( int at )
{
    d->idx = at;
}


/*!  Protected method provided for derived classes to indicate whether
  or not the current statement is a SQL SELECT statement.
*/

void QSqlResult::setSelect( bool s )
{
    d->isSel = s;
}

/*!  Returns TRUE if the current result is from a SELECT statement,
  otherwise FALSE is returned.
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


/*! Protected method provided for derived classes to set the internal active
    state to the value of \a a.

  \sa isActive()

*/

void QSqlResult::setActive( bool a )
{
    d->active = a;
}

/*! Protected method provided for derived classes to set the last error
    to the value of \a e.

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


/*! \fn  QSqlFieldList   fields() const = 0;
    Returns a list of fields used in the result.

*/

/*! \fn int size() const = 0;
    Returns the size of the result.
*/

/*! \fn int affectedRows() const;
    Returns the number of affected rows in the result.
*/

/*! \fn  QVariant QSqlResult::data( int i )

    Returns the data for field \a i (zero-based) as a QVariant.  This method is only called if the
    result is in an active state and is positioned on a valid record and the field
    \a i is non-negative.  Derived classes must reimplement this method and return
    the value of field \a i, or QVariant() if it cannot be determined.

*/

/*! \fn  bool QSqlResult::reset ( const QString& query )

    Sets the result to use the SQL statement \a query for subsequent data retrieval.
    Derived classes must reimplement this method and apply the \a query to the database.
    This method is called only after the result is set to an inactive state and is
    positioned before the first record of the new result.  Derived classes should return
    TRUE if the query was successful and ready to be used, FALSE otherwise.

*/

/*! \fn bool QSqlResult::fetch( int i )

    Positions the result to a random index \a i.  This method is only called
    if the result is in an active state.  Derived classes must override this
    method and position the result to the index \a i, and call setAt() with
    an appropriate value.    Return TRUE to indicate success, FALSE for failure.

*/

/*! \fn bool QsqlResult::fetchFirst()

    Positions the result to the first record in the result.  This methods is only
    called if the result is in an active state.  Derived classes must override this
    method and position the result to the first record, and call setAt() with an
    appropriate value.  Return TRUE to indicate success, FALSE for failure.

*/

/*! \fn bool QsqlResult::fetchLast()

    Positions the result to the last record in the result.  This methods is only
    called if the result is in an active state.  Derived classes must override this
    method and position the result to the last record, and call setAt() with an
    appropriate value.  Return TRUE to indicate success, FALSE for failure.

*/

/*! Positions the result to the next available record in the result.  This methods is only
    called if the result is in an active state.  The default implementation calls fetch() with
    the next index.  Derived classes can override this method and position the result to the next
    record in some other way, and call setAt() with an appropriate value.  Return TRUE to indicate success,
    FALSE for failure.

*/

bool QSqlResult::fetchNext()
{
    return fetch( at() + 1 );
}

/*! Positions the result to the previous available record in the result.  This methods is only
    called if the result is in an active state.  The default implementation calls fetch() with
    the previous index.  Derived classes can override this method and position the result to the next
    record in some other way, and call setAt() with an appropriate value.  Return TRUE to indicate success,
    FALSE for failure.

*/

bool QSqlResult::fetchPrevious()
{
    return fetch( at() - 1 );
}

#endif // QT_NO_SQL













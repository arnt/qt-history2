#include "qsqldriver.h"
#include "qdatetime.h"

#ifndef QT_NO_SQL

// database states
#define DBState_Open	    	0x0001
#define DBState_OpenError   	0x0002

/*!
  \class QSqlDriver qsqldriver.h
  \brief Class used for accessing databases

  \module database

  This is an abstract base class which defines an interface for accessing SQL databases.  This
  class should not be used directly.  Use QSqlDatabase instead.

*/

/*!  Default constructor.

*/

QSqlDriver::QSqlDriver( QObject * parent, const char * name )
: QObject(parent, name),
  dbState(0),
  hasTrans(FALSE),
  hasQuerySize(FALSE),
  error()
{
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlDriver::~QSqlDriver()
{
}

/*! \fn bool QSqlDriver::open( const QString& db, const QString& user, const QString& password, const QString& host )
    Derived classes must override this abstract virtual method in order to open the database.
    Return TRUE on success, FALSE on failure.

    \sa setOpen()

*/

/*! \fn bool QSqlDriver::close()
    Derived classes must override this abstract virtual method in order to close the database.
    Return TRUE on success, FALSE on failure.

    \sa setOpen()

*/

/*! \fn QSqlQuery QSqlDriver::createQuery() const
    Creates an empty SQL result on the database.  Derived classes must override this method
    and return a QSqlQuery object appropriate for their database to the caller.

*/

//void QSqlDriver::destroyResult( QSqlResult* r ) const
//{
//    if ( r )
//    	delete r;
//}

/*!  Returns TRUE if the database state is open, FALSE otherwise.

*/

bool QSqlDriver::isOpen() const
{
    return ((dbState & DBState_Open) == DBState_Open);
}

/*!  Returns TRUE if the there was an error opening the database, FALSE otherwise.

*/

bool QSqlDriver::isOpenError() const
{
    return ((dbState & DBState_OpenError) == DBState_OpenError);
}

/*! Returns TRUE if the database supports transactions, FALSE otherwise.
    Note that some databases need to be open() before this can be determined.
    The default implementation returns FALSE.

    \sa setTransactionSupport()

*/

bool QSqlDriver::hasTransactionSupport() const
{
    return hasTrans;
}

/*! Returns TRUE if the database supports reporting information about
    the size of a query, FALSE otherwise.  Note that some databases do
    not support returning the size (in number of rows returned) of a
    query, so therefore QSql::size() will return -1.  The default
    implementation returns FALSE.

    \sa setQuerySizeSupport()

*/

bool QSqlDriver::hasQuerySizeSupport() const
{
    return hasQuerySize;
}

/*! Protected method which sets the open state of the database to \a o.
    Derived classes can use this method to report the status of open().

    \sa open(), setOpenError()

*/

void QSqlDriver::setOpen( bool o )
{
    if ( o )
	dbState |= DBState_Open;
    else
	dbState &= ~DBState_Open;
}

/*! Protected method which sets the open error state of the database to \a e.
    Derived classes can use this method to report the status of open().
    Note that if \a e is TRUE the open state of the database is set to closed
    (i.e., isOpen() returns FALSE).

    \sa open(), setOpenError()

*/

void QSqlDriver::setOpenError( bool e )
{
    if ( e ) {
	dbState |= DBState_OpenError;
	dbState &= ~DBState_Open;
    }
    else
	dbState &= ~DBState_OpenError;
}

/*! Protected method which allows derived classed to set the transaction support of
    the database to \a t.

    \sa transaction(), commit(), rollback()

*/

void QSqlDriver::setTransactionSupport( bool t )
{
    hasTrans = t;
}

/*! Protected method which allows derived classed to set the query size  support of
    the database to \a s.

    \sa hasQuerySizeSupport()

*/

void QSqlDriver::setQuerySizeSupport( bool s )
{
    hasQuerySize = s;
}


/*! Protected method which derived classes can override to begin a transaction.
    If successful, return TRUE, otherwise return FALSE.  The default implementation returns FALSE.

    \sa setTransactionSupport(), transaction(), commit(), rollback()

*/

bool QSqlDriver::beginTransaction()
{
    return FALSE;
}

/*! Protected method which derived classes can override to commit a transaction.
    If successful, return TRUE, otherwise return FALSE. The default implementation returns FALSE.

    \sa setTransactionSupport(), transaction(), commit(), rollback()

*/

bool QSqlDriver::commitTransaction()
{
    return FALSE;
}

/*! Protected method which derived classes can override to rollback a transaction.
    If successful, return TRUE, otherwise return FALSE.  The default implementation returns FALSE.

    \sa setTransactionSupport(), transaction(), commit(), rollback()

*/

bool QSqlDriver::rollbackTransaction()
{
    return FALSE;
}

/*! Protected method which allows derived classes to set the value of the last error that occurred
    on the database.

    \sa lastError()

*/

void QSqlDriver::setLastError( const QSqlError& e )
{
    error = e;
}

/*! Returns a QSqlError object which contains information about the last error that occurred on the
    database.

*/

QSqlError QSqlDriver::lastError() const
{
    return error;
}

/*!
  Returns a list of tables in the database.  The default
  implementation returns an empty list.
*/

QStringList QSqlDriver::tables( const QString&  ) const
{
    return QStringList();
}

/*!
  Returns the primary index for table \a tablename.  If no
  such index exists, the QSqlIndex that is returned will be
  empty.  The default implementation returns an empty index.

*/

QSqlIndex QSqlDriver::primaryIndex( const QString&  ) const
{
    return QSqlIndex();
}


/*!
  Returns a list of fields for table \a tablename.  If no
  such table exists, an empty list is returned.  The default
  implementation returns an empty record.

*/

QSqlRecord QSqlDriver::record( const QString&  ) const
{
    return QSqlRecord();
}

/*!  Returns a list of fields for the SQL \a query. The default
implementation returns an empty record.

*/

QSqlRecord QSqlDriver::record( const QSqlQuery& ) const
{
   return QSqlRecord();
}

/*!  Returns a string representation of the 'NULL' value for the
  database.  This is used, for example, when constructing INSERT and
  UPDATE statements.  The default implementation returns 'NULL'.

*/

QString QSqlDriver::nullText() const
{
    return "NULL";
}

/*!  Returns a string representation of the \a field value for the
  database.  This is used, for example, when constructing INSERT and
  UPDATE statements.  The default implementation returns the value
  formatted as a string.  If \a field is character data, the value is
  returned enclosed by single quotation marks, which is appropriate
  for many SQL databases.

  \sa QVariant::toString().

*/
QString QSqlDriver::formatValue( const QSqlField* field ) const
{
    QString r;
    if ( field->isNull() )
	r = nullText();
    else {
	switch ( field->type() ) {
	case QVariant::Date:
	    r = "'" + field->value().toDate().toString( Qt::ISODate ) + "'";
	    break;
	case QVariant::Time:
	    r = "'" + field->value().toTime().toString( Qt::ISODate ) + "'";
	    break;
	case QVariant::DateTime:
	    r = "'" + field->value().toDateTime().toString( Qt::ISODate ) + "'";
	    break;
	case QVariant::String:
	case QVariant::CString:
	    r = "'" + field->value().toString() + "'";
	    break;
	default:
	    r = field->value().toString();
	    break;
	}
    }
    return r;
}

#endif // QT_NO_SQL











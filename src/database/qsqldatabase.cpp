#include "qsqldatabase.h"
#ifndef QT_NO_SQL

#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldriverplugin.h"
#include <stdlib.h> //### for getenv, get rid of this soon!

class QNullResult : public QSqlResult
{
public:
    QNullResult(const QSqlDriver* d): QSqlResult(d){}
    ~QNullResult(){}
protected:
    QVariant    data( int i ) { return QVariant();Q_UNUSED(i) }
    bool	reset ( const QString& sqlquery ) { QString s(sqlquery); return FALSE; }
    bool	fetch( int i ) { i = i; return FALSE; }
    bool	fetchFirst() { return FALSE; }
    bool	fetchLast() { return FALSE; }
    bool	isNull( int i ) const {return FALSE;Q_UNUSED(i);}
    QSqlFieldList   fields() {return QSqlFieldList();}
    int             size() const {return 0;}
    int             affectedRows() const {return 0;}
};

class QNullDriver : public QSqlDriver
{
public:
    QNullDriver(): QSqlDriver(){}
    ~QNullDriver(){}
    bool    open( const QString & db,
    			const QString & user,
			const QString & password,
			const QString & host ) {
				Q_CONST_UNUSED(db);
				Q_CONST_UNUSED(user);
				Q_CONST_UNUSED(password);
				Q_CONST_UNUSED(host);
				return FALSE;
			}
    void    close() {}
    QSql    createResult() const { return QSql( new QNullResult(this) ); }
};

class QSqlDatabasePrivate
{
public:
    QSqlDatabasePrivate(): driver(0), plugIns(0) {}
    ~QSqlDatabasePrivate()
    {
    }
    QSqlDriver* driver;
    QSqlDriverPlugInManager* plugIns;
    QString dbname;
    QString uname;
    QString pword;
    QString hname;
};

/*!
    \class QSqlDatabase qsql.h
    \brief Class used for accessing SQL databases

    \module database

     This class is used to access SQL databases.  QSqlDatabase provides an abstract
     interface for accessing many types of backends.

     Database-specific drivers are used internally to actually access and manipulate data. (see QSqlDriver)
     Result set objects  provide the interface for executing and manipulating SQL queries ( see QSql ).

*/

/*!  Creates a QSqlDatabase that uses the driver described by \a type.  If the
     \a type is not recognized, the database will have no functionality.

     Available types are:

     <ul>
     <li>QODBC - ODBC (Open Database Connectivity) Driver
     <li>QOCI - Oracle Call Interface Driver
     <li>QPSQL - PostgreSQL Driver
     <li>QMYSQL - MySQL Driver
     </ul>
*/
QSqlDatabase::QSqlDatabase( const QString& type, QObject * parent, const char * name)
: QObject(parent, name)
{
    init( type );
}

/*!  Equivalent to the above constructor.  In addition, it resets the database with new
     connection parameters.

     \sa reset()
 */
QSqlDatabase::QSqlDatabase( const QString& type,
    			const QString & db,
    			const QString & user,
			const QString & password,
			const QString & host,
			QObject * parent,
			const char * name )
: QObject(parent, name)
{
    init( type );
    reset( db, user, password, host );
}

/*!
  \internal
*/
void QSqlDatabase::init( const QString& type )
{
    d = new QSqlDatabasePrivate();
    d->plugIns = new QSqlDriverPlugInManager( QString((char*)getenv( "QTDIR" )) + "/lib" ); // ### make this better
    QPlugIn* pi = d->plugIns->plugIn( type );
    //    QPlugIn* pi;
    //    if ( d->plugIns->selectFeature( type ) )
    //	pi = d->plugIns->plugIn( type );
    if ( pi && pi->queryPlugInInterface() == "QSqlDriverInterface" )
     	d->driver = ((QSqlDriverPlugIn*)pi)->create( type );
    if ( !d->driver ) {
#ifdef CHECK_RANGE
	qWarning("QSqlDatabase warning: driver not loaded");
#endif
	d->driver = new QNullDriver();
    }
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlDatabase::~QSqlDatabase()
{
    delete d->driver;
    delete d->plugIns;
    delete d;
}

/*! Sends the query \a sqlquery to the database and returns a QSql
    object for accessing the result data.

    \sa QSql
*/

QSql QSqlDatabase::query( const QString & sqlquery ) const
{
    return d->driver->query( sqlquery );
}

/*! Executes an SQL statement (i.e., INSERT, UPDATE, DELETE statement) on the database,
    and returns the number of affected rows.
    \sa query(), createResult()
*/

int QSqlDatabase::exec( const QString & sql ) const
{
    QSql r = d->driver->createResult();
    r << sql;
    return r.affectedRows();
}

/*! Opens the database using the connection values which were passed to reset().
    Returns TRUE on success, and FALSE if there was an error.  Error
    information can be retrieved using the lastError() method.

    \sa lastError()
*/

bool QSqlDatabase::open()
{
    return d->driver->open( d->dbname,
				d->uname,
				d->pword,
				d->hname);
}

/*! Closes the database, freeing any resources aquired.

*/

void QSqlDatabase::close()
{
    d->driver->close();
}

/*! Creates an uninitialized QSql result object which can be used to send
    queries to the database.

*/

QSql QSqlDatabase::createResult() const
{
    return d->driver->createResult();
}

/*! Returns TRUE if the database is currently opened, otherwise FALSE is returned.

*/

bool QSqlDatabase::isOpen() const
{
    return d->driver->isOpen();
}

/*! Return TRUE if there was an error opening the database, otherwise FALSE is returned.
    Error information can be retrieved using the lastError() method.

*/

bool QSqlDatabase::isOpenError() const
{
    return d->driver->isOpenError();
}

/*! Returns TRUE if the database has support for transactions, otherwise FALSE is
    returned.

*/

bool QSqlDatabase::hasTransactionSupport() const
{
    return d->driver->hasTransactionSupport();
}

/*! Closes the database and resets it to use new connection values.  \a db specifies the
    name of the database, \a user specifies the user name connecting to the database, \a password
    specifies the user's database password and \a host specifies the host on which the
    database is running.

*/

void QSqlDatabase::reset( const QString & db,
     		 const QString & user,
     		 const QString & password,
     		 const QString & host )
{
    d->dbname = db;
    d->uname = user;
    d->pword = password;
    d->hname = host;
    //    d->driver->close();
}

/*! Begins a transaction on the database if the driver supports transactions.
    Returns TRUE if the operation succeeded, FALSE otherwise.

    \sa hasTransactionSupport() commit() rollback()
*/

bool QSqlDatabase::transaction( )
{
   if ( !d->driver->hasTransactionSupport() )
	return FALSE;
    return d->driver->beginTransaction();
}

/*! Commits a transaction to the database if the driver supports transactions.
    Returns TRUE if the operation succeeded, FALSE otherwise.

    \sa hasTransactionSupport() rollback()
*/

bool QSqlDatabase::commit()
{
    if ( !d->driver->hasTransactionSupport() )
	return FALSE;
    return d->driver->commitTransaction();
}

/*! Rolls a transaction back on the database if the driver supports transactions.
    Returns TRUE if the operation succeeded, FALSE otherwise.

    \sa hasTransactionSupport() commit() transaction()
*/

bool QSqlDatabase::rollback()
{
    if ( !d->driver->hasTransactionSupport() )
	return FALSE;
    return d->driver->rollbackTransaction();
}

/*! Returns the name of the database, or QString::null if a name has not been set.

*/

QString QSqlDatabase::databaseName() const
{
    return d->dbname;
}

/*! Returns the database user name, or QString::null if a name has not been set.

*/

QString QSqlDatabase::userName() const
{
    return d->uname;
}

/*! Returns the database user password, or QString::null if one has not been set.

*/

QString QSqlDatabase::password() const
{
    return d->pword;
}

/*! Returns the database host name, or QString::null if one has not been set.

*/

QString QSqlDatabase::hostName() const
{
    return d->hname;
}

/*! Returns a pointer to the database driver used to access the database.

*/

QSqlDriver* QSqlDatabase::driver() const
{
    return d->driver;
}

/*! Returns information about the last error that occurred on the database.  See
    QSqlError for more information.

*/

QSqlError QSqlDatabase::lastError() const
{
    return d->driver->lastError();
}


/*!
  Returns a list of tables in the database.

*/

QStringList QSqlDatabase::tables() const
{
    return d->driver->tables( userName() );
}

/*!
  Returns the primary index for table \a tablename.  If no
  such index exists, the QSqlIndex that is returned will be
  empty.

*/

QSqlIndex QSqlDatabase::primaryIndex( const QString& tablename ) const
{
    return d->driver->primaryIndex( tablename );
}


/*!
  Returns a list of fields for table \a tablename.  If not such
  table exists, an empty list is returned.

*/

QSqlFieldList QSqlDatabase::fields( const QString& tablename ) const
{
    return d->driver->fields( tablename );
}

#endif // QT_NO_SQL


#include "qsqldatabase.h"
#ifndef QT_NO_SQL

#ifdef QT_SQL_POSTGRES
#include "src/psql/qsql_psql.h"
#endif
#ifdef QT_SQL_MYSQL
#include "src/mysql/qsql_mysql.h"
#endif
#ifdef QT_SQL_ODBC
#include "src/odbc/qsql_odbc.h"
#endif
#ifdef QT_SQL_OCI
#include "src/oci/qsql_oci.h"
#endif

#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldriverplugin.h"
#include "qobject.h"
#include "qdict.h"
#include "qapplication.h"
#include <stdlib.h>

QT_STATIC_CONST_IMPL char * const QSqlDatabase::defaultDatabase = "qt_sql_default_database";

class QSqlDatabaseManager : public QObject
{
public:
    static QSqlDatabase* database( const QString& name );
    static QSqlDatabase* addDatabase( QSqlDatabase* db, const QString & name );
    static void          removeDatabase( const QString& name );

protected:
    static QSqlDatabaseManager* instance();
    QSqlDatabaseManager( QObject* parent=0, const char* name=0 );
    ~QSqlDatabaseManager();
    QDict< QSqlDatabase > dbDict;
};

/*!  Constructs an SQL database manager.

*/

QSqlDatabaseManager::QSqlDatabaseManager( QObject* parent, const char* name )
    : QObject( parent, name ), dbDict( 1 )
{
}

/*!
  Destroys the object and frees any allocated resources.  All open
  databases are closed.  All databases are deleted.

*/

QSqlDatabaseManager::~QSqlDatabaseManager()
{
    QDictIterator< QSqlDatabase > it( dbDict );
    while ( it.current() ) {
	it.current()->close();
	++it;
    }
}

/*!
  \internal

*/

QSqlDatabaseManager* QSqlDatabaseManager::instance()
{
    static QSqlDatabaseManager* sqlConnection = 0;
    // ### use cleanup handler
    if ( !sqlConnection ) {
#ifdef QT_CHECK_RANGE
	if ( !qApp )
	    qWarning("Warning: creating QSqlDatabaseManager with no parent." );
#endif
	sqlConnection = new QSqlDatabaseManager( qApp, "qt_qsqldatabasemanager_instance" );
    }
    return sqlConnection;
}

/*!
  Returns a pointer to the database with name \a name.  If the database was not previously
  opened, it is opened now.  If \name does not exist in the list of managed database,
  0 is returned.

*/

QSqlDatabase* QSqlDatabaseManager::database( const QString& name )
{
    QSqlDatabaseManager* sqlConnection = instance();
    QSqlDatabase* db = sqlConnection->dbDict.find( name );
#ifdef QT_CHECK_RANGE
    if ( !db )
	qWarning("Warning: QSqlDatabaseManager unable to find database " + name );
#endif
    if ( db && !db->isOpen() ) {
	db->open();
#ifdef QT_CHECK_RANGE
	if ( !db->isOpen() )
	    qWarning("Warning: QSqlDatabaseManager unable to open database: " + db->lastError().databaseText() + ": " + db->lastError().driverText() );
#endif
    }
    return db;
}



/*!
  Adds a database to the SQL connection manager.  The database is
  referred to by \name.  A pointer to the newly added database is
  returned.

  \sa QSqlDatabase database()

*/

QSqlDatabase* QSqlDatabaseManager::addDatabase( QSqlDatabase* db, const QString & name )
{
    QSqlDatabaseManager* sqlConnection = instance();
    sqlConnection->dbDict.insert( name, db );
    return db;
}


/*!
  Removes the database \a name from the SQL connection manager.  Note that
  there should be no open queries on the database when this method is called,
  otherwise resources will be leaked.

*/

void QSqlDatabaseManager::removeDatabase( const QString& name )
{
    QSqlDatabaseManager* sqlConnection = instance();
    sqlConnection->dbDict.setAutoDelete( TRUE );
    sqlConnection->dbDict.remove( name );
    sqlConnection->dbDict.setAutoDelete( FALSE );
}

//

class QNullResult : public QSqlResult
{
public:
    QNullResult(const QSqlDriver* d): QSqlResult(d){}
    ~QNullResult(){}
protected:
    QVariant    data( int ) { return QVariant(); }
    bool	reset ( const QString& sqlquery ) { QString s(sqlquery); return FALSE; }
    bool	fetch( int i ) { i = i; return FALSE; }
    bool	fetchFirst() { return FALSE; }
    bool	fetchLast() { return FALSE; }
    bool	isNull( int ) {return FALSE; }
    QSqlRecord   record() {return QSqlRecord();}
    int             size()  {return 0;}
    int             numRowsAffected() {return 0;}
};

class QNullDriver : public QSqlDriver
{
public:
    QNullDriver(): QSqlDriver(){}
    ~QNullDriver(){}
    bool    hasTransactionSupport() const { return FALSE;} ;
    bool    hasQuerySizeSupport() const { return FALSE;} ;
    bool    canEditBinaryFields() const { return FALSE;} ;
    bool    open( const QString & ,
    			const QString & ,
			const QString & ,
			const QString &  ) {
				return FALSE;
			}
    void    close() {}
    QSqlQuery createQuery() const { return QSqlQuery( new QNullResult(this) ); }
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
    \class QSqlDatabase qsqldatabase.h
    \brief Class used for accessing SQL databases

    \module database

     This class is used to access SQL databases.  QSqlDatabase
     provides an abstract interface for accessing many types of
     backends.

     Database-specific drivers are used internally to actually access
     and manipulate data. (see QSqlDriver) Result set objects provide
     the interface for executing and manipulating SQL queries ( see
     QSqlQuery ).

*/

QSqlDatabase* QSqlDatabase::addDatabase( const QString& type, const QString& name )
{
    return QSqlDatabaseManager::addDatabase( new QSqlDatabase( type, name ), name );
}

QSqlDatabase* QSqlDatabase::database( const QString& name )
{
    return QSqlDatabaseManager::database( name );
}


/*!  Creates a QSqlDatabase with name \a databaseName that uses the
     driver described by \a type.  If the \a type is not recognized,
     the database will have no functionality.

     Available types are:

     <ul>
     <li>QODBC - ODBC (Open Database Connectivity) Driver
     <li>QOCI - Oracle Call Interface Driver
     <li>QPSQL - PostgreSQL Driver
     <li>QMYSQL - MySQL Driver
     </ul>
*/
QSqlDatabase::QSqlDatabase( const QString& type, const QString& name, QObject * parent, const char * objname )
: QObject(parent, objname)
{
    init( type, name );
}

/*!
  \internal
*/
void QSqlDatabase::init( const QString& type, const QString&  )
{
    d = new QSqlDatabasePrivate();
#ifndef QT_NO_PLUGIN
    //    d->plugIns = new QSqlDriverPlugInManager( QString((char*)getenv( "QTDIR" )) + "/lib" ); // ###
    //    d->driver = d->plugIns->create( type );
#endif
    if ( !d->driver ) {
#ifdef QT_SQL_POSTGRES
	if ( type == "QPSQL" )
	    d->driver = new QPSQLDriver();
#endif
#ifdef QT_SQL_MYSQL
	if ( type == "QMYSQL" )
	    d->driver = new QMySQLDriver();
#endif
#ifdef QT_SQL_ODBC
	if ( type == "QODBC" )
	    d->driver = new QODBCDriver();
#endif
#ifdef QT_SQL_OCI
	if ( type == "QOCI" )
	    d->driver = new QOCIDriver();
#endif
    }
    if ( !d->driver ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlDatabase warning: %s driver not loaded", type.data());
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

/*! Executes an SQL statement (i.e., INSERT, UPDATE, DELETE statement)
    on the database, and returns a QSqlQuery object.  Use lastError()
    to recover error information. If \a query is QString::null, an
    empty, invalid query is returned and lastError() is not affected.

    \sa QSqlQuery lastError()
*/

QSqlQuery QSqlDatabase::exec( const QString & query ) const
{
    QSqlQuery r = d->driver->createQuery();
    if ( !query.isNull() ) {
	r.exec( query );
	d->driver->setLastError( r.lastError() );
    }
    return r;
}

/*! Opens the database using the current connection values .  Returns
    TRUE on success, and FALSE if there was an error.  Error
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

/*! Opens the database using \a user name and \a password.  Returns
 TRUE on success, and FALSE if there was an error.  Error information
 can be retrieved using the lastError() method.

    \sa lastError()
*/

bool QSqlDatabase::open( const QString& user, const QString& password )
{
    setUserName( user );
    setPassword( password );
    return open();
}

/*! Closes the database, freeing any resources aquired.

*/

void QSqlDatabase::close()
{
    d->driver->close();
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

/*! Sets the name of the database.

*/

void QSqlDatabase::setDatabaseName( const QString& name )
{
    d->dbname = name;
}

/*! Sets the name of the database user.

*/

void QSqlDatabase::setUserName( const QString& name )
{
    d->uname = name;
}

/*! Sets the password of the database user.

*/

void QSqlDatabase::setPassword( const QString& password )
{
    d->pword = password;
}

/*! Sets the host name of the database.

*/

void QSqlDatabase::setHostName( const QString& host )
{
    d->hname = host;
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

QSqlRecord QSqlDatabase::record( const QString& tablename ) const
{
    return d->driver->record( tablename );
}


/*!
  Returns a list of fields used in the SQL \a query.

*/

QSqlRecord QSqlDatabase::record( const QSqlQuery& query ) const
{
    return d->driver->record( query );
}


#endif // QT_NO_SQL

/****************************************************************************
**
** Implementation of QSqlDatabase class
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

#include "qapplication.h"
#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldriverinterface.h"
#include "qinterfacemanager.h"
#include "qobject.h"
#include "qdict.h"
#include "qapplication.h"
#include <stdlib.h>

QT_STATIC_CONST_IMPL char * const QSqlDatabase::defaultConnection = "qt_sql_default_connection";

class QNullResult : public QSqlResult
{
public:
    QNullResult(const QSqlDriver* d): QSqlResult(d){}
    ~QNullResult(){}
protected:
    QVariant    data( int ) { return QVariant(); }
    bool        reset ( const QString& sqlquery ) { QString s(sqlquery); return FALSE; }
    bool        fetch( int i ) { i = i; return FALSE; }
    bool        fetchFirst() { return FALSE; }
    bool        fetchLast() { return FALSE; }
    bool        isNull( int ) {return FALSE; }
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
		  const QString &,
		  int ) {
	return FALSE;
    }
    void    close() {}
    QSqlQuery createQuery() const { return QSqlQuery( new QNullResult(this) ); }
};

class QSqlDatabaseManager : public QObject
{
public:
    QSqlDatabaseManager( QObject * parent = 0, const char * name = 0 );
    ~QSqlDatabaseManager();
    static QSqlDatabase* database( const QString& name, bool open );
    static QSqlDatabase* addDatabase( QSqlDatabase* db, const QString & name );
    static void          removeDatabase( const QString& name );
    static bool          contains( const QString& name );

protected:
    static QSqlDatabaseManager* instance();
    QDict< QSqlDatabase > dbDict;
};

/*!  Constructs an SQL database manager.

*/

QSqlDatabaseManager::QSqlDatabaseManager( QObject * parent, const char * name )
    : QObject( parent, name ),
      dbDict( 1 )
{
}

/*!
  Destroys the object and frees any allocated resources.  All open
  database connections are closed.  All database connections are deleted.

*/

QSqlDatabaseManager::~QSqlDatabaseManager()
{
    QDictIterator< QSqlDatabase > it( dbDict );
    while ( it.current() ) {
	it.current()->close();
	++it;
    }
    dbDict.setAutoDelete( TRUE );
}

static QSqlDatabaseManager * sqlConnection = 0;

/*!
  \internal

*/
QSqlDatabaseManager* QSqlDatabaseManager::instance()
{
    if ( !sqlConnection ) {
	if( qApp == 0 ){
	    qWarning( "QSqlDatabaseManager: A QApplication object has to be "
		      "instantiated in order to use the SQL module." );
	    return 0;
	}
	sqlConnection = new QSqlDatabaseManager( qApp, "database manager" );
    }
    return sqlConnection;
}

/*!  Returns a pointer to the database connection with name \a name.  If \a open
  is TRUE, the database connection is opened.  If \a name does not exist in the
  list of managed databases, 0 is returned.

*/

QSqlDatabase* QSqlDatabaseManager::database( const QString& name, bool open )
{
    if ( !contains( name ) ) {
	qWarning("Warning: QSqlDatabaseManager unable to find database " + name );
	return 0;
    }

    QSqlDatabaseManager* sqlConnection = instance();
    QSqlDatabase* db = sqlConnection->dbDict.find( name );
    if ( db && !db->isOpen() && open ) {
	db->open();
#ifdef QT_CHECK_RANGE
	if ( !db->isOpen() )
	    qWarning("Warning: QSqlDatabaseManager unable to open database: " + db->lastError().databaseText() + ": " + db->lastError().driverText() );
#endif
    }
    return db;
}

/*! Returns TRUE if the list of database connections contains \a name,
  otherwise returns FALSE.

 */

bool QSqlDatabaseManager::contains( const QString& name )
{
   QSqlDatabaseManager* sqlConnection = instance();
   QSqlDatabase* db = sqlConnection->dbDict.find( name );
   if ( db )
       return TRUE;
   return FALSE;
}


/*!  Adds a database to the SQL connection manager.  The database
  connection is referred to by \a name.  A pointer to the newly added
  database connection is returned.

  \sa QSqlDatabase database()

*/

QSqlDatabase* QSqlDatabaseManager::addDatabase( QSqlDatabase* db, const QString & name )
{
    QSqlDatabaseManager* sqlConnection = instance();
    if( sqlConnection == 0 )
	return 0;
    sqlConnection->removeDatabase( name );
    sqlConnection->dbDict.insert( name, db );
    return db;
}


/*!  Removes the database connection \a name from the SQL connection manager.
  Note that there should be no open queries on the database connection when this
  function is called, otherwise a resource leak will occur.

*/

void QSqlDatabaseManager::removeDatabase( const QString& name )
{
    QSqlDatabaseManager* sqlConnection = instance();
    sqlConnection->dbDict.setAutoDelete( TRUE );
    sqlConnection->dbDict.remove( name );
    sqlConnection->dbDict.setAutoDelete( FALSE );
}

class QSqlDatabase::Private
{
public:
    Private(): driver(0), plugIns(0), port(-1) {}
    ~Private()
    {
    }
    QSqlDriver* driver;
    QInterfaceManager<QSqlDriverInterface> *plugIns;
    QString dbname;
    QString uname;
    QString pword;
    QString hname;
    QString drvName;
    int port;
};

/*!
    \class QSqlDatabase qsqldatabase.h
    \brief This class is used to create SQL database connections and provide transaction handling

    \module sql

    This class is used to create connections to SQL databases. It also
    provides transaction handling functions for those database drivers
    that support transactions.

    The QSqlDatabase class itself provides an abstract interface for
    accessing many types of database backend. Database-specific drivers
    are used internally to actually access and manipulate data, (see
    QSqlDriver). Result set objects provide the interface for executing
    and manipulating SQL queries (see QSqlQuery).

*/

/*!  Adds a database to the list of database connections.  The
  database connection is referred to by \name.  A pointer to the newly
  added database connection is returned.  This pointer is owned by
  QSqlDatabase and will be deleted on program exit or when
  removeDatabase() is called.  If \a connectionName is not specified,
  the newly added database connection becomes the default database
  connection for the application, and subsequent calls to database()
  (without a database name parameter) will return a pointer to it.

  \sa database() removeDatabase()

*/
QSqlDatabase* QSqlDatabase::addDatabase( const QString& type, const QString& connectionName )
{
    return QSqlDatabaseManager::addDatabase( new QSqlDatabase( type, connectionName ), connectionName );
}

/*! Returns a pointer to the database connection named \a name.  The
  database connection must have been previously added with database().
  If \a open is TRUE (the default) and the database connection is not
  already open it is opened now.  If no \a connectionName is specified
  the default connection is used. If \a connectionName does not exist
  in the list of databases, 0 is returned.  The pointer returned is
  owned by QSqlDatabase and should \e not be deleted.

*/

QSqlDatabase* QSqlDatabase::database( const QString& connectionName, bool open )
{
    return QSqlDatabaseManager::database( connectionName, open );
}

/*!  Removes the database connection \a connectionName from the list
  of database connections.  Note that there should be no open queries
  on the database connection when this function is called, otherwise a
  resource leak will occur.

*/

void QSqlDatabase::removeDatabase( const QString& connectionName )
{
    QSqlDatabaseManager::removeDatabase( connectionName );
}

/*! Returns a list of all available database drivers.
*/

QStringList QSqlDatabase::drivers()
{
    QStringList l;
#ifdef QT_SQL_POSTGRES
    l << "QPSQL6" << "QPSQL7";
#endif
#ifdef QT_SQL_MYSQL
    l << "QMYSQL";
#endif
#ifdef QT_SQL_ODBC
    l << "QODBC";
#endif
#ifdef QT_SQL_OCI
    l << "QOCI";
#endif
#ifndef QT_NO_COMPONENT
    QInterfaceManager<QSqlDriverInterface> *plugIns;
    plugIns = new QInterfaceManager<QSqlDriverInterface>( IID_QSqlDriverInterface );
    plugIns->addLibraryPath( QString((char*)getenv( "QTDIR" )) + "/plugins/sqldrivers" );
    QStringList paths(QApplication::libraryPaths());
    QStringList::Iterator it = paths.begin();
    while (it != paths.end()) {
	plugIns->addLibraryPath(*it + "/sqldrivers");
	it++;
    }
    l += plugIns->featureList();
    delete plugIns;
#endif
    return l;
}

/*! Returns TRUE if the list of database connections contains \a
  connectionName, otherwise returns FALSE.

 */

bool QSqlDatabase::contains( const QString& connectionName )
{
    return QSqlDatabaseManager::contains( connectionName );
}


/*!  Creates a QSqlDatabase connection named \a name that uses the
     driver referred to by \a driver.  If the \a driver is not recognized,
     the database connection will have no functionality.

     The currently available drivers are:

     <ul>
     <li>QODBC - ODBC (Open Database Connectivity) Driver
     <li>QOCI - Oracle Call Interface Driver
     <li>QPSQL6 - PostgreSQL v6.x Driver
     <li>QPSQL7 - PostgreSQL v7.x Driver
     <li>QMYSQL - MySQL Driver
     </ul>

*/

QSqlDatabase::QSqlDatabase( const QString& driver, const QString& name, QObject * parent, const char * objname )
: QObject(parent, objname)
{
    init( driver, name );
}

/*!
  \internal
*/
void QSqlDatabase::init( const QString& type, const QString&  )
{

    d = new Private();
    d->drvName = type;

    if ( !d->driver ) {

#ifdef QT_SQL_POSTGRES
	if ( type == "QPSQL6" )
	    d->driver = new QPSQLDriver( QPSQLDriver::Version6 );
	if ( type == "QPSQL7" )
	    d->driver = new QPSQLDriver( QPSQLDriver::Version7 );
#endif

#ifdef QT_SQL_MYSQL
	if ( type == "QMYSQL" )
	    d->driver = new QMYSQLDriver();
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

#ifndef QT_NO_COMPONENT
    if ( !d->driver ) {
	d->plugIns =
	    new QInterfaceManager<QSqlDriverInterface>( IID_QSqlDriverInterface );
	d->plugIns->addLibraryPath( QString((char*)getenv( "QTDIR" )) +
				    "/plugins/sqldrivers" );
	QStringList paths(QApplication::libraryPaths());
	QStringList::Iterator it = paths.begin();
	while (it != paths.end()) {
	    d->plugIns->addLibraryPath(*it + "/sqldrivers");
	    it++;
	}
	QSqlDriverInterface *iface = d->plugIns->queryInterface( type );
	if( iface ){
	    d->driver = iface->create( type );
	    iface->release();
	}
    }
#endif

    if ( !d->driver ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlDatabase warning: %s driver not loaded", type.data());
#endif
	d->driver = new QNullDriver();
	d->driver->setLastError( QSqlError( "Driver not loaded", "Driver not loaded" ) );
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

/*! Executes an SQL statement (e.g. an INSERT, UPDATE or DELETE statement)
    on the database, and returns a QSqlQuery object.  Use lastError()
    to retrieve error information. If \a query is QString::null, an
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

/*! Opens the database connection using the current connection values.  Returns
    TRUE on success, and FALSE if there was an error.  Error
    information can be retrieved using the lastError() function.

    \sa lastError()
*/

bool QSqlDatabase::open()
{
    return d->driver->open( d->dbname,
				d->uname,
				d->pword,
				d->hname);
}

/*! Opens the database connection using \a user name and \a password.  Returns
 TRUE on success, and FALSE if there was an error.  Error information
 can be retrieved using the lastError() function.

    \sa lastError()
*/

bool QSqlDatabase::open( const QString& user, const QString& password )
{
    setUserName( user );
    setPassword( password );
    return open();
}

/*! Closes the database connection, freeing any resources acquired.

*/

void QSqlDatabase::close()
{
    d->driver->close();
}

/*! Returns TRUE if the database connection is currently open, otherwise
 returns FALSE.

*/

bool QSqlDatabase::isOpen() const
{
    return d->driver->isOpen();
}

/*! Returns TRUE if there was an error opening the database connection,
    otherwise returns FALSE. Error information can be retrieved
    using the lastError() function.

*/

bool QSqlDatabase::isOpenError() const
{
    return d->driver->isOpenError();
}

/*! Begins a transaction on the database if the driver supports transactions.
    Returns TRUE if the operation succeeded, FALSE otherwise.

    \sa hasTransactionSupport() commit() rollback()
*/

bool QSqlDatabase::transaction()
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

/*! Sets the name of the database connection to \a name.

*/

void QSqlDatabase::setDatabaseName( const QString& name )
{
    d->dbname = name;
}

/*! Sets the name of the database user to \a name.

*/

void QSqlDatabase::setUserName( const QString& name )
{
    d->uname = name;
}

/*! Sets the password of the database user to \a password.

*/

void QSqlDatabase::setPassword( const QString& password )
{
    d->pword = password;
}

/*! Sets the host name of the database to \a host.

*/

void QSqlDatabase::setHostName( const QString& host )
{
    d->hname = host;
}

/*! Sets the port used by the connection to \a p.

*/

void QSqlDatabase::setPort( int p )
{
    d->port = p;
}

/*! Returns the name of the database connection, or QString::null if a
 name has not been set.

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

/*! Returns the database user password, or QString::null if a password
   has not been set.

*/

QString QSqlDatabase::password() const
{
    return d->pword;
}

/*! Returns the database host name, or QString::null if a host name has
   not been set.

*/

QString QSqlDatabase::hostName() const
{
    return d->hname;
}

/*! Returns the name of the driver used by the database connection.

*/
QString QSqlDatabase::driverName() const
{
    return d->drvName;
}

/*! Returns the port used by the database connection.

*/
int QSqlDatabase::port() const
{
    return d->port;
}

/*! Returns a pointer to the database driver used to access the database connection.

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
  primary index exists an empty QSqlIndex will be returned.

*/

QSqlIndex QSqlDatabase::primaryIndex( const QString& tablename ) const
{
    return d->driver->primaryIndex( tablename );
}


/*!  Returns a QSqlRecord populated with the names of all the fields in
    the table (or view) named \a tablename. The order in which the fields are
    returned is undefined.  If no such table (or view) exists, an empty
    list is returned.

*/

QSqlRecord QSqlDatabase::record( const QString& tablename ) const
{
    return d->driver->record( tablename );
}


/*!
  Returns a QSqlRecord populated with the names of all the fields used
  in the SQL \a query. If the query is a "SELECT *" the order in which
  fields are returned is undefined.

*/

QSqlRecord QSqlDatabase::record( const QSqlQuery& query ) const
{
    return d->driver->record( query );
}


#endif // QT_NO_SQL

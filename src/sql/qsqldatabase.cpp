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
#include "drivers/psql/qsql_psql.h"
#endif
#ifdef QT_SQL_MYSQL
#include "drivers/mysql/qsql_mysql.h"
#endif
#ifdef QT_SQL_ODBC
#include "drivers/odbc/qsql_odbc.h"
#endif
#ifdef QT_SQL_OCI
#include "drivers/oci/qsql_oci.h"
#endif
#ifdef QT_SQL_TDS
#include "drivers/tds/qsql_tds.h"
#endif

#include "qapplication.h"
#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldriverinterface_p.h"
#include <private/qpluginmanager_p.h>
#include "qobject.h"
#include "qdict.h"
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
    bool    hasFeature( DriverFeature /* f */ ) const { return FALSE; } ;
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

typedef QDict<QSqlDriverCreatorBase> QDriverDict;

class QSqlDatabaseManager : public QObject
{
public:
    QSqlDatabaseManager( QObject * parent = 0, const char * name = 0 );
    ~QSqlDatabaseManager();
    static QSqlDatabase* database( const QString& name, bool open );
    static QSqlDatabase* addDatabase( QSqlDatabase* db, const QString & name );
    static void          removeDatabase( const QString& name );
    static bool          contains( const QString& name );
    static QDriverDict*  driverDict();

protected:
    static QSqlDatabaseManager* instance();
    QDict< QSqlDatabase > dbDict;
    QDriverDict* drDict;
};

/*!  Constructs an SQL database manager.

*/

QSqlDatabaseManager::QSqlDatabaseManager( QObject * parent, const char * name )
    : QObject( parent, name ), dbDict( 1 ), drDict( 0 )
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
	delete it.current();
	++it;
    }
    delete drDict;
}

/*!
  \internal
*/
QDriverDict* QSqlDatabaseManager::driverDict()
{
    QSqlDatabaseManager* sqlConnection = instance();
    if ( !sqlConnection->drDict ) {
	sqlConnection->drDict = new QDriverDict();
	sqlConnection->drDict->setAutoDelete( TRUE );
    }
    return sqlConnection->drDict;
}


/*!
  \internal
*/
QSqlDatabaseManager* QSqlDatabaseManager::instance()
{
    static QSqlDatabaseManager *sqlConnection = 0;
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

class QSqlDatabasePrivate
{
public:
    QSqlDatabasePrivate():
	driver(0),
#ifndef QT_NO_COMPONENT
	plugIns(0),
#endif
	port(-1) {}
    ~QSqlDatabasePrivate()
    {
    }
    QSqlDriver* driver;
#ifndef QT_NO_COMPONENT
    QPluginManager<QSqlDriverFactoryInterface> *plugIns;
#endif
    QString dbname;
    QString uname;
    QString pword;
    QString hname;
    QString drvName;
    int port;
};

/*!
    \class QSqlDatabase qsqldatabase.h
    \ingroup database
  \mainclass

    \brief The QSqlDatabase class is used to create SQL database
    connections and provide transaction handling.

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

/*!  Adds a database to the list of database connections using the
  driver \a type and the connection name \a connectionName.

  The database connection is referred to by \a connectionName.  A
  pointer to the newly added database connection is returned.  This
  pointer is owned by QSqlDatabase and will be deleted on program exit
  or when removeDatabase() is called.  If \a connectionName is not
  specified, the newly added database connection becomes the default
  database connection for the application, and subsequent calls to
  database() (without a database name parameter) will return a pointer
  to it.

  \sa database() removeDatabase()

*/
QSqlDatabase* QSqlDatabase::addDatabase( const QString& type, const QString& connectionName )
{
    return QSqlDatabaseManager::addDatabase( new QSqlDatabase( type, connectionName ), connectionName );
}

/*! Returns a pointer to the database connection named \a
  connectionName.  The database connection must have been previously
  added with database().  If \a open is TRUE (the default) and the
  database connection is not already open it is opened now.  If no \a
  connectionName is specified the default connection is used. If \a
  connectionName does not exist in the list of databases, 0 is
  returned.  The pointer returned is owned by QSqlDatabase and should
  \e not be deleted.

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

#ifndef QT_NO_COMPONENT
    QPluginManager<QSqlDriverFactoryInterface> *plugIns;
    plugIns = new QPluginManager<QSqlDriverFactoryInterface>( IID_QSqlDriverFactory, QApplication::libraryPaths(), "/sqldrivers" );

    l = plugIns->featureList();
    delete plugIns;
#endif

    QDictIterator<QSqlDriverCreatorBase> itd( *QSqlDatabaseManager::driverDict() );
    while ( itd.current() ) {
	if ( !l.contains( itd.currentKey() ) )
	    l << itd.currentKey();
	++itd;
    }

#ifdef QT_SQL_POSTGRES
    if ( !l.contains( "QPSQL7" ) )
	l << "QPSQL7";
#endif
#ifdef QT_SQL_MYSQL
    if ( !l.contains( "QMYSQL3" ) )
	l << "QMYSQL3";
#endif
#ifdef QT_SQL_ODBC
    if ( !l.contains( "QODBC3" ) )
	l << "QODBC3";
#endif
#ifdef QT_SQL_OCI
    if ( !l.contains( "QOCI8" ) )
	l << "QOCI8";
#endif
#ifdef QT_SQL_TDS
    if ( !l.contains( "QTDS7" ) )
	l << "QTDS7";
#endif

    return l;
}

/*!
  \internal
*/
void QSqlDatabase::registerSqlDriver( const QString& name, const QSqlDriverCreatorBase* dcb )
{
    QSqlDatabaseManager::driverDict()->remove( name );
    if ( dcb ) {
	QSqlDatabaseManager::driverDict()->insert( name, dcb );
    }
}

/*! Returns TRUE if the list of database connections contains \a
  connectionName, otherwise returns FALSE.

 */

bool QSqlDatabase::contains( const QString& connectionName )
{
    return QSqlDatabaseManager::contains( connectionName );
}


/*!  Creates a QSqlDatabase connection named \a name that uses the
     driver referred to by \a driver, with the parent \a parent and
     the object name \a objname.  If the \a driver is not recognized,
     the database connection will have no functionality.

     The currently available drivers are:

     \list     
     \i QODBC3 - ODBC (Open Database Connectivity) Driver
     \i QOCI8 - Oracle Call Interface Driver
     \i QPSQL7 - PostgreSQL v6.x and v7.x Driver
     \i QTDS7 - Sybase Adaptive Server and Microsoft SQL Server Driver
     \i QMYSQL3 - MySQL Driver
     \endlist

     Note that additional 3<sup>rd</sup> party drivers can be loaded
     dynamically.

*/

QSqlDatabase::QSqlDatabase( const QString& driver, const QString& name, QObject * parent, const char * objname )
: QObject(parent, objname)
{
    init( driver, name );
}

/*!
  \internal

  Iniitializes the database with driver \a type and name \a name.
*/
void QSqlDatabase::init( const QString& type, const QString&  )
{

    d = new QSqlDatabasePrivate();
    d->drvName = type;

    if ( !d->driver ) {

#ifdef QT_SQL_POSTGRES
	if ( type == "QPSQL7" )
	    d->driver = new QPSQLDriver();
#endif

#ifdef QT_SQL_MYSQL
	if ( type == "QMYSQL3" )
	    d->driver = new QMYSQLDriver();
#endif

#ifdef QT_SQL_ODBC
	if ( type == "QODBC3" )
	    d->driver = new QODBCDriver();
#endif

#ifdef QT_SQL_OCI
	if ( type == "QOCI8" )
	    d->driver = new QOCIDriver();
#endif

#ifdef QT_SQL_TDS
	if ( type == "QTDS7" )
	    d->driver = new QTDSDriver();
#endif

    }

    if ( !d->driver ) {
	QDictIterator<QSqlDriverCreatorBase> it( *QSqlDatabaseManager::driverDict() );
	while ( it.current() && !d->driver ) {
	    if ( type == it.currentKey() ) {
		d->driver = it.current()->createObject();
	    }
	    ++it;
	}
    }

#ifndef QT_NO_COMPONENT
    if ( !d->driver ) {
	d->plugIns =
	    new QPluginManager<QSqlDriverFactoryInterface>( IID_QSqlDriverFactory, QApplication::libraryPaths(), "/sqldrivers" );

	QInterfacePtr<QSqlDriverFactoryInterface> iface = 0;
	d->plugIns->queryInterface( type, &iface );
	if( iface )
	    d->driver = iface->create( type );
    }
#endif

    if ( !d->driver ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QSqlDatabase warning: %s driver not loaded", type.latin1() );
	qWarning( "QSqlDatabase: available drivers: " + drivers().join(" ") );
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
#ifndef QT_NO_COMPONENT
    delete d->plugIns;
#endif
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
				d->hname,
				d->port);
}

/*! \overload

  Opens the database connection using \a user name and \a password.  Returns
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

    \sa QSqlDriver::hasFeature() commit() rollback()
*/

bool QSqlDatabase::transaction()
{
    if ( !d->driver->hasFeature( QSqlDriver::Transactions ) )
	return FALSE;
    return d->driver->beginTransaction();
}

/*! Commits a transaction to the database if the driver supports transactions.
    Returns TRUE if the operation succeeded, FALSE otherwise.

    \sa QSqlDriver::hasFeature() rollback()
*/

bool QSqlDatabase::commit()
{
    if ( !d->driver->hasFeature( QSqlDriver::Transactions ) )
	return FALSE;
    return d->driver->commitTransaction();
}

/*! Rolls a transaction back on the database if the driver supports transactions.
    Returns TRUE if the operation succeeded, FALSE otherwise.

    \sa QSqlDriver::hasFeature() commit() transaction()
*/

bool QSqlDatabase::rollback()
{
    if ( !d->driver->hasFeature( QSqlDriver::Transactions ) )
	return FALSE;
    return d->driver->rollbackTransaction();
}

/*! \property QSqlDatabase::databaseName

  \brief the name of the database

*/

void QSqlDatabase::setDatabaseName( const QString& name )
{
    d->dbname = name;
}

/*! \property QSqlDatabase::userName

  \brief the user name connected to the database

*/

void QSqlDatabase::setUserName( const QString& name )
{
    d->uname = name;
}

/*! \property QSqlDatabase::password

  \brief the password used to connect to the database

*/

void QSqlDatabase::setPassword( const QString& password )
{
    d->pword = password;
}

/*! \property QSqlDatabase::hostName

  \brief the host name where the database resides

*/

void QSqlDatabase::setHostName( const QString& host )
{
    d->hname = host;
}

/*! \property QSqlDatabase::port

  \brief the port used to connect to the database

*/

void QSqlDatabase::setPort( int p )
{
    d->port = p;
}

QString QSqlDatabase::databaseName() const
{
    return d->dbname;
}

QString QSqlDatabase::userName() const
{
    return d->uname;
}

QString QSqlDatabase::password() const
{
    return d->pword;
}

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
    record is returned.

    \sa recordInfo()
*/

QSqlRecord QSqlDatabase::record( const QString& tablename ) const
{
    return d->driver->record( tablename );
}


/*! \overload

  Returns a QSqlRecord populated with the names of all the fields used
  in the SQL \a query. If the query is a "SELECT *" the order in which
  fields are returned is undefined.

  \sa recordInfo()
*/

QSqlRecord QSqlDatabase::record( const QSqlQuery& query ) const
{
    return d->driver->record( query );
}

/*!
  Returns a QSqlRecordInfo populated with meta-data about the table (or view)
  \a tablename. If no such table (or view) exists, an empty record is returned.

  \sa QSqlRecordInfo, QSqlFieldInfo, record()
*/
QSqlRecordInfo QSqlDatabase::recordInfo( const QString& tablename ) const
{
    return d->driver->recordInfo( tablename );
}

/*! \overload

    Returns a QSqlRecordInfo object with meta data for the QSqlQuery \a query.
    Note that this overloaded function may return not as much information as
    the recordInfo function which takes the name of a table as parameter.

   \sa QSqlRecordInfo, QSqlFieldInfo, record()
*/
QSqlRecordInfo QSqlDatabase::recordInfo( const QSqlQuery& query ) const
{
    return d->driver->recordInfo( query );
}


#endif // QT_NO_SQL

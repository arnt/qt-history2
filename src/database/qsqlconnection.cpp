#include "qsqlconnection.h"
#ifndef QT_NO_SQL

#include "qsqldatabase.h"
#include "qapplication.h"

/*!  Constructs a SQL connection manager.

*/

QSqlConnection::QSqlConnection( QObject* parent, const char* name )
    : QObject( parent, name ), dbDict( 1 )
{
    dbDict.setAutoDelete( TRUE );
}

/*!
  Destroys the object and frees any allocated resources.  All open
  databases are closed.

*/

QSqlConnection::~QSqlConnection()
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

QSqlConnection* QSqlConnection::instance()
{
    static QSqlConnection* sqlConnection = 0;
    if ( !sqlConnection ) {
#ifdef CHECK_RANGE
	if ( !qApp )
	    qWarning("Warning: creating QSqlConnection with no parent.  Use QSqlConnection::free() to clean up resources." );
#endif
	sqlConnection = new QSqlConnection( qApp, "QSqlConnection instance" );
    }
    return sqlConnection;
}

/*!
  Returns a pointer to the database with name \a name.  If the database was not previously
  opened, it is opened now.  If \name does not exist in the list of managed database,
  0 is returned.

*/

QSqlDatabase* QSqlConnection::database( const QString& name )
{
    QSqlConnection* sqlConnection = instance();
    QSqlDatabase* db = sqlConnection->dbDict.find( name );
    if ( db && !db->isOpen() )
	db->open();
    return db;
}



/*!
  Adds a database to the SQL connection manager.  The database is
  referred to by \name.  A pointer to the newly added database is
  returned.

  \sa QSqlDatabase database()

*/

QSqlDatabase* QSqlConnection::addDatabase( const QString& type,
						  const QString & db,
						  const QString & user,
						  const QString & password,
						  const QString & host,
						  const QString & name )
{
    QSqlDatabase* database = new QSqlDatabase( type, db, user, password, host );
    QSqlConnection* sqlConnection = instance();
    sqlConnection->dbDict.insert( name, database );
    return database;
}


/*!
  Removes the database \a name from the SQL connection manager.  Note that
  there should be no open queries on the database when this method is called,
  otherwise resources will be leaked.

*/

void QSqlConnection::removeDatabase( const QString& name )
{
    QSqlConnection* sqlConnection = instance();
    sqlConnection->dbDict.remove( name );
}


/*!
  Deletes all open database connections and frees all connection resources.

*/

void QSqlConnection::free()
{
    delete instance();
}

#endif // QT_NO_SQL



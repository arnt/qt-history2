#include "qsqlconnection.h"
#ifndef QT_NO_SQL

#include "qsqldatabase.h"

/*!  Constructs a SQL connection manager.

*/

QSqlConnection::QSqlConnection( QObject* parent, const char* name )
    : QObject( parent, name ), dbDict( 1 )
{
    dbDict.setAutoDelete( TRUE );
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlConnection::~QSqlConnection()
{
    
}

/*!
  \internal

*/

QSqlConnection* QSqlConnection::instance()
{
    static QSqlConnection* sqlConnection = 0;
    if ( !sqlConnection )
	sqlConnection = new QSqlConnection();
    return sqlConnection;
}

/*! 
  Returns a pointer to the database with name \a name.  If \name
  does not exist, 0 is returned.

*/

QSqlDatabase* QSqlConnection::database( const QString& name )
{
    QSqlConnection* sqlConnection = instance();
    return sqlConnection->dbDict.find( name );
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

#endif // QT_NO_SQL

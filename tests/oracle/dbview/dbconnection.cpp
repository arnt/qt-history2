#include "dbconnection.h"
#include "qsqldatabase.h"
#include "qsqlcursor.h"
#include "qlistview.h"
#include "qstringlist.h"

DBConnection::DBConnection()
{
    db = QSqlDatabase::addDatabase( "QOCI8" );
    db->setDatabaseName( "AXAPTA_TEST" );
    db->setUserName( "system" );
    db->setPassword( "manager" );
    db->setHostName( "breiflabb" );
    db->open();

    QSqlQuery qUsers( "SELECT USERNAME FROM ALL_USERS" );
    while( qUsers.next() ) {
	QString userName = qUsers.value( 0 ).toString();
	userList << userName;

	QSqlQuery qTables( QString( "SELECT TABLE_NAME FROM ALL_TABLES WHERE OWNER = '%1'" ).arg( userName ) );
	while( qTables.next() ) {
	    tableMap[ userName ] << qTables.value( 0 ).toString();
	}

	QSqlQuery qViews( QString( "SELECT VIEW_NAME FROM ALL_VIEWS WHERE OWNER = '%1'" ).arg( userName ) );
	while( qViews.next() ) {
	    viewMap[ userName ] << qViews.value( 0 ).toString();
	}

	QSqlQuery qIndexes( QString( "SELECT INDEX_NAME FROM ALL_INDEXES WHERE OWNER = '%1'" ).arg( userName ) );
	while( qIndexes.next() ) {
	    indexMap[ userName ] << qIndexes.value( 0 ).toString();
	}
    }
    
    QSqlQuery qTableSpaces( "SELECT TABLESPACE_NAME FROM DBA_TABLESPACES" );
    while( qTableSpaces.next() ) {
	tableSpaceList << qTableSpaces.value( 0 ).toString();
    }
}

DBConnection::~DBConnection()
{
    db->close();
    delete db;
}

QSqlDatabase* DBConnection::database()
{
    return db;
}

QStringList& DBConnection::users()
{
    return userList;
}

QStringList& DBConnection::tableSpaces()
{
    return tableSpaceList;
}

QStringList& DBConnection::tables( QString userName )
{
    return tableMap[ userName ];
}

QStringList& DBConnection::views( QString userName )
{
    return viewMap[ userName ];
}

QStringList& DBConnection::indexes( QString userName )
{
    return indexMap[ userName ];
}


#include "app.h"
#include <qsqlcursor.h>
#include <qregexp.h>
#include <qfile.h>
#include <qdatetime.h>

ImportApp::ImportApp( int argc, char** argv ) : QApplication( argc, argv )
{
    axaptaDB = QSqlDatabase::addDatabase( "QOCI8" );
    axaptaDB->setUserName( "bmssa" );
    axaptaDB->setDatabaseName( "axdb.troll.no" );
    axaptaDB->setPassword( "bmssa_pwd" );
    axaptaDB->setHostName( "minitrue.troll.no" );
}

void ImportApp::doImport()
{
    if( axaptaDB->open() ) {
	QSqlCursor licenseCursor( "LICENSE" );
	QSqlCursor custCursor( "CUSTTABLE" );

	licenseCursor.select( "DATAAREAID = 'ts3' AND LENGTH( CUSTACCOUNT ) < 2" );
	licenseCursor.first();
	while( licenseCursor.isValid() ) {
	    QString tmp = "DATAAREAID = 'ts3' AND INTERNID = '" + licenseCursor.value( "INTERNCUSTOMER" ).toString() + "'";
	    custCursor.select( tmp );
	    custCursor.first();
	    if( custCursor.isValid() ) {
		QSqlQuery query;
		QString buffer( "UPDATE LICENSE SET CUSTACCOUNT = %1 WHERE DATAAREAID = 'ts3' AND LICENSEID = %2" );
		buffer = buffer.arg( custCursor.value( "ACCOUNTNUM" ).toInt() ).arg( licenseCursor.value( "LICENSEID" ).toInt() );
		query.exec( buffer );
/*
		QSqlRecord* buffer = licenseCursor.primeUpdate();
		QString account = custCursor.value( "ACCOUNTNUM" ).toString();
		buffer->setValue( "CUSTACCOUNT", account );
		licenseCursor.update( false );
*/
	    }
	    licenseCursor.next();
	}
    }
}

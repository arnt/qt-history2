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
	QSqlCursor contactCursor( "CONTACTPERSON" );

	licenseCursor.select( "DATAAREAID = 'nor'" );
	licenseCursor.first();
	while( licenseCursor.isValid() ) {
	    QString tmp( "DATAAREAID = 'nor' AND INTERNID = '%1'" );
	    tmp = tmp.arg( licenseCursor.value( "INTERNCUSTOMER" ).toString() );
	    custCursor.select( tmp );
	    custCursor.first();
	    if( custCursor.isValid() ) {
		QSqlQuery query;
		QString buffer( "UPDATE LICENSE SET CUSTACCOUNT = %1 WHERE DATAAREAID = 'nor' AND LICENSEID = %2" );
		buffer = buffer.arg( custCursor.value( "ACCOUNTNUM" ).toInt() ).arg( licenseCursor.value( "LICENSEID" ).toInt() );
		query.exec( buffer );
	    }
	    tmp = QString( "DATAAREAID = 'nor' AND UPPER( NAME ) = UPPER( '%1' ) AND UPPER( EMAIL ) = UPPER( '%2' )" ).arg( licenseCursor.value( "CONTACTPERSONNAME" ).toString() ).arg( licenseCursor.value( "CONTACTPERSONEMAIL" ).toString() );
	    contactCursor.select( tmp );
	    contactCursor.first();
	    if( contactCursor.isValid() ) {
		QSqlQuery query;
		QString buffer( "UPDATE LICENSE SET LICENSEE = %1 WHERE DATAAREAID = 'nor' AND LICENSEID = %2" );
		buffer = buffer.arg( contactCursor.value( "CONTACTPERSONID" ).toInt() ).arg( licenseCursor.value( "LICENSEID" ).toInt() );
		query.exec( buffer );
	    }
	    licenseCursor.next();
	}
    }
}

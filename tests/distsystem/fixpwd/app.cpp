#include "app.h"
#include <qsqlcursor.h>
#include <qregexp.h>
#include <qfile.h>

#define DATAAREA "nor"

ImportApp::ImportApp( int argc, char** argv ) : QApplication( argc, argv )
{
    internDB = QSqlDatabase::addDatabase( "QMYSQL3", "intern" );
    internDB->setUserName( QString::null );
    internDB->setDatabaseName( "troll" );
    internDB->setPassword( QString::null );
    internDB->setHostName( "lupinella.troll.no" );
    axaptaDB = QSqlDatabase::addDatabase( "QOCI8", "axapta" );
    axaptaDB->setUserName( "bmssa" );
    axaptaDB->setDatabaseName( "axdb.troll.no" );
    axaptaDB->setPassword( "bmssa_pwd" );
    axaptaDB->setHostName( "minitrue.troll.no" );
}

void ImportApp::doImport()
{
    QStringList skippedIds;

    if( internDB->open() ) {
	if( axaptaDB->open() ) {
	    QSqlCursor internCursor( "License", true, internDB );
	    QSqlCursor axaptaCursor( "License", true, axaptaDB );

	    internCursor.select();
	    internCursor.first();
	    while( internCursor.isValid() ) {
		QString aselect = ( QString( "LicenseID = %1 and dataareaid = '%2'" ).arg( internCursor.value( "licenseID" ).toString() ).arg( DATAAREA ) );
		axaptaCursor.select( aselect );
		axaptaCursor.first();
		if( axaptaCursor.isValid() ) {
			// We found a record, hooray :)
			QSqlQuery q( QString::null, axaptaDB );
			QString tmp = QString( "UPDATE LICENSE SET PASSWORD = '%1' WHERE DATAAREAID = '%2' AND LICENSEID = %3" ).arg( internCursor.value( "password" ).toString() ).arg( DATAAREA ).arg( internCursor.value( "licenseID" ).toString() );
			q.exec( tmp );
		}
		internCursor.next();
	    }
	}
    }
}

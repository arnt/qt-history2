#include <qapplication.h>
#include <qsqldatabase.h>
#include "pingpongfrontend.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSqlDatabase * db = QSqlDatabase::addDatabase( "QPSQL6" );
    db->setDatabaseName( "pingpong" );
    db->setUserName( "db" );
    db->setPassword( "db" );
    db->setHostName( "silverfish.troll.no" );

    if( !db->open() ){
	qWarning( "Unable to open database: " + db->lastError().driverText());
	qWarning( db->lastError().databaseText() );
	return 0;
    }

    PingpongFrontEnd dbapp;

    a.setMainWidget( &dbapp );
    dbapp.show();

    return a.exec();
}

#include <qapplication.h>
#include <qsqldatabase.h>
#include "pingpongapp.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSqlDatabase * db = QSqlDatabase::addDatabase( "QPSQL6" );
    db->setDatabaseName( "pingpong" );
    db->setUserName( "db" );
    db->setPassword( "db" );
    db->setHostName( "silverfish" );

    if( !db->open() ){
	qWarning( "Unable to open database: " + db->lastError().driverText());
	qWarning( db->lastError().databaseText() );
	return 0;
    }

    PingPongApp dbapp;

    a.setMainWidget( &dbapp );
    dbapp.show();

    return a.exec();
}

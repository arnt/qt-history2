#include <qapplication.h>
#include <qeditorfactory.h>
#include <qsqlform.h>
#include <qsqltable.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qsqldatabase.h>
#include "databaseapp.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSqlDatabase * db = QSqlDatabase::addDatabase( "QPSQL" );
    db->setDatabaseName( "test" );
    db->setUserName( "db" );
    db->setPassword( "db" );
    db->setHostName( "cocktail" );
    
    if( !db->open() ){
	qWarning( "Unable to open database: " + db->lastError().driverText());
	qWarning( db->lastError().databaseText() );
	return 0;
    }
    
    DatabaseApp dbapp;
    
    a.setMainWidget( &dbapp );
    dbapp.show();
    
    return a.exec();
}

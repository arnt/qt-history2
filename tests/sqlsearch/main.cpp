#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqltable.h>
#include <qsqlcursor.h>
#include <qsqlform.h>

#include "search.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSqlDatabase * db = QSqlDatabase::addDatabase( "QPSQL7" );
    db->setDatabaseName( "test" );
    db->setUserName( "db" );
    db->setPassword( "db" );
    db->setHostName( "cocktail" );

    if( !db->open() ){
	qWarning( db->lastError().databaseText() );
	return 0;
    }

    Search mySearch;
	
    a.setMainWidget( &mySearch );
    mySearch.show();

    return a.exec();
}

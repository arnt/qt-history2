#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqltable.h>
#include <qsqlcursor.h>
#include <qsqlform.h>

#include "form.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSqlDatabase * db = QSqlDatabase::addDatabase( "QPSQL6" );
    db->setDatabaseName( "simpledb" );
    db->setUserName( "trond" );
    db->setPassword( "trond" );
    db->setHostName( "silverfish" );

    if( !db->open() ){
	qWarning( db->lastError().databaseText() );
	return 0;
    }

    QStringList tables = db->tables();
    if( !tables.contains("simpletable") && !tables.contains("SIMPLETABLE") ){
	db->exec( "create table simpletable (id int4 primary key, date date,"
		  "name varchar(20), address varchar(20) );");
	db->exec( "insert into simpletable (id, date, name, address) values "
		  "(0, '2000-11-03', 'Trond', 'Somewhere');" );
	db->exec( "insert into simpletable (id, date, name, address) values "
		  "(1, '2000-11-04', 'Dave', 'Here');" );
    }
    
    Form myForm;
	
    a.setMainWidget( &myForm );
    myForm.show();

    return a.exec();
}

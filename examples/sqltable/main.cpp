#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqltable.h>
#include <qsqlcursor.h>

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
	db->exec( "drop table simpletable;");
	db->exec( "create table simpletable (id int4 primary key,"
		  "name varchar(20), address varchar(20) );");
	db->exec( "insert into simpletable (id, name, address) values "
		  "(0, 'Trond', 'Somewhere');" );
	db->exec( "insert into simpletable (id, name, address) values "
		  "(1, 'Dave', 'Here');" );
    }

    QSqlCursor cursor( "simpletable" );
    cursor.setDisplayLabel( "name", "Name" );
    cursor.setDisplayLabel( "address", "Address" );
        
    QSqlTable table( &cursor, FALSE );
    table.addColumn( cursor.field( "name" ) );
    table.addColumn( cursor.field( "address" ) );
	
    a.setMainWidget( &table );
    table.show();

    return a.exec();
}

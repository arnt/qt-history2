/****************************************************************************
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qdatatable.h>
#include <qsqlcursor.h>

/* This example program expects a table called 'simpletable' to exist
   in the database.  You can create this table by running the
   following SQL script (modify to suit your backend, if necessary):

   drop table simpletable;
   create table simpletable
   (id number primary key,
    name varchar(20),
    address varchar(20) );

   -- optional, some sample data
   insert into simpletable (id, name, address)
	  values (1, 'Trond', 'Oslo');
   insert into simpletable (id, name, address)
	  values (2, 'Dave', 'Oslo');
*/

/* Modify the following to match your environment */
#define DRIVER       "QPSQL6"
#define DATABASE     "simpledb"
#define USER         "trond"
#define PASSWORD     "trond"
#define HOST         "silverfish.troll.no"

class SimpleCursor : public QSqlCursor
{
public:
    SimpleCursor () : QSqlCursor( "simpletable" ) {}
protected:
    QSqlRecord* primeInsert()
    {
	QSqlRecord* buf = QSqlCursor::primeInsert();
	QSqlQuery q( "select max(id)+1 from simpletable;" );
	if ( q.next() )
	       buf->setValue( "id", q.value(0) );
	return buf;
    }
};

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSqlDatabase * db = QSqlDatabase::addDatabase( DRIVER );
    db->setDatabaseName( DATABASE );
    db->setUserName( USER );
    db->setPassword( PASSWORD );
    db->setHostName( HOST );

    if( !db->open() ){
	qWarning( "Unable to open database: " + db->lastError().databaseText() );
	return 0;
    }

    SimpleCursor cursor;

    QDataTable table( &cursor );
    table.addColumn( "name", "Name" );
    table.addColumn( "address", "Address" );
    table.setSorting( TRUE );

    a.setMainWidget( &table );
    table.refresh( TRUE, TRUE ); /* load data */
    table.show();    /* show widget */

    return a.exec();
}

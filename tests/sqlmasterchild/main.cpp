#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include "mainwindow.h"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QSqlDatabase* db = QSqlDatabase::addDatabase( qApp->argv()[1] );
    db->setDatabaseName( qApp->argv()[2] );
    db->setUserName( qApp->argv()[3] );
    db->setPassword( qApp->argv()[4] );
    db->setHostName( qApp->argv()[5] );
    db->open();

    /* create sample tables */
    db->exec("create table qsql_master "
	     "(id numeric(10) primary key,"
	     "name char(30));");
    
    QSqlRecord* buf;
    
    QSqlCursor master( "qsql_master" );
    buf = master.editBuffer( TRUE );
    buf->setValue( "id", 1 );
    buf->setValue( "name", "Trolltech" );
    master.insert();
    buf->setValue( "id", 2 );
    buf->setValue( "name", "ACME" );
    master.insert();
    buf->setValue( "id", 3 );
    buf->setValue( "name", "MS" );
    master.insert();

    db->exec("create table qsql_child "
	     "(id numeric(10) primary key,"
	     "masterid numeric(10),"
	     "name char(30));");
    QSqlCursor child( "qsql_child" );
    buf = child.editBuffer( TRUE );
    
    buf->setValue( "id", 1 );
    buf->setValue( "masterid", 1 );
    buf->setValue( "name", "db" );
    child.insert();

    buf->setValue( "id", 2 );
    buf->setValue( "masterid", 1 );
    buf->setValue( "name", "trond" );
    child.insert();

    buf->setValue( "id", 3 );
    buf->setValue( "masterid", 2 );
    buf->setValue( "name", "roadrunner" );
    child.insert();

    buf->setValue( "id", 4 );
    buf->setValue( "masterid", 2 );
    buf->setValue( "name", "coyote" );
    child.insert();

    buf->setValue( "id", 5 );
    buf->setValue( "masterid", 2 );
    buf->setValue( "name", "chicken" );
    child.insert();

    buf->setValue( "id", 6 );
    buf->setValue( "masterid", 3 );
    buf->setValue( "name", "bill" );
    child.insert();

    buf->setValue( "id", 7 );
    buf->setValue( "masterid", 3 );
    buf->setValue( "name", "steve" );
    child.insert();

    MainWindow* w = new MainWindow();
    a.setMainWidget( w );
    w->show();
    int x = a.exec();

    /* clean up */
    db->exec( "drop table qsql_child;");
    db->exec( "drop table qsql_master;");

    delete w;
    return x;
};


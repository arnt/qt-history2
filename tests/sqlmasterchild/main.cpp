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
    QSqlCursor master( "qsql_master" );
    master["id"] = 1;
    master["name"] = "Trolltech";
    master.insert();
    master["id"] = 2;
    master["name"] = "ACME";
    master.insert();
    master["id"] = 3;
    master["name"] = "MS";
    master.insert();

    db->exec("create table qsql_child "
	     "(id numeric(10) primary key,"
	     "masterid numeric(10),"
	     "name char(30));");
    QSqlCursor child( "qsql_child" );
    child["id"] = 1;
    child["masterid"] = 1;
    child["name"] = "db";
    child.insert();

    child["id"] = 2;
    child["masterid"] = 1;
    child["name"] = "trond";
    child.insert();

    child["id"] = 3;
    child["masterid"] = 2;
    child["name"] = "roadrunner";
    child.insert();

    child["id"] = 4;
    child["masterid"] = 2;
    child["name"] = "coyote";
    child.insert();

    child["id"] = 5;
    child["masterid"] = 2;
    child["name"] = "chicken";
    child.insert();


    child["id"] = 6;
    child["masterid"] = 3;
    child["name"] = "bill";
    child.insert();

    child["id"] = 7;
    child["masterid"] = 3;
    child["name"] = "steve";
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


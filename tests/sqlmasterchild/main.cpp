#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlview.h>
#include "mainwindow.h"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QSqlConnection::addDatabase( qApp->argv()[1],
				 qApp->argv()[2],
				 qApp->argv()[3],
				 qApp->argv()[4],
				 qApp->argv()[5]);
    QSqlDatabase* db = QSqlConnection::database();

    // create sample tables
    db->exec("create table qsql_master "
	     "(id numeric(10) primary key,"
	     "name char(30));");
    QSqlView masterView( "qsql_master" );
    masterView.setMode( SQL_Writable );
    masterView["id"] = 1;
    masterView["name"] = "Trolltech";
    masterView.insert();
    masterView["id"] = 2;
    masterView["name"] = "ACME";
    masterView.insert();
    masterView["id"] = 3;
    masterView["name"] = "MS";
    masterView.insert();

    db->exec("create table qsql_child "
	     "(id numeric(10) primary key,"
	     "masterid numeric(10),"
	     "name char(30));");
    QSqlView childView( "qsql_child" );
    childView.setMode( SQL_Writable );
    childView["id"] = 1;
    childView["masterid"] = 1;
    childView["name"] = "db";
    childView.insert();

    childView["id"] = 2;
    childView["masterid"] = 1;
    childView["name"] = "trond";
    childView.insert();

    childView["id"] = 3;
    childView["masterid"] = 2;
    childView["name"] = "roadrunner";
    childView.insert();

    childView["id"] = 4;
    childView["masterid"] = 2;
    childView["name"] = "coyote";
    childView.insert();

    childView["id"] = 5;
    childView["masterid"] = 2;
    childView["name"] = "chicken";
    childView.insert();


    childView["id"] = 6;
    childView["masterid"] = 3;
    childView["name"] = "bill";
    childView.insert();

    childView["id"] = 7;
    childView["masterid"] = 3;
    childView["name"] = "steve";
    childView.insert();


    MainWindow* w = new MainWindow();
    a.setMainWidget( w );
    w->show();
    int x = a.exec();

    // clean up
    db->exec( "drop table qsql_child;");
    db->exec( "drop table qsql_master;");

    delete w;
    return x;
};


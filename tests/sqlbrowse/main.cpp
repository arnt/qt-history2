#include <qapplication.h>
#include <qsqldatabase.h>
#include "resultwindow.h"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QSqlDatabase* database = new QSqlDatabase( qApp->argv()[1] );
    database->reset( qApp->argv()[2], qApp->argv()[3], qApp->argv()[4], qApp->argv()[5]);
    database->open();
    ResultWindow* rw = new ResultWindow( database );
    a.setMainWidget( rw );
    rw->show();
    int x = a.exec();
    //    delete rw;
    database->close();
    delete database;
    return x;
};


#include <qapplication.h>
#include <qsqldatabase.h>
#include "resultwindow.h"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QSqlDatabase* db = QSqlDatabase::addDatabase( qApp->argv()[1] );
    db->setDatabaseName( qApp->argv()[2] );
    db->setUserName( qApp->argv()[3] );
    db->setPassword( qApp->argv()[4] );
    db->setHostName( qApp->argv()[5] );

    ResultWindow* rw = new ResultWindow();
    a.setMainWidget( rw );
    rw->show();
    int x = a.exec();
    delete rw;
    return x;
};


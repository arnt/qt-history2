#include <qapplication.h>
#include "sqlquery.h"


int main( int argc, char **argv )
{
    QApplication a(argc,argv);

    ResultWindow* rw = new ResultWindow();
    a.setMainWidget( rw );
    rw->show();
    int x = a.exec();
    delete rw;
    return x;
};


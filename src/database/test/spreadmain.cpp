#include <qapplication.h>
#include "sqlspreadsheet.h"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    SQLSpreadsheetWindow* w = new SQLSpreadsheetWindow();
    a.setMainWidget( w );
    w->show();
    int x = a.exec();
    delete w;
    return x;
};


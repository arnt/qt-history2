#include "mainwindow.h"

#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    MainWindow mw;
    app.setMainWidget( &mw );
    mw.show();
    
    return app.exec();
}

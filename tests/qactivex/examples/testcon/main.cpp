#include <qapplication.h>
#include "mainwindow.h"

int main( int argc, char **argv ) 
{
    QApplication app( argc, argv );

    MainWindow mw;
    mw.show();

    app.setMainWidget( &mw );
    return app.exec();;
}

#include <qapplication.h>
#include <qmotif.h>

#include "mainwindow.h"


int main( int argc, char **argv )
{
    XtSetLanguageProc( NULL, NULL, NULL );

    QMotif integrator( "dialog" );
    QApplication app( argc, argv );

    MainWindow mainwindow;
    app.setMainWidget( &mainwindow );
    mainwindow.show();

    return app.exec();
}

#include "mainwindow.h"

#include <qapplication.h>
#include <qmotif.h>


int main( int argc, char **argv )
{
    XtSetLanguageProc( NULL, NULL, NULL );

    QMotif integrator( "customwidget" );
    QApplication app( argc, argv );

    MainWindow mainwindow;
    app.setMainWidget( &mainwindow );
    mainwindow.show();

    return app.exec();
}

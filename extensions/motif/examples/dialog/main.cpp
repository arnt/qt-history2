#include <qapplication.h>
#include <qmotif.h>
#include <qmotifeventloop.h>

#include "mainwindow.h"


int main( int argc, char **argv )
{
    XtSetLanguageProc( NULL, NULL, NULL );

    QMotif integrator;
    QMotifEventLoop eventloop( &integrator );
    QApplication app( argc, argv );

    integrator.initialize( &argc, argv, "dialog", NULL, 0 );

    MainWindow mainwindow;
    app.setMainWidget( &mainwindow );
    mainwindow.show();

    return app.exec();
}

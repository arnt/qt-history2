#include "mainwindow.h"

#include <qapplication.h>
#include <qmotif.h>
#include <qmotifeventloop.h>


int main( int argc, char **argv )
{
    XtSetLanguageProc( NULL, NULL, NULL );

    QMotif integrator;
    QMotifEventLoop eventloop( &integrator);
    QApplication app( argc, argv );

    integrator.initialize( &argc, argv, "customwidget", NULL, 0 );

    MainWindow mainwindow;
    app.setMainWidget( &mainwindow );
    mainwindow.show();

    return app.exec();
}

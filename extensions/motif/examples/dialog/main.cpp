#include <qapplication.h>
#include <qmotif.h>
#include <qmotifeventloop.h>

#include "mainwindow.h"


int main( int argc, char **argv )
{
    XtSetLanguageProc( NULL, NULL, NULL );

    QMotif integrator;
    QApplication app( argc, argv );
    QMotifEventLoop eventloop( &integrator, &app );
    app.setEventLoop( &eventloop );

    integrator.initialize( &argc, argv, "dialog", NULL, 0 );

    MainWindow mainwindow;
    app.setMainWidget( &mainwindow );

    XtManageChild( mainwindow.motifWidget() );
    XtRealizeWidget( XtParent( mainwindow.motifWidget() ) );

    return app.exec();
}

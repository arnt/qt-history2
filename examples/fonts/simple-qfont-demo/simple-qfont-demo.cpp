/* $Id$ */

#include "viewer.h"

#include <qapplication.h>
 
int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    Viewer * textViewer = new Viewer();

    app.setMainWidget( textViewer );
    textViewer->show();
    return app.exec();
}                  

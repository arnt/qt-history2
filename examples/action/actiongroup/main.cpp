/*
$Id$
*/  

#include <qapplication.h>

#include "editor.h"

int main( int argc, char ** argv)
{
    QApplication app( argc, argv );
    Editor editor;
    editor.setCaption( "Qt Example - Actiongroup" );
    app.setMainWidget( &editor );
    editor.show();
    return app.exec();
}

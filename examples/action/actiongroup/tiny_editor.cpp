/*
$Id$
*/  

#include <qapplication.h>

#include "editor.h"

int main( int argc, char ** argv)
{
    QApplication app( argc, argv );
    Editor * editor = new Editor;

    app.setMainWidget( editor );
    editor->show();
    return app.exec();
}

#include <qapplication.h>
#include "inspectorimpl.h"

#include <qvariant.h>

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    LibraryInspector view( 0, 0, TRUE );

    app.setMainWidget( &view );
    view.exec();

    return 0;
}

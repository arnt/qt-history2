#include <qapplication.h>

#include "plugmainwindow.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    // little test scenario
    PlugMainWindow mw;

    // the rest ist silence...
    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}

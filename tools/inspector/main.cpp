#include <qapplication.h>
#include "inspector.h"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    InspectorBase main;
    app.setMainWidget( &main );
    main.show();

    return app.exec();
}

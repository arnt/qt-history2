/*
$Id$
*/

#include "converter.h"

#include <qapplication.h>

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );

    EuroConverter * euroconvert = new EuroConverter();

    app.setMainWidget( euroconvert );
    euroconvert->show();
    return app.exec();
}

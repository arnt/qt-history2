/*
  main.cpp
*/

#include <qapplication.h>

#include "clock.h"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    Clock clock;
    clock.resize( 140, 60 );
    app.setMainWidget( &clock );
    clock.show();
    return app.exec();
}

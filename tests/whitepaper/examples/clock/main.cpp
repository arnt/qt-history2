/*
  main.cpp
*/

#include <qapplication.h>

#include "clock.h"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    Clock *clock = new Clock;
    app.setMainWidget( clock );
    clock->show();
    return app.exec();
}

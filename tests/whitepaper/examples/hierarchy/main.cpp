/*
  main.cpp
*/

#include <qapplication.h>

#include "classhierarchy.h"
    
int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    ClassHierarchy *hierarchy = new ClassHierarchy;
    app.setMainWidget( hierarchy );
    hierarchy->show();
    return app.exec();
}

/*
  main.cpp
*/

#include <qapplication.h>

#include "converter.h"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    Converter *converter = new Converter;
    converter->setCaption( "Live Currency" );
    app.setMainWidget( converter );
    converter->show();
    return app.exec();
}

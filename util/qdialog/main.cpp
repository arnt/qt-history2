#include "mainwindow.h"

#include <qapplication.h>

int main( int argc, char** argv )
{
  // QApplication::useXResourceManager( false );
  QApplication app( argc, argv );

  DMainWindow* w = new DMainWindow;
  app.setMainWidget( w );

  if ( argc > 1 )
    w->slotOpen( argv[1] );

  w->show();

  app.exec();

  return 0;
}

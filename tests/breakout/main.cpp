#include "wall.h"

#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );			

    GameMain game;
    game.show();

    app.setMainWidget( &game );
    return app.exec();
}

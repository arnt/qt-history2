/****************************************************************************
** Qt tutorial 14
*****************************************************************************/

#include <qapp.h>

#include "gamebrd.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    GameBoard gb;
    gb.setGeometry( 100, 100, 500, 355 );
    a.setMainWidget( &gb );
    gb.show();
    return a.exec();
}

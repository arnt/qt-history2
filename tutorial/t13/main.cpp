/****************************************************************
**
** Qt tutorial 13
**
****************************************************************/

#include <qapplication.h>

#include "gamebrd.h"


int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a( argc, argv );

    GameBoard gb;
    a.setMainWidget( &gb );
    gb.show();
    return a.exec();
}

/****************************************************************
**
** Qt tutorial 14
**
****************************************************************/

#include "gamebrd.h"
#include <qapp.h>
#include <qdatetm.h>

#include <stdlib.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QTime midnight( 0, 0, 0 );
    srand( midnight.secsTo(QTime::currentTime()) );

    GameBoard gb;
    gb.setGeometry( 100, 100, 500, 355 );
    a.setMainWidget( &gb );
    gb.show();
    return a.exec();
}

/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <stdlib.h>
#include "tictac.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    int n = 3;
    if ( argc == 2 )				// get board size n
        n = atoi(argv[1]);
    if ( n < 3 || n > 10 ) {			// out of range
        qWarning( "%s: Board size must be from 3x3 to 10x10", argv[0] );
        return 1;
    }
    TicTacToe ttt( n );				// create game
    a.setMainWidget( &ttt );
    ttt.setWindowTitle("Qt Example - TicTac");
    ttt.show();					// show widget
    return a.exec();				// go
}

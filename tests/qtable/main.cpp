/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtable.h"
#include <qpushbutton.h>
#include <qapplication.h>


/*
  Constants
*/

const int numRows = 100;				// Tablesize: number of rows
const int numCols = 100;				// Tablesize: number of columns

/*
  The program starts here.
*/

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    QTable v( numRows, numCols );

    a.setMainWidget( &v );
    v.show();
    return a.exec();
}

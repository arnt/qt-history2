/****************************************************************************
** $Id: //depot/qt/main/examples/table/main.cpp#2 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

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

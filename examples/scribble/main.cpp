/****************************************************************************
** $Id: //depot/qt/main/examples/scribble/main.cpp#1 $
**
** Copyright ( C ) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "scribble.h"
#include <qapplication.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    Scribble* scribble = new Scribble;
    scribble->resize( 500, 350 );
    a.setMainWidget( scribble );
    scribble->show();

    int res = a.exec();

    delete scribble;
    return res;
}

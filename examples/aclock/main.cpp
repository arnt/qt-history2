/****************************************************************************
** $Id: //depot/qt/main/examples/aclock/main.cpp#1 $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "aclock.h"
#include <qapplication.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    AnalogClock *clock = new AnalogClock;
    clock->resize( 100, 100 );
    a.setMainWidget( clock );
    clock->show();
    return a.exec();
}

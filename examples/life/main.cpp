/****************************************************************************
** $Id: //depot/qt/main/examples/life/main.cpp#1 $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "lifedlg.h"
#include <qapplication.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    LifeDialog *life = new LifeDialog;
    a.setMainWidget( life );
    life->show();

    return a.exec();
}

/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/main.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "sheetdlg.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.resize(400,300);
    a.setMainWidget(&w);
    w.show();
    return a.exec();
}

/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "mainwindow.h"
#include "qfileiconview.h"

#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    FileMainWindow mw;
    mw.resize( 640, 480 );
    mw.fileView()->setDirectory( "/" );
    a.setMainWidget( &mw );
    mw.show();

    return a.exec();
}

/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/main.cpp#1 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtmainwindow.h"

#include <qapplication.h>

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QTMainWindow mw( "mainwindow" );
    mw.resize( 640, 480 );
    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}

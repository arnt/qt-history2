/****************************************************************************
** $Id: //depot/qt/main/examples/dirview/main.cpp#1 $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "dirview.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    QListView mw;
    a.setMainWidget( &mw );
    mw.setCaption( "Directory Browser" );
    mw.addColumn( "Name" );
    mw.addColumn( "Type" );
    mw.resize( 400, 400 );
    mw.setTreeStepSize( 20 );
    Directory * root = new Directory( &mw );
    root->setOpen( TRUE );
    mw.show();
    return a.exec();
}

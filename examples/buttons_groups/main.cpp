/****************************************************************************
** $Id: //depot/qt/main/examples/buttons_groups/main.cpp#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "buttons_groups.h"
#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    ButtonsGroups buttonsgroups;
    buttonsgroups.resize( 500, 250 );
    buttonsgroups.setCaption( "Examples for Buttons and Groups" );
    a.setMainWidget( &buttonsgroups );
    buttonsgroups.show();

    return a.exec();
}

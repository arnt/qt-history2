/****************************************************************************
** $Id: //depot/qt/main/examples/checklists/main.cpp#1 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "checklists.h"
#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    CheckLists checklists;
    checklists.resize( 650, 350 );
    checklists.setCaption( "CheckList Examples" );
    a.setMainWidget( &checklists );
    checklists.show();

    return a.exec();
}

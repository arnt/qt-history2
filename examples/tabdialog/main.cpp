/****************************************************************************
** $Id: //depot/qt/main/examples/tabdialog/main.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "tabdialog.h"
#include <qapplication.h>
#include <qstring.h>

int main( int argc, char **argv )
{
    if (argc < 2) {
        qWarning( "\n\n\n" );
        qWarning( "--------------------------" );
        qWarning( "Usage: tabdialog <filename>" );
        qWarning( "--------------------------" );
        qWarning( "\n\n\n" );
        return 0;
    }

    QApplication a( argc, argv );

    TabDialog tabdialog( 0L, "tabdialog", QString( argv[1] ) );
    tabdialog.resize( 450, 350 );
    tabdialog.setCaption( "Example for a Tabbed Dialog" );
    a.setMainWidget( &tabdialog );
    tabdialog.show();

    return a.exec();
}

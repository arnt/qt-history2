/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "buttongroups.h"
#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    ButtonsGroups buttonsgroups;
    buttonsgroups.resize( 500, 250 );
    buttonsgroups.setWindowTitle( "Qt Example - Buttongroups" );
    a.setMainWidget( &buttonsgroups );
    buttonsgroups.show();

    return a.exec();
}

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

#include <qapplication.h>

#include "composer.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    Composer c;
    a.setMainWidget( &c );
    c.resize( 400, 500 );
    c.show();
    return a.exec();
}

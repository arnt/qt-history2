/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <qapplication.h>
#include "mainwindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainWindow w;
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}

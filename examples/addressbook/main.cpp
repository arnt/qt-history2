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

#include "mainwindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    ABMainWindow *mw = new ABMainWindow();
    mw->setWindowTitle( "Qt Example - Addressbook" );
    a.setMainWidget( mw );
    mw->show();

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    int result = a.exec();
    delete mw;
    return result;
}

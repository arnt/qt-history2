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
#include <qdesktopwidget.h>
#include "qwerty.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );


    bool isSmall =  qApp->desktop()->size().width() < 450
		  || qApp->desktop()->size().height() < 450;

    int i;
    for ( i= argc <= 1 ? 0 : 1; i<argc; i++ ) {
	Editor *e = new Editor;
	e->setWindowTitle("Qt Example - QWERTY");
	if ( i > 0 )
	    e->load( argv[i] );
	if ( isSmall ) {
	    e->showMaximized();
	} else {
	    e->resize( 400, 400 );
	    e->show();
	}
    }
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}

/****************************************************************************
** $Id: //depot/qt/main/examples/qwerty/main.cpp#1 $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "qwerty.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    int i;
    for ( i=0; i<argc; i++ ) {
	Editor *e = new Editor;
	e->resize( 400, 400 );
	if ( i > 0 )
	    e->load( argv[i] );
	e->show();
    }
    QObject::connect( &a, SIGNAL(lastWindowClosed()),
		      &a, SLOT(quit()) );
    return a.exec();
}

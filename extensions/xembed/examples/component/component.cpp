/****************************************************************************
** $Id: //depot/qt/main/examples/application/main.cpp#4 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qxembed.h>
#include <qapplication.h>
#include <qmultilineedit.h>
#include <stdio.h>

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    QMultiLineEdit edit;
    edit.setText(
		 "Hi!\n\nI'm the external 'component' application, embedded\n"
		 "into the 'shell' program. You don't believe me? Just kill my process\n"
		 "and restart me again." 
		 );
    a.setMainWidget( &edit );
    
    if ( !QXEmbed::processClientCmdline( &edit, argc, argv ) ) {
	fprintf( stderr, "Usage: %s -embed <window-id>\n", argv[0] );
	exit( 1 );
    }
    return a.exec();
}

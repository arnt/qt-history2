/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qapplication.h>
#include <stdlib.h>
#include <stdio.h>

#include "qvfb.h"

void usage( const char *app )
{
    printf( "Usage: %s [-width width] [-height height] [-depth depth] [-nocursor]\n", app );
}

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    int width = 240;
    int height = 320;
    int depth = 8;
    bool cursor = true;

    for ( int i = 1; i < argc; i++ ){
	QString arg = argv[i];
	if ( arg == "-width" ) {
	    width = atoi( argv[++i] );
	} else if ( arg == "-height" ) {
	    height = atoi( argv[++i] );
	} else if ( arg == "-depth" ) {
	    depth = atoi( argv[++i] );
	} else if ( arg == "-nocursor" ) {
	    cursor = false;
	} else {
	    printf( "Unknown parameter %s\n", arg.latin1() );
	    usage( argv[0] );
	    exit(1);
	}
    }

    QVFb *mw = new QVFb( width, height, depth );
    app.setMainWidget( mw );
    mw->enableCursor(cursor);
    mw->show();

    app.exec();

    delete mw;
}



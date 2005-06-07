/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qvfb.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qregexp.h>

#include <stdlib.h>
#include <stdio.h>

void usage( const char *app )
{
    printf( "Usage: %s [-width width] [-height height] [-depth depth] "
	    "[-nocursor] [-qwsdisplay :id]\n"
	    "Supported depths: 1, 4, 8, 32\n", app );
}

int main( int argc, char *argv[] )
{
    Q_INIT_RESOURCE(qvfb);

    QApplication app( argc, argv );

    int width = 240;
    int height = 320;
    int depth = 32;
    bool cursor = true;
    QString displaySpec( ":0" );
    QString skin;

    for ( int i = 1; i < argc; i++ ){
	QString arg = argv[i];
	if ( arg == "-width" ) {
	    width = atoi( argv[++i] );
	} else if ( arg == "-height" ) {
	    height = atoi( argv[++i] );
	} else if ( arg == "-skin" ) {
	    skin = argv[++i];
	} else if ( arg == "-depth" ) {
	    depth = atoi( argv[++i] );
	} else if ( arg == "-nocursor" ) {
	    cursor = false;
	} else if ( arg == "-qwsdisplay" ) {
	    displaySpec = argv[++i];
	} else {
	    printf( "Unknown parameter %s\n", arg.latin1() );
	    usage( argv[0] );
	    exit(1);
	}
    }

    int displayId = 0;
    QRegExp rx( ":[0-9]" );
    int m = rx.indexIn(displaySpec, 0);
    if ( m >= 0 ) {
	displayId = displaySpec.mid( m+1, rx.matchedLength()-1 ).toInt();
    }

    qDebug( "Using display %d", displayId );

    QVFb mw( displayId, width, height, depth, skin );
    app.setMainWidget( &mw );
    mw.enableCursor(cursor);
    mw.show();

    return app.exec();
}

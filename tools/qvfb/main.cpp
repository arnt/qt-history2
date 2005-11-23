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

#include <QApplication>
#include <QRegExp>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

void fn_quit_qvfb(int)
{
    // pretend that we have quit normally
    qApp->quit();
}


void usage( const char *app )
{
    printf( "Usage: %s [-width width] [-height height] [-depth depth] "
	    "[-nocursor] [-qwsdisplay :id] [-skin skindirectory]\n"
	    "Supported depths: 1, 4, 8, 32\n", app );
}
int qvfb_protocol = 0;

int main( int argc, char *argv[] )
{
    Q_INIT_RESOURCE(qvfb);

    QApplication app( argc, argv );

    int width = 0;
    int height = 0;
    int depth = 32;
    int rotation = 0;
    bool cursor = true;
    double zoom = 1.0;
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
	} else if ( arg == "-mmap" ) {
	    qvfb_protocol = 1;
	} else if ( arg == "-zoom" ) {
	    zoom = atof( argv[++i] );
	} else if ( arg == "-qwsdisplay" ) {
	    displaySpec = argv[++i];
	} else {
	    printf( "Unknown parameter %s\n", arg.latin1() );
	    usage( argv[0] );
	    exit(1);
	}
    }

    int displayId = 0;
    QRegExp r( ":[0-9]+" );
    int m = r.indexIn( displaySpec, 0 );
    int len = r.matchedLength();
    if ( m >= 0 ) {
	displayId = displaySpec.mid( m+1, len-1 ).toInt();
    }
    QRegExp rotRegExp( "Rot[0-9]+" );
    m = rotRegExp.indexIn( displaySpec, 0 );
    len = r.matchedLength();
    if ( m >= 0 ) {
	rotation = displaySpec.mid( m+3, len-3 ).toInt();
    }

    qDebug( "Using display %d", displayId );
    signal(SIGINT, fn_quit_qvfb);
    signal(SIGTERM, fn_quit_qvfb);

    QVFb mw( displayId, width, height, depth, rotation, skin );
    mw.setZoom(zoom);
    app.setMainWidget( &mw );
    mw.enableCursor(cursor);
    mw.show();

    return app.exec();
}

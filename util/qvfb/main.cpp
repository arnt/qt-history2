

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



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
    app.setOrganizationDomain("trolltech.com");
    app.setOrganizationName("Trolltech");
    app.setApplicationName("QVFb");

    QVFb mw;
    for ( int i = 1; i < argc; i++ ){
	QString arg = argv[i];
	if ( arg == "-width" ) {
	    mw.setWidth(atoi(argv[++i]));
	} else if ( arg == "-height" ) {
	    mw.setHeight(atoi(argv[++i]));
	} else if ( arg == "-skin" ) {
	    mw.setSkin(argv[++i]);
	} else if ( arg == "-depth" ) {
	    mw.setDepth(atoi(argv[++i]));
	} else if ( arg == "-nocursor" ) {
	    mw.setCursor(false);
	} else if ( arg == "-mmap" ) {
	    qvfb_protocol = 1;
	} else if ( arg == "-zoom" ) {
	    mw.setZoom(atof(argv[++i]));
	} else if ( arg == "-qwsdisplay" ) {
	    mw.setDisplay(argv[++i]);
	} else {
	    printf( "Unknown parameter %s\n", arg.latin1() );
	    usage( argv[0] );
	    exit(1);
	}
    }
    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}

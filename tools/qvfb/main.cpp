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
#include "qvfboptions.h"

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

    QVFbOptions(argc, argv);

    if (argc > 1) {
        // argc and argv hold the failed.
        
        printf( "Unknown parameter %s\n", argv[1] );
        usage( argv[0] );
        exit(1);
    }

    QVFb mw;
    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}

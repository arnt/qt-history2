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

#include "lifedlg.h"
#include <qapplication.h>
#include <stdlib.h>
 
void usage()
{
    qWarning( "Usage: life [-scale scale]" );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    int scale = 10;

    for ( int i = 1; i < argc; i++ ){
        QString arg = argv[i];
	if ( arg == "-scale" )
	    scale = atoi( argv[++i] );
	else {
	    usage();
	    exit(1);
	}
    }

    if ( scale < 2 )
	scale = 2;

    LifeDialog *life = new LifeDialog( scale );
    a.setMainWidget( life );
    life->setWindowTitle("Qt Example - Life");
    life->show();

    return a.exec();
}

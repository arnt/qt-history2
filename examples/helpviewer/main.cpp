/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "helpwindow.h"
#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qstylesheet.h>
#include <stdlib.h>


int main( int argc, char ** argv )
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication a(argc, argv);

    QString home;
    if (argc > 1) {
        home = argv[1];
    } else {
        home = QString( qInstallPathDocs() ) + "/html/index.html";

	// if $QTDIR is set, use that instead
	const char *qtdirenv = getenv( "QTDIR" );
	if ( qtdirenv ) {
	    home = QString( qtdirenv ) + "/doc/html/index.html";
	}
    }

    HelpWindow *help = new HelpWindow(home, ".", 0, "help viewer");
    help->setCaption("Qt Example - Helpviewer");
    if ( QApplication::desktop()->width() > 400
	 && QApplication::desktop()->height() > 500 )
	help->show();
    else
	help->showMaximized();

    QObject::connect( &a, SIGNAL(lastWindowClosed()),
                      &a, SLOT(quit()) );

    return a.exec();
}

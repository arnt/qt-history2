/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include "application.h"
#include "glinfo.h"
#include <qstring.h>

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    int res;
    if ( argc > 1 && QString(argv[1]) == "-i" ) {
	GLInfo info(0);
	info.resize( 640, 480 );
	res = info.exec();
    } else if ( argc > 1 && QString(argv[1]) == "-l" ) {
	GLInfo info(0);
	QString s = info.getText();
	QStringList l = info.getViewList();
	qWarning( s );
	//         .... .. ....... . .. . ....    . . . . . . . .. .. .. .. .. .. . .
	qWarning( " Id  CB Draw to T Bz L Type    D S R G B A X Dp St Ra Ga Ba Aa N B\n"
		  "------------------------------------------------------------------" );
	QStringList::ConstIterator it;
	for ( it = l.begin(); it != l.end(); ++it )
	    qWarning( *it );
    } else {
	ApplicationWindow * mw = new ApplicationWindow();
	mw->setCaption( "Qt Example - Open GL (MDI)" );
	mw->show();
	a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
	res = a.exec();
    }
    return res;
}

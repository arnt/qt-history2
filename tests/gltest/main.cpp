/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

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
	//         .... .. ....... . .. . .... . . . . . . . .. .. .. .. .. .. . .
	qWarning( " Id  CB Draw to T Bz L Type D S R G B A X Dp St Ra Ga Ba Aa N B\n"
		  "---------------------------------------------------------------" );
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

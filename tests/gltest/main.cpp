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
    } else {
	ApplicationWindow * mw = new ApplicationWindow();
	mw->setCaption( "Qt Example - Open GL (MDI)" );
	mw->show();
	a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
	res = a.exec();
    }
    return res;
}

/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#8 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "mainwindow.h"
#include "qfileiconview.h"

#include <qapplication.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    if ( argc == 2 && QString( "-desktop" ) == argv[1] ) {
	QtFileIconView fiv( QString::null, TRUE );
	a.setMainWidget( &fiv );
	fiv.setFrameStyle( QFrame::NoFrame );
	fiv.setCaption( "desktop" );
	fiv.showMaximized();
	fiv.setSelectionMode( QIconView::StrictMulti );
	fiv.setViewMode( QIconSet::Large );
	fiv.setDirectory( "/" );
	return a.exec();
    } else {
	FileMainWindow mw;
	mw.resize( 680, 480 );
	a.setMainWidget( &mw );
	mw.show();
	mw.fileView()->setDirectory( "/" );
	return a.exec();
    }
}

/****************************************************************************
** $Id: //depot/qt/main/examples/kiosk/main.cpp#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <qapplication.h>
#include <qdir.h>
#include "kioskwidget.h"
#include "../themes/wood.h"

main(int argc, char **argv)
{
    QApplication app(argc,argv);
    QApplication::setFont( QFont( "helvetica", 16, QFont::Bold ) );
    QApplication::setStyle( new NorwegianWoodStyle( 30 ) );
    QDir dir( "." );

    QStringList files;
    if ( argc > 1 ) {
	for ( int i = 1; i < argc ; i++ )
	    if ( dir.exists( argv[i] )  )
		      files.append( argv[i] );
    } 
    
    if ( files.count() < 1 ) {
	files = dir.entryList( "*.mpg" );
    }
    
    KioskWidget mw( files );
    mw.showMaximized();

    
    QObject::connect(qApp,SIGNAL(lastWindowClosed()),qApp,SLOT(quit()));
    return app.exec();
}

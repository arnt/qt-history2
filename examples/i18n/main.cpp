/****************************************************************************
** $Id: //depot/qt/main/examples/i18n/main.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qtranslatordialog.h>

#include "mywidget.h"

main( int argc, char** argv )
{
	QString lfile;

	if ( argc != 2 ) {
		qWarning( "\n\n\nUsage: i18n <language> (where <language> can be 'de', 'en' and 'no')\n\n" );
        return 0;
    } else {
		if ( QString( argv[1] ) == "de" )
			lfile = "mywidget_de.tr";
		else if ( QString( argv[1] ) == "no" ||
				  QString( argv[1] ) == "en" )
			lfile = "mywidget_en.tr";
		else {
			qWarning( "\n\n\nUsage: i18n <language> (where <language> can be 'de', 'en' and 'no')\n\n" );
            return 0;
		}
    }

	QApplication app( argc, argv );

	//QAppTranslator t;

	QTranslator translator( 0 );
	translator.load( lfile, "." );
	app.installTranslator( &translator );

	MyWidget m( 0, QString( argv[1] ), "mainwindow" );
	m.resize( 400, 300 );
	app.setMainWidget( &m );
	m.show();

	return app.exec();
}

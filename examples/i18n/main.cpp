/****************************************************************************
** $Id: //depot/qt/main/examples/i18n/main.cpp#4 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qtranslator.h>
#include <qfileinfo.h>

#include "mywidget.h"

main( int argc, char** argv )
{
    QApplication app( argc, argv );

    if ( argc != 2 ) {
	qWarning( "\n\nUsage: %s <language> (where <language> can be 'de', 'en', 'no'...)\n", argv[0] );
        return 0;
    }
    QString lang( argv[1] );

    QString lfile = "mywidget_" + lang + ".qm";

    QFileInfo fi( lfile );
    if ( !fi.exists() ) {
    	qWarning( "\n%s: cannot find translation for language '%s'" 
		  " (try 'de', 'en' or 'no')\n", argv[0], argv[1] );
	return 0;
    }

    QTranslator translator( 0 );
    translator.load( lfile, "." );
    app.installTranslator( &translator );

    MyWidget m;
    m.resize( 400, 300 );
    app.setMainWidget( &m );
    m.show();

    return app.exec();
}

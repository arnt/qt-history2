/****************************************************************************
** $Id: //depot/qt/main/examples/i18n/main.cpp#5 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qtranslator.h>
#include <qfileinfo.h>
#include <qmessagebox.h>

#include "mywidget.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QString lang;
    if ( argc != 2 ) {
	int i = QMessageBox::information(0, "Language?", "Which language?",
		    "Deutsch", "English", "Norsk" );
	switch ( i ) {
	  case 0: lang = "de"; break;
	  case 1: lang = "en"; break;
	  case 2: lang = "no"; break;
	}
    } else {
	lang = argv[1];
    }

    QString lfile = "mywidget_" + lang + ".qm";

    QFileInfo fi( lfile );
    if ( !fi.exists() ) {
    	QMessageBox::warning( 0, "File error",
		  QString("Cannot find translation for language: "+lang+
		  "\n(try 'de', 'en' or 'no')") );
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

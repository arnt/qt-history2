/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#23 $
**
** Main program of Qt/FB central server
**
** Created : 991214
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qapplication.h>

#include "qws.h"
#include "qws_gui.h"

#include <stdlib.h>

main(int argc, char** argv)
{
#ifdef _WS_X11_
    bool useGUI = getenv( "DISPLAY" ) != 0;
#else
    bool useGUI = FALSE;
#endif
    QApplication app(argc, argv, useGUI);

    if ( useGUI ) {
	int depth=32;
	int refresh_delay=50;
	while ( argc > 1 && argv[1][0]=='-' ) {
	    if ( argc > 2 ) {
		switch ( argv[1][1] ) {
		  case 'r':
		    argc--; argv++;
		    refresh_delay = atoi(argv[1]);
		    break;
		  case 'd':
		    argc--; argv++;
		    depth = atoi(argv[1]);
		    break;
		}
		argc--; argv++;
	    }
	}
	DebuggingGUI m;
	app.setMainWidget(&m);
	m.show();
	m.serve(depth,refresh_delay);
	return app.exec();
    } else {
	int w = 0;
	int h = 0;
	if ( argc > 1 ) {
	    QString s = argv[1];
	    int x = s.find( "x" );
	    if ( x > 0 ) {
		bool ok;
		QString sub = s.mid(0,x);
		w = sub.toInt(&ok);
		sub = s.mid(x+1);
		if ( ok )
		    h = sub.toInt(&ok);
		if ( !ok ) {
		    w = 0;
		    h = 0;
		}
		qDebug( "%dx%d", w, h );
	    }
	}
	(void)new QWSServer( w, h );
	return app.exec();
    }
}

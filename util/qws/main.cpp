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
	int refresh_delay=500;
	if ( argc > 1 ) {
	    refresh_delay = atoi(argv[1]);
	}
	DebuggingGUI m;
	app.setMainWidget(&m);
	m.show();
	m.serve(refresh_delay);
	return app.exec();
    } else {
	(void)new QWSServer;
	return app.exec();
    }
}

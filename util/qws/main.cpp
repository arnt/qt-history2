/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>

#include "qws.h"
#include "qws_gui.h"

#include <stdlib.h>
#include <signal.h>

static QWSServer *server = 0;

void handleHardExit(int sig);

main(int argc, char** argv)
{
#ifdef Q_WS_X11
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
	signal(SIGINT, handleHardExit);
	signal(SIGTERM, handleHardExit);
	signal(SIGQUIT, handleHardExit);
	signal(SIGTERM, handleHardExit);
	signal(SIGSEGV, handleHardExit);
	int w = 0;
	int h = 0;
	int flags = QWSServer::DisableKeyboard;
	int i = 1;
	while ( i < argc  ) {
	    //OK, OK this isn't the world's best command line parser
	    bool ok = FALSE;
	    QCString s = argv[i++];
	    if ( s[0] == '-' ) {
		ok = TRUE;
		if ( s.find( 'a' ) > 0 )
		    flags |= QWSServer::DisableAccel;
		if ( s.find( 'm' ) > 0 )
		    flags |= QWSServer::DisableMouse;
		if ( s.find( 'k' ) > 0 )
		    flags |= QWSServer::DisableKeyboard;
		if ( s.find( 'h' ) > 0 )
		    ok = FALSE; //give help
	    } else if ( s[0] == '+' ) {
		ok = TRUE;
		if ( s.find( 'k' ) > 0 )
		    flags &= ~QWSServer::DisableKeyboard;
		else
		    ok = FALSE;
	    } else {
		int x = s.find( 'x' );
		if ( x > 0 ) {
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
	    if ( !ok ) {
		qDebug( "usage %s [-kma] [+k] [widthxheight]", argv[0] );
		qDebug( "  -k  Disable keyboard [default]" );
		qDebug( "  +k  Enable keyboard" );
		qDebug( "  -m  Disable mouse" );
		qDebug( "  -a  Disable acceleration (not yet implemented)" );
		exit(1);
	    }
	}
	server = new QWSServer( w, h, 0, flags );
	int rv = app.exec();
	delete server;
	return rv;
    }
}

void handleHardExit(int sig)
{
    if (server) {
	delete server;	// will cleanup device connections.
	qApp->quit(); // leave loop
    }
    if (sig == SIGSEGV)
	abort();
}


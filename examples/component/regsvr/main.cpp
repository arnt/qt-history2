/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qcomponentfactory.h>

#include <stdio.h>

int main( int argc, char **argv )
{
    QString file;
    int option = 0;

    for ( int i = 1; i < argc; i++ ) {
	QString a( argv[i] );
	if ( a == "-i" || a == "-install" ) {
	    option = 1;
	} else if ( a == "-u" || a == "-uninstall" ) {
	    option = 2;
	} else {
	    file = a;
	}
    }

    if ( !option || file.isEmpty() ) {
	fprintf( stderr, "Usage: regsvr -[i|u] <file>\n\n"
			 "\t-i: Install the component server\n"
			 "\t-u: Uninstall the component server\n" );
	return -1;
    }

    switch ( option == 1 ? QComponentFactory::registerServer( file ) : 
			   QComponentFactory::unregisterServer( file ) ) {
    case QS_FALSE:
	fprintf( stderr, option == 1 ? "Registering the server failed." : 
				       "Unregistering the server failed." );
	return 1;

    case QE_NOCOMPONENT:
	fprintf( stderr, "Server component not found." );
	return 2;

    case QE_NOINTERFACE:
	fprintf( stderr, "Server interface not supported by component." );
	return 2;

    default:
	break;
    }

    return 0;
}

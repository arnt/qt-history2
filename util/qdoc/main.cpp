/*
  main.cpp
*/

#include <qdir.h>

#include <stdlib.h>

#include "config.h"
#include "emitter.h"
#include "messages.h"




#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>

void myMessageOutput( QtMsgType type, const char *msg )
{
    switch ( type ) {
        case QtDebugMsg:
            fprintf( stderr, "Debug: %s\n", msg );
            break;
        case QtWarningMsg:
            fprintf( stderr, "Warning: %s\n", msg );
            abort();                        // dump core on purpose
            break;
        case QtFatalMsg:
            fprintf( stderr, "Fatal: %s\n", msg );
            abort();                        // dump core on purpose
    }
}


int main( int argc, char **argv )
{
    qInstallMsgHandler( myMessageOutput ); // ###

    config = new Config( argc, argv );

    QDir dir( config->outputDir() );
    if ( !dir.exists() ) {
	if ( !dir.mkdir(config->outputDir()) ) {
	    warning( 1, "Cannot create '%s'", config->outputDir().latin1() );
	    return EXIT_FAILURE;
	}
    }

    DocEmitter docEmitter;
    docEmitter.start();

    BookEmitter bookEmitter;
    bookEmitter.start( docEmitter.resolver() );

    delete config;
    config = 0;
    return EXIT_SUCCESS;
}

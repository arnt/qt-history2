/*
  main.cpp
*/

#include <qdir.h>

#include <stdlib.h>

#include "config.h"
#include "emitter.h"
#include "messages.h"

int main( int argc, char **argv )
{
    config = new Config( argc, argv );

    QDir dir( config->outputDir() );
    if ( !dir.exists() ) {
	if ( !dir.mkdir(config->outputDir()) ) {
	    message( 1, "Cannot create '%s'", config->outputDir().latin1() );
	    return EXIT_FAILURE;
	}
    }

    DocEmitter docEmitter;
    docEmitter.start();

    BookEmitter bookEmitter;
    bookEmitter.start( docEmitter.resolver() );

    warnAboutOmitted();

    delete config;
    config = 0;
    return EXIT_SUCCESS;
}

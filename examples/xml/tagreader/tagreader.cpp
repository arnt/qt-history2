/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "structureparser.h"
#include <qfile.h>
#include <qxml.h>
#include <qwindowdefs.h>

int main( int argc, char **argv )
{
    if ( argc < 2 ) {
	fprintf( stderr, "Usage: %s <xmlfile>\n", argv[0] );
	return 1;
    }
    for ( int i=1; i < argc; i++ ) {
        StructureParser handler;
        QFile xmlFile( argv[i] );
        QXmlInputSource source( &xmlFile );
        QXmlSimpleReader reader;
        reader.setContentHandler( &handler );
        reader.parse( source );
    }
    return 0;
}

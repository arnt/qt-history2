/*
$Id$
*/

#include "structureparser.h"
#include <qfile.h>
#include <qxml.h>
#include <qwindowdefs.h>

int main( int argc, char **argv )
{
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

/*
$Id$
*/ 

#include "structureparser.h"
#include <qfile.h>
#include <qxml.h>
 
int main( int argc, char **argv )
{
    for ( int i=1; i < argc; i++ ) {
        StructureParser handler;
        QFile xmlFile( argv[i] );
	if ( !xmlFile.open( IO_ReadOnly ) ) {
	    qWarning( "Can't open file %s", argv[i] );
	    continue;
	}
        QXmlInputSource source( &xmlFile );
	xmlFile.close();
        QXmlSimpleReader reader;
        reader.setContentHandler( &handler );
        reader.parse( source );
    }
    return 0;
} 

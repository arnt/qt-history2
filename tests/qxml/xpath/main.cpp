#include "qxpath.h"

int main( int argc, char** argv )
{
    char *paths[] = {
	"child::para",
	"child::*/child::para",
	"child::para[position()=3.1415*.5]",
	"para[attribute::type='warning']",
	0 };
    if ( argc == 1 ) {
	int i = 0;
	while ( paths[i] != 0 ) {
	    QXPath xpath( paths[i] );
	    xpath.evaluate( 0 );
	    i++;
	}
    } else {
	for ( int i=1; i<argc; i++ ) {
	    QXPath xpath( argv[i] );
	    xpath.evaluate( 0 );
	}
    }
    return 0;
}

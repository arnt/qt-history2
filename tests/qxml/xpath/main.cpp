#include "qxpath.h"

int main( int argc, char** argv )
{
    char *paths[] = {
	"child::para",
	"child::*/child::para",
	"child::para[position()=3*5]",
	"para[attribute::type='warning']",
	0 };
    if ( argc == 1 ) {
	int i = 0;
	while ( paths[i] != 0 ) {
	    QXPath xpath( paths[i] );
	    i++;
	}
    } else {
	for ( int i=1; i<argc; i++ ) {
	    QXPath xpath( argv[i] );
	}
    }
    return 0;
}

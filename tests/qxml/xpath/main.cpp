#include "qxpath.h"

int main( int argc, char** argv )
{
    char *paths[] = {
#if 0
	"child::para",
	"child::*/child::para",
	"child::para[position()=3.1415*.5]",
	"para[attribute::type='warning']",
#else
	"1 + 2*3",
	"1*2 +3",
	"1+ 2*3 - 4*5 +6",
	"1<2 and 2>=3",
#endif
	0 };
    if ( argc == 1 ) {
	int i = 0;
	while ( paths[i] != 0 ) {
	    QXPath xpath( paths[i] );
	    qDebug( "*** Evaluating: %s", paths[i] );
	    xpath.evaluate( 0 );
	    i++;
	}
    } else {
	for ( int i=1; i<argc; i++ ) {
	    QXPath xpath( argv[i] );
	    qDebug( "*** Evaluating: %s", argv[i] );
	    xpath.evaluate( 0 );
	}
    }
    return 0;
}

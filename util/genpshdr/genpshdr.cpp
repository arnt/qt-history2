#include <qstring.h>
#include <stdio.h>
#include <ctype.h>

#define BUFSIZE 8192


int main( int argc, char **argv )
{
    FILE *f;
    int len, i;
    static char buf[BUFSIZE];

    if ( argc != 2 ) {
	fprintf( stderr, "Usage:\n%s file\n", argv[0] );
	exit( 1 );
    }
    f = fopen( argv[1], "r" );
    if ( !f ) {
	fprintf( stderr, "%s: No such file: %s\n", argv[0], argv[1] );
	exit( 1 );
    }
    QString s;
    while ( !feof(f) ) {			// read to end of file
	char *p;
	if ( !fgets(buf, BUFSIZE, f) )		// read line
	    break;				// EOF reached
	s = buf;
	int index = s.find( '%' );
	if ( index >= 0 && s[index+1] != '%' )
	    s.truncate( index );
	s = s.simplifyWhiteSpace();
	if ( !s.isEmpty() )
	    printf( "    \"%s\\n\"\n", s.data() );
    }
    fclose( f );
}

#include <qstring.h>
#include <qfile.h>


char *outputstr( char *p, int i )
{
    while ( p[i] != ' ' )
	i--;
    p[i] = '\0';

    char *x = strchr(p,'%');
    if ( x ) {					// special treatment for %%
	char *start = p;
	p[i] = ' ';				// terminate elsewhere
	p = x + 2;
	while ( *p != '%' )			// find %% terminator, %
	    *p++;
	*p = '\0';
	if ( x > start ) {	
	    x[-1] = '\0';			// terminate before '%%'
	    printf( "\"%s\\n\"\n", start );
	    printf( "\"%s\\n\"\n", x );
	}
	return p+2;
    }
    else {
	printf( "\"%s\\n\"\n", p );
	return p+i+1;
    }
}


int main( int argc, char **argv )
{
    if ( argc != 2 ) {
	fprintf( stderr, "Usage:\n%s file\n", argv[0] );
	exit( 1 );
    }
    QFile f( argv[1] );
    if ( !f.open(IO_ReadOnly) ) {
	fprintf( stderr, "%s: No such file: %s\n", argv[0], argv[1] );
	exit( 1 );
    }
    QString buf( f.size()+1 );
    f.readBlock( buf.data(), f.size() );	// read the file into buf
    f.close();

    char *p = buf.data();

    while ( *p ) {				// strip comments
	if ( *p == '%' ) {
	    if ( *(p+1) == '%' && p > buf.data() && *(p-1) == '\n' ) {
		while ( *p && *p != '\n' )	// not a comment
		    p++;
		if ( *p )			// \n -> %
		    *p++ = '%';
	    }
	    else {
		while ( *p && *p != '\n' )	// replace with white space
		    *p++ = ' ';
	    }
	}
	else
	    p++;
    }

    buf = buf.simplifyWhiteSpace();
    p = buf.data();
    char *last = &p[buf.length()];

    while ( p+74<last )
	p = outputstr( p, 74 );
    outputstr( p, 0 );
}

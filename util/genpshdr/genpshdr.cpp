#include <qstring.h>
#include <stdio.h>
#include <ctype.h>

#define BUFSIZE 8192


void convert( char *buf )
{
    static char tmp[BUFSIZE];
    static int lineno = 0;
    int ch;
    int lastch;
    int inquotes;
    int tc  = 0;
    int len = 0;
    char *p = buf, *r = tmp;

    ++lineno;

  // First, convert any tabs to spaces

    while ( (ch = *p++) != 0 ) {
	len++;
	if ( ch == '\t' ) {
	    tc = 8 - (tc & 7);
	    while ( tc-- )
		*r++ = ' ';
	    tc = 0;
	}
	else {
	    *r++ = ch;
	    tc++;
	}
    }
    *r = '\0';

    if ( len > BUFSIZE - 10 ) {
	fprintf( stderr, "fixtabws: Line too long (%d)\n", lineno );
    }

  // Then, skip any white space at the end of line. Keep the '\n'.

    if ( r > tmp ) {
	int nl = *--r == '\n';
	while ( r >= tmp && isspace(*r) )
	    r--;
	r++;
	if ( nl )
	    *r++ = '\n';
	*r = '\0';
    }

  // Finally, convert all space chars back to tabs (except in "")

    p = tmp;
    r = buf;
    inquotes = 0;
    lastch = 0;
    tc = 0;

    while ( (ch = *p++) != 0 ) {
	if ( ch == '"' && lastch != '\\' ) {
	    inquotes = !inquotes;
	    *r++ = ch;
	}
	else if ( ch == ' ' && *p == ' ' && !inquotes ) {
	    int nspaces = 2;
	    int start = tc & 7;
	    p++;
	    tc++;
	    while ( *p == ' ' ) {
		p++;
		tc++;
		nspaces++;
	    }
	    while ( start + nspaces >= 8 ) {
		*r++ = '\t';
		nspaces -= 8 - start;
		start = 0;
	    }
	    while ( nspaces-- )
		*r++ = ' ';
	}
	else
	    *r++ = ch;
	lastch = ch;
	tc++;
    }
    *r = '\0';
}


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

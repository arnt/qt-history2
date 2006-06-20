/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <string.h>
#include <stdio.h>

#if defined(__WIN32__) || defined(__WIN64__)
#  include <fcntl.h>
#  include <io.h>
#  if defined(__BORLANDC__) || defined(__TURBOC__)
#    define _setmode(a,b) setmode(a,b)
#  endif
#endif

int main( int argc, char ** argv )
{
#if defined(__WIN32__) || defined(__WIN64__)
    _setmode( _fileno(stdin), _O_BINARY );
    _setmode( _fileno(stdout), _O_BINARY );
    _setmode( _fileno(stderr), _O_BINARY );
#endif

    int useStdout = 1;
    int useStderr = 0;
    for ( int i=1; i<argc; i++ ) {
	if ( strcmp( argv[i], "-stdout_and_stderr" ) == 0 ) {
	    useStdout = 1;
	    useStderr = 1;
	} else if ( strcmp( argv[i], "-stderr" ) == 0 ) {
	    useStdout = 0;
	    useStderr = 1;
	}
    }

    int ch;
    while( (ch = fgetc(stdin)) != -1 ) {
	if ( useStdout ) {
	    printf("%c", (char) ch);
	    fflush(stdout);
	    // if both, useStdout and useStderr are used, we want to write
	    // different characters
	    ch++;
	}
	if ( useStderr ) {
	    fprintf(stderr, "%c", (char) ch);
            fflush(stderr);
	}
    }
    return ch;
}

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qregexp.cpp#4 $
**
** Implementation of QRegExp class
**
** Author  : Haavard Nord
** Created : 950126
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qregexp.h"
#include <ctype.h>
#if defined(_OS_MAC_) && defined(VXWORKS)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qregexp.cpp#4 $";
#endif


//
// The regexp pattern is internally represented as an array of ushorts,
// each element containing an 8-bit character or a 16-bit code (listed below).
// Character classes are encoded as 256 bits, i.e. 16*16 bits.
//

const ushort BOL	= 0x8001;		// beginning of line 	^
const ushort EOL	= 0x8002;		// end of line		$
const ushort BOW	= 0x8003;		// beginning of word	\<
const ushort EOW	= 0x8004;		// end of word		\>
const ushort ANY	= 0x8005;		// any character	.
const ushort CCL	= 0x8006;		// character class	[]
const ushort CLO	= 0x8007;		// Kleene closure	*
const ushort END	= 0x0000;

// ---------------------------------------------------------------------------
// QRegExp member functions
//

QRegExp::QRegExp()
{
    rxdata = 0;
    valid = FALSE;
}

QRegExp::QRegExp( const char *pattern, bool wildcard )
{
    rxstring = pattern;
    rxdata = 0;
    if ( wildcard )
	wc2rx();
    compile();
}

QRegExp::~QRegExp()
{
    delete rxdata;
}

QRegExp &QRegExp::operator=( const QRegExp &r )
{
    rxstring = (const char *)r.rxstring;
    compile();
    return *this;
}

QRegExp &QRegExp::operator=( const char *pattern )
{
    rxstring = pattern;
    compile();
    return *this;
}


//
// Find the first instance of the regexp pattern in a string.
//

int QRegExp::match( const char *str, int index, int *len ) const
{
    if ( !valid || !rxdata || !*rxdata )	// nothing to match
	return -1;
    register char *p = (char *)str + index;
    ushort *d  = rxdata;
    char   *ep = 0;

    if ( *d == BOL )				// match from beginning of line
	ep = matchstr( d, p, p );
    else {
	if ( (*d & 0x8000) == 0 ) {
	    while ( *p && *p != (char)*d )	// find first char occurrence
		p++;
	}
	while ( *p ) {				// regular match
	    if ( (ep=matchstr(d,p,(char*)str+index)) )
		break;
	    p++;
	}
    }
    if ( ep ) {					// match
	if ( len )
	    *len = ep - p;
	return (int)(p - str);
    }
    else {					// no match
	if ( len )
	    *len = 0;
	return -1;
    }
}


inline bool iswordchar( int x )
{
    return isalnum(x) || x == '_';
}


char *QRegExp::matchstr( ushort *rxd, char *str, char *bol ) const
{
    register char *p = str;
    ushort *d = rxd;

    while ( *d ) {
	if ( (*d & 0x8000) == 0 ) {		// match char
	    if ( *p++ != (char)*d )
		return 0;
	    d++;
	}
	else switch ( *d++ ) {
	    case ANY:				// match anything
		if ( !*p++ )
		    return 0;
		break;
	    case CCL:				// match char class
		if ( (d[*p >> 4] & (1 << (*p & 0xf))) == 0 )
		    return 0;
		p++;
		d += 16;
		break;
	    case BOL:				// match beginning of line
		if ( p != bol )
		    return 0;
		break;
	    case EOL:				// match end of line
		if ( *p )
		    return 0;
		break;
	    case BOW:				// match beginning of word
		if ( !iswordchar(*p) || (p > bol && iswordchar(*(p-1)) ) )
		    return 0;
		break;
	    case EOW:				// match end of word
		if ( iswordchar(*p) || p == bol || !iswordchar(*(p-1)) )
		    return 0;
		break;
	    case CLO:				// closure
		{
		char *first_p = p;
		if ( (*d & 0x8000) == 0 ) {	// match char
		    while ( *p && *p == (char)*d )
			p++;
		}
		else switch ( *d ) {
		    case ANY:
			while ( *p )
			    p++;
			break;
		    case CCL:
			d++;
			while ( *p && d[*p >> 4] & (1 << (*p & 0xf)) )
			    p++;
			d += 15;
			break;
		    default:			// error
			return 0;
		}
		d++;
		d++;
		char *end;
		while ( p >= first_p ) {	// go backwards
		    if ( (end = matchstr(d,p,bol)) )
			return end;
		    --p;
		}
		}				// fall through!
	    default:				// error
		return 0;
	}
    }
    return p;
}


//
// Translate wildcard pattern to standard regexp pattern.
// Ex:   *.cpp	==> ^.*\.cpp$
//

void QRegExp::wc2rx()
{
    register char *p = (char *)rxstring;
    if ( !p )					// no regexp pattern
	return;
    QString wcpattern = "^";
    char c;
    while ( (c=*p++) ) {
	switch ( c ) {
	    case '*':				// '*' ==> '.*'
		wcpattern += '.';
		break;
	    case '?':				// '?' ==> '.'
		c = '.';
		break;
	    case '.':				// quote special regexp chars
	    case '+':
	    case '\\':
	    case '^':
	    case '$':
		wcpattern += '\\';
		break;
	}
	wcpattern += c;
    }
    wcpattern += '$';
    rxstring = wcpattern;			// set new regexp pattern
}


//
// Internal: Get char value and increment pointer.
//

static int char_val( char **str )		// get char value
{
    register char *p = *str;
    int len = 1;
    int v = 0;
    if ( *p == '\\' ) {				// escaped code
	p++;
	if ( *p == 0 ) {			// it is just a '\'
	    (*str)++;
	    return '\\';
	}
	len++;					// length at least 2
	switch ( tolower(*p) ) {
	    case 'b':  v = '\b';  break;	// bell
	    case 'f':  v = '\f';  break;	// form feed
	    case 'n':  v = '\n';  break;	// newline
	    case 'r':  v = '\r';  break;	// return
	    case 't':  v = '\t';  break;	// tab

	    case 'x': {				// hex code
		p++;
		int  c = tolower(*p);
		bool a = c >= 'a' && c <= 'f';
		if ( isdigit(c) || a ) {	// hex digit?
		    v = a ? 10 + c - 'a' : c - '0';
		    len++;
		}
		p++;
		c = tolower(*p);
		a = c >= 'a' && c <= 'f';
		if ( isdigit(c) || a ) {	// another hex digit?
		    v *= 16;
		    v += a ? 10 + c - 'a' : c - '0';
		    len++;
		}
	        }
		break;

	    default: {
		int i;
		--len;				// first check if octal
		for ( i=0; i<3 && *p >= '0' && *p <= '7'; i++ ) {
		    v *= 8;
		    v += *p++ - '0';
		    len++;
		}
		if ( i == 0 ) {			// not an octal number
		    v = *p;
		    len++;
		}
	    }
	}
    }
    else
	v = *p;
    *str += len;
    return v;
}


#if defined(DEBUG)
ushort *dump( ushort *p )			// DEBUG !!!
{
    while ( *p != END ) {
	switch ( *p++ ) {
	    case BOL:
	        debug( "\tBOL" );
		break;
	    case EOL:
	        debug( "\tEOL" );
		break;
	    case BOW:
	        debug( "\tBOW" );
		break;
	    case EOW:
	        debug( "\tEOW" );
		break;
	    case ANY:
	        debug( "\tANY" );
		break;
	    case CCL: {
		QString s = "";
		QString buf;
		for ( int n=0; n<256; n++ ) {
		    if ( p[n >> 4] & (1 << (n & 0xf)) ) {
			if ( isprint(n) )
			    s += (char)n;
			else {
			    buf.sprintf( "\\X%.2X", n );
			    s += buf;
			}
		    }
		}
	        debug( "\tCCL\t%s", (char *)s );
		p += 16;
	        }
		break;
	    case CLO:
		debug( "\tCLO" );
		p = dump( p );
		break;
	    default:
		ASSERT( (*(p-1) & 0x8000) == 0 );
		debug( "\tCHR\t%c (%d)", *(p-1), *(p-1) );
		break;
	}
    }
    debug( "\tEND" );
    return p+1;
}
#endif // DEBUG


//
// Compile the regexp pattern and store the result in rxdata.
// The 'valid' flag is set to FALSE if an error is detected.
//

void QRegExp::compile()
{
    if ( rxstring.isEmpty() ) {			// no regexp pattern set
	valid = FALSE;
	return;
    }

    const maxlen = 1024;			// huge array
    if ( !rxdata ) {
	rxdata = new ushort[ maxlen ];
	CHECK_PTR( rxdata );
    }
    valid  = TRUE;				// assume valid pattern

    char   *p = rxstring;			// pattern pointer
    ushort *d = rxdata;				// data pointer
    ushort *prev_d = 0;

#define GEN(x)	*d++ = (x)

    while ( *p ) {
	switch ( *p ) {

	    case '^':				// beginning of line
		prev_d = d;
		GEN( p == rxstring.data() ? BOL : *p );
		p++;
		break;

	    case '$':				// end of line
		prev_d = d;
		GEN( *(p+1) == 0 ? EOL : *p );
		p++;
		break;

	    case '.':				// any char
		prev_d = d;
		GEN( ANY );
		p++;
		break;

	    case '[':				// character class
		{
		char cc[256];
		char neg;			// mask for CCL
		prev_d = d;
		GEN( CCL );
		p++;
		memset( cc, 0, 256 );		// reset char class array
		if ( *p == '^' ) {		// negate!
		    neg = 1;
		    p++;
		}
		else
		    neg = 0;
		if ( *p == ']' )		// bracket, not end
		    cc[*p++] = 1;
		int prev_c = -1;
		while ( *p && *p != ']' ) {	// scan the char set
		    if ( *p == '-' && *(p+1) && *(p+1) != ']' ) {
			p++;			// range!
			if ( prev_c == -1 )	// no previous char
			    cc['-'] = 1;
			else {
			    int start = prev_c;
			    int stop = char_val( &p );
			    if ( start > stop ) { // swap start and stop
				int tmp = start;
				start = stop;
				stop = tmp;
			    }
			    while ( start++ < stop )
				cc[start] = 1;
			}
		    }
		    else			// normal char
			cc[(prev_c=char_val(&p))] = 1;
		}
		if ( *p != ']' ) {		// missing close bracket
		    valid = FALSE;
		    return;
		}
		if ( d + 16 >= rxdata + maxlen ) {
		    valid = FALSE;		// buffer overflow
		    return;
		}
		memset( d, 0, 16*sizeof(ushort) );
		for ( int i=0; i<256; i++ ) {	// set bits
		    if ( cc[i] ^ neg )
			d[i >> 4] |= (1 << (i & 0xf));
		}
		d += 16;
		p++;
		}
		break;

	    case '*':				// Kleene closure
	    case '+':				// positive closure
		{
		if ( prev_d == 0 ) {		// no previous expression
		    valid = FALSE;		// empty closure
		    return;
		}
		switch ( *prev_d ) {
		    case BOL:
		    case BOW:
		    case EOW:
		    case CLO:
			valid = FALSE;
			return;
		}
		int ddiff = d - prev_d;
		if ( *p == '+' ) {		// duplicate expression
		    if ( d + ddiff >= rxdata + maxlen ) {
			valid = FALSE;		// overflow
			return;
		    }
		    memcpy( d, prev_d, ddiff*sizeof(ushort) );
		    d += ddiff;
		    prev_d += ddiff;
		}
		memmove( prev_d+1, prev_d, ddiff*sizeof(ushort) );
		*prev_d = CLO;
		d++;
		GEN( END );
		p++;
		}
		break;

	    default:
		prev_d = d;
		if ( *p == '\\' && (*(p+1) == '<' || *(p+1) == '>') ) {
		    GEN( *++p == '<' ? BOW : EOW );
		    p++;
		}
		else
		    GEN( char_val(&p) );
	}
	if ( d >= rxdata + maxlen ) {		// oops!
	    valid = FALSE;			// buffer overflow
	    return;
	}
    }
    GEN( END );
//  dump( rxdata );	// comment this out for debugging!!!
}


// ---------------------------------------------------------------------------
// QString member functions that use QRegExp
//

int QString::find( const QRegExp &r, int index ) const
{						// find substring
    return (uint)index >= size() ? -1 : r.match( data(), index );
}

int QString::findRev( const QRegExp &r, int index ) const
{						// reverse find substring
    if ( index < 0 ) {				// neg index ==> start from end
	if ( size() )
	    index = size() - 1;
	else					// empty string
	    return -1;
    }
    else if ( (uint)index >= size() )		// bad index
	return -1;
    while( index >= 0 ) {
	if ( r.match(data(),index) == index )
	    return index;
	index--;
    }
    return -1;
}

int QString::contains( const QRegExp &r ) const
{						// get # substrings
    int count = 0;
    register char *d = data();
    if ( !d )					// null string
	return 0;
    int index = -1;
    while ( TRUE ) {				// count overlapping matches
	index = r.match( d, index+1 );
	if ( index >= 0 )
	    count++;
	else
	    break;
    }
    return count;
}

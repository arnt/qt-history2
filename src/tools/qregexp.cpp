/****************************************************************************
** $Id: //depot/qt/main/src/tools/qregexp.cpp#1 $
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


//
// The regexp pattern is internally represented as an array of ushorts,
// each element containing an 8-bit character or a 16-bit code (listed below).
// Character classes are encoded as 256 bits, i.e. 16*16 bits.
//

const ushort BOL	= 0x8001;		// beginning of line 	^
const ushort EOL	= 0x8002;		// end of line		$
const ushort BOW	= 0x8003;		// beginning of word	\<
const ushort EOW	= 0x8004;		// end of word		\>
const ushort BOT	= 0x8005;		// beginning of tag	\(
const ushort EOT	= 0x8006;		// end of tag		\)
const ushort ANY	= 0x8007;		// any character	.
const ushort CCL	= 0x8008;		// character class	[]
const ushort CLO	= 0x8009;		// Kleene closure	*
const ushort OPT	= 0x800a;		// optional closure	?
const ushort REF	= 0x800b;		// backref		\n
const ushort END	= 0x0000;

// ---------------------------------------------------------------------------
// QRegExp class member functions
//

QRegExp::QRegExp()
{
    rxdata = 0;
    valid = wcmode = FALSE;
}

QRegExp::QRegExp( const char *pattern, bool wildcardMode )
{
    rxstring = pattern;
    rxdata = 0;
    wcmode = wildcardMode;
    compile();					// compile the pattern
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
    register ushort *d = rxdata;
    register char   *p = (char *)str + index;
    char	    *ep;
    if ( *d == BOL )
	ep = matchsub( d, p, p );
    else {
	if ( (*d & 0x8000) == 0 ) {
	    while ( *p && *p != (char)*d )	// find first char occurrence
		p++;
	    if ( !*p )
		return -1;
	}
	while ( *p ) {
	    if ( (ep=matchsub(d,p,(char*)str+index)) )
		break;
	    p++;
	}
    }
    if ( ep ) {
	if ( len )
	    *len = ep - p;
	return (int)(p - str);
    }
    else
	return -1;
}


char *QRegExp::matchsub( ushort *rxd, char *str, char *bol ) const
{
    register ushort *d = rxd;
    register char   *p = str;
    ushort r;
    char   c;

    while ( (r=*d++) != END ) {
	if ( (r & 0x8000) == 0 ) {
	    if ( *p++ != (char)r )
		return 0;
	}
	else switch ( r ) {
	    case ANY:
		if ( !*p++ )
		    return 0;
		break;
	    case CCL:
		if ( (d[*p >> 4] & (1 << (*p & 0xf))) == 0 )
		    return 0;
		p++;
		d += 16;
		break;
	    case BOL:
		if ( p != bol )
		    return 0;
		break;
	    case EOL:
		if ( *p )
		    return 0;
		break;
	    case CLO:
		{
		char *save_p = p;
		if ( (*d & 0x8000) == 0 ) {
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
			d += 16;
			break;
		    default:
			debug( "QRegExp: INTERNAL ERROR #1" );
		}
		d++;
		char *ep;
		while ( p > save_p ) {
		    if ( (ep = matchsub(d,p,bol)) )
			return ep;
		    --p;
		}
		return 0;
		}
	    default:
		debug( "QRegExp: INTERNAL ERROR #2" );
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
    ASSERT( wcmode );
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
	if ( *p == 0 || *p == ']' ) {		// it is just a '/'
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


ushort *dump( ushort *p )				// DEBUG !!!
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


//
// Compile the regexp pattern and store the result in rxdata.
// The 'valid' flag is set to FALSE if an error is detected.
//

void QRegExp::compile()
{
    if ( wcmode )				// convert wildcard
	wc2rx();
    if ( rxdata ) {				// delete the old data
	delete rxdata;
	rxdata = 0;
    }
    if ( rxstring.isEmpty() ) {			// no regexp pattern set
	valid = FALSE;
	return;
    }

    int maxlen = 2048;				// reserve huge array
    rxdata = (ushort *)malloc( maxlen*sizeof(ushort) );
    CHECK_PTR( rxdata );
    valid  = TRUE;				// valid pattern

    char   *p = rxstring;			// pattern pointer
    ushort *d = rxdata;				// data pointer
    ushort *prev_d = 0;

#define GEN(x)	*d++ = (x)

    while ( *p ) {
	switch ( *p ) {

	    case '^':				// beginning of line
		prev_d = d;
		GEN( p == rxstring.data() ? BOL : *p );
		break;

	    case '$':				// end of line
		prev_d = d;
		GEN( *(p+1) == 0 ? EOL : *p );
		break;

	    case '.':				// any char
		prev_d = d;
		GEN( ANY );
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
		    debug( "QRegExp: Missing ]" );
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
		}
		break;

	    case '*':				// Kleene closure
	    case '+':				// positive closure
	    case '?':				// optional closure
		{
		if ( prev_d == 0 ) {		// no previous expression
		    valid = FALSE;		// empty closure
		    debug( "QRegExp: Empty closure" );
		    return;
		}
		switch ( *prev_d ) {
		    case BOL:
		    case BOW:
		    case EOW:
		    case BOT:
		    case EOT:
		    case CLO:
		    case OPT:
		    case REF:
			debug( "QRegExp: Illegal closure" );
			valid = FALSE;
			return;
		}
		int ddiff = d - prev_d;
		if ( *p == '+' ) {		// duplicate expression
		    memcpy( d, prev_d, ddiff*sizeof(ushort) );
		    d += ddiff;
		    prev_d += ddiff;
		}
		memmove( prev_d+1, prev_d, ddiff*sizeof(ushort) );
		*prev_d = *p == '?' ? OPT : CLO;
		d++;
		GEN( END );
		}
		break;

	    default:
		prev_d = d;
		GEN( *p );
	}
        p++;
	if ( d >= rxdata + maxlen ) {		// oops!
	    valid = FALSE;			// buffer overflow
	    return;
	}
    }
    GEN( END );
    dump( rxdata );
}

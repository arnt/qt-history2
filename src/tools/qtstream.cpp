/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtstream.cpp#1 $
**
** Implementation of QTextStream class
**
** Author  : Haavard Nord
** Created : 990922
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qtstream.h"
#include "qstring.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qtstream.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// QTextStream member functions
//

#if defined(CHECK_STATE)
#define CHECK_STREAM_PRECOND  if ( !dev ) {				\
				warning( "QTextStream: No device" );	\
				return *this; }
#else
#define CHECK_STREAM_PRECOND
#endif


#define I_BASE_2	0x0001
#define I_BASE_8	0x0002
#define I_BASE_10	0x0003
#define I_BASE_16	0x0004
#define I_BASE_MASK	0x000f

#define I_SHORT		0x0010
#define I_INT		0x0020
#define I_LONG		0x0030
#define I_TYPE_MASK	0x00f0

#define I_SIGNED	0x0100
#define I_UNSIGNED	0x0200
#define I_SIGN_MASK	0x0f00


QTextStream::QTextStream()
{
    dev = 0;					// no device set
    base = I_BASE_10;
}

QTextStream::QTextStream( QIODevice *d )
{
    dev = d;					// set device
    base = I_BASE_10;
}

QTextStream::~QTextStream()
{
}


// --------------------------------------------------------------------------
// QTextStream read functions
//


// !!! NOT IMPLEMENTED !!!


// --------------------------------------------------------------------------
// QTextStream write functions
//

QTextStream &QTextStream::operator<<( char c )	// write char
{
    CHECK_STREAM_PRECOND
    dev->putch( c );
    return *this;
}


QTextStream &QTextStream::output_int( int format, ulong n, bool neg )
{
    static char hexdigits_lower[] = "0123456789abcdef";
    static char hexdigits_upper[] = "0123456789ABCDEF";
    CHECK_STREAM_PRECOND
    char buf[72];
    register int len;
    register char *p;
    char *hexdigits;
    switch ( base & I_BASE_MASK ) {
	case I_BASE_2:				// output bits
	    switch ( format & I_TYPE_MASK ) {
		case I_SHORT: len=16; break;
		case I_INT:   len=sizeof(int)*8; break;
		case I_LONG:  len=32; break;
		default:      len = 0;
	    }
	    p = &buf[len];
	    *p-- = '\0';
	    while ( len-- ) {
		*p-- = ((n & 1) + '0');
		n >>= 1;
	    }
	    p = buf;
	    break;
	case I_BASE_8:
	    p = &buf[64];			// go in reverse order
	    *p = '\0';
	    do {
		*--p = (n&7) + '0';
		n /= 8;
	    } while ( n );
	    break;
	case I_BASE_10:
	    sprintf( buf, "%d", n );
	    p = buf;
	    break;
	    p = &buf[64];			// go in reverse order
	    *p = '\0';
	    if ( neg )
		n = (ulong)(-(long)n);
	    do {
		*--p = ((int)(n%10)) + '0';
		n /= 10;
	    } while ( n );
	    if ( neg )
		*--p = '-';
	    break;
	case I_BASE_16:
	    p = &buf[64];			// go in reverse order
	    *p = '\0';
	    hexdigits = hexdigits_lower;
	    do {
		*--p = hexdigits[(int)n&0xf];
		n /= 16;
	    } while ( n );
	    break;
	default:				// unsupported base -> ignore
	    return *this;
    }
    dev->writeBlock( p, strlen(p) );
    return *this;
}


QTextStream &QTextStream::operator<<( signed short i )
{
    return output_int( I_SHORT | I_SIGNED, i, i < 0 );
}


QTextStream &QTextStream::operator<<( unsigned short i )
{
    return output_int( I_SHORT | I_UNSIGNED, i, FALSE );
}


QTextStream &QTextStream::operator<<( signed int i )
{
    return output_int( I_INT | I_SIGNED, i, i < 0 );
}


QTextStream &QTextStream::operator<<( unsigned int i )
{
    return output_int( I_INT | I_UNSIGNED, i, FALSE );
}


QTextStream &QTextStream::operator<<( signed long i )
{
    return output_int( I_LONG | I_SIGNED, i, i < 0 );
}


QTextStream &QTextStream::operator<<( unsigned long i )
{
    return output_int( I_LONG | I_UNSIGNED, i, FALSE );
}


QTextStream &QTextStream::operator<<( float f )	// write float value
{
    CHECK_STREAM_PRECOND
    char buf[32];
    sprintf( buf, "%g", f );
    dev->writeBlock( buf, strlen(buf) );
    return *this;
}


QTextStream &QTextStream::operator<<( double f )// write double value
{
    CHECK_STREAM_PRECOND
    char buf[32];
    sprintf( buf, "%g", f );
    dev->writeBlock( buf, strlen(buf) );
    return *this;
}


QTextStream &QTextStream::operator<<( const char *s )
{						// write 0-term char array
    CHECK_STREAM_PRECOND
    uint len = strlen( s );			// don't write null terminator
    dev->writeBlock( s, len );
    return *this;
}


QTextStream &QTextStream::operator<<( QTSMF mf )
{
    (this->*mf)();
    return *this;
}


QTextStream &QTextStream::operator<<( QTSFUNC f )
{
    return (*f)( *this );
}


void QTextStream::bin()
{
    base = I_BASE_2;
}

void QTextStream::oct()
{
    base = I_BASE_8;
}

void QTextStream::dec()
{
    base = I_BASE_10;
}

void QTextStream::hex()
{
    base = I_BASE_16;
}

void QTextStream::endl()
{
    if ( dev )
	dev->putch( '\n' );
}

void QTextStream::flush()
{
    if ( dev )
	dev->flush();
}


QTextStream &hex(QTextStream &s) { s.hex(); return s; }

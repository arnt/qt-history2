/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdstream.cpp#4 $
**
** Implementation of QDataStream class
**
** Author  : Haavard Nord
** Created : 930831
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdstream.h"
#include "qstring.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(UNIX)
#include <sys/types.h>				// htonl etc.
#include <netinet/in.h>
#endif

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qdstream.cpp#4 $";
#endif


// --------------------------------------------------------------------------
// QDataStream member functions
//

#if defined(CHECK_STATE)
#define CHECK_STREAM_PRECOND  if ( !dev ) {				\
				warning( "QDataStream: No device" );	\
				return *this; }
#else
#define CHECK_STREAM_PRECOND
#endif

static int wordSize = 0;
static int bigEndian;


QDataStream::QDataStream()
{
    if ( wordSize == 0 )			// get system features
	qSysInfo( &wordSize, &bigEndian );
    dev = 0;					// no device set
    printable = FALSE;
}

QDataStream::QDataStream( QIODevice *d )
{
    if ( wordSize == 0 )			// get system features
	qSysInfo( &wordSize, &bigEndian );
    dev = d;					// set device
    printable = FALSE;
}

QDataStream::~QDataStream()
{
}


// --------------------------------------------------------------------------
// QDataStream read functions
//

static INT32 read_int_d7( QDataStream *s )	// read data7 int constant
{
    register int n = 0;
    char buf[40];
    while ( TRUE ) {
	buf[n] = s->device()->getch();
	if ( buf[n] == '$' || n > 38 )		// $-terminator
	    break;
	n++;
    }
    buf[n] = '\0';
    return atol( buf );
}


QDataStream &QDataStream::operator>>( INT8 &i )	// read 8-bit signed int (char)
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	i = (INT8)dev->getch();
	if ( i == '\\' ) {			// read octal code
	    char buf[4];
	    dev->readBlock( buf, 3 );
	    i = (buf[2] & 0x07)+((buf[1] & 0x07) << 3)+((buf[0] & 0x07) << 6);
	}
    }
    else					// data or text
	i = (INT8)dev->getch();
    return *this;
}


QDataStream &QDataStream::operator>>( INT16 &i )// read 16-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	i = (INT16)read_int_d7( this );
    else
    if ( bigEndian )				// no conversion needed
	dev->readBlock( (char *)&i, sizeof(INT16) );
    else {					// convert to little endian
#if defined(UNIX)
	dev->readBlock( (char *)&i, sizeof(INT16) );
	i = (INT16)ntohs( i );
#else
	register unsigned char *p = (unsigned char *)(&i);
	char x[2];
	dev->readBlock( x, 2 );
	*p++ = x[1];
	*p = x[0];
#endif
    }
    return *this;
}


QDataStream &QDataStream::operator>>( INT32 &i )// read 32-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	i = read_int_d7( this );
    else
    if ( bigEndian )				// no conversion needed
	dev->readBlock( (char *)&i, sizeof(INT32) );
    else {					// convert to little endian
#if defined(UNIX)
	dev->readBlock( (char *)&i, sizeof(INT32) );
	i = (INT32)ntohl( i );
#else
	register unsigned char *p = (unsigned char*)(&i);
	char x[4];
	dev->readBlock( x, 4 );
	*p++ = x[3];
	*p++ = x[2];
	*p++ = x[1];
	*p = x[0];
#endif
    }
    return *this;
}


QDataStream &QDataStream::operator>>( int &i )	// read integer as INT32
{
    INT32 n;
    *this >> n;
    i = (int)n;
    return *this;
}


QDataStream &QDataStream::operator>>( uint &i )	// read uinteger as UINT32
{
    UINT32 n;
    *this >> n;
    i = (uint)n;
    return *this;
}


static double read_double_d7( QDataStream *s )	// read data7 double constant
{
    register int n = 0;
    char buf[80];
    while ( TRUE ) {
	buf[n] = s->device()->getch();
	if ( buf[n] == '$' || n > 78 )		// $-terminator
	    break;
	n++;
    }
    buf[n] = '\0';
    return atof( buf );
}


QDataStream &QDataStream::operator>>( float &f )// read 32-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	f = (float)read_double_d7( this );
    else
    if ( bigEndian )				// no conversion needed
	dev->readBlock( (char *)&f, sizeof(float) );
    else {					// convert to little endian
	register unsigned char *p = (unsigned char *)(&f);
	char x[4];
	dev->readBlock( x, 4 );
	*p++ = x[3];
	*p++ = x[2];
	*p++ = x[1];
	*p = x[0];
    }
    return *this;
}


QDataStream &QDataStream::operator>>( double &f)// read 64-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	f = read_double_d7( this );
    else
    if ( bigEndian )				// no conversion needed
	dev->readBlock( (char *)&f, sizeof(double) );
    else {					// convert to little endian
	register unsigned char *p = (unsigned char *)(&f);
	char x[8];
	dev->readBlock( x, 8 );
	*p++ = x[7];
	*p++ = x[6];
	*p++ = x[5];
	*p++ = x[4];
	*p++ = x[3];
	*p++ = x[2];
	*p++ = x[1];
	*p = x[0];
    }
    return *this;
}


QDataStream &QDataStream::operator>>( char *&s )// read char array
{
    uint len = 0;
    return readBytes( s, len );
}


QDataStream &QDataStream::readBytes( char *&s, uint &l )
{						// read length-encoded bytes
    CHECK_STREAM_PRECOND
    UINT32 len;
    *this >> len;				// first read length spec
    l = (uint)len;
    s = new char[len];				// create char array
    CHECK_PTR( s );
    if ( !s )					// no memory
	return *this;
    return readRawBytes( s, (uint)len );
}


QDataStream &QDataStream::readRawBytes( char *s, uint len )
{						// read len bytes
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	register char *p = s;
	while ( len-- )
	    *this >> *p++;
    }
    else					// read data char array
	dev->readBlock( s, len );
    return *this;
}


// --------------------------------------------------------------------------
// QDataStream write functions
//

QDataStream &QDataStream::operator<<( INT8 i )	// write 8-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable && !isprint(i) ) {		// printable data
	char buf[8];				// write octal code
	buf[0] = '\\';
	buf[1] = '0' + ((i >> 6) & 0x07);
	buf[2] = '0' + ((i >> 3) & 0x07);
	buf[3] = '0' + (i & 0x07);
	buf[4] = '\0';
	dev->writeBlock( buf, 4 );
    }
    else
	dev->putch( i );
    return *this;
}


QDataStream &QDataStream::operator<<( INT16 i )	// write 16-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[16];
	sprintf( buf, "%d$", i );
	dev->writeBlock( buf, strlen(buf) );
    }
    else
    if ( bigEndian )				// no conversion needed
	dev->writeBlock( (char *)&i, sizeof(INT16) );
    else {					// convert to big endian
#if defined(UNIX)
	i = (INT16)htons( i );
	dev->writeBlock( (char *)&i, sizeof(INT16) );
#else
	register puchar p = (puchar)(&i);
	char x[2];
	x[1] = *p++;
	x[0] = *p;
	dev->writeBlock( x, 2 );
#endif
    }
    return *this;
}


QDataStream &QDataStream::operator<<( INT32 i )	// write 32-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[16];
	sprintf( buf, "%ld$", i );
	dev->writeBlock( buf, strlen(buf) );
    }
    else
    if ( bigEndian )				// no conversion needed
	dev->writeBlock( (char *)&i, sizeof(INT32) );
    else {					// convert to big endian
#if defined(UNIX)
	i = (INT32)htonl( i );
	dev->writeBlock( (char *)&i, sizeof(INT32) );
#else
	register unsigned char *p = (unsigned char *)(&i);
	char x[4];
	x[3] = *p++;
	x[2] = *p++;
	x[1] = *p++;
	x[0] = *p;
	dev->writeBlock( x, 4 );
#endif
    }
    return *this;
}


QDataStream &QDataStream::operator<<( float f )	// write 32-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[32];
	sprintf( buf, "%g$", f );
	dev->writeBlock( buf, strlen(buf) );
    }
    else {
	float g = f;				// fixes float-on-stack problem
	if ( bigEndian )			// no conversion needed
	    dev->writeBlock( (char *)&g, sizeof(float) );
	else {					// convert to big endian
	    register unsigned char *p = (unsigned char *)(&g);
	    char x[4];
	    x[3] = *p++;
	    x[2] = *p++;
	    x[1] = *p++;
	    x[0] = *p;
	    dev->writeBlock( x, 4 );
	}
    }
    return *this;
}


QDataStream &QDataStream::operator<<( double f )// write 64-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[32];
	sprintf( buf, "%g$", f );
	dev->writeBlock( buf, strlen(buf) );
    }
    else
    if ( bigEndian )				// no conversion needed
	dev->writeBlock( (char *)&f, sizeof(double) );
    else {					// convert to big endian
	register puchar p = (puchar)(&f);
	char x[8];
	x[7] = *p++;
	x[6] = *p++;
	x[5] = *p++;
	x[4] = *p++;
	x[3] = *p++;
	x[2] = *p++;
	x[1] = *p++;
	x[0] = *p;
	dev->writeBlock( x, 8 );
    }
    return *this;
}


QDataStream &QDataStream::operator<<( const char *s )
{						// write 0-term char array
    uint len = strlen( s ) + 1;			// also write null terminator
    *this << (INT32)len;			// write length specifier
    return writeRawBytes( s, len );
}


QDataStream &QDataStream::writeBytes(const char *s, uint len)
{						// write char array with length
    CHECK_STREAM_PRECOND
    *this << (INT32)len;			// write length specifier
    if ( len )
	writeRawBytes( s, len );
    return *this;
}


QDataStream &QDataStream::writeRawBytes( const char *s, uint len )
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// write printable
	register char *p = (char *)s;
	while ( len-- )
	    *this << *p++;
    }
    else					// write data char array
	dev->writeBlock( s, len );
    return *this;
}

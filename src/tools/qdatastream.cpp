/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatastream.cpp#10 $
**
** Implementation of QDataStream class
**
** Author  : Haavard Nord
** Created : 930831
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
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
static char ident[] = "$Id: //depot/qt/main/src/tools/qdatastream.cpp#10 $";
#endif


/*!
\class QDataStream qdstream.h
\brief The QDataStream class provides basic functions for serialization of
binary data to a QIODevice.

A data stream is a binary stream of encoded information which is 100%
independent of the host computer operation system, CPU or byte order.
A stream that is written by a PC under DOS/Windows can easily be read
by a Sun SPARC running Solaris.

The QDataStream class implements serialization of primitive types,
like \c char, \c short, \c int, \c char* etc.  Serialization of more
complex data is accomplished by breaking up the data into primitive units.

The programmer can select which byte order to use when serializing data.
The default setting is big endian (MSB first). Changing it to little
endian (LSB first) breaks the portability.
We therefore recommend keeping this setting unless you have special needs or
requirements.

A data stream cooperates closely with a QIODevice. A QIODevice represents
an input/output medium one can read data from and write data to.
The QFile class is an example of an IO device.

Example of how to serialize to a stream:
\code
  QFile	f( "file.dta" );
  f.open( IO_WriteOnly );		\/ open file for writing
  QDataStream s( &f );			\/ serialize using f
  s << "the answer is";			\/ serialize string
  s << 42;				\/ serialize integer
  f.close();				\/ done
\endcode

Example of how to serialize from a stream:
\code
  QFile	f( "file.dta" );
  f.open( IO_ReadOnly );		\/ open file for reading
  QDataStream s( &f );			\/ serialize using f
  char *str;
  int  a;
  s >> str >> a;			\/ "the answer is" and 42
  f.close();				\/ done
  delete str;				\/ delete string
\endcode

\sa QTextStream.
*/

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

static int  systemWordSize = 0;
static bool systemBigEndian;


/*!
Constructs a data stream that has no IO device.
*/

QDataStream::QDataStream()
{
    if ( systemWordSize == 0 )			// get system features
	qSysInfo( &systemWordSize, &systemBigEndian );
    dev = 0;					// no device set
    byteorder = BigEndian;			// default byte order
    printable = FALSE;
    noswap = systemBigEndian;
}

/*!
Constructs a data stream that uses the IO device \e d.
*/

QDataStream::QDataStream( QIODevice *d )
{
    if ( systemWordSize == 0 )			// get system features
	qSysInfo( &systemWordSize, &systemBigEndian );
    dev = d;					// set device
    byteorder = BigEndian;			// default byte order
    printable = FALSE;
    noswap = systemBigEndian;
}

/*!
Destroys the data stream.

The destructor will not affect the current IO device.
*/

QDataStream::~QDataStream()
{
}


/*!
\fn QIODevice *QDataStream::device() const
Returns the IO device currently set.
*/

/*!
\fn void QDataStream::setDevice(QIODevice *d )
Sets the IO device to \e d.
*/

/*! \fn void QDataStream::unsetDevice()
Unsets the IO device.  This is the same as calling setDevice( 0 ).
*/

/*!
\fn bool QDataStream::eos() const
Returns TRUE if the IO device has reached the end position (end of stream) or
if there is no IO device set.

Returns FALSE if the current position of the read/write head of the IO
device is somewhere before the end position.
*/

/*!
\fn int QDataStream::byteOrder() const
Returns the current byte order setting.

\sa setByteOrder().
*/

/*!
Sets the serialization byte order to \e bo.

The \e bo parameter can be QDataStream::BigEndian or QDataStream::LittleEndian.

The default setting is big endian.  We recommend leaving this setting unless
you have special requirements.
*/

void QDataStream::setByteOrder( int bo )
{
    byteorder = bo;
    if ( byteorder == LittleEndian )
	noswap = !systemBigEndian;
    else
	noswap = systemBigEndian;
}


/*!
\fn bool QDataStream::isPrintableData() const
Returns TRUE if the printable data flag has been set.

\sa setPrintableData().
*/

/*!
\fn void QDataStream::setPrintableData( bool enable )
Sets or clears the printable data flag.

If this flag is set, the write functions will generate output that
consists of printable characters (7 bit ASCII).

We recommend enabling printable data only for debugging purposes
(it is slower and creates bigger output).
*/


// --------------------------------------------------------------------------
// QDataStream read functions
//

static INT32 read_int_ascii( QDataStream *s )	// read data7 int constant
{
    register int n = 0;
    char buf[40];
    while ( TRUE ) {
	buf[n] = s->device()->getch();
	if ( buf[n] == '\n' || n > 38 )		// $-terminator
	    break;
	n++;
    }
    buf[n] = '\0';
    return atol( buf );
}


/*!
\fn QDataStream &QDataStream::operator>>( UINT8 &i )
Reads an unsigned byte from the stream and returns a reference to
the stream.
*/

/*!
Reads a signed byte from the stream.
*/

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


/*!
\fn QDataStream &QDataStream::operator>>( UINT16 &i )
Reads an unsigned 16-bit integer from the stream and returns a reference to
the stream.
*/

/*!
Reads a signed 16-bit integer from the stream and returns a reference to
the stream.
*/

QDataStream &QDataStream::operator>>( INT16 &i )// read 16-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	i = (INT16)read_int_ascii( this );
    else

    if ( noswap )				// no conversion needed
	dev->readBlock( (char *)&i, sizeof(INT16) );
    else {					// swap bytes
	dev->readBlock( (char *)&i, sizeof(INT16) );
	i = ((i >> 8) & 0xff) | ((i<< 8) & 0xff00);
    }
    return *this;
}


/*!
\fn QDataStream &QDataStream::operator>>( UINT32 &i )
Reads an unsigned 32-bit integer from the stream and returns a reference to
the stream.
*/

/*!
Reads a signed 32-bit integer from the stream and returns a reference to
the stream.
*/

QDataStream &QDataStream::operator>>( INT32 &i )// read 32-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	i = read_int_ascii( this );
    else
    if ( noswap )				// no conversion needed
	dev->readBlock( (char *)&i, sizeof(INT32) );
    else {					// swap bytes
	register unsigned char *p = (unsigned char*)(&i);
	dev->readBlock( (char *)p, 4 );
	i = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    }
    return *this;
}


/*!
Reads a signed integer from the stream as a 32-bit signed integer (INT32).
Returns a reference to the stream.
*/

QDataStream &QDataStream::operator>>( int &i )	// read integer as INT32
{
    INT32 n;
    *this >> n;
    i = (int)n;
    return *this;
}


/*!
Reads an unsigned integer from the stream as a 32-bit unsigned integer
(UINT32).  Returns a reference to the stream.
*/

QDataStream &QDataStream::operator>>( uint &i )	// read uinteger as UINT32
{
    UINT32 n;
    *this >> n;
    i = (uint)n;
    return *this;
}


static double read_double_ascii( QDataStream *s )// read data7 double constant
{
    register int n = 0;
    char buf[80];
    while ( TRUE ) {
	buf[n] = s->device()->getch();
	if ( buf[n] == '\n' || n > 78 )		// $-terminator
	    break;
	n++;
    }
    buf[n] = '\0';
    return atof( buf );
}


/*!
Reads a 32-bit floating point number from the stream using the standard
IEEE754 format.  Returns a reference to the stream.
*/

QDataStream &QDataStream::operator>>( float &f )// read 32-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	f = (float)read_double_ascii( this );
    else
    if ( noswap )				// no conversion needed
	dev->readBlock( (char *)&f, sizeof(float) );
    else {					// swap bytes
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


/*!
Reads a 64-bit floating point number from the stream using the standard
IEEE754 format.  Returns a reference to the stream.
*/

QDataStream &QDataStream::operator>>( double &f)// read 64-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable )				// printable data
	f = read_double_ascii( this );
    else
    if ( noswap )				// no conversion needed
	dev->readBlock( (char *)&f, sizeof(double) );
    else {					// swap bytes
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


/*!
Reads the '\0'-terminated string \e s from the stream and returns
a reference to the stream.

The string is serialized using readBytes().
*/

QDataStream &QDataStream::operator>>( char *&s )// read char array
{
    uint len = 0;
    return readBytes( s, len );
}


/*!
Reads the buffer \e s from the stream and returns a reference to the
stream.

The buffer \e s will be allocated with \c new. Destroy it with the
\c delete operator.
If \e s cannot be allocated, \e s will be set to 0.

The \e l parameter will be set to the length of the buffer.

The serialization format is an UINT32 length specifier first, then
the data (\e length bytes).


\sa readRawBytes().
*/

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


/*!
Reads \e len bytes from the stream into \e e s and returns a reference to
the stream.

The buffer \e s must be preallocated.

\sa readBytes() and QIODevice::readBlock().
*/

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

/*!
\fn QDataStream &QDataStream::operator<<( UINT8 i )
Writen an unsigned byte to the stream and returns a reference to
the stream.
*/

/*!
Writes a signed byte to the stream.
*/

QDataStream &QDataStream::operator<<( INT8 i )	// write 8-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable && (i == '\\' || !isprint(i)) ) {
	char buf[6];				// write octal code
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


/*!
\fn QDataStream &QDataStream::operator<<( UINT16 i )
Writes an unsigned 16-bit integer to the stream and returns a reference to
the stream.
*/

/*!
Writes a signed 16-bit integer to the stream and returns a reference to
the stream.
*/

QDataStream &QDataStream::operator<<( INT16 i )	// write 16-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[16];
	sprintf( buf, "%d\n", i );
	dev->writeBlock( buf, strlen(buf) );
    }
    else
    if ( noswap )				// no conversion needed
	dev->writeBlock( (char *)&i, sizeof(INT16) );
    else {					// swap bytes
	i = ((i >> 8) & 0xff) | ((i<< 8) & 0xff00);
	dev->writeBlock( (char *)&i, 2 );
    }
    return *this;
}


/*!
\fn QDataStream &QDataStream::operator<<( UINT32 i )
Writes an unsigned 32-bit integer to the stream and returns a reference to
the stream.
*/

/*!
Writes a signed 32-bit integer to the stream and returns a reference to
the stream.
*/

QDataStream &QDataStream::operator<<( INT32 i )	// write 32-bit signed int
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[16];
	sprintf( buf, "%ld\n", i );
	dev->writeBlock( buf, strlen(buf) );
    }
    else
    if ( noswap )				// no conversion needed
	dev->writeBlock( (char *)&i, sizeof(INT32) );
    else {					// swap bytes
	register unsigned char *p = (unsigned char *)(&i);
	i = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
	dev->writeBlock( (char *)p, 4 );
    }
    return *this;
}


/*!
\fn QDataStream &QDataStream::operator<<( uint i )
Writes an unsigned integer to the stream as a 32-bit unsigned integer (UINT32).
Returns a reference to the stream.
*/

/*!
\fn QDataStream &QDataStream::operator<<( int i )
Writes a signed integer to the stream as a 32-bit signed integer (INT32).
Returns a reference to the stream.
*/


/*!
Writes a 32-bit floating point number to the stream using the standard
IEEE754 format.  Returns a reference to the stream.
*/

QDataStream &QDataStream::operator<<( float f )	// write 32-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[32];
	sprintf( buf, "%g\n", f );
	dev->writeBlock( buf, strlen(buf) );
    }
    else {
	float g = f;				// fixes float-on-stack problem
	if ( noswap )				// no conversion needed
	    dev->writeBlock( (char *)&g, sizeof(float) );
	else {					// swap bytes
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


/*!
Writes a 64-bit floating point number to the stream using the standard
IEEE754 format.  Returns a reference to the stream.
*/

QDataStream &QDataStream::operator<<( double f )// write 64-bit floating point
{
    CHECK_STREAM_PRECOND
    if ( printable ) {				// printable data
	char buf[32];
	sprintf( buf, "%g\n", f );
	dev->writeBlock( buf, strlen(buf) );
    }
    else
    if ( noswap )				// no conversion needed
	dev->writeBlock( (char *)&f, sizeof(double) );
    else {					// swap bytes
	register unsigned char *p = (unsigned char *)(&f);
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


/*!
Writes the '\0'-terminated string \e s to the stream and returns
a reference to the stream.

The string is serialized using writeBytes().
*/

QDataStream &QDataStream::operator<<( const char *s )
{						// write 0-term char array
    uint len = strlen( s ) + 1;			// also write null terminator
    *this << (UINT32)len;			// write length specifier
    return writeRawBytes( s, len );
}


/*!
Writes the length specifier \e len and the buffer \e s to the stream and
returns a reference to the stream.

The \e len is serialized as an UINT32, followed by \e len bytes from
\e s.

\sa writeRawBytes().
*/

QDataStream &QDataStream::writeBytes(const char *s, uint len)
{						// write char array with length
    CHECK_STREAM_PRECOND
    *this << (UINT32)len;			// write length specifier
    if ( len )
	writeRawBytes( s, len );
    return *this;
}


/*!
Writes \e len bytes from \e s to the stream and returns a reference to the
stream.

The \e len is serialized as an UINT32, followed by \e len bytes from
\e s.

\sa writeBytes(), QIODevice::writeBlock().
*/

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

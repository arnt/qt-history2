/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtstream.cpp#12 $
**
** Implementation of QTextStream class
**
** Author  : Haavard Nord
** Created : 940922
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtstream.h"
#include "qfile.h"
#include "qstring.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qtstream.cpp#12 $";
#endif


/*!
\class QTextStream qtstream.h
\brief The QTextStream class provides basic functions for reading and
writing text to an QIODevice.

  \ingroup tools

The text stream class a functional interface that is very similar to
that of the standard C++ iostream class.  The difference between
iostream and QTextStream is that our stream operates on a QIODevice.

The QTextStream class reads and writes ASCII text and it is not
appropriate for dealing with binary data (but QDataStream is).

\sa QDataStream.
*/

/*! \class QTSManip qtstream.h

  \brief The QTSManip stream is used for manipuating QTextStream in
  much the same way as iomanip manipulates iostream.

  This class is not yet documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */


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


#define I_SHORT		0x0010
#define I_INT		0x0020
#define I_LONG		0x0030
#define I_TYPE_MASK	0x00f0

#define I_BASE_2	QTS::bin
#define I_BASE_8	QTS::oct
#define I_BASE_10	QTS::dec
#define I_BASE_16	QTS::hex
#define I_BASE_MASK	(QTS::bin | QTS::oct | QTS::dec | QTS::hex)

#define I_SIGNED	0x0100
#define I_UNSIGNED	0x0200
#define I_SIGN_MASK	0x0f00


const long QTS::basefield   = I_BASE_MASK;
const long QTS::adjustfield = QTS::left | QTS::right | QTS::internal;
const long QTS::floatfield  = QTS::scientific | QTS::fixed;


/*!
Constructs a data stream that has no IO device.
*/

QTextStream::QTextStream()
{
    dev = 0;					// no device set
    fstrm = owndev = FALSE;
    reset();
}

/*!
Constructs a data stream that uses the file handle \e fh (sort of IO device).

This constructor makes it convenient to do such things:
\code
  QTextStream cout( stdout );
  QTextStream cin ( stdin );
  QTextStream cerr( stderr );
\endcode
*/

QTextStream::QTextStream( FILE *fh )
{
    dev = new QFile;
    ((QFile *)dev)->open( IO_ReadWrite, fh );
    fstrm = owndev = TRUE;
    reset();
}

/*!
Constructs a text stream that uses the IO device \e d.
*/

QTextStream::QTextStream( QIODevice *d )
{
    dev = d;					// set device
    fstrm = owndev = FALSE;
    reset();
}

/*!
Destroys the text stream.

The destructor will not affect the current IO device.
*/

QTextStream::~QTextStream()
{
    if ( owndev )
	delete dev;
}


/*!
Resets the text stream.

<ul>
<li> All flags are set to 0.
<li> Width is set to 0.
<li> Fill character is set to ' ' (space).
<li> Precision is set to 6.
</ul>

\sa setf() width(), fill() and precision().
*/

void QTextStream::reset()
{
    fflags = 0;
    fwidth = 0;
    fillchar = ' ';
    fprec = 6;
}


/*!
\fn QIODevice *QTextStream::device() const
Returns the IO device currently set.
*/

/*!
Sets the IO device to \e d.
*/

void QTextStream::setDevice( QIODevice *d )
{
    if ( owndev ) {
	delete dev;
	owndev = 0;
    }
    dev = d;   
}

/*!
Unsets the IO device.

Same as calling setDevice( 0 ).
*/

void QTextStream::unsetDevice()
{
    setDevice( 0 );
}

/*!
\fn bool QTextStream::eos() const
Returns TRUE if the IO device has reached the end position (end of stream) or
if there is no IO device set.

Returns FALSE if the current position of the read/write head of the IO
device is somewhere before the end position.
*/


// --------------------------------------------------------------------------
// QTextStream read functions
//


/*!
Reads a \c char from the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator>>( char &c )
{
    CHECK_STREAM_PRECOND
    if ( flags() & skipws )
	ws( *this );
    c = dev->getch();
    return *this;
}


static ulong input_bin( QTextStream *s )
{
    ulong val = 0;
    register int c = s->device()->getch();
    while ( c == '0' || c == '1' ) {
	val <<= 1;
	val += c - '0';
	c = s->device()->getch();
    }
    s->device()->ungetch( c );
    return val;
}

static ulong input_oct( QTextStream *s )
{
    ulong val = 0;
    register int c = s->device()->getch();
    while ( c >= '0' && c <= '7' ) {
	val <<= 3;
	val += c - '0';
	c = s->device()->getch();
    }
    s->device()->ungetch( c );
    return val;
}

static ulong input_dec( QTextStream *s )
{
    ulong val = 0;
    register int c = s->device()->getch();
    while ( isdigit(c) ) {
	val *= 10;
	val += c - '0';
	c = s->device()->getch();
    }
    s->device()->ungetch( c );
    return val;
}

static ulong input_hex( QTextStream *s )
{
    ulong val = 0;
    register int c = s->device()->getch();
    while ( isxdigit(c) ) {
	val <<= 4;
	if ( isdigit(c) )
	    val += c - '0';
	else
	    val += 10 + tolower(c) - 'a';
	c = s->device()->getch();
    }
    s->device()->ungetch( c );
    return val;
}

long QTextStream::input_int()
{
    long val;
    int c;
    if ( flags() & skipws )			// skip whitespace
	ws( *this );
    switch ( flags() & basefield ) {
	case bin:
	    val = (long)input_bin( this );
	    break;
	case oct:
	    val = (long)input_oct( this );
	    break;
	case dec:
	    c = dev->getch();
	    if ( !(c == '-' || c == '+') )
		dev->ungetch( c );
	    val = (long)input_dec( this );
	    if ( c == '-' )
		val = -val;
	    break;
	case hex:
	    val = (long)input_hex( this );
	    break;
	default:
	    val = 0;
	    c = dev->getch();
	    if ( c == '0' ) {		// bin, oct or hex
		c = dev->getch();
		if ( tolower(c) == 'x' )
		    val = (long)input_hex( this );
		else if ( tolower(c) == 'b' )
		    val = (long)input_bin( this );
		else {			// octal
		    dev->ungetch( c );
		    val = (long)input_oct( this );
		}
	    }
	    else if ( c >= '1' && c <= '9' ) {
		dev->ungetch( c );
		val = (long)input_dec( this );
	    }
	    else if ( c == '-' || c == '+' ) {
		val = (long)input_dec( this );
		if ( c == '-' )
		    val = -val;
	    }
    }
    return val;
}


/*!
Reads a signed \c short integer from the stream and returns a reference to the
stream.
*/

QTextStream &QTextStream::operator>>( signed short &i )
{
    CHECK_STREAM_PRECOND
    i = (signed short)input_int();
    return *this;
}


/*!
Reads an unsigned \c short integer from the stream and returns a reference to
the stream.
*/

QTextStream &QTextStream::operator>>( unsigned short &i )
{
    CHECK_STREAM_PRECOND
    i = (unsigned short)input_int();
    return *this;
}


/*!
Reads a signed \c int from the stream and returns a reference to the
stream.
*/

QTextStream &QTextStream::operator>>( signed int &i )
{
    CHECK_STREAM_PRECOND
    i = (signed int)input_int();
    return *this;
}


/*!
Reads an unsigned \c int from the stream and returns a reference to the
stream.
*/

QTextStream &QTextStream::operator>>( unsigned int &i )
{
    CHECK_STREAM_PRECOND
    i = (unsigned int)input_int();
    return *this;
}


/*!
Reads a signed \c long int from the stream and returns a reference to the
stream.
*/

QTextStream &QTextStream::operator>>( signed long &i )
{
    CHECK_STREAM_PRECOND
    i = (signed long)input_int();
    return *this;
}


/*!
Reads an unsigned \c long int from the stream and returns a reference to the
stream.
*/

QTextStream &QTextStream::operator>>( unsigned long &i )
{
    CHECK_STREAM_PRECOND
    i = (unsigned long)input_int();
    return *this;
}


/*!
Reads a \c float from the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator>>( float &f )
{
    CHECK_STREAM_PRECOND
    double f2;
    *this >> f2;
    f = (float)f2;
    return *this;
}


/*!
Reads a \c double from the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator>>( double & )
{
    CHECK_STREAM_PRECOND
    return *this;
}


/*!
Reads a string from the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator>>( char *& )
{
    CHECK_STREAM_PRECOND
    return *this;
}


// --------------------------------------------------------------------------
// QTextStream write functions
//

/*!
Writes a \c char to the stream and returns a reference to the stream.
*/

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
    char buf[76];
    register char *p;
    int   len;
    char *hexdigits;

    switch ( flags() & I_BASE_MASK ) {

	case I_BASE_2:				// output binary number
	    switch ( format & I_TYPE_MASK ) {
		case I_SHORT: len=16; break;
		case I_INT:   len=sizeof(int)*8; break;
		case I_LONG:  len=32; break;
		default:      len = 0;
	    }
	    p = &buf[74];			// go reverse order
	    *p = '\0';
	    while ( len-- ) {
		*--p = (char)(n&1) + '0';
		n >>= 1;
	    }
	    if ( flags() & showbase ) {		// show base
		*--p = (flags() & uppercase) ? 'B' : 'b';
		*--p = '0';
	    }
	    break;

	case I_BASE_8:				// output octal number
	    p = &buf[74];
	    *p = '\0';
	    do {
		*--p = (char)(n&7) + '0';
		n >>= 3;
	    } while ( n );
	    if ( flags() & showbase )
		*--p = '0';
	    break;

	case I_BASE_16:				// output hexadecimal number
	    p = &buf[74];
	    *p = '\0';
	    hexdigits = (flags() & uppercase) ?
		hexdigits_upper : hexdigits_lower;
	    do {
		*--p = hexdigits[(int)n&0xf];
		n >>= 4;
	    } while ( n );
	    if ( flags() & showbase ) {
		*--p = (flags() & uppercase) ? 'X' : 'x';
		*--p = '0';
	    }
	    break;

	default:				// decimal base is default
	    p = &buf[74];
	    *p = '\0';
	    if ( neg )
		n = (ulong)(-(long)n);
	    do {
		*--p = ((int)(n%10)) + '0';
		n /= 10;
	    } while ( n );
	    if ( neg )
		*--p = '-';
	    else if ( flags() & showpos )
		*--p = '+';
	    if ( (flags() & internal) && fwidth && !isdigit(*p) ) {
		dev->putch( *p );		// special case for internal
		++p;				//   padding
		fwidth--;
		return *this << (const char *)p;
	    }
    }
    if ( fwidth ) {				// adjustment required
	if ( !(flags() & left) ) {		// but NOT left adjustment
	    len = strlen(p);
	    int padlen = fwidth - len;
	    if ( padlen <= 0 )			// no padding required
		dev->writeBlock( p, len );
	    else if ( padlen < (int)(p-buf) ) {	// speeds up padding
		memset( p-padlen, fillchar, padlen );
		dev->writeBlock( p-padlen, padlen+len );
	    }
	    else				// standard padding
		*this << (const char *)p;
	}
	else
	    *this << (const char *)p;
	fwidth = 0;				// reset field width
    }
    else
	dev->writeBlock( p, strlen(p) );
    return *this;
}


/*!
Writes a signed \c short integer to the stream and returns a reference to
the stream.
*/

QTextStream &QTextStream::operator<<( signed short i )
{
    return output_int( I_SHORT | I_SIGNED, i, i < 0 );
}


/*!
Writes an unsigned \c short integer to the stream and returns a reference to
the stream.
*/

QTextStream &QTextStream::operator<<( unsigned short i )
{
    return output_int( I_SHORT | I_UNSIGNED, i, FALSE );
}


/*!
Writes a signed \c int to the stream and returns a reference to
the stream.
*/

QTextStream &QTextStream::operator<<( signed int i )
{
    return output_int( I_INT | I_SIGNED, i, i < 0 );
}


/*!
Writes an unsigned \c int to the stream and returns a reference to
the stream.
*/

QTextStream &QTextStream::operator<<( unsigned int i )
{
    return output_int( I_INT | I_UNSIGNED, i, FALSE );
}


/*!
Writes a signed \c long int to the stream and returns a reference to
the stream.
*/

QTextStream &QTextStream::operator<<( signed long i )
{
    return output_int( I_LONG | I_SIGNED, i, i < 0 );
}


/*!
Writes an unsigned \c long int to the stream and returns a reference to
the stream.
*/

QTextStream &QTextStream::operator<<( unsigned long i )
{
    return output_int( I_LONG | I_UNSIGNED, i, FALSE );
}


/*!
Writes a \c float to the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<( float f )
{
    return *this << (double)f;
}


/*!
Writes a \c double to the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<( double f )
{
    CHECK_STREAM_PRECOND
    char buf[64];
    char f_char;
    char format[16];
    if ( (flags()&floatfield) == fixed )
	f_char = 'f';
    else if ( (flags()&floatfield) == scientific )
	f_char = (flags() & uppercase) ? 'E' : 'e';
    else
	f_char = (flags() & uppercase) ? 'G' : 'g';
    register char *fs = format;			// generate format string
    *fs++ = '%';				//   "%.<prec>l<f_char>"
    *fs++ = '.';
    int prec = precision();
    if ( prec > 99 )
	prec = 99;
    if ( prec >= 10 ) {
	*fs++ = prec / 10 + '0';
	*fs++ = prec % 10 + '0';
    }
    else
	*fs++ = prec + '0';
    ASSERT( prec > 0 && prec <10 );	// TESTING
    *fs++ = 'l';
    *fs++ = f_char;
    *fs = '\0';
    sprintf( buf, format, f );			// the easy way out...
    if ( fwidth )				// padding
	*this << (const char *)buf;
    else					// just write it
	dev->writeBlock( buf, strlen(buf) );
    return *this;
}


/*!
Writes a string to the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<( const char *s )
{						// write 0-term char array
    CHECK_STREAM_PRECOND
    char padbuf[48];
    uint len = strlen( s );			// don't write null terminator
    if ( fwidth ) {				// field width set
	int padlen = fwidth - len;
	fwidth = 0;				// reset width
	if ( padlen > 0 ) {
	    char *ppad;
	    if ( padlen > 46 ) {		// create extra big fill buffer
		ppad = new char[padlen];
		CHECK_PTR( ppad );
	    }
	    else
		ppad = padbuf;
	    memset( ppad, fillchar, padlen );	// fill with fillchar
	    if ( !(flags() & left) ) {
		dev->writeBlock( ppad, padlen );
		padlen = 0;
	    }
	    dev->writeBlock( s, len );
	    if ( padlen )
		dev->writeBlock( ppad, padlen );
	    if ( ppad != padbuf )		// delete extra big fill buf
		delete[] ppad;
	    return *this;
	}
    }
    dev->writeBlock( s, len );
    return *this;
}


/*!
Writes a pointer to the stream and returns a reference to the stream.

The \e ptr is output as an unsigned long hexadecimal integer.
*/

QTextStream &QTextStream::operator<<( void *ptr )
{
    long f = flags();
    setf( hex, basefield );
    setf( showbase );
    unsetf( uppercase );
    output_int( I_LONG | I_UNSIGNED, (ulong)ptr, FALSE );
    flags( f );
    return *this;
}


/*!
Reads \e len bytes from the stream into \e e s and returns a reference to
the stream.

The buffer \e s must be preallocated.

\sa QIODevice::readBlock().
*/

QTextStream &QTextStream::readRawBytes( char *s, uint len )
{
    dev->readBlock( s, len );
    return *this;
}

/*!
Writes the \e len bytes from \e s to the stream and returns a reference to
the stream.

\sa QIODevice::writeBlock().
*/

QTextStream &QTextStream::writeRawBytes( const char *s, uint len )
{
    dev->writeBlock( s, len );
    return *this;
}


// --------------------------------------------------------------------------
// QTextStream manipulators
//

QTextStream &bin( QTextStream &s )
{
    s.setf(QTS::bin,QTS::basefield);
    return s;
}

QTextStream &oct( QTextStream &s )
{
    s.setf(QTS::oct,QTS::basefield);
    return s;
}

QTextStream &dec( QTextStream &s )
{
    s.setf(QTS::dec,QTS::basefield);
    return s;
}

QTextStream &hex( QTextStream &s )
{
    s.setf(QTS::hex,QTS::basefield);
    return s;
}

QTextStream &endl( QTextStream &s )
{
    return s << '\n';
}

QTextStream &flush( QTextStream &s )
{
    if ( s.device() )
	s.device()->flush();
    return s;
}

QTextStream &ws( QTextStream &s )
{
    register QIODevice *d = s.device();
    if ( d ) {
	int ch = d->getch();
	while ( isspace(ch) )
	    ch = d->getch();
	d->ungetch( ch );
    }
    return s;
}

QTextStream &reset( QTextStream &s )
{
    s.reset();
    return s;
}

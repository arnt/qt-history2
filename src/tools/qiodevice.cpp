/****************************************************************************
** $Id: //depot/qt/main/src/tools/qiodevice.cpp#6 $
**
** Implementation of QIODevice class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qiodev.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qiodevice.cpp#6 $";
#endif


/*!
\class QIODevice qiodev.h
\brief The QIODevice class is the base class of IO devices.

An IO device represents a medium that one can read bytes from and write
bytes to.  The QIODevice class itself is not capable of reading or writing
any data, but it has virtual functions for doing so. These functions are
implemented by the subclasses QFile, QBuffer etc.

There are two types of IO devices; <em>direct access</em> or <em>sequential
access </em> devices.
Files can normally be accessed directly, except \c stdin etc., which must be
processed sequentially.  Buffers are always direct access devices.

The access mode of an IO device can be either \e raw or \e buffered.
QFile objects can be creating using one of these.  Raw access mode is more
low level, while buffered access use smart buffering techniques.
The raw access mode is best when IO is block-operated using 4kB block size
or greater.  Buffered access works better when reading small portions of
data at a time.

An IO device operation can be executed in either \e synchronous or
\e asynchronous mode.  The IO devices currently supported by Qt only
execute synchronously.

\sa QDataStream, QTextStream, QTSManip.
*/


// --------------------------------------------------------------------------
// QIODevice member functions
//

/*!
Constructs an IO device.
*/

QIODevice::QIODevice()
{
    ioMode = 0;					// initial mode
    ioSt = IO_Ok;
    index = 0;
}

/*!
Destroys an IO device.
*/

QIODevice::~QIODevice()
{
}


/*!
\fn int QIODevice::flags() const
Returns the current IO device flags setting.

Flags consists of mode flags and state flags.

\sa mode() and state().
*/

/*!
\fn int QIODevice::mode() const
Returns bits OR'ed together that specify the current operation mode.

These are the flags that were given to the open() function.

The flags are: \c IO_ReadOnly, \c IO_WriteOnly, \c IO_ReadWrite,
\c IO_Append, \c IO_Translate, \c IO_Truncate.
*/

/*!
\fn int QIODevice::state() const
Returns bits OR'ed together that specify the current state.

The flags are: \c IO_Open.

Subclasses may define more flags.
*/


/*!
Internal, used by subclasses to set the device type.
*/

void QIODevice::setType( int t )		// set device type
{
#if defined(CHECK_RANGE)
    if ( (t & IO_TypeMask) != t )
	warning( "QIODevice::setType: Specified type out of range" );
#endif
    ioMode &= ~IO_TypeMask;			// reset type bits
    ioMode |= t;
}

/*!
Internal, used by subclasses to set the device mode.
*/

void QIODevice::setMode( int m )		// set device mode
{
#if defined(CHECK_RANGE)
    if ( (m & IO_ModeMask) != m )
	warning( "QIODevice::setMode: Specified mode out of range" );
#endif
    ioMode &= ~IO_ModeMask;			// reset mode bits
    ioMode |= m;
}

/*!
Internal, used by subclasses to set the device state.
*/

void QIODevice::setState( int s )		// set device state
{
#if defined(CHECK_RANGE)
    if ( ((uint)s & IO_StateMask) != (uint)s )
	warning( "QIODevice::setState: Specified state out of range" );
#endif
    ioMode &= ~IO_StateMask;			// reset state bits
    ioMode |= (uint)s;
}

/*!
Internal, used by subclasses to set the device status (not state).
*/

void QIODevice::setStatus( int s )		// set status
{
    ioSt = s;
}


/*!
Virtual function that returns the current IO device index.

This index is the data read/write head of the IO device.
*/

long QIODevice::at() const			// get data index
{
    return index;
}

/*!
Virtual function that sets the IO device index to \e n.
*/

bool QIODevice::at( long n )			// set data index
{
#if defined(CHECK_RANGE)
    if ( n > size() ) {
	warning( "QIODevice::at: Index %lu out of range", n );
	return FALSE;
    }
#endif
    index = n;
    return TRUE;
}

/*!
Virtual function that returns TRUE if the IO device index is at the
end of the input.
*/

bool QIODevice::atEnd() const			// at end of data
{
    return at() == size();
}


/*!
\fn int QIODev::readBlock( char *data, uint len ) = 0
Reads a \e len bytes from the IO device into the buffer pointer to by data.
Returns the number of bytes that actually were read.

This virtual function must be reimplemented by subclasses.
*/

/*!
\fn int QIODev::writeBlock( const char *data, uint len ) = 0
Write a \e len bytes from buffer to the IO device.
Returns the number of bytes that actually were written.

This virtual function must be reimplemented by subclasses.
*/

/*!
Reads a line of text, maximum \e maxlen bytes.

This virtual function can be reimplemented by subclasses.
*/

int QIODevice::readLine( char *data, uint maxlen )
{
    long pos = at();				// get current position
    long sz  = size();				// size of IO device
    char *p = data;
    while ( pos++ < sz && --maxlen ) {		// read one byte at a time
	readBlock( p, 1 );
	if ( *p++ == '\n' )			// end of line
	    break;
    }
    *p++ = '\0';
    return (int)((long)p - (long)data);
}

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbuffer.cpp#45 $
**
** Implementation of QBuffer class
**
** Created : 930812
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qbuffer.h"
#include <stdlib.h>

// NOT REVISED
/*!
  \class QBuffer qbuffer.h
  \brief The QBuffer class is an I/O device that operates on a QByteArray

  \ingroup io

  QBuffer is an I/O device for reading and writing a memory buffer. A
  QBuffer may be used directly (readBlock() and writeBlock()) or more
  conveniently via QDataStream or QTextStream.  Most of its behavior
  is inherited from QIODevice.

  A QBuffer has an associated QByteArray which holds the buffer data.
  Writing data at the end (i.e. size()) of the buffer expands the byte
  array.

  For convenience, the byte stream classes QDataStream and QTextStream
  can operate on a QByteArray (or a QString) via an internal QBuffer:
  \code
    QString str;
    QTextStream ts( str, IO_WriteOnly );
    ts << "pi = " << 3.14;			// str == "pi = 3.14"
  \endcode

  \sa QFile, QDataStream, QTextStream
*/


/*!
  Constructs an empty buffer.
*/

QBuffer::QBuffer()
{
    setFlags( IO_Direct );
    a_inc = 16;					// initial increment
    a_len = 0;
    ioIndex = 0;
}


/*!
  Constructs a buffer and sets the buffer contents to \a buf.
  \sa setBuffer()
*/

QBuffer::QBuffer( QByteArray buf ) : a(buf)
{
    setFlags( IO_Direct );
    a_len = a.size();
    a_inc = (a_len > 512) ? 512 : a_len;	// initial increment
    if ( a_inc < 16 )
	a_inc = 16;
    ioIndex = 0;
}

/*!
  Destroys the buffer.
*/

QBuffer::~QBuffer()
{
}


/*!
  Replaces the buffer's contents with \a buf.

  This may not be done while the buffer is \link open() open\endlink.

  Note that if you open the buffer in write mode (\c IO_WriteOnly or
  IO_ReadWrite) and write something into the buffer, \a buf is also
  modified because QByteArray is an explicitly shared class.

  Example:
  \code
    QString str = "abc";
    QBuffer b( str );
    b.open( IO_WriteOnly );
    b.at( 3 );					// position at \0
    b.writeBlock( "def", 4 );			// write including \0
    b.close();
      // Now, str == "abcdef"
  \endcode

  \sa open, \link shclass.html Shared Classes\endlink
*/

bool QBuffer::setBuffer( QByteArray buf )
{
    if ( isOpen() ) {
#if defined(CHECK_STATE)
	qWarning( "QBuffer::setBuffer: Buffer is open");
#endif
	return FALSE;
    }
    a = buf;
    a_len = a.size();
    a_inc = (a_len > 512) ? 512 : a_len;	// initial increment
    if ( a_inc < 16 )
	a_inc = 16;
    ioIndex = 0;
    return TRUE;
}

/*!
  \fn QByteArray QBuffer::buffer() const

  Returns the buffer most recently set by setBuffer(), or at construction.
*/

/*!
  Opens the file specified by the file name currently set, using the mode \a m.
  Returns TRUE if successful, otherwise FALSE.

  The mode parameter \a m must be a combination of the following flags.
  <ul>
  <li>\c IO_ReadOnly opens a buffer in read-only mode.
  <li>\c IO_WriteOnly opens a buffer in write-only mode.
  <li>\c IO_ReadWrite opens a buffer in read/write mode.
  <li>\c IO_Append sets the buffer index to the end of the buffer.
  <li>\c IO_Truncate truncates the buffer.
  </ul>

  \sa close(), isOpen()
*/

bool QBuffer::open( int m  )
{
    if ( isOpen() ) {				// buffer already open
#if defined(CHECK_STATE)
	qWarning( "QBuffer::open: Buffer already open" );
#endif
	return FALSE;
    }
    setMode( m );
    if ( m & IO_Truncate ) {			// truncate buffer
	a.resize( 0 );
	a_len = 0;
    }
    if ( m & IO_Append ) {			// append to end of buffer
	ioIndex = a.size();
    } else {
	ioIndex = 0;
    }
    a_inc = 16;
    setState( IO_Open );
    setStatus( 0 );
    return TRUE;
}

/*!
  Closes an open buffer.
  \sa open()
*/

void QBuffer::close()
{
    if ( isOpen() ) {
	setFlags( IO_Direct );
	ioIndex = 0;
	a_inc = 16;
    }
}

/*!
  The flush function does nothing.
*/

void QBuffer::flush()
{
    return;
}


/*!
  \fn int QBuffer::at() const
  Returns the buffer index.
  \sa size()
*/

/*!
  \fn uint QBuffer::size() const
  Returns the number of bytes in the buffer.
  \sa at()
*/

/*!
  Sets the buffer index to \a pos. Returns TRUE if successful, otherwise FALSE.
  \sa size()
*/

bool QBuffer::at( int pos )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {
	qWarning( "QBuffer::at: Buffer is not open" );
	return FALSE;
    }
#endif
    if ( (uint)pos > a_len ) {
#if defined(CHECK_RANGE)
	qWarning( "QBuffer::at: Index %d out of range", pos );
#endif
	return FALSE;
    }
    ioIndex = pos;
    return TRUE;
}


/*!
  Reads at most \a len bytes from the buffer into \a p and returns the
  number of bytes actually read.

  Returns -1 if a serious error occurred.

  \sa writeBlock()
*/

int QBuffer::readBlock( char *p, uint len )
{
#if defined(CHECK_STATE)
    CHECK_PTR( p );
    if ( !isOpen() ) {				// buffer not open
	qWarning( "QBuffer::readBlock: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	qWarning( "QBuffer::readBlock: Read operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)ioIndex + len > a.size() ) {	// overflow
	if ( (uint)ioIndex >= a.size() ) {
	    setStatus( IO_ReadError );
	    return -1;
	} else {
	    len = a.size() - (uint)ioIndex;
	}
    }
    memcpy( p, a.data()+ioIndex, len );
    ioIndex += len;
    return len;
}

/*!
  Writes \a len bytes from \a p into the buffer at the current index,
  overwriting any characters there and extending the buffer if necessary.
  Returns the number of bytes actually written.

  Returns -1 if a serious error occurred.

  \sa readBlock()
*/

int QBuffer::writeBlock( const char *p, uint len )
{
#if defined(CHECK_NULL)
    if ( p == 0 && len != 0 )
	qWarning( "QBuffer::writeBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	qWarning( "QBuffer::writeBlock: Buffer not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	qWarning( "QBuffer::writeBlock: Write operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)ioIndex + len >= a_len ) {		// overflow
	uint new_len = a_len + a_inc*(((uint)ioIndex+len-a_len)/a_inc+1);
	if ( !a.resize( new_len ) ) {		// could not resize
#if defined(CHECK_NULL)
	    qWarning( "QBuffer::writeBlock: Memory allocation error" );
#endif
	    setStatus( IO_ResourceError );
	    return -1;
	}
	a_inc *= 2;				// double increment
	a_len = new_len;
	a.shd->len = (uint)ioIndex + len;
    }
    memcpy( a.data()+ioIndex, p, len );
    ioIndex += len;
    if ( a.shd->len < (uint)ioIndex )
	a.shd->len = (uint)ioIndex;		// fake (not alloc'd) length
    return len;
}


/*!
  Reads a line of text.

  Reads bytes from the buffer until end-of-line is reached, or up to
  \a maxlen bytes.

  \sa readBlock()
*/

int QBuffer::readLine( char *p, uint maxlen )
{
#if defined(CHECK_STATE)
    CHECK_PTR( p );
    if ( !isOpen() ) {				// buffer not open
	qWarning( "QBuffer::readLine: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	qWarning( "QBuffer::readLine: Read operation not permitted" );
	return -1;
    }
#endif
    if ( maxlen == 0 )
	return 0;
    uint start = (uint)ioIndex;
    char *d = a.data() + ioIndex;
    maxlen--;					// make room for 0-terminator
    if ( a.size() - (uint)ioIndex < maxlen )
	maxlen = a.size() - (uint)ioIndex;
    while ( maxlen-- ) {
	if ( (*p++ = *d++) == '\n' )
	    break;
    }
    *p = '\0';
    ioIndex = d - a.data();
    return (uint)ioIndex - start;
}


/*!
  Reads a single byte/character from the buffer.

  Returns the byte/character read, or -1 if the end of the buffer has been
  reached.

  \sa putch(), ungetch()
*/

int QBuffer::getch()
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	qWarning( "QBuffer::getch: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	qWarning( "QBuffer::getch: Read operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)ioIndex+1 > a.size() ) {		// overflow
	setStatus( IO_ReadError );
	return -1;
    }
    return uchar(*(a.data()+ioIndex++));
}

/*!
  Writes the character \a ch into the buffer, overwriting
  the character at the current index, extending the buffer
  if necessary.

  Returns \a ch, or -1 if some error occurred.

  \sa getch(), ungetch()
*/

int QBuffer::putch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	qWarning( "QBuffer::putch: Buffer not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	qWarning( "QBuffer::putch: Write operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)ioIndex + 1 >= a_len ) {		// overflow
	char buf[1];
	buf[0] = (char)ch;
	if ( writeBlock(buf,1) != 1 )
	    return -1;				// write error
    } else {
	*(a.data() + ioIndex++) = (char)ch;
	if ( a.shd->len < (uint)ioIndex )
	    a.shd->len = (uint)ioIndex;
    }
    return ch;
}

/*!
  Puts the character \a ch back into the buffer and decrements the index if
  it is not zero.

  This function is normally called to "undo" a getch() operation.

  Returns \a ch, or -1 if some error occurred.

  \sa getch(), putch()
*/

int QBuffer::ungetch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	qWarning( "QBuffer::ungetch: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	qWarning( "QBuffer::ungetch: Read operation not permitted" );
	return -1;
    }
#endif
    if ( ch != -1 ) {
	if ( ioIndex )
	    ioIndex--;
	else
	    ch = -1;
    }
    return ch;
}

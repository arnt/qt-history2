/****************************************************************************
**
** Implementation of QBuffer class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbuffer.h"

/* The two macros below are to conserve the QBuffer semantics. The old
 * bytearray had explicit sharing making it easy to write to it. Since
 * it is now implicitly shared, we have the problem that we cannot
 * modify the bytearray member b of QBuffer without triggering a
 * detach. For this reason we store a pointer to the original
 * bytearray and a copy of it.
 *
 * Before writing to the buffer, we check if b is detached. If it is,
 * we use p, otherwise we assume the original pointer is still valid
 * and write to that one.
 *
 * We can only use p and not b within BEGIN_BUFFER_WRITE/END_BUFFER_WRITE pairs.
 */
#define BEGIN_BUFFER_WRITE \
    if (p == &b || b.isDetached()) \
	p = &b;         \
    else                \
	b.clear()

#define END_BUFFER_WRITE \
    b = *p


/*!
    \class QBuffer qbuffer.h
    \reentrant
    \brief The QBuffer class is an I/O device that operates on a QByteArray.

    \ingroup io

    QBuffer is used to read and write to a memory buffer. It is
    normally used with a QTextStream or a QDataStream. QBuffer has an
    associated QByteArray which holds the buffer data. The size() of
    the buffer is automatically adjusted as data is written.

    The constructor \c QBuffer(QByteArray) creates a QBuffer using an
    existing byte array. The byte array can also be set with
    setBuffer(). Writing to the QBuffer will modify the original byte
    array because QByteArray is \link shclass.html explicitly
    shared.\endlink

    Use open() to open the buffer before use and to set the mode
    (read-only, write-only, etc.). close() closes the buffer. The
    buffer must be closed before reopening or calling setBuffer().

    A common way to use QBuffer is through \l QDataStream or \l
    QTextStream, which have constructors that take a QBuffer
    parameter. For convenience, there are also QDataStream and
    QTextStream constructors that take a QByteArray parameter. These
    constructors create and open an internal QBuffer.

    Note that QTextStream can also operate on a QString (a Unicode
    string); a QBuffer cannot.

    You can also use QBuffer directly through the standard QIODevice
    functions readBlock(), writeBlock() readLine(), at(), getch(),
    putch() and ungetch().

    \sa QFile, QDataStream, QTextStream, QByteArray, \link shclass.html Shared Classes\endlink
*/


/*!
    Constructs an empty buffer.
*/

QBuffer::QBuffer()
{
    p = &b;
    setFlags( IO_Direct );
    ioIndex = 0;
}


/*!
    Constructs a buffer that operates on \a buf.

    If you open the buffer in write mode (\c IO_WriteOnly or
    \c IO_ReadWrite) and write something into the buffer, \a buf
    will be modified.

    Example:
    \code
    QCString str = "abc";
    QBuffer b( str );
    b.open( IO_WriteOnly );
    b.at( 3 ); // position at the 4th character (the terminating \0)
    b.writeBlock( "def", 4 ); // write "def" including the terminating \0
    b.close();
    // Now, str == "abcdef" with a terminating \0
    \endcode

    \sa setBuffer()
*/

QBuffer::QBuffer( QByteArray &buf )
{
    buf.detach();
    b = buf;
    p = &buf;
    setFlags( IO_Direct );
    ioIndex = 0;
}

/*!
    Constructs a buffer that operates on \a buf.
*/

QBuffer::QBuffer( const QByteArray &buf )
{
    b = buf;
    p = &b;
    setFlags( IO_Direct );
    ioIndex = 0;
}

/*!
    Destroys the buffer.
*/

QBuffer::~QBuffer()
{
    p = 0;
}


/*!
    Replaces the buffer's contents with \a buf and returns TRUE.

    Does nothing (and returns FALSE) if isOpen() is TRUE.

    Note that if you open the buffer in write mode (\c IO_WriteOnly or
    IO_ReadWrite) and write something into the buffer, \a buf is also
    modified because QByteArray is an explicitly shared class.

    \sa buffer(), open(), close()
*/

bool QBuffer::setBuffer( QByteArray &buf )
{
    if ( isOpen() ) {
        qWarning( "QBuffer::setBuffer: Buffer is open" );
        return FALSE;
    }
    buf.detach();
    b = buf;
    p = &buf;
    ioIndex = 0;
    return TRUE;
}

/*!
    \fn QByteArray QBuffer::buffer() const

    Returns this buffer's byte array.

    \sa setBuffer()
*/

/*!
    \reimp

    Opens the buffer in mode \a m. Returns TRUE if successful;
    otherwise returns FALSE. The buffer must be opened before use.

    The mode parameter \a m must be a combination of the following flags.
    \list
    \i \c IO_ReadOnly opens the buffer in read-only mode.
    \i \c IO_WriteOnly opens the buffer in write-only mode.
    \i \c IO_ReadWrite opens the buffer in read/write mode.
    \i \c IO_Append sets the buffer index to the end of the buffer.
    \i \c IO_Truncate truncates the buffer.
    \endlist

    \sa close(), isOpen()
*/

bool QBuffer::open( int m  )
{
    if ( isOpen() ) {                           // buffer already open
        qWarning( "QBuffer::open: Buffer already open" );
        return FALSE;
    }
    setMode( m );
    BEGIN_BUFFER_WRITE;
    if ( m & IO_Truncate )
        p->resize( 0 );
    if ( m & IO_Append )                       // append to end of buffer
        ioIndex = p->size();
    else
        ioIndex = 0;

    setState( IO_Open );
    resetStatus();
    END_BUFFER_WRITE;
    return TRUE;
}

/*!
    \reimp

    Closes an open buffer.

    \sa open()
*/

void QBuffer::close()
{
    if ( isOpen() ) {
        setFlags( IO_Direct );
        ioIndex = 0;
    }
}

/*!
    \reimp

    The flush function does nothing for a QBuffer.
*/

void QBuffer::flush()
{
    return;
}


/*!
    \fn QIODevice::Offset QBuffer::at() const

    \reimp
*/

/*!
    \fn QIODevice::Offset QBuffer::size() const

    \reimp
*/

/*!
  \reimp
*/

bool QBuffer::at( Offset pos )
{
    if ( !isOpen() ) {
        qWarning( "QBuffer::at: Buffer is not open" );
        return FALSE;
    }
    // #### maybe resize if not readonly?
    if ( pos > (Offset)b.size() ) {
        qWarning( "QBuffer::at: Index %lld out of range", pos );
        return FALSE;
    }
    ioIndex = pos;
    return TRUE;
}


/*!
  \reimp
*/

Q_LONG QBuffer::readBlock( char *p, Q_ULONG len )
{
    if ( !p ) {
	qWarning( "QBuffer::readBlock: Null pointer error" );
	return -1;
    }
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::readBlock: Buffer not open" );
        return -1;
    }
    if ( !isReadable() ) {                      // reading not permitted
        qWarning( "QBuffer::readBlock: Read operation not permitted" );
        return -1;
    }
    if ( ioIndex + len > b.size() ) {   // overflow
        if ( (int)ioIndex >= b.size() ) {
            return 0;
        } else {
            len = b.size() - ioIndex;
        }
    }
    memcpy( p, b.constData()+ioIndex, len );
    ioIndex += len;
    return len;
}

/*!
    \overload Q_LONG QBuffer::writeBlock( const QByteArray& data )

    This convenience function is the same as calling
    \c{writeBlock( data.data(), data.size() )} with \a data.
*/

/*!
    Writes \a len bytes from \a ptr into the buffer at the current
    index position, overwriting any characters there and extending the
    buffer if necessary. Returns the number of bytes actually written.

    Returns -1 if an error occurred.

    \sa readBlock()
*/

Q_LONG QBuffer::writeBlock( const char *ptr, Q_ULONG len )
{
    if ( len == 0 )
        return 0;

    if ( ptr == 0 ) {
        qWarning( "QBuffer::writeBlock: Null pointer error" );
        return -1;
    }
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::writeBlock: Buffer not open" );
        return -1;
    }
    if ( !isWritable() ) {                      // writing not permitted
        qWarning( "QBuffer::writeBlock: Write operation not permitted" );
        return -1;
    }
    BEGIN_BUFFER_WRITE;
    if ( ioIndex + len > p->size() ) {             // overflow
	p->resize(ioIndex +len);
        if ( p->size() != (int)(ioIndex +len) ) {           // could not resize
            qWarning( "QBuffer::writeBlock: Memory allocation error" );
            setStatus( IO_ResourceError );
	    END_BUFFER_WRITE;
            return -1;
        }
    }
    memcpy( p->data()+ioIndex, ptr, len );
    ioIndex += len;
    END_BUFFER_WRITE;
    return len;
}


/*!
  \reimp
*/

Q_LONG QBuffer::readLine( char *p, Q_ULONG maxlen )
{
    if ( p == 0 ) {
        qWarning( "QBuffer::readLine: Null pointer error" );
        return -1;
    }
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::readLine: Buffer not open" );
        return -1;
    }
    if ( !isReadable() ) {                      // reading not permitted
        qWarning( "QBuffer::readLine: Read operation not permitted" );
        return -1;
    }
    if ( maxlen == 0 )
        return 0;
    Q_ULONG start = ioIndex;
    const char *d = b.constData() + ioIndex;
    maxlen--;                                   // make room for 0-terminator
    if ( b.size() - ioIndex < maxlen )
        maxlen = b.size() - ioIndex;
    while ( maxlen-- ) {
        if ( (*p++ = *d++) == '\n' )
            break;
    }
    *p = '\0';
    ioIndex = d - b.constData();
    return ioIndex - start;
}


/*!
  \reimp
*/

int QBuffer::getch()
{
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::getch: Buffer not open" );
        return -1;
    }
    if ( !isReadable() ) {                      // reading not permitted
        qWarning( "QBuffer::getch: Read operation not permitted" );
        return -1;
    }
    if ( ioIndex + 1 > b.size() ) {               // overflow
        setStatus( IO_ReadError );
        return -1;
    }
    return uchar(*(b.constData()+ioIndex++));
}

/*!
    \reimp

    Writes the character \a ch into the buffer at the current index
    position, overwriting any existing character and extending the
    buffer if necessary.

    Returns \a ch, or -1 if an error occurred.

    \sa getch(), ungetch()
*/

int QBuffer::putch( int ch )
{
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::putch: Buffer not open" );
        return -1;
    }
    if ( !isWritable() ) {                      // writing not permitted
        qWarning( "QBuffer::putch: Write operation not permitted" );
        return -1;
    }
    BEGIN_BUFFER_WRITE;
    if ( ioIndex + 1 > p->size() ) {               // overflow
        char buf[1];
        buf[0] = (char)ch;
        if ( writeBlock(buf,1) != 1 ) {
	    END_BUFFER_WRITE;
            return -1;                          // write error
	}
    } else {
        *(p->data() + ioIndex++) = (char)ch;
    }
    END_BUFFER_WRITE;
    return ch;
}

/*!
  \reimp
*/

int QBuffer::ungetch( int ch )
{
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::ungetch: Buffer not open" );
        return -1;
    }
    if ( !isReadable() ) {                      // reading not permitted
        qWarning( "QBuffer::ungetch: Read operation not permitted" );
        return -1;
    }
    if ( ch != -1 ) {
        if ( ioIndex )
            ioIndex--;
        else
            ch = -1;
    }
    return ch;
}

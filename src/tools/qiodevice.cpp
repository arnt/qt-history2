/****************************************************************************
** $Id: $
**
** Implementation of QIODevice class
**
** Created : 940913
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qiodevice.h"

/*!
  \class QIODevice qiodevice.h

  \brief The QIODevice class is the base class of I/O devices.

  \ingroup io
  \mainclass

  An I/O device represents a medium that one can read bytes from
  and/or write bytes to.  The QIODevice class is the abstract
  superclass of all such devices; classes such as QFile, QBuffer and
  QSocket inherit QIODevice and implement virtual functions such as
  write() appropriately.

  Although applications sometimes use QIODevice directly, it is
  usually better to go through QTextStream and QDataStream, which provide
  stream operations on any QIODevice subclass.  QTextStream provides
  text-oriented stream functionality (for human-readable ASCII files,
  for example), whereas QDataStream deals with binary data in a totally
  platform-independent manner.

  The public member functions in QIODevice roughly fall into two
  groups: the action functions and the state access functions.  The
  most important action functions are:

  \list

  \i  open() opens a device for reading and/or writing, depending on
  the argument to open().

  \i  close() closes the device and tidies up.

  \i  readBlock() reads a block of data from the device.

  \i  writeBlock() writes a block of data to the device.

  \i  readLine() reads a line (of text, usually) from the device.

  \i  flush() ensures that all buffered data are written to the real device.

  \endlist

  There are also some other, less used, action functions:

  \list

  \i  getch() reads a single character.

  \i  ungetch() forgets the last call to getch(), if possible.

  \i  putch() writes a single character.

  \i  size() returns the size of the device, if there is one.

  \i  at() returns the current read/write pointer's position, if there
  is one for this device, or it moves the pointer.

  \i  atEnd() says whether there is more to read, if that is a
  meaningful question for this device.

  \i  reset() moves the read/write pointer to the start of the
  device, if that is possible for this device.

  \endlist

  The state access are all "get" functions.  The QIODevice subclass
  calls setState() to update the state, and simple access functions
  tell the user of the device what the device's state is.  Here are
  the settings, and their associated access functions:

  \list

  \i  Access type.  Some devices are direct access (it is possible to
  read/write anywhere), whereas others are sequential.  QIODevice
  provides the access functions (isDirectAccess(), isSequentialAccess(),
  and isCombinedAccess()) to tell users what a given I/O device
  supports.

  \i  Buffering.  Some devices are accessed in raw mode, whereas others
  are buffered.  Buffering usually provides greater efficiency,
  particularly for small read/write operations.  isBuffered() tells
  the user whether a given device is buffered.  (This can often be set
  by the application in the call to open().)

  \i  Synchronicity.  Synchronous devices work immediately (for
  example, files).  When you read from a file, the file delivers its
  data straight away.  Other kinds of device, such as a socket
  connected to a HTTP server, may not deliver the data until seconds
  after you ask to read it.  isSynchronous() and isAsynchronous() tell
  the user how this device operates.

  \i  CR/LF translation.  For simplicity, applications often like to
  see just a single CR/LF style, and QIODevice subclasses can provide
  this.  isTranslated() returns TRUE if this object translates CR/LF
  to just LF.  (This can often be set by the application in the call
  to open().)

  \i  Permissions.  Some files cannot be written. For example,
  isReadable(), isWritable() and isReadWrite() tell the application
  whether it can read from and write to a given device.  (This can
  often be set by the application in the call to open().)

  \i  Finally, isOpen() returns TRUE if the device is open, i.e. after
  an open() call.

  \endlist

  QIODevice provides numerous pure virtual functions that you need to
  implement when subclassing it.  Here is a skeleton subclass with all
  the members you are certain to need and some that you probably
  will need:

  \code
    class MyDevice : public QIODevice
    {
    public:
	MyDevice();
	~MyDevice();

	bool open( int mode );
	void close();
	void flush();

	uint size() const;
	int  at() const;	// non-pure virtual
	bool at( int );		// non-pure virtual
	bool atEnd() const;	// non-pure virtual

	int readBlock( char *data, uint maxlen );
	int writeBlock( const char *data, uint len );
	int readLine( char *data, uint maxlen );

	int getch();
	int putch( int );
	int ungetch( int );
    };
  \endcode

  The three non-pure virtual functions need not be reimplemented
  for sequential devices.

  \sa QDataStream, QTextStream
*/

/*! \enum QIODevice::Offset
    The offset within the device.
*/


/*!
  Constructs an I/O device.
*/

QIODevice::QIODevice()
{
    ioMode = 0;					// initial mode
    ioSt = IO_Ok;
    ioIndex = 0;
}

/*!
  Destroys the I/O device.
*/

QIODevice::~QIODevice()
{
}


/*!
  \fn int QIODevice::flags() const
  Returns the current I/O device flags setting.

  Flags consists of mode flags and state flags.

  \sa mode(), state()
*/

/*!
  \fn int QIODevice::mode() const
  Returns bits OR'ed together that specify the current operation mode.

  These are the flags that were given to the open() function.

  The flags are \c IO_ReadOnly, \c IO_WriteOnly, \c IO_ReadWrite,
  \c IO_Append, \c IO_Truncate and \c IO_Translate.
*/

/*!
  \fn int QIODevice::state() const
  Returns bits OR'ed together that specify the current state.

  The flags are: \c IO_Open.

  Subclasses may define additional flags.
*/

/*!
  \fn bool QIODevice::isDirectAccess() const
  Returns TRUE if the I/O device is a direct access device; otherwise
  returns FALSE, i.e. if the device is a sequential access device.
  \sa isSequentialAccess()
*/

/*!
  \fn bool QIODevice::isSequentialAccess() const
  Returns TRUE if the device is a sequential access device; otherwise
  returns FALSE, i.e. if the device is a direct access device.

  Operations involving size() and at(int) are not valid
  on sequential devices.
  \sa isDirectAccess()
*/

/*!
  \fn bool QIODevice::isCombinedAccess() const
  Returns TRUE if the I/O device is a combined access (both direct and
  sequential) device; otherwise returns FALSE.

  This access method is currently not in use.
*/

/*!
  \fn bool QIODevice::isBuffered() const
  Returns TRUE if the I/O device is a buffered device; otherwise
  returns FALSE, i.e. the device is a raw device.
  \sa isRaw()
*/

/*!
  \fn bool QIODevice::isRaw() const
  Returns TRUE if the device is a raw device; otherwise
  returns FALSE, i.e. if the device is a buffered device.
  \sa isBuffered()
*/

/*!
  \fn bool QIODevice::isSynchronous() const
  Returns TRUE if the I/O device is a synchronous device; otherwise
  returns FALSE, i.e. the device is an asynchronous device.
  \sa isAsynchronous()
*/

/*!
  \fn bool QIODevice::isAsynchronous() const
  Returns TRUE if the device is an asynchronous device; otherwise
  returns FALSE, i.e. if the device is a synchronous device.

  This mode is currently not in use.

  \sa isSynchronous()
*/

/*!
  \fn bool QIODevice::isTranslated() const
  Returns TRUE if the I/O device translates carriage-return and linefeed
  characters; otherwise returns FALSE.

  A QFile is translated if it is opened with the \c IO_Translate mode
  flag.
*/

/*!
  \fn bool QIODevice::isReadable() const
  Returns TRUE if the I/O device was opened using \c IO_ReadOnly or
  \c IO_ReadWrite mode; otherwise returns FALSE.
  \sa isWritable(), isReadWrite()
*/

/*!
  \fn bool QIODevice::isWritable() const
  Returns TRUE if the I/O device was opened using \c IO_WriteOnly or
  \c IO_ReadWrite mode; otherwise returns FALSE.
  \sa isReadable(), isReadWrite()
*/

/*!
  \fn bool QIODevice::isReadWrite() const
  Returns TRUE if the I/O device was opened using \c IO_ReadWrite
  mode; otherwise returns FALSE.
  \sa isReadable(), isWritable()
*/

/*!
  \fn bool QIODevice::isInactive() const
  Returns TRUE if the I/O device state is 0, i.e. the device is not
  open; otherwise returns FALSE.
  \sa isOpen()
*/

/*!
  \fn bool QIODevice::isOpen() const
  Returns TRUE if the I/O device has been opened; otherwise returns FALSE.
  \sa isInactive()
*/


/*!
  \fn int QIODevice::status() const
  Returns the I/O device status.

  The I/O device status returns an error code.	If open() returns FALSE
  or readBlock() or writeBlock() return -1, this function can be called to
  find out the reason why the operation did not succeed.

  \keyword IO_Ok
  \keyword IO_ReadError
  \keyword IO_WriteError
  \keyword IO_FatalError
  \keyword IO_OpenError
  \keyword IO_ConnectError
  \keyword IO_AbortError
  \keyword IO_TimeOutError
  \keyword IO_UnspecifiedError

  The status codes are:
  \list
  \i \c IO_Ok - The operation was successful.
  \i \c IO_ReadError - Could not read from the device.
  \i \c IO_WriteError - Could not write to the device.
  \i \c IO_FatalError - A fatal unrecoverable error occurred.
  \i \c IO_OpenError - Could not open the device.
  \i \c IO_ConnectError - Could not connect to the device.
  \i \c IO_AbortError - The operation was unexpectedly aborted.
  \i \c IO_TimeOutError - The operation timed out.
  \i \c IO_UnspecifiedError - An unspecified error happened on close.
  \endlist

  \sa resetStatus()
*/

/*!
  \fn void QIODevice::resetStatus()

  Sets the I/O device status to \c IO_Ok.

  \sa status()
*/


/*!
  \fn void QIODevice::setFlags( int f )
  \internal
  Used by subclasses to set the device flags.
*/

/*!
  \internal
  Used by subclasses to set the device type.
*/

void QIODevice::setType( int t )
{
#if defined(QT_CHECK_RANGE)
    if ( (t & IO_TypeMask) != t )
	qWarning( "QIODevice::setType: Specified type out of range" );
#endif
    ioMode &= ~IO_TypeMask;			// reset type bits
    ioMode |= t;
}

/*!
  \internal
  Used by subclasses to set the device mode.
*/

void QIODevice::setMode( int m )
{
#if defined(QT_CHECK_RANGE)
    if ( (m & IO_ModeMask) != m )
	qWarning( "QIODevice::setMode: Specified mode out of range" );
#endif
    ioMode &= ~IO_ModeMask;			// reset mode bits
    ioMode |= m;
}

/*!
  \internal
  Used by subclasses to set the device state.
*/

void QIODevice::setState( int s )
{
#if defined(QT_CHECK_RANGE)
    if ( ((uint)s & IO_StateMask) != (uint)s )
	qWarning( "QIODevice::setState: Specified state out of range" );
#endif
    ioMode &= ~IO_StateMask;			// reset state bits
    ioMode |= (uint)s;
}

/*!
  \internal
  Used by subclasses to set the device status (not state) to \a s.
*/

void QIODevice::setStatus( int s )
{
    ioSt = s;
}


/*!
  \fn bool QIODevice::open( int mode )
  Opens the I/O device using the specified \a mode.
  Returns TRUE if the device was successfully opened; otherwise
  returns FALSE.

  The mode parameter \a mode must be an OR'ed combination of the
  following flags.
  \list
  \i \c IO_Raw specified raw (unbuffered) file access.
  \i \c IO_ReadOnly opens a file in read-only mode.
  \i \c IO_WriteOnly opens a file in write-only mode.
  \i \c IO_ReadWrite opens a file in read/write mode.
  \i \c IO_Append sets the file index to the end of the file.
  \i \c IO_Truncate truncates the file.
  \i \c IO_Translate enables carriage returns and linefeed translation
  for text files under MS-DOS, Windows and Macintosh.  On Unix systems
  this flag has no effect. Use with caution as it will also transform
  every linefeed written to the file into a CRLF pair. This is likely to
  corrupt your file if you write write binary data. Cannot be combined
  with \c IO_Raw.
  \endlist

  This virtual function must be reimplemented by all subclasses.

  \sa close()
*/

/*!
  \fn void QIODevice::close()
  Closes the I/O device.

  This virtual function must be reimplemented by all subclasses.

  \sa open()
*/

/*!
  \fn void QIODevice::flush()

  Flushes an open I/O device.

  This virtual function must be reimplemented by all subclasses.
*/


/*!
  \fn QIODevice::Offset QIODevice::size() const
  Virtual function that returns the size of the I/O device.
  \sa at()
*/

/*!
  Virtual function that returns the current I/O device position.

  This is the position of the data read/write head of the I/O device.

  \sa size()
*/

QIODevice::Offset QIODevice::at() const
{
    return ioIndex;
}

/*!
    \overload
  Virtual function that sets the I/O device position to \a pos.
  \sa size()
*/

bool QIODevice::at( Offset pos )
{
#if defined(QT_CHECK_RANGE)
    if ( (uint)pos > size() ) {
	qWarning( "QIODevice::at: Index %ld out of range", pos );
	return FALSE;
    }
#endif
    ioIndex = pos;
    return TRUE;
}

/*!
  Virtual function that returns TRUE if the I/O device position is at the
  end of the input; otherwise returns FALSE.
*/

bool QIODevice::atEnd() const
{
    if ( isSequentialAccess() || isTranslated() ) {
	QIODevice* that = (QIODevice*)this;
	int c = that->getch();
	bool result = c < 0;
	that->ungetch(c);
	return result;
    } else {
	return at() == size();
    }
}

/*!
  \fn bool QIODevice::reset()
  Sets the device index position to 0.
  \sa at()
*/


/*!
  \fn int QIODevice::readBlock( char *data, Q_ULONG maxlen )
  Reads at most \a maxlen bytes from the I/O device into \a data and
  returns the number of bytes actually read.

  This virtual function must be reimplemented by all subclasses.

  \sa writeBlock()
*/

/*!
  This convenience function returns all of the remaining data in the
  device.
*/
QByteArray QIODevice::readAll()
{
    if ( isDirectAccess() ) {
	// we now the size
	int n = size()-at(); // ### fix for 64-bit or large files?
	QByteArray ba(size()-at());
	char* c = ba.data();
	while ( n ) {
	    int r = readBlock( c, n );
	    if ( r < 0 )
		return QByteArray();
	    n -= r;
	    c += r;
	}
	return ba;
    } else {
	// read until we reach the end
	const int blocksize = 512;
	int nread = 0;
	QByteArray ba;
	while ( !atEnd() ) {
	    ba.resize( nread + blocksize );
	    int r = readBlock( ba.data()+nread, blocksize );
	    if ( r < 0 )
		return QByteArray();
	    nread += r;
	}
	ba.resize( nread );
	return ba;
    }
}

/*!
  \fn int QIODevice::writeBlock( const char *data, Q_ULONG len )
  Writes \a len bytes from \a data to the I/O device and returns the
  number of bytes actually written.

  This virtual function must be reimplemented by all subclasses.

  \sa readBlock()
*/

/*!
    \overload
  This convenience function is the same as calling
  writeBlock( data.data(), data.size() ).
*/
Q_LONG QIODevice::writeBlock( const QByteArray& data )
{
    return writeBlock( data.data(), data.size() );
}

/*!
  Reads a line of text, (or up to \a maxlen bytes if a newline isn't
  encountered) plus a terminating \0 into \a data.  If there is a
  newline at the end if the line, it is not stripped.

  Returns the number of bytes read, or -1 in case of error.

  This virtual function can be reimplemented much more efficiently by
  the most subclasses.

  \sa readBlock(), QTextStream::readLine()
*/

Q_LONG QIODevice::readLine( char *data, Q_ULONG maxlen )
{
    if ( maxlen == 0 )				// application bug?
	return 0;
    Q_ULONG pos = at();				// get current position
    Q_ULONG s  = size();			// size of I/O device
    char *p = data;
    if ( pos >= s )
	return 0;
    while ( pos++ < s && --maxlen ) {		// read one byte at a time
	readBlock( p, 1 );
	if ( *p++ == '\n' )			// end of line
	    break;
    }
    *p++ = '\0';
    return p - data;
}


/*!
  \fn int QIODevice::getch()

  Reads a single byte/character from the I/O device.

  Returns the byte/character read, or -1 if the end of the I/O device has been
  reached.

  This virtual function must be reimplemented by all subclasses.

  \sa putch(), ungetch()
*/

/*!
  \fn int QIODevice::putch( int ch )

  Writes the character \a ch to the I/O device.

  Returns \a ch, or -1 if an error occurred.

  This virtual function must be reimplemented by all subclasses.

  \sa getch(), ungetch()
*/

/*!
  \fn int QIODevice::ungetch( int ch )

  Puts the character \a ch back into the I/O device and decrements the
  index position if it is not zero.

  This function is normally called to "undo" a getch() operation.

  Returns \a ch, or -1 if an error occurred.

  This virtual function must be reimplemented by all subclasses.

  \sa getch(), putch()
*/

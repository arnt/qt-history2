/****************************************************************************
** $Id: //depot/qt/main/src/tools/qiodev.cpp#18 $
**
** Implementation of QIODevice class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qiodev.h"

RCSTAG("$Id: //depot/qt/main/src/tools/qiodev.cpp#18 $")


/*----------------------------------------------------------------------------
  \class QIODevice qiodev.h

  \brief The QIODevice class is the base class of IO devices.

  \ingroup tools
  \ingroup files
  \ingroup streams

  An IO device represents a medium that one can read bytes from and write
  bytes to.  The QIODevice class itself is not capable of reading or writing
  any data, but it has virtual functions for doing so. These functions are
  implemented by the subclasses QFile and QBuffer.

  There are two types of IO devices; <em>direct access</em> and <em>sequential
  access </em> devices.
  Files can normally be accessed directly, except \c stdin etc., which must be
  processed sequentially.  Buffers are always direct access devices.

  The access mode of an IO device can be either \e raw or \e buffered.
  QFile objects can be created using one of these.  Raw access mode is more
  low level, while buffered access use smart buffering techniques.
  The raw access mode is best when IO is block-operated using 4kB block size
  or greater.  Buffered access works better when reading small portions of
  data at a time.

  An IO device operation can be executed in either \e synchronous or
  \e asynchronous mode.	 The IO devices currently supported by Qt only
  execute synchronously.

  The QDataStream and QTextStream provide binary and text operations
  on QIODevice objects.

  \sa QDataStream, QTextStream
 ----------------------------------------------------------------------------*/


/*****************************************************************************
  QIODevice member functions
 *****************************************************************************/


/*----------------------------------------------------------------------------
  Constructs an IO device.
 ----------------------------------------------------------------------------*/

QIODevice::QIODevice()
{
    ioMode = 0;					// initial mode
    ioSt = IO_Ok;
    index = 0;
}

/*----------------------------------------------------------------------------
  Destroys an IO device.
 ----------------------------------------------------------------------------*/

QIODevice::~QIODevice()
{
}


/*----------------------------------------------------------------------------
  \fn int QIODevice::flags() const
  Returns the current IO device flags setting.

  Flags consists of mode flags and state flags.

  \sa mode(), state()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QIODevice::mode() const
  Returns bits OR'ed together that specify the current operation mode.

  These are the flags that were given to the open() function.

  The flags are: \c IO_ReadOnly, \c IO_WriteOnly, \c IO_ReadWrite,
  \c IO_Append, \c IO_Truncate and \c IO_Translate.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QIODevice::state() const
  Returns bits OR'ed together that specify the current state.

  The flags are: \c IO_Open.

  Subclasses may define more flags.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isDirectAccess() const
  Returns TRUE if the IO device is a direct access (not sequential) device,
  otherwise FALSE.
  \sa isSequentialAccess()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isSequentialAccess() const
  Returns TRUE if the IO device is a sequential access (not direct) device,
  otherwise FALSE.
  \sa isDirectAccess()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isCombinedAccess() const
  Returns TRUE if the IO device is a combined access (both direct and
  sequential) device,  otherwise FALSE.

  This access method is currently not in use.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isBuffered() const
  Returns TRUE if the IO device is a buffered (not raw) device, otherwise
  FALSE.
  \sa isRaw()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isRaw() const
  Returns TRUE if the IO device is a raw (not buffered) device, otherwise
  FALSE.
  \sa isBuffered()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isSynchronous() const
  Returns TRUE if the IO device is a synchronous device, otherwise
  FALSE.
  \sa isAsynchronous()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isAsynchronous() const
  Returns TRUE if the IO device is a asynchronous device, otherwise
  FALSE.

  This mode is currently not in use.

  \sa isSynchronous()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isTranslated() const
  Returns TRUE if the IO device translates carriage-return and linefeed
  characters.

  A QFile is translated if it is opened with the \c IO_Translate mode
  flag.
  ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isReadable() const
  Returns TRUE if the IO device was opened using \c IO_ReadOnly or
  \c IO_ReadWrite mode.
  \sa isWritable(), isReadWrite()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isWritable() const
  Returns TRUE if the IO device was opened using \c IO_WriteOnly or
  \c IO_ReadWrite mode.
  \sa isReadable(), isReadWrite()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isReadWrite() const
  Returns TRUE if the IO device was opened using \c IO_ReadWrite mode.
  \sa isReadable(), isWritable()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isInactive() const
  Returns TRUE if the IO device state is 0, i.e. the device is not open.
  \sa isOpen()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QIODevice::isOpen() const
  Returns TRUE if the IO device state has been opened, otherwise FALSE.
  \sa isInactive()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn int QIODevice::status() const
  Returns the IO device status.

  The IO device status returns an error code.  If open() returns FALSE
  or readBlock() or writeBlock() return -1, this function can be called to
  get the reason why the operation did not succeed.

  The status codes are:
  <ul>
  <li>\c IO_Ok The operation was successful.
  <li>\c IO_ReadError Could not read from the device.
  <li>\c IO_WriteError Could not write to the device.
  <li>\c IO_FatalError A fatal unrecoverable error occurred.
  <li>\c IO_OpenError Could not open the device.
  <li>\c IO_ConnectError Could not connect to the device.
  <li>\c IO_AbortError The operation was unexpectedly aborted.
  <li>\c IO_TimeOutError The operation timed out.
  </ul>

  \sa resetStatus()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QIODevice::resetStatus()

  Sets the IO device status to \c IO_Ok.

  \sa status()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \internal
  Used by subclasses to set the device type.
 ----------------------------------------------------------------------------*/

void QIODevice::setType( int t )
{
#if defined(CHECK_RANGE)
    if ( (t & IO_TypeMask) != t )
	warning( "QIODevice::setType: Specified type out of range" );
#endif
    ioMode &= ~IO_TypeMask;			// reset type bits
    ioMode |= t;
}

/*----------------------------------------------------------------------------
  \internal
  Used by subclasses to set the device mode.
 ----------------------------------------------------------------------------*/

void QIODevice::setMode( int m )
{
#if defined(CHECK_RANGE)
    if ( (m & IO_ModeMask) != m )
	warning( "QIODevice::setMode: Specified mode out of range" );
#endif
    ioMode &= ~IO_ModeMask;			// reset mode bits
    ioMode |= m;
}

/*----------------------------------------------------------------------------
  \internal
  Used by subclasses to set the device state.
 ----------------------------------------------------------------------------*/

void QIODevice::setState( int s )
{
#if defined(CHECK_RANGE)
    if ( ((uint)s & IO_StateMask) != (uint)s )
	warning( "QIODevice::setState: Specified state out of range" );
#endif
    ioMode &= ~IO_StateMask;			// reset state bits
    ioMode |= (uint)s;
}

/*----------------------------------------------------------------------------
  \internal
  Used by subclasses to set the device status (not state).
 ----------------------------------------------------------------------------*/

void QIODevice::setStatus( int s )
{
    ioSt = s;
}


/*----------------------------------------------------------------------------
  \fn bool QIODevice::open( int mode )
  Opens the IO device using the specified \e mode.
  Returns TRUE if successful, or FALSE if the device could not be opened.

  The mode parameter \e m must be a combination of the following flags.
  <ul>
  <li>\c IO_Raw specified raw (unbuffered) file access.
  <li>\c IO_ReadOnly opens a file in read-only mode.
  <li>\c IO_WriteOnly opens a file in write-only mode.
  <li>\c IO_ReadWrite opens a file in read/write mode.
  <li>\c IO_Append sets the file index to the end of the file.
  <li>\c IO_Truncate truncates the file.
  <li>\c IO_Translate enables carriage returns and linefeed translation
  for text files under MS-DOS, Window, OS/2 and Macintosh.  Cannot be
  combined with \c IO_Raw.
  </ul>

  This virtual function must be reimplemented by subclasses.

  \sa close()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QIODevice::close()
  Closes the IO device.

  This virtual function must be reimplemented by subclasses.

  \sa open()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QIODevice::flush()

  Flushes an open IO device.

  This virtual function must be reimplemented by subclasses.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn long QIODevice::size() const
  Virtual function that returns the size of the IO device.
  \sa at()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Virtual function that returns the current IO device index.

  This index is the data read/write head of the IO device.

  \sa size()
 ----------------------------------------------------------------------------*/

long QIODevice::at() const
{
    return index;
}

/*----------------------------------------------------------------------------
  Virtual function that sets the IO device index to \e n.

  \sa size()
 ----------------------------------------------------------------------------*/

bool QIODevice::at( long n )
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

/*----------------------------------------------------------------------------
  Virtual function that returns TRUE if the IO device index is at the
  end of the input.
 ----------------------------------------------------------------------------*/

bool QIODevice::atEnd() const
{
    return at() == size();
}

/*----------------------------------------------------------------------------
  \fn bool QIODevice::reset()
  Sets the device index to 0.
  \sa at()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn int QIODev::readBlock( char *data, uint len ) = 0
  Reads at most \e len bytes from the IO device into \e data and
  returns the number of bytes actually read.

  This virtual function must be reimplemented by subclasses.

  \sa writeBlock()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QIODev::writeBlock( const char *data, uint len ) = 0
  Writes \e len bytes from \e p to the IO device and returns the number of
  bytes actually written.

  This virtual function must be reimplemented by subclasses.

  \sa readBlock()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Reads a line of text, up to \e maxlen bytes.

  This virtual function can be reimplemented by subclasses.

  \sa readBlock()
 ----------------------------------------------------------------------------*/

int QIODevice::readLine( char *data, uint maxlen )
{
    long pos = at();				// get current position
    long s  = size();				// size of IO device
    char *p = data;
    if ( pos >= s )
	return 0;
    while ( pos++ < s && --maxlen ) {		// read one byte at a time
	readBlock( p, 1 );
	if ( *p++ == '\n' )			// end of line
	    break;
    }
    *p++ = '\0';
    return (int)((long)p - (long)data);
}

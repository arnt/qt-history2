/****************************************************************************
**
** Implementation of QIOEngine class.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qioengine.h"
#include "qioengine_p.h"
#include <stdio.h>

#define d d_func()
#define q q_func()

/*!
    \class QIOEngine qioengine.h
    \reentrant

    \brief The QIOEngine class provides an abstraction for block reading and writing.

    \ingroup io
    \mainclass

    The QIOEngine class is used to directly read and write to the
    backing store for a QIODevice. All read/write operations will be
    based on simple characters, making operations completely platform
    independent.

    \omit
    QTextStream will in turn interpret the bytes the same when read.
    \endomit

*/


/*!
   Constructs a QIOEngine.
 */
QIOEngine::QIOEngine() : d_ptr(new QIOEnginePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
   \internal

   Constructs a QIOEngine.
 */
QIOEngine::QIOEngine(QIOEnginePrivate &dd) : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
  Destroys the QIOEngine.
 */
QIOEngine::~QIOEngine()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  \fn Q_LONG QIOEngine::readLine(char *data, Q_LONG maximum)

  Reads a single line (ending with \\n) from the device into \a data.
  At most, the \a maximum number of bytes will be read. If successful,
  the number of characters read from the device is returned; otherwise
  EOF is returned.

  Many QIOEngine subclasses can be optimized for this function. The
  base implementation will simply read a single character at a time.

  \sa readBlock
 */
Q_LONG QIOEngine::readLine(char *data, Q_LONG maxlen)
{
    if (maxlen == 0)                                // application bug?
        return 0;
    char *p = data;
    while (--maxlen && (readBlock(p,1)>0)) {        // read one byte at a time
        if (*p++ == '\n')                        // end of line
            break;
    }
    if(p != data) {
        *p++ = '\0';
        return p - data;
    } 
    return -1;
}

/*!
   Reads all the remaining data from the source, and returns it in a
   QByteArray. This can be optimized in many subclasses but the base
   implementation will just read a block at a time.

   \sa readBlock
 */
QByteArray QIOEngine::readAll()
{
    if (isSequential()) {     // read until we reach the end
        const int blocksize = 512;
        int nread = 0;
        QByteArray ba;
        while (!atEnd()) {
            ba.resize(nread + blocksize);
            int r = readBlock(ba.data()+nread, blocksize);
            if (r < 0) {
                ba.resize(nread);
                return ba;
            }
            nread += r;
        }
        ba.resize(nread);
        return ba;
    }
    // we know the size
    int n = size() - at(); // ### fix for 64-bit or large files?
    int totalRead = 0;
    QByteArray ba;
    ba.resize(n);
    char* c = ba.data();
    while (n) {
        int r = readBlock(c, n);
        if (r < 0)
            return QByteArray();
        n -= r;
        c += r;
        totalRead += r;
        // If we have a translated file, then it is possible that
        // we read less bytes than size() reports
        if (atEnd()) {
            ba.resize(totalRead);
            break;
        }
    }
    return ba;
}


/*!
  Reads and returns a single character from the QIODevice. If the end of
  the file is reached, EOF is returned.

  The base implementation will simply read a single character.

  \sa readBlock
 */
int QIOEngine::getch()
{
    uchar ret;
    if(readBlock((char*)&ret, 1) != 1) 
        return EOF;
    return (int)ret;
}
 
/*!
  \fn int QIOEngine::putch(int character)

  Puts a single \a character into the device, returning the value
  given if successful; otherwise EOF is returned to indicate failure.

  The base implementation will simply write a single character to
  the device.

  \sa writeBlock
 */
int QIOEngine::putch(int ch)
{
    uchar ret = ch;
    if(writeBlock((char*)&ret, 1) != 1)
        return EOF;
    return (int)ret;
}

/*!
   Returns true if the end of file has been reached; otherwise returns
   false.
 */
bool QIOEngine::atEnd() const
{
    return at() == size();
}

/*!
  Returns the QIODevice::Status that resulted from the last failed
  operation. If QIOevice::UnspecifiedError is returned, QIODevice will
  use its own idea of the error status.

  \sa QIODeivce::Status, errorString
 */
QIODevice::Status QIOEngine::errorStatus() const
{
    return QIODevice::UnspecifiedError;
}

/*!
  Returns the human-readable message appropriate to the current error
  reported by errorStatus(). If no suitable string is available, a null
  string is returned.

  \sa errorStatus(), QString::isNull()
 */
QString QIOEngine::errorString() const
{
    return QString::null;
}

/*!
  \fn uchar *QIOEngine::map(QIODevice::Offset offset, Q_LONG number)

  Maps the file contents from \a offset for the given \a number of
  bytes, returning a pointer (uchar *) to the contents. If this fails,
  0 is returned.

  The default implementation falls back to block reading/writing if
  this function returns 0.

  \sa unmap
 */
uchar *QIOEngine::map(QIODevice::Offset /*offset*/, Q_LONG /*len*/)
{
    return 0;
}

/*!
   \fn void QIOEngine::unmap(uchar *data)

   Unmap previously mapped file \a data from memory.

   \sa map
 */
void QIOEngine::unmap(uchar * /*data*/)
{

}

/*! 
    \fn QIOEngine::Type QIOEngine::type() const

    Returns the I/O type. This can be used as simple RTTI information.

    This virtual function must be reimplemented by all subclasses.

    \sa QIOEngine::Type
 */

/*!
    \fn bool QIOEngine::open(int flags)

    Opens the device for access in the way described by the \a flags given.
    Returns true if the device is opened; otherwise returns false and sets
    QIOEngine::errorStatus to an appropriate value.

    \a flags is a selection of the values defined in QIODevice::OpenModes,
    combined using the bitwise OR operator.

    This virtual function must be reimplemented by all subclasses.

    \sa errorStatus, QIODevice::OpenModes, close
 */

/*!
    \fn bool QIOEngine::close()

    Closes the device. Returns true if the device is closed successfully;
    otherwise returns false. QIOEngine::errorStatus is set if an error
    occurs.

    This virtual function must be reimplemented by all subclasses.
 */

/*!
    \fn void QIOEngine::flush()

    Flushes all pending reads and writes to the device.

    This virtual function must be reimplemented by all subclasses.
 */

/*!
    \fn QIODevice::Offset QIOEngine::size() const

    Requests the size of your IO destination.

    This virtual function must be reimplemented by all subclasses.
 */

/*!
    \fn QIODevice::Offset QIOEngine::at() const

    Returns the current position of your IO destination.

    This virtual function must be reimplemented by all subclasses.
 */

/*! 
    \fn bool QIOEngine::seek(QIODevice::Offset offset)

    Sets the current device position to the given \a offset relative to
    the beginning of the line. If the engine is a sequential device then
    \a offset will be relative to the current position.

    This virtual function must be reimplemented by all subclasses.

    \sa at, isSequential
 */

/*! 
    \fn bool QIOEngine::isSequential() const

    Returns true if the engine can be used for sequential read/write access;
    otherwise returns false.

    This virtual function must be reimplemented by all subclasses.
 */

/*! 
    \fn Q_LONG QIOEngine::readBlock(char *data, Q_LONG maximum)

    Reads a number of characters from the device into \a data. At most,
    the \a maximum number of characters will be read. If successful,
    the number of bytes read from the device is returned; otherwise EOF
    is returned.

    This virtual function must be reimplemented by all subclasses.
 */

/*! 
    \fn Q_LONG QIOEngine::writeBlock(const char *data, Q_LONG maximum)

    Writes a number of characters from \a data to the device. At most,
    the \a maximum of characters will be written. If successful,
    the number of characters written is returned; otherwise EOF is
    returned.

    This virtual function must be reimplemented by all subclasses.
 */

/*! 
    \fn int QIOEngine::ungetch(int character)

    Places the \a character back into the stream at the current
    position, returning the value given if successful; otherwise -1 is
    returned to indicate failure.

    This virtual function must be reimplemented by all subclasses.
 */

/*!
    \enum QIOEngine::Type

    This enum defines the type of device in use:

    \value File      The device represents a file.
    \value Resource  The device represents a resource.
    \value Socket    The device represents a socket.
    \value String    The device represents a string.

    User-defined devices must define types within the following range:

    \value User      First available user-defined device.
    \value MaxUser   Last available user-defined device.
*/

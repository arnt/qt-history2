/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

#include "qbytearray.h"
#include "qiodevice.h"
#include "qiodevice_p.h"
#if defined(QT_BUILD_CORE_LIB)
# include "qcoreapplication.h"
#endif

#define d d_func()
#define q q_func()

extern QString qt_errorstr(int errorCode);

QIODevicePrivate::QIODevicePrivate() : q_ptr(0), ioMode(0), ioSt(QIODevice::Ok)
{

}

QIODevicePrivate::~QIODevicePrivate()
{
}

/*!
    \class QIODevice
    \reentrant

    \brief The QIODevice class is the base class of I/O devices.

    \ingroup io

    An I/O device represents a medium that one can read bytes from
    and/or write bytes to. The QIODevice class is the abstract
    super-class of all such devices; classes such as QFile, QBuffer and
    QSocket inherit QIODevice, and implement virtual functions such as
    write() appropriately.

    Although applications sometimes use QIODevice directly, it is
    usually better to use QTextStream and QDataStream which provide
    stream operations on any QIODevice subclass. QTextStream provides
    text-oriented stream functionality (for human-readable ASCII
    files, for example), whereas QDataStream deals with binary data in
    a totally platform-independent manner.

    The public member functions in QIODevice roughly fall into two
    groups: the action functions and the state access functions. The
    most important action functions are:

    \list

    \i  open() opens a device for reading and/or writing, depending on
    the mode argument.

    \i  close() closes the device and tidies up (e.g. flushes buffered
    data).

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
    is one for this device, or it moves the pointer if given an offset.

    \i  atEnd() indicates whether there is more to read, if this is
    meaningful for this device.

    \i  reset() moves the read/write pointer to the start of the
    device, if that is possible for this device.

    \endlist

    The state access are all "get" functions. The QIODevice subclass
    calls setState() to update the state, and simple access functions
    tell the user of the device what the device's state is. Here are
    the settings, and their associated access functions:

    \list

    \i  Access type. Some devices are direct access (it is possible
    to read/write anywhere), whereas others are sequential. QIODevice
    provides the access functions (isDirectAccess(),
    isSequentialAccess(), and isCombinedAccess()) to tell users what a
    given I/O device supports.

    \i  Buffering. Some devices are accessed in raw mode, whereas
    others are buffered. Buffering usually provides greater
    efficiency, particularly for small read/write operations.
    isBuffered() tells the user whether a given device is buffered.
    (This can often be set by the application in the call to open().)

    \i  Synchronicity. Synchronous devices work immediately (for
    example, files). When you read from a file, the file delivers its
    data straight away. Other kinds of device, such as a socket
    connected to a HTTP server, may not deliver the data until seconds
    after you ask to read it. isSynchronous() and isAsynchronous()
    tell the user how this device operates.

    \i  CR/LF translation. For simplicity, applications often like to
    see just a single CR/LF style, and QIODevice subclasses can
    provide this. isTranslated() returns true if this object
    translates CR/LF to just LF. (This can often be set by the
    application in the call to open().)

    \i  Permissions. Some files cannot be written. For example,
    isReadable(), isWritable(), and isReadWrite() tell the application
    whether it can read from and write to a given device. (This can
    often be set by the application in the call to open().)

    \i  Finally, isOpen() returns true if the device is open; i.e.
    after an open() call.

    \endlist

    \sa QDataStream, QTextStream
*/

/*!
    \enum QIODevice::AccessTypes

    \internal

    \value Direct      The device supports random access.
    \value Sequential  The device must be accessed sequentially.
    \value Combined    The device supports both Direct and Sequential access.
    \omitvalue TypeMask
*/

/*!
    \enum QIODevice::HandlingModes

    \internal

    \value Raw   Unbuffered
    \value Async Asynchronous
*/

/*!
    \enum QIODevice::OpenModes

    \value ReadOnly     The device can only be read from.
    \value WriteOnly    The device can only be written to.
    \value ReadWrite    The device can be both read from and written to.
    \value Append       Data written to the device is appended to the end of
                        existing data.
    \value Truncate
    \value Translate    Translate line ending conventions.
    \omitvalue ModeMask
*/

/*!
    \enum QIODevice::State

    \internal

    \value Open
    \value StateMask
*/

/*!
    \enum QIODevice::Status

    \internal

    \value Ok
    \value ReadError
    \value WriteError
    \value FatalError
    \value ResourceError
    \value OpenError
    \value ConnectError
    \value AbortError
    \value TimeOutError
    \value UnspecifiedError

*/

/*!
    \enum QIODevice::Offset

    The offset (device position) within the device.
*/


/*!
    Constructs an I/O device.
*/

QIODevice::QIODevice()
    : d_ptr(new QIODevicePrivate)
{
    d_ptr->q_ptr = this;
}

/*! \internal
*/
QIODevice::QIODevice(QIODevicePrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys the I/O device.
*/

QIODevice::~QIODevice()
{
    delete d_ptr;
    d_ptr = 0;
}


/*!
    Returns the current I/O device flags setting.

    The flags value is a logical OR of the mode flags and state flags.

    \sa mode(), state()
*/

int QIODevice::flags() const
{
    return d->ioMode;
}

/*!
    \fn int QIODevice::mode() const

    Returns a value specifying the current operation mode. This is a
    selection of mode flags, combined using the logical OR operator.

    \sa OpenModes
*/

/*!
    \fn int QIODevice::state() const

    Returns a value specifying the current state. This is a selection
    of state values, combined using the OR operator.

    The flags are: \c QIODevice::Open.

    Subclasses may define additional flags.
*/

/*!
    \fn bool QIODevice::isDirectAccess() const

    Returns true if the I/O device is a direct access device;
    returns false if the device is a sequential access device.

    \sa isSequentialAccess()
*/

/*!
    \fn bool QIODevice::isSequentialAccess() const

    Returns true if the device is a sequential access device;
    returns false if the device is a direct access device.

    Operations involving size() and seek(int) are not valid on
    sequential devices.

    \sa isDirectAccess()
*/

/*!
    \fn bool QIODevice::isCombinedAccess() const

    Returns true if the I/O device is a combined access (both direct
    and sequential) device; otherwise returns false.

    This access method is currently not in use.
*/

/*!
    \fn bool QIODevice::isBuffered() const

    Returns true if the I/O device is a buffered device; returns false if
    the device is a raw device.

    \sa isRaw()
*/

/*!
    \fn bool QIODevice::isRaw() const

    Returns true if the device is a raw device; otherwise returns
    false if the device is a buffered device.

    \sa isBuffered()
*/

/*!
    \fn bool QIODevice::isSynchronous() const

    Returns true if the I/O device is a synchronous device; otherwise
    returns false.

    \sa isAsynchronous()
*/

/*!
    \fn bool QIODevice::isAsynchronous() const

    Returns true if the device is an asynchronous device; returns false if
    the device is a synchronous device.

    This mode is currently not in use.

    \sa isSynchronous()
*/

/*!
    \fn bool QIODevice::isTranslated() const

    Returns true if the I/O device translates carriage return and
    linefeed characters; otherwise returns false.

    A QFile is translated if it is opened with the \c QIODevice::Translate
    mode flag.
*/

/*!
    \fn bool QIODevice::isReadable() const

    Returns true if the I/O device was opened in \c QIODevice::ReadOnly
    or \c QIODevice::ReadWrite mode; otherwise returns false.

    \sa isWritable(), isReadWrite()
*/

/*!
    \fn bool QIODevice::isWritable() const

    Returns true if the I/O device was opened in \c QIODevice::WriteOnly
    or \c QIODevice::ReadWrite mode; otherwise returns false.

    \sa isReadable(), isReadWrite()
*/

/*!
    \fn bool QIODevice::isReadWrite() const

    Returns true if the I/O device was opened in \c QIODevice::ReadWrite
    mode; otherwise returns false.

    \sa isReadable(), isWritable()
*/

/*!
    \fn bool QIODevice::isInactive() const

    Returns true if the I/O device state is 0, i.e. the device is not
    open; otherwise returns false.

    \sa isOpen()
*/

/*!
    \fn bool QIODevice::isOpen() const

    Returns true if the I/O device has been opened; otherwise returns
    false.

    \sa isInactive()
*/


/*!
    Returns the I/O device status.

    \keyword QIODevice::Ok
    \keyword QIODevice::ReadError
    \keyword QIODevice::WriteError
    \keyword QIODevice::FatalError
    \keyword QIODevice::OpenError
    \keyword QIODevice::ConnectError
    \keyword QIODevice::AbortError
    \keyword QIODevice::TimeOutError
    \keyword QIODevice::UnspecifiedError

    The I/O device status returns an error code. For example, if open()
    returns false, or a read/write operation returns -1, this function can
    be called to find out the reason why the operation failed.

    The status codes are:
    \table
    \header \i Status code \i Meaning
    \row \i \c QIODevice::Ok \i The operation was successful.
    \row \i \c QIODevice::ReadError \i Could not read from the device.
    \row \i \c QIODevice::WriteError \i Could not write to the device.
    \row \i \c QIODevice::FatalError \i A fatal unrecoverable error occurred.
    \row \i \c QIODevice::OpenError \i Could not open the device.
    \row \i \c QIODevice::ConnectError \i Could not connect to the device.
    \row \i \c QIODevice::AbortError \i The operation was unexpectedly aborted.
    \row \i \c QIODevice::TimeOutError \i The operation timed out.
    \row \i \c QIODevice::UnspecifiedError \i An unspecified error happened on close.
    \endtable

    \sa resetStatus()
*/

int QIODevice::status() const
{
    return d->ioSt;
}

/*!
    Sets the I/O device status to \c QIODevice::Ok.

    \sa status()
*/
void QIODevice::resetStatus()
{
    d->ioSt = Ok;
    d->errStr.clear();
}

/*!
    \fn void QIODevice::setFlags(int flags)

    Used by subclasses to set the device \a flags.

    \sa OpenModes
*/
void QIODevice::setFlags(int f)
{
    d->ioMode = f;
}

/*!
  \internal
  Used by subclasses to set the device type.
*/

void QIODevice::setType(int t)
{
    if ((t & QIODevice::TypeMask) != t)
        qWarning("QIODevice::setType: Specified type out of range");
    d->ioMode &= ~QIODevice::TypeMask;                        // reset type bits
    d->ioMode |= t;
}

/*!
  \internal
  Used by subclasses to set the device mode.
*/

void QIODevice::setMode(int m)
{
    if ((m & QIODevice::ModeMask) != m)
        qWarning("QIODevice::setMode: Specified mode out of range");
    d->ioMode &= ~QIODevice::ModeMask;                        // reset mode bits
    d->ioMode |= m;
}

/*!
  \internal
  Used by subclasses to set the device state.
*/

void QIODevice::setState(int s)
{
    if (((uint)s & QIODevice::StateMask) != (uint)s)
        qWarning("QIODevice::setState: Specified state out of range");
    d->ioMode &= ~QIODevice::StateMask;                        // reset state bits
    d->ioMode |= (uint)s;
}

/*!
    \internal
    Used by subclasses to set the device status (not state) to \a status.
*/

void QIODevice::setStatus(int status)
{
    d->ioSt = status;
    d->errStr.clear();
}

/*!
    \internal
*/
void QIODevice::setStatus(int status, const QString &errorString)
{
    d->ioSt = status;
    d->errStr = errorString;
}

/*!
    \internal
*/
void QIODevice::setStatus(int status, int errNum)
{
    d->ioSt = status;
    d->errStr = qt_errorstr(errNum);
}

/*!
    \fn void QIODevice::close()

    Closes the I/O device.

    This virtual function must be reimplemented by all subclasses.

    \sa open()
*/

/*!
    \fn bool QIODevice::open(int mode)

    Opens the I/O device in the specified \a mode. Returns true if
    the device was successfully opened; otherwise returns false.

    The mode parameter \a mode must be selection of the following flags,
    combined using the OR operator:
    \table
    \header \i Mode flags \i Meaning
    \row \i \c QIODevice::Raw \i specifies raw (unbuffered) file access.
    \row \i \c QIODevice::ReadOnly \i opens a file in read-only mode.
    \row \i \c QIODevice::WriteOnly \i opens a file in write-only mode.
    \row \i \c QIODevice::ReadWrite \i opens a file in read/write mode.
    \row \i \c QIODevice::Append \i sets the file index to the end of the file.
    \row \i \c QIODevice::Truncate \i truncates the file.
    \row \i \c QIODevice::Translate \i enables carriage returns and linefeed
    translation for text files under MS-DOS, Windows, and Mac OS X. On
    Unix systems this flag has no effect. Use with caution as it will
    also transform every linefeed written to the file into a CRLF
    pair. This is likely to corrupt your file if you write write
    binary data. Cannot be combined with \c QIODevice::Raw.
    \endtable

    Some devices make less sense to use this entry point to open, for
    example connecting to a server in QSocketDevice. In this case the
    default implementation will return false indicating that the open
    failed and another approach to put the file into isOpen() mode
    should be attempted.
*/

/*!
    \fn void QIODevice::flush()

    Flushes the open I/O device.
*/

/*!
    \fn Q_LLONG QIODevice::size() const

    Returns the size of the I/O device.

    This virtual function must be reimplemented by all subclasses.

    \sa at()
*/

/*!
     \fn Q_LLONG QIODevice::at() const

    Returns the current I/O device position.

    This is the position of the data read/write head of the I/O
    device.

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/

/*!
    \fn bool QIODevice::seek(Q_LLONG offset)

    Sets the I/O device position to the \a offset given. Returns true if the
    position was successfully set (the \a offset is within range and the
    seek was successful); otherwise returns false.

    If the device is sequential, the \a offset is relative to the current
    position.

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/

/*!
    \fn bool QIODevice::reset()

    Sets the device position to 0.

    \sa at() seek()
*/

/*!
    \fn Q_LLONG QIODevice::read(char *data, Q_LLONG len)

    Reads a number of characters from the I/O device into \a data. At most,
    the \a maximum number of characters will be read.

    Returns -1 if a fatal error occurs, or 0 if there are no bytes to read.

    The device must be opened for reading, and \a data must not be 0.

    This virtual function must be reimplemented by all subclasses.

    \sa writeBlock() isOpen() isReadable()

*/

/*!
     \fn QByteArray QIODevice::read(Q_LLONG maxlen)
    \overload

    This convenience function will read data into a preallocated QByteArray.
*/

/*!
    This convenience function returns all of the remaining data in the
    device.
*/
QByteArray QIODevice::readAll()
{
    if (!isOpen()) {
        qWarning("QIODevice::readAll: File not open");
        return QByteArray();
    }
    if (!isReadable()) {
        qWarning("QIODevice::readAll: Read operation not permitted");
        return QByteArray();
    }

    const int chunk_size = 1024;
    QByteArray ret;
    //from device
    while(1) {
        int oldpos = ret.size();
        ret.resize(oldpos+chunk_size);
        int got = read(ret.data()+oldpos, chunk_size);
        if(got == -1) {
            ret.resize(oldpos);
            break;
        }
        if(got < chunk_size)
            ret.resize(oldpos + got);
    }
    return ret;
}

/*!
    \fn Q_LONG QIODevice::write(const char *data, Q_LLONG len)

    Writes a number of characters from \a data to the I/O device. At most,
    the \a maximum of characters will be written. If successful,
    the number of characters written is returned; otherwise EOF is
    returned.

    This virtual function must be reimplemented by all subclasses.

    \sa read()
*/

/*!
    \fn Q_LONG QIODevice::write(const QByteArray &data)
    \overload

    This convenience function is the same as:

    \code
        write(data.data(), data.size());
    \endcode
*/

/*!
    Reads a single line (ending with \\n) from the device into \a data.
    At most, the \a maximum number of bytes will be read. If there is a
    newline at the end if the line, it is not stripped. A terminating
    '\0' is appended to the characters read into \a data.

    Returns the number of bytes read, including the terminating '\0';
    returns -1 if an error occurred.

    To ensure that your \a data buffer is large enough to contain the
    \a maximum number of characters read from the device, pass a value
    that is one character less than the size of your buffer.

    For example, a buffer that is 1024 characters in length will be
    filled if the following call returns the maximum number of characters
    specified:

    \code
        device->readLine(data, 1023);
    \endcode

    \sa read(), QTextStream::readLine()
*/

Q_LLONG 
QIODevice::readLine(char *data, Q_LLONG maxlen)
{
    if (maxlen <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QIODevice::readLine: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QIODevice::readLine: Read operation not permitted");
        return -1;
    }
 
    char *p = data;
    while (--maxlen && (*p=getch()) > 0) {        // read one byte at a time
        if (*p++ == '\n')                    // end of line
            break;
    }
    if(p != data) {
        *p++ = '\0';
        return p - data;
    } 
    return -1;
}

/*!
  \overload
  
  This convenience function will read a line and place it into a QByteArray.

  \sa QIODevice::readLine()
*/

QByteArray
QIODevice::readLine()
{
    if (!isOpen()) {
        qWarning("QIODevice::readLine: File not open");
        return QByteArray();
    }
    if (!isReadable()) {
        qWarning("QIODevice::readLine: Read operation not permitted");
        return QByteArray();
    }
 
    int used = 0;
    QByteArray ret;
    for(char tmp, *p=0; (tmp = getch()) > 0 && tmp != '\n'; used++) {
        if(!used || used <= ret.size()) {
            ret.resize(used+1024);
            p = ret.data();
        }
        *(p+used) = tmp;
    }
    ret.resize(used);
    return ret;
}


/*!
    Reads a single character from the I/O device.

    Returns the character read, or -1 if the end of the I/O
    device has been reached.

    \sa putch(), ungetch()
*/

int
QIODevice::getch()
{
    if (!isOpen()) {                                // file not open
        qWarning("QIODevice::getch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QIODevice::getch: Read operation not permitted");
        return EOF;
    }
    uchar ch;
    Q_LLONG got = read((char*)&ch, 1);
    if(got <= 0)
        return -1;
    return (int)ch;
}


/*!
    Writes the \a character to the I/O device.

    Returns the \a character, or -1 if an error occurred.

    \sa getch()
*/

int
QIODevice::putch(int character)
{
    if (!isOpen()) {                                // file not open
        qWarning("QIODevice::putch: File not open");
        return EOF;
    }
    if (!isWritable()) {                        // write not permitted
        qWarning("QIODevice::putch: Write operation not permitted");
        return EOF;
    }
    uchar ch = (char)character;
    Q_LLONG got = write((char*)&ch, 1);
    if(got <= 0)
        return -1;
    return ch;
}

/*!
    \fn int QIODevice::ungetch(int character)

    Puts the \a character back into the I/O device, and decrements
    the index position if it is not zero.

    This function is normally called to "undo" a getch() operation.

    Returns \a character, or -1 if an error occurred.

    This virtual function must be reimplemented by all subclasses.

    \sa getch(), putch()
*/

/*!
    Returns a human-readable description of an error that occurred on
    the device. The error described by the string corresponds to
    changes of QIODevice::status(). If the status is reset, the error
    string is also reset.

    \code
        QFile file("address.dat");
        if (!file.open(QIODevice::ReadOnly) {
            QMessageBox::critical(this, tr("Error"),
                    tr("Could not open file for reading: %1")
                    .arg(file.errorString()));
            return;
        }
    \endcode

    \sa resetStatus()
*/

QString QIODevice::errorString() const
{
    if (d->errStr.isEmpty()) {
        const char *str = 0;
        switch (d->ioSt) {
        case Ok:
        case UnspecifiedError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Unknown error");
            break;
        case ReadError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Could not read from the device");
            break;
        case WriteError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Could not write to the device");
            break;
        case FatalError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Fatal error");
            break;
        case ResourceError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Resource error");
            break;
        case OpenError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Could not open the device");
            break;
        case ConnectError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Could not connect to host");
            break;
        case AbortError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Aborted");
            break;
        case TimeOutError:
            str = QT_TRANSLATE_NOOP("QIODevice", "Connection timed out");
        }
#if defined(QT_BUILD_CORE_LIB)
        return QCoreApplication::translate("QIODevice", str);
#else
        return QString::fromLatin1(str);
#endif
    }
    return d->errStr;
}


/*!
    \fn bool QIODevice::atEnd() const

    Returns true if the I/O device position is at the end of the input;
    otherwise returns false.

    \sa at() size()
*/



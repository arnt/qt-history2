/****************************************************************************
**
** Implementation of QIODevice class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

#include "qbytearray.h"
#include "qiodevice.h"
#include "qiodevice_p.h"

#define d d_func()
#define q q_func()

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
    superclass of all such devices; classes such as QFile, QBuffer and
    QSocket inherit QIODevice and implement virtual functions such as
    write() appropriately.

    Although applications sometimes use QIODevice directly, it is
    usually better to use QTextStream and QDataStream, which provide
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
    data)

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
    isReadable(), isWritable() and isReadWrite() tell the application
    whether it can read from and write to a given device. (This can
    often be set by the application in the call to open().)

    \i  Finally, isOpen() returns true if the device is open, i.e.
    after an open() call.

    \endlist

    QIODevice provides a QIOEngine which has the responsibility of
    doing actual read, writes, file information. 

    \sa QDataStream, QTextStream, QIOEngine
*/

/*!
    \enum QIODevice::AccessTypes

    \internal

    \value Direct
    \value Sequential
    \value Combined (Direct and Sequential)
    \value TypeMask (internal)
*/

/*!
    \enum QIODevice::HandlingModes

    \internal

    \value Raw unbuffered
    \value Async asynchronous
*/

/*!
    \enum QIODevice::OpenModes

    \value ReadOnly
    \value WriteOnly
    \value ReadWrite
    \value Append
    \value Truncate
    \value Translate translate line ending conventions
    \value ModeMask (internal)
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

    The offset within the device.
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

    Flags consists of mode flags and state flags.

    \sa mode(), state()
*/

int QIODevice::flags() const
{
    return d->ioMode;
}

/*!
    \fn int QIODevice::mode() const

    Returns bits OR'ed together that specify the current operation
    mode.

    These are the flags that were given to the open() function.

    The flags are \c QIODevice::ReadOnly, \c QIODevice::WriteOnly, \c QIODevice::ReadWrite,
    \c QIODevice::Append, \c QIODevice::Truncate and \c QIODevice::Translate.
*/

/*!
    \fn int QIODevice::state() const

    Returns bits OR'ed together that specify the current state.

    The flags are: \c QIODevice::Open.

    Subclasses may define additional flags.
*/

/*!
    \fn bool QIODevice::isDirectAccess() const

    Returns true if the I/O device is a direct access device;
    otherwise returns false, i.e. if the device is a sequential access
    device.

    \sa isSequentialAccess()
*/

/*!
    \fn bool QIODevice::isSequentialAccess() const

    Returns true if the device is a sequential access device;
    otherwise returns false, i.e. if the device is a direct access
    device.

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

    Returns true if the I/O device is a buffered device; otherwise
    returns false, i.e. the device is a raw device.

    \sa isRaw()
*/

/*!
    \fn bool QIODevice::isRaw() const

    Returns true if the device is a raw device; otherwise returns
    false, i.e. if the device is a buffered device.

    \sa isBuffered()
*/

/*!
    \fn bool QIODevice::isSynchronous() const

    Returns true if the I/O device is a synchronous device; otherwise
    returns false, i.e. the device is an asynchronous device.

    \sa isAsynchronous()
*/

/*!
    \fn bool QIODevice::isAsynchronous() const

    Returns true if the device is an asynchronous device; otherwise
    returns false, i.e. if the device is a synchronous device.

    This mode is currently not in use.

    \sa isSynchronous()
*/

/*!
    \fn bool QIODevice::isTranslated() const

    Returns true if the I/O device translates carriage-return and
    linefeed characters; otherwise returns false.

    A QFile is translated if it is opened with the \c QIODevice::Translate
    mode flag.
*/

/*!
    \fn bool QIODevice::isReadable() const

    Returns true if the I/O device was opened using \c QIODevice::ReadOnly or
    \c QIODevice::ReadWrite mode; otherwise returns false.

    \sa isWritable(), isReadWrite()
*/

/*!
    \fn bool QIODevice::isWritable() const

    Returns true if the I/O device was opened using \c QIODevice::WriteOnly or
    \c QIODevice::ReadWrite mode; otherwise returns false.

    \sa isReadable(), isReadWrite()
*/

/*!
    \fn bool QIODevice::isReadWrite() const

    Returns true if the I/O device was opened using \c QIODevice::ReadWrite
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

    The I/O device status returns an error code. If open() returns
    false or readBlock() or writeBlock() return -1, this function can
    be called to find out the reason why the operation failed.

    \keyword QIODevice::Ok
    \keyword QIODevice::ReadError
    \keyword QIODevice::WriteError
    \keyword QIODevice::FatalError
    \keyword QIODevice::OpenError
    \keyword QIODevice::ConnectError
    \keyword QIODevice::AbortError
    \keyword QIODevice::TimeOutError
    \keyword QIODevice::UnspecifiedError

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
    Used by subclasses to set the device flags to \a f; see \c
    OpenModes.
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
    d->errStr = QT_TRANSLATE_NOOP("QIODevice", qt_errorstr(errNum));
}

/*!
    Opens the I/O device using the specified \a mode. Returns true if
    the device was successfully opened; otherwise returns false.

    The mode parameter \a mode must be an OR'ed combination of the
    following flags.
    \table
    \header \i Mode flags \i Meaning
    \row \i \c QIODevice::Raw \i specifies raw (unbuffered) file access.
    \row \i \c QIODevice::ReadOnly \i opens a file in read-only mode.
    \row \i \c QIODevice::WriteOnly \i opens a file in write-only mode.
    \row \i \c QIODevice::ReadWrite \i opens a file in read/write mode.
    \row \i \c QIODevice::Append \i sets the file index to the end of the file.
    \row \i \c QIODevice::Truncate \i truncates the file.
    \row \i \c QIODevice::Translate \i enables carriage returns and linefeed
    translation for text files under MS-DOS, Windows and Macintosh. On
    Unix systems this flag has no effect. Use with caution as it will
    also transform every linefeed written to the file into a CRLF
    pair. This is likely to corrupt your file if you write write
    binary data. Cannot be combined with \c QIODevice::Raw.
    \endtable

    \sa close()
*/

bool
QIODevice::open(int mode)
{
    if (isOpen()) {
        qWarning("QIODevice::open: File already open");
        return false;
    }
    if(mode & Append) //append implies write
        mode |= WriteOnly;
    setFlags(QIODevice::Direct);
    resetStatus();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QIODevice::open: File access not specified");
        return false;
    }
    if(ioEngine()->open(flags())) {
        setState(QIODevice::Open);
        if(ioEngine()->isSequential())
            setType(Sequential);
        return true;
    }
    QIODevice::Status error = ioEngine()->errorStatus();
    if(error == QIODevice::UnspecifiedError)
        error = QIODevice::OpenError;
    setStatus(error, ioEngine()->errorMessage());
    return false;
}


/*!
    Closes the I/O device.

    \sa open()
*/

void
QIODevice::close()
{
    if(!isOpen())
        return;

    bool closed = true;
    if(ioEngine() && !ioEngine()->close()) {
        closed = false;
        QIODevice::Status error = ioEngine()->errorStatus();
        if(error == QIODevice::UnspecifiedError)
            error = QIODevice::UnspecifiedError;
        setStatus(error, ioEngine()->errorMessage());
    }
    if(closed) {
        setFlags(QIODevice::Direct);
        resetStatus();
    }
}


/*!
    Flushes an open I/O device.
*/
void
QIODevice::flush()
{
    if(isOpen() && ioEngine())
        ioEngine()->flush();
}


/*!
    Returns the size of the I/O device.

    \sa at()
*/
QIODevice::Offset
QIODevice::size() const
{
    if(!ioEngine())
        return 0;
    return ioEngine()->size();
}

/*!
    Returns the current I/O device position.

    This is the position of the data read/write head of the I/O
    device.

    \sa size()
*/

QIODevice::Offset
QIODevice::at() const
{
    if (!isOpen())
        return 0;
    return ioEngine()->at();
}

/*!
    Sets the I/O device position to \a pos.  Returns true if the
    position was successfully set, i.e. \a pos is within range and the
    seek was successful; otherwise returns false.

    If the device is sequential, \a offset is treated as relative to
    the current position.

    \sa size()
*/

bool
QIODevice::seek(QIODevice::Offset offset)
{
    if (!isOpen() || !ioEngine()) {
        qWarning("QIODevice::seek: IODevice is not open");
        return false;
    }
    if(ioEngine()->seek(offset)) {
        resetStatus();
        return true;
    }
    QIODevice::Status error = ioEngine()->errorStatus();
    if(error == QIODevice::UnspecifiedError)
        error = QIODevice::PositionError;
    setStatus(error, ioEngine()->errorMessage());
    return false;
}

/*!
    Returns true if the I/O device position is
    at the end of the input; otherwise returns false.

    \sa at() size()
*/

bool
QIODevice::atEnd() const
{
    if (!isOpen()) {
        qWarning("QIODevice::atEnd: File is not open");
        return false;
    }
    return ioEngine()->atEnd();
}

/*!
    \fn bool QIODevice::reset()

    Sets the device index position to 0.

    \sa at() seek()
*/


/*!
    \fn Q_LONG QIODevice::readBlock(char *data, Q_LONG maxlen)

    Reads at most \a maxlen bytes from the I/O device into \a data and
    returns the number of bytes actually read.

    This function should return -1 if a fatal error occurs and should
    return 0 if there are no bytes to read.

    The device must be opened for reading, and \a data must not be 0.

    This virtual function must be reimplemented by all subclasses.

    \sa writeBlock() isOpen() isReadable()
*/

Q_LONG
QIODevice::readBlock(char *data, Q_LONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QIODevice::readBlock: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QIODevice::readBlock: Read operation not permitted");
        return -1;
    }

    Q_LONG ret = ioEngine()->readBlock(data, len);
    if (len && ret <= 0) {
        ret = 0;
        QIODevice::Status error = ioEngine()->errorStatus();
        if(error == QIODevice::UnspecifiedError)
            error = QIODevice::ReadError;
        setStatus(error, ioEngine()->errorMessage());
    }
    return ret;
}

/*!
    This convenience function returns all of the remaining data in the
    device.
*/
QByteArray QIODevice::readAll()
{
    if (isDirectAccess()) {
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
    } else {
        // read until we reach the end
        const int blocksize = 512;
        int nread = 0;
        QByteArray ba;
        while (!atEnd()) {
            ba.resize(nread + blocksize);
            int r = readBlock(ba.data()+nread, blocksize);
            if (r < 0)
                return QByteArray();
            nread += r;
        }
        ba.resize(nread);
        return ba;
    }
}

/*!
    Writes \a len bytes from \a data to the I/O device and returns the
    number of bytes actually written.

    This function should return -1 if a fatal error occurs.

    \sa readBlock()
*/

Q_LONG
QIODevice::writeBlock(const char *data, Q_LONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {                                // file not open
        qWarning("QIODevice::writeBlock: File not open");
        return -1;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QIODevice::writeBlock: Write operation not permitted");
        return -1;
    }

    resetStatus();
    Q_LONG ret = ioEngine()->writeBlock(data, len);
    if (ret != len) { // write error
        QIODevice::Status error = ioEngine()->errorStatus();
        if(error == QIODevice::UnspecifiedError)
            error = QIODevice::WriteError;
        setStatus(error, ioEngine()->errorMessage());
    }
    return ret;
}

/*!
    \fn Q_LONG QIODevice::writeBlock(const QByteArray& data)
    \overload

    This convenience function is the same as calling writeBlock(
    data.data(), data.size()).
*/

/*!
    Reads a line of text, (or up to \a maxlen bytes if a newline isn't
    encountered) plus a terminating '\0' into \a data. If there is a
    newline at the end if the line, it is not stripped.

    Returns the number of bytes read including the terminating '\0',
    or -1 if an error occurred.

    This virtual function can be reimplemented much more efficiently
    by the most subclasses.

    \sa readBlock(), QTextStream::readLine()
*/

Q_LONG QIODevice::readLine(char *data, Q_LONG maxlen)
{
    if (maxlen <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QIODevice::readBlock: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QIODevice::readBlock: Read operation not permitted");
        return -1;
    }
    return ioEngine()->readLine(data, maxlen);
}


/*!
    Reads a single byte/character from the I/O device.

    Returns the byte/character read, or -1 if the end of the I/O
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
    return ioEngine()->getch();
}


/*!
    Writes the character \a ch to the I/O device.

    Returns \a ch, or -1 if an error occurred.

    \sa getch(), ungetch()
*/

int
QIODevice::putch(int ch)
{
    if (!isOpen()) {                                // file not open
        qWarning("QIODevice::getch: File not open");
        return EOF;
    }
    if (!isWritable()) {                        // write not permitted
        qWarning("QIODevice::putch: Write operation not permitted");
        return EOF;
    }
    return ioEngine()->putch(ch);
}


/*!
    Puts the character \a ch back into the I/O device and decrements
    the index position if it is not zero.

    This function is normally called to "undo" a getch() operation.

    Returns \a ch, or -1 if an error occurred.

    \sa getch(), putch()
*/

int
QIODevice::ungetch(int ch)
{
    if (!isOpen()) {                                // file not open
        qWarning("QIODevice::ungetch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QIODevice::ungetch: Read operation not permitted");
        return EOF;
    }
    if (ch == EOF)                                // cannot unget EOF
        return ch;
    return ioEngine()->ungetch(ch);
}


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
            str = QT_TR_NOOP("Unknown error");
            break;
        case ReadError:
            str = QT_TR_NOOP("Could not read from the device");
            break;
        case WriteError:
            str = QT_TR_NOOP("Could not write to the device");
            break;
        case FatalError:
            str = QT_TR_NOOP("Fatal error");
            break;
        case ResourceError:
            str = QT_TR_NOOP("Resource error");
            break;
        case OpenError:
            str = QT_TR_NOOP("Could not open the device");
            break;
        case ConnectError:
            str = QT_TR_NOOP("Could not connect to host");
            break;
        case AbortError:
            str = QT_TR_NOOP("Aborted");
            break;
        case TimeOutError:
            str = QT_TR_NOOP("Connection timed out");
        }
        return QT_TRANSLATE_NOOP("QIODevice", str);
    }
    return d->errStr;
}

/*!
    \fn QIOEngine *QIODevice::ioEngine() const

    ####### SDM
*/

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

#include "qbytearray.h"
#include "qiodevice_p.h"
#include "qfile.h"

#define d d_func()
#define q q_func()

#define CHECK_OPEN(function, returnType) \
    do { \
        if (!isOpen()) { \
            return returnType; \
        } \
    } while (0)

#define CHECK_MAXLEN(function, returnType) \
    do { \
        if (maxlen < 0) { \
            qWarning("QIODevice::"#function" called with maxlen < 0"); \
            return returnType; \
        } \
    } while (0)

#define CHECK_WRITABLE(function) \
   do { \
       if ((d->deviceMode & WriteOnly) == 0) { \
           qWarning("QIODevice::"#function" called on a ReadOnly device"); \
           return Q_LONGLONG(-1); \
       } \
   } while (0)

#define CHECK_READABLE(function) \
   do { \
       if ((d->deviceMode & ReadOnly) == 0) { \
           qWarning("QIODevice::"#function" called on a WriteOnly device"); \
           return Q_LONGLONG(-1); \
       } \
   } while (0)

QIODevicePrivate::QIODevicePrivate()
{
    deviceMode = QIODevice::NotOpen;
    errorString = QT_TRANSLATE_NOOP(QIODevice, "Unknown error");
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

    \i  read() reads a block of data from the device.

    \i  write() writes a block of data to the device.

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
    \enum QIODevice::AccessType

    \internal

    \value Direct      The device supports random access.
    \value Sequential  The device must be accessed sequentially.
    \value Combined    The device supports both Direct and Sequential access.
    \omitvalue TypeMask
*/

/*!
    \enum QIODevice::HandlingMode

    \internal

    \value Raw   Unbuffered
    \value Async Asynchronous
*/

/*!
    \enum QIODevice::OpenMode

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
    \compat
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
    \enum Q_LONGLONG

    The offset (device position) within the device.
*/


/*!
    Constructs an I/O device.
*/

#ifdef QT_NO_QOBJECT
QIODevice::QIODevice()
    : d_ptr(new QIODevicePrivate)
{
    d->q_ptr = this;
}

/*! \internal
*/
QIODevice::QIODevice(QIODevicePrivate &dd)
    : d_ptr(&dd)
{
    d->q_ptr = this;
}
#else

/*!
    Constructs an I/O device.
*/

QIODevice::QIODevice()
    : QObject(*new QIODevicePrivate, 0)
{
}

/*!
    Constructs an I/O device.
*/

QIODevice::QIODevice(QObject *parent)
    : QObject(*new QIODevicePrivate, parent)
{
}

/*! \internal
*/
QIODevice::QIODevice(QIODevicePrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
}
#endif


/*!
    Destroys the I/O device.
*/

QIODevice::~QIODevice()
{
}

bool QIODevice::isSequential() const
{
    return false;
}

QIODevice::DeviceMode QIODevice::deviceMode() const
{
    return d->deviceMode;
}

void QIODevice::setDeviceMode(DeviceMode deviceMode)
{
    d->deviceMode = deviceMode;
}

bool QIODevice::isOpen() const
{
    return d->deviceMode != NotOpen;
}

bool QIODevice::isReadable() const
{
    return (deviceMode() & ReadOnly) != 0;
}

bool QIODevice::isWritable() const
{
    return (deviceMode() & WriteOnly) != 0;
}

bool QIODevice::open(DeviceMode mode)
{
    d->deviceMode = mode;
    return true;
}

void QIODevice::close()
{
    d->deviceMode = NotOpen;
#ifdef QT_NO_QOBJECT
    d->errorString = QT_TRANSLATE_NOOP(QIODevice, "Unknown error");
#else
    d->errorString = tr("Unknown error");
#endif
}

bool QIODevice::flush()
{
    return true;
}

Q_LONGLONG QIODevice::pos() const
{
    return Q_LONGLONG(0);
}

Q_LONGLONG QIODevice::size() const
{
    return Q_LONGLONG(0);
}

bool QIODevice::seek(Q_LONGLONG offset)
{
    if (offset > 0)
        read(offset);
    return true;
}

bool QIODevice::atEnd() const
{
    return pos() == size();
}

bool QIODevice::reset()
{
    return seek(0);
}

Q_LONGLONG QIODevice::bytesAvailable() const
{
    return size() - pos();
}

Q_LONGLONG QIODevice::bytesToWrite() const
{
    return Q_LONGLONG(0);
}

Q_LONGLONG QIODevice::read(char *data, Q_LONGLONG maxlen)
{
    CHECK_OPEN(read, Q_LONGLONG(-1));
    CHECK_READABLE(read);
    CHECK_MAXLEN(read, Q_LONGLONG(-1));
    Q_LONGLONG readSoFar = Q_LONGLONG(0);

    int ungetSize = d->ungetBuffer.size();
    while (ungetSize > 0) {
        if (readSoFar + 1 > maxlen)
            return readSoFar;
        data[readSoFar++] = d->ungetBuffer[ungetSize-- - 1];
    }
    d->ungetBuffer.resize(d->ungetBuffer.size() - readSoFar);

    Q_LONGLONG ret = readData(data + readSoFar, maxlen - readSoFar);
    if (ret == -1)
        return readSoFar ? readSoFar : -1;
    return ret + readSoFar;
}

QByteArray QIODevice::read(Q_LONGLONG maxlen)
{
    CHECK_MAXLEN(read, QByteArray());
    QByteArray tmp;
    Q_LONGLONG readSoFar = 0;
    char buffer[4096];

    do {
        Q_LONGLONG bytesToRead = qMin(int(maxlen - readSoFar), int(sizeof(buffer)));
        Q_LONGLONG readBytes = read(buffer, bytesToRead);
        if (readBytes <= 0)
            break;
        tmp += QByteArray(buffer, (int) readBytes);
    } while (readSoFar < maxlen && bytesAvailable() > 0);

    return tmp;
}

QByteArray QIODevice::readAll()
{
    return read(bytesAvailable());
}

Q_LONGLONG QIODevice::readLine(char *data, Q_LONGLONG maxlen)
{
    if (maxlen < 1) {
        qWarning("QIODevice::readLine() called with maxlen < 1");
        return Q_LONGLONG(-1);
    }

    Q_LONGLONG readSoFar = 0;
    char c;
    bool lastGetSucceeded = false;
    while (readSoFar + 1 < maxlen && (lastGetSucceeded = getChar(&c))) {
        *data++ = c;
        ++readSoFar;
        if (c == '\n')
            break;
    }

    if (readSoFar < maxlen)
        *data = '\0';

    if (!lastGetSucceeded && readSoFar == 0)
        return Q_LONGLONG(-1);
    return readSoFar;
}

QByteArray QIODevice::readLine(Q_LONGLONG maxlen)
{
    CHECK_MAXLEN(readLine, QByteArray());
    QByteArray tmp;
    char buffer[4096];
    Q_LONGLONG readSoFar = 0;
    Q_LONGLONG readBytes = 0;

    do {
        if (maxlen != 0)
            tmp.resize(readSoFar + qMin(int(maxlen), int(sizeof(buffer))));
        else
            tmp.resize(readSoFar + int(sizeof(buffer)));
        readBytes = readLine(tmp.data() + readSoFar, tmp.size());
        readSoFar += readBytes;
    } while (readSoFar < maxlen && readBytes > 0
             && readBytes == tmp.size() && tmp.at(readBytes - 1) != '\n');

    tmp.resize(readSoFar);
    return tmp;
}

bool QIODevice::getChar(char *c)
{
    char tmp;
    if (readData(&tmp, 1) != 1)
        return false;
    if (c)
        *c = tmp;
    return true;
}

Q_LONGLONG QIODevice::write(const char *data, Q_LONGLONG maxlen)
{
    CHECK_OPEN(write, Q_LONGLONG(-1));
    CHECK_WRITABLE(write);
    CHECK_MAXLEN(write, Q_LONGLONG(-1));
    return writeData(data, maxlen);
}

Q_LONGLONG QIODevice::write(const QByteArray &byteArray)
{
    return write(byteArray.constData(), byteArray.size());
}

bool QIODevice::putChar(char c)
{
    CHECK_WRITABLE(putChar);
    return writeData(&c, 1) == 1;
}

void QIODevice::ungetChar(char c)
{
    d->ungetBuffer.append(c);
}

void QIODevice::setErrorString(const QString &str)
{
    d->errorString = str;
}

QString QIODevice::errorString() const
{
    return d->errorString;
}

#if defined QT_COMPAT
int QIODevice::status() const
{
    const QFile *f = qt_cast<const QFile *>(this);
    if (f) return (int) f->error();
    return isOpen() ? IO_Ok : IO_UnspecifiedError;
}

void QIODevice::resetStatus()
{
    QFile *f = qt_cast<QFile *>(this);
    if (f) f->unsetError();
}
#endif

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

#define Q_VOID

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

#define CHECK_WRITABLE(function, returnType) \
   do { \
       if ((d->openMode & WriteOnly) == 0) { \
           qWarning("QIODevice::"#function" called on a ReadOnly device"); \
           return returnType; \
       } \
   } while (0)

#define CHECK_READABLE(function, returnType) \
   do { \
       if ((d->openMode & ReadOnly) == 0) { \
           qWarning("QIODevice::"#function" called on a WriteOnly device"); \
           return returnType; \
       } \
   } while (0)

/*! \internal
 */
QIODevicePrivate::QIODevicePrivate()
{
    openMode = QIODevice::NotOpen;
    errorString = QT_TRANSLATE_NOOP(QIODevice, "Unknown error");
}

/*! \internal
 */
QIODevicePrivate::~QIODevicePrivate()
{
}

/*!
    \class QIODevice
    \reentrant

    \brief The QIODevice class is the base interface class of all I/O
    devices in Qt.

    \ingroup io

    QIODevice provides both a common implementation and an abstract
    interface for devices that support reading and writing of blocks
    of data, such as QFile, QBuffer and QTcpSocket. QIODevice is
    abstract and can not be instantiated, but is it common to use the
    interface it defines to provide device-independent I/O features.
    For example, Qt's XML classes operate on a QIODevice pointer,
    allowing them to be used with various devices (such as files and
    buffers).

    Before accessing the device, open() must be called to set the
    correct OpenMode (such as ReadOnly or ReadWrite). You can then
    write to the device with write() or putChar(), and read by calling
    either read(), readLine(), or readAll(). Call close() when you are
    done with the device.

    QIODevice distinguishes between two types of devices:
    random-access devices and sequential devices.

    \list
    \o Random-access devices support seeking to arbitrary
    positions using seek(). The current position in the file is
    available by calling pos(). QFile and QBuffer are examples of
    random-access devices.

    \o Sequential devices don't support seeking to arbitrary
    positions. The data must be read in one pass. Functions
    like pos(), seek(), and size() don't work for sequential
    devices. QTcpSocket and QProcess are examples of sequential
    devices.
    \endlist

    You can use isSequential() to determine the type of device.

    QIODevice emits readyRead() when new data is available for
    reading; for example, if new data has arrived on the network or if
    additional data is appended to a file that you are reading
    from. You can call bytesAvailable() to determine the number of
    bytes that currently available for reading. It's common to use
    bytesAvailable() together with the readyRead() signal when
    programming with asynchronous devices such as QTcpSocket, where
    fragments of data can arrive at arbitrary points in
    time. QIODevice emits the bytesWritten() signal every time a
    payload of data has been written to the device. Use bytesToWrite()
    to determine the current amount of data waiting to be written.

    Certain subclasses of QIODevice, such as QTcpSocket and QProcess,
    are asynchronous. This means that I/O functions such as write()
    or read() always return immediately, while communication with the
    device itself may happen when control goes back to the event loop.
    QIODevice provides functions that allow you to force these
    operations to be performed immediately, while blocking the
    calling thread and without entering the event loop. This allows
    QIODevice subclasses to be used without an event loop, or in
    a separate thread:

    \list
    \o flush() - This function suspends operation in the calling
    thread until all outgoing buffered data has been written to the
    device.

    \o waitForReadyRead() - This function suspends operation in the
    calling thread until new data is available for reading.

    \o waitForBytesWritten() - This function suspends operation in the
    calling thread until one payload of data has been written to the
    device.

    \o waitFor....() - Subclasses of QIODevice implement blocking
    functions for device-specific operations. For example, QProcess
    has a function called waitForStarted() which suspends operation in
    the calling thread until the process has started.
    \endlist

    Calling these functions from the main, GUI thread, may cause your
    user interface to freeze. Example:

    \code
        QProcess gzip;
        gzip.start("gzip", QStringList() << "-c");
        if (!gzip.waitForStarted())
            return false;

        gzip.write("uncompressed data");
        gzip.flush();

        QByteArray compressed;
        while (gzip.waitForReadyRead())
            compressed += gzip.readAll();
    \endcode

    By subclassing QIODevice, you can provide the same interface to
    your own I/O devices. Subclasses of QIODevice are only required to
    implement the protected readData() and writeData() functions.
    QIODevice uses these functions to implement all its convenience
    functions, such as getChar(), readLine() and write(). QIODevice
    also handles access control for you, so you can safely assume that
    the device is opened in write mode if writeData() is called.

    Some subclasses, such as QFile and QTcpSocket, are implemented
    using a memory buffer for intermediate storing of data. This
    reduces the number of required device accessing calls, which are
    often very slow. Buffering makes functions like getChar() and
    putChar() fast, as they can operate on the memory buffer instead
    of directly on the device itself. Certain I/O operations, however,
    don't work well with a buffer. For example, if several users open
    the same device and read it character by character, they may end
    up reading the same data when they meant to read a separate chunk
    each. For this reason, QIODevice allows you to bypass any
    buffering by passing the Unbuffered flag to open(). When
    subclassing QIODevice, remember to bypass any buffer you may use
    when the device is open in Unbuffered mode.

    \sa QBuffer QFile QTcpSocket
*/

/*!
    \enum QIODevice::Offset
    \compat

    Use \c qint64 instead.
*/

/*!
    \enum QIODevice::Status
    \compat

    Use QIODevice::OpenMode instead, or see the documentation for
    specific devices.
*/

/*!
    \enum QIODevice::OpenModeFlag

    This enum is used with open() to describe the mode in which a device
    is opened. It is also returned by openMode().

    \value NotOpen   The device is not open.
    \value ReadOnly  The device is open for reading.
    \value WriteOnly The device is open for writing.
    \value ReadWrite The device is open for reading and writing.
    \value Append    The device is opened in append mode, so that all data is
                     written to the end of the file.
    \value Truncate  If possible, the device is truncated before it is opened.
                     All earlier contents of the device are lost.
    \value Text When reading lines using readLine(), end-of-line
                     terminators are translated to the local encoding.
    \value Unbuffered Any buffer in the device is bypassed.

    Certain flags, such as QIODevice::Unbuffered and
    QIODevice::Truncate, might be meaningless for some subclasses.
    (For example, access to a QBuffer is always unbuffered.)
*/

/*!     \fn QIODevice::bytesWritten(qint64 bytes)

    This signal is emitted every time a payload of data has been
    written to the device. The \a bytes argument is set to the number
    of bytes that were written in this payload.

    bytesWritten() is not emitted recursively; if you reenter the event loop
    or call waitForBytesWritten() inside a slot connected to the
    bytesWritten() signal, the signal will not be reemitted (although
    waitForBytesWritten() may still return true).

    \sa readyRead()
*/

/*!     \fn QIODevice::readyRead()

    This signal is emitted once every time new data is available for
    reading from the device. It will only be emitted again once new
    data is available, such as when a new payload of network data has
    arrived on your network socket, or when a new block of data has
    been appended to your file.

    readyRead() is not emitted recursively; if you reenter the event loop or
    call waitForReadyRead() inside a slot connected to the readyRead() signal,
    the signal will not be reemitted (although waitForReadyRead() may still
    return true).

    \sa bytesWritten()
*/

/*! \fn QIODevice::aboutToClose()

    This signal is emitted when the device is about to close. Connect
    this signal if you have operations that need to be performed
    before the device closes (e.g., if you have data in a separate
    buffer that needs to be written to the device).
*/

#ifdef QT_NO_QOBJECT
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
#else

/*!
    Constructs a QIODevice object.
*/

QIODevice::QIODevice()
    : QObject(*new QIODevicePrivate, 0)
{
}

/*!
    Constructs a QIODevice object with the given \a parent.
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
    Destructs the QIODevice object.
*/
QIODevice::~QIODevice()
{
}

/*!
    Returns true if this device is sequential; otherwise returns
    false.

    Sequential devices, as opposed to a random-access devices, have no
    concept of a start, an end, a size, or a current position, and they
    do not support seeking. You can only read from the device when it
    reports that data is available. The most common example of a
    sequential device is a network socket. On Unix, special files such
    as /dev/zero and fifo pipes are sequential.

    Regular files, on the other hand, do support random access. They
    have both a size and a current position, and they also support
    seeking backwards and forwards in the data stream. Regular files
    are non-sequential.

    \sa bytesAvailable()
*/
bool QIODevice::isSequential() const
{
    return false;
}

/*!
    Returns the mode in which the device has been opened;
    i.e. ReadOnly or WriteOnly.

    \sa OpenMode
*/
QIODevice::OpenMode QIODevice::openMode() const
{
    return d_func()->openMode;
}

/*!
    Sets the OpenMode of the device to \a openMode. Reimplement this
    function to set the open mode when reimplementing open().

    \sa openMode() OpenMode
*/
void QIODevice::setOpenMode(OpenMode openMode)
{
    d_func()->openMode = openMode;
}

/*!
    If \a enabled is true, this function sets the \l Text flag on the device;
    otherwise the \l Text flag is removed. This feature is useful for classes
    that provide custom end-of-line handling on a QIODevice.

    \sa open(), setOpenMode()
 */
void QIODevice::setTextModeEnabled(bool enabled)
{
    Q_D(QIODevice);
    if (enabled)
        d->openMode |= Text;
    else
        d->openMode &= ~Text;
}

/*!
    Returns true if the \l Text flag is enabled; otherwise returns false.

    \sa setTextModeEnabled()
*/
bool QIODevice::isTextModeEnabled() const
{
    return d_func()->openMode & Text;
}

/*!
    Returns true if the device is open; otherwise returns false. A
    device is open if it can be read from and/or written to. By
    default, this function returns false if openMode() returns
    \c NotOpen.

    \sa openMode() OpenMode
*/
bool QIODevice::isOpen() const
{
    return d_func()->openMode != NotOpen;
}

/*!
    Returns true if data can be read from the device; otherwise returns
    false. Use bytesAvailable() to determine how many bytes can be read.

    This is a convenience function which checks if the OpenMode of the
    device contains the ReadOnly flag.

    \sa openMode() OpenMode
*/
bool QIODevice::isReadable() const
{
    return (openMode() & ReadOnly) != 0;
}

/*!
    Returns true if data can be written to the device; otherwise returns
    false.

    This is a convenience function which checks if the OpenMode of the
    device contains the WriteOnly flag.

    \sa openMode() OpenMode
*/
bool QIODevice::isWritable() const
{
    return (openMode() & WriteOnly) != 0;
}

/*!
    Opens the device and sets its OpenMode to \a mode.

    \sa openMode() OpenMode
*/
bool QIODevice::open(OpenMode mode)
{
    d_func()->openMode = mode;
    return true;
}

/*!
    First emits aboutToClose(), then closes the device and sets its
    OpenMode to NotOpen. The error string is also reset.

    \sa setOpenMode() OpenMode
*/
void QIODevice::close()
{
    Q_D(QIODevice);
    if (d->openMode == NotOpen)
        return;

#ifndef QT_NO_QOBJECT
    emit aboutToClose();
#endif
    d->openMode = NotOpen;
#ifdef QT_NO_QOBJECT
    d->errorString = QT_TRANSLATE_NOOP(QIODevice, "Unknown error");
#else
    d->errorString = tr("Unknown error");
#endif
}

/*!
    This function flushes any pending written data to the
    device. Returns true on success; otherwise returns false.

    If this function returns true, all data is guaranteed to have been
    written to the device. If it returns false, the number of bytes
    that have been written can be checked by calling bytesToWrite()
    before and after the call, or by connecting to bytesWritten().

    Calling this function is equivalent to calling
    waitForBytesWritten(-1) until bytesToWrite() returns 0.
*/
bool QIODevice::flush()
{
    Q_D(QIODevice);
    qint64 toWrite = bytesToWrite();
    while (bytesToWrite() > 0) {
        if (!waitForBytesWritten(-1))
            return false;
    }
    d->ungetBuffer.chop(toWrite);
    return true;
}

/*!
    For random-access devices, this function returns the position that
    data is written to or read from. For sequential devices or closed
    devices, where there is no concept of a "current position", 0 is
    returned.

    \sa isSequential(), seek()
*/
qint64 QIODevice::pos() const
{
    return qint64(0);
}

/*!
    For open random-access devices, this function returns the size of the
    device. For open sequential devices, bytesAvailable() is returned.

    If the device is closed, the size returned will not reflect the actual
    size of the device.
*/
qint64 QIODevice::size() const
{
    if (isSequential())
        return bytesAvailable();
    return 0;
}

/*!
    For random-access devices, this function sets the current position
    to \a pos, returning true on success, or false if an error occurred.
    For sequential devices, the default behavior is to do nothing and
    return false.

    When subclassing QIODevice, you must call QIODevice::seek() at the
    start of your function to ensure integrity with QIODevice's
    built-in ungetbuffer. The base implementation always returns true.

    \sa pos()
*/
bool QIODevice::seek(qint64 pos)
{
    Q_D(QIODevice);
    if (pos > 0)
        d->ungetBuffer.chop(pos);
    else
        d->ungetBuffer.clear();
    return true;
}

/*!
    Returns true if the current read and write position is at the end
    of the device (i.e. there is no more data available for reading on
    the device); otherwise returns false.
*/
bool QIODevice::atEnd() const
{
    return isOpen() && (pos() == size());
}

/*!
    Seeks to the start of input for random-access devices. Returns
    true on success; otherwise returns false (for example, if the
    device is not open).

    \sa seek()
*/
bool QIODevice::reset()
{
    return seek(0);
}

/*!
    Returns the number of bytes that are available for reading. This
    function is commonly used with sequential devices to determine the
    number of bytes to allocate in a buffer before reading.
*/
qint64 QIODevice::bytesAvailable() const
{
    if (!isSequential())
        return size() - pos();
    return 0;
}

/*!
    For buffered devices, this function returns the number of bytes
    waiting to be written. For devices with no buffer, this function
    returns 0.

    \sa flush()
*/
qint64 QIODevice::bytesToWrite() const
{
    return qint64(0);
}

/*!
    Reads at most \a maxlen bytes from the device into \a data, and
    returns the number of bytes read. If an error occurs, such as when
    attempting to read from a device opened in WriteOnly mode, this
    function returns -1.

    0 is returned when no more data is available for reading.

    \sa readData() readLine() write()
*/
qint64 QIODevice::read(char *data, qint64 maxlen)
{
    Q_D(QIODevice);
    CHECK_OPEN(read, qint64(-1));
    CHECK_READABLE(read, qint64(-1));
    CHECK_MAXLEN(read, qint64(-1));
    qint64 readSoFar = qint64(0);

    if (int ungetSize = d->ungetBuffer.size()) {
        do {
            if (readSoFar + 1 > maxlen) {
                d->ungetBuffer.resize(d->ungetBuffer.size() - readSoFar);
                return readSoFar;
            }

            data[readSoFar++] = d->ungetBuffer[ungetSize-- - 1];
        } while (ungetSize > 0);
        d->ungetBuffer.resize(d->ungetBuffer.size() - readSoFar);
    }

    qint64 ret = readData(data + readSoFar, maxlen - readSoFar);
    if (ret <= 0)
        return readSoFar ? readSoFar : ret;

    if (d->openMode & Text) {
        forever {
            char *readPtr = data + readSoFar;
            char *endPtr = readPtr + ret;

            // optimization to avoid initial self-assignment
            while (*readPtr != '\r') {
                if (++readPtr == endPtr)
                    return readSoFar + ret;
            }

            char *writePtr = readPtr;

            while (readPtr < endPtr) {
                char ch = *readPtr++;
                if (ch != '\r') {
                    *writePtr++ = ch;
                } else {
                    --ret;
                }
            }

            if (readPtr == writePtr)
                break;

            qint64 newRet = readData(writePtr, readPtr - writePtr);
            if (newRet <= 0)
                break;

            readSoFar += ret;
            ret = newRet;
        }
    }

    return readSoFar + ret;
}

/*!
    \overload

    Reads at most \a maxlen bytes from the device, and returns the
    data read as a QByteArray.

    This function has no way of reporting errors; returning an empty
    QByteArray() can mean either that no data was currently available
    for reading, or that an error occurred.
*/
QByteArray QIODevice::read(qint64 maxlen)
{
    CHECK_MAXLEN(read, QByteArray());
    QByteArray tmp;
    qint64 readSoFar = 0;
    char buffer[4096];

    do {
        qint64 bytesToRead = qMin(int(maxlen - readSoFar), int(sizeof(buffer)));
        qint64 readBytes = read(buffer, bytesToRead);
        if (readBytes <= 0)
            break;
        tmp += QByteArray(buffer, (int) readBytes);
        readSoFar += readBytes;
    } while (readSoFar < maxlen && bytesAvailable() > 0);

    return tmp;
}

/*!
    \overload

    Reads all available data from the device, and returns it as a
    QByteArray.

    This function has no way of reporting errors; returning an empty
    QByteArray() can mean either that no data was currently available
    for reading, or that an error occurred.
*/
QByteArray QIODevice::readAll()
{
    const int chunkSize = 4096;
    qint64 totalRead = 0;
    QByteArray tmp;
    forever {
        tmp.resize(tmp.size() + chunkSize);
        qint64 readBytes = read(tmp.data() + totalRead, chunkSize);
        if (readBytes < chunkSize) {
            tmp.chop(chunkSize - (readBytes < 0 ? qint64(0) : readBytes));
            return tmp;
        }
        totalRead += readBytes;
    }
}

/*!
    This function reads a line of ASCII characters from the device, up
    to a maximum of \a maxlen bytes, stores the characters in \a data,
    and returns the number of bytes read. If an error occurred, -1 is
    returned.

    If there is room in the buffer (i.e. the line read is shorter than
    \a maxlen characters), a '\0' byte is appended to \a data.

    Data is read until either of the following conditions are met:

    \list
    \o The first '\n' character is read.
    \o \a maxlen bytes are read.
    \o The end of the device data is detected.
    \endlist

    For example, the following code reads a line of characters from a
    file:

    \code
        QFile file("box.txt");
        if (file.open(QFile::ReadOnly)) {
            char buf[1024];
            qint64 lineLength = file.readLine(buf, sizeof(buf));
            if (lineLength != -1) {
                // the line is available in buf
            }
        }
    \endcode

    If the '\n' character is the 1024th character read then it will be
    inserted into the buffer; if it occurs after the 1024 character then
    it is not read.

    This function calls readLineData(), which is implemented using
    repeated calls to getChar(). You can provide a more efficient
    implementation by reimplementing readLineData() in your own
    subclass.

    \sa getChar(), read(), write()
*/
qint64 QIODevice::readLine(char *data, qint64 maxlen)
{
    Q_D(QIODevice);
    if (maxlen < 1) {
        qWarning("QIODevice::readLine() called with maxlen < 1");
        return qint64(-1);
    }

    qint64 readSoFar = 0;
    if (int ungetSize = d->ungetBuffer.size()) {
        do {
            if (readSoFar + 1 > maxlen) {
                if (readSoFar < maxlen)
                    data[readSoFar] = '\0';
                d->ungetBuffer.resize(d->ungetBuffer.size() - readSoFar);
                return readSoFar;
            }

            char c = d->ungetBuffer[ungetSize-- - 1];
            data[readSoFar++] = c;
            if (c == '\n') {
                if (readSoFar < maxlen)
                    data[readSoFar] = '\0';
                return readSoFar;
            }
        } while (ungetSize > 0);
        d->ungetBuffer.resize(d->ungetBuffer.size() - readSoFar);
    }

    return readLineData(data + readSoFar, maxlen - readSoFar);
}

/*!
    \overload

    Reads a line from the device, but no more than \a maxlen characters,
    and returns the result as a QByteArray.

    This function has no way of reporting errors; returning an empty
    QByteArray() can mean either that no data was currently available
    for reading, or that an error occurred.
*/
QByteArray QIODevice::readLine(qint64 maxlen)
{
    CHECK_MAXLEN(readLine, QByteArray());
    QByteArray tmp;
    char buffer[4096];
    Q_UNUSED(buffer);
    qint64 readSoFar = 0;
    qint64 readBytes = 0;

    do {
        if (maxlen != 0)
            tmp.resize(readSoFar + qMin(int(maxlen), int(sizeof(buffer))));
        else
            tmp.resize(readSoFar + int(sizeof(buffer)));
        readBytes = readLineData(tmp.data() + readSoFar, tmp.size());
        readSoFar += readBytes;
    } while (readSoFar < maxlen && readBytes > 0
             && readBytes == tmp.size() && tmp.at(readBytes - 1) != '\n');

    tmp.resize(readSoFar);
    return tmp;
}

/*!
    This function is called by readLine(), and provides its base
    implementation, using getChar(). Buffered devices can improve the
    performance of readLine() by reimplementing this function.

    When reimplementing this function, keep in mind that you must
    handle the \l Text flag which translates end-of-line characters.
*/
qint64 QIODevice::readLineData(char *data, qint64 maxlen)
{
    qint64 readSoFar = 0;
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
        return qint64(-1);
    return readSoFar;
}

/*!
    Returns true if a complete line of data can be read from the device;
    otherwise returns false.

    Note that unbuffered devices, which have no way of determining what
    can be read, always return false.

    This function is often called in conjunction with the readyRead()
    signal.

    \sa readyRead(), readLine()
*/
bool QIODevice::canReadLine() const
{
    return false;
}

/*! \fn bool QIODevice::getChar(char *c)

    Reads one character from the device and stores it in \a c. If \a c
    is 0, the character is discarded. Returns true on success;
    otherwise returns false.

    \sa read() putChar() ungetChar()
*/

/*!
    Writes at most \a maxlen bytes of data from \a data to the
    device. Returns the number of bytes that were actually written, or
    -1 if an error occurred.

    \sa read() writeData()
*/
qint64 QIODevice::write(const char *data, qint64 maxlen)
{
    Q_D(QIODevice);
    CHECK_OPEN(write, qint64(-1));
    CHECK_WRITABLE(write, qint64(-1));
    CHECK_MAXLEN(write, qint64(-1));

#ifdef Q_OS_WIN
    if (d->openMode & Text) {
        const char *endOfData = data + maxlen;
        const char *startOfBlock = data;

        qint64 writtenSoFar = 0;

        forever {
            const char *endOfBlock = startOfBlock;
            while (endOfBlock < endOfData && *endOfBlock != '\n')
                ++endOfBlock;

            qint64 blockSize = endOfBlock - startOfBlock;
            if (blockSize > 0) {
                qint64 ret = writeData(startOfBlock, blockSize);
                if (ret <= 0) {
                    if (writtenSoFar)
                        d->ungetBuffer.chop(writtenSoFar);
                    return writtenSoFar ? writtenSoFar : ret;
                }
                writtenSoFar += ret;
            }

            if (endOfBlock == endOfData)
                break;

            qint64 ret = writeData("\r\n", 2);
            if (ret <= 0) {
                if (writtenSoFar)
                    d->ungetBuffer.chop(writtenSoFar);
                return writtenSoFar ? writtenSoFar : ret;
            }
            ++writtenSoFar;

            startOfBlock = endOfBlock + 1;
        }

        if (writtenSoFar)
            d->ungetBuffer.chop(writtenSoFar);
        return writtenSoFar;
    }
#endif
    qint64 written = writeData(data, maxlen);
    d->ungetBuffer.chop(written);
    return written;
}

/*! \fn qint64 QIODevice::write(const QByteArray &byteArray)

    \overload

    Writes the content of \a byteArray to the device. Returns the number of
    bytes that were actually written, or -1 if an error occurred.

    \sa read() writeData()
*/

/*! \fn bool QIODevice::putChar(char c)

    Writes the character \a c to the device. Returns true on success;
    otherwise returns false.

    \sa write() getChar() ungetChar()
*/

/*!
    Puts the character \a c back into the device, and decrements the
    current position unless the position is 0. This function is
    usually called to "undo" a getChar() operation, such as when
    writing a backtracking parser.

    If \a c was not previously read from the device, the behavior is
    undefined.
*/
void QIODevice::ungetChar(char c)
{
    Q_D(QIODevice);
    CHECK_OPEN(write, Q_VOID);
    CHECK_READABLE(read, Q_VOID);
    d->ungetBuffer.append(c);
    if (!isSequential()) {
        qint64 curPos = pos();
        if (curPos > 0)
            seek(curPos - 1);
    }
}

/*!
    Blocks until data is available for reading and the readyRead()
    signal has been emitted, or until \a msecs milliseconds have
    passed. If msecs is -1, this function will not time out.

    Returns true if data is available for reading; otherwise returns
    false (if the operation timed out or if an error occurred).

    This function can operate without an event loop. It is
    useful when writing non-GUI applications and when performing
    I/O operations in a non-GUI thread.

    If called from within a slot connected to the readyRead() signal,
    readyRead() will not be reemitted.
    
    \warning Calling this function from the main (GUI) thread
    might cause your user interface to freeze.

    \sa waitForBytesWritten()
*/
bool QIODevice::waitForReadyRead(int msecs)
{
    Q_UNUSED(msecs);
    return false;
}

/*!
    For buffered devices, this function waits until a payload of
    buffered written data has been written to the device and the
    bytesWritten() signal has been emitted, or until \a msecs
    milliseconds have passed. If msecs is -1, this function will
    not time out. For unbuffered devices, it returns immediately.

    Returns true if a payload of data was written to the device;
    otherwise returns false (i.e. if the operation timed out, or if an
    error occurred).

    This function can operate without an event loop. It is
    useful when writing non-GUI applications and when performing
    I/O operations in a non-GUI thread.

    If called from within a slot connected to the bytesWritten() signal,
    bytesWritten() will not be reemitted.

    \warning Calling this function from the main (GUI) thread
    might cause your user interface to freeze.

    \sa waitForReadyRead()
*/
bool QIODevice::waitForBytesWritten(int msecs)
{
    Q_UNUSED(msecs);
    return false;
}

/*!
    Sets the human readable description of the last device error that
    occurred to \a str.
*/
void QIODevice::setErrorString(const QString &str)
{
    d_func()->errorString = str;
}

/*!
    Returns a human-readable description of the last device error that
    occurred.
*/
QString QIODevice::errorString() const
{
    return d_func()->errorString;
}

/*!
    \fn qint64 QIODevice::readData(char *data, qint64 maxlen)

    Reads up to \a maxlen bytes from the device into \a data, and
    returns the number of bytes read or -1 if an error occurred.

    This function is called by QIODevice. Reimplement this function
    when creating a subclass of QIODevice.

    \sa read() readLine() writeData()
*/

/*!
    \fn qint64 QIODevice::writeData(const char *data, qint64 maxlen)

    Writes up to \a maxlen bytes from \a data to the device. Returns
    the number of bytes written, or -1 if an error occurred.

    This function is called by QIODevice. Reimplement this function
    when creating a subclass of QIODevice.

    \sa read() write()
*/

/*!
    \fn QIODevice::Offset QIODevice::status() const

    For device specific error handling, please refer to the
    individual device documentation.

    \sa qobject_cast<>()
*/

/*!
    \fn QIODevice::Offset QIODevice::at() const

    Use pos() instead.
*/

/*!
    \fn bool QIODevice::at(Offset offset)

    Use seek(\a offset) instead.
*/

/*! \fn int QIODevice::flags() const

    Use openMode() instead.
*/

/*! \fn int QIODevice::getch()

    Use getChar() instead.
*/

/*!
    \fn bool QIODevice::isAsynchronous() const

    This functionality is no longer available. This function always
    returns true.
*/

/*!
    \fn bool QIODevice::isBuffered() const

    Use !(openMode() & QIODevice::Unbuffered) instead.
*/

/*!
    \fn bool QIODevice::isCombinedAccess() const

    Use openMode() instead.
*/

/*!
    \fn bool QIODevice::isDirectAccess() const

    Use !isSequential() instead.
*/

/*!
    \fn bool QIODevice::isInactive() const

    Use isOpen(), isReadable(), or isWritable() instead.
*/

/*!
    \fn bool QIODevice::isRaw() const

    Use openMode() instead.
*/

/*!
    \fn bool QIODevice::isSequentialAccess() const

    Use isSequential() instead.
*/

/*!
    \fn bool QIODevice::isSynchronous() const

    This functionality is no longer available. This function always
    returns false.
*/

/*!
    \fn bool QIODevice::isTranslated() const

    Use openMode() instead.
*/

/*!
    \fn bool QIODevice::mode() const

    Use openMode() instead.
*/

/*! \fn int QIODevice::putch(int ch)

    Use putChar(\a ch) instead.
*/

/*! \fn int QIODevice::ungetch(int ch)

    Use ungetChar(\a ch) instead.
*/

/*!
    \fn quint64 QIODevice::readBlock(char *data, quint64 size)

    Use read(\a data, \a size) instead.
*/

/*! \fn int QIODevice::state() const

    Use isOpen() instead.
*/

/*!
    \fn qint64 QIODevice::writeBlock(const char *data, quint64 size)

    Use write(\a data, \a size) instead.
*/

/*!
    \fn qint64 QIODevice::writeBlock(const QByteArray &data)

    Use write(\a data) instead.
*/

#if defined QT3_SUPPORT
QIODevice::Status QIODevice::status() const
{
#if !defined(QT_NO_QOBJECT)
    const QFile *f = qobject_cast<const QFile *>(this);
    if (f) return (int) f->error();
#endif
    return isOpen() ? 0 /* IO_Ok */ : 8 /* IO_UnspecifiedError */;
}

/*!
    For device specific error handling, please refer to the
    individual device documentation.

    \sa qobject_cast()
*/
void QIODevice::resetStatus()
{
#if !defined(QT_NO_QOBJECT)
    QFile *f = qobject_cast<QFile *>(this);
    if (f) f->unsetError();
#endif
}
#endif

/****************************************************************************
**
** Implementation of QBuffer class.
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

#include "qbuffer.h"
#include "qobjectdefs.h"
#include "private/qiodevice_p.h"

#define d d_func()
#define q q_func()

class QBufferPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QBuffer)

public:
    ~QBufferPrivate();

    QByteArray defaultBuf;
    QByteArray *buf;
};

QBufferPrivate::~QBufferPrivate()
{
}

/*!
    \class QBuffer
    \reentrant
    \brief The QBuffer class is an I/O device that operates on a QByteArray.

    \ingroup io

    QBuffer is used to read and write to a memory buffer. It is
    normally used with a QTextStream or a QDataStream. QBuffer has an
    associated QByteArray which holds the buffer data. The buffer is
    automatically resized as data is written.

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
    constructors operate on an internal QBuffer.

    Note that QTextStream can also operate on a QString (a Unicode
    string), but a QBuffer cannot.

    You can also use QBuffer directly through the standard QIODevice
    functions readBlock(), writeBlock() readLine(), at(), getch(),
    putch() and ungetch().

    \sa QFile, QDataStream, QTextStream, QByteArray, \link shclass.html Shared Classes\endlink
*/


/*!
    Constructs an empty buffer.
*/

QBuffer::QBuffer()
    : QIODevice(*new QBufferPrivate)
{
    d->buf = &d->defaultBuf;
    setFlags(QIODevice::Direct);
    ioIndex = 0;
}


/*!
    Constructs a buffer that operates on QByteArray \a a.

    If you open the buffer in write mode (\c QIODevice::WriteOnly or
    \c QIODevice::ReadWrite) and write something into the buffer, the byte
    array, \a a will be modified.

    Example:
    \code
        QCString str = "abc";
        QBuffer b(str);
        b.open(QIODevice::WriteOnly);
        b.at(3); // position at the 4th character (the terminating \0)
        b.writeBlock("def", 4); // write "def" including the terminating \0
        b.close();
        // Now, str == "abcdef" with a terminating \0
    \endcode

    \sa setBuffer()
*/

QBuffer::QBuffer(QByteArray *a)
    : QIODevice(*new QBufferPrivate)
{
    d->buf = a;
    setFlags(QIODevice::Direct);
    ioIndex = 0;
}

/*!
    Destroys the buffer.
*/

QBuffer::~QBuffer()
{
}

/*!
    Replaces the buffer's contents with \a a.

    Does nothing if isOpen() is true.

    Note that if you open the buffer in write mode (\c QIODevice::WriteOnly or
    QIODevice::ReadWrite) and write something into the buffer, and \a a is not
    0, \a a is modified because QByteArray is an explicitly shared
    class.

    If \a a is 0, the buffer creates its own (initially empty)
    internal QByteArray to work on.

    \sa buffer(), open(), close()
*/

void QBuffer::setBuffer(QByteArray *a)
{
    if (isOpen()) {
        qWarning("QBuffer::setBuffer: Buffer is open");
        return;
    }
    if (a) {
        d->buf = a;
    } else {
        d->buf = &d->defaultBuf;
    }
    d->defaultBuf.clear();
    ioIndex = 0;
}

/*!
    \fn void QBuffer::setData(const char *data, int len)

    \overload

    Sets the data to be the first \a len bytes of the \a data string.
*/

/*!
    Sets the byte array for the buffer to be \a data.

    Does nothing if isOpe() is true. Since \a data is const the buffer
    can only be used to read from it.
*/
void QBuffer::setData(const QByteArray &data)
{
    if (isOpen()) {
        qWarning("QBuffer::setData: Buffer is open");
        return;
    }
    *d->buf = data;
}

/*!
    Returns this buffer's byte array.

    \sa setBuffer()
*/

QByteArray &QBuffer::buffer()
{
    return *d->buf;
}

/*!
    \overload
*/
const QByteArray &QBuffer::buffer() const
{
    return *d->buf;
}

/*!
    \reimp

    Opens the buffer in mode \a m. Returns true if successful;
    otherwise returns false. The buffer must be opened before use.

    The mode parameter \a m must be a combination of the following flags.
    \list
    \i \c QIODevice::ReadOnly opens the buffer in read-only mode.
    \i \c QIODevice::WriteOnly opens the buffer in write-only mode.
    \i \c QIODevice::ReadWrite opens the buffer in read/write mode.
    \i \c QIODevice::Append sets the buffer index to the end of the buffer.
    \i \c QIODevice::Truncate truncates the buffer.
    \endlist

    \sa close(), isOpen()
*/

bool QBuffer::open(int m)
{
    if (isOpen()) {                           // buffer already open
        qWarning("QBuffer::open: Buffer already open");
        return false;
    }
    setMode(m);
    if (m & QIODevice::Truncate)
        d->buf->resize(0);
    if (m & QIODevice::Append)                       // append to end of buffer
        ioIndex = d->buf->size();
    else
        ioIndex = 0;

    setState(QIODevice::Open);
    resetStatus();
    return true;
}

/*!
    \reimp

    Closes an open buffer.

    \sa open()
*/

void QBuffer::close()
{
    if (isOpen()) {
        setFlags(QIODevice::Direct);
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
    \reimp
*/
QIODevice::Offset QBuffer::at() const
{
    return ioIndex;
}

/*!
    \reimp
*/
QIODevice::Offset QBuffer::size() const
{
    return (Offset)d->buf->size();
}

/*!
  \reimp
*/

bool QBuffer::at(Offset pos)
{
    if (!isOpen()) {
        qWarning("QBuffer::at: Buffer is not open");
        return false;
    }
    // #### maybe resize if not readonly?
    if (pos > (Offset)d->buf->size()) {
        qWarning("QBuffer::at: Index %lld out of range", pos);
        return false;
    }
    ioIndex = pos;
    return true;
}


/*!
  \reimp
*/

Q_LONG QBuffer::readBlock(char *p, Q_LONG len)
{
    if (!p) {
        qWarning("QBuffer::readBlock: Null pointer error");
        return -1;
    }
    if (!isOpen()) {                          // buffer not open
        qWarning("QBuffer::readBlock: Buffer not open");
        return -1;
    }
    if (!isReadable()) {                      // reading not permitted
        qWarning("QBuffer::readBlock: Read operation not permitted");
        return -1;
    }
    if (ioIndex + len > d->buf->size()) {   // overflow
        if ((int)ioIndex >= d->buf->size()) {
            return 0;
        } else {
            len = d->buf->size() - ioIndex;
        }
    }
    memcpy(p, d->buf->constData() + ioIndex, len);
    ioIndex += len;
    return len;
}

/*!
    \fn Q_LONG QBuffer::writeBlock(const QByteArray& data)
    \overload

    This convenience function is the same as calling
    \c{writeBlock(data.data(), data.size())} with \a data.
*/

/*!
    Writes \a len bytes from \a ptr into the buffer at the current
    index position, overwriting any characters there and extending the
    buffer if necessary. Returns the number of bytes actually written.

    Returns -1 if an error occurred.

    \sa readBlock()
*/

Q_LONG QBuffer::writeBlock(const char *ptr, Q_LONG len)
{
    if (len == 0)
        return 0;

    if (ptr == 0) {
        qWarning("QBuffer::writeBlock: Null pointer error");
        return -1;
    }
    if (!isOpen()) {                          // buffer not open
        qWarning("QBuffer::writeBlock: Buffer not open");
        return -1;
    }
    if (!isWritable()) {                      // writing not permitted
        qWarning("QBuffer::writeBlock: Write operation not permitted");
        return -1;
    }
    if (ioIndex + len > d->buf->size()) {             // overflow
        d->buf->resize(ioIndex + len);
        if (d->buf->size() != (int)(ioIndex + len)) {           // could not resize
            qWarning("QBuffer::writeBlock: Memory allocation error");
            setStatus(QIODevice::ResourceError);
            return -1;
        }
    }
    memcpy(d->buf->data() + ioIndex, ptr, len);
    ioIndex += len;
    return len;
}


/*!
  \reimp
*/

Q_LONG QBuffer::readLine(char *p, Q_LONG maxlen)
{
    if (p == 0) {
        qWarning("QBuffer::readLine: Null pointer error");
        return -1;
    }
    if (!isOpen()) {                          // buffer not open
        qWarning("QBuffer::readLine: Buffer not open");
        return -1;
    }
    if (!isReadable()) {                      // reading not permitted
        qWarning("QBuffer::readLine: Read operation not permitted");
        return -1;
    }
    if (maxlen == 0)
        return 0;
    Q_LONG start = ioIndex;
    const char *dat = d->buf->constData() + ioIndex;
    maxlen--;                                   // make room for 0-terminator
    if (d->buf->size() - ioIndex < maxlen)
        maxlen = d->buf->size() - ioIndex;
    while (maxlen--) {
        if ((*p++ = *dat++) == '\n')
            break;
    }
    *p = '\0';
    ioIndex = dat - d->buf->constData();
    return ioIndex - start;
}


/*!
  \reimp
*/

int QBuffer::getch()
{
    if (!isOpen()) {                          // buffer not open
        qWarning("QBuffer::getch: Buffer not open");
        return -1;
    }
    if (!isReadable()) {                      // reading not permitted
        qWarning("QBuffer::getch: Read operation not permitted");
        return -1;
    }
    if (ioIndex >= d->buf->size()) {               // overflow
        setStatus(QIODevice::ReadError);
        return -1;
    }
    return uchar(d->buf->constData()[ioIndex++]);
}

/*!
    \reimp

    Writes the character \a ch into the buffer at the current index
    position, overwriting any existing character and extending the
    buffer if necessary.

    Returns \a ch, or -1 if an error occurred.

    \sa getch(), ungetch()
*/

int QBuffer::putch(int ch)
{
    if (!isOpen()) {                          // buffer not open
        qWarning("QBuffer::putch: Buffer not open");
        return -1;
    }
    if (!isWritable()) {                      // writing not permitted
        qWarning("QBuffer::putch: Write operation not permitted");
        return -1;
    }
    if (ioIndex >= d->buf->size()) {               // overflow
        char tinyBuf;
        tinyBuf = (char)ch;
        if (writeBlock(&tinyBuf, 1) != 1)
            return -1;                          // write error
    } else {
        d->buf->data()[ioIndex++] = (char)ch;
    }
    return ch;
}

/*!
  \reimp
*/

int QBuffer::ungetch(int ch)
{
    if (!isOpen()) {                          // buffer not open
        qWarning("QBuffer::ungetch: Buffer not open");
        return -1;
    }
    if (!isReadable()) {                      // reading not permitted
        qWarning("QBuffer::ungetch: Read operation not permitted");
        return -1;
    }
    if (ch != -1) {
        if (ioIndex)
            ioIndex--;
        else
            ch = -1;
    }
    return ch;
}

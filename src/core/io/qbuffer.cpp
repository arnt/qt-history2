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
#include "qioengine.h"
#include "qobjectdefs.h"
#include "private/qiodevice_p.h"
#include "private/qioengine_p.h"

#define d d_func()
#define q q_func()

/** QBufferEngine **/
class QBufferEnginePrivate;
class QBufferEngine : public QIOEngine
{
private:
    Q_DECLARE_PRIVATE(QBufferEngine)
public:
    QBufferEngine(QByteArray *);
    inline void setData(QByteArray *);

    virtual bool open(int flags);
    virtual bool close();
    virtual void flush();

    virtual QIODevice::Offset size() const;
    virtual QIODevice::Offset at() const;
    virtual bool seek(QIODevice::Offset);

    virtual bool isSequential() const { return false; }

    virtual Q_LONG readBlock(char *data, Q_LONG maxlen);
    virtual Q_LONG writeBlock(const char *data, Q_LONG len);
    virtual Q_LONG readLine(char *data, Q_LONG maxlen);

    virtual Type type() const;

    virtual int getch();
    virtual int putch(int);
    virtual int ungetch(int);
};

class QBufferEnginePrivate : public QIOEnginePrivate
{
private:
    Q_DECLARE_PUBLIC(QBufferEngine)
protected:    
    QBufferEnginePrivate() : ioIndex(0), buf(0) { }
    int ioIndex;
    QByteArray *buf;
};

QBufferEngine::QBufferEngine(QByteArray *data) : QIOEngine(*new QBufferEnginePrivate)
{
    setData(data);
}

void QBufferEngine::setData(QByteArray *data)
{
    d->ioIndex = 0;
    d->buf = data;
}

bool QBufferEngine::open(int flags)
{
    if (flags & QIODevice::Truncate) 
        d->buf->resize(0);
    if (flags & QIODevice::Append)                       // append to end of buffer
        d->ioIndex = d->buf->size();
    else
        d->ioIndex = 0;
    return true;
}

bool QBufferEngine::close()
{
    if(d->ioIndex == -1)
        return false;
    d->ioIndex = -1;
    return true;
}

void QBufferEngine::flush()
{
}


QIODevice::Offset QBufferEngine::at() const
{
    return d->ioIndex;
}

QIODevice::Offset QBufferEngine::size() const
{
    return (QIODevice::Offset)d->buf->size();
}

bool QBufferEngine::seek(QIODevice::Offset pos)
{
    // #### maybe resize if not readonly?
    if (pos > (QIODevice::Offset)d->buf->size()) {
        qWarning("QBufferEngine::seek: Index %lld out of range", pos);
        return false;
    }
    d->ioIndex = pos;
    return true;
}

Q_LONG QBufferEngine::readBlock(char *ptr, Q_LONG len)
{
    if (d->ioIndex + len > d->buf->size()) {   // overflow
        if ((int)d->ioIndex >= d->buf->size()) {
            return 0;
        } else {
            len = d->buf->size() - d->ioIndex;
        }
    }
    memcpy(ptr, d->buf->constData() + d->ioIndex, len);
    d->ioIndex += len;
    return len;
}

Q_LONG QBufferEngine::writeBlock(const char *ptr, Q_LONG len)
{
    if (d->ioIndex + len > d->buf->size()) {             // overflow
        d->buf->resize(d->ioIndex + len);
        if (d->buf->size() != (int)(d->ioIndex + len)) {           // could not resize
            qWarning("QBufferEngine::writeBlock: Memory allocation error");
            return -1;
        }
    }
    memcpy(d->buf->data() + d->ioIndex, (uchar *)ptr, len);
    d->ioIndex += len;
    return len;
}

Q_LONG QBufferEngine::readLine(char *p, Q_LONG maxlen)
{
    Q_LONG start = d->ioIndex;
    const char *dat = d->buf->constData() + d->ioIndex;
    maxlen--;                                   // make room for 0-terminator
    if (d->buf->size() - d->ioIndex < maxlen)
        maxlen = d->buf->size() - d->ioIndex;
    while (maxlen--) {
        if ((*p++ = *dat++) == '\n')
            break;
    }
    *p = '\0';
    d->ioIndex = dat - d->buf->constData();
    return d->ioIndex - start;
}

QIOEngine::Type 
QBufferEngine::type() const
{
    return QIOEngine::String;
}

int QBufferEngine::getch()
{
    if (d->ioIndex >= d->buf->size()) // overflow
        return -1;
    return (uchar)d->buf->constData()[d->ioIndex++];
}

int QBufferEngine::putch(int ch)
{
    if (d->ioIndex >= d->buf->size()) {               // overflow
        char tinyBuf;
        tinyBuf = (char)ch;
        if (writeBlock(&tinyBuf, 1) != 1)
            return -1;                          // write error
    } else {
        d->buf->data()[d->ioIndex++] = (uchar)ch;
    }
    return ch;
}

int QBufferEngine::ungetch(int ch)
{
    if (ch != -1) {
        if (d->ioIndex) 
            d->buf->data()[--d->ioIndex] = (char)ch;
        else
            ch = -1;
    }
    return ch;
}

/** QBufferPrivate **/
class QBufferPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QBuffer)

public:
    QBufferPrivate() : bufferEngine(0) { }
    ~QBufferPrivate();

    QByteArray defaultBuf;
    QByteArray *buf;
    mutable QBufferEngine *bufferEngine;
};

QBufferPrivate::~QBufferPrivate()
{
    delete bufferEngine;
    bufferEngine = 0;
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

QBuffer::QBuffer() : QIODevice(*new QBufferPrivate)
{
    d->buf = &d->defaultBuf;
    setFlags(QIODevice::Direct);
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

QBuffer::QBuffer(QByteArray *a) : QIODevice(*new QBufferPrivate)
{
    d->buf = a;
    setFlags(QIODevice::Direct);
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
    if(d->bufferEngine)
        d->bufferEngine->setData(d->buf);
    d->defaultBuf.clear();
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
    if(d->bufferEngine)
        d->bufferEngine->setData(d->buf);
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
*/

QIOEngine *QBuffer::ioEngine() const
{
    if(!d->bufferEngine)
        d->bufferEngine = new QBufferEngine(d->buf);
    return d->bufferEngine;
}


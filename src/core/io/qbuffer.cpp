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

#include "qbuffer.h"
#include "private/qiodevice_p.h"
#ifndef QT_NO_QOBJECT
#include "qsignal.h"
#endif
#include "qtimer.h"

/** QBufferPrivate **/
class QBufferPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QBuffer)

public:
    QBufferPrivate()
        :
#ifndef QT_NO_QOBJECT
        signalsEmitted(false), writtenSinceLastEmit(0),
#endif
        ioIndex(0), isOpen(false), buf(0)  { }
    ~QBufferPrivate() { }

#ifndef QT_NO_QOBJECT
    // private slots
    void emitSignals();

    bool signalsEmitted;
    Q_LONGLONG writtenSinceLastEmit;
#endif

    Q_LONGLONG ioIndex;
    bool isOpen;
    QByteArray *buf;

    QByteArray defaultBuf;
};

#ifndef QT_NO_QOBJECT
void QBufferPrivate::emitSignals()
{
    Q_Q(QBuffer);
    emit q->bytesWritten(writtenSinceLastEmit);
    writtenSinceLastEmit = 0;
    emit q->readyRead();
    signalsEmitted = false;
}
#endif

/*!
    \class QBuffer
    \reentrant
    \brief The QBuffer class provides a QIODevice interface for a QByteArray.

    \ingroup io

    QBuffer is a wrapper for QByteArray which allows you to access an
    array using the QIODevice interface. The QByteArray is treated as
    a FIFO queue (First In, First Out), so the first data you write to
    the buffer is the first data you will read. Example:

    \code
    QBuffer buffer;

    if (buffer.open(QBuffer::ReadWrite)) {
        buffer.write("Qt rocks!");
        char c;
        buffer.getChar(&c); // 'Q'
        buffer.getChar(&c); // 't'
        buffer.getChar(&c); // ' '
        buffer.getChar(&c); // 'r'
    }
    \endcode

    By default, an internal QByteArray buffer is created for you when
    you construct a QBuffer. You can access this buffer directly by
    calling buffer(). You can also use QBuffer with an existing
    QByteArray by calling setBuffer(), or by passing your array to
    QBuffer's constructor.

    Call open() to open the buffer. Then call \l write() or \l
    putChar() to write to the buffer, and \l read(), \l readLine(), \l
    readAll() or \l getChar() to read from it. size() returns the
    current size of the buffer, and you can seek to arbitrary
    positions in the buffer by calling seek(). When you are done with
    accessing the buffer, call close().

    QBuffer emits \l readyRead() when new data has arrived in the
    buffer. By connecting to this signal, you can use QBuffer to store
    temporary data before processing it. For example, you can pass the
    buffer to QFtp when downloading a file from an FTP server.
    Whenever a new payload of data has been downloaded, your slot
    connected to \l readyRead() will be called. QBuffer also emits
    \l bytesWritten() every time new data has been written to the buffer.

    QBuffer can be used with QTextStream and QDataStream's stream
    operators (operator<<() and operator>>()).

    \sa QFile, QDataStream, QTextStream, QByteArray, \link shclass.html Shared Classes\endlink
*/


#ifdef QT_NO_QOBJECT
QBuffer::QBuffer()
    : QIODevice(*new QBufferPrivate)
{
    Q_D(QBuffer);
    d->buf = &d->defaultBuf;
}
QBuffer::QBuffer(QByteArray *a)
    : QIODevice(*new QBufferPrivate)
{
    Q_D(QBuffer);
    d->buf = a;
}
#else
/*!
    Constructs an empty buffer.
*/
QBuffer::QBuffer()
    : QIODevice(*new QBufferPrivate, 0)
{
    Q_D(QBuffer);
    d->buf = &d->defaultBuf;
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
    : QIODevice(*new QBufferPrivate, 0)
{
    Q_D(QBuffer);
    d->buf = a;
}

/*!
    Constructs an empty buffer with the parent \a parent.
*/
QBuffer::QBuffer(QObject *parent)
    : QIODevice(*new QBufferPrivate, parent)
{
    Q_D(QBuffer);
    d->buf = &d->defaultBuf;
}

/*!
    Constructs a buffer that operates on QByteArray \a a, with the
    parent \a parent.
*/
QBuffer::QBuffer(QByteArray *a, QObject *parent)
    : QIODevice(*new QBufferPrivate, parent)
{
    Q_D(QBuffer);
    d->buf = a;
}
#endif

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
    Q_D(QBuffer);
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
}

/*!
    Returns this buffer's byte array.

    \sa setBuffer()
*/

QByteArray &QBuffer::buffer()
{
    Q_D(const QBuffer);
    return *d->buf;
}

/*!
    Returns this buffer's byte array.

    \sa setBuffer()
*/

const QByteArray &QBuffer::buffer() const
{
    Q_D(const QBuffer);
    return *d->buf;
}

/*!
    \fn void QBuffer::setData(const char *data, int len)

    \overload

    Sets the data to be the first \a len bytes of the \a data string.
*/

/*!
    Returns this buffer's byte array.

    \sa setData()
*/

const QByteArray &QBuffer::data() const
{
    Q_D(const QBuffer);
    return *d->buf;
}

/*!
    Sets the byte array for the buffer to be \a data.

    Does nothing if isOpen() is true. Since \a data is const the buffer
    can only be used to read from it.
*/
void QBuffer::setData(const QByteArray &data)
{
    Q_D(QBuffer);
    if (isOpen()) {
        qWarning("QBuffer::setData: Buffer is open");
        return;
    }
    *d->buf = data;
}

/*!
   \reimp
*/
bool QBuffer::open(OpenMode flags)
{
    Q_D(QBuffer);

    if ((flags & Append) == Append)
        flags |= WriteOnly;
    setOpenMode(flags);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }

    if ((flags & QIODevice::Truncate) == QIODevice::Truncate)
        d->buf->resize(0);
    if ((flags & QIODevice::Append) == QIODevice::Append) // append to end of buffer
        d->ioIndex = Q_LONGLONG(d->buf->size());
    else
        d->ioIndex = 0;

    return true;
}

/*!
  \reimp
*/

void QBuffer::close()
{
    Q_D(QBuffer);
    if(!isOpen())
        return;
    d->isOpen = false;
    if(d->ioIndex == Q_LONGLONG(-1))
        return;
    d->ioIndex = Q_LONGLONG(-1);
    return;
}

/*!
  \reimp
*/

Q_LONGLONG QBuffer::pos() const
{
    Q_D(const QBuffer);
    if (!isOpen())
        return 0;
    return d->ioIndex;
}

/*!
  \reimp
*/

Q_LONGLONG QBuffer::size() const
{
    Q_D(const QBuffer);
    return Q_LONGLONG(d->buf->size());
}

/*!
  \reimp
*/

bool QBuffer::seek(Q_LONGLONG pos)
{
    Q_D(QBuffer);
    if (!isOpen()) {
        qWarning("QBuffer::seek: IODevice is not open");
        return false;
    }

    // #### maybe resize if not readonly?
    if (pos > Q_LONGLONG(d->buf->size())) {
        qWarning("QBuffer::seek: Index %lld out of range", pos);
        return false;
    }
    d->ioIndex = pos;
    return true;
}


/*!
  \reimp
*/
bool QBuffer::atEnd() const
{
    if (!isOpen()) {
        qWarning("QBuffer::atEnd: IODevice is not open");
        return false;
    }

    QBuffer *that = const_cast<QBuffer *>(this);
    return that->d_func()->ioIndex == Q_LONGLONG(that->d_func()->buf->size());
}

/*!
  \reimp
*/

Q_LONGLONG QBuffer::readData(char *data, Q_LONGLONG len)
{
    Q_D(QBuffer);
    if (d->ioIndex + len > Q_LONGLONG(d->buf->size())) {   // overflow
        if (d->ioIndex >= Q_LONGLONG(d->buf->size())) {
            return 0;
        } else {
            len = Q_LONGLONG(d->buf->size()) - d->ioIndex;
        }
    }
    memcpy(data, d->buf->constData() + int(d->ioIndex), int(len));
    d->ioIndex += len;
    return len;
}

/*!
  \reimp
*/
Q_LONGLONG QBuffer::writeData(const char *data, Q_LONGLONG len)
{
    Q_D(QBuffer);
    if (d->ioIndex + len > Q_LONGLONG(d->buf->size())) { // overflow
        d->buf->resize(int(d->ioIndex + len));
        if (Q_LONGLONG(d->buf->size()) != d->ioIndex + len) { // could not resize
            qWarning("QBuffer::writeData: Memory allocation error");
            return -1;
        }
    }

    memcpy(d->buf->data() + int(d->ioIndex), (uchar *)data, int(len));
    d->ioIndex += len;

#ifndef QT_NO_QOBJECT
    d->writtenSinceLastEmit += len;
    if (!d->signalsEmitted) {
        d->signalsEmitted = true;
        qInvokeMetaMember(this, "emitSignals", Qt::QueuedConnection);
    }
#endif
    return len;
}

#ifndef QT_NO_QOBJECT
#  define d d_func()
#  define q q_func()
#  include "moc_qbuffer.cpp"
#endif

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

    int ioIndex;
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
QBuffer::QBuffer()
    : QIODevice(*new QBufferPrivate, 0)
{
    Q_D(QBuffer);
    d->buf = &d->defaultBuf;
}
QBuffer::QBuffer(QByteArray *a)
    : QIODevice(*new QBufferPrivate, 0)
{
    Q_D(QBuffer);
    d->buf = a;
}
QBuffer::QBuffer(QObject *parent)
    : QIODevice(*new QBufferPrivate, parent)
{
    Q_D(QBuffer);
    d->buf = &d->defaultBuf;
}
QBuffer::QBuffer(QByteArray *a, QObject *parent)
    : QIODevice(*new QBufferPrivate, parent)
{
    Q_D(QBuffer);
    d->buf = a;
}
#endif

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
        d->ioIndex = d->buf->size();
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
    if(d->ioIndex == -1)
        return;
    d->ioIndex = -1;
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
    return (Q_LONGLONG)d->buf->size();
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
    if (pos > (Q_LONGLONG)d->buf->size()) {
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
    return that->d_func()->ioIndex == that->d_func()->buf->size();
}

/*!
  \reimp
*/

Q_LONGLONG QBuffer::readData(char *data, Q_LONGLONG len)
{
    Q_D(QBuffer);
    if (d->ioIndex + len > d->buf->size()) {   // overflow
        if ((int)d->ioIndex >= d->buf->size()) {
            return 0;
        } else {
            len = d->buf->size() - d->ioIndex;
        }
    }
    memcpy(data, d->buf->constData() + d->ioIndex, len);
    d->ioIndex += len;
    return len;
}

/*!
  \reimp
*/
Q_LONGLONG QBuffer::writeData(const char *data, Q_LONGLONG len)
{
    Q_D(QBuffer);
    if (d->ioIndex + len > d->buf->size()) {             // overflow
        d->buf->resize(d->ioIndex + len);
        if (d->buf->size() != (int)(d->ioIndex + len)) {           // could not resize
            qWarning("QBuffer::writeBlock: Memory allocation error");
            return -1;
        }
    }

    memcpy(d->buf->data() + d->ioIndex, (uchar *)data, len);
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

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

#define d d_func()
#define q q_func()

/** QBufferPrivate **/
class QBufferPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QBuffer)

public:
    QBufferPrivate() : ioIndex(0), buf(0)  { }
    ~QBufferPrivate() { }

    int ioIndex;
    QByteArray *buf;

    QByteArray defaultBuf;
};

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
    d->defaultBuf.clear();
}

/*!
    \fn void QBuffer::setData(const char *data, int len)

    \overload

    Sets the data to be the first \a len bytes of the \a data string.
*/

/*!
    Sets the byte array for the buffer to be \a data.

    Does nothing if isOpen() is true. Since \a data is const the buffer
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
   ### must document
*/
bool QBuffer::open(int mode)
{
    if(mode & (Append|WriteOnly)) //append implies write
        mode |= WriteOnly;
    setFlags(QIODevice::Direct);
    resetStatus();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }

    if (mode & QIODevice::Truncate) 
        d->buf->resize(0);
    if (mode & QIODevice::Append)                       // append to end of buffer
        d->ioIndex = d->buf->size();
    else
        d->ioIndex = 0;
    setMode(mode);
    setState(QIODevice::Open);
    return true;
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

void QBuffer::close()
{
    if(!isOpen())
        return;
    if(d->ioIndex == -1)
        return;
    d->ioIndex = -1;
    return;
}

/*!
  \reimp
*/

Q_LLONG QBuffer::at() const
{
    if (!isOpen())
        return 0;
    return d->ioIndex;
}

/*!
  \reimp
*/

Q_LLONG QBuffer::size() const
{
    return (Q_LLONG)d->buf->size();
}

/*!
  \reimp
*/

bool QBuffer::seek(Q_LLONG pos)
{
    if (!isOpen()) {
        qWarning("QBuffer::seek: IODevice is not open");
        return false;
    }

    // #### maybe resize if not readonly?
    if (pos > (Q_LLONG)d->buf->size()) {
        qWarning("QBuffer::seek: Index %lld out of range", pos);
        return false;
    }
    d->ioIndex = pos;
    return true;
}

/*!
  \reimp
*/

Q_LLONG QBuffer::read(char *data, Q_LLONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QIODevice::read: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QIODevice::read: Read operation not permitted");
        return -1;
    }
    resetStatus();

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

Q_LLONG QBuffer::write(const char *data, Q_LLONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {                                // file not open
        qWarning("QBuffer::write: File not open");
        return -1;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QBuffer::write: Write operation not permitted");
        return -1;
    }
    resetStatus();

    if (d->ioIndex + len > d->buf->size()) {             // overflow
        d->buf->resize(d->ioIndex + len);
        if (d->buf->size() != (int)(d->ioIndex + len)) {           // could not resize
            qWarning("QBuffer::writeBlock: Memory allocation error");
            return -1;
        }
    }
    memcpy(d->buf->data() + d->ioIndex, (uchar *)data, len);
    d->ioIndex += len;
    return len;
}

/*!
  \reimp
*/

int QBuffer::getch()
{
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::getch: Buffer not open" );
        return -1;
    }
    if ( !isReadable() ) {                      // reading not permitted
        qWarning( "QBuffer::getch: Read operation not permitted" );
        return -1;
    }

    if ( d->ioIndex+1 > d->buf->size() ) {               // overflow
        setStatus( IO_ReadError );
        return -1;
    }
    return uchar(*(d->buf->data()+d->ioIndex++));
}

/*!
  \reimp
*/

int QBuffer::putch(int character)
{
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::putch: Buffer not open" );
        return -1;
    }
    if ( !isWritable() ) {                      // writing not permitted
        qWarning( "QBuffer::putch: Write operation not permitted" );
        return -1;
    }

    if ( d->ioIndex + 1 > d->buf->size() ) {                // overflow
        char tmp = (char)character;
        if ( write(&tmp,1) != 1 )
            return -1;                          // write error
    } else {
        *(d->buf->data() + d->ioIndex++) = (char)character;
    }
    return character;

}

/*!
  \reimp
*/

int QBuffer::ungetch(int character)
{
    if ( !isOpen() ) {                          // buffer not open
        qWarning( "QBuffer::ungetch: Buffer not open" );
        return -1;
    }
    if ( !isReadable() ) {                      // reading not permitted
        qWarning( "QBuffer::ungetch: Read operation not permitted" );
        return -1;
    }

    if ( character != -1 ) {
        if ( d->ioIndex )
            d->ioIndex--;
        else
            character = -1;
    }
    return character;
}

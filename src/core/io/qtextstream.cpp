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

//#define QTEXTSTREAM_DEBUG

#include "qtextstream.h"

#ifndef QT_NO_TEXTCODEC
#include <qtextcodec.h>
#endif

#include <qregexp.h>
#include <qbuffer.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qchar.h>

#ifndef Q_OS_TEMP
#include <locale.h>
#endif

// for strtod()
#include <stdlib.h>

#if defined QTEXTSTREAM_DEBUG
#include <qstring.h>
#include <ctype.h>

/*
    Returns a human readable representation of the first \a len
    characters in \a data.
*/
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len; ++i) {
        char c = data[i];
        if (isprint(c)) {
            out += c;
        } else switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            QString tmp;
            tmp.sprintf("\\x%x", (unsigned int)(unsigned char)c);
            out += tmp.toLatin1();
        }
    }

    if (len < maxSize)
        out += "...";

    return out;
}
#endif

/*****************************************************************************
  QTextStream member functions
 *****************************************************************************/

static const int QTEXTSTREAM_BUFFERSIZE = 16384;

#define Q_VOID
#define CHECK_VALID_STREAM(x) do { \
    if (!d->string && !d->device) { \
        qWarning("QTextStream: No device"); \
        return x; \
    } } while (0)

#define IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(type) do { \
    Q_D(QTextStream); \
    CHECK_VALID_STREAM(*this); \
    qulonglong tmp; \
    i = d->getNumber(&tmp) ? (type)tmp : (type)0; \
    return *this; } while (0)

#define IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(type) do { \
    Q_D(QTextStream); \
    CHECK_VALID_STREAM(*this); \
    double tmp; \
    f = d->getReal(&tmp) ? (type)tmp : (type)0; \
    return *this; } while (0)

class QTextStreamPrivate
{
    Q_DECLARE_PUBLIC(QTextStream)
public:
    QTextStreamPrivate(QTextStream *q_ptr);
    ~QTextStreamPrivate();
    void reset();

    // device
    QIODevice *device;
    bool deleteDevice;

    // string
    QString *string;
    int stringOffset;
    QIODevice::OpenMode stringOpenMode;

#ifndef QT_NO_TEXTCODEC
    // codec
    QTextCodec *codec;
    QTextCodec::ConverterState readConverterState;
    QTextCodec::ConverterState writeConverterState;
    bool autoDetectUnicode;
#endif

    // i/o
    enum TokenDelimiter {
        NoDelimiter = 0,
        Not = 0x1000,
        Space = 0x1,
        NotSpace = Not | Space,
        EndOfLine = 0x2,
        EndOfFile = 0x4
    };

    bool scan(QChar **ptr, int *tokenLength,
              int maxlen, TokenDelimiter delimiter);
    inline QChar *readPtr();
    inline void consumeLastToken();
    inline void consume(int nchars);
    int lastTokenSize;

    bool write(const QString &data);
    bool getChar(QChar *ch);
    void ungetChar(const QChar &ch);
    bool getNumber(qulonglong *l);
    bool getReal(double *f);

    bool putNumber(qulonglong number, bool negative);
    bool putString(const QString &ch);

    // buffers
    bool fillReadBuffer();
    bool flushWriteBuffer();
    QString writeBuffer;
    QString readBuffer;
    int readBufferOffset;
    QString endOfBufferState;

    // streaming parameters
    int fieldFlags;
    int fieldWidth;
    QChar fillChar;
    int fieldPrecision;

    QTextStream *q_ptr;
};

QTextStreamPrivate::QTextStreamPrivate(QTextStream *q_ptr)
{
    this->q_ptr = q_ptr;
    reset();
}

QTextStreamPrivate::~QTextStreamPrivate()
{
    if (deleteDevice)
        delete device;
}

void QTextStreamPrivate::reset()
{
    fieldFlags = 0;
    fieldWidth = 0;
    fillChar = QLatin1Char(' ');
    fieldPrecision = 6;
    device = 0;
    deleteDevice = false;
    string = 0;
    stringOffset = 0;
    stringOpenMode = QIODevice::NotOpen;
#ifndef QT_NO_TEXTCODEC
    codec = QTextCodec::codecForLocale();
    readConverterState = QTextCodec::ConverterState();
    writeConverterState = QTextCodec::ConverterState();
    writeConverterState.flags |= QTextCodec::IgnoreHeader;
    autoDetectUnicode = true;
#endif
    readBufferOffset = 0;
    endOfBufferState.clear();
    lastTokenSize = 0;
}

bool QTextStreamPrivate::fillReadBuffer()
{
    // no buffer next to the QString itself; this function should only
    // be called internally, for devices.
    Q_ASSERT(!string);
    Q_ASSERT(device);

    // handle text translation and bypass the Text flag in the device.
    bool textModeEnabled = device->isTextModeEnabled();
    if (textModeEnabled)
        device->setTextModeEnabled(false);

    // read raw data into a temporary buffer
    char buf[QTEXTSTREAM_BUFFERSIZE];
    qint64 bytesRead = device->read(buf, sizeof(buf));
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::fillReadBuffer(), device->read(\"%s\", %d) == %d",
           qt_prettyDebug(buf, qMin(32,int(bytesRead)) , int(bytesRead)).constData(), sizeof(buf), int(bytesRead));
#endif
    if (bytesRead <= 0)
        return false;

#ifndef QT_NO_TEXTCODEC
    // codec auto detection, explicitly defaults to locale encoding if
    // the codec has been set to 0.
    if (!codec || autoDetectUnicode) {
        autoDetectUnicode = false;
        if (bytesRead >= 2 && (uchar(buf[0]) == 0xff && uchar(buf[1]) == 0xfe
                               || uchar(buf[0]) == 0xfe && uchar(buf[1]) == 0xff)) {
            codec = QTextCodec::codecForName("UTF-16");
        } else if (!codec) {
            codec = QTextCodec::codecForLocale();
            writeConverterState.flags |= QTextCodec::IgnoreHeader;
        }
    }
#endif

    readBuffer += endOfBufferState;
#ifndef QT_NO_TEXTCODEC
    // convert to unicode
    readBuffer += codec->toUnicode(buf, bytesRead, &readConverterState);
#else
    readBuffer += QString(QByteArray(buf, bytesRead));
#endif

    // reset the Text flag.
    if (textModeEnabled) {
        device->setTextModeEnabled(true);
        readBuffer.replace(QLatin1String("\r\n"), QLatin1String("\n"));
        if (readBuffer.endsWith(QLatin1String("\r")) && !device->atEnd()) {
            endOfBufferState = QLatin1String("\r");
            readBuffer.chop(1);
        } else {
            endOfBufferState.clear();
        }
    }

#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::fillReadBuffer() read %d bytes from device", int(bytesRead));
#endif
    return true;
}

bool QTextStreamPrivate::flushWriteBuffer()
{
    // no buffer next to the QString itself; this function should only
    // be called internally, for devices.
    if (string || !device)
        return false;
    if (writeBuffer.isEmpty())
        return true;

#if defined (Q_OS_WIN)
    // handle text translation and bypass the Text flag in the device.
    bool textModeEnabled = device->isTextModeEnabled();
    if (textModeEnabled) {
        device->setTextModeEnabled(false);
        writeBuffer.replace(QLatin1Char('\n'), QLatin1String("\r\n"));
    }
#endif

#ifndef QT_NO_TEXTCODEC
    if (!codec)
        codec = QTextCodec::codecForLocale();

    // convert from unicode to raw data
    QByteArray data = codec->fromUnicode(writeBuffer.data(), writeBuffer.size(), &writeConverterState);
#else
    QByteArray data = writeBuffer.toLocal8Bit();
#endif
    writeBuffer.clear();

    // write raw data to the device
    qint64 bytesWritten = device->write(data);
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::flushWriteBuffer(), device->write(\"%s\") == %d",
           qt_prettyDebug(data.constData(), qMin(data.size(),32), data.size()).constData(), int(bytesWritten));
#endif
    if (bytesWritten <= 0)
        return false;

#if defined (Q_OS_WIN)
    // replace the text flag
    if (textModeEnabled)
        device->setTextModeEnabled(true);
#endif

    // flush the device
    bool flushed = device->flush();

#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::flushWriteBuffer() wrote %d bytes",
           int(bytesWritten));
#endif
    return flushed && bytesWritten == qint64(data.size());
}

bool QTextStreamPrivate::scan(QChar **ptr, int *length, int maxlen, TokenDelimiter delimiter)
{
    int totalSize = 0;
    int delimSize = 0;
    bool consumeDelimiter = false;
    QChar lastChar = QLatin1Char('\0');

    bool foundToken = false;
    int startOffset = device ? readBufferOffset : stringOffset;
    do {
        int endOffset = device ? readBuffer.size() : string->size();

        for (; !foundToken && (!maxlen || totalSize+1 < maxlen) && startOffset < endOffset; ++startOffset) {
            QChar ch = device ? readBuffer.at(startOffset) : string->at(startOffset);
            ++totalSize;

            switch (delimiter) {
            case EndOfLine:
                if (ch == QLatin1Char('\n')) {
                    foundToken = true;
                    consumeDelimiter = true;
                    delimSize = (lastChar == QLatin1Char('\r')) ? 2 : 1;
                }
                break;
            case Space:
                if (ch.isSpace()) {
                    foundToken = true;
                    delimSize = 1;
                }
                break;
            case NotSpace:
                if (!ch.isSpace()) {
                    foundToken = true;
                    delimSize = 1;
                }
                break;
            default:
                break;
            }

            lastChar = ch;
        }

    } while (!foundToken && (!maxlen || totalSize < maxlen) && (device && fillReadBuffer()));

    if (!foundToken) {
        if ((maxlen && totalSize < maxlen)
            || (string && stringOffset + totalSize < string->size())
            || (device && !device->atEnd())) {
#if defined (QTEXTSTREAM_DEBUG)
            qDebug("QTextStreamPrivate::scan() did not find the token.");
#endif
            return false;
        }
    }

    // if we find a '\r' at the end of the data when reading lines,
    // don't make it part of the line.
    if (totalSize > 0 && !foundToken && delimiter == EndOfLine) {
        if (((string && stringOffset + totalSize == string->size()) || (device && device->atEnd()))
            && lastChar == QLatin1Char('\r')) {
            consumeDelimiter = true;
            ++delimSize;
        }
    }

    // set the read offset and length of the token
    if (length)
        *length = totalSize - delimSize;
    if (ptr)
        *ptr = readPtr();

    // update last token size
    lastTokenSize = totalSize;
    if (!consumeDelimiter)
        lastTokenSize -= delimSize;

#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::scan(%p, %p, %d, %x) token length = %d, delimiter = %d",
           ptr, length, maxlen, (int)delimiter, totalSize - delimSize, delimSize);
#endif
    return true;
}

inline QChar *QTextStreamPrivate::readPtr()
{
    Q_ASSERT(readBufferOffset <= readBuffer.size());
    if (string)
        return string->data() + stringOffset;
    return readBuffer.data() + readBufferOffset;
}

inline void QTextStreamPrivate::consumeLastToken()
{
    if (lastTokenSize)
        consume(lastTokenSize);
    lastTokenSize = 0;
}

inline void QTextStreamPrivate::consume(int size)
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::consume(%d)", size);
#endif
    if (string) {
        stringOffset += size;
        if (stringOffset > string->size())
            stringOffset = string->size();
    } else {
        readBufferOffset += size;
        if (readBufferOffset >= readBuffer.size()) {
            readBufferOffset = 0;
            readBuffer.clear();
        }
    }
}

bool QTextStreamPrivate::write(const QString &data)
{
    if (string) {
        string->append(data);
        return true;
    }
    writeBuffer += data;
    if (writeBuffer.size() > QTEXTSTREAM_BUFFERSIZE)
        return flushWriteBuffer();
    return true;
}

bool QTextStreamPrivate::getChar(QChar *ch)
{
    if ((string && stringOffset == string->size()) || (device && readBuffer.isEmpty() && !fillReadBuffer())) {
        if (ch)
            *ch = 0;
        return false;
    }
    if (ch)
        *ch = *readPtr();
    consume(1);
    return true;
}

void QTextStreamPrivate::ungetChar(const QChar &ch)
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::ungetChar(%x)", ch.unicode());
#endif
    if (string) {
        if (stringOffset == 0)
            string->prepend(ch);
        else
            (*string)[--stringOffset] = ch;
        return;
    }

    if (readBufferOffset == 0) {
        readBuffer.prepend(ch);
        return;
    }

    readBuffer[--readBufferOffset] = ch;
}

bool QTextStreamPrivate::putString(const QString &s)
{
    QString tmp = s;

    // handle padding
    int padSize = fieldWidth - s.size();
    if (padSize > 0) {
        QString pad(padSize > 0 ? padSize : 0, fillChar);
        if (fieldFlags & QTextStream::left)
            tmp.append(pad);
        else
            tmp.prepend(pad);
    }

#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::putString(\"%s\") calls write(\"%s\")",
           s.toLatin1().constData(), tmp.toLatin1().constData());
#endif
    return write(tmp);
}

QTextStream::QTextStream()
    : d_ptr(new QTextStreamPrivate(this))
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStream::QTextStream()");
#endif
}

/*!
    Constructs a text stream that uses the IO device \a device.
*/

QTextStream::QTextStream(QIODevice *device)
    : d_ptr(new QTextStreamPrivate(this))
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStream::QTextStream(QIODevice *device == *%p)",
           device);
#endif
    Q_D(QTextStream);
    d->device = device;
}

/*!
    \fn QTextStream::QTextStream(QString *str, QIODevice::OpenMode mode)

    Constructs a text stream that operates on the Unicode QString, \a
    str, through an internal device. The \a mode argument is passed
    to the device's open() function; see \l{QIODevice::mode()}.

    If you set an encoding or codec with setEncoding() or setCodec(),
    this setting is ignored for text streams that operate on QString.

    Example:
    \code
        QString str;
        QTextStream ts(&str, QIODevice::WriteOnly);
        ts << "pi = " << 3.14; // str == "pi = 3.14"
    \endcode

    Writing data to the text stream will modify the contents of the
    string. The string will be expanded when data is written beyond
    the end of the string. Note that the string will not be truncated:
    \code
        QString str = "pi = 3.14";
        QTextStream ts(&str, QIODevice::WriteOnly);
        ts <<  "2+2 = " << 2+2; // str == "2+2 = 414"
    \endcode

    Note that because QString is Unicode, you should not use
    readRawBytes() or writeRawBytes() on such a stream.
*/

QTextStream::QTextStream(QString *string, QIODevice::OpenMode openMode)
    : d_ptr(new QTextStreamPrivate(this))
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStream::QTextStream(QString *string == *%p, openMode = %d)",
           string, int(openMode));
#endif
    Q_D(QTextStream);
    d->string = string;
    d->stringOpenMode = openMode;
}

/*!
    \fn QTextStream::QTextStream(QByteArray *a, QIODevice::OpenMode mode)

    Constructs a text stream that operates on a byte array, \a a. The
    \a mode is a QIODevice::mode(), usually either \c QIODevice::ReadOnly or
    \c QIODevice::WriteOnly. Use QTextStream(const QByteArray&, int) if you
    just want to read from a byte array.

    Since QByteArray is not a QIODevice subclass, internally a QBuffer
    is created to wrap the byte array.

    Example:
    \code
        QByteArray array;
        QTextStream ts(array, QIODevice::WriteOnly);
        ts << "pi = " << 3.14 << '\0';
        // array == "pi = 3.14"
    \endcode

    Writing data to the text stream will modify the contents of the
    byte array. The byte array will be expanded when data is written
    beyond the end of the string.

    Same example, using a QBuffer:
    \code
        QByteArray array;
        QBuffer buf(array);
        buf.open(QIODevice::WriteOnly);
        QTextStream ts(&buf);
        ts << "pi = " << 3.14 << '\0'; // array == "pi = 3.14"
        buf.close();
    \endcode
*/

QTextStream::QTextStream(QByteArray *array, QIODevice::OpenMode openMode)
    : d_ptr(new QTextStreamPrivate(this))
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStream::QTextStream(QByteArray *array == *%p, openMode = %d)",
           array, int(openMode));
#endif
    Q_D(QTextStream);
    d->device = new QBuffer(array);
    d->device->open(openMode);
    d->deleteDevice = true;
}

/*!
    \fn QTextStream::QTextStream(const QByteArray &a, QIODevice::OpenMode mode)

    Constructs a text stream that operates on byte array \a a. Since
    the byte array is passed as a const it can only be read from; use
    QTextStream(QByteArray*, int) if you want to write to a byte
    array. The \a mode is a QIODevice::mode() and should normally be
    \c QIODevice::ReadOnly.

    Since QByteArray is not a QIODevice subclass, internally a QBuffer
    is created to wrap the byte array.
*/
QTextStream::QTextStream(const QByteArray &array, QIODevice::OpenMode openMode)
    : d_ptr(new QTextStreamPrivate(this))
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStream::QTextStream(const QByteArray &array == *(%p), openMode = %d)",
           &array, int(openMode));
#endif
    QBuffer *buffer = new QBuffer;
    buffer->setData(array);
    buffer->open(openMode);

    Q_D(QTextStream);
    d->device = buffer;
    d->deleteDevice = true;
}

/*!
    \fn QTextStream::QTextStream(FILE *fh, QIODevice::OpenMode mode)

    Constructs a text stream that operates on an existing file handle
    \a fh, through an internal QFile device. The \a mode argument is
    passed to the device's open() function; see \l{QIODevice::mode()}.

    \warning If you create a QTextStream \c cout or another name that
    is also used for another variable of a different type, some
    linkers may confuse the two variables, which will often cause
    crashes.
*/

QTextStream::QTextStream(FILE *fileHandle, QIODevice::OpenMode openMode)
    : d_ptr(new QTextStreamPrivate(this))
{
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStream::QTextStream(FILE *fileHandle = %p, openMode = %d)",
           fileHandle, int(openMode));
#endif
    QFile *file = new QFile;
    file->open(openMode, fileHandle);

    Q_D(QTextStream);
    d->device = file;
    d->deleteDevice = true;
}

/*!
    Destroys the text stream.

    The destructor does not affect the current IO device.
*/

QTextStream::~QTextStream()
{
    Q_D(QTextStream);
#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStream::~QTextStream()");
#endif
    if (!d->writeBuffer.isEmpty())
        d->flushWriteBuffer();
}

/*!
    Resets the text stream.

    \list
    \i All flags are set to 0.
    \i The field width is set to 0.
    \i The fill character is set to ' ' (Space).
    \i The precision is set to 6.
    \endlist

    \sa setf(), width(), fill(), precision()
*/

void QTextStream::reset()
{
    Q_D(QTextStream);
    d->fieldFlags = 0;
    d->fieldWidth = 0;
    d->fillChar = QLatin1Char(' ');
    d->fieldPrecision = 6;
}

void QTextStream::flush()
{
    Q_D(QTextStream);
    d->flushWriteBuffer();
}

void QTextStream::skipWhiteSpace()
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(Q_VOID);
    d->scan(0, 0, 0, QTextStreamPrivate::NotSpace);
    d->consumeLastToken();
}

/*!
    Returns the IO device currently set.

    \sa setDevice(), unsetDevice()
*/

QIODevice *QTextStream::device() const
{
    Q_D(const QTextStream);
    return d->device;
}


/*!
    Sets the IO device to \a iod.

    \sa device(), unsetDevice()
*/

void QTextStream::setDevice(QIODevice *device)
{
    Q_D(QTextStream);
    flush();
    if (d->deleteDevice) {
        delete d->device;
        d->deleteDevice = false;
    }
    d->device = device;
}

QString *QTextStream::string() const
{
    Q_D(const QTextStream);
    return d->string;
}

void QTextStream::setString(QString *string)
{
    Q_D(QTextStream);
    flush();
    d->string = string;
}

/*!
    Returns the current stream flags. The default value is 0.

    \table
    \header \i Flag \i Meaning
    \row \i \c skipws \i Not currently used; whitespace is always skipped
    \row \i \c left \i Numeric fields are left-aligned
    \row \i \c right
         \i Not currently used (by default, numerics are right-aligned)
    \row \i \c internal \i Puts any padding spaces between +/- and value
    \row \i \c bin \i Output \e and input only in binary
    \row \i \c oct \i Output \e and input only in octal
    \row \i \c dec \i Output \e and input only in decimal
    \row \i \c hex \i Output \e and input only in hexadecimal
    \row \i \c showbase
         \i Annotates numeric outputs with 0b, 0, or 0x if in \c bin,
         \c oct, or \c hex format
    \row \i \c showpoint \i Not currently used
    \row \i \c uppercase \i Uses 0B and 0X rather than 0b and 0x
    \row \i \c showpos \i Shows + for positive numeric values
    \row \i \c scientific \i Uses scientific notation for floating point values
    \row \i \c fixed \i Uses fixed-point notation for floating point values
    \endtable

    Note that unless \c bin, \c oct, \c dec, or \c hex is set, the
    input base is octal if the value starts with 0, hexadecimal if it
    starts with 0x, binary if it starts with 0b, and decimal
    otherwise.

    \sa setf(), unsetf()
*/

int QTextStream::flags() const
{
    Q_D(const QTextStream);
    return d->fieldFlags;
}


/*!
    Sets the stream flags to \a f.

    \sa setf(), unsetf(), flags()
*/

void QTextStream::setFlags(int flags)
{
    Q_D(QTextStream);
    d->fieldFlags = flags;
}

/*!
    \fn int QTextStream::flags(int f)

    Use setFlags and flags() instead.
*/

/*!
    Sets the stream flag bits \a bits. Returns the previous stream
    flags.

    Equivalent to \c{setFlags(flags() | bits)}.

    \sa setf(), unsetf()
*/

int QTextStream::setf(int bits)
{
    Q_D(QTextStream);
    int oldf = d->fieldFlags;
    d->fieldFlags |= bits;
    return oldf;
}


/*!
    \overload

    Sets the stream flag bits \a bits with a bit mask \a mask. Returns
    the previous stream flags.

    Equivalent to \c{setFlags((flags() & ~mask) | (bits & mask))}.

    \sa setf(), unsetf()
*/

int QTextStream::setf(int bits, int mask)
{
    Q_D(QTextStream);
    int oldf = d->fieldFlags;
    d->fieldFlags = (d->fieldFlags & ~mask) | (bits & mask);
    return oldf;
}


/*!
    Clears the stream flag bits \a bits. Returns the previous stream
    flags.

    Equivalent to \c{setFlags(flags() & ~mask)}.

    \sa setf()
*/

int QTextStream::unsetf(int bits)
{
    Q_D(QTextStream);
    int oldf = d->fieldFlags;
    d->fieldFlags &= ~bits;
    return oldf;
}


/*!
    Returns the field width. The default value is 0.
*/

int QTextStream::width() const
{
    Q_D(const QTextStream);
    return d->fieldWidth;
}


/*!
    Sets the field width to \a w.
*/

void QTextStream::setWidth(int w)
{
    Q_D(QTextStream);
    d->fieldWidth = w;
}

/*!
    \fn int QTextStream::width(int w)

    Use setWidth() and width() instead.
*/

/*!
    Returns the fill character. The default value is ' ' (space).
*/

int QTextStream::fill() const
{
    Q_D(const QTextStream);
    return d->fillChar.unicode();
}

/*!
    Sets the fill character to \a ch.
*/
void QTextStream::setFill(int ch)
{
    Q_D(QTextStream);
    d->fillChar = QChar(ch);
}

/*!
    \fn int QTextStream::fill(int f)

    Use setFill() and fill() instead.
*/

/*!
    Returns the precision. The default value is 6.
*/
int QTextStream::precision() const
{
    Q_D(const QTextStream);
    return d->fieldPrecision;
}

/*!
    Sets the precision to \a p.
*/
void QTextStream::setPrecision(int precision)
{
    Q_D(QTextStream);
    d->fieldPrecision = precision;
}

/*!
    \fn bool QTextStream::atEnd() const

    Returns true if the IO device has reached the end position (end of
    the stream or file) or if there is no IO device set; otherwise
    returns false.

    \sa QIODevice::atEnd()
*/
bool QTextStream::atEnd() const
{
    Q_D(const QTextStream);
    CHECK_VALID_STREAM(true);

    if (d->string)
        return d->string->size() == d->stringOffset;
    return d->readBuffer.isEmpty() && d->device->atEnd();
}

/*****************************************************************************
  QTextStream read functions
 *****************************************************************************/

QString QTextStream::readAll()
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(QLatin1String(""));

    QChar *readPtr;
    int length;
    if (!d->scan(&readPtr, &length, /* maxlen = */ 0, QTextStreamPrivate::EndOfFile))
        return QLatin1String("");

    QString tmp = QString(readPtr, length);
    d->consumeLastToken();
    return tmp;
}

QString QTextStream::readLine(qint64 maxlen)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(QLatin1String(""));

    QChar *readPtr;
    int length;
    if (!d->scan(&readPtr, &length, int(maxlen), QTextStreamPrivate::EndOfLine))
        return QLatin1String("");

    QString tmp = QString(readPtr, length);
    d->consumeLastToken();
    return tmp;
}


bool QTextStreamPrivate::getNumber(qulonglong *ret)
{
    Q_Q(QTextStream);

    scan(0, 0, 0, NotSpace);
    consumeLastToken();

    // detect int encoding
    int fieldEncoding = fieldFlags & q->basefield;
    if (fieldEncoding == 0) {
        QChar ch;
        if (!getChar(&ch))
            return false;
        if (ch == QLatin1Char('0')) {
            QChar ch2;
            if (!getChar(&ch2)) {
                ungetChar(ch);
                return false;
            }
            ch2 = ch2.toLower();

            if (ch2 == QLatin1Char('x')) {
                fieldEncoding = QTextStream::hex;
            } else if (ch2 == QLatin1Char('b')) {
                fieldEncoding = QTextStream::bin;
            } else if (ch2.isDigit() && ch2.digitValue() >= 0 && ch2.digitValue() <= 7) {
                fieldEncoding = QTextStream::oct;
            } else {
                fieldEncoding = QTextStream::dec;
            }
            ungetChar(ch2);
        } else if (ch == QLatin1Char('-') || ch == QLatin1Char('+') || ch.isDigit()) {
            fieldEncoding = QTextStream::dec;
        } else {
            ungetChar(ch);
            return false;
        }
        ungetChar(ch);
    }

    qulonglong val=0;
    switch (fieldEncoding) {
    case QTextStream::bin:
    {
        QChar tmp;
        if (!getChar(&tmp) || tmp != QLatin1Char('0'))
            return false;
        if (!getChar(&tmp) || tmp.toLower() != QLatin1Char('b'))
            return false;
        while (getChar(&tmp)) {
            int n = tmp.toLower().unicode();
            if (n == '0' || n == '1') {
                val <<= 1;
                val += n - '0';
            } else {
                ungetChar(tmp);
                break;
            }
        }
        break;
    }
    case QTextStream::oct:
    {
        QChar tmp;
        if (!getChar(&tmp) || tmp != QLatin1Char('0'))
            return false;
        while (getChar(&tmp)) {
            int n = tmp.toLower().unicode();
            if (n >= '0' && n <= '7') {
                val *= 8;
                val += n - '0';
            } else {
                ungetChar(tmp);
                break;
            }
        }
        break;
    }
    case QTextStream::dec:
    {
        QChar sign;
        if (!getChar(&sign))
            return false;
        if (sign != QLatin1Char('-') && sign != QLatin1Char('+')) {
            if (!sign.isDigit())
                return false;
            val += sign.digitValue();
        }
        QChar ch;
        while (getChar(&ch)) {
            if (ch.isDigit()) {
                val *= 10;
                val += ch.digitValue();
            } else {
                ungetChar(ch);
                break;
            }
        }
        if (sign == QLatin1Char('-'))
            val = -val;
        break;
    }
    case QTextStream::hex:
    {
        QChar tmp;
        if (!getChar(&tmp) || tmp != QLatin1Char('0'))
            return false;
        if (!getChar(&tmp) || tmp.toLower() != QLatin1Char('x'))
            return false;
        while (getChar(&tmp)) {
            int n = tmp.toLower().unicode();
            if (n >= '0' && n <= '9') {
                val <<= 4;
                val += n - '0';
            } else if (n >= 'a' && n <= 'f') {
                val <<= 4;
                val += 10 + (n - 'a');
            } else {
                ungetChar(tmp);
                break;
            }
        }
        break;
    }
    default:
        return false;
    }

    if (ret)
        *ret = val;
    return true;
}

bool QTextStreamPrivate::getReal(double *f)
{
    // We use a table-driven FSM to parse floating point numbers
    // strtod() cannot be used directly since we may be reading from a
    // QIODevice.
    enum ParserState {
        Init = 0,
        Sign = 1,
        Mantissa = 2,
        Dot = 3,
        Abscissa = 4,
        ExpMark = 5,
        ExpSign = 6,
        Exponent = 7,
        Done = 8
    };
    enum InputToken {
        None = 0,
        InputSign = 1,
        InputDigit = 2,
        InputDot = 3,
        InputExp = 4
    };

    static uchar table[8][5] = {
        // None InputSign InputDigit InputDot InputExp
        { 0,    Sign,     Mantissa,  Dot,     0        }, // Init
        { 0,    0,        Mantissa,  Dot,     0        }, // Sign
        { Done, Done,     Mantissa,  Dot,     ExpMark  }, // Mantissa
        { 0,    0,        Abscissa,  0,       0        }, // Dot
        { Done, Done,     Abscissa,  Done,    ExpMark  }, // Abscissa
        { 0,    ExpSign,  Exponent,  0,       0        }, // ExpMark
        { 0,    0,        Exponent,  0,       0        }, // ExpSign
        { Done, Done,     Exponent,  Done,    Done     }  // Exponent
    };

    ParserState state = Init;
    InputToken input = None;

    scan(0, 0, 0, NotSpace);
    consumeLastToken();

    const int BufferSize = 128;
    char buf[BufferSize];
    int i = 0;

    QChar c;
    while (getChar(&c)) {
        switch (c.unicode()) {
        case '+':
        case '-':
            input = InputSign;
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            input = InputDigit;
            break;
        case '.':
            input = InputDot;
            break;
        case 'e':
        case 'E':
            input = InputExp;
            break;
        default:
            input = None;
            break;
        }

        state = ParserState(table[state][input]);

        if  (state == Init || state == Done || i > (BufferSize - 5)) {
            if (i > (BufferSize - 5)) { // ignore rest of digits
                while (getChar(&c)) {
                    if (!c.isDigit()) {
                        ungetChar(c);
                        break;
                    }
                }
            }
            break;
        }

        buf[i++] = c.toLatin1();
    }
    buf[i] = '\0';

    if (f)
        *f = strtod(buf, 0);
    return true;
}

/*!
    \overload

    Reads a char \a c from the stream and returns a reference to the
    stream. Note that whitespace is skipped.
*/

QTextStream &QTextStream::operator>>(char &c)
{
    QChar ch;
    *this >> ch;
    c = ch.toLatin1();
    return *this;
}

/*!
    Reads a char \a c from the stream and returns a reference to the
    stream. Note that whitespace is \e not skipped.
*/

QTextStream &QTextStream::operator>>(QChar &c)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->scan(0, 0, 0, QTextStreamPrivate::NotSpace);
    d->getChar(&c);
    return *this;
}

/*!
    \overload

    Reads a signed \c short integer \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(signed short &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed short);
}


/*!
    \overload

    Reads an unsigned \c short integer \a i from the stream and
    returns a reference to the stream. See flags() for an explanation
    of the expected input format.
*/

QTextStream &QTextStream::operator>>(unsigned short &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned short);
}


/*!
    \overload

    Reads a signed \c int \a i from the stream and returns a reference
    to the stream. See flags() for an explanation of the expected
    input format.
*/

QTextStream &QTextStream::operator>>(signed int &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed int);
}


/*!
    \overload

    Reads an unsigned \c int \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(unsigned int &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned int);
}


/*!
    \overload

    Reads a signed \c long int \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(signed long &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed long);
}


/*!
    \overload

    Reads an unsigned \c long int \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(unsigned long &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned long);
}


QTextStream &QTextStream::operator>>(qlonglong &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(qlonglong);
}

QTextStream &QTextStream::operator>>(qulonglong &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(qulonglong);
}


/*!
    \overload

    Reads a \c float \a f from the stream and returns a reference to
    the stream. See flags() for an explanation of the expected input
    format.
*/

QTextStream &QTextStream::operator>>(float &f)
{
    IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(float);
}


/*!
    \overload

    Reads a \c double \a f from the stream and returns a reference to
    the stream. See flags() for an explanation of the expected input
    format.
*/

QTextStream &QTextStream::operator>>(double &f)
{
    IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(double);
}

/*!
    \overload

    Reads a "word" from the stream into \a s and returns a reference
    to the stream.

    A word consists of characters for which isspace() returns false.
*/

QTextStream &QTextStream::operator>>(char *c)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->scan(0, 0, 0, QTextStreamPrivate::NotSpace);
    d->consumeLastToken();

    QChar *ptr;
    int length;
    if (!d->scan(&ptr, &length, 0, QTextStreamPrivate::Space))
        return *this;

    for (int i = 0; i < length; ++i)
        *c++ = ptr[i].toLatin1();
    *c = '\0';
    d->consumeLastToken();
    return *this;
}

/*!
    \overload

    Reads a "word" from the stream into \a str and returns a reference
    to the stream.

    A word consists of characters for which isspace() returns false.
*/

QTextStream &QTextStream::operator>>(QString &str)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);

    str.clear();
    d->scan(0, 0, 0, QTextStreamPrivate::NotSpace);
    d->consumeLastToken();

    QChar *ptr;
    int length;
    if (!d->scan(&ptr, &length, 0, QTextStreamPrivate::Space))
        return *this;

    str = QString(ptr, length);
    d->consumeLastToken();
    return *this;
}

/*!
    \overload

    Reads a "word" from the stream into \a str and returns a reference
    to the stream.

    A word consists of characters for which isspace() returns false.
*/

QTextStream &QTextStream::operator>>(QByteArray &array)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);

    array.clear();
    d->scan(0, 0, 0, QTextStreamPrivate::NotSpace);
    d->consumeLastToken();

    QChar *ptr;
    int length;
    if (!d->scan(&ptr, &length, 0, QTextStreamPrivate::Space))
        return *this;

    for (int i = 0; i < length; ++i)
        array += ptr[i].toLatin1();

    d->consumeLastToken();
    return *this;
}

/*****************************************************************************
  QTextStream write functions
 *****************************************************************************/

bool QTextStreamPrivate::putNumber(qulonglong number, bool negative)
{
    QString tmp;
    switch (fieldFlags & QTextStream::basefield) {
    case QTextStream::bin:
        tmp = (fieldFlags & QTextStream::uppercase) ? "0B" : "0b";
        if (negative)
            tmp.prepend(QLatin1Char('-'));
        break;
    case QTextStream::hex: {
        // ### optim
        QString format;
        if (fieldFlags & QTextStream::showbase)
            format += "0x";
        format += QLatin1String("%ll");
        format += (fieldFlags & QTextStream::uppercase) ? "X" : "x";

        tmp.sprintf(format.toLatin1().constData(), number);
        break;
    }
    case QTextStream::oct:
        tmp.sprintf("0%llo", number);
        if (negative)
            tmp.prepend(QLatin1Char('-'));
        break;
    case QTextStream::dec:
    default:
        tmp.sprintf("%llu", number);
        break;
    }

    if (negative)
        tmp.prepend('-');
    else if (fieldFlags & QTextStream::showpos)
        tmp.prepend(QLatin1Char('+'));

    return putString(tmp);
}

/*!
    Writes character \c char to the stream and returns a reference to
    the stream.

    The character \a c is assumed to be Latin1 encoded independent of
    the Encoding set for the QTextStream.
*/
QTextStream &QTextStream::operator<<(QChar c)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(QString(c));
    return *this;
}

/*!
    \overload

    Writes character \a c to the stream and returns a reference to the
    stream.
*/
QTextStream &QTextStream::operator<<(char c)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(QString(QChar(c)));
    return *this;
}

/*!
    \overload

    Writes a \c short integer \a i to the stream and returns a
    reference to the stream.
*/

QTextStream &QTextStream::operator<<(signed short i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber((qulonglong)qAbs(qlonglong(i)), i < 0);
    return *this;
}


/*!
    \overload

    Writes an \c unsigned \c short integer \a i to the stream and
    returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<(unsigned short i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber((qulonglong)i, false);
    return *this;
}


/*!
    \overload

    Writes an \c int \a i to the stream and returns a reference to the
    stream.
*/

QTextStream &QTextStream::operator<<(signed int i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber((qulonglong)qAbs(qlonglong(i)), i < 0);
    return *this;
}


/*!
    \overload

    Writes an \c unsigned \c int \a i to the stream and returns a
    reference to the stream.
*/

QTextStream &QTextStream::operator<<(unsigned int i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber((qulonglong)i, false);
    return *this;
}


/*!
    \overload

    Writes a \c long \c int \a i to the stream and returns a reference
    to the stream.
*/

QTextStream &QTextStream::operator<<(signed long i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber((qulonglong)qAbs(qlonglong(i)), i < 0);
    return *this;
}


/*!
    \overload

    Writes an \c unsigned \c long \c int \a i to the stream and
    returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<(unsigned long i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber((qulonglong)i, false);
    return *this;
}


/*!
    \overload

    Writes a \c long \c int \a i to the stream and returns a reference
    to the stream.
*/

QTextStream &QTextStream::operator<<(qlonglong i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber((qulonglong)qAbs(i), i < 0);
    return *this;
}


/*!
    \overload

    Writes an \c unsigned \c long \c int \a i to the stream and
    returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<(qulonglong i)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(i, false);
    return *this;
}


/*!
    \overload

    Writes a \c float \a f to the stream and returns a reference to
    the stream.
*/

QTextStream &QTextStream::operator<<(float f)
{
    return *this << double(f);
}

/*!
    \overload

    Writes a \c double \a f to the stream and returns a reference to
    the stream.
*/

QTextStream &QTextStream::operator<<(double f)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);

    char f_char;
    char format[16];
    if ((d->fieldFlags & floatfield) == fixed)
        f_char = 'f';
    else if ((d->fieldFlags & floatfield) == scientific)
        f_char = (d->fieldFlags & uppercase) ? 'E' : 'e';
    else
        f_char = (d->fieldFlags & uppercase) ? 'G' : 'g';

    // generate format string
    register char *fs = format;

    // "%.<prec>l<f_char>"
    *fs++ = '%';
    *fs++ = '.';
    int prec = d->fieldPrecision;
    if (prec > 99)
        prec = 99;
    if (prec >= 10) {
        *fs++ = prec / 10 + '0';
        *fs++ = prec % 10 + '0';
    } else {
        *fs++ = prec + '0';
    }
    *fs++ = 'l';
    *fs++ = f_char;
    *fs = '\0';
    QString num;
    num.sprintf(format, f);                        // convert to text

    if (f > 0.0 && (d->fieldFlags & showpos))
        num.prepend(QLatin1Char('+'));

    d->putString(num);
    return *this;
}


/*!
    \overload

    Writes a string to the stream and returns a reference to the
    stream.

    The string \a s is assumed to be Latin1 encoded independent of the
    Encoding set for the QTextStream.
*/

QTextStream &QTextStream::operator<<(const char *s)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(QLatin1String(s));
    return *this;
}

/*!
    \overload

    Writes \a s to the stream and returns a reference to the stream.

    The string \a s is assumed to be Latin1 encoded independent of the
    Encoding set for the QTextStream.
*/

QTextStream &QTextStream::operator<<(const QByteArray &array)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(array);
    return *this;
}

/*!
    \overload

    Writes \a s to the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<(const QString &s)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(s);
    return *this;
}


/*!
    \overload

    Writes a pointer to the stream and returns a reference to the
    stream.

    The \a ptr is output as an unsigned long hexadecimal integer.
*/

QTextStream &QTextStream::operator<<(const void *p)
{
    Q_D(QTextStream);
    CHECK_VALID_STREAM(*this);
    int f = d->fieldFlags;
    setf(hex, basefield);
    setf(showbase);
#if (QT_POINTER_SIZE == 4)
    d->putNumber((qint32)p, false);
#else
    d->putNumber((qint64)p, false);
#endif

    setFlags(f);
    return *this;
}



 /*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

QTextStream &bin(QTextStream &s)
{
    s.setf(QTextStream::bin,QTextStream::basefield);
    return s;
}

QTextStream &oct(QTextStream &s)
{
    s.setf(QTextStream::oct,QTextStream::basefield);
    return s;
}

QTextStream &dec(QTextStream &s)
{
    s.setf(QTextStream::dec,QTextStream::basefield);
    return s;
}

QTextStream &hex(QTextStream &s)
{
    s.setf(QTextStream::hex,QTextStream::basefield);
    return s;
}

QTextStream &endl(QTextStream &s)
{
    return s << '\n';
}

QTextStream &flush(QTextStream &s)
{
    if (s.device())
        s.device()->flush();
    return s;
}

QTextStream &ws(QTextStream &s)
{
    s.skipWhiteSpace();
    return s;
}

QTextStream &reset(QTextStream &s)
{
    s.reset();
    return s;
}


/*!
    \class QTextIStream
    \brief The QTextIStream class is a convenience class for input streams.

    \reentrant
    \ingroup io
    \ingroup text

    This class provides a shorthand for creating simple input
    \l{QTextStream}s without having to pass a \e mode argument to the
    constructor.

    This class makes it easy to write things like this:
    \code
    QString data = "123 456";
    int a, b;
    QTextIStream(&data) >> a >> b;
    \endcode

    \sa QTextOStream
*/

/*!
    \fn QTextIStream::QTextIStream(const QString *s)

    Constructs a stream to read from the string \a s.
*/
/*!
    \fn QTextIStream::QTextIStream(QByteArray *ba)

    Constructs a stream to read from the byte array \a ba.
*/
/*!
    \fn QTextIStream::QTextIStream(FILE *f)

    Constructs a stream to read from the file handle \a f.
*/


/*!
    \class QTextOStream
    \brief The QTextOStream class is a convenience class for output streams.

    \reentrant
    \ingroup io
    \ingroup text

    This class provides a shorthand for creating simple output
    \l{QTextStream}s without having to pass a \e mode argument to the
    constructor.

    This makes it easy to write things like this:
    \code
    QString result;
    QTextOStream(&result) << "pi = " << 3.14;
    \endcode
*/

/*!
    \fn QTextOStream::QTextOStream(QString *s)

    Constructs a stream to write to string \a s.
*/
/*!
    \fn QTextOStream::QTextOStream(QByteArray *ba)

    Constructs a stream to write to the byte array \a ba.
*/
/*!
    \fn QTextOStream::QTextOStream(FILE *f)

    Constructs a stream to write to the file handle \a f.
*/

#ifndef QT_NO_TEXTCODEC
/*!
    Sets the codec for this stream to \a codec. Will not try to
    autodetect Unicode.

    Note that this function should be called before any data is read
    to/written from the stream.

    \sa setEncoding(), codec()
*/

void QTextStream::setCodec(QTextCodec *codec)
{
    Q_D(QTextStream);
    d->codec = codec;
}

/*!
    Returns the codec actually used for this stream.

    If Unicode is automatically detected on input, a codec with \link
    QTextCodec::name() name() \endlink "ISO-10646-UCS-2" is returned.

    \sa setCodec()
*/

QTextCodec *QTextStream::codec() const
{
    Q_D(const QTextStream);
    return d->codec;
}

void QTextStream::setAutoDetectUnicode(bool enabled)
{
    Q_D(QTextStream);
    d->autoDetectUnicode = enabled;
}

bool QTextStream::autoDetectUnicode() const
{
    Q_D(const QTextStream);
    return d->autoDetectUnicode;
}

#ifdef QT_COMPAT
void QTextStream::setEncoding(Encoding encoding)
{
    Q_D(QTextStream);
    d->readConverterState = QTextCodec::ConverterState();
    d->writeConverterState = QTextCodec::ConverterState();

    switch (encoding) {
    case Locale:
        d->writeConverterState.flags |= QTextCodec::IgnoreHeader;
        setCodec(QTextCodec::codecForLocale());
        d->autoDetectUnicode = true;
        break;
    case Latin1:
        d->readConverterState.flags |= QTextCodec::IgnoreHeader;
        d->writeConverterState.flags |= QTextCodec::IgnoreHeader;
        setCodec(QTextCodec::codecForName("ISO-8851-1"));
        d->autoDetectUnicode = false;
        break;
    case Unicode:
        setCodec(QTextCodec::codecForName("UTF-16"));
        d->autoDetectUnicode = false;
        break;
    case RawUnicode:
        d->readConverterState.flags |= QTextCodec::IgnoreHeader;
        d->writeConverterState.flags |= QTextCodec::IgnoreHeader;
        setCodec(QTextCodec::codecForName("UTF-16"));
        d->autoDetectUnicode = false;
        break;
    case UnicodeNetworkOrder:
        d->readConverterState.flags |= QTextCodec::IgnoreHeader;
        d->writeConverterState.flags |= QTextCodec::IgnoreHeader;
        setCodec(QTextCodec::codecForName("UTF-16BE"));
        d->autoDetectUnicode = false;
        break;
    case UnicodeReverse:
        d->readConverterState.flags |= QTextCodec::IgnoreHeader;
        d->writeConverterState.flags |= QTextCodec::IgnoreHeader;
        setCodec(QTextCodec::codecForName("UTF-16LE"));
        d->autoDetectUnicode = false;
        break;
    case UnicodeUTF8:
        d->writeConverterState.flags |= QTextCodec::IgnoreHeader;
        setCodec(QTextCodec::codecForName("UTF-8"));
        d->autoDetectUnicode = true;
        break;
    }
}
#endif

#endif

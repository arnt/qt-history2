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
static const int QTEXTSTREAM_BUFFERSIZE = 16384;

/*! \class QTextStream

    \brief The QTextStream class provides a convenient interface for
    reading and writing text.

    QTextStream can operate on a QIODevice, a QByteArray or a
    QString. Using QTextStream's streaming operators, you can
    conveniently read and write strings, lines and numbers. With
    support for formatting options similar to those of C++ iostreams,
    you can use QTextStream to write any type of text output. Example:

    \code
        QFile data("output.txt");
        data.open(QFile::WriteOnly);

        QTextStream stream(&data);
        data << "Result: " << qSetW(10) << 3.14 << endl;
    \endcode

    Internally, QTextStream uses a Unicode buffer, and QTextCodec is
    used by QTextStream to support different character sets. By
    default, QTextCodec::codecForLocale() is used for reading and
    writing, but you can also set the codec by calling
    setCodec(). Automatic Unicode detection is also supported. When
    this feature is enabled, which it is by default, QTextStream will
    detect the UTF-16 BOM (Byte Order Mark) and switch to the
    appropriate UTF-16 codec when reading.

    By default, when reading numbers from a stream of text,
    QTextStream will automatically detect the number's textual
    representation. For example, if the number starts with "0x", it is
    assumed to be in hexadecimal form. If it starts with the digits
    1-9, it is assumed to be in decimal form, and so on. You can set
    the representation type, disabling the automatic detection, by
    calling setFlags() or using a manipulator function. Example:

    \code
        QTextStream stream("0x50 0x20");
        int firstNumber, secondNumber;

        stream >> firstNumber; // firstNumber == 80
        stream >> dec >> secondNumber; // secondNumber == 0

        char ch;
        stream >> ch; // ch == 'x'
    \encode




*/

#include "qtextstream.h"

#include <qbuffer.h>
#include <qfile.h>
#ifndef QT_NO_TEXTCODEC
#include <qtextcodec.h>
#endif
#ifndef Q_OS_TEMP
#include <locale.h>
#endif

// for strtod()
#include <stdlib.h>

#if defined QTEXTSTREAM_DEBUG
#include <ctype.h>

// Returns a human readable representation of the first \a len
// characters in \a data.
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

// A precondition macro
#define Q_VOID
#define CHECK_VALID_STREAM(x) do { \
    if (!d->string && !d->device) { \
        qWarning("QTextStream: No device"); \
        return x; \
    } } while (0)

// Base implementations of operator>> for ints and reals
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

//-------------------------------------------------------------------
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
        Space,
        NotSpace,
        EndOfLine,
        EndOfFile
    };

    bool scan(const QChar **ptr, int *tokenLength,
              int maxlen, TokenDelimiter delimiter);
    inline const QChar *readPtr() const;
    inline void consumeLastToken();
    inline void consume(int nchars);
    int lastTokenSize;

    inline bool write(const QString &data);
    inline bool getChar(QChar *ch);
    inline void ungetChar(const QChar &ch);
    bool getNumber(qulonglong *l);
    bool getReal(double *f);

    bool putNumber(qulonglong number, bool negative);
    inline bool putString(const QString &ch);

    // buffers
    bool fillReadBuffer();
    bool flushWriteBuffer();
    QString writeBuffer;
    QString readBuffer;
    int readBufferOffset;
    QString endOfBufferState;

    // streaming parameters
    int realPrecision;
    int numberBase;
    int padWidth;
    QChar padChar;
    QTextStream::PadMode padMode;
    QTextStream::RealNumberMode realNumberMode;
    QTextStream::NumberDisplayFlags numberDisplayFlags;

    QTextStream *q_ptr;
};

/*****************************************************************************
  QTextStreamPrivate implementation
 *****************************************************************************/

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
    realPrecision = 6;
    numberBase = 0;
    padWidth = 0;
    padChar = QLatin1Char(' ');
    padMode = QTextStream::NoPadding;
    realNumberMode = QTextStream::Floating;
    numberDisplayFlags = 0;

    device = 0;
    deleteDevice = false;
    string = 0;
    stringOffset = 0;
    stringOpenMode = QIODevice::NotOpen;

    readBufferOffset = 0;
    endOfBufferState.clear();
    lastTokenSize = 0;

#ifndef QT_NO_TEXTCODEC
    codec = QTextCodec::codecForLocale();
    readConverterState = QTextCodec::ConverterState();
    writeConverterState = QTextCodec::ConverterState();
    writeConverterState.flags |= QTextCodec::IgnoreHeader;
    autoDetectUnicode = true;
#endif
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

/*! \internal

    Scans no more than \a maxlen QChars in the current buffer for the
    first \a delimiter. Stores a pointer to the start offset of the
    token in \a ptr, and the length in QChars in \a length.
*/
bool QTextStreamPrivate::scan(const QChar **ptr, int *length, int maxlen, TokenDelimiter delimiter)
{
    int totalSize = 0;
    int delimSize = 0;
    bool consumeDelimiter = false;
    bool foundToken = false;
    int startOffset = device ? readBufferOffset : stringOffset;
    QChar lastChar;

    do {
        int endOffset;
        const QChar *chPtr;
        if (device) {
            chPtr = readBuffer.constData();
            endOffset = readBuffer.size();
        } else {
            chPtr = string->constData();
            endOffset = string->size();
        }
        chPtr += startOffset;

        for (; !foundToken && startOffset < endOffset && (!maxlen || totalSize+1 < maxlen); ++startOffset) {
            const QChar ch = *chPtr++;
            ++totalSize;

            if (delimiter == Space && ch.isSpace()) {
                foundToken = true;
                delimSize = 1;
            } else if (delimiter == NotSpace && !ch.isSpace()) {
                foundToken = true;
                delimSize = 1;
            } else if (delimiter == EndOfLine && ch == QLatin1Char('\n')) {
                foundToken = true;
                delimSize = (lastChar == QLatin1Char('\r')) ? 2 : 1;
                consumeDelimiter = true;
            }

            lastChar = ch;
        }
    } while (!foundToken && (!maxlen || totalSize < maxlen) && (device && fillReadBuffer()));

    // if the token was not found, but we reached the end of input,
    // then we accept what we got. if we are not at the end of input,
    // we return false.
    if (!foundToken && ((maxlen && totalSize < maxlen)
                        || (string && stringOffset + totalSize < string->size())
                        || (device && !device->atEnd()))) {
#if defined (QTEXTSTREAM_DEBUG)
        qDebug("QTextStreamPrivate::scan() did not find the token.");
#endif
        return false;
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

    // update last token size. the callee will call consumeLastToken() when
    // done.
    lastTokenSize = totalSize;
    if (!consumeDelimiter)
        lastTokenSize -= delimSize;

#if defined (QTEXTSTREAM_DEBUG)
    qDebug("QTextStreamPrivate::scan(%p, %p, %d, %x) token length = %d, delimiter = %d",
           ptr, length, maxlen, (int)delimiter, totalSize - delimSize, delimSize);
#endif
    return true;
}

inline const QChar *QTextStreamPrivate::readPtr() const
{
    Q_ASSERT(readBufferOffset <= readBuffer.size());
    if (string)
        return string->constData() + stringOffset;
    return readBuffer.constData() + readBufferOffset;
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

inline bool QTextStreamPrivate::write(const QString &data)
{
    if (string) {
        string->append(data);
    } else {
        writeBuffer += data;
        if (writeBuffer.size() > QTEXTSTREAM_BUFFERSIZE)
            return flushWriteBuffer();
    }
    return true;
}

inline bool QTextStreamPrivate::getChar(QChar *ch)
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

inline void QTextStreamPrivate::ungetChar(const QChar &ch)
{
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

inline bool QTextStreamPrivate::putString(const QString &s)
{
    QString tmp = s;

    // handle padding
    int padSize = padWidth - s.size();
    if (padSize > 0) {
        QString pad(padSize > 0 ? padSize : 0, padChar);
        if (padMode == QTextStream::PadLeft)
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

/*****************************************************************************
  QTextStream implementation
 *****************************************************************************/

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

void QTextStream::reset()
{
    Q_D(QTextStream);

    d->realPrecision = 6;
    d->numberBase = 0;
    d->padWidth = 0;
    d->padChar = QLatin1Char(' ');
    d->padMode = QTextStream::NoPadding;
    d->realNumberMode = QTextStream::Floating;
    d->numberDisplayFlags = 0;
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


void QTextStream::setPadMode(PadMode mode)
{
    Q_D(QTextStream);
    d->padMode = mode;
}

QTextStream::PadMode QTextStream::padMode() const
{
    Q_D(const QTextStream);
    return d->padMode;
}

void QTextStream::setPadChar(QChar ch)
{
    Q_D(QTextStream);
    d->padChar = ch;
}

QChar QTextStream::padChar() const
{
    Q_D(const QTextStream);
    return d->padChar;
}

void QTextStream::setPadWidth(int width)
{
    Q_D(QTextStream);
    d->padWidth = width;
}

int QTextStream::padWidth() const
{
    Q_D(const QTextStream);
    return d->padWidth;
}

void QTextStream::setNumberDisplayFlags(NumberDisplayFlags flags)
{
    Q_D(QTextStream);
    d->numberDisplayFlags = flags;
}

QTextStream::NumberDisplayFlags QTextStream::numberDisplayFlags() const
{
    Q_D(const QTextStream);
    return d->numberDisplayFlags;
}

void QTextStream::setNumberBase(int base)
{
    Q_D(QTextStream);
    d->numberBase = base;
}

int QTextStream::numberBase() const
{
    Q_D(const QTextStream);
    return d->numberBase;
}

void QTextStream::setRealNumberMode(RealNumberMode mode)
{
    Q_D(QTextStream);
    d->realNumberMode = mode;
}

QTextStream::RealNumberMode QTextStream::realNumberMode() const
{
    Q_D(const QTextStream);
    return d->realNumberMode;
}

#ifdef QT_COMPAT
int QTextStream::flagsInternal() const
{
    Q_D(const QTextStream);

    int f = 0;
    switch (d->padMode) {
    case PadLeft: f |= left; break;
    case PadRight: f |= right; break;
    case PadCentered: f |= internal; break;
    default:
        break;
    }
    switch (d->numberBase) {
    case 2: f |= bin; break;
    case 8: f |= oct; break;
    case 10: f |= dec; break;
    case 16: f |= hex; break;
    default:
        break;
    }
    switch (d->realNumberMode) {
    case Fixed: f |= fixed; break;
    case Scientific: f |= scientific; break;
    default:
        break;
    }
    if (d->numberDisplayFlags & ShowBase)
        f |= showbase;
    if (d->numberDisplayFlags & ShowPoint)
        f |= showpoint;
    if (d->numberDisplayFlags & ShowSign)
        f |= showpos;
    if (d->numberDisplayFlags & UppercaseBase)
        f |= uppercase;
    return f;
}

int QTextStream::flagsInternal(int newFlags)
{
    int oldFlags = flagsInternal();

    if (newFlags & left)
        setPadMode(PadLeft);
    else if (newFlags & right)
        setPadMode(PadRight);
    else if (newFlags & internal)
        setPadMode(PadCentered);

    if (newFlags & bin)
        setNumberBase(2);
    else if (newFlags & oct)
        setNumberBase(8);
    else if (newFlags & dec)
        setNumberBase(10);
    else if (newFlags & hex)
        setNumberBase(16);

    if (newFlags & showbase)
        setNumberDisplayFlags(numberDisplayFlags() | ShowBase);
    if (newFlags & showpos)
        setNumberDisplayFlags(numberDisplayFlags() | ShowSign);
    if (newFlags & showpoint)
        setNumberDisplayFlags(numberDisplayFlags() | ShowPoint);
    if (newFlags & uppercase)
        setNumberDisplayFlags(numberDisplayFlags() | UppercaseBase);

    if (newFlags & fixed)
        setRealNumberMode(Fixed);
    else if (newFlags & scientific)
        setRealNumberMode(Scientific);

    return oldFlags;
}
#endif

int QTextStream::precision() const
{
    Q_D(const QTextStream);
    return d->realPrecision;
}

void QTextStream::setPrecision(int precision)
{
    Q_D(QTextStream);
    d->realPrecision = precision;
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

    const QChar *readPtr;
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

    const QChar *readPtr;
    int length;
    if (!d->scan(&readPtr, &length, int(maxlen), QTextStreamPrivate::EndOfLine))
        return QLatin1String("");

    QString tmp = QString(readPtr, length);
    d->consumeLastToken();
    return tmp;
}


bool QTextStreamPrivate::getNumber(qulonglong *ret)
{
    scan(0, 0, 0, NotSpace);
    consumeLastToken();

    // detect int encoding
    int base = numberBase;
    if (base == 0) {
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
                base = 16;
            } else if (ch2 == QLatin1Char('b')) {
                base = 2;
            } else if (ch2.isDigit() && ch2.digitValue() >= 0 && ch2.digitValue() <= 7) {
                base = 8;
            } else {
                base = 10;
            }
            ungetChar(ch2);
        } else if (ch == QLatin1Char('-') || ch == QLatin1Char('+') || ch.isDigit()) {
            base = 10;
        } else {
            ungetChar(ch);
            return false;
        }
        ungetChar(ch);
    }

    qulonglong val=0;
    switch (base) {
    case 2: {
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
    case 8: {
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
    case 10: {
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
    case 16: {
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

    const QChar *ptr;
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

    const QChar *ptr;
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

    const QChar *ptr;
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
    if (negative)
        tmp = QLatin1Char('-');
    else if (numberDisplayFlags & QTextStream::ShowSign)
        tmp = QLatin1Char('+');

    if (numberDisplayFlags & QTextStream::ShowBase) {
        switch (numberBase) {
        case 2: tmp += "0b"; break;
        case 8: tmp += "0"; break;
        case 16: tmp += "0x"; break;
        default: break;
        }
    }

    tmp += QString::number(number, numberBase ? numberBase : 10);
    if (numberDisplayFlags & QTextStream::UppercaseBase)
        tmp = tmp.toUpper(); // ### in-place instead

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
    if (d->realNumberMode == Fixed)
        f_char = 'f';
    else if (d->realNumberMode == Scientific)
        f_char = (d->numberDisplayFlags & UppercaseBase) ? 'E' : 'e';
    else
        f_char = (d->numberDisplayFlags & UppercaseBase) ? 'G' : 'g';

    // generate format string
    register char *fs = format;

    // "%.<prec>l<f_char>"
    *fs++ = '%';
    *fs++ = '.';
    int prec = d->realPrecision;
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

    if (f > 0.0 && (d->numberDisplayFlags & ShowSign))
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
    int oldBase = d->numberBase;
    NumberDisplayFlags oldFlags = d->numberDisplayFlags;
    d->numberBase = 16;
    d->numberDisplayFlags |= ShowBase;
    d->putNumber(reinterpret_cast<quint64>(p), false);
    d->numberBase = oldBase;
    d->numberDisplayFlags = oldFlags;

    return *this;
}



 /*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

QTextStream &bin(QTextStream &s)
{
    s.setNumberBase(2);
    return s;
}

QTextStream &oct(QTextStream &s)
{
    s.setNumberBase(8);
    return s;
}

QTextStream &dec(QTextStream &s)
{
    s.setNumberBase(10);
    return s;
}

QTextStream &hex(QTextStream &s)
{
    s.setNumberBase(16);
    return s;
}

QTextStream &showbase(QTextStream &s)
{
    s.setNumberDisplayFlags(s.numberDisplayFlags() | QTextStream::ShowBase);
    return s;
}

QTextStream &showpoint(QTextStream &s)
{
    s.setNumberDisplayFlags(s.numberDisplayFlags() | QTextStream::ShowPoint);
    return s;
}

QTextStream &showsign(QTextStream &s)
{
    s.setNumberDisplayFlags(s.numberDisplayFlags() | QTextStream::ShowSign);
    return s;
}

QTextStream &uppercasebase(QTextStream &s)
{
    s.setNumberDisplayFlags(s.numberDisplayFlags() | QTextStream::UppercaseBase);
    return s;
}

QTextStream &fixed(QTextStream &s)
{
    s.setRealNumberMode(QTextStream::Fixed);
    return s;
}

QTextStream &scientific(QTextStream &s)
{
    s.setRealNumberMode(QTextStream::Scientific);
    return s;
}

QTextStream &floating(QTextStream &s)
{
    s.setRealNumberMode(QTextStream::Floating);
    return s;
}

QTextStream &padleft(QTextStream &s)
{
    s.setPadMode(QTextStream::PadLeft);
    return s;
}

QTextStream &padright(QTextStream &s)
{
    s.setPadMode(QTextStream::PadRight);
    return s;
}

QTextStream &padcentered(QTextStream &s)
{
    s.setPadMode(QTextStream::PadCentered);
    return s;
}

QTextStream &endl(QTextStream &s)
{
    return s << QLatin1Char('\n');
}

QTextStream &flush(QTextStream &s)
{
    s.flush();
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

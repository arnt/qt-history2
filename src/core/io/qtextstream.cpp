/****************************************************************************
**
** Implementation of QTextStream class.
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

#include "qtextstream.h"

#ifndef QT_NO_TEXTSTREAM
#include "qtextcodec.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qfile.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <qioengine.h>
#ifndef Q_OS_TEMP
#include <locale.h>
#endif

#include <qdatetime.h>
#include <qchar.h>

#if defined(Q_OS_WIN32)
#include "qt_windows.h"
#endif

#define d d_func()
#define q q_func()

/*!
    \class QTextStream
    \brief The QTextStream class provides functions for reading and
    writing text using a QIODevice.

    \reentrant
    \ingroup io
    \ingroup text
    \mainclass

    The text stream class has a functional interface that is very
    similar to that of the standard C++ iostream class.

    Qt provides several global functions similar to the ones in iostream:
    \table
    \header \i Function \i Meaning
    \row \i bin \i sets the QTextStream to read/write binary numbers
    \row \i oct \i sets the QTextStream to read/write octal numbers
    \row \i dec \i sets the QTextStream to read/write decimal numbers
    \row \i hex \i sets the QTextStream to read/write hexadecimal numbers
    \row \i endl \i forces a line break
    \row \i flush \i forces the QIODevice to flush any buffered data
    \row \i ws \i eats any available whitespace (on input)
    \row \i reset \i resets the QTextStream to its default mode (see reset())
    \row \i qSetW(int) \i sets the \link width() field width \endlink
    to the given argument
    \row \i qSetFill(int) \i sets the \link fill() fill character
    \endlink to the given argument
    \row \i qSetPrecision(int) \i sets the \link precision() precision
    \endlink to the given argument
    \endtable

    \warning By default QTextStream will automatically detect whether
    integers in the stream are in decimal, octal, hexadecimal or
    binary format when reading from the stream. In particular, a
    leading '0' signifies octal, i.e. the sequence "0100" will be
    interpreted as 64.

    The QTextStream class reads and writes text; it is not appropriate
    for dealing with binary data (but QDataStream is).

    By default, output of Unicode text (i.e. QString) is done using
    the local 8-bit encoding. This can be changed using the
    setEncoding() method. For input, the QTextStream will auto-detect
    standard Unicode "byte order marked" text files; otherwise the
    local 8-bit encoding is used.

    The underlying QIODevice is set in the constructor, or later using
    setDevice(). If the end of the input is reached, atEnd() returns
    true. Data can be read into variables of the appropriate type
    using the operator>>() overloads, or read in its entirety into a
    single string using read(), or read a line at a time using
    readLine(). Whitespace can be skipped over using skipWhiteSpace().
    You can set flags for the stream using flags() or setf(). The
    stream also supports width(), precision() and fill(); use reset()
    to reset the defaults.

    \sa QDataStream
*/

/*!
    \enum QTextStream::Encoding

    \value Locale
    \value Latin1
    \value Unicode
    \value UnicodeNetworkOrder
    \value UnicodeReverse
    \value RawUnicode
    \value UnicodeUTF8

    See setEncoding() for an explanation of the encodings.
*/

/*
  \class QTSManip

  \brief The QTSManip class is an internal helper class for the
  QTextStream.

  \internal

    \ingroup text

  It is generally a very bad idea to use this class directly in
  application programs.

  This class makes it possible to give the QTextStream function objects
  with arguments, like this:
  \code
    QTextStream cout(stdout, QIODevice::WriteOnly);
    cout << setprecision(8);                // QTSManip used here!
    cout << 3.14159265358979323846;
  \endcode

  The setprecision() function returns a QTSManip object.
  The QTSManip object contains a pointer to a member function in
  QTextStream and an integer argument.
  When serializing a QTSManip into a QTextStream, the function
  is executed with the argument.
*/

/*! \fn QTSManip::QTSManip(QTSMFI m, int a)

  Constructs a QTSManip object which will call \a m (a member function
  in QTextStream which accepts a single int) with argument \a a when
  QTSManip::exec() is called. Used internally in e.g. endl:

  \code
    s << "some text" << endl << "more text";
  \endcode
*/

/*! \fn void QTSManip::exec(QTextStream& s)

  Calls the member function specified in the constructor, for object
  \a s. Used internally in e.g. endl:

  \code
    s << "some text" << endl << "more text";
  \endcode
*/


/*****************************************************************************
  QTextStream member functions
 *****************************************************************************/

#ifndef QT_NO_DEBUG
#undef  CHECK_STREAM_PRECOND
#define CHECK_STREAM_PRECOND(x)  if (x->d->sourceType != QTextStreamPrivate::String && !x->d->dev) { \
                                          qWarning("QTextStream: No device");                        \
                                          return *x; }
#else
#define CHECK_STREAM_PRECOND(x)
#endif

#define TS_MOD_NOT   0x10
#define TS_SPACE     0x01
#define TS_EOL       0x02
#define TS_HEX       0x03
#define TS_DIGIT     0x04
#define TS_BIN       0x05

#define I_SHORT                0x0010
#define I_INT                0x0020
#define I_LONG                0x0030
#define I_TYPE_MASK        0x00f0

#define I_BASE_2        QTS::bin
#define I_BASE_8        QTS::oct
#define I_BASE_10        QTS::dec
#define I_BASE_16        QTS::hex
#define I_BASE_MASK        (QTS::bin | QTS::oct | QTS::dec | QTS::hex)

#define I_SIGNED        0x0100
#define I_UNSIGNED        0x0200
#define I_SIGN_MASK        0x0f00


static const unsigned short QEOF = 0xffff; //guaranteed not to be a character.
static const uint getstr_tmp_size  = 64; //these are the temp buffers created on the stack,
static const uint getnum_tmp_size  = 8;  //they are low to prevent excessive allocation.
static const uint getbuf_cache_size  = 1024;

const int QTextStream::basefield   = I_BASE_MASK;
const int QTextStream::adjustfield = (QTextStream::left |
                                       QTextStream::right |
                                       QTextStream::internal);
const int QTextStream::floatfield  = (QTextStream::scientific |
                                       QTextStream::fixed);

class QCircularBuffer {
    QByteArray buf[2];
    uint used, start_off, start_buff, curr_buff;
public:
    QCircularBuffer() : used(0), start_off(0), start_buff(0),
                        curr_buff(0) { }

    char *alloc(uint);
    char *take(uint, uint* =NULL);
    void free(uint);

    inline int numBuffers() { return 2; }
    inline bool isEmpty() { return !size(); }
    inline uint size() { return used; }
    inline void truncate(uint i) { used -= i; }
};
inline char *QCircularBuffer::alloc(uint size)
{
    if(buf[curr_buff].size() <
       (int)(used+size+(curr_buff == start_buff ? start_off : 0))) {
        if(curr_buff == start_buff && buf[curr_buff].size()) {
            buf[curr_buff].resize(start_off + used);
            curr_buff = !curr_buff;
            if(!buf[curr_buff].size())
                buf[curr_buff].resize(getbuf_cache_size*2);
        } else {
            int sz = buf[curr_buff].size();
            buf[curr_buff].resize(qMax((uint)sz + (sz / 2), (getbuf_cache_size*2)));
        }
    }
    int off = used;
    used += size;
    if(curr_buff != start_buff)
        off -= buf[start_buff].size() - start_off;
    else
        off += start_off;
    return buf[curr_buff].data()+off;
}
inline char *QCircularBuffer::take(uint size, uint *real_size)
{
    if(size > used) {
        qWarning("Warning: asked to take too much %d [%d]", size, used);
        size = used;
    }
    if(real_size)
        *real_size = qMin(size, buf[start_buff].size() - start_off);
    return buf[start_buff].data()+start_off;
}

inline void QCircularBuffer::free(uint size)
{
    if(size > used) {
        qWarning("Warning: asked to free too much %d [%d]", size, used);
        size = used;
    }
    used -= size;
    if(used == 0) {
        curr_buff = start_buff = start_off = 0;
        return;
    }

    uint start_size = buf[start_buff].size() - start_off;
    if(start_size > size) {
        start_off += size;
    } else if(start_buff != curr_buff) {
        start_buff = curr_buff;
        start_off = start_size - size;
    } else {
        start_off = 0;
    }
}

class QTextStreamPrivate {
    QTextStream *q_ptr;
    Q_DECLARE_PUBLIC(QTextStream)

protected:
#ifndef QT_NO_TEXTCODEC
    QTextStreamPrivate() : decoder(0), encoder(0), sourceType(NotSet)  {  init();  }
    ~QTextStreamPrivate() {
        delete decoder;
        delete encoder;
    }
    QTextDecoder *decoder;
    QTextEncoder *encoder;
#else
    QTextStreamPrivate() : sourceType(NotSet) { init(); }
    ~QTextStreamPrivate() { }
#endif

    inline void init() {
        dev = 0;
        str = 0;
        owndev = false;
        doUnicodeHeader = true; // autodetect
        mapper = 0;
        latin1 = true; // should use locale?
        internalOrder = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
        networkOrder = true;
    }

    long input_int();
    QTextStream &output_int(int, ulong, bool);

    QChar ts_getc();
    void ts_ungetc(QChar);
    void ts_putc(int);

    /* These are the only functions that actually interact with the input/output */
    bool ts_getbuf(QChar*, uint, uchar =0, uint * =NULL);
    void ts_putc(QChar);

    ulong input_bin();
    ulong input_oct();
    ulong input_dec();
    ulong input_hex();
    double input_double();
    QTextStream &writeBlock(const char* p, uint len);
    QTextStream &writeBlock(const QChar* p, uint len);

    QTextCodec         *mapper;
    bool internalOrder, networkOrder;

    bool doUnicodeHeader, owndev, latin1;
    QString ungetcBuf;
    mutable QCircularBuffer cacheReadBuf;

    enum SourceType { NotSet, IODevice, String, ByteArray, File };
    SourceType sourceType;

    QIODevice *dev;

    QString *str;
    uint strOff;

    int fflags;
    int fwidth;
    int fillchar;
    int fprec;
};


void QTextStream::init()
{
    d_ptr = new QTextStreamPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Constructs a data stream that has no IO device.

    \sa setDevice()
*/

QTextStream::QTextStream()
{
    init();
    setEncoding(Locale);
    reset();
    d->sourceType = QTextStreamPrivate::NotSet;
}

/*!
    Constructs a text stream that uses the IO device \a iod.
*/

QTextStream::QTextStream(QIODevice *iod)
{
    init();
    setEncoding(Locale);
    d->dev = iod;
    reset();
    d->sourceType = QTextStreamPrivate::IODevice;
}

/*!
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

QTextStream::QTextStream(QString *str, int)
{
    init();
    d->str = str;
    d->strOff = 0;
    setEncoding(RawUnicode);
    reset();
    d->sourceType = QTextStreamPrivate::String;
}

/*!
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

QTextStream::QTextStream(QByteArray *a, int mode)
{
    init();
    QBuffer *buf = new QBuffer(a);
    buf->open(mode);
    d->dev = buf;
    d->owndev = true;
    setEncoding(Latin1); //### Locale???
    reset();
    d->sourceType = QTextStreamPrivate::ByteArray;
}

/*!
    Constructs a text stream that operates on byte array \a a. Since
    the byte array is passed as a const it can only be read from; use
    QTextStream(QByteArray*, int) if you want to write to a byte
    array. The \a mode is a QIODevice::mode() and should normally be
    \c QIODevice::ReadOnly.

    Since QByteArray is not a QIODevice subclass, internally a QBuffer
    is created to wrap the byte array.
*/
QTextStream::QTextStream(const QByteArray &a, int mode)
{
    init();
    QBuffer *buf = new QBuffer;
    buf->setData(a);
    buf->open(mode);
    d->dev = buf;
    d->owndev = true;
    setEncoding(Latin1); //### Locale???
    reset();
    d->sourceType = QTextStreamPrivate::ByteArray;

    if (buf->isWritable())
        qWarning("QTextStream::QTextStream: Use the QTextStream(QByteArray *, int) constructor "
                 "instead");
}

/*!
    Constructs a text stream that operates on an existing file handle
    \a fh, through an internal QFile device. The \a mode argument is
    passed to the device's open() function; see \l{QIODevice::mode()}.

    \warning If you create a QTextStream \c cout or another name that
    is also used for another variable of a different type, some
    linkers may confuse the two variables, which will often cause
    crashes.
*/

QTextStream::QTextStream(FILE *fh, int mode)
{
    init();
    setEncoding(Locale); //###
    d->dev = new QFile;
    ((QFile *)d->dev)->open(mode, fh);
    d->owndev = true;
    reset();
    d->sourceType = QTextStreamPrivate::File;
}

/*!
    Destroys the text stream.

    The destructor does not affect the current IO device.
*/

QTextStream::~QTextStream()
{
    if (d->owndev)
        delete d->dev;
    delete d_ptr;
}

/*!
    Positions the read pointer at the first non-whitespace character.
*/
void QTextStream::skipWhiteSpace()
{
    while(!d->ts_getbuf(NULL, getstr_tmp_size, TS_MOD_NOT|TS_SPACE));
}

/*
    Returns true if the conditions in flags are met
*/
inline static int ts_end(const QChar *c, uint len, uchar flags)
{
    if((QChar)(c->unicode()) == QEOF || !len)
        return 1;
    int end = 0;
    switch((flags & 0x0F)) {
    case 0:
        return false;
    case TS_EOL:
        if(*c == '\n')
            end = 1;
        else if(len >= 2 && *c == '\r' && *(c+1) == '\n')
            end = 2;
        break;
    case TS_SPACE:
        if(c->isSpace())
            end = 1;
        break;
    case TS_DIGIT:
        if(c->isDigit())
            end = 1;
        break;
    case TS_HEX:
        if(isxdigit(c->latin1()))
            end = 1;
        break;
    case TS_BIN:
        if(c->isDigit() && (c->latin1() == '0' || c->latin1() == '1'))
            end = 1;
        break;
    default:
        qWarning("Unknown flags 0x%02x", flags);
        break;
    }
    if(flags & TS_MOD_NOT)
        return !end;
    return end;
}

/*
    Tries to read \a len characters from the stream and stores them in \a
    buf. Placing the number of characters really read into \a l. This will
    return true if the \a end_flags are met (or end of file), false if the
    buffer is just filled.

    \warning There will no QEOF appended if the read reaches the end
    of the file. EOF is reached when the return value does not equal
    \a len.
*/
bool QTextStreamPrivate::ts_getbuf(QChar* buf, uint len, uchar end_flags, uint *l)
{
    if (len < 1) {
        if(l)
            *l = 0;
        return false;
    }

    //just read directly from the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String) {
        const uint remaining = d->str->length()-(d->strOff/sizeof(QChar));
        const QChar *data = (QChar*)((char*)d->str->unicode()+d->strOff);
        for(uint i = 0; i < len; i++) {
            if(i == remaining) {
                if(l)
                    *l = i;
                d->strOff += i * sizeof(QChar);
                return true;
            } else if(int end = ts_end(data+i, remaining - i, end_flags)) {
                i += end - 1;
                if(l)
                    *l = i;
                d->strOff += i * sizeof(QChar);
                return true;
            }
            if(buf)
                buf[i] = data[i];
        }
        if(l)
            *l = len;
        d->strOff += len * sizeof(QChar);
        return false;
    }

    //read from the device
    enum { NO_FINISH, END_FOUND, END_BUFFER } ret = NO_FINISH;
    uint rnum = 0;   // the number of QChars really read

    if (d && d->ungetcBuf.length()) {
        uint ungetc_len = d->ungetcBuf.length();
        const QChar *ungetc_buff = d->ungetcBuf.unicode();
        while(rnum < ungetc_len) {
            if(int end = ts_end(ungetc_buff+rnum, ungetc_len-rnum, end_flags)) {
                rnum += (end - 1);
                ret = END_FOUND;
                break;
            }
            if(buf)
                   *(buf++) = *(ungetc_buff+rnum);
            rnum++;
            if(rnum >= len) {
                ret = END_BUFFER;
                break;
            }
        }
        d->ungetcBuf = d->ungetcBuf.mid(rnum);
        if (ret != NO_FINISH) {
            if(l)
                *l = rnum;
            return (ret == END_FOUND) ? true : false;
        }
    }

    if (d->doUnicodeHeader) {
        d->doUnicodeHeader = false; // only at the top
        int c1 = d->dev->getch();
        if (c1 == EOF) {
            if(l)
                *l = rnum;
            return true;
        }
        int c2 = d->dev->getch();
        if (c1 == 0xfe && c2 == 0xff) {
            d->mapper = 0;
            d->latin1 = false;
            d->internalOrder = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
            d->networkOrder = true;
        } else if (c1 == 0xff && c2 == 0xfe) {
            d->mapper = 0;
            d->latin1 = false;
            d->internalOrder = (QSysInfo::ByteOrder != QSysInfo::BigEndian);
            d->networkOrder = false;
        } else {
            if (c2 != EOF) {
                char *b = d->cacheReadBuf.alloc(2);
                b[0] = c1;
                b[1] = c2;
            } else {
                /*
                  A small bug might hide here. If only the first byte
                  of a file has made it so far, and that first byte
                  is half of the byte-order mark, then the utfness
                  will not be detected.
                */
                *d->cacheReadBuf.alloc(1) = (char)c1;
            }
        }
    }

    int iter = 0, at_end = 0;
    do {
        iter++;
        if(d->dev->atEnd()) {
            at_end++;
            if(d->cacheReadBuf.isEmpty() ||
               at_end > d->cacheReadBuf.numBuffers()) {
                ret = END_FOUND;
                break;
            }
        } else {
            const uint buf_size = d->cacheReadBuf.size();
            uint need_num = ((len - rnum) * iter);
            if(need_num > buf_size) {
                if(need_num < getbuf_cache_size)
                    need_num = getbuf_cache_size;
                char *buff = d->cacheReadBuf.alloc(need_num);
                uint r = d->dev->readBlock(buff, need_num);
                if(r < need_num)
                    d->cacheReadBuf.truncate(need_num - r);
                if(!r && iter > 1) {
                    ret = END_FOUND;
                    break;
                }
            }
        }

        uint buffer_len = d->cacheReadBuf.size();
        char *buffer_data = d->cacheReadBuf.take(buffer_len, &buffer_len);
        if(!buffer_len)
            break;

#ifndef QT_NO_TEXTCODEC
        if(d->mapper) {
            if (!d->decoder)
                d->decoder = d->mapper->makeDecoder();
            /* We don't use buffer_len here, but instead use a "guess" at how
               much to decode. We do this to avoid decoding more than we need to
               (but it will grab more with each iteration if necesary) --Sam */
            int need_num = qMin((len - rnum) * iter, buffer_len);
            QString s = d->decoder->toUnicode(buffer_data, need_num);
            d->cacheReadBuf.free(need_num);

            uint used_len = qMin((len - rnum), (uint)s.length());
            if(end_flags) {
                for(uint i = 0; i < used_len; i++) {
                    if(int end = ts_end(s.unicode()+i, used_len - i, end_flags)) {
                        used_len = i + (end - 1);
                        ret = END_FOUND;
                        break;
                    }
                }
            }
            if(buf) {
                memcpy(buf, s.unicode(), used_len*sizeof(buf[0]));
                buf += used_len;
            }
            rnum += used_len;
            if (s.length() > (int)used_len)
                d->ungetcBuf = s.mid(used_len);
        } else
#endif
        if (d->latin1) {
            uint used_len = 0;
            for(char *it = buffer_data, *end = it + buffer_len;
                rnum < len && it < end; it++) {
                if(buf)
                    *(buf++) = *it;
                if(end_flags) {
                    int end = 0;
                    if((end_flags & 0x0F) == TS_EOL) {
                        if(*it == '\n')
                            end = 1;
                        else if(used_len+1 <= buffer_len &&
                                *it == '\r' && *(it+1) == '\n')
                            end = 2;
                        if(end_flags & TS_MOD_NOT)
                            end = !end;
                    }
                    if(!end) {
                        QChar c(*it);
                        end = ts_end(&c, 1, end_flags);
                    }
                    if(end) {
                        used_len += (end - 1);
                        rnum += (end - 1);
                        ret = END_FOUND;
                        break;
                    }
                }
                used_len++;
                rnum++;
            }
            if(used_len)
                d->cacheReadBuf.free(used_len);
        } else { // ISO-10646-UCS-2 or UTF-16
            int used_len = 0;
            for(uint i = 0; rnum < len && i+1 < buffer_len; i+=2) {
                QChar next_c;
                if (d->networkOrder)
                    next_c = QChar(buffer_data[i+1], buffer_data[i]);
                else
                    next_c = QChar(buffer_data[i], buffer_data[i+1]);
                if(end_flags) {
                    int end = 0;
                    if((end_flags & 0x0F) == TS_EOL) {
                        if(next_c == '\r' && i + 4 <= buffer_len) {
                            QChar n;
                            if (d->networkOrder)
                                n = QChar(buffer_data[i+3],
                                          buffer_data[i+2]);
                            else
                                n = QChar(buffer_data[i+2],
                                          buffer_data[i+3]);
                            if(n == '\n')
                                end = 2;
                            if(end_flags & TS_MOD_NOT)
                                end = !end;
                            if(end)
                                ret = END_FOUND;
                        }
                    }
                    if(!end)
                        end = ts_end(&next_c, 1, end_flags);
                    if(end) {
                        used_len += (end - 1);
                        rnum += (end - 1);
                        ret = END_FOUND;
                        break;
                    }
                }
                if(buf)
                    *(buf++) = next_c;
                rnum++;
                used_len += 2;
            }
            if(used_len)
                d->cacheReadBuf.free(used_len);
        }
        if(ret == NO_FINISH && rnum >= len)
            ret = END_BUFFER;
    } while(ret == NO_FINISH);
    if(l)
        *l = rnum;
    if(q->atEnd())
        return rnum == 0;
    return ret == END_FOUND;
}

/*
    Puts one character into the stream.
*/
void QTextStreamPrivate::ts_putc(QChar c)
{
    //just append directly onto the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String) {
        d->str->append(c);
        return;
    }

    //put it into the device
#ifndef QT_NO_TEXTCODEC
    if (d->mapper) {
        if (!d->encoder)
            d->encoder = d->mapper->makeEncoder();
        int len = 1;
        QString s(c);
        QByteArray block = d->encoder->fromUnicode(s, len);
        d->dev->writeBlock(block, len);
    } else
#endif
    if (d->latin1) {
        if (c.row())
            d->dev->putch('?'); // unknown character
        else
            d->dev->putch(c.cell());
    } else {
        if (d->doUnicodeHeader) {
            d->doUnicodeHeader = false;
            if (!d->dev->isSequentialAccess() && d->dev->at() == 0)
                ts_putc(QChar::ByteOrderMark);
        }
        if (d->internalOrder) {
            // this case is needed by QStringBuffer
            d->dev->writeBlock((char*)&c, sizeof(QChar));
        } else if (d->networkOrder) {
            d->dev->putch(c.row());
            d->dev->putch(c.cell());
        } else {
            d->dev->putch(c.cell());
            d->dev->putch(c.row());
        }
    }
}

/*
    Puts one character into the stream.
*/
void QTextStreamPrivate::ts_putc(int ch)
{
    ts_putc(QChar((ushort)ch));
}

void QTextStreamPrivate::ts_ungetc(QChar c)
{
    if (c.unicode() == 0xffff)
        return;
    //just append directly onto the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String) {
        if(d->strOff > 0)
            *((QChar*)((char*)d->str->data()+(d->strOff-=2))) = c;
        return;
    }
    //stick it into the buffer
    d->ungetcBuf.prepend(c);
}



/*!
    Reads \a len bytes from the stream into \a s and returns a
    reference to the stream.

    The buffer \a s must be preallocated.

    Note that no encoding is done by this function.

    \warning The behavior of this function is undefined unless the
    stream's encoding is set to Unicode or Latin1.

    \sa QIODevice::readBlock()
*/

QTextStream &QTextStream::readRawBytes(char *s, uint len)
{
    //just append directly onto the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String) {
        len = qMin((d->str->length()*sizeof(QChar))-d->strOff, len);
        memcpy(s, ((char *)d->str->unicode())+d->strOff, len);
        d->strOff += len;
        return *this;
    }
    //from device
    d->dev->readBlock(s, len);
    return *this;
}

/*!
    Writes the \a len bytes from \a s to the stream and returns a
    reference to the stream.

    Note that no encoding is done by this function.

    \sa QIODevice::writeBlock()
*/

QTextStream &QTextStream::writeRawBytes(const char* s, uint len)
{
    //just append directly onto the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String) {
        d->str->append(QString::fromLatin1(s, len));
        return *this;
    }
    //from device
    d->dev->writeBlock(s, len);
    return *this;
}


QTextStream &QTextStreamPrivate::writeBlock(const char* p, uint len)
{
    //just append directly onto the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String) {
        d->str->append(QString::fromLatin1(p, len));
        return *q;
    }

    //from device
    if (d->doUnicodeHeader) {
        d->doUnicodeHeader = false;
        if (!d->mapper && !d->latin1 && !d->dev->isSequentialAccess() && d->dev->at() == 0)
            d->ts_putc(QChar::ByteOrderMark);
    }
    // QByteArray and const char * are treated as Latin1
    if (!d->mapper && d->latin1) {
        d->dev->writeBlock(p, len);
    } else if (!d->mapper && d->internalOrder) {
        QChar *u = new QChar[len];
        for (uint i = 0; i < len; i++)
            u[i] = p[i];
        d->dev->writeBlock((char*)u, len * sizeof(QChar));
        delete [] u;
    }
#ifndef QT_NO_TEXTCODEC
    else if (d->mapper) {
        if (!d->encoder)
            d->encoder = d->mapper->makeEncoder();
        QString s = QString::fromLatin1(p, len);
        int l = len;
        QByteArray block = d->encoder->fromUnicode(s, l);
        d->dev->writeBlock(block, l);
    }
#endif
    else {
        for (uint i = 0; i < len; i++)
            d->ts_putc((uchar)p[i]);
    }
    return *q;
}

QTextStream &QTextStreamPrivate::writeBlock(const QChar* p, uint len)
{
    //just append directly onto the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String) {
        d->str->append(QString(p, len));
        return *q;
    }

    //from device
#ifndef QT_NO_TEXTCODEC
    if (d->mapper) {
        if (!d->encoder)
            d->encoder = d->mapper->makeEncoder();
        QString s(p, len);
        int l = len;
        QByteArray block = d->encoder->fromUnicode(s, l);
        d->dev->writeBlock(block, l);
    } else
#endif
    if (d->latin1) {
        QString cstr = QString::fromRawData(p, len);
        d->dev->writeBlock(cstr.latin1(), len);
    } else if (d->internalOrder) {
        if (d->doUnicodeHeader) {
            d->doUnicodeHeader = false;
            if (!d->dev->isSequentialAccess() && d->dev->at() == 0)
                d->ts_putc(QChar::ByteOrderMark);
        }
        d->dev->writeBlock((char*)p, sizeof(QChar)*len);
    } else {
        for (uint i=0; i<len; i++)
            d->ts_putc(p[i]);
    }
    return *q;
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
    d->fflags = 0;
    d->fwidth = 0;
    d->fillchar = ' ';
    d->fprec = 6;
}

/*!
    Returns the IO device currently set.

    \sa setDevice(), unsetDevice()
*/

QIODevice *QTextStream::device() const
{
    return d->dev;
}


/*!
    Sets the IO device to \a iod.

    \sa device(), unsetDevice()
*/

void QTextStream::setDevice(QIODevice *iod)
{
    if (d->owndev) {
        delete d->dev;
        d->owndev = false;
    }
    d->dev = iod;
    d->sourceType = QTextStreamPrivate::IODevice;
}

/*!
    Unsets the IO device. Equivalent to setDevice(0).

    \sa device(), setDevice()
*/

void QTextStream::unsetDevice()
{
    setDevice(0);
    d->sourceType = QTextStreamPrivate::NotSet;
}

/*!
    \fn bool QTextStream::atEnd() const

    Returns true if the IO device has reached the end position (end of
    the stream or file) or if there is no IO device set; otherwise
    returns false.

    \sa QIODevice::atEnd()
*/

/*!\fn bool QTextStream::eof() const

  \obsolete

  This function has been renamed to atEnd().

  \sa QIODevice::atEnd()
*/

/*****************************************************************************
  QTextStream read functions
 *****************************************************************************/


/*!
    \overload

    Reads a char \a c from the stream and returns a reference to the
    stream. Note that whitespace is skipped.
*/

QTextStream &QTextStream::operator>>(char &c)
{
    CHECK_STREAM_PRECOND(this)
    skipWhiteSpace();
    c = d->ts_getc().latin1();
    return *this;
}

/*!
    Reads a char \a c from the stream and returns a reference to the
    stream. Note that whitespace is \e not skipped.
*/

QTextStream &QTextStream::operator>>(QChar &c)
{
    CHECK_STREAM_PRECOND(this)
    c = d->ts_getc();
    return *this;
}


ulong QTextStreamPrivate::input_bin()
{
    uint l;
    ulong val = 0;
    const int buf_size = getnum_tmp_size;
    QChar buf[buf_size];
    while(1) {
        bool sr = d->ts_getbuf(buf, buf_size, TS_MOD_NOT|TS_BIN, &l);
        for(uint i = 0; i < l; i++)
            val = (val << 1) + buf[i].digitValue();
        if(sr)
            break;
    }
    return val;
}

ulong QTextStreamPrivate::input_oct()
{
    ulong val = 0;
    q->skipWhiteSpace();
    while(1) {
        QChar ch = ts_getc();
        int dv = ch.digitValue();
        if(dv < 0 && dv > 7) {
            if (ch.unicode() != QEOF)
                ts_ungetc(ch);
            break;
        } else if (dv == 8 || dv == 9) {
            while (ch.isDigit())
                ch = ts_getc();
            if (ch.unicode() != QEOF)
                ts_ungetc(ch);
        }
        val = (val << 3) + dv;
    }
    return val;
}

ulong QTextStreamPrivate::input_dec()
{
    uint l;
    ulong val = 0;
    const int buf_size = getnum_tmp_size;
    QChar buf[buf_size];
    while(1) {
        bool sr = d->ts_getbuf(buf, buf_size, TS_MOD_NOT|TS_DIGIT, &l);
        for(uint i = 0; i < l; i++)
            val = val * 10 + buf[i].digitValue();
        if(sr)
            break;
    }
    return val;
}

ulong QTextStreamPrivate::input_hex()
{
    uint l;
    ulong val = 0;
    const int buf_size = getnum_tmp_size;
    QChar buf[buf_size];
    while(1) {
        bool sr = d->ts_getbuf(buf, buf_size, TS_MOD_NOT|TS_HEX, &l);
        for(uint i = 0; i < l; i++) {
            char c = buf[i].toLower().latin1();
            val = (val << 4) + (buf[i].isDigit() ? c - '0' : 10 + c-'a');
        }
        if(sr)
            break;
    }
    return val;
}

long QTextStreamPrivate::input_int()
{
    long val=0;
    switch (q->flags() & q->basefield) {
    case QTextStream::bin:
        val = (long)input_bin();
        break;
    case QTextStream::oct:
        val = (long)input_oct();
        break;
    case QTextStream::dec: {
        q->skipWhiteSpace();
        QChar c = ts_getc();
        if(c.unicode() != QEOF) {
            if (c != '-' && c != '+')
                ts_ungetc(c);
            val = (long)input_dec();
            if (val && c == '-')
                val -= (val * 2);
        }
        break; }
    case QTextStream::hex:
        val = (long)input_hex();
        break;
    default: {
        q->skipWhiteSpace();
        QChar c = ts_getc();
        if (c == '0') {                // bin, oct or hex
            c = ts_getc();
            if (c.toLower() == 'x') {
                val = (long)input_hex();
            } else if (c.toLower() == 'b') {
                val = (long)input_bin();
            } else {                        // octal
                ts_ungetc(c);
                if (c >= '0' && c <= '7')
                    val = (long)input_oct();
            }
        } else if (c == '-' || c == '+') {
            val = (long)input_dec();
            if (val && c == '-')
                val -= (val * 2);
        } else if (c.isDigit()) {
            ts_ungetc(c);
            val = (long)input_dec();
        }
        break; }
    }
    return val;
}

//
// We use a table-driven FSM to parse floating point numbers
// strtod() cannot be used directly since we're reading from a QIODevice
//

double QTextStreamPrivate::input_double()
{
    const int Init         = 0;                        // states
    const int Sign         = 1;
    const int Mantissa         = 2;
    const int Dot         = 3;
    const int Abscissa         = 4;
    const int ExpMark         = 5;
    const int ExpSign         = 6;
    const int Exponent         = 7;
    const int Done         = 8;

    const int InputSign         = 1;                        // input tokens
    const int InputDigit = 2;
    const int InputDot         = 3;
    const int InputExp         = 4;

    static const uchar table[8][5] = {
     /* None         InputSign   InputDigit InputDot InputExp */
        { 0,            Sign,     Mantissa,         Dot,           0,           }, // Init
        { 0,            0,              Mantissa,         Dot,           0,           }, // Sign
        { Done,            Done,     Mantissa,         Dot,           ExpMark,}, // Mantissa
        { 0,            0,              Abscissa,         0,           0,           }, // Dot
        { Done,            Done,     Abscissa,         Done,           ExpMark,}, // Abscissa
        { 0,            ExpSign,  Exponent,         0,           0,           }, // ExpMark
        { 0,            0,              Exponent,         0,           0,           }, // ExpSign
        { Done,            Done,     Exponent,         Done,           Done           }  // Exponent
    };

    int state = Init;                                // parse state
    int input;                                        // input token

    const int buf_size = 128;
    char buf[buf_size];
    int i = 0;
    q->skipWhiteSpace();

    QChar c = ts_getc();
    for (;;) {

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
                input = 0;
                break;
        }

        state = table[state][input];

        if  (state == 0 || state == Done || i > (buf_size - 5)) {
            if (i > (buf_size - 5)) {        // ignore rest of digits
                do { c = ts_getc(); } while (c.unicode() != QEOF && c.isDigit());
            }
            if (c.unicode() != QEOF)
                ts_ungetc(c);
            buf[i] = '\0';
            char *end;
            return strtod(buf, &end);
        }

        buf[i++] = c.latin1();
        c = ts_getc();
    }

#if !defined(Q_CC_EDG)
    return 0.0;
#endif
}


/*!
    \overload

    Reads a signed \c short integer \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(signed short &i)
{
    CHECK_STREAM_PRECOND(this)
    i = (signed short)d->input_int();
    return *this;
}


/*!
    \overload

    Reads an unsigned \c short integer \a i from the stream and
    returns a reference to the stream. See flags() for an explanation
    of the expected input format.
*/

QTextStream &QTextStream::operator>>(unsigned short &i)
{
    CHECK_STREAM_PRECOND(this)
    i = (unsigned short)d->input_int();
    return *this;
}


/*!
    \overload

    Reads a signed \c int \a i from the stream and returns a reference
    to the stream. See flags() for an explanation of the expected
    input format.
*/

QTextStream &QTextStream::operator>>(signed int &i)
{
    CHECK_STREAM_PRECOND(this)
    i = (signed int)d->input_int();
    return *this;
}


/*!
    \overload

    Reads an unsigned \c int \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(unsigned int &i)
{
    CHECK_STREAM_PRECOND(this)
    i = (unsigned int)d->input_int();
    return *this;
}


/*!
    \overload

    Reads a signed \c long int \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(signed long &i)
{
    CHECK_STREAM_PRECOND(this)
    i = (signed long)d->input_int();
    return *this;
}


/*!
    \overload

    Reads an unsigned \c long int \a i from the stream and returns a
    reference to the stream. See flags() for an explanation of the
    expected input format.
*/

QTextStream &QTextStream::operator>>(unsigned long &i)
{
    CHECK_STREAM_PRECOND(this)
    i = (unsigned long)d->input_int();
    return *this;
}


/*!
    \overload

    Reads a \c float \a f from the stream and returns a reference to
    the stream. See flags() for an explanation of the expected input
    format.
*/

QTextStream &QTextStream::operator>>(float &f)
{
    CHECK_STREAM_PRECOND(this)
    f = (float)d->input_double();
    return *this;
}


/*!
    \overload

    Reads a \c double \a f from the stream and returns a reference to
    the stream. See flags() for an explanation of the expected input
    format.
*/

QTextStream &QTextStream::operator>>(double &f)
{
    CHECK_STREAM_PRECOND(this)
    f = d->input_double();
    return *this;
}


/*!
    \overload

    Reads a "word" from the stream into \a s and returns a reference
    to the stream.

    A word consists of characters for which isspace() returns false.
*/

QTextStream &QTextStream::operator>>(char *s)
{
    CHECK_STREAM_PRECOND(this)
    skipWhiteSpace();

    uint maxlen = width(0), l, total=0;
    const uint buf_size = getstr_tmp_size;
    QChar buf[buf_size];
    while(1) {
        bool sr = d->ts_getbuf(buf, buf_size, TS_SPACE, &l);
        for(uint i = 0; i < l; i++)
            *(s++) = buf[i].latin1();
        total += l;
        if(sr || (maxlen && total >= maxlen-1))
           break;
    }
    *s = '\0';
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
    CHECK_STREAM_PRECOND(this)
    str=QString::null;
    skipWhiteSpace();

    uint l;
    const uint buf_size = getstr_tmp_size;
    QChar buf[buf_size];
    while(1) {
        bool sr = d->ts_getbuf(buf, buf_size, TS_SPACE, &l);
        str.append(QString(buf, l));
        if(sr)
            break;
    }
    return *this;
}

/*!
    \overload

    Reads a "word" from the stream into \a str and returns a reference
    to the stream.

    A word consists of characters for which isspace() returns false.
*/

QTextStream &QTextStream::operator>>(QByteArray &str)
{
    CHECK_STREAM_PRECOND(this)
    skipWhiteSpace();

    uint used = 0, l;
    const uint buf_size = getstr_tmp_size;
    str.resize(buf_size);
    QChar buf[buf_size];
    while(1) {
        bool sr = d->ts_getbuf(buf, buf_size, TS_SPACE, &l);
        if(l) {
            if((int)(used+l) >= str.size())
                str.resize(used+l+1);
            for(uint i = 0; i < l; i++)
                str[(int)(used+i)] = buf[i].latin1();
            used += l;
        }
        if(sr)
            break;
    }
    str.resize(used);
    return *this;
}


/*!
    Reads a line from the stream and returns a string containing the
    text.

    The returned string does not contain any trailing newline or
    carriage return. Note that this is different from
    QIODevice::readLine(), which does not strip the newline at the end
    of the line.

    On EOF you will get a QString that is null. On reading an empty
    line the returned QString is empty but not null.

    \sa QIODevice::readLine()
*/

QString QTextStream::readLine()
{
    if (d->sourceType != QTextStreamPrivate::String && !d->dev) {
        qWarning("QTextStream::readLine: No device");
        return QString::null;
    }

    QString result;
    const int buf_size = getstr_tmp_size;
    QChar buf[buf_size];

    uint l;
    while(1) {
        bool sr = d->ts_getbuf(buf, buf_size, TS_EOL, &l);
        result.append(QString(buf, l));
        if(sr)
            break;
    }
    d->ts_getbuf(NULL, getstr_tmp_size, TS_MOD_NOT|TS_EOL);
    return result;
}


/*!
    Reads the entire stream and returns a string containing the text.

    \sa QIODevice::readLine()
*/

QString QTextStream::read()
{
    if (d->sourceType != QTextStreamPrivate::String && !d->dev) {
        qWarning("QTextStream::read: No device");
        return QString::null;
    }
    QString    result;
    const uint bufsize = 512;
    QChar      buf[bufsize];
    uint       i, num, start;
    bool       skipped_cr = false;

    while(1) {
        bool sr = d->ts_getbuf(buf, bufsize, 0, &num);
        // convert dos (\r\n) and mac (\r) style eol to unix style (\n)
        start = 0;
        for (i=0; i<num; i++) {
            if (buf[i] == '\r') {
                // Only skip single cr's preceding lf's
                if (skipped_cr) {
                    result += buf[i];
                    start++;
                } else {
                    result += QString(&buf[start], i-start);
                    start = i+1;
                    skipped_cr = true;
                }
            } else {
                if (skipped_cr) {
                    if (buf[i] != '\n') {
                        // Should not have skipped it
                        result += '\n';
                    }
                    skipped_cr = false;
                }
            }
        }
        if (start < num)
            result += QString(&buf[start], i-start);
        if (sr) // EOF
            break;
    }
    return result;
}



/*****************************************************************************
  QTextStream write functions
 *****************************************************************************/

/*!
    Writes character \c char to the stream and returns a reference to
    the stream.

    The character \a c is assumed to be Latin1 encoded independent of
    the Encoding set for the QTextStream.
*/
QTextStream &QTextStream::operator<<(QChar c)
{
    CHECK_STREAM_PRECOND(this)
    d->ts_putc(c);
    return *this;
}

/*!
    \overload

    Writes character \a c to the stream and returns a reference to the
    stream.
*/
QTextStream &QTextStream::operator<<(char c)
{
    CHECK_STREAM_PRECOND(this)
    unsigned char uc = (unsigned char) c;
    d->ts_putc(uc);
    return *this;
}

QTextStream &QTextStreamPrivate::output_int(int format, ulong n, bool neg)
{
    static const char hexdigits_lower[] = "0123456789abcdef";
    static const char hexdigits_upper[] = "0123456789ABCDEF";
    CHECK_STREAM_PRECOND(q)
    char buf[76];
    register char *p;
    int          len;
    const char *hexdigits;

    switch (q->flags() & I_BASE_MASK) {
        case I_BASE_2:                                // output binary number
            switch (format & I_TYPE_MASK) {
                case I_SHORT: len=16; break;
                case I_INT:   len=sizeof(int)*8; break;
                case I_LONG:  len=32; break;
                default:      len = 0;
            }
            p = &buf[74];                        // go reverse order
            *p = '\0';
            while (len--) {
                *--p = (char)(n&1) + '0';
                n >>= 1;
                if (!n)
                    break;
            }
            if (q->flags() & q->showbase) {                // show base
                *--p = (q->flags() & q->uppercase) ? 'B' : 'b';
                *--p = '0';
            }
            break;

        case I_BASE_8:                                // output octal number
            p = &buf[74];
            *p = '\0';
            do {
                *--p = (char)(n&7) + '0';
                n >>= 3;
            } while (n);
            if (q->flags() & q->showbase)
                *--p = '0';
            break;

        case I_BASE_16:                                // output hexadecimal number
            p = &buf[74];
            *p = '\0';
            hexdigits = (q->flags() & q->uppercase) ?
                hexdigits_upper : hexdigits_lower;
            do {
                *--p = hexdigits[(int)n&0xf];
                n >>= 4;
            } while (n);
            if (q->flags() & q->showbase) {
                *--p = (q->flags() & q->uppercase) ? 'X' : 'x';
                *--p = '0';
            }
            break;

        default:                                // decimal base is default
            p = &buf[74];
            *p = '\0';
            if (neg)
                n = (ulong)(-(long)n);
            do {
                *--p = ((int)(n%10)) + '0';
                n /= 10;
            } while (n);
            if (neg)
                *--p = '-';
            else if (q->flags() & q->showpos)
                *--p = '+';
            if ((q->flags() & q->internal) && d->fwidth && !QChar(*p).isDigit()) {
                d->ts_putc(*p);                        // special case for internal
                ++p;                                //   padding
                d->fwidth--;
                return *q << (const char*)p;
            }
    }
    if (d->fwidth) {                                // adjustment required
        if (!(q->flags() & q->left)) {                // but NOT left adjustment
            len = qstrlen(p);
            int padlen = d->fwidth - len;
            if (padlen <= 0) {                // no padding required
                writeBlock(p, len);
            } else if (padlen < (int)(p-buf)) { // speeds up padding
                memset(p-padlen, (char)d->fillchar, padlen);
                writeBlock(p-padlen, padlen+len);
            }
            else                                // standard padding
                *q << (const char*)p;
        }
        else
            *q << (const char*)p;
        d->fwidth = 0;                                // reset field width
    }
    else
        writeBlock(p, qstrlen(p));
    return *q;
}


/*!
    \overload

    Writes a \c short integer \a i to the stream and returns a
    reference to the stream.
*/

QTextStream &QTextStream::operator<<(signed short i)
{
    return d->output_int(I_SHORT | I_SIGNED, i, i < 0);
}


/*!
    \overload

    Writes an \c unsigned \c short integer \a i to the stream and
    returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<(unsigned short i)
{
    return d->output_int(I_SHORT | I_UNSIGNED, i, false);
}


/*!
    \overload

    Writes an \c int \a i to the stream and returns a reference to the
    stream.
*/

QTextStream &QTextStream::operator<<(signed int i)
{
    return d->output_int(I_INT | I_SIGNED, i, i < 0);
}


/*!
    \overload

    Writes an \c unsigned \c int \a i to the stream and returns a
    reference to the stream.
*/

QTextStream &QTextStream::operator<<(unsigned int i)
{
    return d->output_int(I_INT | I_UNSIGNED, i, false);
}


/*!
    \overload

    Writes a \c long \c int \a i to the stream and returns a reference
    to the stream.
*/

QTextStream &QTextStream::operator<<(signed long i)
{
    return d->output_int(I_LONG | I_SIGNED, i, i < 0);
}


/*!
    \overload

    Writes an \c unsigned \c long \c int \a i to the stream and
    returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<(unsigned long i)
{
    return d->output_int(I_LONG | I_UNSIGNED, i, false);
}


/*!
    \overload

    Writes a \c float \a f to the stream and returns a reference to
    the stream.
*/

QTextStream &QTextStream::operator<<(float f)
{
    return *this << (double)f;
}

/*!
    \overload

    Writes a \c double \a f to the stream and returns a reference to
    the stream.
*/

QTextStream &QTextStream::operator<<(double f)
{
    CHECK_STREAM_PRECOND(this)
    char f_char;
    char format[16];
    if ((flags()&floatfield) == fixed)
        f_char = 'f';
    else if ((flags()&floatfield) == scientific)
        f_char = (flags() & uppercase) ? 'E' : 'e';
    else
        f_char = (flags() & uppercase) ? 'G' : 'g';
    register char *fs = format;                        // generate format string
    *fs++ = '%';                                //   "%.<prec>l<f_char>"
    *fs++ = '.';
    int prec = precision();
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
    if (d->fwidth)                                // padding
        *this << num.latin1();
    else                                        // just write it
        d->writeBlock(num.latin1(), num.length());
    return *this;
}


/*!
    \overload

    Writes a string to the stream and returns a reference to the
    stream.

    The string \a s is assumed to be Latin1 encoded independent of the
    Encoding set for the QTextStream.
*/

QTextStream &QTextStream::operator<<(const char* s)
{
    CHECK_STREAM_PRECOND(this)
    char padbuf[48];
    uint len = qstrlen(s);                        // don't write null terminator
    if (d->fwidth) {                                // field width set
        int padlen = d->fwidth - len;
        d->fwidth = 0;                                // reset width
        if (padlen > 0) {
            char *ppad;
            if (padlen > 46) {                // create extra big fill buffer
                ppad = new char[padlen];
            } else {
                ppad = padbuf;
            }
            memset(ppad, (char)d->fillchar, padlen);        // fill with d->fillchar
            if (!(flags() & left)) {
                d->writeBlock(ppad, padlen);
                padlen = 0;
            }
            d->writeBlock(s, len);
            if (padlen)
                d->writeBlock(ppad, padlen);
            if (ppad != padbuf)                // delete extra big fill buf
                delete[] ppad;
            return *this;
        }
    }
    d->writeBlock(s, len);
    return *this;
}

/*!
    \overload

    Writes \a s to the stream and returns a reference to the stream.

    The string \a s is assumed to be Latin1 encoded independent of the
    Encoding set for the QTextStream.
*/

QTextStream &QTextStream::operator<<(const QByteArray & s)
{
    return operator<<(s.constData());
}

/*!
    \overload

    Writes \a s to the stream and returns a reference to the stream.
*/

QTextStream &QTextStream::operator<<(const QString& s)
{
    if (!d->mapper && d->latin1)
        return operator<<(s.latin1());
    CHECK_STREAM_PRECOND(this)
    QString s1 = s;
    if (d->fwidth) {                                // field width set
        if ((flags() & left))
            s1 = s.leftJustified(d->fwidth, (char)d->fillchar);
        else
            s1 = s.rightJustified(d->fwidth, (char)d->fillchar);
        d->fwidth = 0;                                // reset width
    }
    d->writeBlock(s1.unicode(), s1.length());
    return *this;
}


/*!
    \overload

    Writes a pointer to the stream and returns a reference to the
    stream.

    The \a ptr is output as an unsigned long hexadecimal integer.
*/

QTextStream &QTextStream::operator<<(const void *ptr)
{
    int f = flags();
    setf(hex, basefield);
    setf(showbase);
    unsetf(uppercase);
    d->output_int(I_LONG | I_UNSIGNED, (ulong)ptr, false);
    flags(f);
    return *this;
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
    return d->d->fflags;
}


/*!
    \overload

    Sets the stream flags to \a f. Returns the previous stream flags.

    \sa setf(), unsetf(), flags()
*/

int QTextStream::flags(int f)
{
    int oldf = d->fflags;
    d->fflags = f;
    return oldf;
}


/*!
    Sets the stream flag bits \a bits. Returns the previous stream
    flags.

    Equivalent to \c{flags(flags() | bits)}.

    \sa setf(), unsetf()
*/

int QTextStream::setf(int bits)
{
    int oldf = d->fflags;
    d->fflags |= bits;
    return oldf;
}


/*!
    \overload

    Sets the stream flag bits \a bits with a bit mask \a mask. Returns
    the previous stream flags.

    Equivalent to \c{flags((flags() & ~mask) | (bits & mask))}.

    \sa setf(), unsetf()
*/

int QTextStream::setf(int bits, int mask)
{
    int oldf = d->fflags;
    d->fflags = (d->fflags & ~mask) | (bits & mask);
    return oldf;
}


/*!
    Clears the stream flag bits \a bits. Returns the previous stream
    flags.

    Equivalent to \c{flags(flags() & ~mask)}.

    \sa setf()
*/

int QTextStream::unsetf(int bits)
{
    int oldf = d->fflags;
    d->fflags &= ~bits;
    return oldf;
}


/*!
    Returns the field width. The default value is 0.
*/

int QTextStream::width() const
{
    return d->fwidth;
}


/*!
    \overload

    Sets the field width to \a w. Returns the previous field width.
*/

int QTextStream::width(int w)
{
    int oldw = d->fwidth;
    d->fwidth = w;
    return oldw;
}


/*!
    Returns the fill character. The default value is ' ' (space).
*/

int QTextStream::fill() const
{
    return d->fillchar;
}


/*!
    \overload

    Sets the fill character to \a f. Returns the previous fill character.
*/

int QTextStream::fill(int f)
{
    int oldc = d->fillchar;
    d->fillchar = f;
    return oldc;
}

/*!
    Returns the precision. The default value is 6.
*/

int QTextStream::precision() const
{
    return d->fprec;
}

/*!
    \overload

    Sets the precision to \a p. Returns the previous precision setting.
*/

int QTextStream::precision(int p)
{
    int oldp = d->fprec;
    d->fprec = p;
    return oldp;
}

 /*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

QTextStream &bin(QTextStream &s)
{
    s.setf(QTS::bin,QTS::basefield);
    return s;
}

QTextStream &oct(QTextStream &s)
{
    s.setf(QTS::oct,QTS::basefield);
    return s;
}

QTextStream &dec(QTextStream &s)
{
    s.setf(QTS::dec,QTS::basefield);
    return s;
}

QTextStream &hex(QTextStream &s)
{
    s.setf(QTS::hex,QTS::basefield);
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



/*!
    Sets the encoding of this stream to \a e, where \a e is one of the
    following values:
  \table
  \header \i Encoding \i Meaning
  \row \i Locale
       \i Uses the local file format (Latin1 if locale is not set),
       but autodetecting Unicode(utf16) on input.
  \row \i Unicode
       \i Uses Unicode(utf16) for input and output. Output will be
       written in the order most efficient for the current platform
       (i.e. the order used internally in QString).
  \row \i UnicodeUTF8
       \i Uses Unicode(utf8) for input and output. If you use it for
       input it will autodetect utf16 and use it instead of utf8.
  \row \i Latin1
       \i ISO-8859-1. Will not autodetect utf16.
  \row \i UnicodeNetworkOrder
       \i Uses network order Unicode(utf16) for input and output.
       Useful when reading Unicode data that does not start with the
       byte order marker.
  \row \i UnicodeReverse
       \i Uses reverse network order Unicode(utf16) for input and
       output. Useful when reading Unicode data that does not start
       with the byte order marker or when writing data that should be
       read by buggy Windows applications.
  \row \i RawUnicode
       \i Like Unicode, but does not write the byte order marker nor
       does it auto-detect the byte order. Only useful when writing to
       non-persistent storage used by a single process.
  \endtable

    \c Locale and all Unicode encodings, except \c RawUnicode, will
    look at the first two bytes in an input stream to determine the
    byte order. The initial byte order marker will be stripped off
    before data is read.

    Note that this function should be called before any data is read
    to or written from the stream.

    \sa setCodec()
*/

void QTextStream::setEncoding(Encoding e)
{
    if (d->sourceType == QTextStreamPrivate::String)
        return;

    switch (e) {
    case Unicode:
        d->mapper = 0;
        d->latin1 = false;
        d->doUnicodeHeader = true;
        d->internalOrder = true;
        d->networkOrder = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
        break;
    case UnicodeUTF8:
#ifndef QT_NO_TEXTCODEC
        d->mapper = QTextCodec::codecForMib(106);
        d->latin1 = false;
        d->doUnicodeHeader = true;
        d->internalOrder = true;
        d->networkOrder = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
#else
        d->mapper = 0;
        d->latin1 = true;
        d->doUnicodeHeader = true;
#endif
        break;
    case UnicodeNetworkOrder:
        d->mapper = 0;
        d->latin1 = false;
        d->doUnicodeHeader = true;
        d->internalOrder = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
        d->networkOrder = true;
        break;
    case UnicodeReverse:
        d->mapper = 0;
        d->latin1 = false;
        d->doUnicodeHeader = true;
        d->internalOrder = (QSysInfo::ByteOrder != QSysInfo::BigEndian);
        d->networkOrder = false;
        break;
    case RawUnicode:
        d->mapper = 0;
        d->latin1 = false;
        d->doUnicodeHeader = false;
        d->internalOrder = true;
        d->networkOrder = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
        break;
    case Locale:
        d->latin1 = true; // fallback to Latin1
#ifndef QT_NO_TEXTCODEC
        d->mapper = QTextCodec::codecForLocale();
        // optimized Latin1 processing
#if defined(Q_OS_WIN32)
        if (GetACP() == 1252)
            d->mapper = 0;
#endif
        if (d->mapper && d->mapper->mibEnum() == 4)
#endif
            d->mapper = 0;

        d->doUnicodeHeader = true; // If it reads as Unicode, accept it
        break;
    case Latin1:
        d->mapper = 0;
        d->doUnicodeHeader = false;
        d->latin1 = true;
        break;
    }
}


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
    if (d->sourceType == QTextStreamPrivate::String)
        return; // QString does not need any codec
    d->mapper = codec;
    d->latin1 = (codec->mibEnum() == 4);
    if (d->latin1)
        d->mapper = 0;
    d->doUnicodeHeader = false;
}

/*!
    Returns the codec actually used for this stream.

    If Unicode is automatically detected on input, a codec with \link
    QTextCodec::name() name() \endlink "ISO-10646-UCS-2" is returned.

    \sa setCodec()
*/

QTextCodec *QTextStream::codec()
{
    if (d->mapper) {
        return d->mapper;
    } else {
        // 4 is "ISO 8859-1", 1000 is "ISO-10646-UCS-2"
        return QTextCodec::codecForMib(d->latin1 ? 4 : 1000);
    }
}

#endif

bool QTextStream::atEnd() const
{
    //just append directly onto the string (optimization)
    if (d->sourceType == QTextStreamPrivate::String)
        return d->strOff == (d->str->length()*sizeof(QChar));
    //device
    return ((!d->dev || d->dev->atEnd()) &&
            d->cacheReadBuf.isEmpty() && d->ungetcBuf.isEmpty());
}

/*!
  Returns one character from the stream, or EOF.
*/

QChar QTextStreamPrivate::ts_getc()
{
    QChar r;
    uint l;
    d->ts_getbuf(&r, 1, 0, &l);
    if (!l)
        r = QChar(0xffff);
    return r;
}

#endif // QT_NO_TEXTSTREAM

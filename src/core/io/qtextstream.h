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

#ifndef QTEXTSTREAM_H
#define QTEXTSTREAM_H

#include "QtCore/qiodevice.h"
#include "QtCore/qstring.h"
#include "QtCore/qchar.h"

#ifndef QT_NO_TEXTCODEC
#ifdef QT_COMPAT
#include "QtCore/qtextcodec.h"
#endif
#endif

#include <stdio.h>

class QTextCodec;
class QTextDecoder;

class QTextStreamPrivate;
class Q_CORE_EXPORT QTextStream                                // text stream class
{
    Q_DECLARE_PRIVATE(QTextStream)

public:
    QTextStream();
    explicit QTextStream(QIODevice *device);
    explicit QTextStream(FILE *fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit QTextStream(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit QTextStream(QByteArray *array, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit QTextStream(const QByteArray &array, QIODevice::OpenMode openMode = QIODevice::ReadOnly);
    virtual ~QTextStream();

#ifndef QT_NO_TEXTCODEC
    void setCodec(QTextCodec *codec);
    QTextCodec *codec() const;
    void setAutoDetectUnicode(bool enabled);
    bool autoDetectUnicode() const;
#endif

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setString(QString *string);
    QString *string() const;

    bool atEnd() const;
    void reset();
    void flush();
    bool seek(qint64 offset);

    void skipWhiteSpace();

    QString readLine(qint64 maxlen = 0);
    QString readAll();

    QTextStream &operator>>(QChar &ch);
    QTextStream &operator>>(char &ch);
    QTextStream &operator>>(signed short &i);
    QTextStream &operator>>(unsigned short &i);
    QTextStream &operator>>(signed int &i);
    QTextStream &operator>>(unsigned int &i);
    QTextStream &operator>>(signed long &i);
    QTextStream &operator>>(unsigned long &i);
    QTextStream &operator>>(qlonglong &i);
    QTextStream &operator>>(qulonglong &i);
    QTextStream &operator>>(float &f);
    QTextStream &operator>>(double &f);
    QTextStream &operator>>(char *c);
    QTextStream &operator>>(QString &s);
    QTextStream &operator>>(QByteArray &array);

    QTextStream &operator<<(QChar ch);
    QTextStream &operator<<(char ch);
    QTextStream &operator<<(signed short i);
    QTextStream &operator<<(unsigned short i);
    QTextStream &operator<<(signed int i);
    QTextStream &operator<<(unsigned int i);
    QTextStream &operator<<(signed long i);
    QTextStream &operator<<(unsigned long i);
    QTextStream &operator<<(qlonglong i);
    QTextStream &operator<<(qulonglong i);
    QTextStream &operator<<(float f);
    QTextStream &operator<<(double f);
    QTextStream &operator<<(const char *c);
    QTextStream &operator<<(const QString &s);
    QTextStream &operator<<(const QByteArray &array);
    QTextStream &operator<<(const void *ptr);

#ifdef QT_USE_FIXED_POINT
    inline QTextStream &operator>>(QFixedPoint &f) { double d; operator>>(d); f = d; return *this; }
    inline QTextStream &operator<<(QFixedPoint f) { return operator<<(f.toDouble()); }
#endif

    enum RealNumberMode {
        Floating = 0,
        Fixed = 1,
        Scientific = 2
    };
    enum PadMode {
        NoPadding = 0,
        PadLeft = 1,
        PadRight = 2,
        PadCentered = 3
        // ### PadInternal "+      1.43"
    };
    enum NumberDisplayFlag {
        ShowBase = 0x1,
        ShowPoint = 0x2,
        ShowSign = 0x4,
        UppercaseBase = 0x8
    };
    Q_DECLARE_FLAGS(NumberDisplayFlags, NumberDisplayFlag)

    void setPadMode(PadMode mode);
    PadMode padMode() const;

    void setPadChar(QChar ch);
    QChar padChar() const;

    void setPadWidth(int width);
    int padWidth() const;

    void setNumberDisplayFlags(NumberDisplayFlags flags);
    NumberDisplayFlags numberDisplayFlags() const;

    void setNumberBase(int base);
    int numberBase() const;

    void setRealNumberMode(RealNumberMode mode);
    RealNumberMode realNumberMode() const;

    void setPrecision(int p);
    int precision() const;

#ifdef QT_COMPAT
    // not marked as QT_COMPAT to avoid double compiler warnings, as
    // they are used in the QT_COMPAT functions below.
    inline QT_COMPAT int flags() const { return flagsInternal(); }
    inline QT_COMPAT int flags(int f) { return flagsInternal(f); }

    inline QT_COMPAT int setf(int bits)
    { int old = flagsInternal(); flagsInternal(flagsInternal() | bits); return old; }
    inline QT_COMPAT int setf(int bits, int mask)
    { int old = flagsInternal(); flagsInternal(flagsInternal() | (bits & mask)); return old; }
    inline QT_COMPAT int unsetf(int bits)
    { int old = flagsInternal(); flagsInternal(flagsInternal() & ~bits); return old; }

    inline QT_COMPAT int width(int w)
    { int old = padWidth(); setPadWidth(w); return old; }
    inline QT_COMPAT int fill(int f)
    { QChar ch = padChar(); setPadChar(QChar(f)); return ch.unicode(); }
    inline QT_COMPAT int precision(int p)
    { int old = precision(); setPrecision(p); return old; }

    enum {
        skipws       = 0x0001,                        // skip whitespace on input
        left         = 0x0002,                        // left-adjust output
        right        = 0x0004,                        // right-adjust output
        internal     = 0x0008,                        // pad after sign
        bin          = 0x0010,                        // binary format integer
        oct          = 0x0020,                        // octal format integer
        dec          = 0x0040,                        // decimal format integer
        hex          = 0x0080,                        // hex format integer
        showbase     = 0x0100,                        // show base indicator
        showpoint    = 0x0200,                        // force decimal point (float)
        uppercase    = 0x0400,                        // upper-case hex output
        showpos      = 0x0800,                        // add '+' to positive integers
        scientific   = 0x1000,                        // scientific float output
        fixed        = 0x2000                         // fixed float output
    };
    enum {
        basefield = bin | oct | dec | hex,
        adjustfield = left | right | internal,
        floatfield = scientific | fixed
    };

    enum Encoding { Locale, Latin1, Unicode, UnicodeNetworkOrder,
                    UnicodeReverse, RawUnicode, UnicodeUTF8 };
    QT_COMPAT void setEncoding(Encoding encoding);
    inline QT_COMPAT QString read() { return readAll(); }
    inline QT_COMPAT void unsetDevice() { setDevice(0); }
#endif

private:
#ifdef QT_COMPAT
    int flagsInternal() const;
    int flagsInternal(int flags);
#endif

    Q_DISABLE_COPY(QTextStream)

    QTextStreamPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextStream::NumberDisplayFlags)

/*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

typedef QTextStream & (*QTextStreamFunction)(QTextStream &);// manipulator function
typedef void (QTextStream::*QTSMFI)(int); // manipulator w/int argument
typedef void (QTextStream::*QTSMFC)(QChar); // manipulator w/QChar argument

class Q_CORE_EXPORT QTextStreamManipulator
{
public:
    QTextStreamManipulator(QTSMFI m, int a) { mf = m; mc = 0; arg = a; }
    QTextStreamManipulator(QTSMFC m, QChar c) { mf = 0; mc = m; ch = c; }
    void exec(QTextStream &s) { if (mf) { (s.*mf)(arg); } else { (s.*mc)(ch); } }

private:
    QTSMFI mf;                                        // QTextStream member function
    QTSMFC mc;                                        // QTextStream member function
    int arg;                                          // member function argument
    QChar ch;
};

inline QTextStream &operator>>(QTextStream &s, QTextStreamFunction f)
{ return (*f)(s); }

inline QTextStream &operator<<(QTextStream &s, QTextStreamFunction f)
{ return (*f)(s); }

inline QTextStream &operator<<(QTextStream &s, QTextStreamManipulator m)
{ m.exec(s); return s; }

Q_CORE_EXPORT QTextStream &bin(QTextStream &s);        // set bin notation
Q_CORE_EXPORT QTextStream &oct(QTextStream &s);        // set oct notation
Q_CORE_EXPORT QTextStream &dec(QTextStream &s);        // set dec notation
Q_CORE_EXPORT QTextStream &hex(QTextStream &s);        // set hex notation

Q_CORE_EXPORT QTextStream &showbase(QTextStream &s);
Q_CORE_EXPORT QTextStream &showsign(QTextStream &s);
Q_CORE_EXPORT QTextStream &showpoint(QTextStream &s);
Q_CORE_EXPORT QTextStream &uppercasebase(QTextStream &s);

Q_CORE_EXPORT QTextStream &fixed(QTextStream &s);
Q_CORE_EXPORT QTextStream &scientific(QTextStream &s);
Q_CORE_EXPORT QTextStream &floating(QTextStream &s);

Q_CORE_EXPORT QTextStream &padleft(QTextStream &s);
Q_CORE_EXPORT QTextStream &padright(QTextStream &s);
Q_CORE_EXPORT QTextStream &padcentered(QTextStream &s);

Q_CORE_EXPORT QTextStream &endl(QTextStream &s);        // insert EOL ('\n')
Q_CORE_EXPORT QTextStream &flush(QTextStream &s);        // flush output
Q_CORE_EXPORT QTextStream &ws(QTextStream &s);        // eat whitespace on input
Q_CORE_EXPORT QTextStream &reset(QTextStream &s);        // set default flags

inline QTextStreamManipulator qSetW(int w)
{
    QTSMFI func = &QTextStream::setPadWidth;
    return QTextStreamManipulator(func,w);
}

inline QTextStreamManipulator qSetFill(QChar ch)
{
    QTSMFC func = &QTextStream::setPadChar;
    return QTextStreamManipulator(func,ch);
}

inline QTextStreamManipulator qSetPrecision(int p)
{
    QTSMFI func = &QTextStream::setPrecision;
    return QTextStreamManipulator(func,p);
}

#ifdef QT_COMPAT
typedef QTextStream QTS;

class Q_CORE_EXPORT QTextIStream : public QTextStream
{
public:
    explicit QTextIStream(const QString *s) : QTextStream(const_cast<QString *>(s), QIODevice::ReadOnly) {}
    explicit QTextIStream(QByteArray *a) : QTextStream(a, QIODevice::ReadOnly) {}
    QTextIStream(FILE *f) : QTextStream(f, QIODevice::ReadOnly) {}

private:
    Q_DISABLE_COPY(QTextIStream)
};
class Q_CORE_EXPORT QTextOStream : public QTextStream
{
public:
    explicit QTextOStream(QString *s) : QTextStream(s, QIODevice::WriteOnly) {}
    explicit QTextOStream(QByteArray *a) : QTextStream(a, QIODevice::WriteOnly) {}
    QTextOStream(FILE *f) : QTextStream(f, QIODevice::WriteOnly) {}

private:
    Q_DISABLE_COPY(QTextOStream)
};
#endif


#endif // QTEXTSTREAM_H

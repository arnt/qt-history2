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
    QTextStream(FILE *fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    QTextStream(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    QTextStream(QByteArray *array, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    QTextStream(const QByteArray &array, QIODevice::OpenMode openMode = QIODevice::ReadOnly);
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

    QString readLine(qint64 maxlen = 0);
    QString readAll();

    QTextStream &operator>>(QChar &);
    QTextStream &operator>>(char &);
    QTextStream &operator>>(signed short &);
    QTextStream &operator>>(unsigned short &);
    QTextStream &operator>>(signed int &);
    QTextStream &operator>>(unsigned int &);
    QTextStream &operator>>(signed long &);
    QTextStream &operator>>(unsigned long &);
    QTextStream &operator>>(qlonglong&);
    QTextStream &operator>>(qulonglong&);

    QTextStream &operator>>(float &);
    QTextStream &operator>>(double &);
#ifdef QT_USE_FIXED_POINT
    inline QTextStream &operator>>(QFixedPoint &f) { double d; operator>>(d); f = d; return *this; }
#endif
    QTextStream &operator>>(char *);
    QTextStream &operator>>(QString &);
    QTextStream &operator>>(QByteArray &);

    QTextStream &operator<<(QChar);
    QTextStream &operator<<(char);
    QTextStream &operator<<(signed short);
    QTextStream &operator<<(unsigned short);
    QTextStream &operator<<(signed int);
    QTextStream &operator<<(unsigned int);
    QTextStream &operator<<(signed long);
    QTextStream &operator<<(unsigned long);
    QTextStream &operator<<(qlonglong);
    QTextStream &operator<<(qulonglong);
    QTextStream &operator<<(float);
    QTextStream &operator<<(double);
#ifdef QT_USE_FIXED_POINT
    inline QTextStream &operator<<(QFixedPoint f) { return operator<<(f.toDouble()); }
#endif
    QTextStream &operator<<(const char *);
    QTextStream &operator<<(const QString &);
    QTextStream &operator<<(const QByteArray &);
    QTextStream &operator<<(const void *);                // any pointer

    void skipWhiteSpace();
    bool seek(qint64 offset);

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

    int flags() const;
    void setFlags(int f);
    int setf(int bits);
    int setf(int bits, int mask);
    int unsetf(int bits);

    void reset();
    void flush();

    int width() const;
    void setWidth(int w);
    int fill() const;
    void setFill(int ch);
    int precision() const;
    void setPrecision(int p);

#ifdef QT_COMPAT
    enum Encoding { Locale, Latin1, Unicode, UnicodeNetworkOrder,
                    UnicodeReverse, RawUnicode, UnicodeUTF8 };
    QT_COMPAT void setEncoding(Encoding encoding);
    inline QT_COMPAT QString read() { return readAll(); }
    inline QT_COMPAT void unsetDevice() { setDevice(0); }
    inline QT_COMPAT int flags(int f) { int old = flags(); setFlags(f); return old; }
    inline QT_COMPAT int width(int w) { int old = width(); setWidth(w); return old; }
    inline QT_COMPAT int fill(int f) { int old = fill(); setFill(f); return old; }
    inline QT_COMPAT int precision(int p) { int old = precision(); setPrecision(p); return old; }
#endif

private:
    Q_DISABLE_COPY(QTextStream)

    void init();
    QTextStreamPrivate *d_ptr;
};

#ifdef QT_COMPAT
typedef QTextStream QTS;
#endif

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

/*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

typedef QTextStream & (*QTextStreamFunction)(QTextStream &);// manipulator function
typedef void (QTextStream::*QTSMFI)(int);        // manipulator w/int argument

class Q_CORE_EXPORT QTextStreamManipulator
{
public:
    QTextStreamManipulator(QTSMFI m, int a) { mf = m; arg = a; }
    void exec(QTextStream &s) { (s.*mf)(arg); }

private:
    QTSMFI mf;                                        // QTextStream member function
    int arg;                                          // member function argument
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
Q_CORE_EXPORT QTextStream &endl(QTextStream &s);        // insert EOL ('\n')
Q_CORE_EXPORT QTextStream &flush(QTextStream &s);        // flush output
Q_CORE_EXPORT QTextStream &ws(QTextStream &s);        // eat whitespace on input
Q_CORE_EXPORT QTextStream &reset(QTextStream &s);        // set default flags

inline QTextStreamManipulator qSetW(int w)
{
    QTSMFI func = &QTextStream::setWidth;
    return QTextStreamManipulator(func,w);
}

inline QTextStreamManipulator qSetFill(int ch)
{
    QTSMFI func = &QTextStream::setFill;
    return QTextStreamManipulator(func,ch);
}

inline QTextStreamManipulator qSetPrecision(int p)
{
    QTSMFI func = &QTextStream::setPrecision;
    return QTextStreamManipulator(func,p);
}

#endif // QTEXTSTREAM_H

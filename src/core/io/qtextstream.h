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

#include "qiodevice.h"
#include "qstring.h"
#include "qchar.h"

#include <stdio.h>

#ifndef QT_NO_TEXTSTREAM
class QTextCodec;
class QTextDecoder;

class QTextStreamPrivate;

class Q_CORE_EXPORT QTextStream                                // text stream class
{
    Q_DECLARE_PRIVATE(QTextStream)

public:
    enum Encoding { Locale, Latin1, Unicode, UnicodeNetworkOrder,
                    UnicodeReverse, RawUnicode, UnicodeUTF8 };

    void setEncoding(Encoding);
#ifndef QT_NO_TEXTCODEC
    void setCodec(QTextCodec*);
    QTextCodec *codec();
#endif

    QTextStream();
    QTextStream(QIODevice *);
    QTextStream(QString *, int mode);
    QTextStream(QByteArray *, int mode);
    QTextStream(const QByteArray &, int mode);
    QTextStream(FILE *, int mode);
    virtual ~QTextStream();

    QIODevice *device() const;
    void setDevice(QIODevice *);
    void unsetDevice();

    bool         atEnd() const;
    inline bool eof() const { return atEnd(); }

    QTextStream &operator>>(QChar &);
    QTextStream &operator>>(char &);
    QTextStream &operator>>(signed short &);
    QTextStream &operator>>(unsigned short &);
    QTextStream &operator>>(signed int &);
    QTextStream &operator>>(unsigned int &);
    QTextStream &operator>>(signed long &);
    QTextStream &operator>>(unsigned long &);
    QTextStream &operator>>(float &);
    QTextStream &operator>>(double &);
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
    QTextStream &operator<<(float);
    QTextStream &operator<<(double);
    QTextStream &operator<<(const char*);
    QTextStream &operator<<(const QString &);
    QTextStream &operator<<(const QByteArray &);
    QTextStream &operator<<(const void *);                // any pointer

    QTextStream &readRawBytes(char *, Q_LONGLONG len);
    QTextStream &writeRawBytes(const char* , Q_LONGLONG len);

    QString readLine();
    QString read();
    void skipWhiteSpace();
    bool seek(Q_LONGLONG offset);

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

    static const int basefield;    // bin | oct | dec | hex
    static const int adjustfield;  // left | right | internal
    static const int floatfield;   // scientific | fixed

    int flags() const;
    void setFlags(int f);
    int setf(int bits);
    int setf(int bits, int mask);
    int unsetf(int bits);

    void  reset();

    int width() const;
    void setWidth(int w);
    int fill() const;
    void setFill(int f);
    int precision() const;
    void setPrecision(int p);

#ifdef QT_COMPAT
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
    QTextIStream(const QString *s) : QTextStream(const_cast<QString *>(s), QIODevice::ReadOnly) {}
    QTextIStream(QByteArray *a) : QTextStream(a, QIODevice::ReadOnly) {}
    QTextIStream(FILE *f) : QTextStream(f, QIODevice::ReadOnly) {}

private:
    Q_DISABLE_COPY(QTextIStream)
};

class Q_CORE_EXPORT QTextOStream : public QTextStream
{
public:
    QTextOStream(QString *s) : QTextStream(s, QIODevice::WriteOnly) {}
    QTextOStream(QByteArray *a) : QTextStream(a, QIODevice::WriteOnly) {}
    QTextOStream(FILE *f) : QTextStream(f, QIODevice::WriteOnly) {}

private:
    Q_DISABLE_COPY(QTextOStream)
};

/*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

typedef QTextStream & (*QTSFUNC)(QTextStream &);// manipulator function
typedef void (QTextStream::*QTSMFI)(int);        // manipulator w/int argument

class Q_CORE_EXPORT QTSManip
{
public:
    QTSManip(QTSMFI m, int a) { mf = m; arg = a; }
    void exec(QTextStream &s) { (s.*mf)(arg); }

private:
    QTSMFI mf;                                        // QTextStream member function
    int arg;                                          // member function argument
};

inline QTextStream &operator>>(QTextStream &s, QTSFUNC f)
{ return (*f)(s); }

inline QTextStream &operator<<(QTextStream &s, QTSFUNC f)
{ return (*f)(s); }

inline QTextStream &operator<<(QTextStream &s, QTSManip m)
{ m.exec(s); return s; }

Q_CORE_EXPORT QTextStream &bin(QTextStream &s);        // set bin notation
Q_CORE_EXPORT QTextStream &oct(QTextStream &s);        // set oct notation
Q_CORE_EXPORT QTextStream &dec(QTextStream &s);        // set dec notation
Q_CORE_EXPORT QTextStream &hex(QTextStream &s);        // set hex notation
Q_CORE_EXPORT QTextStream &endl(QTextStream &s);        // insert EOL ('\n')
Q_CORE_EXPORT QTextStream &flush(QTextStream &s);        // flush output
Q_CORE_EXPORT QTextStream &ws(QTextStream &s);        // eat whitespace on input
Q_CORE_EXPORT QTextStream &reset(QTextStream &s);        // set default flags

inline QTSManip qSetW(int w)
{
    QTSMFI func = &QTextStream::setWidth;
    return QTSManip(func,w);
}

inline QTSManip qSetFill(int f)
{
    QTSMFI func = &QTextStream::setFill;
    return QTSManip(func,f);
}

inline QTSManip qSetPrecision(int p)
{
    QTSMFI func = &QTextStream::setPrecision;
    return QTSManip(func,p);
}

#endif // QT_NO_TEXTSTREAM
#endif // QTEXTSTREAM_H

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

#ifndef QSTRING_H
#define QSTRING_H

#include "QtCore/qchar.h"
#include "QtCore/qbytearray.h"
#include "QtCore/qatomic.h"
#include "QtCore/qnamespace.h"


#ifdef QT_INCLUDE_COMPAT
#include <qcstring.h>
#endif

#ifndef QT_NO_STL
# if defined (Q_CC_MSVC_NET) && _MSV_VER < 1310 // Avoids nasty warning for xlocale, line 450
#  pragma warning (push)
#  pragma warning (disable : 4189)
#  include <string>
#  pragma warning (pop)
# else
#  include <string>
# endif
#endif // QT_NO_STL

#include <stdarg.h>

// POSIX defines truncate to truncate64
#ifdef truncate
#error qstring.h must be included before any header file that defines truncate
#endif

class QCharRef;
class QRegExp;
class QStringList;
class QTextCodec;
class QLatin1String;

class Q_CORE_EXPORT QString
{
public:
    inline QString();
    QString(const QChar *unicode, int size);
    explicit QString(QChar c);
    QString(int size, QChar c);
    inline QString(const QLatin1String &latin1);
    inline QString(const QString &);
    inline ~QString();
    QString &operator=(QChar c);
    QString &operator=(const QString &);
    inline QString &operator=(const QLatin1String &);

    inline int size() const { return d->size; }
    inline int count() const { return d->size; }
    inline int length() const;
    inline bool isEmpty() const;
    void resize(int size);

    QString &fill(QChar c, int size = -1);
    void truncate(int pos);
    void chop(int n);

    int capacity() const;
    inline void reserve(int size) { if (d->ref != 1 || size > d->alloc) realloc(size); }
    inline void squeeze() { if (d->size < d->alloc) realloc(); }

    inline const QChar *unicode() const;
    inline QChar *data();
    inline const QChar *data() const;
    inline const QChar *constData() const;

    inline void detach();
    inline bool isDetached() const;
    void clear();

    inline const QChar at(int i) const;
    const QChar operator[](int i) const;
    QCharRef operator[](int i);
    const QChar operator[](uint i) const;
    QCharRef operator[](uint i);

    QString arg(Q_LONGLONG a, int fieldwidth=0, int base=10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(Q_ULONGLONG a, int fieldwidth=0, int base=10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(long a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(ulong a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(int a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(uint a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(short a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(ushort a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(double a, int fieldWidth = 0, char fmt = 'g', int prec = -1, const QChar &fillChar = QLatin1Char(' ')) const;
#ifdef QT_USE_FIXED_POINT
    QString arg(QFixedPoint a, int fieldWidth = 0, char fmt = 'g', int prec = -1, const QChar &fillChar = QLatin1Char(' ')) const
        { return arg(a.toDouble(), fieldWidth, fmt, prec, fillChar); }
#endif
    QString arg(char a, int fieldWidth = 0, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(QChar a, int fieldWidth = 0, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(const QString &a, int fieldWidth = 0, const QChar &fillChar = QLatin1Char(' ')) const;
    QString arg(const QString &a1, const QString &a2) const;
    QString arg(const QString &a1, const QString &a2, const QString &a3) const;
    QString arg(const QString &a1, const QString &a2, const QString &a3, const QString &a4) const;

#ifndef QT_NO_SPRINTF
    QString    &vsprintf(const char *format, va_list ap);
    QString    &sprintf(const char *format, ...)
#if defined(Q_CC_GNU) && !defined(__INSURE__)
        __attribute__ ((format (printf, 2, 3)))
#endif
        ;
#endif

    int indexOf(QChar c, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int indexOf(const QString &s, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(QChar c, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(const QString &s, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    inline QBool contains(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    inline QBool contains(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

#ifndef QT_NO_REGEXP
    int indexOf(const QRegExp &, int from = 0) const;
    int lastIndexOf(const QRegExp &, int from = -1) const;
    inline QBool contains(const QRegExp &rx) const { return QBool(indexOf(rx) != -1); }
    int count(const QRegExp &) const;
#endif

    enum SectionFlags {
        SectionDefault             = 0x00,
        SectionSkipEmpty           = 0x01,
        SectionIncludeLeadingSep   = 0x02,
        SectionIncludeTrailingSep  = 0x04,
        SectionCaseInsensitiveSeps = 0x08
    };
    QString section(QChar sep, int start, int end = -1, int flags = SectionDefault) const;
    QString section(const QString &in_sep, int start, int end = -1, int flags = SectionDefault) const;
#ifndef QT_NO_REGEXP
    QString section(const QRegExp &reg, int start, int end = -1, int flags = SectionDefault) const;
#endif

    QString left(int len) const;
    QString right(int len) const;
    QString mid(int i, int len = -1) const;

    bool startsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool startsWith(const QLatin1String &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(const QLatin1String &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    QString leftJustified(int width, QChar fill = QLatin1Char(' '), bool trunc = false) const;
    QString rightJustified(int width, QChar fill = QLatin1Char(' '), bool trunc = false) const;

    QString toLower() const;
    QString toUpper() const;

    QString trimmed() const;
    QString simplified() const;

    QString &insert(int i, QChar c);
    QString &insert(int i, const QChar *uc, int len);
    inline QString &insert(int i, const QString &s) { return insert(i, s.constData(), s.length()); }
    QString &insert(int i, const QLatin1String &s);
    QString &append(QChar c);
    QString &append(const QString &s);
    QString &append(const QLatin1String &s);
    inline QString &prepend(QChar c) { return insert(0, c); }
    inline QString &prepend(const QString &s) { return insert(0, s); }
    inline QString &prepend(const QLatin1String &s) { return insert(0, s); }
    inline QString &operator+=(QChar c) { return append(c); }
    inline QString &operator+=(QChar::SpecialCharacter c) { return append(QChar(c)); }
    inline QString &operator+=(const QString &s) { return append(s); }
    inline QString &operator+=(const QLatin1String &s) { return append(s); }

    QString &remove(int i, int len);
    QString &remove(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString &remove(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString &replace(int i, int len, QChar after);
    QString &replace(int i, int len, const QChar *s, int slen);
    QString &replace(int i, int len, const QString &after);
    QString &replace(QChar before, QChar after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString &replace(QChar c, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString &replace(const QString &before, const QString &after,
                     Qt::CaseSensitivity cs = Qt::CaseSensitive);
#ifndef QT_NO_REGEXP_CAPTURE
    QString &replace(const QRegExp &rx, const QString &after);
    inline QString &remove(const QRegExp &rx)
    { return replace(rx, QString()); }
#endif

    enum SplitBehavior { KeepEmptyParts, SkipEmptyParts };

    QStringList split(const QString &sep, SplitBehavior behavior = KeepEmptyParts,
                      Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    QStringList split(const QChar &sep, SplitBehavior behavior = KeepEmptyParts,
                      Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    QStringList split(const QRegExp &sep, SplitBehavior behavior = KeepEmptyParts) const;

    enum NormalizationForm {
        NormalizationForm_D,
        NormalizationForm_C,
        NormalizationForm_KD,
        NormalizationForm_KC
    };
    QString normalize(NormalizationForm mode);
    QString normalize(NormalizationForm mode, QChar::UnicodeVersion version);

    const ushort *utf16() const;

    QByteArray toAscii() const;
    QByteArray toLatin1() const;
    QByteArray toUtf8() const;
    QByteArray toLocal8Bit() const;

    static QString fromAscii(const char *, int size = -1);
    static QString fromLatin1(const char *, int size = -1);
    static QString fromUtf8(const char *, int size = -1);
    static QString fromLocal8Bit(const char *, int size = -1);
    static QString fromUtf16(const ushort *, int size = -1);
    static QString fromRawData(const QChar *, int size);

    QString &setUnicode(const QChar *unicode, int size);
    inline QString &setUtf16(const ushort *utf16, int size)
    { return setUnicode(reinterpret_cast<const QChar *>(utf16), size); }

    int compare(const QString &s) const;
    static inline int compare(const QString &s1, const QString &s2)
    { return s1.compare(s2); }
    int localeAwareCompare(const QString& s) const;
    static int localeAwareCompare(const QString& s1, const QString& s2)
    { return s1.localeAwareCompare(s2); }

    short  toShort(bool *ok=0, int base=10) const;
    ushort toUShort(bool *ok=0, int base=10) const;
    int toInt(bool *ok=0, int base=10) const;
    uint toUInt(bool *ok=0, int base=10) const;
    long toLong(bool *ok=0, int base=10) const;
    ulong toULong(bool *ok=0, int base=10) const;
    Q_LONGLONG toLongLong(bool *ok=0, int base=10) const;
    Q_ULONGLONG toULongLong(bool *ok=0, int base=10) const;
    float toFloat(bool *ok=0) const;
    double toDouble(bool *ok=0) const;

    QString &setNum(short, int base=10);
    QString &setNum(ushort, int base=10);
    QString &setNum(int, int base=10);
    QString &setNum(uint, int base=10);
    QString &setNum(long, int base=10);
    QString &setNum(ulong, int base=10);
    QString &setNum(Q_LONGLONG, int base=10);
    QString &setNum(Q_ULONGLONG, int base=10);
    QString &setNum(float, char f='g', int prec=6);
    QString &setNum(double, char f='g', int prec=6);
#ifdef QT_USE_FIXED_POINT
    inline QString &setNum(QFixedPoint d, char f = 'g', int prec = 6) { return setNum(d.toDouble(), f, prec); }
#endif

    static QString number(long, int base=10);
    static QString number(ulong, int base=10);
    static QString number(int, int base=10);
    static QString number(uint, int base=10);
    static QString number(Q_LONGLONG, int base=10);
    static QString number(Q_ULONGLONG, int base=10);
    static QString number(double, char f='g', int prec=6);
#ifdef QT_USE_FIXED_POINT
    inline static QString number(QFixedPoint d, char f = 'g', int prec = 6)
        { return number(d.toDouble(), f, prec); }
#endif

    bool operator==(const QString &s) const;
    bool operator<(const QString &s) const;
    inline bool operator>(const QString &s) const { return s < *this; }
    inline bool operator!=(const QString &s) const { return !operator==(s); }
    inline bool operator<=(const QString &s) const { return !operator>(s); }
    inline bool operator>=(const QString &s) const { return !operator<(s); }

    bool operator==(const QLatin1String &s) const;
    bool operator<(const QLatin1String &s) const;
    bool operator>(const QLatin1String &s) const;
    inline bool operator!=(const QLatin1String &s) const { return !operator==(s); }
    inline bool operator<=(const QLatin1String &s) const { return !operator>(s); }
    inline bool operator>=(const QLatin1String &s) const { return !operator<(s); }

    // ASCII compatibility
#ifndef QT_NO_CAST_FROM_ASCII
    inline QString(const char *ch) : d(&shared_null)
    { ++d->ref; *this = fromAscii(ch); }
    inline QString(const QByteArray &a) : d(&shared_null)
    { ++d->ref; *this = fromAscii(a); }
    inline QString &operator=(const char *ch)
    { return (*this = fromAscii(ch)); }
    inline QString &operator=(const QByteArray &a)
    { return (*this = fromAscii(a)); }

    // these are needed, so it compiles with STL support enabled
    inline QString &prepend(const char *s)
    { return prepend(QString::fromAscii(s)); }
    inline QString &prepend(const QByteArray &s)
    { return prepend(QString(s)); }
    inline QString &append(const char *s)
    { return append(QString::fromAscii(s)); }
    inline QString &append(const QByteArray &s)
    { return append(QString(s)); }
    inline QString &operator+=(const char *s)
    { return append(QString::fromAscii(s)); }
    inline QString &operator+=(const QByteArray &s)
    { return append(QString(s)); }

    inline bool operator==(const char *s) const;
    inline bool operator!=(const char *s) const;
    inline bool operator<(const char *s) const;
    inline bool operator<=(const char *s2) const;
    inline bool operator>(const char *s2) const;
    inline bool operator>=(const char *s2) const;

    inline bool operator==(const QByteArray &s) const { return (*this == s.constData()); }
    inline bool operator!=(const QByteArray &s) const { return !(*this == s.constData()); }
    inline bool operator<(const QByteArray &s) const { return *this < s.constData(); }
    inline bool operator>(const QByteArray &s) const { return *this > s.constData(); }
    inline bool operator<=(const QByteArray &s) const { return *this <= s.constData(); }
    inline bool operator>=(const QByteArray &s) const { return *this >= s.constData(); }
#endif

    typedef QChar *iterator;
    typedef const QChar *const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    iterator begin();
    const_iterator begin() const;
    const_iterator constBegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator constEnd() const;

    // STL compatibility
    inline void push_back(QChar c) { append(c); }
    inline void push_back(const QString &s) { append(s); }
    inline void push_front(QChar c) { prepend(c); }
    inline void push_front(const QString &s) { prepend(s); }

#ifndef QT_NO_STL
    inline explicit QString(const std::string &s);
    inline std::string toStdString() const;
#endif

    // compatibility
    struct Null { };
    static const Null null;
    inline QString(const Null &): d(&shared_null) { ++d->ref; }
    inline QString &operator=(const Null &) { *this = QString(); return *this; }
    inline bool isNull() const { return d == &shared_null; }

#ifdef QT_COMPAT
    inline QT_COMPAT const char *ascii() const { return ascii_helper(); }
    inline QT_COMPAT const char *latin1() const { return latin1_helper(); }
    inline QT_COMPAT QByteArray utf8() const { return toUtf8(); }
    inline QT_COMPAT QByteArray local8Bit() const{ return toLocal8Bit(); }
    inline QT_COMPAT void setLength(int nl) { resize(nl); }
    inline QT_COMPAT QString copy() const { return *this; }
    inline QT_COMPAT QString &remove(QChar c, bool cs)
    { return remove(c, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT QString &remove(const QString  &s, bool cs)
    { return remove(s, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT QString &replace(QChar c, const QString  &after, bool cs)
    { return replace(c, after, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT QString &replace(const QString &before, const QString &after, bool cs)
    { return replace(before, after, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
#ifndef QT_NO_CAST_FROM_ASCII
    inline QT_COMPAT QString &replace(char c, const QString &after, bool cs)
    { return replace(QChar(c), after, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    // strange overload, required to avoid GCC 3.3 error
    inline QT_COMPAT QString &replace(char c, const QString &after, Qt::CaseSensitivity cs)
    { return replace(QChar(c), after, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
#endif
    inline QT_COMPAT int find(QChar c, int i = 0, bool cs = true) const
    { return indexOf(c, i, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT int find(const QString &s, int i = 0, bool cs = true) const
    { return indexOf(s, i, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT int findRev(QChar c, int i = -1, bool cs = true) const
    { return lastIndexOf(c, i, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT int findRev(const QString &s, int i = -1, bool cs = true) const
    { return lastIndexOf(s, i, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
#ifndef QT_NO_REGEXP
    inline QT_COMPAT int find(const QRegExp &rx, int i=0) const
    { return indexOf(rx, i); }
    inline QT_COMPAT int findRev(const QRegExp &rx, int i=-1) const
    { return lastIndexOf(rx, i); }
#endif
    inline QT_COMPAT QBool contains(QChar c, bool cs) const
    { return contains(c, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT QBool contains(const QString &s, bool cs) const
    { return contains(s, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT bool startsWith(const QString &s, bool cs) const
    { return startsWith(s, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT bool endsWith(const QString &s, bool cs) const
    { return endsWith(s, cs?Qt::CaseSensitive:Qt::CaseInsensitive); }
    inline QT_COMPAT QChar constref(uint i) const
    { return at(i); }
    QT_COMPAT QChar &ref(uint i);
    inline QT_COMPAT QString leftJustify(int width, QChar fill = QLatin1Char(' '), bool trunc=false) const
    { return leftJustified(width, fill, trunc); }
    inline QT_COMPAT QString rightJustify(int width, QChar fill = QLatin1Char(' '), bool trunc=false) const
    { return rightJustified(width, fill, trunc); }
    inline QT_COMPAT QString lower() const { return toLower(); }
    inline QT_COMPAT QString upper() const { return toUpper(); }
    inline QT_COMPAT QString stripWhiteSpace() const { return trimmed(); }
    inline QT_COMPAT QString simplifyWhiteSpace() const { return simplified(); }
    inline QT_COMPAT QString &setUnicodeCodes(const ushort *unicode_as_ushorts, int size)
    { return setUtf16(unicode_as_ushorts, size); }
    inline QT_COMPAT const ushort *ucs2() const { return utf16(); }
    inline static QT_COMPAT QString fromUcs2(const ushort *unicode, int size = -1)
    { return fromUtf16(unicode, size); }
    inline QT_COMPAT QString &setAscii(const char *str, int len = -1)
    { *this = fromAscii(str, len); return *this; }
    inline QT_COMPAT QString &setLatin1(const char *str, int len = -1)
    { *this = fromLatin1(str, len); return *this; }
protected:
    friend class QObject;
    const char *ascii_helper() const;
    const char *latin1_helper() const;
public:
#ifndef QT_NO_CAST_TO_ASCII
    inline QT_COMPAT operator const char *() const { return ascii_helper(); }
private:
    QT_COMPAT operator QNoImplicitBoolCast() const;
public:
#endif
#endif

    bool isSimpleText() const { if (!d->clean) updateProperties(); return d->simpletext; }
    bool isRightToLeft() const { if (!d->clean) updateProperties(); return d->righttoleft; }

private:
#if defined(QT_NO_CAST_FROM_ASCII) && !defined(Q_NO_DECLARED_NOT_DEFINED)
    QString &operator+=(const char *s);
    QString &operator+=(const QByteArray &s);
    QString(const char *ch);
    QString(const QByteArray &a);
    QString &operator=(const char  *ch);
    QString &operator=(const QByteArray &a);
#endif

    struct Data {
        QBasicAtomic ref;
        int alloc, size;
        ushort *data;
        ushort clean : 1;
        ushort simpletext : 1;
        ushort righttoleft : 1;
        ushort asciiCache : 1;
        ushort reserved : 12;
        ushort array[1];
    };
    static Data shared_null;
    static Data shared_empty;
    Data *d;
    QString(Data *dd, int /*dummy*/) : d(dd) {}
#ifndef QT_NO_TEXTCODEC
    static QTextCodec *codecForCStrings;
#endif
    static int grow(int);
    static void free(Data *);
    void realloc();
    void realloc(int alloc);
    void expand(int i);
    void updateProperties() const;
    QString multiArg(int numArgs, const QString &a1, const QString &a2,
                     const QString &a3 = QString(), const QString &a4 = QString()) const;
    friend class QCharRef;
    friend class QTextCodec;
};


class Q_CORE_EXPORT QLatin1String
{
public:
    inline explicit QLatin1String(const char *s) : chars(s) {}
    inline const char *latin1() const { return chars; }

    inline bool operator==(const QString &s) const
    { return s == *this; }
    inline bool operator!=(const QString &s) const
    { return s != *this; }
    inline bool operator>(const QString &s) const
    { return s < *this; }
    inline bool operator<(const QString &s) const
    { return s > *this; }
    inline bool operator>=(const QString &s) const
    { return s <= *this; }
    inline bool operator<=(const QString &s) const
    { return s >= *this; }

private:
    const char *chars;
};


inline QString::QString(const QLatin1String &latin1) : d(&shared_null)
{ ++d->ref; *this = fromLatin1(latin1.latin1()); }
inline int QString::length() const
{ return d->size; }
inline const QChar QString::at(int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i]; }
inline const QChar QString::operator[](int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i]; }
inline const QChar QString::operator[](uint i) const
{ Q_ASSERT(i < uint(size())); return d->data[i]; }
inline bool QString::isEmpty() const
{ return d->size == 0; }
inline const QChar *QString::unicode() const
{ return reinterpret_cast<const QChar*>(d->data); }
inline const QChar *QString::data() const
{ return reinterpret_cast<const QChar*>(d->data); }
inline QChar *QString::data()
{ detach(); return reinterpret_cast<QChar*>(d->data); }
inline const QChar *QString::constData() const
{ return reinterpret_cast<const QChar*>(d->data); }
inline void QString::detach()
{ if (d->ref != 1 || d->data != d->array) realloc(); }
inline bool QString::isDetached() const
{ return d->ref == 1; }
inline QString &QString::operator=(const QLatin1String &s)
{
    *this = fromLatin1(s.latin1());
    return *this;
}
inline void QString::clear()
{ if (!isNull()) *this = QString(); }
inline QString::QString(const QString &s) : d(s.d)
{ Q_ASSERT(&s != this); ++d->ref; }
inline int QString::capacity() const
{ return d->alloc; }
inline QString &QString::setNum(short n, int base)
{ return setNum(Q_LONGLONG(n), base); }
inline QString &QString::setNum(ushort n, int base)
{ return setNum(Q_ULONGLONG(n), base); }
inline QString &QString::setNum(int n, int base)
{ return setNum(Q_LONGLONG(n), base); }
inline QString &QString::setNum(uint n, int base)
{ return setNum(Q_ULONGLONG(n), base); }
inline QString &QString::setNum(long n, int base)
{ return setNum(Q_LONGLONG(n), base); }
inline QString &QString::setNum(ulong n, int base)
{ return setNum(Q_ULONGLONG(n), base); }
inline QString &QString::setNum(float n, char f, int prec)
{ return setNum(double(n),f,prec); }
inline QString QString::arg(long a, int fieldWidth, int base, const QChar &fillChar) const
{ return arg(Q_LONGLONG(a), fieldWidth, base, fillChar); }
inline QString QString::arg(ulong a, int fieldWidth, int base, const QChar &fillChar) const
{ return arg(Q_ULONGLONG(a), fieldWidth, base, fillChar); }
inline QString QString::arg(int a, int fieldWidth, int base, const QChar &fillChar) const
{ return arg(Q_LONGLONG(a), fieldWidth, base, fillChar); }
inline QString QString::arg(uint a, int fieldWidth, int base, const QChar &fillChar) const
{ return arg(Q_ULONGLONG(a), fieldWidth, base, fillChar); }
inline QString QString::arg(short a, int fieldWidth, int base, const QChar &fillChar) const
{ return arg(Q_LONGLONG(a), fieldWidth, base, fillChar); }
inline QString QString::arg(ushort a, int fieldWidth, int base, const QChar &fillChar) const
{ return arg(Q_ULONGLONG(a), fieldWidth, base, fillChar); }
inline QString QString::arg(const QString &a1, const QString &a2) const
{ return multiArg(2, a1, a2); }
inline QString QString::arg(const QString &a1, const QString &a2, const QString &a3) const
{ return multiArg(3, a1, a2, a3); }
inline QString QString::arg(const QString &a1, const QString &a2,  const QString &a3, const QString &a4) const
{ return multiArg(4, a1, a2, a3, a4); }
inline QString QString::section(QChar sep, int start, int end, int flags) const
{ return section(QString(sep), start, end, flags); }


class Q_CORE_EXPORT QCharRef {
    QString &s;
    int i;
    inline QCharRef(QString &str, int idx)
        : s(str),i(idx) {}
    friend class QString;
public:

    // most QChar operations repeated here

    // all this is not documented: We just say "like QChar" and let it be.
    inline operator QChar() const
        { return i < s.d->size ? s.d->data[i] : 0; }
    inline QCharRef &operator=(const QChar &c)
        { if (s.d->ref != 1 || i >= s.d->size) s.expand(i);
          s.d->data[i] = c.unicode();  return *this; }

    // An operator= for each QChar cast constructors
#ifndef QT_NO_CAST_FROM_ASCII
    inline QCharRef operator=(char c) { return operator=(QChar(c)); }
    inline QCharRef operator=(uchar c) { return operator=(QChar(c)); }
#endif
    inline QCharRef operator=(const QCharRef &c) { return operator=(QChar(c)); }
    inline QCharRef operator=(ushort rc) { return operator=(QChar(rc)); }
    inline QCharRef operator=(short rc) { return operator=(QChar(rc)); }
    inline QCharRef operator=(uint rc) { return operator=(QChar(rc)); }
    inline QCharRef operator=(int rc) { return operator=(QChar(rc)); }

    // each function...
    inline bool isNull() const { return QChar(*this).isNull(); }
    inline bool isPrint() const { return QChar(*this).isPrint(); }
    inline bool isPunct() const { return QChar(*this).isPunct(); }
    inline bool isSpace() const { return QChar(*this).isSpace(); }
    inline bool isMark() const { return QChar(*this).isMark(); }
    inline bool isLetter() const { return QChar(*this).isLetter(); }
    inline bool isNumber() const { return QChar(*this).isNumber(); }
    inline bool isLetterOrNumber() { return QChar(*this).isLetterOrNumber(); }
    inline bool isDigit() const { return QChar(*this).isDigit(); }

    inline int digitValue() const { return QChar(*this).digitValue(); }
    QChar toLower() const { return QChar(*this).toLower(); }
    QChar toUpper() const { return QChar(*this).toUpper(); }
#ifdef QT_COMPAT
    inline QT_COMPAT QChar lower() const { return QChar(*this).toLower(); }
    inline QT_COMPAT QChar upper() const { return QChar(*this).toUpper(); }
#endif

    QChar::Category category() const { return QChar(*this).category(); }
    QChar::Direction direction() const { return QChar(*this).direction(); }
    QChar::Joining joining() const { return QChar(*this).joining(); }
    bool hasMirrored() const { return QChar(*this).hasMirrored(); }
#ifdef QT_COMPAT
    inline bool QT_COMPAT mirrored() const { return hasMirrored(); }
#endif
    QChar mirroredChar() const { return QChar(*this).mirroredChar(); }
    QString decomposition() const { return QChar(*this).decomposition(); }
    QChar::Decomposition decompositionTag() const { return QChar(*this).decompositionTag(); }
    uchar combiningClass() const { return QChar(*this).combiningClass(); }

    QChar::UnicodeVersion unicodeVersion() const { return QChar(*this).unicodeVersion(); }

    inline uchar cell() const { return QChar(*this).cell(); }
    inline uchar row() const { return QChar(*this).row(); }
    inline void setCell(uchar cell) { QChar(*this).setCell(cell); }
    inline void setRow(uchar row) { QChar(*this).setRow(row); }

    const char toAscii() const { return QChar(*this).toAscii(); }
    const char toLatin1() const { return QChar(*this).toLatin1(); }
#ifdef QT_COMPAT
    const char latin1() const { return QChar(*this).toLatin1(); }
    const char ascii() const { return QChar(*this).toAscii(); }
#endif
    const ushort unicode() const { return QChar(*this).unicode(); }
};
inline QString::QString() : d(&shared_null) { ++d->ref; }
inline QString::~QString() { if (!--d->ref) free(d); }
inline QCharRef QString::operator[](int i)
{ Q_ASSERT(i >= 0); return QCharRef(*this, i); }
inline QCharRef QString::operator[](uint i)
{ return QCharRef(*this, i); }
inline QString::iterator QString::begin()
{ detach(); return reinterpret_cast<QChar*>(d->data); }
inline QString::const_iterator QString::begin() const
{ return reinterpret_cast<const QChar*>(d->data); }
inline QString::const_iterator QString::constBegin() const
{ return reinterpret_cast<const QChar*>(d->data); }
inline QString::iterator QString::end()
{ detach(); return reinterpret_cast<QChar*>(d->data + d->size); }
inline QString::const_iterator QString::end() const
{ return reinterpret_cast<const QChar*>(d->data + d->size); }
inline QString::const_iterator QString::constEnd() const
{ return reinterpret_cast<const QChar*>(d->data + d->size); }
inline QBool QString::contains(const QString &s, Qt::CaseSensitivity cs) const
{ return QBool(indexOf(s, 0, cs) != -1); }
inline QBool QString::contains(QChar c, Qt::CaseSensitivity cs) const
{ return QBool(indexOf(c, 0, cs) != -1); }


inline bool operator==(QString::Null, QString::Null) { return true; }
inline bool operator==(QString::Null, const QString &s) { return s.isNull(); }
inline bool operator==(const QString &s, QString::Null) { return s.isNull(); }
inline bool operator!=(QString::Null, QString::Null) { return false; }
inline bool operator!=(QString::Null, const QString &s) { return !s.isNull(); }
inline bool operator!=(const QString &s, QString::Null) { return !s.isNull(); }

#ifndef QT_NO_CAST_FROM_ASCII
inline bool QString::operator==(const char *s) const {
#  ifndef QT_NO_TEXTCODEC
    if (codecForCStrings) return (*this == QString::fromAscii(s));
#  endif
    return (*this == QLatin1String(s));
}
inline bool QString::operator!=(const char *s) const{ return !(*this == s); }
inline bool QString::operator<(const char *s) const { return *this < QString::fromAscii(s); }
inline bool QString::operator>(const char *s) const { return *this > QString::fromAscii(s); }
inline bool QString::operator<=(const char *s) const { return *this <= QString::fromAscii(s); }
inline bool QString::operator>=(const char *s) const { return *this >= QString::fromAscii(s); }

inline bool operator==(const char *s1, const QString &s2) { return (s2 == s1); }
inline bool operator!=(const char *s1, const QString &s2) { return !(s2 == s1); }
inline bool operator<(const char *s1, const QString &s2) { return (QString::fromAscii(s1) < s2); }
inline bool operator>(const char *s1, const QString &s2) { return (QString::fromAscii(s1) > s2); }
inline bool operator<=(const char *s1, const QString &s2) { return (QString::fromAscii(s1) <= s2); }
inline bool operator>=(const char *s1, const QString &s2) { return (QString::fromAscii(s1) >= s2); }

inline bool QByteArray::operator==(const QString &s) const { return constData() == s; }
inline bool QByteArray::operator!=(const QString &s) const { return !(constData() == s); }
inline bool QByteArray::operator<(const QString &s) const { return constData() < s; }
inline bool QByteArray::operator>(const QString &s) const { return constData() > s; }
inline bool QByteArray::operator<=(const QString &s) const { return constData() <= s; }
inline bool QByteArray::operator>=(const QString &s) const { return constData() >= s; }
#endif   // QT_NO_CAST_FROM_ASCII

#ifndef QT_NO_CAST_TO_ASCII
inline QByteArray &QByteArray::append(const QString &s)
{ return append(s.toAscii()); }
inline QByteArray &QByteArray::insert(int i, const QString &s)
{ return insert(i, s.toAscii()); }
inline QByteArray &QByteArray::replace(char c, const QString &after)
{ return replace(c, after.toAscii()); }
inline QByteArray &QByteArray::replace(const QString &before, const char *after)
{ return replace(before.toAscii(), after); }
inline QByteArray &QByteArray::replace(const QString &before, const QByteArray &after)
{ return replace(before, after.constData()); }
inline QByteArray &QByteArray::operator+=(const QString &s)
{ return operator+=(s.toAscii()); }
inline int QByteArray::indexOf(const QString &s, int from) const
{ return indexOf(s.toAscii(), from); }
inline int QByteArray::lastIndexOf(const QString &s, int from) const
{ return lastIndexOf(s.toAscii(), from); }
#ifdef QT_COMPAT
inline int QByteArray::find(const QString &s, int from) const
{ return indexOf(s, from); }
inline int QByteArray::findRev(const QString &s, int from) const
{ return lastIndexOf(s, from); }
#endif // QT_COMPAT
#endif // QT_NO_CAST_TO_ASCII

inline const QString operator+(const QString &s1, const QString &s2)
{ QString t(s1); t += s2; return t; }
inline const QString operator+(const QString &s1, QChar s2)
{ QString t(s1); t += s2; return t; }
inline const QString operator+(QChar s1, const QString &s2)
{ QString t(s1); t += s2; return t; }
#ifndef QT_NO_CAST_FROM_ASCII
inline const QString operator+(const QString &s1, const char *s2)
{ QString t(s1); t += QString::fromAscii(s2); return t; }
inline const QString operator+(const char *s1, const QString &s2)
{ QString t(s1); t += s2; return t; }
inline const QString operator+(char c, const QString &s)
{ QString t = s; t.prepend(QChar(c)); return t; }
inline const QString operator+(const QString &s, char c)
{ QString t(s); t += c; return t; }
inline const QString operator+(const QByteArray &ba, const QString &s)
{ QString t(ba); t += s; return t; }
inline const QString operator+(const QString &s, const QByteArray &ba)
{ QString t(s); t += ba; return t; }
#endif

#ifndef QT_NO_STL
inline std::string QString::toStdString() const
{
    return toAscii().data();
}

inline QString::QString(const std::string &s)
    : d(&shared_null)
{
    ++d->ref;
    *this = fromAscii(s.c_str());
}
#endif

#ifdef QT_COMPAT
inline QChar &QString::ref(uint i)
{
    if (int(i) > d->size || d->ref != 1)
        resize(qMax(int(i), d->size));
    return reinterpret_cast<QChar&>(d->data[i]);
}
#endif

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QString &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QString &);
#endif

#ifdef QT_COMPAT
class QConstString : public QString
{
public:
    inline QT_COMPAT_CONSTRUCTOR QConstString(const QChar *unicode, int size)
        :QString(unicode, size){} // cannot use fromRawData() due to changed semantics
    inline QT_COMPAT const QString &string() const { return *this; }
};
#endif

Q_DECLARE_TYPEINFO(QString, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QString);

#if defined(Q_OS_WIN32)
extern Q_CORE_EXPORT QByteArray qt_winQString2MB(const QString& s, int len=-1);
extern Q_CORE_EXPORT QByteArray qt_winQString2MB(const QChar *ch, int len);
extern Q_CORE_EXPORT QString qt_winMB2QString(const char* mb, int len=-1);
#endif

#endif // QSTRING_H

/****************************************************************************
**
** Definition of QString class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTRING_H
#define QSTRING_H

#ifndef QT_H
#include "qchar.h"
#include "qbytearray.h"
#include "qatomic.h"
#endif // QT_H

#ifndef QT_NO_STL
#if defined ( Q_CC_MSVC_NET ) && _MSV_VER < 1310 // Avoids nasty warning for xlocale, line 450
#  pragma warning ( push )
#  pragma warning ( disable : 4189 )
#  include <string>
#  pragma warning ( pop )
#else
#  include <string>
#endif // avoid warning in xlocale on windows .net 1310
#endif // QT_NO_STL

#ifdef QT_INCLUDE_COMPAT
#include <qcstring.h>
#endif

class QCharRef;
class QRegExp;

class Q_EXPORT QString
{
public:
    QString();
    QString(const QChar *unicode, int size);
    QString(int size, QChar c);
    explicit QString(QChar c);
    QString(const QString &);
    ~QString();
    QString &operator=(QChar c);
    QString &operator=(const QString  &);

    int size() const;
    int length() const;
    bool isEmpty() const;
    void resize(int size);

    bool operator!() const;

    QString &fill(QChar c, int size = -1);
    void truncate(int maxSize);

    int capacity() const;
    void reserve(int size);
    void squeeze();

    const QChar *unicode() const;
    operator const QChar*() const;
    QChar *data();
    const QChar *data() const;
    const QChar *constData() const;

    void detach();
    bool isDetached() const;
    void clear();

    const QChar at(int i) const;
    const QChar operator[](int i) const;
    QCharRef operator[](int i);
    const QChar operator[](uint i) const;
    QCharRef operator[](uint i);

    QString arg( Q_LLONG a, int fieldwidth=0, int base=10 ) const;
    QString arg( Q_ULLONG a, int fieldwidth=0, int base=10 ) const;
    QString arg(long a, int fieldWidth = 0, int base = 10) const;
    QString arg(ulong a, int fieldWidth = 0, int base = 10) const;
    QString arg(int a, int fieldWidth = 0, int base = 10) const;
    QString arg(uint a, int fieldWidth = 0, int base = 10) const;
    QString arg(short a, int fieldWidth = 0, int base = 10) const;
    QString arg(ushort a, int fieldWidth = 0, int base = 10) const;
    QString arg(double a, int fieldWidth = 0, char fmt = 'g', int prec = -1) const;
    QString arg(char a, int fieldWidth = 0) const;
    QString arg(QChar a, int fieldWidth = 0) const;
    QString arg(const QString &a, int fieldWidth = 0) const;
#ifndef QT_NO_CAST_FROM_ASCII
    inline QString arg(const QByteArray &a, int fieldWidth = 0) const
    { return arg( QString(a), fieldWidth); }
    inline QString arg(const char *s, int fieldWidth = 0) const
    { return arg( QString::fromAscii(s), fieldWidth); }
#endif
    QString arg(const QString &a1, const QString &a2) const;
    QString arg(const QString &a1, const QString &a2, const QString &a3) const;
    QString arg(const QString &a1, const QString &a2, const QString &a3,  const QString &a4) const;

#ifndef QT_NO_SPRINTF
    QString    &sprintf(const char *format, ...)
#if defined(Q_CC_GNU) && !defined(__INSURE__)
        __attribute__ ((format (printf, 2, 3)))
#endif
        ;
#endif
    // #### move to qnamespace
    enum CaseSensitivity { CaseInsensitive, CaseSensitive };

    int indexOf(QChar c, int from = 0, CaseSensitivity cs = CaseSensitive) const;
    int indexOf(const QString &s, int from = 0, CaseSensitivity cs = CaseSensitive) const;
    int lastIndexOf(QChar c, int from = -1, CaseSensitivity cs = CaseSensitive) const;
    int lastIndexOf(const QString &s, int from = -1, CaseSensitivity cs = CaseSensitive) const;

    QBool contains(QChar c, CaseSensitivity cs = CaseSensitive) const;
    QBool contains(const QString &s, CaseSensitivity cs = CaseSensitive) const;
    int count(QChar c, CaseSensitivity cs = CaseSensitive) const;
    int count(const QString &s, CaseSensitivity cs = CaseSensitive) const;

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
    QString section(QChar sep, int start, int end = 0xffffffff, int flags = SectionDefault) const;
    QString section(char sep, int start, int end = 0xffffffff, int flags = SectionDefault) const;
#ifndef QT_NO_CAST_FROM_ASCII
    QString section(const char *in_sep, int start, int end = 0xffffffff, int flags = SectionDefault) const;
#endif
    QString section(const QString &in_sep, int start, int end = 0xffffffff, int flags = SectionDefault) const;
#ifndef QT_NO_REGEXP
    QString section(const QRegExp &reg, int start, int end = 0xffffffff, int flags = SectionDefault) const;
#endif

    QString left(int len)  const;
    QString right(int len) const;
    QString mid(int i, int len=-1) const;

    bool startsWith(const QString &s, CaseSensitivity cs = CaseSensitive) const;
    bool endsWith(const QString &s, CaseSensitivity cs = CaseSensitive) const;

    QString leftJustified(int width, QChar fill=' ', bool trunc=false) const;
    QString rightJustified(int width, QChar fill=' ', bool trunc=false) const;

    QString toLower() const;
    QString toUpper() const;

    QString trimmed() const;
    QString simplified() const;

    QString &insert(int i, QChar c);
    QString &insert(int i, const QString &s);
    QString &append(QChar c);
    QString &append(const QString &s);
    QString &prepend(QChar c);
    QString &prepend(const QString &s);
    inline QString &operator+=(QChar c) { return append(c); }
    inline QString &operator+=(QChar::SpecialChars c ) { return append(QChar(c)); }
    inline QString &operator+=(const QString &s) { return append(s); }

    QString &remove(int i, int len);
    QString &remove(QChar c, CaseSensitivity cs = CaseSensitive);
    QString &remove(const QString &s, CaseSensitivity cs = CaseSensitive);
    QString &replace(int i, int len, QChar after);
    QString &replace(int i, int len, const QString &after);
    QString &replace(QChar before, QChar after, CaseSensitivity cs = CaseSensitive);
    QString &replace(QChar c, const QString &after, CaseSensitivity cs = CaseSensitive);
    QString &replace(const QString &before, const QString &after, CaseSensitivity cs = CaseSensitive);
#ifndef QT_NO_REGEXP_CAPTURE
    QString &replace(const QRegExp &rx, const QString &after);
    inline QString &remove(const QRegExp &rx)
    { return replace(rx, QString()); }
#endif

    inline const char *ascii() const { return toAscii(); }
    inline const char *latin1() const{ return toLatin1(); }
    inline const char *utf8() const { return toUtf8(); }
    inline const char *local8Bit() const{ return toLocal8Bit(); }
    const ushort *ucs2() const;

    QByteArray toAscii() const;
    QByteArray toLatin1() const;
    QByteArray toUtf8() const;
    QByteArray toLocal8Bit() const;

    static QString fromAscii(const char*, int size=-1);
    static QString fromLatin1(const char*, int size=-1);
    static QString fromUtf8(const char*, int size=-1);
    static QString fromLocal8Bit(const char*, int size=-1);
    static QString fromUcs2( const unsigned short *ucs2 );

    QString &setUnicode(const QChar *unicode, int size);
    QString &setUnicodeCodes(const ushort *unicode_as_ushorts, int size);

    int compare(const QString &s) const;
    static inline int compare(const QString &s1, const QString &s2)
    { return s1.compare(s2); }
    int localeAwareCompare( const QString& s ) const;
    static int localeAwareCompare( const QString& s1, const QString& s2 )
    { return s1.localeAwareCompare( s2 ); }

    short  toShort(bool *ok=0, int base=10) const;
    ushort toUShort(bool *ok=0, int base=10) const;
    int toInt(bool *ok=0, int base=10)   const;
    uint toUInt(bool *ok=0, int base=10)  const;
    long toLong(bool *ok=0, int base=10)  const;
    ulong toULong(bool *ok=0, int base=10) const;
    Q_LLONG toLongLong(bool *ok=0, int base=10) const;
    Q_ULLONG toULongLong(bool *ok=0, int base=10) const;
    float toFloat(bool *ok=0) const;
    double toDouble(bool *ok=0) const;

    QString &setNum(short, int base=10);
    QString &setNum(ushort, int base=10);
    QString &setNum(int, int base=10);
    QString &setNum(uint, int base=10);
    QString &setNum(long, int base=10);
    QString &setNum(ulong, int base=10);
    QString &setNum(Q_LLONG, int base=10);
    QString &setNum(Q_ULLONG, int base=10);
    QString &setNum(float, char f='g', int prec=6);
    QString &setNum(double, char f='g', int prec=6);

    static QString number(long, int base=10);
    static QString number(ulong, int base=10);
    static QString number(int, int base=10);
    static QString number(uint, int base=10);
    static QString number(Q_LLONG, int base=10);
    static QString number(Q_ULLONG, int base=10);
    static QString number(double, char f='g', int prec=6);

    // ascii compatibility
#ifndef QT_NO_CAST_FROM_ASCII
    QString(const char *);
    inline QString(const QByteArray &a):d(&shared_null)
    { ++d->ref; *this = fromAscii(a, a.size()); }
    QString &operator=(const char  *);
    inline QString &operator=(char c) { return operator=(QChar(c)); }
    inline QString &operator=(const QByteArray &a)
    { return operator=(a.constData()); }
    QString &append(const char *s);
    inline QString &append(const QByteArray &a)
    { return append(a.constData()); }
    QString &prepend(const char *s);
    inline QString &prepend(char c)
    { return prepend(QChar(c)); }
    inline QString &prepend(const QByteArray &a)
    { return prepend(a.constData()); }
    inline QString &operator+=(const char *s)
    { return append(s); }
    inline QString &operator+=(const QByteArray &a)
    { return append(a); }
#endif
#ifndef QT_NO_CAST_TO_ASCII
    inline operator const char *() const { return ascii(); }
#endif

    typedef QChar *Iterator;
    typedef const QChar *ConstIterator;
    Iterator begin();
    ConstIterator begin() const;
    ConstIterator constBegin() const;
    Iterator end();
    ConstIterator end() const;
    ConstIterator constEnd() const;

    // stl compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    inline void push_back(QChar c) { append(c); }
    inline void push_back(const QString &s) { append(s); }
#ifndef QT_NO_CAST_FROM_ASCII
    inline void push_back(const char *s) { append(s); }
    inline void push_back(const QByteArray &a) { append(a); }
#ifndef QT_NO_STL
    inline QString(const std::string &s): d(&shared_null)
    { ++d->ref; *this = fromAscii(s.c_str()); }
    inline operator const std::string () const
    { return ascii(); }
    inline QString &operator=(const std::string &s)
    { return operator=(s.c_str()); }
    inline void push_back(const std::string &s)
    { append(s.c_str()); }
    inline QString &append(const std::string &s)
    { return append(s.c_str()); }
    inline QString &operator+=(const std::string &s)
    { return append(s.c_str()); }
#endif
#endif

    // compatibility
#ifndef QT_NO_COMPAT
    static struct Null {
	inline bool operator==(const Null&){return true; }
	inline bool operator!=(const Null&){return false; }
    } null;
    inline QString(const Null &): d(&shared_null) { ++d->ref; }
    inline QString &operator=(const Null &) { *this = QString(); return *this; }
    inline bool isNull() const { return d == &shared_null; }
    inline void setLength(int nl) { resize(nl); }
    inline QString copy() const { return *this; }
    inline QString &insert(int i, const QChar *uc, int len)
    { return insert(i, QString(uc, len)); }
    inline QString &replace(int i, int len, const QChar *s, int clen)
    { return replace(i, len, QString(s, clen)); }
    inline QString &remove(QChar c, bool cs)
    { return remove(c, cs?CaseSensitive:CaseInsensitive); }
    inline QString &remove(const QString  &s, bool cs)
    { return remove(s, cs?CaseSensitive:CaseInsensitive); }
    inline QString &replace(QChar c, const QString  &after, bool cs)
    { return replace(c, after, cs?CaseSensitive:CaseInsensitive); }
    inline QString &replace(const QString  &before, const QString  &after, bool cs)
    { return replace(before, after, cs?CaseSensitive:CaseInsensitive); }
    inline int find(QChar c, int i = 0, bool cs = true) const
    { return indexOf(c, i, cs?CaseSensitive:CaseInsensitive); }
    inline int find(const QString &s, int i = 0, bool cs = true) const
    { return indexOf(s, i, cs?CaseSensitive:CaseInsensitive); }
    inline int findRev(QChar c, int i = -1, bool cs = true) const
    { return lastIndexOf(c, i, cs?CaseSensitive:CaseInsensitive); }
    inline int findRev(const QString &s, int i = -1, bool cs = true) const
    { return lastIndexOf(s, i, cs?CaseSensitive:CaseInsensitive); }
#ifndef QT_NO_REGEXP
    int find(const QRegExp &rx, int i=0) const
    { return indexOf(rx, i); }
    int findRev(const QRegExp &rx, int i=-1) const
    { return lastIndexOf(rx, i); }
#endif
    inline QBool contains(QChar c, bool cs) const
    { return contains(c, cs?CaseSensitive:CaseInsensitive); }
    inline QBool contains(const QString &s, bool cs) const
    { return contains(s, cs?CaseSensitive:CaseInsensitive); }
    inline bool startsWith(const QString &s, bool cs) const
    { return startsWith(s, cs?CaseSensitive:CaseInsensitive); }
    inline bool endsWith(const QString &s, bool cs) const
    { return endsWith(s, cs?CaseSensitive:CaseInsensitive); }
    QCharRef at(int i);
    inline QString &setAscii( const char *str, int len=-1 )
    { *this = fromAscii(str, len); return *this; }
    inline QString &setLatin1( const char *str, int len=-1 )
    { *this = fromLatin1(str, len); return *this; }
    inline QChar constref(uint i) const
    { return at(i); }
    QChar &ref(uint i);
    inline QString leftJustify(int width, QChar fill=' ', bool trunc=false) const
    { return leftJustified(width, fill, trunc); }
    inline QString rightJustify(int width, QChar fill=' ', bool trunc=false) const
    { return rightJustified(width, fill, trunc); }
    inline QString lower() const { return toLower(); }
    inline QString upper() const { return toUpper(); }
    inline QString stripWhiteSpace() const { return trimmed(); }
    inline QString simplifyWhiteSpace() const { return simplified(); }
#endif

    inline bool ensure_constructed()
    { if (!d) { d = &shared_null; ++d->ref; return false; } return true; }

    bool isSimpleText() const { if (!d->clean) updateProperties(); return (bool)d->simpletext; }
    bool isRightToLeft() const { if (!d->clean) updateProperties(); return (bool)d->righttoleft; }

private:
#ifdef QT_NO_CAST_TO_ASCII
    operator const char *() const;
#endif
    struct Data {
	QAtomic ref;
	int alloc, size;
	void *c;
	ushort *data;
	ushort clean : 1;
 	ushort encoding : 2;
	ushort cache : 1;
 	ushort simpletext : 1;
 	ushort righttoleft : 1;
	ushort reserved : 10;
	ushort array[1];
	enum { Latin1, Ascii, Local8Bit, Utf8 };
    };
    QString(Data *dd) : d(dd) {}
    static Data shared_null;
    static Data shared_empty;
    Data *d;
    static int grow(int);
    static void free(Data *);
    void realloc();
    void realloc(int alloc);
    void expand(int i);
    void updateProperties() const;
    QString multiArg(int numArgs, const QString &a1, const QString &a2,
		      const QString &a3 = QString(), const QString &a4 = QString()) const;
    friend class QCharRef;
    friend class QConstString;
};


inline QString::QString() :d(&shared_null)
{ ++d->ref; }
inline int QString::size() const
{ return d->size; }
inline int QString::length() const
{ return d->size; }
inline void QString::truncate(int maxSize)
{ if (maxSize < d->size) resize(maxSize); }
inline const QChar QString::at(int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i]; }
inline const QChar QString::operator[](int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i]; }
inline const QChar QString::operator[](uint i) const
{ Q_ASSERT(i < (uint)size()); return d->data[i]; }
inline bool QString::isEmpty() const
{ return d->size == 0; }
inline bool QString::operator!() const
{ return d->size == 0; }
inline const QChar *QString::unicode() const
{ return (const QChar*) d->data; }
inline QString::operator const QChar*() const
{ return (const QChar*) d->data; }
inline const QChar *QString::data() const
{ return (const QChar*) d->data; }
inline QChar *QString::data()
{ detach(); return (QChar*) d->data; }
inline const QChar *QString::constData() const
{ return (const QChar*) d->data; }
inline const ushort *QString::ucs2() const
{ d->data[d->size] = 0; return d->data; } //###
inline void QString::detach()
{ if (d->ref != 1 || d->data != d->array) realloc(); }
inline bool QString::isDetached() const
{ return d->ref == 1; }
inline QString &QString::operator=(const QString & a)
{
    Data *x = a.d; ++x->ref;
    x = qAtomicSetPtr( &d, x );
    if (!--x->ref) free(x);
    return *this;
}
inline QString::~QString()
{ if (!--d->ref) free(d); }
inline void QString::clear()
{ *this = QString(); }
inline QString::QString(const QString &s) : d(s.d)
{ Q_ASSERT(&s != this); ++d->ref; }
inline int QString::capacity() const
{ return d->alloc; }
inline QString &QString::setNum(short n, int base)
{ return setNum((Q_LLONG)n, base); }
inline QString &QString::setNum(ushort n, int base)
{ return setNum((Q_ULLONG)n, base); }
inline QString &QString::setNum(int n, int base)
{ return setNum((Q_LLONG)n, base); }
inline QString &QString::setNum(uint n, int base)
{ return setNum((ulong)n, base); }
inline QString &QString::setNum(long n, int base)
{ return setNum((Q_LLONG)n, base); }
inline QString &QString::setNum(ulong n, int base)
{ return setNum((Q_ULLONG)n, base); }
inline QString &QString::setNum(float n, char f, int prec)
{ return setNum((double)n,f,prec); }
inline QString QString::arg(long a, int fieldWidth, int base) const
{ return arg((Q_LLONG)a, fieldWidth, base); }
inline QString QString::arg(ulong a, int fieldWidth, int base) const
{ return arg((Q_ULLONG)a, fieldWidth, base); }
inline QString QString::arg(int a, int fieldWidth, int base) const
{ return arg((Q_LLONG)a, fieldWidth, base); }
inline QString QString::arg(uint a, int fieldWidth, int base) const
{ return arg((Q_ULLONG)a, fieldWidth, base); }
inline QString QString::arg(short a, int fieldWidth, int base) const
{ return arg((Q_LLONG)a, fieldWidth, base); }
inline QString QString::arg(ushort a, int fieldWidth, int base) const
{ return arg((Q_ULLONG)a, fieldWidth, base); }
inline QString QString::arg(const QString &a1, const QString &a2) const
{ return multiArg(2, a1, a2); }
inline QString QString::arg(const QString &a1, const QString &a2, const QString &a3) const
{ return multiArg(3, a1, a2, a3); }
inline QString QString::arg(const QString &a1, const QString &a2,  const QString &a3, const QString &a4) const
{ return multiArg(4, a1, a2, a3, a4); }
inline QString QString::section(QChar sep, int start, int end, int flags) const
{ return section(QString(sep), start, end, flags); }
inline QString QString::section(char sep, int start, int end, int flags) const
{ return section(QChar(sep), start, end, flags); }
#ifndef QT_NO_CAST_FROM_ASCII
inline QString QString::section(const char *in_sep, int start, int end, int flags) const
{ return section(QString(in_sep), start, end, flags); }
#endif


class Q_EXPORT QCharRef {
    QString &s;
    int i;
    inline QCharRef(QString &str, int idx)
	: s(str),i(idx) {}
    friend class QString;
public:

    // most QChar operations repeated here

    // all this is not documented: We just say "like QChar" and let it be.
#ifndef Q_QDOC
    inline operator QChar() const
	{ return i < s.d->size ? s.d->data[i] : 0; }
    inline QCharRef &operator=(const QChar &c)
	{ if (s.d->ref != 1 || i >= s.d->size || s.d->c) s.expand(i);
	  s.d->data[i] = c.unicode();  return *this; }

    // An operator= for each QChar cast constructors
#ifndef QT_NO_CAST_FROM_ASCII
    inline QCharRef operator=(char c) { return operator=(QChar(c)); }
    inline QCharRef operator=(uchar c) { return operator=(QChar(c)); }
#endif
    inline QCharRef operator=(const QCharRef &c) { return operator=((QChar)c); }
    inline QCharRef operator=(ushort rc) { return operator=(QChar(rc)); }
    inline QCharRef operator=(short rc) { return operator=(QChar(rc)); }
    inline QCharRef operator=(uint rc) { return operator=(QChar(rc)); }
    inline QCharRef operator=(int rc) { return operator=(QChar(rc)); }

    // each function...
    inline bool isNull() const { return ((QChar)*this).isNull(); }
    inline bool isPrint() const { return ((QChar)*this).isPrint(); }
    inline bool isPunct() const { return ((QChar)*this).isPunct(); }
    inline bool isSpace() const { return ((QChar)*this).isSpace(); }
    inline bool isMark() const { return ((QChar)*this).isMark(); }
    inline bool isLetter() const { return ((QChar)*this).isLetter(); }
    inline bool isNumber() const { return ((QChar)*this).isNumber(); }
    inline bool isLetterOrNumber() { return ((QChar)*this).isLetterOrNumber(); }
    inline bool isDigit() const { return ((QChar)*this).isDigit(); }

    inline int digitValue() const { return ((QChar)*this).digitValue(); }
    QChar lower() const { return ((QChar)*this).lower(); }
    QChar upper() const { return ((QChar)*this).upper(); }

    QChar::Category category() const { return ((QChar)*this).category(); }
    QChar::Direction direction() const { return ((QChar)*this).direction(); }
    QChar::Joining joining() const { return ((QChar)*this).joining(); }
    bool mirrored() const { return ((QChar)*this).mirrored(); }
    QChar mirroredChar() const { return ((QChar)*this).mirroredChar(); }
    QString decomposition() const { return ((QChar)*this).decomposition(); }
    QChar::Decomposition decompositionTag() const { return ((QChar)*this).decompositionTag(); }
    unsigned char combiningClass() const { return ((QChar)*this).combiningClass(); }

    inline uchar cell() const { return ((QChar)*this).cell(); }
    inline uchar row() const { return ((QChar)*this).row(); }
    inline void setCell(uchar cell) { ((QChar)*this).setCell(cell); }
    inline void setRow(uchar row) { ((QChar)*this).setRow(row); }

    char ascii() const { return ((QChar)*this).ascii(); }
    char latin1() const { return ((QChar)*this).latin1(); }
    ushort unicode() const { return ((QChar)*this).unicode(); }
#endif
};
inline QCharRef QString::operator[](int i)
{ Q_ASSERT(i >= 0); return QCharRef(*this, i); }
inline QCharRef QString::operator[](uint i)
{ return QCharRef(*this, i); }
inline QString::Iterator QString::begin()
{ detach(); return (QChar*) d->data; }
inline QString::ConstIterator QString::begin() const
{ return (const QChar*)d->data; }
inline QString::ConstIterator QString::constBegin() const
{ return (const QChar*)d->data; }
inline QString::Iterator QString::end()
{ detach(); return (QChar*) d->data + d->size; }
inline QString::ConstIterator QString::end() const
{ return (const QChar*)d->data + d->size; }
inline QString::ConstIterator QString::constEnd() const
{ return (const QChar*)d->data + d->size; }
inline QBool QString::contains(const QString &s, CaseSensitivity cs) const
{ return QBool(indexOf(s, 0, cs) != -1); }
inline QBool QString::contains(QChar c, CaseSensitivity cs) const
{ return QBool(indexOf(c, 0, cs) != -1); }

Q_EXPORT bool operator!=(const QString &s1, const QString &s2);
Q_EXPORT bool operator<(const QString &s1, const QString &s2);
Q_EXPORT bool operator<=(const QString &s1, const QString &s2);
Q_EXPORT bool operator==(const QString &s1, const QString &s2);
Q_EXPORT bool operator>(const QString &s1, const QString &s2);
Q_EXPORT bool operator>=(const QString &s1, const QString &s2);
#ifndef QT_NO_CAST_FROM_ASCII
Q_EXPORT bool operator==(const QString &s1, const char *s2);
inline bool operator==(const char *s1, const QString &s2) { return (s2 == s1); }
inline bool operator!=(const QString &s1, const char *s2) { return !(s1==s2); }
inline bool operator!=(const char *s1, const QString &s2) { return !(s2==s1); }
Q_EXPORT bool operator<(const QString &s1, const char *s2);
Q_EXPORT bool operator<=(const QString &s1, const char *s2);
Q_EXPORT bool operator>(const QString &s1, const char *s2);
Q_EXPORT bool operator>=(const QString &s1, const char *s2);
Q_EXPORT bool operator<(const char *s1, const QString &s2);
Q_EXPORT bool operator<=(const char *s1, const QString &s2);
Q_EXPORT bool operator>(const char *s1, const QString &s2);
Q_EXPORT bool operator>=(const char *s1, const QString &s2);
#endif

inline const QString operator+(const QString &s1, const QString &s2)
{ return QString(s1) += s2; }
inline const QString operator+(const QString &s1, QChar s2)
{ return QString(s1) += s2; }
inline const QString operator+(QChar s1, const QString &s2)
{ return QString(s1) += s2; }
#ifndef QT_NO_CAST_FROM_ASCII
inline const QString operator+(const QString &s1, const char *s2)
{ return QString(s1) += s2; }
inline const QString operator+(const char *s1, const QString &s2)
{ return QString(s1) += s2; }
inline const QString operator+(char c, const QString &s)
{ return QString(QChar(c)) += s; }
inline const QString operator+(const QString &s, char c)
{ return QString(s) += c; }
#endif

#ifndef QT_NO_COMPAT
inline bool operator==(QString::Null, const QString &s)
{ return s.isNull(); }
inline bool operator==(const QString &s, QString::Null)
{ return s.isNull(); }
inline bool operator!=(QString::Null, const QString &s)
{ return !s.isNull(); }
inline bool operator!=(const QString &s, QString::Null)
{ return !s.isNull(); }
inline QCharRef QString::at(int i)
{ Q_ASSERT(i >= 0); return QCharRef(*this, i); }
inline QChar &QString::ref(uint i)
{ if ((int)i > d->size || d->ref != 1) resize(qMax((int)i, d->size)); return (QChar&)d->data[i]; }
#endif

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QString & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QString & );
#endif

class Q_EXPORT QConstString : public QString
{
public:
    QConstString(const QChar *unicode, int length);
    QString string() { return *this; }
};

Q_DECLARE_TYPEINFO(QString, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QString);

#if defined(Q_OS_WIN32)
extern Q_EXPORT QByteArray qt_winQString2MB( const QString& s, int len=-1 );
extern Q_EXPORT QString qt_winMB2QString( const char* mb, int len=-1 );
#endif

#endif

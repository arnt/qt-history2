#ifndef QBYTEARRAY_H
#define QBYTEARRAY_H

#ifndef QT_H
#include "qglobal.h"
#include "qatomic.h"
#endif // QT_H

#include <string.h>

/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

Q_EXPORT void *qmemmove( void *dst, const void *src, uint len );

Q_EXPORT char *qstrdup( const char * );

Q_EXPORT inline uint qstrlen( const char *str )
{ return str ? (uint)strlen(str) : 0; }
Q_EXPORT inline char *qstrcpy( char *dst, const char *src )
{ return src ? strcpy(dst, src) : 0; }
Q_EXPORT char *qstrncpy( char *dst, const char *src, uint len );

Q_EXPORT inline int qstrcmp(const char *str1, const char *str2)
{
    return (str1 && str2) ? strcmp(str1, str2)
	: (str1 ? 1 : (str2 ? -1 : 0));
}
Q_EXPORT inline int qstrncmp( const char *str1, const char *str2, uint len )
{
    return ( str1 && str2 ) ? strncmp( str1, str2, len )
	: ( str1 ? 1 : ( str2 ? -1 : 0 ) );
}
Q_EXPORT int qstricmp( const char *, const char * );
Q_EXPORT int qstrnicmp( const char *, const char *, uint len );

#ifndef QT_CLEAN_NAMESPACE
Q_EXPORT inline uint cstrlen( const char *str )
{ return (uint)strlen(str); }
Q_EXPORT inline char *cstrcpy( char *dst, const char *src )
{ return strcpy(dst,src); }
Q_EXPORT inline int cstrcmp( const char *str1, const char *str2 )
{ return strcmp(str1,str2); }
Q_EXPORT inline int cstrncmp( const char *str1, const char *str2, uint len )
{ return strncmp(str1,str2,len); }
#endif

// qChecksum: Internet checksum

Q_EXPORT Q_UINT16 qChecksum( const char *s, uint len );

class QByteRef;
class QString;
class QDataStream;

class Q_EXPORT QByteArray
{
public:
    QByteArray();
    QByteArray(const char *);
    QByteArray(const char*, int size);
#ifdef QT_NO_COMPAT
    QByteArray(int size, char c);
#endif
    QByteArray(const QByteArray &);
    ~QByteArray();

    QByteArray &operator=(const QByteArray &);
    QByteArray &operator=(const char  *);

    int size() const;
    bool isEmpty() const;
    void resize(int size);

    bool operator!() const;

    QByteArray &fill(char c, int size = -1);

    void reserve(int size);
    int capacity() const;


    operator const char *() const;
    operator const void *() const;
    char *data();
    const char *data() const;
    const char *constData() const;
    void detach();
    bool isDetached() const;
    void clear();

    const char at(int i) const;
    const char operator[](int i) const;
    QByteRef operator[](int i);
    const char operator[](uint i) const;
    QByteRef operator[](uint i);

    int indexOf(char c, int from = 0) const;
    int indexOf(const char *c, int from = 0) const;
    int indexOf(const QByteArray &a, int from = 0) const;
    int lastIndexOf(char c, int from = -1) const;
    int lastIndexOf(const char *c, int from = -1) const;
    int lastIndexOf(const QByteArray &a, int from = -1) const;

    QBool contains(char c) const;
    QBool contains(const char *a) const;
    QBool contains(const QByteArray &a) const;
    int count(char c) const;
    int count(const char *a) const;
    int count(const QByteArray &a) const;

    QByteArray left(int len)  const;
    QByteArray right(int len) const;
    QByteArray mid(int index, int len=-1) const;

    QByteArray toLower() const;
    QByteArray toUpper() const;

    QByteArray trimmed() const;
    QByteArray simplified() const;

    QByteArray &prepend(char c);
    QByteArray &prepend(const char *s);
    QByteArray &prepend(const QByteArray &a);
    QByteArray &append(char c);
    QByteArray &append(const char *s);
    QByteArray &append(const QByteArray &a);
    QByteArray &insert(int i, char c);
    QByteArray &insert(int i, const char *s);
    QByteArray &insert(int i, const QByteArray &a);
    QByteArray &remove(int index, int len);
    QByteArray &replace(int index, int len, const char *s);
    QByteArray &replace(int index, int len, const QByteArray &s);
    QByteArray &replace(char before, const char *after);
    QByteArray &replace(char before, const QByteArray &after);
    QByteArray &replace(const char *before, const char *after);
    QByteArray &replace(const QByteArray &before, const QByteArray &after);
    QByteArray &replace(char before, char after);
    QByteArray &operator+=(char c);
    QByteArray &operator+=(const char *s);
    QByteArray &operator+=(const QByteArray &a);

    typedef char *Iterator;
    typedef const char *ConstIterator;
    Iterator begin();
    ConstIterator begin() const;
    ConstIterator constBegin() const;
    Iterator end();
    ConstIterator end() const;
    ConstIterator constEnd() const;

    // stl compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    void push_back(char c);
    void push_back(const char *c);
    void push_back(const QByteArray &a);
    void push_front(char c);
    void push_front(const char *c);
    void push_front(const QByteArray &a);

    // compatibility
#ifndef QT_NO_COMPAT
    explicit QByteArray(int size, char c = '\0');
    inline bool isNull() const
    { return d == &shared_null; }
    inline int count() const
    { return d->size; }
    QByteArray& duplicate(const QByteArray& a)
    { *this = a; return *this; }
    QByteArray& duplicate(const char *a, uint n)
    { *this = QByteArray(a, n); return *this; }
    QByteArray& setRawData( const char *a, uint n )
    { detach(); d->data = (char *)a; d->size = n; return *this; }
    void resetRawData( const char *, uint )
    { detach(); d->data = d->array; d->size = 0; }
    int length() const { return size(); }
    inline void truncate( int l ) { resize(l); }
    inline QByteArray lower() const { return toLower(); }
    inline QByteArray upper() const { return toUpper(); }
    inline QByteArray stripWhiteSpace() const { return trimmed(); }
    inline QByteArray simplifyWhiteSpace() const { return simplified(); }
    inline int find(char c, int from = 0) const { return indexOf(c, from); }
    inline int find(const char *c, int from = 0) const { return indexOf(c, from); }
    inline int findRev(char c, int from = -1) const { return lastIndexOf(c, from); }
    inline int findRev(const char *c, int from = -1) const { return lastIndexOf(c, from); }
#endif

    inline bool ensure_constructed()
    { if (!d) { d = &shared_null; ++d->ref; return false; } return true; }

private:
    struct Data {
	QAtomic ref;
	int alloc, size;
	char *data;
	char array[1];
    };
    static Data shared_null;
    static Data shared_empty;
    Data *d;
    QByteArray(Data *dd) : d(dd) {}
    void realloc(int alloc);
    void expand(int i);
    friend class QByteRef;
    friend class QString;
    friend class QConstByteArray;
};

class Q_EXPORT QConstByteArray : public QByteArray
{
public:
    QConstByteArray(const char *chars, int length);
};

inline QByteArray:: QByteArray():d(&shared_null)
{ ++d->ref; }
inline int QByteArray::size() const
{ return d->size; }
inline const char QByteArray::at(int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i]; }
inline const char QByteArray::operator[](int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i]; }
inline const char QByteArray::operator[](uint i) const
{ Q_ASSERT(i < (uint)size()); return d->data[i]; }
inline bool QByteArray::isEmpty() const
{ return d->size == 0; }
inline bool QByteArray::operator!() const
{ return d->size == 0; }
inline QByteArray::operator const char *() const
{ return d->data; }
inline QByteArray::operator const void *() const
{ return d->data; }
inline char *QByteArray::data()
{ detach(); return d->data; }
inline const char *QByteArray::data() const
{ return d->data; }
inline const char *QByteArray::constData() const
{ return d->data; }
inline void QByteArray::detach()
{ if (d->ref != 1 || d->data != d->array) realloc(d->size); }
inline bool QByteArray::isDetached() const
{ return d->ref == 1; }
inline QByteArray::~QByteArray()
{ if (!--d->ref) qFree(d); }
inline void QByteArray::clear()
{ if (!--d->ref) qFree(d); d = &shared_null; ++d->ref; }
inline QByteArray::QByteArray(const QByteArray &a) : d(a.d)
{ ++d->ref; }
inline int QByteArray::capacity() const
{ return d->alloc; }

class Q_EXPORT QByteRef {
    QByteArray &a;
    int i;
    inline QByteRef(QByteArray &array, int idx)
	: a(array),i(idx) {}
    friend class QByteArray;
public:
    inline operator const char() const
	{ return i < a.d->size ? a.d->data[i] : 0; }
    inline QByteRef &operator=(char c)
	{ if (a.d->ref != 1 || i >= a.d->size) a.expand(i);
	  a.d->data[i] = c;  return *this; }
    inline QByteRef &operator=(const QByteRef &c)
	{ if (a.d->ref != 1 || i >= a.d->size) a.expand(i);
	  a.d->data[i] = c.a.d->data[c.i];  return *this; }
    inline bool operator==(char c) const
    { return a.d->data[i] == c; }
    inline bool operator!=(char c) const
    { return a.d->data[i] != c; }
    inline bool operator>(char c) const
    { return a.d->data[i] > c; }
    inline bool operator>=(char c) const
    { return a.d->data[i] >= c; }
    inline bool operator<(char c) const
    { return a.d->data[i] < c; }
    inline bool operator<=(char c) const
    { return a.d->data[i] <= c; }
};

inline QByteRef QByteArray::operator[](int i)
{ Q_ASSERT(i >= 0); return QByteRef(*this, i); }
inline QByteRef QByteArray::operator[](uint i)
{ return QByteRef(*this, i); }
inline QByteArray::Iterator QByteArray::begin()
{ detach(); return d->data; }
inline QByteArray::ConstIterator QByteArray::begin() const
{ return d->data; }
inline QByteArray::ConstIterator QByteArray::constBegin() const
{ return d->data; }
inline QByteArray::Iterator QByteArray::end()
{ detach(); return d->data + d->size; }
inline QByteArray::ConstIterator QByteArray::end() const
{ return d->data + d->size; }
inline QByteArray::ConstIterator QByteArray::constEnd() const
{ return d->data + d->size; }
inline QByteArray &QByteArray::operator+=(char c)
{ return append(c); }
inline QByteArray &QByteArray::operator+=(const char *s)
{ return append(s); }
inline QByteArray &QByteArray::operator+=(const QByteArray &a)
{ return append(a); }
inline void QByteArray::push_back(char c)
{ append(c); }
inline void QByteArray::push_back(const char *c)
{ append(c); }
inline void QByteArray::push_back(const QByteArray &a)
{ append(a); }
inline void QByteArray::push_front(char c)
{ prepend(c); }
inline void QByteArray::push_front(const char *c)
{ prepend(c); }
inline void QByteArray::push_front(const QByteArray &a)
{ prepend(a); }
inline QBool QByteArray::contains(const QByteArray &a) const
{ return QBool(indexOf(a) != -1); }
inline QBool QByteArray::contains(char c) const
{ return QBool(indexOf(c) != -1); }
inline QByteArray &QByteArray::operator=(const QByteArray & a)
{
    Data *x = a.d; ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref) qFree(x);
    return *this;
}

inline bool operator==(const QByteArray &a1, const QByteArray &a2)
{ return (a1.size() == a2.size()) && (memcmp(a1, a2, a1.size())==0); }
inline bool operator==(const QByteArray &a1, const char *a2)
{ return a2 ? strcmp(a1,a2) == 0 : a1.isEmpty(); }
inline bool operator==(const char *a1, const QByteArray &a2)
{ return a1 ? strcmp(a1,a2) == 0 : a2.isEmpty(); }
inline bool operator!=(const QByteArray &a1, const QByteArray &a2)
{ return !(a1==a2); }
inline bool operator!=(const QByteArray &a1, const char *a2)
{ return a2 ? strcmp(a1,a2) != 0 : !a1.isEmpty(); }
inline bool operator!=(const char *a1, const QByteArray &a2)
{ return a1 ? strcmp(a1,a2) != 0 : !a2.isEmpty(); }
inline bool operator<(const QByteArray &a1, const QByteArray &a2)
{ return strcmp(a1, a2) < 0; }
 inline bool operator<(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) < 0; }
inline bool operator<(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) < 0; }
inline bool operator<=(const QByteArray &a1, const QByteArray &a2)
{ return strcmp(a1, a2) <= 0; }
inline bool operator<=(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) <= 0; }
inline bool operator<=(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) <= 0; }
inline bool operator>(const QByteArray &a1, const QByteArray &a2)
{ return strcmp(a1, a2) > 0; }
inline bool operator>(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) > 0; }
inline bool operator>(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) > 0; }
inline bool operator>=(const QByteArray &a1, const QByteArray &a2)
{ return strcmp(a1, a2) >= 0; }
inline bool operator>=(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) >= 0; }
inline bool operator>=(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) >= 0; }
inline const QByteArray operator+(const QByteArray &a1, const QByteArray &a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const QByteArray &a1, const char *a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const QByteArray &a1, char a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const char *a1, const QByteArray &a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(char a1, const QByteArray &a2)
{ return QByteArray(&a1, 1) += a2; }
inline int QByteArray::indexOf(const char *c, int i) const
{ QConstByteArray cb(c, strlen(c)); return indexOf(cb, i); }
inline int QByteArray::lastIndexOf(const char *c, int i) const
{ QConstByteArray cb(c, strlen(c)); return lastIndexOf(cb, i); }
inline QBool QByteArray::contains(const char *c) const
{ QConstByteArray cb(c, strlen(c)); return contains(cb); }
inline QByteArray &QByteArray::replace(int index, int len, const char *s)
{ QConstByteArray cb(s, strlen(s)); return replace(index, len, cb); }
inline QByteArray &QByteArray::replace(char before, const char *after)
{ QConstByteArray cb(after, strlen(after)); return replace(before, cb); }
inline QByteArray &QByteArray::replace(const char *before, const char *after)
{ QConstByteArray cb(before, strlen(before)); QConstByteArray ca(after, strlen(after)); return replace(cb, ca); }


#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QByteArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QByteArray & );
#endif

#ifndef QT_NO_COMPRESS
Q_EXPORT QByteArray qCompress( const uchar* data, int nbytes );
Q_EXPORT QByteArray qUncompress( const uchar* data, int nbytes );
Q_EXPORT inline QByteArray qCompress( const QByteArray& data)
{ return qCompress( (const uchar*)data.constData(), data.size() ); }
Q_EXPORT inline QByteArray qUncompress( const QByteArray& data )
{ return qUncompress( (const uchar*)data.constData(), data.size() ); }
#endif


Q_DECLARE_TYPEINFO(QByteArray, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QByteArray);

#endif // QBYTEARRAY_H

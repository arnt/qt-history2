#ifndef  QVECTOR_H
#define QVECTOR_H

#ifndef QT_H
#include "qiterator.h"
#include "qatomic.h"
#include "qtl.h"
#include <new>
#endif // QT_H

struct Q_EXPORT QVectorData
{
    QAtomic ref;
    int alloc, size;
    static QVectorData shared_null;
    static QVectorData* malloc(int size, int sizeofT);
    static QVectorData* malloc(int size, int sizeofT, QVectorData* init);
    QVectorData* realloc(int size, int sizeofT);
    static int grow(int size, int sizeofT, bool excessive);
};

template <typename T>
class QVector
{
 public:
    inline QVector() : p(&QVectorData::shared_null) { ++d->ref; }
    explicit QVector(int size);
    QVector(int size, const T &t);
    inline QVector(const QVector &v) : d(v.d) { ++d->ref; }
    inline ~QVector() { if (!--d->ref) free(d); }
    QVector &operator=(const QVector  &a);
    bool operator== (const QVector &v) const;
    inline bool operator!= (const QVector &v) const { return !(*this == v); }

    inline int size() const { return d->size; }
    inline bool isEmpty() const { return d->size == 0; }
    void resize(int size);

    void reserve(int size);
    inline int capacity() const { return d->alloc; }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }

    inline bool operator!() const { return d->size == 0; }
    inline operator const T*() const { return d->array; }
    inline operator const void*() const { return d->array; }
    inline T* data() { detach(); return d->array; }
    inline const T* data() const { return d->array; }
    inline const T* constData() const { return d->array; }
    void clear();

    const T &at(int i) const;
    T &operator[](int i);
    const T &operator[](int i) const;
    void append(const T &t);

    QVector &fill(const T &t, int size = - 1);

    int indexOf(const T &t, int from = 0) const;
    int lastIndexOf(const T &t, int from = -1) const;
    bool contains(const T &t) const;
    int count(const T &t) const;

    // stl style
    typedef T* Iterator;
    typedef const T* ConstIterator;
    inline Iterator begin() { detach(); return d->array; }
    inline ConstIterator begin() const { return d->array; }
    inline ConstIterator constBegin() const { return d->array; }
    inline Iterator end() { detach(); return d->array + d->size; }
    inline ConstIterator end() const { return d->array + d->size; }
    inline ConstIterator constEnd() const { return d->array + d->size; }
    Iterator insert( Iterator pos, int n, const T& x );
    inline Iterator insert( Iterator pos, const T& x ) { return insert(pos, 1, x); }
    Iterator erase( Iterator first, Iterator last );
    inline Iterator erase( Iterator pos ) { return erase(pos, pos+1); }

    // more Qt
    inline int count() const { return d->size; }
    inline T& first() { Q_ASSERT(!isEmpty()); return *begin(); }
    inline const T& first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    inline T& last() { Q_ASSERT(!isEmpty()); return *(end()-1); }
    inline const T& last() const { Q_ASSERT(!isEmpty()); return *(end()-1); }

    // stl compatibility
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
#ifndef QT_NO_STL
    typedef ptrdiff_t difference_type;
#else
    typedef int difference_type;
#endif
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    typedef int size_type;
    inline void push_back(const T &t)
    { append(t); }
    void pop_back() { Q_ASSERT(!isEmpty()); erase(end()-1); }
    inline bool empty() const
    { return d->size == 0; }
    inline T& front() { return first(); }
    inline const_reference front() const { return first(); }
    inline reference back() { return last(); }
    inline const_reference back() const { return last(); }

#ifndef QT_NO_COMPAT
    // compatibility
#endif

    //comfort
    QVector &operator+=(const QVector &l);
    inline QVector operator+(const QVector &l) const
    { QVector n = *this; n += l; return n; }
    inline void operator+=(const T &t)
    { append(t); }
    inline QVector &operator<< (const T &t)
    { append(t); return *this; }

    inline bool ensure_constructed()
    { if (!p) { p = &QVectorData::shared_null; ++p->ref; return false; } return true; }

private:
    void detach_helper();
    void realloc(int size, int alloc);
    struct Data {
	QAtomic ref;
	int alloc, size;
	T array[1];
    };
    void free(Data *d);
    union { QVectorData* p; Data* d; };
};

template <typename T>
void QVector<T>::detach_helper()
{ realloc(d->size, d->alloc); }
template <typename T>
void QVector<T>::reserve(int size)
{ if (size > d->alloc) realloc(d->size, size); }
template <typename T>
void QVector<T>::resize(int size)
{ realloc(size, (size>d->alloc||(size<d->size && size < (d->alloc >> 1))) ?
	   QVectorData::grow(size, sizeof(T), QTypeInfo<T>::isStatic)
	   : d->alloc); }
template <typename T>
inline void QVector<T>::clear()
{ *this = QVector<T>(); }
template <typename T>
inline const T &QVector<T>::at(int i) const
{ Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::at", "index out of range");
  return d->array[i]; }
template <typename T>
inline const T &QVector<T>::operator[](int i) const
{ Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::operator[]", "index out of range");
  return d->array[i]; }
template <typename T>
inline T &QVector<T>::operator[](int i)
{ Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::operator[]", "index out of range");
  return data()[i]; }
template <typename T>
QVector<T> &QVector<T>::operator=(const QVector<T> &v)
{
    typename QVector::Data *x = v.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	free(x);
    return *this;
}

template <typename T>
QVector<T>::QVector(int size)
{
    p = QVectorData::malloc(size, sizeof(T));
    d->ref = 1;
    d->alloc = d->size = size;
    if (QTypeInfo<T>::isComplex) {
	T* b = d->array;
	T* i = d->array + d->size;
	while (i != b)
	    new (--i) T;
    }
}

template <typename T>
QVector<T>::QVector(int size, const T &t)
{
    p = QVectorData::malloc(size, sizeof(T));
    d->ref = 1;
    d->alloc = d->size = size;
    T* i = d->array + d->size;
    while (i != d->array)
	new (--i) T(t);
}

template <typename T>
void QVector<T>::free(Data *d)
{
    if (QTypeInfo<T>::isComplex) {
	T* b = d->array;
	T* i = b + d->size;
	while (i-- != b)
	    i->~T();
    }
    qFree(d);
}

template <typename T>
void QVector<T>::realloc(int size, int alloc)
{
    T *j, *i, *b;
    union { QVectorData *p; Data *d; } x;
    x.d = d;
    if (alloc != d->alloc || d->ref != 1) {
	if (QTypeInfo<T>::isStatic)
	    x.p = QVectorData::malloc(alloc, sizeof(T));
	else if (d->ref != 1)
	    x.p = QVectorData::malloc(alloc, sizeof(T), p);
	else
	    x.p = p = p->realloc(alloc, sizeof(T));
	x.d->ref = 1;
    }
    if (QTypeInfo<T>::isComplex) {
	if (size < d->size) {
	    i = d->array + d->size;
	    j = d->array + size;
	    while (i-- != j)
		i->~T();
	    i = x.d->array + size;
	} else {
	    i = x.d->array + size;
	    j = x.d->array + d->size;
	    while (i != j)
		new (--i) T;
	    j = d->array + d->size;
	}
	if (i != j) {
	    b = x.d->array;
	    while (i != b)
		new (--i) T(*--j);
	}
    }
    x.d->size = size;
    x.d->alloc = alloc;
    if (d != x.d) {
	x.d = qAtomicSetPtr(&d, x.d);
	if (!--x.d->ref)
	    free(x.d);
    }
}

template <typename T>
void QVector<T>::append(const T &t)
{
    if (d->ref != 1 || d->size +1 > d->alloc)
	realloc(d->size, QVectorData::grow(d->size+1, sizeof(T),
					   QTypeInfo<T>::isStatic));
    if (QTypeInfo<T>::isComplex)
	new (d->array + d->size++) T(t);
    else
	d->array[d->size++] = t;
}


template <typename T>
typename QVector<T>::Iterator QVector<T>::insert( Iterator pos, size_type n, const T& t )
{
    int p = pos - d->array;
    if ( n != 0 ) {
	if (d->ref != 1 || d->size + n > d->alloc)
	    realloc(d->size, QVectorData::grow(d->size+n, sizeof(T),
					       QTypeInfo<T>::isStatic));
	if (QTypeInfo<T>::isComplex) {
	    T *b = d->array+d->size;
	    T *i = d->array+d->size+n;
	    while (i != b)
		new (--i) T;
	    i = d->array+d->size;
	    T *j = i+n;
	    b = d->array+p;
	    while (i != b)
		*--j = *--i;
	    i = b+n;
	    while (i != b)
		*--i = t;
	} else {
	    T *b = d->array+p;
	    T *i = b+n;
	    memmove(i, b, (d->size-p)*sizeof(T));
	    while (i!= b)
		*(--i) = t;
	}
    }
    d->size += n;
    return d->array+p;
}

template <typename T>
typename QVector<T>::Iterator QVector<T>::erase( Iterator first, Iterator last )
{
    int f = first - d->array;
    int l = last - d->array;
    int n = l - f;
    detach();
    if (QTypeInfo<T>::isComplex) {
	qCopy( d->array+l, d->array+d->size, d->array+f );
	T *i = d->array+d->size;
	T* b = d->array+d->size-n;
	while (i != b)
	    (--i)->~T();
    } else {
	memmove(d->array + f, d->array + l, (d->size-l)*sizeof(T));
    }
    d->size -= n;
    return d->array + f;
}

template <typename T>
bool QVector<T>::operator== (const QVector<T> &v) const
{
    if (d->size != v.d->size)
	return false;
    if (d == v.d)
	return true;
    T* b = d->array;
    T* i = b + d->size;
    T* j = v.d->array + d->size;
    while (i != b)
	if (!(*--i == *--j))
	    return false;
    return true;
}

template <typename T>
QVector<T> &QVector<T>::fill(const T &t, int size)
{
    resize(size < 0 ? d->size : size);
    if (d->size) {
	T* i = d->array + d->size;
	T* b = d->array;
	while (i != b)
	    *--i = t;
    }
    return *this;
}

template <typename T>
QVector<T>  &QVector<T>::operator+=(const QVector &l)
{
    realloc(d->size, d->size + l.d->size);
    d->size += l.d->size;
    T* w = d->array + d->size;
    T* i = l.d->array + l.d->size;
    T* b = l.d->array;
    while (i != b)
	if (QTypeInfo<T>::isComplex)
	    new (--w) T(*--i);
	else
	    *--w = *--i;
    return *this;
}

template <typename T>
int QVector<T>::indexOf(const T &t, int from) const
{
    if (from < 0)
	from = QMAX(from + d->size, 0);
    if (from < d->size) {
	T* n = d->array + from - 1;
	T* e = d->array + d->size;
	while (++n != e)
	    if (*n == t)
		return n - d->array;
    }
    return -1;
}

template <typename T>
int QVector<T>::lastIndexOf(const T &t, int from) const
{
    if (from < 0)
	from += d->size;
    else if (from >= d->size)
	from = d->size-1;
    if (from >= 0) {
	T* b = d->array;
	T* n = d->array + from + 1;
	while (n != b) {
	    if (*--n == t)
		return n - b;
	}
    }
    return -1;
}

template <typename T>
bool QVector<T>::contains(const T &t) const
{
    T* b = d->array;
    T* i = d->array + d->size;
    while (i != b)
	if (*--i == t)
	    return true;
    return false;
}

template <typename T>
int QVector<T>::count(const T &t) const
{
    int c = 0;
    T* b = d->array;
    T* i = d->array + d->size;
    while (i != b)
	if (*--i == t)
	    ++c;
    return c;
}

Q_DECLARE_ITERATOR(QVector)


#ifndef QT_NO_DATASTREAM
class QDataStream;

template<typename T>
QDataStream& operator>>( QDataStream& s, QVector<T>& v )
{
    v.clear();
    Q_UINT32 c;
    s >> c;
    v.resize( c );
    for( Q_UINT32 i = 0; i < c; ++i ) {
	T t;
	s >> t;
	v[i] = t;
    }
    return s;
}

template<typename T>
QDataStream& operator<<( QDataStream& s, const QVector<T>& v )
{
    s << (Q_UINT32)v.size();
    const T* it = v.begin();
    for( ; it != v.end(); ++it )
	s << *it;
    return s;
}
#endif // QT_NO_DATASTREAM

#endif // QVECTOR_H

#ifndef  QVECTOR_H
#define QVECTOR_H

#ifndef QT_H
#include "qiterator.h"
#include "qatomic.h"
#include <new>
#endif // QT_H

struct QVectorData
{
    QAtomic ref;
    int alloc, size;
    static QVectorData shared_null;
    static QVectorData* malloc(int size, int sizeofT);
    static QVectorData* malloc(int size, int sizeofT, QVectorData* init);
    QVectorData* realloc(int size, int sizeofT);
    static int grow(int size, int sizeofT, bool excessive);
};

template <class T>
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

    inline operator const T*() const { return d->array; }
    inline const T* data() const { return d->array; }
    inline T* detach() { if (d->ref != 1) detach_helper(); assert(d->ref == 1);return d->array; }
    inline bool isDetached() const { return d->ref == 1; }
    void clear();

    const T &at(int i) const;
    T &operator[](int i);
    const T &operator[](int i) const;
    void append(const T &t);

    QVector &fill(const T &t, int size = - 1);

    int find(const T &t, int i = 0) const;
    int findRev(const T &t, int i = -1) const;
    bool contains(const T &t) const;
    int count(const T &t) const;

    typedef T* Iterator;
    typedef const T* ConstIterator;

    inline Iterator begin() { detach(); return d->array; }
    inline ConstIterator begin() const { return d->array; }
    inline ConstIterator constBegin() const { return d->array; }
    inline Iterator end() { detach(); return d->array + d->size; }
    inline ConstIterator end() const { return d->array + d->size; }
    inline ConstIterator constEnd() const { return d->array + d->size; }

    // stl compatibility
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    inline void push_back(const T &t)
    { return append(t); }
    inline bool empty() const
    { return d->size == 0; }

#ifndef QT_NO_COMPAT
    // compatibility
    inline int count() const
    { return d->size; }
#endif

    //comfort
    QVector &operator+=(const QVector &l);
    QVector operator+(const QVector &l) const
    { QVector n = *this; n += l; return n; }
    inline void operator+=(const T &t)
    { append(t); }
    inline QVector &operator<< (const T &t)
    { append(t); return *this; }

    inline bool ensure_constructed()
    { if (!d) { d = &QVectorData::shared_null; ++d->ref; return false; } return true; }

private:
    void detach_helper();
    void realloc(int size, int alloc);
    struct Data {
	int ref, alloc, size;
	T array[1];
    };
    void free(Data *d);
    union { QVectorData* p; Data* d; };
};

template <class T>
void QVector<T>::detach_helper()
{ realloc(d->size, d->alloc); }
template <class T>
void QVector<T>::reserve(int size)
{ if (size > d->alloc) realloc(d->size, size); }
template <class T>
void QVector<T>::resize(int size)
{ realloc(size, (size>d->alloc||(size<d->size && size < (d->alloc >> 1))) ?
	   QVectorData::grow(size, sizeof(T), Q_TYPEINFO_STATIC(T))
	   : d->alloc); }
template <class T>
inline void QVector<T>::clear()
{ *this = QVector<T>(); }
template <class T>
inline const T &QVector<T>::at(int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->array[i]; }
template <class T>
inline const T &QVector<T>::operator[](int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->array[i]; }
template <class T>
inline T &QVector<T>::operator[](int i)
{ Q_ASSERT(i >= 0 && i < size()); return detach()[i]; }
template <class T>
QVector<T> &QVector<T>::operator=(const QVector<T> &v)
{
    QVector::Data *x = v.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	free(x);
    return *this;
}

template <class T>
QVector<T>::QVector(int size)
{
    p = QVectorData::malloc(size, sizeof(T));
    d->ref = 1;
    d->alloc = d->size = size;
    if (Q_TYPEINFO_COMPLEX(T)) {
	T* b = d->array;
	T* i = d->array + d->size;
	while (i != b)
	    new (--i) T;
    }
}

template <class T>
QVector<T>::QVector(int size, const T &t)
{
    p = QVectorData::malloc(size, sizeof(T));
    d->ref = 1;
    d->alloc = d->size = size;
    T* i = d->array + d->size;
    while (i != d->array)
	new (--i) T(t);
}

template <class T>
void QVector<T>::free(Data *d)
{
    if (Q_TYPEINFO_COMPLEX(T)) {
	T* b = d->array;
	T* i = b + d->size;
	while (i-- != b)
	    i->~T();
    }
    qFree(d);
}

template <class T>
void QVector<T>::realloc(int size, int alloc)
{
    T *j, *i, *b;
    union { QVectorData *p; Data *d; } x;
    x.d = d;
    if (alloc != d->alloc || d->ref != 1) {
	if (Q_TYPEINFO_STATIC(T))
	    x.p = QVectorData::malloc(alloc, sizeof(T));
	else if (d->ref != 1)
	    x.p = QVectorData::malloc(alloc, sizeof(T), p);
	else
	    x.p = p = p->realloc(alloc, sizeof(T));
	x.d->ref = 1;
    }
    if (Q_TYPEINFO_COMPLEX(T)) {
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

template <class T>
void QVector<T>::append(const T &t)
{
    if (d->ref != 1 || d->size +1 > d->alloc)
	realloc(d->size,
		QVectorData::grow(d->size+1, sizeof(T),
				     Q_TYPEINFO_STATIC(T)));
    if (Q_TYPEINFO_COMPLEX(T))
	new (d->array + d->size++) T(t);
    else
	d->array[d->size++] = t;
}

template <class T>
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

template <class T>
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

template <class T>
QVector<T>  &QVector<T>::operator+=(const QVector &l)
{
    realloc(d->size, d->size + l.d->size);
    d->size += l.d->size;
    T* w = d->array + d->size;
    T* i = l.d->array + l.d->size;
    T* b = l.d->array;
    while (i != b)
	if (Q_TYPEINFO_COMPLEX(T))
	    new (--w) T(*--i);
	else
	    *--w = *--i;
    return *this;
}

template <class T>
int QVector<T>::find(const T &t, int i) const
{
    if (i < 0)
	i = QMAX(i + d->size, 0);
    if (i < d->size) {
	T* n = d->array + i - 1;
	T* e = d->array + d->size;
	while (++n != e)
	    if (*n == t)
		return n - d->array;
    }
    return -1;
}

template <class T>
int QVector<T>::findRev(const T &t, int i) const
{
    if (i < 0)
	i += d->size();
    else if (i >= d->size())
	i = d->size()-1;
    if (i >= 0) {
	T* b = d->array;
	T* n = d->array + i + 1;
	while (n != b) {
	    if (*--n == t)
		return n - b;
	}
    }
    return -1;
}

template <class T>
bool QVector<T>::contains(const T &t) const
{
    T* b = d->array;
    T* i = d->array + d->size;
    while (i != b)
	if (*--i == t)
	    return true;
    return false;
}

template <class T>
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

#endif // QVECTOR_H

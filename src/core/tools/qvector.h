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

#ifndef QVECTOR_H
#define QVECTOR_H

#include "qiterator.h"
#include "qatomic.h"
#include "qalgorithms.h"

struct Q_CORE_EXPORT QVectorData
{
    QBasicAtomic ref;
    int alloc;
    int size;
    uint sharable : 1;

    static QVectorData shared_null;
    static QVectorData *malloc(int sizeofTypedData, int size, int sizeofT, QVectorData *init);
    static int grow(int sizeofTypedData, int size, int sizeofT, bool excessive);
};

template <typename T>
struct QVectorTypedData
{
    QBasicAtomic ref;
    int alloc;
    int size;
    uint sharable : 1;
    T array[1];
};

template <typename T>
class QVector
{
    typedef QVectorTypedData<T> Data;
    union { QVectorData *p; QVectorTypedData<T> *d; };

public:
    inline QVector() : p(&QVectorData::shared_null) { ++d->ref; }
    explicit QVector(int size);
    QVector(int size, const T &t);
    inline QVector(const QVector &v) : d(v.d) { ++d->ref; if (!d->sharable) detach_helper(); }
    inline ~QVector() { if (!d) return; if (!--d->ref) free(d); }
    QVector &operator=(const QVector &v);
    bool operator==(const QVector &v) const;
    inline bool operator!=(const QVector &v) const { return !(*this == v); }

    inline int size() const { return d->size; }

    inline bool isEmpty() const { return d->size == 0; }

    void resize(int size);

    inline int capacity() const { return d->alloc; }
    void reserve(int size);
    inline void squeeze() { realloc(d->size, d->size); }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }
    inline void setSharable(bool sharable) { if (!sharable) detach(); d->sharable = sharable; }

    inline T *data() { detach(); return d->array; }
    inline const T *data() const { return d->array; }
    inline const T *constData() const { return d->array; }
    void clear();

    const T &at(int i) const;
    T &operator[](int i);
    const T &operator[](int i) const;
    void append(const T &t);
    void prepend(const T &t);
    void insert(int i, const T &t);
    void insert(int i, int n, const T &t);
    void replace(int i, const T &t);
    void remove(int i);
    void remove(int i, int n);

    QVector &fill(const T &t, int size = -1);

    int indexOf(const T &t, int from = 0) const;
    int lastIndexOf(const T &t, int from = -1) const;
    bool contains(const T &t) const;
    int count(const T &t) const;

    // STL-style
    typedef T* iterator;
    typedef const T* const_iterator;
    inline iterator begin() { detach(); return d->array; }
    inline const_iterator begin() const { return d->array; }
    inline const_iterator constBegin() const { return d->array; }
    inline iterator end() { detach(); return d->array + d->size; }
    inline const_iterator end() const { return d->array + d->size; }
    inline const_iterator constEnd() const { return d->array + d->size; }
    iterator insert(iterator before, int n, const T &x);
    inline iterator insert(iterator before, const T &x) { return insert(before, 1, x); }
    iterator erase(iterator begin, iterator end);
    inline iterator erase(iterator pos) { return erase(pos, pos+1); }

    // more Qt
    inline int count() const { return d->size; }
    inline T& first() { Q_ASSERT(!isEmpty()); return *begin(); }
    inline const T& first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    inline T& last() { Q_ASSERT(!isEmpty()); return *(end()-1); }
    inline const T& last() const { Q_ASSERT(!isEmpty()); return *(end()-1); }
    QVector<T> mid(int pos, int length = -1) const;

    T value(int i) const;
    T value(int i, const T &defaultValue) const;

    // STL compatibility
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
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    typedef int size_type;
    inline void push_back(const T &t) { append(t); }
    inline void push_front(const T &t) { prepend(t); }
    void pop_back() { Q_ASSERT(!isEmpty()); erase(end()-1); }
    void pop_front() { Q_ASSERT(!isEmpty()); erase(begin()); }
    inline bool empty() const
    { return d->size == 0; }
    inline T& front() { return first(); }
    inline const_reference front() const { return first(); }
    inline reference back() { return last(); }
    inline const_reference back() const { return last(); }

    //comfort
    QVector &operator+=(const QVector &l);
    inline QVector operator+(const QVector &l) const
    { QVector n = *this; n += l; return n; }
    inline void operator+=(const T &t)
    { append(t); }
    inline QVector &operator<< (const T &t)
    { append(t); return *this; }

private:
    void detach_helper();
    QVectorData *malloc(int alloc);
    void realloc(int size, int alloc);
    void free(Data *d);
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
          QVectorData::grow(sizeof(Data), size, sizeof(T), QTypeInfo<T>::isStatic)
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
inline void QVector<T>::insert(int i, const T &t)
{ Q_ASSERT_X(i >= 0 && i <= d->size, "QVector<T>::insert", "index out of range");
  insert(begin()+i, 1, t); }
template <typename T>
inline void QVector<T>::insert(int i, int n, const T &t)
{ Q_ASSERT_X(i >= 0 && i <= d->size, "QVector<T>::insert", "index out of range");
  insert(begin() + i, n, t); }
template <typename T>
inline void QVector<T>::remove(int i, int n)
{ Q_ASSERT_X(i >= 0 && i + n <= d->size, "QVector<T>::remove", "index out of range");
  erase(begin() + i, begin() + i + n); }
template <typename T>
inline void QVector<T>::remove(int i)
{ Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::remove", "index out of range");
  erase(begin() + i, begin() + i + 1); }
template <typename T>
inline void QVector<T>::prepend(const T &t)
{ insert(begin(), 1, t); }
template <typename T>
inline void QVector<T>::replace(int i, const T &t)
{ Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::replace", "index out of range");
  data()[i] = t; }
template <typename T>
QVector<T> &QVector<T>::operator=(const QVector<T> &v)
{
    typename QVector::Data *x = v.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        free(x);
    if (!d->sharable)
        detach_helper();
    return *this;
}

template <typename T>
inline QVectorData *QVector<T>::malloc(int alloc)
{
    return static_cast<QVectorData *>(qMalloc(sizeof(Data) + (alloc - 1) * sizeof(T)));
}

template <typename T>
QVector<T>::QVector(int size)
{
    p = malloc(size);
    d->ref.init(1);
    d->alloc = d->size = size;
    d->sharable = true;
    if (QTypeInfo<T>::isComplex) {
        T* b = d->array;
        T* i = d->array + d->size;
        while (i != b)
            new (--i) T;
    } else {
        qMemSet(d->array, 0, size * sizeof(T));
    }
}

template <typename T>
QVector<T>::QVector(int size, const T &t)
{
    p = malloc(size);
    d->ref.init(1);
    d->alloc = d->size = size;
    d->sharable = true;
    T* i = d->array + d->size;
    while (i != d->array)
        new (--i) T(t);
}

template <typename T>
void QVector<T>::free(Data *x)
{
    if (QTypeInfo<T>::isComplex) {
        T* b = x->array;
        T* i = b + x->size;
        while (i-- != b)
             i->~T();
    }
    qFree(x);
}

template <typename T>
void QVector<T>::realloc(int size, int alloc)
{
    T *j, *i, *b;
    union { QVectorData *p; Data *d; } x;
    x.d = d;

    if (QTypeInfo<T>::isComplex && alloc == d->alloc && d->ref == 1) {
        // pure resize
        i = d->array + d->size;
        j = d->array + size;
        if (i > j) {
            while (i-- != j)
                i->~T();
        } else {
            while (j-- != i)
                new (j) T;
        }
        d->size = size;
        return;
    }

    if (alloc != d->alloc || d->ref != 1) {
        // (re)allocate memory
        if (QTypeInfo<T>::isStatic) {
            x.p = malloc(alloc);
        } else if (d->ref != 1) {
            x.p = QVectorData::malloc(sizeof(Data), alloc, sizeof(T), p);
        } else {
            if (QTypeInfo<T>::isComplex) {
                // call the destructor on all objects that need to be
                // destroyed when shrinking
                if (size < d->size) {
                    j = d->array + size;
                    i = d->array + d->size;
                    while (i-- != j)
                        i->~T();
                    i = d->array + size;
                }
            }
            x.p = p =
                  static_cast<QVectorData *>(qRealloc(p, sizeof(Data) + (alloc - 1) * sizeof(T)));
        }
        x.d->ref.init(1);
        x.d->sharable = true;
    }
    if (QTypeInfo<T>::isComplex) {
        if (size < d->size) {
            j = d->array + size;
            i = x.d->array + size;
        } else {
            // construct all new objects when growing
            i = x.d->array + size;
            j = x.d->array + d->size;
            while (i != j)
                new (--i) T;
            j = d->array + d->size;
        }
        if (i != j) {
            // copy objects from the old array into the new array
            b = x.d->array;
            while (i != b)
                new (--i) T(*--j);
        }
    } else if (size > d->size) {
        // initialize newly allocated memory to 0
        qMemSet(x.d->array + d->size, 0, (size - d->size) * sizeof(T));
    }
    x.d->size = size;
    x.d->alloc = alloc;
    if (d != x.d) {
        x.d = qAtomicSetPtr(&d, x.d);
        if (!--x.d->ref)
            free(x.d);
    }
}

template<typename T>
Q_OUTOFLINE_TEMPLATE T QVector<T>::value(int i) const
{
    if (i < 0 || i >= p->size) {
        T t;
        qInit(t);
        return t;
    }
    return d->array[i];
}
template<typename T>
Q_OUTOFLINE_TEMPLATE T QVector<T>::value(int i, const T &defaultValue) const
{
    return ((i < 0 || i >= p->size) ? defaultValue : d->array[i]);
}

template <typename T>
void QVector<T>::append(const T &t)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
        realloc(d->size, QVectorData::grow(sizeof(Data), d->size + 1, sizeof(T),
                                           QTypeInfo<T>::isStatic));
    if (QTypeInfo<T>::isComplex)
        new (d->array + d->size++) T(t);
    else
        d->array[d->size++] = t;
}


template <typename T>
Q_TYPENAME QVector<T>::iterator QVector<T>::insert(iterator before, size_type n, const T& t)
{
    int p = before - d->array;
    if (n != 0) {
        if (d->ref != 1 || d->size + n > d->alloc)
            realloc(d->size, QVectorData::grow(sizeof(Data), d->size + n, sizeof(T),
                                               QTypeInfo<T>::isStatic));
        if (QTypeInfo<T>::isStatic) {
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
            while (i != b)
                new (--i) T(t);
        }
    }
    d->size += n;
    return d->array+p;
}

template <typename T>
Q_TYPENAME QVector<T>::iterator QVector<T>::erase(iterator begin, iterator end)
{
    int f = begin - d->array;
    int l = end - d->array;
    int n = l - f;
    detach();
    if (QTypeInfo<T>::isComplex) {
        qCopy(d->array+l, d->array+d->size, d->array+f);
        T *i = d->array+d->size;
        T* b = d->array+d->size-n;
        while (i != b) {
            --i;
            i->~T();
        }
    } else {
        memmove(d->array + f, d->array + l, (d->size-l)*sizeof(T));
    }
    d->size -= n;
    return d->array + f;
}

template <typename T>
bool QVector<T>::operator==(const QVector<T> &v) const
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
QVector<T> &QVector<T>::operator+=(const QVector &l)
{
    int newSize = d->size + l.d->size;
    realloc(d->size, newSize);

    T *w = d->array + newSize;
    T *i = l.d->array + l.d->size;
    T *b = l.d->array;
    while (i != b) {
        if (QTypeInfo<T>::isComplex)
            new (--w) T(*--i);
        else
            *--w = *--i;
    }
    d->size = newSize;
    return *this;
}

template <typename T>
int QVector<T>::indexOf(const T &t, int from) const
{
    if (from < 0)
        from = qMax(from + d->size, 0);
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

template<typename T>
Q_OUTOFLINE_TEMPLATE QVector<T> QVector<T>::mid(int pos, int length) const
{
    if (length < 0)
        length = size() - pos;
    if (pos == 0 && length == size())
        return *this;
    QVector<T> copy;
    if (pos + length > size())
        length = size() - pos;
    for (int i = pos; i < pos + length; ++i)
        copy += at(i);
    return copy;
}


Q_DECLARE_ITERATOR(QVector)

#endif // QVECTOR_H

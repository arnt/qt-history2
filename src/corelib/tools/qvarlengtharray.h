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

#ifndef QVARLENGTHARRAY_H
#define QVARLENGTHARRAY_H

#include <QtCore/qglobal.h>
#include <new>

QT_MODULE(Core)

template<class T, int Prealloc = 256>
class QVarLengthArray
{
public:
    inline explicit QVarLengthArray(int size = 0);

    inline QVarLengthArray(const QVarLengthArray<T, Prealloc> &other)
        : a(Prealloc), s(0), ptr(reinterpret_cast<T *>(array))
    {
        append(other.constData(), other.size());
    }

    inline ~QVarLengthArray() {
        if (QTypeInfo<T>::isComplex) {
            T *i = ptr + s;
            while (i-- != ptr)
                i->~T();
        }
        if (ptr != reinterpret_cast<T *>(array))
            qFree(ptr);
    }
    inline QVarLengthArray<T, Prealloc> &operator=(const QVarLengthArray<T, Prealloc> &other)
    {
        if (this != &other) {
            clear();
            append(other.constData(), other.size());
        }
        return *this;
    }

    inline int size() const { return s; }
    inline int count() const { return s; }
    inline bool isEmpty() const { return (s == 0); }
    inline void resize(int size);
    inline void clear() { resize(0); }

    inline int capacity() const { return a; }
    inline void reserve(int size);

    inline T &operator[](int idx) {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }
    inline const T &operator[](int idx) const {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }

    inline void append(const T &t) {
        const int idx = s;
        resize(idx + 1);
        ptr[idx] = t;
    }
    void append(const T *buf, int size);

    inline T *data() { return ptr; }
    inline const T *data() const { return ptr; }
    inline const T * constData() const { return ptr; }

private:
    void realloc(int size, int alloc);

    int a;
    int s;
    T *ptr;
    qint64 array[((Prealloc * sizeof(T)) / sizeof(qint64)) + 1];
};

template <class T, int Prealloc>
Q_INLINE_TEMPLATE QVarLengthArray<T, Prealloc>::QVarLengthArray(int asize)
    : s(asize) {
    if (s > Prealloc) {
        ptr = reinterpret_cast<T *>(qMalloc(s * sizeof(T)));
        a = s;
    } else {
        ptr = reinterpret_cast<T *>(array);
        a = Prealloc;
    }
    if (QTypeInfo<T>::isComplex) {
        T *i = ptr + s;
        while (i != ptr)
            new (--i) T;
    }
}

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void QVarLengthArray<T, Prealloc>::resize(int asize)
{ realloc(asize, qMax(asize, a)); }

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void QVarLengthArray<T, Prealloc>::reserve(int asize)
{ if (asize > a) realloc(s, asize); }

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE void QVarLengthArray<T, Prealloc>::append(const T *abuf, int asize)
{
    Q_ASSERT(abuf);
    if (asize <= 0)
        return;

    const int idx = s;
    resize(idx + asize);

    if (QTypeInfo<T>::isComplex) {
        T *i = ptr + idx;
        T *j = i + asize;
        while (i < j)
            new (i++) T(*abuf++);
    } else {
        qMemCopy(&ptr[idx], abuf, asize * sizeof(T));
    }
}

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE void QVarLengthArray<T, Prealloc>::realloc(int asize, int aalloc)
{
    Q_ASSERT(aalloc >= asize);
    T *oldPtr = ptr;
    int osize = s;
    s = asize;

    if (aalloc != a) {
        ptr = reinterpret_cast<T *>(qMalloc(aalloc * sizeof(T)));
        a = aalloc;

        if (QTypeInfo<T>::isStatic) {
            T *i = ptr + osize;
            T *j = oldPtr + osize;
            while (i != ptr) {
                new (--i) T(*--j);
                j->~T();
            }
        } else {
            qMemCopy(ptr, oldPtr, osize * sizeof(T));
        }
    }

    if (QTypeInfo<T>::isComplex) {
        if (asize < osize) {
            T *i = oldPtr + osize;
            T *j = oldPtr + asize;
            while (i-- != j)
                i->~T();
        } else {
            T *i = ptr + asize;
            T *j = ptr + osize;
            while (i != j)
                new (--i) T;
        }
    }

    if (oldPtr != reinterpret_cast<T *>(array) && oldPtr != ptr)
        qFree(oldPtr);
}

#endif // QVARLENGTHARRAY_H

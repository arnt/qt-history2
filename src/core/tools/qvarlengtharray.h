#ifndef QSTACKARRAY_H
#define QSTACKARRAY_H

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

template<class T, int Prealloc = 256>
class QVarLengthArray
{
public:
    inline QVarLengthArray(int size = 0)
        : s(size) {
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
    inline ~QVarLengthArray() {
        if (QTypeInfo<T>::isComplex) {
            T *i = ptr + s;
            while (i-- != ptr)
                i->~T();
        }
        if (ptr != reinterpret_cast<T *>(array))
            qFree(ptr);
    }

    inline int size() const { return s; }
    inline int count() const { return s; }
    inline bool isEmpty() const { return (s == 0); }
    inline void resize(int size) { realloc(size, qMax(size, a)); }

    inline int capacity() const { return a; }
    inline void reserve(int size) { if (size > a) realloc(s, size); }

    inline T &operator[](int idx) {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }
    inline const T &operator[](int idx) const {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }

    inline operator T *() { return ptr; }
    inline operator const T *() const { return ptr; }

    inline T *data() { return ptr; }
    inline const T *data() const { return ptr; }
    inline const T * constData() const { return ptr; }

private:
#if defined(Q_DISABLE_COPY)
    QVarLengthArray(const QVarLengthArray &);
    QVarLengthArray &operator=(const QVarLengthArray &);
#endif
    void *operator new(size_t sz);

    void realloc(int size, int alloc);
    int a;
    int s;
    unsigned char array[Prealloc * sizeof(T)];
    T *ptr;
};

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE void QVarLengthArray<T, Prealloc>::realloc(int size, int alloc)
{
    Q_ASSERT(alloc >= size);
    T *oldPtr = ptr;
    int osize = s;
    s = size;

    if (alloc != a) {
        ptr = (T *)qMalloc(alloc * sizeof(T));
        a = alloc;

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
        if (size < osize) {
            T *i = oldPtr + osize;
            T *j = oldPtr + size;
            while (i-- != j)
                i->~T();
        } else {
            T *i = ptr + size;
            T *j = ptr + osize;
            while (i != j)
                new (--i) T;
        }
    }

    if (oldPtr != reinterpret_cast<T *>(array) && oldPtr != ptr)
        qFree(oldPtr);
}

#endif

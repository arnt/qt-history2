#ifndef QSTACKARRAY_H
#define QSTACKARRAY_H

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

template<class T, int prealloc = 256>
class QVarLengthArray
{
public:
    inline QVarLengthArray(int size = 0)
	: a(prealloc), s(size) {
	ptr = reinterpret_cast<T*>(array);
	if (s > prealloc)
	    ptr = reinterpret_cast<T *>(qMalloc(s * sizeof(T)));
	if (QTypeInfo<T>::isComplex) {
	    T* i = ptr + s;
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
	if (ptr != reinterpret_cast<T*>(array))
	    qFree(ptr);
    }

    inline int size() const { return s; }
    inline bool isEmpty() const { return (s == 0); }
    inline void resize(int size) {
	realloc(size, qMax(size,a));
    }

    void reserve(int size) {
	if (size > a) realloc(s, size);
    }

    inline int capacity() const { return a; }


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
    // disallow construction on the heap and copying
    void *operator new(size_t sz);
    QVarLengthArray(const QVarLengthArray &);
    QVarLengthArray &operator =(const QVarLengthArray &);
    void realloc(int size, int alloc);
    int a;
    int s;
    unsigned char array[prealloc*sizeof(T)];
    T *ptr;
};

template <class T, int prealloc>
Q_OUTOFLINE_TEMPLATE void QVarLengthArray<T, prealloc>::realloc(int size, int alloc)
{
    T *oldPtr = ptr;
    if (alloc > a) {
	ptr = (T *)qMalloc (alloc*sizeof(T));
	a = alloc;
    }

    int osize = s;
    s = size;
    if (ptr != oldPtr) {
	if (QTypeInfo<T>::isStatic) {
	    T *i = ptr + osize;
	    T *j = oldPtr + osize;
	    while (i != ptr) {
		new (--i) T(*--j);
		j->~T();
	    }
	} else {
	    qMemCopy(ptr, oldPtr, osize*sizeof(T));
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

    if (oldPtr != (T*)array && oldPtr != ptr)
	qFree(oldPtr);
}

#endif

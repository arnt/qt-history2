#ifndef QSTACKARRAY_H
#define QSTACKARRAY_H

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

template<class T, int prealloc = 256>
class QStackArray
{
public:
    inline QStackArray(int size) {
	s = size;
	a = prealloc;
	ptr = (T*)array;
	if (s > prealloc)
	    ptr = (T *) malloc(s * sizeof(T));
	if (QTypeInfo<T>::isComplex) {
	    T* b = ptr;
	    T* i = ptr + s;
	    while (i != b)
		new (--i) T;
	}
    }
    inline ~QStackArray() {
	if (QTypeInfo<T>::isComplex) {
	    T *i = ptr + s;
	    while (i-- != ptr)
		i->~T();
	}
	if (ptr != (T*)array)
	    qFree(ptr);
    }

    inline int size() const { return s; }
    inline bool isEmpty() const { return s == 0; }
    inline void resize(int size) {
	if (size > s) realloc(size, qMax(size,a));
    }

    void reserve(int size) {
	if (size > a) realloc(s, size);
    }

    inline int capacity() const { return a; }


    T &operator[](int idx) {
	Q_ASSERT(idx >= 0 && idx < s);
	return ptr[idx];
    }

    operator T *() { return ptr; }


private:
    void *operator new(size_t sz);
    void realloc(int size, int alloc);
    int a;
    int s;
    unsigned char array[prealloc*sizeof(T)];
    T *ptr;
};

template <class T, int prealloc>
void QStackArray<T, prealloc>::realloc(int size, int alloc)
{
    T *oldPtr = ptr;
    if (alloc > a) {
	ptr = (T *)qMalloc (alloc*sizeof(T));
	a = alloc;
    }

    T *j, *i;

    int osize = s;
    s = size;
    if (QTypeInfo<T>::isComplex) {
	if (size < osize) {
	    i = oldPtr + osize;
	    j = oldPtr + size;
	    while (i-- != j)
		i->~T();
	    i = ptr + size;
	} else {
	    i = ptr + size;
	    j = ptr + osize;
	    while (i != j)
		new (--i) T;
	    j = oldPtr + osize;
	}
	if (i != j)
	    while (i != ptr)
		new (--i) T(*--j);
    } else {
	if ( ptr != oldPtr )
	    qMemCopy(ptr, oldPtr, osize*sizeof(T));
    }
    if (ptr != oldPtr) {
	if (QTypeInfo<T>::isComplex) {
	    i = oldPtr + osize;
	    while (i-- != oldPtr)
		i->~T();
	}
	if (oldPtr != (T*)array)
	    qFree(oldPtr);
    }
}


#endif

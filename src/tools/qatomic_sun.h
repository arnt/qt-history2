#ifndef QATOMIC_SOLARIS_H
#define QATOMIC_SOLARIS_H

#if defined(__sun) || defined(sun)

#include <sys/atomic.h>

template <typename T>
inline T *qAtomicSetPtr( T ** volatile ptr, T *value )
{
    T *tmp;
  spin:
    tmp = *ptr;
    if(((T *)casptr((void *)ptr, (void *) tmp, (void *) value)) != tmp)
	goto spin;
    return tmp;
}

struct QAtomic {
    int atomic;

    inline bool operator++()
    {
	
	return (atomic_add_32_nv((uint32_t *)&atomic, 1) != 0);
    }

    inline bool operator--()
    {
	return (atomic_add_32_nv((uint32_t *)&atomic, -1) != 0);
    }

    inline bool operator==(int x) const
    {
	volatile const int *ptr = &atomic;
	return *ptr == x;
    }

    inline bool operator!=(int x) const
    {
	volatile const int *ptr = &atomic;
	return *ptr != x;
    }

    inline void operator=(int x)
    {
	volatile int *ptr = &atomic;
	*ptr = x;
    }

};

#define Q_ATOMIC_INIT(a) { (a) }

#else
#  error "Unknown compiler/CPU combination."
#endif

#endif // QATOMIC_SOLARIS_H

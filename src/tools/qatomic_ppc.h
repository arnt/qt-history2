#ifndef QATOMIC_PPC_H
#ifndef QT_H
#endif // QT_H
#define QATOMIC_PPC_H

#if defined(__GNUC__) && defined(__ppc__)

template <typename T>
inline T *qAtomicSetPtr( T ** volatile ptr, T *value )
{
    T *tmp;
    __asm __volatile ("1:\tlwarx %0, 0, %2\n\t"         /* load old value */
                      "stwcx. %3, 0, %2\n\t"            /* attempt to store */
                      "bne- 1b\n\t"                     /* spin if failed */
                      : "=&r" (tmp), "+m" (*ptr)
                      : "r" (ptr), "r" (value)
                      : "cc", "memory");
    return tmp;
}

struct QAtomic {
    int atomic;

    inline bool operator++()
    {
        volatile int *ptr = &atomic;
	u_int32_t temp, one = 1;
	__asm __volatile ("1:\tlwarx %0, 0, %2\n\t"	/* load old value */
                          "add %0, %3, %0\n\t"		/* calculate new value */
                          "stwcx. %0, 0, %2\n\t"      	/* attempt to store */
                          "bne- 1b\n\t"			/* spin if failed */
                          : "=&r" (temp), "+m" (*ptr)
                          : "r" (ptr), "r" (one)
                          : "cc", "memory");
        return temp != 0;
    }

    inline bool operator--()
    {
        volatile int *ptr = &atomic;
	u_int32_t temp, one = 1;
	__asm __volatile ("1:\tlwarx %0, 0, %2\n\t"	/* load old value */
                          "subf %0, %3, %0\n\t"		/* calculate new value */
                          "stwcx. %0, 0, %2\n\t"      	/* attempt to store */
                          "bne- 1b\n\t"			/* spin if failed */
                          : "=&r" (temp), "+m" (*ptr)
                          : "r" (ptr), "r" (one)
                          : "cc", "memory");
        return temp != 0;
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

#endif // QATOMIC_PPC_H

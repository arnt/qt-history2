#ifndef QATOMIC_X86_H
#define QATOMIC_X86_H

#if defined(__GNUC__) && defined(__i386__)

template <typename T>
inline T *qAtomicSetPtr( T ** volatile ptr, T *value )
{
    asm ("xchgl %0,%1"
	 :"=r" (value)
	 :"m" (*ptr), "0" (value)
	 :"memory");
    return value;
}

struct QAtomic {
    int atomic;

    inline bool operator++()
    {
	volatile int *ptr = &atomic;
	register unsigned char result;
	asm volatile ("lock; incl %0; sete %1"
		      : "=m" (*ptr), "=qm" (result)
		      : "m" (*ptr)
		      : "memory");
	return (result == 0);
    }

    inline bool operator--()
    {
	volatile int *ptr = &atomic;
	register unsigned char result;
	asm volatile ("lock; decl %0; sete %1"
		      : "=m" (*ptr), "=qm" (result)
		      : "m" (*ptr)
		      : "memory");
	return (result == 0);
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

    inline bool compareAndSet(int compare, int value)
    {
        volatile int *ptr = &atomic;
	int prev;
	asm ("lock cmpxchgl %1, %2"
             : "=a" (prev)
             : "q" (value), "m" (*ptr), "0" (compare)
             : "memory");
	return (prev == compare);
    }
};

#define Q_ATOMIC_INIT(a) { (a) }

#else
#  error "Unknown compiler/CPU combination."
#endif

#endif // QATOMIC_X86_H

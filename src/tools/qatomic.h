#ifndef QATOMIC_H
#define QATOMIC_H

#ifndef QT_H
#endif // QT_H

#include <arch/qatomic.h>

template <typename T>
inline T qAtomicSetPtr(volatile T *ptr, T newval)
{
    void *expected = static_cast<void *>(*ptr), *result;
    for (;;) {
        result = q_cas_ptr(reinterpret_cast<void * volatile *>(ptr), expected,
			   static_cast<void *>(newval));
        if (result == expected) break;

        expected = result;
    }
    return static_cast<T>(result);
}

struct QAtomic {
    int atomic;

    inline bool operator++()
    {
	int * volatile const p = &atomic;
        register int expected = *p, newval, result;
	for (;;) {
            newval = expected + 1;
 	    result = q_cas_32(p, expected, newval);
	    if (result == expected) break;

	    expected = result;
	}
 	return result != 0;
    }

    inline bool operator--()
    {
	int * volatile const p = &atomic;
	register int expected = *p, newval, result;
	for (;;) {
            newval = expected - 1;
 	    result = q_cas_32(p, expected, newval);
	    if (result == expected) break;

	    expected = result;
	}
 	return result != 1;
    }

    inline bool operator==(int x) const
    {
	const int * volatile const ptr = &atomic;
	return *ptr == x;
    }

    inline bool operator!=(int x) const
    {
	const int * volatile const ptr = &atomic;
	return *ptr != x;
    }

    inline void operator=(int x)
    {
	int * volatile const p = &atomic;
        register int expected = *p, result;
        for (;;) {
            result = q_cas_32(p, expected, x);
            if (result == expected) break;

            expected = result;
        }
    }
};

#define Q_ATOMIC_INIT(a) { (a) }

#endif

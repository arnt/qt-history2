/****************************************************************************
**
** Definition of QAtomic.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
	volatile int * const p = &atomic;
        register int expected = *p, newval, result;
	for (;;) {
            newval = expected + 1;
 	    result = q_cas_32((int *)p, expected, newval);
	    if (result == expected) break;

	    expected = result;
	}
 	return result != 0;
    }

    inline bool operator--()
    {
	volatile int * const p = &atomic;
	register int expected = *p, newval, result;
	for (;;) {
            newval = expected - 1;
 	    result = q_cas_32((int *)p, expected, newval);
	    if (result == expected) break;

	    expected = result;
	}
 	return result != 1;
    }

    inline bool operator==(int x) const
    {
	const volatile int * const ptr = &atomic;
	return *ptr == x;
    }

    inline bool operator!=(int x) const
    {
	const volatile int * const ptr = &atomic;
	return *ptr != x;
    }

    inline void operator=(int x)
    {
	volatile int * const ptr = &atomic;
	*ptr = x;
    }
};

#define Q_ATOMIC_INIT(a) { (a) }

#endif

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

#ifndef Q_HAVE_ATOMIC_INCDEC
inline bool q_atomic_increment(volatile int * const p)
{
    register int expected = *p, newval, result;
    for (;;) {
	newval = expected + 1;
	result = q_cas_32((int *)p, expected, newval);
	if (result == expected) break;

	expected = result;
    }
    return result != 0;
}

inline bool q_atomic_decrement(volatile int * const p)
{
    register int expected = *p, newval, result;
    for (;;) {
	newval = expected - 1;
	result = q_cas_32((int *)p, expected, newval);
	if (result == expected) break;

	expected = result;
    }
    return result != 1;
}
#endif

#ifndef Q_HAVE_ATOMIC_SETPOINTER
inline void *q_atomic_set_pointer(void * volatile *ptr, void *newval)
{
    void *expected = static_cast<void *>(*ptr), *result;
    for (;;) {
        result = q_cas_ptr(ptr, expected, newval);
        if (result == expected) break;

        expected = result;
    }
    return result;
}
#endif

template <typename T>
inline T qAtomicSetPtr(volatile T *ptr, T newval)
{ return static_cast<T>(q_atomic_set_pointer(reinterpret_cast<void * volatile *>(ptr), newval)); }

struct QAtomic {
    int atomic;

    inline bool operator++()
    { return q_atomic_increment(&atomic); }

    inline bool operator--()
    { return q_atomic_decrement(&atomic); }

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

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

inline int q_atomic_increment(volatile int * const ptr)
{
    register int expected;
    for (;;) {
	expected = *ptr;
	if (q_atomic_test_and_set_int(ptr, expected, expected + 1)) break;
    }
    return expected != -1;
}

inline bool q_atomic_decrement(volatile int * const ptr)
{
    register int expected;
    for (;;) {
	expected = *ptr;
	if (q_atomic_test_and_set_int(ptr, expected, expected - 1)) break;
    }
    return expected != 1;
}

#endif // Q_HAVE_ATOMIC_INCDEC

#ifndef Q_HAVE_ATOMIC_SET

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    register int expected;
    for (;;) {
	expected = *ptr;
	if (q_atomic_test_and_set_int(ptr, expected, newval)) break;
    }
    return expected;
}

inline void *q_atomic_set_ptr(void * volatile *ptr, void *newval)
{
    register void *expected;
    for (;;) {
	expected = static_cast<void *>(*ptr);
        if (q_atomic_test_and_set_ptr(ptr, expected, newval)) break;
    }
    return expected;
}

#endif // Q_HAVE_ATOMIC_SET

template <typename T>
inline T qAtomicSetPtr(volatile T *ptr, T newval)
{ return static_cast<T>(q_atomic_set_ptr(reinterpret_cast<void * volatile *>(ptr), newval)); }

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
    { q_atomic_set_int(&atomic, x); }
};

#define Q_ATOMIC_INIT(a) { (a) }

#endif

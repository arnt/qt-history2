/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QATOMIC_H
#define QATOMIC_H


extern "C" {
    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
    int q_atomic_test_and_set_ptr(volatile void *, void *, void *);
    int q_atomic_increment(volatile int *);
    int q_atomic_decrement(volatile int *);
    int q_atomic_set_int(volatile int *, int);
    void *q_atomic_set_ptr(volatile void *, void *);
} // extern "C"

#include <arch/qatomic.h>

#ifndef Q_HAVE_ATOMIC_INCDEC

extern "C" {

    inline int q_atomic_increment(volatile int * const ptr)
    {
        register int expected;
        for (;;) {
            expected = *ptr;
            if (q_atomic_test_and_set_int(ptr, expected, expected + 1)) break;
        }
        return expected != -1;
    }

    inline int q_atomic_decrement(volatile int * const ptr)
    {
        register int expected;
        for (;;) {
            expected = *ptr;
            if (q_atomic_test_and_set_int(ptr, expected, expected - 1)) break;
        }
        return expected != 1;
    }

} // extern "C"

#endif // Q_HAVE_ATOMIC_INCDEC

#ifndef Q_HAVE_ATOMIC_SET

extern "C" {

    inline int q_atomic_set_int(volatile int *ptr, int newval)
    {
        register int expected;
        for (;;) {
            expected = *ptr;
            if (q_atomic_test_and_set_int(ptr, expected, newval)) break;
        }
        return expected;
    }

    inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        register void *expected;
        for (;;) {
            expected = *reinterpret_cast<void * volatile *>(ptr);
            if (q_atomic_test_and_set_ptr(ptr, expected, newval)) break;
        }
        return expected;
    }

} // extern "C"

#endif // Q_HAVE_ATOMIC_SET

template <typename T>
inline T qAtomicSetPtr(volatile T *ptr, T newval)
{ return static_cast<T>(q_atomic_set_ptr(ptr, newval)); }

struct QAtomic {
    int atomic;

    inline bool operator++()
    { return q_atomic_increment(&atomic) != 0; }

    inline bool operator--()
    { return q_atomic_decrement(&atomic) != 0; }

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
    { atomic = x; }
};

/*! \internal
  This is a helper for the assignment operators of implicitely shared classes.
  Your assignment operator should look like this:
  { qAtomicAssign(d, other.d); return *this; }
 */
template <typename T>
inline void qAtomicAssign(T *&d, T *x)
{
    ++x->ref;
    x = static_cast<T*>(q_atomic_set_ptr(&d, x));
    if (!--x->ref)
        delete x;
}

/*! \internal
  This is a helper for the detach function. Your d class needs a copy constructor
  which copies the members and sets the refcount to 1. After that, your detach
  function should look like this:
  { qAtomicDetach(d); }
 */
template <typename T>
inline void qAtomicDetach(T *&d)
{
    if (d->ref == 1)
        return;
    T *x = new T(*d);
    x = static_cast<T*>(q_atomic_set_ptr(&d, x));
    if (!--x->ref)
        delete x;
}


#define Q_ATOMIC_INIT(a) { (a) }

#endif

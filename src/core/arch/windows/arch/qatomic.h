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

#ifndef WINDOWS_QATOMIC_H
#define WINDOWS_QATOMIC_H

// use compiler intrinsics for all atomic functions
extern "C" {
    int _InterlockedCompareExchange(volatile int *, int, int);
    int _InterlockedIncrement(volatile int *);
    int _InterlockedDecrement(volatile int *);
    int _InterlockedExchange(volatile int *, int);
}
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedExchange)

#ifndef _M_IX86
extern "C" {
    void *_InterlockedCompareExchangePointer(void * volatile *, void *, void *);
    void *_InterlockedExchangePointer(void * volatile *, void *, void *);
}
#  pragma intrinsic (_InterlockedCompareExchangePointer)
#  pragma intrinsic (_InterlockedExchangePointer)
#else
#  define _InterlockedCompareExchangePointer(a,b,c) \
        (void *)_InterlockedCompareExchange((volatile int *)(a), (int)(b), (int)(c))
#  define _InterlockedExchangePointer(a, b) \
        (void *)_InterlockedExchange((volatile int *)(a), (int)(b))
#endif

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{ return _InterlockedCompareExchange(ptr, newval, expected) == expected; }

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{ return _InterlockedCompareExchangePointer(ptr, newval, expected) == expected; }

inline int q_atomic_increment(volatile int *ptr)
{ return _InterlockedIncrement(ptr); }

inline int q_atomic_decrement(volatile int *ptr)
{ return _InterlockedDecrement(ptr); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{ return _InterlockedExchange(ptr, newval); }

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{ return _InterlockedExchangePointer(ptr, newval); }

#define Q_HAVE_ATOMIC_INCDEC
#define Q_HAVE_ATOMIC_SET

#endif // WINDOWS_QATOMIC_H

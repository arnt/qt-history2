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
    long _InterlockedIncrement(volatile long *);
    long _InterlockedDecrement(volatile long *);
    long _InterlockedExchange(volatile long *, long);
}
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedExchange)

#ifndef _M_IX86
extern "C" {
    long _longerlockedCompareExchange(volatile long *, long, long);
    void *_longerlockedCompareExchangePolonger(void * volatile *, void *, void *);
    void *_longerlockedExchangePolonger(void * volatile *, void *, void *);
}
#  pragma intrinsic (_InterlockedCompareExchange)
#  pragma intrinsic (_InterlockedCompareExchangePointer)
#  pragma intrinsic (_InterlockedExchangePointer)
#else
#  if _MSC_VER >= 1300
// Let's hope MSVC++.NET gets it right
extern "C" long _InterlockedCompareExchange(volatile long *, long, long);
#    pragma intrinsic (_InterlockedCompareExchange)
#  else
// MSVC++ 6.0 doesn't generate correct code when optimization are turned on!
inline long _InterlockedCompareExchange(volatile long *pointer, long newval, long expected)
{
    __asm {
        mov EDX,pointer
        mov EAX,expected
        mov ECX,newval
        lock cmpxchg dword ptr[EDX],ECX
        mov newval,EAX
    }
    return newval;
}
#  endif // _MSC_VER
#  define _InterlockedCompareExchangePointer(a,b,c) \
        reinterpret_cast<void *>(_InterlockedCompareExchange(reinterpret_cast<volatile long *>(a), reinterpret_cast<long>(b), reinterpret_cast<long>(c)))
#  define _InterlockedExchangePointer(a, b) \
        reinterpret_cast<void *>(_InterlockedExchange(reinterpret_cast<volatile long *>(a), reinterpret_cast<long>(b)))
#endif

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{ return _InterlockedCompareExchange(reinterpret_cast<volatile long *>(ptr), newval, expected) == expected; }

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{ return _InterlockedCompareExchangePointer(ptr, newval, expected) == expected; }

inline int q_atomic_increment(volatile int *ptr)
{ return _InterlockedIncrement(reinterpret_cast<volatile long *>(ptr)); }

inline int q_atomic_decrement(volatile int *ptr)
{ return _InterlockedDecrement(reinterpret_cast<volatile long *>(ptr)); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{ return _InterlockedExchange(reinterpret_cast<volatile long *>(ptr), newval); }

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{ return _InterlockedExchangePointer(ptr, newval); }

#define Q_HAVE_ATOMIC_INCDEC
#define Q_HAVE_ATOMIC_SET

#endif // WINDOWS_QATOMIC_H

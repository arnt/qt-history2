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

typedef long LONG;
typedef void *PVOID;

// use compiler intrinsics for all atomic functions
extern "C" {
    LONG  __cdecl _InterlockedCompareExchange(LONG volatile *Dest, LONG Exchange, LONG Comp);
    LONG  __cdecl _InterlockedIncrement(LONG volatile *Addend);
    LONG  __cdecl _InterlockedDecrement(LONG volatile *Addend);
    LONG  __cdecl _InterlockedExchange(LONG volatile *Target, LONG Value);
}
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedExchange)

#ifndef _M_IX86
extern "C" {
    PVOID __cdecl _InterlockedCompareExchangePointer(PVOID volatile *Dest, PVOID Exchange, PVOID Comp);
    PVOID __cdecl _InterlockedExchangePointer(PVOID volatile *Target, PVOID Value);
}
#  pragma intrinsic (_InterlockedCompareExchangePointer)
#  pragma intrinsic (_InterlockedExchangePointer)
#else
#  define _InterlockedCompareExchangePointer(a,b,c) \
        (PVOID)_InterlockedCompareExchange((LONG volatile *)(a), (LONG)(b), (LONG)(c))
#  define _InterlockedExchangePointer(a, b) \
        (PVOID)_InterlockedExchange((LONG volatile *)(a), (LONG)(b))
#endif

extern "C" {

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
        
        
        
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        LONG lexpected = expected;
        LONG lnewval = newval;
        return _InterlockedCompareExchange(lptr, lnewval, lexpected) == lexpected;
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
        PVOID volatile *pptr = reinterpret_cast<PVOID volatile *>(ptr);
        return _InterlockedCompareExchangePointer(pptr, newval, expected) == expected;
    }

    inline int q_atomic_increment(volatile int *ptr)
    {
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        return _InterlockedIncrement(lptr);
    }

    inline int q_atomic_decrement(volatile int *ptr)
    {
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        return _InterlockedDecrement(lptr);
    }

    inline int q_atomic_set_int(volatile int *ptr, int newval)
    {
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        LONG lnewval = newval;
        return _InterlockedExchange(lptr, lnewval);
    }

    inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        PVOID volatile *pptr = reinterpret_cast<PVOID volatile *>(ptr);
        return _InterlockedExchangePointer(pptr, newval);
    }

}

#define Q_HAVE_ATOMIC_INCDEC
#define Q_HAVE_ATOMIC_SET

#endif // WINDOWS_QATOMIC_H

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

// use compiler intrinsics for all atomic functions when not in debug mode
#if !defined(QT_NO_ATOMIC_INTRINSICS) && defined(QT_NO_DEBUG)
typedef long LONG;
typedef void *PVOID;
extern "C" {
    __declspec(dllimport) LONG __stdcall _InterlockedCompareExchange(LONG volatile *Dest, LONG Exchange, LONG Comp);
    __declspec(dllimport) LONG __stdcall _InterlockedIncrement(LONG volatile *Addend);
    __declspec(dllimport) LONG __stdcall _InterlockedDecrement(LONG volatile *Addend);
    __declspec(dllimport) LONG __stdcall _InterlockedExchange(LONG volatile *Target, LONG Value);
#  ifndef _M_IX86
      __declspec(dllimport) PVOID __stdcall _InterlockedCompareExchangePointer(PVOID volatile *Dest, PVOID Exchange, PVOID Comp);
      __declspec(dllimport) PVOID __stdcall _InterlockedExchangePointer(PVOID volatile *Target, PVOID Value);
#  endif
}
#  pragma intrinsic (_InterlockedCompareExchange)
#  pragma intrinsic (_InterlockedIncrement)
#  pragma intrinsic (_InterlockedDecrement)
#  pragma intrinsic (_InterlockedExchange)
#  define InterlockedCompareExchange _InterlockedCompareExchange
#  define InterlockedIncrement _InterlockedIncrement
#  define InterlockedDecrement _InterlockedDecrement
#  define InterlockedExchange _InterlockedExchange
#  ifndef _M_IX86
#    pragma intrinsic (_InterlockedCompareExchangePointer)
#    pragma intrinsic (_InterlockedExchangePointer)
#    define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer
#    define InterlockedExchangePointer _InterlockedExchangePointer
#  endif
#endif

// These for NET2003 these are already defined for _M_IX86.
// Brad can you check to see if they work with 6.0? If so we
// can simply remove the if-block below, to clean up even more
#if 0 //def _M_IX86
#  define InterlockedCompareExchangePointer(a,b,c) \
        (PVOID)_InterlockedCompareExchange((LONG volatile *)(a), (LONG)(b), (LONG)(c))
#  define InterlockedExchangePointer(a, b) \
        (PVOID)_InterlockedExchange((LONG volatile *)(a), (LONG)(b))
#endif

extern "C" {

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        LONG lexpected = expected;
        LONG lnewval = newval;
        return InterlockedCompareExchange(lptr, lnewval, lexpected) == lexpected;
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
        PVOID volatile *pptr = reinterpret_cast<PVOID volatile *>(ptr);
        return InterlockedCompareExchangePointer(pptr, newval, expected) == expected;
    }

    inline int q_atomic_increment(volatile int *ptr)
    {
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        return InterlockedIncrement(lptr);
    }

    inline int q_atomic_decrement(volatile int *ptr)
    {
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        return InterlockedDecrement(lptr);
    }

    inline int q_atomic_set_int(volatile int *ptr, int newval)
    {
        volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
        LONG lnewval = newval;
        return InterlockedExchange(lptr, lnewval);
    }

    inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        PVOID volatile *pptr = reinterpret_cast<PVOID volatile *>(ptr);
        return InterlockedExchangePointer(pptr, newval);
    }

}

#define Q_HAVE_ATOMIC_INCDEC
#define Q_HAVE_ATOMIC_SET

#if !defined(QT_NO_ATOMIC_INTRINSICS) && defined(QT_NO_DEBUG)
#  undef InterlockedCompareExchange
#  undef InterlockedIncrement
#  undef InterlockedDecrement
#  undef InterlockedExchange
#  undef InterlockedCompareExchangePointer
#  undef InterlockedExchangePointer
#endif

#endif // WINDOWS_QATOMIC_H

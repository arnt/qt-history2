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

#ifndef IA64_QATOMIC_H
#define IA64_QATOMIC_H

#  include <qglobal.h>

#ifdef Q_OS_WIN

typedef long LONG;
typedef void *PVOID;

extern "C" {

LONG  __cdecl _InterlockedCompareExchange(LONG volatile *Dest, LONG Exchange, LONG Comp);
PVOID __cdecl _InterlockedCompareExchangePointer(PVOID volatile *Dest, PVOID Exchange, PVOID Comp);
LONG  __cdecl _InterlockedIncrement(LONG volatile *Addend);
LONG  __cdecl _InterlockedDecrement(LONG volatile *Addend);
LONG  __cdecl _InterlockedExchange(LONG volatile *Target, LONG Value);
PVOID __cdecl _InterlockedExchangePointer(PVOID volatile *Target, PVOID Value);

}

#pragma intrinsic (_InterlockedCompareExchange)
#define InterlockedCompareExchange _InterlockedCompareExchange

#pragma intrinsic (_InterlockedCompareExchangePointer)
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer

#pragma intrinsic (_InterlockedIncrement)
#define InterlockedIncrement _InterlockedIncrement

#pragma intrinsic (_InterlockedDecrement)
#define InterlockedDecrement _InterlockedDecrement

#pragma intrinsic (_InterlockedExchange)
#define InterlockedExchange _InterlockedExchange 

#pragma intrinsic (_InterlockedExchangePointer)
#define InterlockedExchangePointer _InterlockedExchangePointer

extern "C" {

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
	volatile LONG *lptr = reinterpret_cast<volatile LONG *>(ptr);
	LONG lexpected = expected;
	LONG lnewval = newval;
        return InterlockedCompareExchange(lptr, lexpected, lnewval) == lexpected;
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
	PVOID volatile *pptr = reinterpret_cast<PVOID volatile *>(ptr);
        return InterlockedCompareExchangePointer(pptr, expected, newval) == expected;
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

#else

extern "C" {

#if defined(Q_CC_GNU) && !defined(Q_CC_INTEL)

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
        int ret;
        asm volatile("mov ar.ccv=%2\n"
                     ";;\n"
                     "cmpxchg4.acq %0=%1,%3,ar.ccv\n"
                     : "=r" (ret), "+m" (*ptr)
                     : "r" (expected), "r" (newval)
                     : "memory");
        return ret == expected;
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval) {
        void *ret;
        asm volatile("mov ar.ccv=%2\n"
                     ";;\n"
                     "cmpxchg8.acq %0=%1,%3,ar.ccv\n"
                     : "=r" (ret), "+m" (*ptr)
                     : "r" (expected), "r" (newval)
                     : "memory");
        return ret == expected;
    }

#else

    Q_CORE_EXPORT
    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);

    Q_CORE_EXPORT
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);

#endif // Q_CC_GNU && !Q_CC_INTEL

} // extern "C"

#endif

#endif // IA64_QATOMIC_H

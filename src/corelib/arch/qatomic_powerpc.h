/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef POWERPC_QATOMIC_H
#define POWERPC_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#if defined(Q_CC_GNU)
#ifdef __64BIT__
#  define LPARX "ldarx"
#  define CMPP  "cmpd"
#  define STPCX "stdcx."
#else
#  define LPARX "lwarx"
#  define CMPP  "cmpw"
#  define STPCX "stwcx."
#endif

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0,0,%2\n"
                 "cmpw   %0,%3\n"
                 "bne-   $+20\n"
                 "stwcx. %4,0,%2\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret)
                 : "r" (ptr), "r" (expected), "r" (newval)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0,0,%3\n"
                 "cmpw   %0,%4\n"
                 "bne-   $+20\n"
                 "stwcx. %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 "eieio\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (*ptr)
                 : "r" (ptr), "r" (expected), "r" (newval)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    register int tmp;
    register int ret;
    asm volatile("eieio\n"
                 "lwarx  %0,0,%3\n"
                 "cmpw   %0,%4\n"
                 "bne-   $+20\n"
                 "stwcx. %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (*ptr)
                 : "r" (ptr), "r" (expected), "r" (newval)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    register void *tmp;
    register int ret;
    asm volatile(LPARX"  %0,0,%3\n"
                 CMPP"   %0,%4\n"
                 "bne-   $+20\n"
                 STPCX"  %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(ptr))
                 : "r" (ptr), "r" (expected), "r" (newval)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_increment(volatile int *ptr)
{
    register int ret;
    register int one = 1;
    asm volatile("lwarx  %0, 0, %2\n"
                 "add    %0, %3, %0\n"
                 "stwcx. %0, 0, %2\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=m" (*ptr)
                 : "r" (ptr), "r" (one)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_decrement(volatile int *ptr)
{
    register int ret;
    register int one = -1;
    asm volatile("lwarx  %0, 0, %2\n"
                 "add    %0, %3, %0\n"
                 "stwcx. %0, 0, %2\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=m" (*ptr)
                 : "r" (ptr), "r" (one)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    register int ret;
    asm volatile("lwarx  %0, 0, %2\n"
                 "stwcx. %2, 0, %2\n"
                 "bne-   $-8\n"
                 : "=&r" (ret), "=m" (*ptr)
                 : "r" (ptr), "r" (newval)
                 : "cc", "memory");
    return ret;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register void *ret;
    asm volatile(LPARX"  %0, 0, %2\n"
                 STPCX"  %2, 0, %2\n"
                 "bne-   $-8\n"
                 : "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(ptr))
                 : "r" (ptr), "r" (newval)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_fetch_and_add(volatile int *ptr, int value)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 "stwcx. %1, 0, %3\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (*ptr)
                 : "r" (ptr), "r" (value)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_fetch_and_add_acquire(volatile int *ptr, int value)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 "stwcx. %1, 0, %3\n"
                 "bne-   $-12\n"
                 "eieio\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (*ptr)
                 : "r" (ptr), "r" (value)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_fetch_and_add_release(volatile int *ptr, int value)
{
    register int tmp;
    register int ret;
    asm volatile("eieio\n"
                 "lwarx  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 "stwcx. %1, 0, %3\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (*ptr)
                 : "r" (ptr), "r" (value)
                 : "cc", "memory");
    return ret;
}

#undef LPARX
#undef CMPP
#undef STPCX

#else

extern "C" {
    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
    int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval);
    int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval);
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
    int q_atomic_increment(volatile int *);
    int q_atomic_decrement(volatile int *);
    int q_atomic_set_int(volatile int *, int);
    void *q_atomic_set_ptr(volatile void *, void *);
    int q_atomic_fetch_and_add(volatile int *ptr, int value);
    int q_atomic_fetch_and_add_acquire(volatile int *ptr, int value);
    int q_atomic_fetch_and_add_release(volatile int *ptr, int value);
} // extern "C"

#endif

QT_END_HEADER

#endif // POWERPC_QATOMIC_H

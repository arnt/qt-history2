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

#ifndef IA64_QATOMIC_H
#define IA64_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#if defined(Q_CC_INTEL)

// intrinsics provided by the Intel C++ Compiler
#include <ia64intrin.h>

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{ return static_cast<int>(_InterlockedCompareExchange(ptr, newval, expected)) == expected; }

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return static_cast<int>(_InterlockedCompareExchange_acq(reinterpret_cast<volatile uint *>(ptr),
                                                          newval, expected)) == expected; }

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return static_cast<int>(_InterlockedCompareExchange_rel(reinterpret_cast<volatile uint *>(ptr),
                                                          newval, expected)) == expected; }

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    return _InterlockedCompareExchangePointer(reinterpret_cast<void * volatile*>(ptr),
                                              newval, expected) == expected;
}

inline int q_atomic_increment(volatile int *ptr)
{ return _InterlockedIncrement(ptr); }

inline int q_atomic_decrement(volatile int *ptr)
{ return _InterlockedDecrement(ptr); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{ return _InterlockedExchange(ptr, newval); }

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    return _InterlockedExchangePointer(reinterpret_cast<void * volatile *>(ptr), newval);
}

#else // !Q_CC_INTEL

#  if defined(Q_CC_GNU)

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

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    int ret;
    asm volatile("mov ar.ccv=%2\n"
                 ";;\n"
                 "cmpxchg4.rel %0=%1,%3,ar.ccv\n"
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
                 : "=r" (ret), "+m" (*reinterpret_cast<volatile unsigned long *>(ptr))
                 : "r" (expected), "r" (newval)
                 : "memory");
    return ret == expected;
}

#elif defined Q_CC_HPACC

#include <ia64/sys/inline.h>

#define FENCE (_Asm_fence)(_UP_CALL_FENCE | _UP_SYS_FENCE | _DOWN_CALL_FENCE | _DOWN_SYS_FENCE)

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    _Asm_mov_to_ar((_Asm_app_reg)_AREG_CCV, (unsigned)expected, FENCE);
    int ret = _Asm_cmpxchg((_Asm_sz)_SZ_W, (_Asm_sem)_SEM_ACQ,
                           ptr, (unsigned)newval, (_Asm_ldhint)_LDHINT_NONE);
    return ret == expected;
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    _Asm_mov_to_ar((_Asm_app_reg)_AREG_CCV, (unsigned)expected, FENCE);
    int ret = _Asm_cmpxchg((_Asm_sz)_SZ_W, (_Asm_sem)_SEM_REL,
                           ptr, newval, (_Asm_ldhint)_LDHINT_NONE);
    return ret == expected;
}

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    _Asm_mov_to_ar((_Asm_app_reg)_AREG_CCV, (unsigned)expected, FENCE);
    int ret = _Asm_cmpxchg((_Asm_sz)_SZ_W, (_Asm_sem)_SEM_ACQ,
                           ptr, (unsigned)newval, (_Asm_ldhint)_LDHINT_NONE);
    return ret == expected;
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    _Asm_mov_to_ar((_Asm_app_reg)_AREG_CCV, (quint64)expected, FENCE);
    void *ret = (void *)_Asm_cmpxchg((_Asm_sz)_SZ_D, (_Asm_sem)_SEM_ACQ,
                                     ptr, (quint64)newval, (_Asm_ldhint)_LDHINT_NONE);
    return ret == expected;
}

#else

extern "C" {
    Q_CORE_EXPORT int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
    Q_CORE_EXPORT int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
} // extern "C"

#endif

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

inline int q_atomic_fetch_and_add(volatile int *ptr, int value)
{
    register int originalValue;
    for (;;) {
        originalValue = *ptr;
        if (q_atomic_test_and_set_int(ptr, originalValue, originalValue + value))
            break;
    }
    return originalValue;
}

inline int q_atomic_fetch_and_add_acquire(volatile int *ptr, int value)
{
    register int originalValue;
    for (;;) {
        originalValue = *ptr;
        if (q_atomic_test_and_set_acquire_int(ptr, originalValue, originalValue + value))
            break;
    }
    return originalValue;
}

inline int q_atomic_fetch_and_add_release(volatile int *ptr, int value)
{
    register int originalValue;
    for (;;) {
        originalValue = *ptr;
        if (q_atomic_test_and_set_release_int(ptr, originalValue, originalValue + value))
            break;
    }
    return originalValue;
}

#endif // Q_CC_INTEL

QT_END_HEADER

#endif // IA64_QATOMIC_H

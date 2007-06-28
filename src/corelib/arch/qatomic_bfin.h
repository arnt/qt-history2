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

#ifndef BFIN_QATOMIC_H
#define BFIN_QATOMIC_H

QT_BEGIN_HEADER

#if defined(Q_OS_LINUX) && defined(Q_CC_GNU)

#include <asm/fixed_code.h>

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    long int readval;
    asm volatile ("P0 = %2;\n\t"
		  "R1 = %3;\n\t"
		  "R2 = %4;\n\t"
		  "CALL (%5);\n\t"
		  "%0 = R0;\n\t"
		  : "=da" (readval), "=m" (*ptr)
		  : "da" (ptr),
		  "da" (expected),
		  "da" (newval),
		  "a" (ATOMIC_CAS32),
		  "m" (*ptr)
		  : "P0", "R0", "R1", "R2", "RETS", "memory", "cc");
    return readval == expected;
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    return q_atomic_test_and_set_int(reinterpret_cast<volatile int *>(ptr),
                                     reinterpret_cast<int>(expected),
                                     reinterpret_cast<int>(newval));
}

inline int q_atomic_increment(volatile int *ptr)
{
    int ret;
    asm volatile("R0 = 1;\n\t"
		 "P0 = %3;\n\t"
                 "CALL (%2);\n\t"
                 "%0 = R0;"
                 : "=da" (ret), "=m" (*ptr)
                 : "a" (ATOMIC_ADD32), "da" (ptr), "m" (*ptr)
                 : "R0", "R1", "P0", "RETS", "memory");
    return ret;
}

inline int q_atomic_decrement(volatile int *ptr)
{
    int ret;
    asm volatile("R0 = 1;\n\t"
		 "P0 = %3;\n\t"
                 "CALL (%2);\n\t"
                 "%0 = R0;"
                 : "=da" (ret), "=m" (*ptr)
                 : "a" (ATOMIC_SUB32), "da" (ptr), "m" (*ptr)
                 : "R0", "R1", "P0", "RETS", "memory");
    return ret;
}

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    asm volatile("R1 = %2;\n\t"
		 "P0 = %4;\n\t"
                 "CALL (%3);\n\t"
                 "%0 = R0;"
                 : "=da" (newval), "=m" (*ptr)
                 : "da" (newval), "a" (ATOMIC_XCHG32), "da" (ptr), "m" (*ptr)
                 : "R0", "R1", "P0", "RETS", "memory");
    return newval;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    return reinterpret_cast<void *>(q_atomic_set_int(reinterpret_cast<volatile int *>(ptr),
                                                     reinterpret_cast<int>(newval)));
}

#error "q_atomic_fetch_and_add_int() not implemented"

#endif

QT_END_HEADER

#endif // BFIN_QATOMIC_H

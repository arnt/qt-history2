/****************************************************************************
**
** Definition of q_cas_* functions.
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

#ifndef QATOMIC_P_H
#define QATOMIC_P_H

#ifndef QT_H
#  include <qglobal.h>
#endif // QT_H

#if defined(Q_CC_GNU)

inline int q_cas_32(int *volatile ptr, int expected, int newval)
{
    int ret;
    asm ("L..QCAS32_1:\t\n"
	 "lwarx %0, 0, %2\n\t"
         "cmplw %3, %0\n\t"
         "bne L..QCAS32_2f\n\t"
         "stwcx. %4, 0, %2\n\t"
         "bne- L..QCAS32_1b\n\t"
         "b L..QCAS32_3f\n\t"
         "L..QCAS32_2:\n\t"
         "stwcx. %0, 0, %2\n\t"
         "L..QCAS32_3:\n\t"
         : "=&r" (ret), "+m" (*ptr)
         : "r" (ptr), "r" (expected), "r" (newval)
         : "cc", "memory");
    return ret;
}

inline void *q_cas_ptr(void *volatile *ptr, void *expected, void *newval)
{
    void *ret;
#ifdef __LP64
    // 64-bit PowerPC
    asm ("L..QCASPTR_1:\t\n"
	 "ldarx %0, 0, %2\n\t"
         "cmpld %3, %0\n\t"
         "bne L..QCASPTR_2f\n\t"
         "stdcx. %4, 0, %2\n\t"
         "bne- L..QCASPTR_1b\n\t"
         "b L..QCASPTR_3f\n\t"
         "L..QCASPTR_2:\n\t"
         "stdcx. %0, 0, %2\n\t"
         "L..QCASPTR_3:\n\t"
         : "=&r" (ret), "+m" (*ptr)
         : "r" (ptr), "r" (expected), "r" (newval)
         : "cc", "memory");
#else
    // 32-bit PowerPC
    asm ("L..QCASPTR_1:\t\n"
	 "lwarx %0, 0, %2\n\t"
         "cmplw %3, %0\n\t"
         "bne L..QCASPTR_2f\n\t"
         "stwcx. %4, 0, %2\n\t"
         "bne- L..QCASPTR_1b\n\t"
         "b L..QCASPTR_3f\n\t"
         "L..QCASPTR_2:\n\t"
         "stwcx. %0, 0, %2\n\t"
         "L..QCASPTR_3:\n\t"
         : "=&r" (ret), "+m" (*ptr)
         : "r" (ptr), "r" (expected), "r" (newval)
         : "cc", "memory");
#endif
    return ret;
}

#else

// compiler doesn't support inline assembly
extern "C" {
    int q_cas_32(int * volatile ptr, int expected, int newval);
    void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval);
}

#endif

#endif // QATOMIC_P_H

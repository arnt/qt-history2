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
    asm ("1:\tlwarx %0, 0, %2\n\t"
         "cmplw %3, %0\n\t"
         "bne 2f\n\t"
         "stwcx. %4, 0, %2\n\t"
         "bne- 1b\n\t"
         "b 3f\n\t"
         "2:\n\t"
         "stwcx. %0, 0, %2\n\t"
         "3:\n\t"
         : "=&r" (ret), "+m" (*ptr)
         : "r" (ptr), "r" (expected), "r" (newval)
         : "cc", "memory");
    return ret;
}

inline void *q_cas_ptr(void *volatile *ptr, void *expected, void *newval)
{
    void *ret;
    asm ("1:\tlwarx %0, 0, %2\n\t"
         "cmplw %3, %0\n\t"
         "bne 2f\n\t"
         "stwcx. %4, 0, %2\n\t"
         "bne- 1b\n\t"
         "b 3f\n\t"
         "2:\n\t"
         "stwcx. %0, 0, %2\n\t"
         "3:\n\t"
         : "=&r" (ret), "+m" (*ptr)
         : "r" (ptr), "r" (expected), "r" (newval)
         : "cc", "memory");
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

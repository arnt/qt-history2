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

inline int q_cas_32(volatile int *ptr, int expected, int newval)
{
    register int ret;
    asm volatile ("      lwarx %0,0,%1\n"
         "      cmpw %0,%2\n"
         "      bne- $+12\n"
         "      stwcx. %3,0,%1\n"
         "      bne- $-16\n"
         : "=&r" (ret)
         : "r" (ptr), "r" (expected), "r" (newval)
         : "cc", "memory");
    return ret;
}

inline void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval)
{
    register void *ret;
    asm volatile (
#ifdef __64BIT__
         "      ldarx %0,0,%1\n"
         "      cmpd %0,%2\n"
         "      bne- $+12\n"
         "      stdcx. %3,0,%1\n"
         "      bne- $-16\n"
#else
         "      lwarx %0,0,%1\n"
         "      cmpw %0,%2\n"
         "      bne- $+12\n"
         "      stwcx. %3,0,%1\n"
         "      bne- $-16\n"
#endif
         : "=&r" (ret)
         : "r" (ptr), "r" (expected), "r" (newval)
         : "cc", "memory");
    return ret;
}

#else

// compiler doesn't support inline assembly
extern "C" {
    int q_cas_32(volatile int *ptr, int expected, int newval);
    void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval);
}

#endif

#endif // QATOMIC_P_H


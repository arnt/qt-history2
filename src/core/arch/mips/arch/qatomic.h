/****************************************************************************
**
** Definition/Implementation of q_cas_* functions.
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
  register int ret, tmp;
  asm volatile("\n"
      "1:  ll   %0,%2\n"
      "    bne  %0,%3,2f\n"
      "    move %1,$0\n"
      "    move %1,%4\n"
      "    sc   %1,%2\n"
      "    beqz %1,1b\n"
      "2:\n"
      : "=&r" (ret), "=&r" (tmp), "+m" (*ptr)
      : "r" (expected), "r" (newval)
      : "memory" );
  return ret;
}

inline void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval)
{
  register void *ret, *tmp;
  asm volatile("\n"
#if _MIPS_SZPTR == 64
      "1:  lld  %0,%2\n"
      "    bne  %0,%3,2f\n"
      "    move %1,$0\n"
      "    move %1,%4\n"
      "    scd  %1,%2\n"
      "    beqz %1,1b\n"
      "2:\n"
#else
      "1:  ll   %0,%2\n"
      "    bne  %0,%3,2f\n"
      "    move %1,$0\n"
      "    move %1,%4\n"
      "    sc   %1,%2\n"
      "    beqz %1,1b\n"
      "2:\n"
#endif
      : "=&r" (ret), "=&r" (tmp), "+m" (*ptr)
      : "r" (expected), "r" (newval)
      : "memory" );
  return ret;
}

#else

extern "C" {
    int q_cas_32(volatile int *ptr, int expected, int newval);
    void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval);
}

#endif

#endif // QATOMIC_P_H


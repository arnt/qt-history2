/****************************************************************************
**
** Definition of q_atomic_test_and_set_* functions.
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

#ifndef POWERPC_QATOMIC_H
#define POWERPC_QATOMIC_H

#ifndef QT_H
#  include <qglobal.h>
#endif // QT_H

extern "C" {

#if defined(Q_CC_GNU)

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
	register int tmp;
	register int ret;
	asm volatile("lwarx  %0,0,%2\n"
		     "cmpw   %0,%3\n"
		     "bne-   $+20\n"
		     "stwcx. %4,0,%2\n"
		     "bne-   $+12\n"
		     "li     %1,1\n"
		     "b      $+8\n"
		     "li     %1,0\n"
		     : "=&r" (tmp), "=&r" (ret)
		     : "r" (ptr), "r" (expected), "r" (newval)
		     : "cc", "memory");
	return ret;
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
	register void *tmp;
	register int ret;

#ifdef __64BIT__
#  define LPARX "ldarx"
#  define CMPP  "cmpd"
#  define STPCX "stdcx."
#else
#  define LPARX "lwarx"
#  define CMPP  "cmpw"
#  define STPCX "stwcx."
#endif

	asm volatile(LPARX"  %0,0,%2\n"
		     CMPP"   %0,%3\n"
		     "bne-   $+20\n"
		     STPCX"  %4,0,%2\n"
		     "bne-   $+12\n"
		     "li     %1,1\n"
		     "b      $+8\n"
		     "li     %1,0\n"
		     : "=&r" (tmp), "=&r" (ret)
		     : "r" (&ptr), "r" (expected), "r" (newval)
		     : "cc", "memory");

#undef LPARX
#undef CMPP
#undef STPCX

	return ret;
    }

#else

    // compiler doesn't support inline assembly

    Q_CORE_EXPORT
    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);

    Q_CORE_EXPORT
    void *q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);

#endif

} // extern "C"

#endif // POWERPC_QATOMIC_H

/****************************************************************************
**
** Implementation of q_cas_* functions.
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

#include "arch/qatomic.h"

#include <qglobal.h>

#if !defined(Q_CC_GNU)
#  error "This file can only be compiled with the GNU C++ compiler."
#endif

extern "C" {

int q_cas_32(volatile int *ptr, int expected, int newval)
{
    int ret;
    asm ("\n"
	 "q_cas_32_retry:\n"
	 "	lwarx %0, 0, %2\n"
	 "	cmplw %3, %0\n"
	 "	bne q_cas_32_store\n"
	 "	stwcx. %4, 0, %2\n"
	 "	bne- q_cas_32_retry\n"
	 "	b q_cas_32_out\n"
	 "q_cas_32_store:\n"
	 "	stwcx. %0, 0, %2\n"
	 "q_cas_32_out:\n"
	 : "=&r" (ret), "+m" (*ptr)
	 : "r" (ptr), "r" (expected), "r" (newval)
	 : "cc", "memory");
    return ret;
}

void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval)
{
    void *ret;
#ifdef __64BIT__
    // 64-bit PowerPC
    asm ("\n"
	 "q_cas_ptr_retry:\n"
	 "	ldarx %0, 0, %2\n"
	 "	cmpld %3, %0\n"
	 "	bne q_cas_ptr_store\n"
	 "	stdcx. %4, 0, %2\n"
	 "	bne- q_cas_ptr_retry\n"
	 "	b q_cas_ptr_out\n"
	 "q_cas_ptr_store:\n"
	 "	stdcx. %0, 0, %2\n"
	 "q_cas_ptr_out:\n"
	 : "=&r" (ret), "+m" (*ptr)
	 : "r" (ptr), "r" (expected), "r" (newval)
	 : "cc", "memory");
#else
    // 32-bit PowerPC
    asm ("\n"
	 "q_cas_ptr_retry:\n"
	 "	lwarx %0, 0, %2\n"
	 "	cmplw %3, %0\n"
	 "	bne q_cas_ptr_store\n"
	 "	stwcx. %4, 0, %2\n"
	 "	bne- q_cas_ptr_retry\n"
	 "	b q_cas_ptr_out\n"
	 "q_cas_ptr_store:\n"
	 "	stwcx. %0, 0, %2\n"
	 "q_cas_ptr_out:\n"
	 : "=&r" (ret), "+m" (*ptr)
	 : "r" (ptr), "r" (expected), "r" (newval)
	 : "cc", "memory");
#endif
    return ret;
}

} // extern "C"

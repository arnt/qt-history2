/****************************************************************************
**
** Definition/Implementation of q_atomic_* functions.
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

extern "C" {

#if defined(Q_CC_GNU)

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    register int ret;
    asm volatile("ldl_l %0,%1\n"   /* ret=*ptr;                               */
		 "cmpeq %0,%2,%0\n"/* if (ret==expected) ret=0; else ret=1;   */
		 "beq   %0,1f\n"   /* if (ret==0) goto 1;                     */
		 "mov   %3,%0\n"   /* ret=newval;                             */
		 "stl_c %0,%1\n"   /* if ((*ptr=ret)!=ret) ret=0; else ret=1; */
		 "1:\n"
		 : "=&r" (ret), "+m" (*ptr)
		 : "r" (expected), "r" (newval)
		 : "memory");
    return ret;
}

inline int q_atomic_test_and_set_ptr(void * volatile *ptr, void *expected, void *newval)
{
    register void *ret;
    asm volatile("ldq_l %0,%1\n"   /* ret=*ptr;                               */
		 "cmpeq %0,%2,%0\n"/* if (ret==expected) tmp=0; else tmp=1;   */
		 "beq   %0,1f\n"   /* if (tmp==0) goto 4;                     */
		 "mov   %3,%0\n"   /* tmp=newval;                             */
		 "stq_c %0,%1\n"   /* if ((*ptr=tmp)!=tmp) tmp=0; else tmp=1; */
		 "1:\n"
		 : "=&r" (ret), "+m" (*ptr)
		 : "r" (expected), "r" (newval)
		 : "memory");
    return static_cast<int>(reinterpret_cast<long>(ret));
}

#else // !Q_CC_GNU

Q_CORE_EXPORT
int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);

Q_CORE_EXPORT
int q_atomic_test_and_set_ptr(void * volatile *ptr, void *expected, void *newval);

#endif // Q_CC_GNU

} // extern "C"

#endif // QATOMIC_P_H


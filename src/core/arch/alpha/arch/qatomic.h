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
    register int tmp, ret;
    asm volatile(
        "1:  ldl_l %0,%2\n"    /* ret=*ptr;                               */
        "    cmpeq %0,%3,%1\n" /* if (ret==expected) tmp=0; else tmp=1;   */ 
        "    beq %1,4f\n"      /* if (tmp==0) goto 4;                     */
        "    mov %4,%1\n"      /* tmp=newval;                             */
        "    stl_c %1,%2\n"    /* if ((*ptr=tmp)!=tmp) tmp=0; else tmp=1; */
        "    beq %1,3f\n"      /* if (tmp==0) goto 3;                     */
        "2:  br 4f\n"          /* goto 4;                                 */
        "3:  br 1b\n"          /* goto 1;                                 */
        "4:\n"
        : "=&r" (ret), "=&r" (tmp), "+m" (*ptr)
        : "r" (expected), "r" (newval)
        : "memory");
    return ret;
}

inline void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval)
{
    register void *tmp, *ret;
    asm volatile(
        "1:  ldq_l %0,%2\n"    /* ret=*ptr;                               */
        "    cmpeq %0,%3,%1\n" /* if (ret==expected) tmp=0; else tmp=1;   */
        "    beq %1,4f\n"      /* if (tmp==0) goto 4;                     */
        "    mov %4,%1\n"      /* tmp=newval;                             */
        "    stq_c %1,%2\n"    /* if ((*ptr=tmp)!=tmp) tmp=0; else tmp=1; */
        "    beq %1,3f\n"      /* if (tmp==0) goto 3;                     */
        "2:  br 4f\n"          /* goto 4;                                 */
        "3:  br 1b\n"          /* goto 1;                                 */
        "4:\n"
        : "=&r" (ret), "=&r" (tmp), "+m" (*ptr)
        : "r" (expected), "r" (newval)
        : "memory");
    return ret;
}

#else

extern "C" {
    int q_cas_32(volatile int *ptr, int expected, int newval);
    void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval);
}

#endif

#endif // QATOMIC_P_H


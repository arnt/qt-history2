/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ARM_QATOMIC_H
#define ARM_QATOMIC_H

#include "QtCore/qglobal.h"

extern Q_CORE_EXPORT char q_atomic_lock;

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    register int tmp = 1, ret = 0;
    asm volatile(
                 // lock
                 "1:\n"
                 "    swpb %0,%0,[%2]\n"
                 "    teq %0,#0\n"
                 "    bne 1b\n"
                 // increment
                 "    ldr %1,[%3]\n"
                 "    teq %4,%1\n"
                 "    streq %5,[%3]\n"
                 "    moveq %1,#1\n"
                 "    movne %1,#0\n"
                 // unlock
                 "    mov %0,#0\n"
                 "    swpb %0,%0,[%2]\n"

                 : "+r"(tmp), "+r"(ret)
                 : "r"(&q_atomic_lock), "r"(ptr), "r"(expected), "r"(newval)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    register int tmp = 1, ret = 0;
    asm volatile(
                 // lock
                 "1:\n"
                 "    swpb %0,%0,[%2]\n"
                 "    teq %0,#0\n"
                 "    bne 1b\n"
                 // increment
                 "    ldr %1,[%3]\n"
                 "    teq %4,%1\n"
                 "    streq %5,[%3]\n"
                 "    moveq %1,#1\n"
                 "    movne %1,#0\n"
                 // unlock
                 "    mov %0,#0\n"
                 "    swpb %0,%0,[%2]\n"

                 : "+r"(tmp), "+r"(ret)
                 : "r"(&q_atomic_lock), "r"(ptr), "r"(expected), "r"(newval)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_increment(volatile int *ptr)
{
    register int tmp = 1, ret = 0;
    asm volatile(
                 // lock
                 "1:\n"
                 "    swpb %0,%0,[%2]\n"
                 "    teq %0,#0\n"
                 "    bne 1b\n"
                 // increment
                 "    ldr %1,[%3]\n"
                 "    add %1,%1,#1\n"
                 "    str %1,[%3]\n"
                 // unlock
                 "    mov %0,#0\n"
                 "    swpb %0,%0,[%2]\n"

                 : "+r"(tmp), "+r"(ret)
                 : "r"(&q_atomic_lock), "r"(ptr)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_decrement(volatile int *ptr)
{
    register int tmp = 1, ret = 0;
    asm volatile(
                 // lock
                 "1:\n"
                 "    swpb %0,%0,[%2]\n"
                 "    teq %0,#0\n"
                 "    bne 1b\n"
                 // increment
                 "    ldr %1,[%3]\n"
                 "    sub %1,%1,#1\n"
                 "    str %1,[%3]\n"
                 // unlock
                 "    mov %0,#0\n"
                 "    swpb %0,%0,[%2]\n"

                 : "+r"(tmp), "+r"(ret)
                 : "r"(&q_atomic_lock), "r"(ptr)
                 : "cc", "memory");
    return ret;
}

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    register int tmp = 1, ret = 0;
    asm volatile(
                 // lock
                 "1:\n"
                 "    swpb %0,%0,[%2]\n"
                 "    teq %0,#0\n"
                 "    bne 1b\n"
                 // increment
                 "    ldr %1,[%3]\n"
                 "    str %4,[%3]\n"
                 // unlock
                 "    mov %0,#0\n"
                 "    swpb %0,%0,[%2]\n"

                 : "+r"(tmp), "+r"(ret)
                 : "r"(&q_atomic_lock), "r"(ptr), "r"(newval)
                 : "cc", "memory");
    return ret;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register int tmp = 1;
    register void *ret = 0;
    asm volatile(
                 // lock
                 "1:\n"
                 "    swpb %0,%0,[%2]\n"
                 "    teq %0,#0\n"
                 "    bne 1b\n"
                 // increment
                 "    ldr %1,[%3]\n"
                 "    str %4,[%3]\n"
                 // unlock
                 "    mov %0,#0\n"
                 "    swpb %0,%0,[%2]\n"

                 : "+r"(tmp), "+r"(ret)
                 : "r"(&q_atomic_lock), "r"(ptr), "r"(newval)
                 : "cc", "memory");
    return ret;
}

#endif // ARM_QATOMIC_H

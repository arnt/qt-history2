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

#ifndef X86_64_QATOMIC_H
#define X86_64_QATOMIC_H

#ifndef QT_H
#  include <qglobal.h>
#endif // QT_H

extern "C" {

#if defined(Q_CC_GNU)

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
        unsigned char ret;
        asm volatile("lock cmpxchgl %2,%3\n"
                     "sete %1\n"
                     : "=a" (newval), "=qm" (ret)
                     : "r" (newval), "m" (*ptr), "0" (expected)
                     : "memory");
        return static_cast<int>(ret);
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
        unsigned char ret;
        asm volatile("lock cmpxchgq %2,%3\n"
                     "sete %1\n"
                     : "=a" (newval), "=qm" (ret)
                     : "r" (newval), "m" (*reinterpret_cast<volatile long *>(ptr)), "0" (expected)
                     : "memory");
        return static_cast<int>(ret);
    }

#define Q_HAVE_ATOMIC_INCDEC

    inline int q_atomic_increment(volatile int *ptr)
    {
        unsigned char ret;
        asm volatile("lock incl %0\n"
                     "setne %1"
                     : "=m" (*ptr), "=qm" (ret)
                     : "m" (*ptr)
                     : "memory");
        return static_cast<int>(ret);
    }

    inline int q_atomic_decrement(volatile int *ptr)
    {
        unsigned char ret;
        asm volatile("lock decl %0\n"
                     "setne %1"
                     : "=m" (*ptr), "=qm" (ret)
                     : "m" (*ptr)
                     : "memory");
        return static_cast<int>(ret);
    }

#define Q_HAVE_ATOMIC_SET

    inline int q_atomic_set_int(volatile int *ptr, int newval)
    {
        asm volatile("xchgl %0,%1"
                     : "=r" (newval)
                     : "m" (*ptr), "0" (newval)
                     : "memory");
        return newval;
    }

    inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        asm volatile("xchgq %0,%1"
                     : "=r" (newval)
                     : "m" (*reinterpret_cast<volatile long *>(ptr)), "0" (newval)
                     : "memory");
        return newval;
    }

#endif

} // extern "C"

#endif // X86_64_QATOMIC_H

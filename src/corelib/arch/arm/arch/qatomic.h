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

#ifdef Q_CC_GNU

extern "C" {
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
    void *q_atomic_set_ptr(volatile void *ptr, void *newval);
}

inline int q_atomic_lock(volatile int *ptr)
{
    register int ret = 1;
    asm volatile(
        "        swp %0,%0,[%1]\n"
        : "=r"(ret)
        : "r"(ptr)
        : "cc");
    return ret;
}

inline void q_atomic_unlock(volatile int *ptr)
{
    *ptr = 0;
    asm ("" : : : "memory");
    /*
      register int tmp = 0;
      asm volatile(
      "        swp %0,%0,[%1]\n"
      : "=r"(tmp)
      : "r"(ptr)
      : "cc");
      Q_UNUSED(tmp);
    */
}

#define Q_SPECIALIZED_QATOMIC

struct QBasicAtomic
{
    int lock;
    volatile int atomic;

    void init(int x  =0)
    {
        lock = 0;
        atomic = x;
    }

    inline bool ref()
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        bool ret = (++atomic != 0);
        q_atomic_unlock(&lock);
        return ret;
    }

    inline bool deref()
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        bool ret = (--atomic != 0);
        q_atomic_unlock(&lock);
        return ret;
    }

    inline bool operator==(int x) const
    { return atomic == x; }

    inline bool operator!=(int x) const
    { return atomic != x; }

    inline bool operator!() const
    { return atomic == 0; }

    inline operator int() const
    { return atomic; }

    inline QBasicAtomic &operator=(int x)
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        atomic = x;
        q_atomic_unlock(&lock);
        return *this;
    }

    inline bool testAndSet(int expected, int newval)
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        if (atomic == expected) {
            atomic = newval;
            q_atomic_unlock(&lock);
            return 1;
        }
        q_atomic_unlock(&lock);
        return 0;
    }

    inline int exchange(int newval)
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        int oldval = atomic;
        atomic = newval;
        q_atomic_unlock(&lock);
        return oldval;
    }
};

template <typename T>
struct QBasicAtomicPointer
{
    int lock;
    volatile T *pointer;

    void init(T *t = 0)
    {
        lock = 0;
        pointer = t;
    }

    inline bool operator==(T *t) const
    { return pointer == t; }

    inline bool operator!=(T *t) const
    { return !operator==(t); }

    inline bool operator!() const
    { return operator==(0); }

    inline operator T *() const
    { return const_cast<T *>(pointer); }

    inline T *operator->() const
    { return const_cast<T *>(pointer); }

    inline QBasicAtomicPointer &operator=(T *t)
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        pointer == t;
        q_atomic_unlock(&lock);
        return *this;
    }

    inline bool testAndSet(T *expected, T *newval)
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        if (pointer == expected) {
            pointer = newval;
            q_atomic_unlock(&lock);
            return 1;
        }
        q_atomic_unlock(&lock);
        return 0;
    }

    inline T *exchange(T * newval)
    {
        while (q_atomic_lock(&lock) != 0)
            ;
        T *oldval = const_cast<T *>(pointer);
        pointer = newval;
        q_atomic_unlock(&lock);
        return oldval;
    }
};

#define Q_ATOMIC_INIT(a) {0,(a)}




#if 0
// ARM v6 and up

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    register int tmp;
    asm volatile(
        "1:\n"
        "        ldrex %0,[%1]\n"
        "        teq %0,%2\n"
        "        bne 2f\n"
        "        strex %0,%3,[%1]\n"
        "        teq %0,#0\n"
        "        bne 1b\n"
        "        mov %0,#1\n"
        "        b 3f\n"
        "2:"
        "        mov    %0,#0"
        "3:"
        : "=&r" (tmp),"+m" (ptr)
        :  "r"(expected), "r" (newval)
        : "cc");
    return tmp;
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    register int tmp;
    asm volatile(
        "1:\n"
        "        ldrex %0,[%1]\n"
        "        teq %0,%2\n"
        "        bne 2f\n"
        "        strex %0,%3,[%1]\n"
        "        teq %0,#0\n"
        "        bne 1b\n"
        "        mov %0,#1\n"
        "        b 3f\n"
        "2:"
        "        mov    %0,#0"
        "3:"
        : "=&r" (tmp),"+m" (ptr)
        :  "r"(expected), "r" (newval)
        : "cc");
    return tmp;
}

inline int q_atomic_increment(volatile int *ptr)
{
    register int ret,tmp;
    asm volatile(
        "1:\n"
        "        ldrex %0,[%2]\n"
        "        add %0,%0,#1\n"
        "        strex %1,%0,[%1]\n"
        "        teq %1,#0\n"
        "        bne 1b\n"
        : "=&r"(ret),"=r&"(tmp), "+m"(ptr)
        :
        : "cc");
    return ret;
}

inline int q_atomic_decrement(volatile int *ptr)
{
    register int ret,tmp;
    asm volatile(
        "1:\n"
        "        ldrex %0,[%2]\n"
        "        sub %0,%0,#1\n"
        "        strex %1,%0,[%1]\n"
        "        teq %1,#0\n"
        "        bne 1b\n"
        : "=&r"(ret),"=r&"(tmp), "+m"(ptr)
        :
        : "cc");
    return ret;
}

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    register int ret, tmp;
    asm volatile(
        "1:     ldrex   %0, [%2]\n"
        "       strex   %1, %3, [%2]\n"
        "       teq     %1, #0\n"
        "       bne     1b"
        : "=&r"(ret), "=&r" (tmp),"+m" (ptr)
        :  "r" (newval)
        : "cc");
    return ret;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register void *ret, *tmp;
    asm volatile(
        "1:     ldrex   %0, [%2]\n"
        "       strex   %1, %3, [%2]\n"
        "       teq     %1, #0\n"
        "       bne     1b"
        : "=&r"(ret), "=&r" (tmp),"+m" (ptr)
        :  "r" (newval)
        : "cc");
    return ret;
}

#endif // ARMv6

#endif // Q_CC_GNU

#endif // ARM _QATOMIC_H

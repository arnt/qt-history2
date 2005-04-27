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

inline int q_atomic_swp(volatile int *ptr, int newval)
{
    register int ret;
    asm volatile("swp %0,%1,[%2]"
                 : "=r"(ret)
                 : "r"(newval), "r"(ptr)
                 : "cc", "memory");
    return ret;
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
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        bool ret = (++atomic != 0);
        (void) q_atomic_swp(&lock, 0);
        return ret;
    }

    inline bool deref()
    {
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        bool ret = (--atomic != 0);
        (void) q_atomic_swp(&lock, 0);
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
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        atomic = x;
        (void) q_atomic_swp(&lock, 0);
        return *this;
    }

    inline bool testAndSet(int expected, int newval)
    {
        bool ret = false;
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        if (atomic == expected) {
            atomic = newval;
            ret = true;
        }
        (void) q_atomic_swp(&lock, 0);
        return ret;
    }

    inline int exchange(int newval)
    {
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        int oldval = atomic;
        atomic = newval;
        (void) q_atomic_swp(&lock, 0);
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
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        pointer == t;
        (void) q_atomic_swp(&lock, 0);
        return *this;
    }

    inline bool testAndSet(T *expected, T *newval)
    {
        bool ret = false;
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        if (pointer == expected) {
            pointer = newval;
            ret = true;
        }
        (void) q_atomic_swp(&lock, 0);
        return ret;
    }

    inline T *exchange(T * newval)
    {
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        T *oldval = const_cast<T *>(pointer);
        pointer = newval;
        (void) q_atomic_swp(&lock, 0);
        return oldval;
    }
};

#define Q_ATOMIC_INIT(a) {0,(a)}

#endif // Q_CC_GNU

#endif // ARM _QATOMIC_H

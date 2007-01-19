/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GENERIC_QATOMIC_H
#define GENERIC_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    if (*ptr == expected) {
        *ptr = newval;
        return 1;
    }
    return 0;
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    if (*reinterpret_cast<void * volatile *>(ptr) == expected) {
        *reinterpret_cast<void * volatile *>(ptr) = newval;
        return 1;
    }
    return 0;
}

inline int q_atomic_increment(volatile int *ptr)
{ return ++(*ptr); }

inline int q_atomic_decrement(volatile int *ptr)
{ return --(*ptr); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    register int ret = *ptr;
    *ptr = newval;
    return ret;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register void *ret = *reinterpret_cast<void * volatile *>(ptr);
    *reinterpret_cast<void * volatile *>(ptr) = newval;
    return ret;
}

inline int q_atomic_fetch_and_add(volatile int *ptr, int value)
{
    int originalValue = *ptr;
    *ptr += value;
    return originalValue;
}

inline int q_atomic_fetch_and_add_acquire(volatile int *ptr, int value)
{
    return q_atomic_fetch_and_add(ptr, value);
}

inline int q_atomic_fetch_and_add_release(volatile int *ptr, int value)
{
    return q_atomic_fetch_and_add(ptr, value);
}

QT_END_HEADER

#endif // GENERIC_QATOMIC_H

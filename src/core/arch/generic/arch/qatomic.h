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

#ifndef GENERIC_QATOMIC_H
#define GENERIC_QATOMIC_H

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    if (*ptr == expected) {
        *ptr = newval;
        return 1;
    }
    return 0;
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

#endif // GENERIC_QATOMIC_H

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

#ifndef MACOSX_QATOMIC_H
#define MACOSX_QATOMIC_H

#include <QtCore/qglobal.h>
#include <libkern/OSAtomic.h>

QT_BEGIN_HEADER

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
     return OSAtomicCompareAndSwap32(expected, newval, const_cast<int *>(ptr));
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return OSAtomicCompareAndSwap32Barrier(expected, newval, const_cast<int *>(ptr));
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return OSAtomicCompareAndSwap32Barrier(expected, newval, const_cast<int *>(ptr));
}

#if defined(__LP64__)
inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    return OSAtomicCompareAndSwap64(reinterpret_cast<int64_t>(expected), reinterpret_cast<int64_t>(newval), reinterpret_cast<int64_t *>(const_cast<void *>(ptr)));
}
#else
inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    return OSAtomicCompareAndSwap32(reinterpret_cast<int32_t>(expected), reinterpret_cast<int32_t>(newval), reinterpret_cast<int32_t *>(const_cast<void *>(ptr)));
}
#endif

inline int q_atomic_increment(volatile int *ptr)
{ return OSAtomicIncrement32(const_cast<int *>(ptr)); }

inline int q_atomic_decrement(volatile int *ptr)
{ return OSAtomicDecrement32(const_cast<int *>(ptr)); }


inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    register int ret;
    do {
        ret = *ptr;
    } while (OSAtomicCompareAndSwap32(ret, newval, const_cast<int *>(ptr)) == false);
    return ret;
}

#if defined(__LP64__)
inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register int64_t ret;
    do {
        ret = *reinterpret_cast<int64_t *>(const_cast<void *>(ptr));
    } while (OSAtomicCompareAndSwap64(ret, reinterpret_cast<int64_t>(newval), reinterpret_cast<int64_t *>(const_cast<void *>(ptr))) == false);
    return reinterpret_cast<void *>(ret);
}
#else
inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register int32_t ret;
    do {
        ret = *reinterpret_cast<int32_t *>(const_cast<void *>(ptr));
    } while (OSAtomicCompareAndSwap32(ret, reinterpret_cast<int32_t>(newval), reinterpret_cast<int32_t *>(const_cast<void *>(ptr))) == false);
    return reinterpret_cast<void *>(ret);
}
#endif

QT_END_HEADER

#endif // MACOSX_QATOMIC_H

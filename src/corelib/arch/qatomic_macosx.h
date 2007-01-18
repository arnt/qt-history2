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

// Use the functions in OSAtomic.h if we are in 64-bit mode. This header is
// unfortunately not available on 10.3, so we can't use it in 32-bit
// mode. (64-bit is not supported on 10.3.)
#if defined (__LP64__)
#include <libkern/OSAtomic.h>
#endif

QT_BEGIN_HEADER

#if defined (__LP64__)

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

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    return OSAtomicCompareAndSwap64(reinterpret_cast<int64_t>(expected), reinterpret_cast<int64_t>(newval), reinterpret_cast<int64_t *>(const_cast<void *>(ptr)));
}

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

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register int64_t ret;
    do {
        ret = *reinterpret_cast<int64_t *>(const_cast<void *>(ptr));
    } while (OSAtomicCompareAndSwap64(ret, reinterpret_cast<int64_t>(newval), reinterpret_cast<int64_t *>(const_cast<void *>(ptr))) == false);
    return reinterpret_cast<void *>(ret);
}

#elif defined(_ARCH_PPC) || defined(Q_CC_XLC)
#  include <QtCore/qatomic_powerpc.h>
#elif defined(__i386__)
#  include <QtCore/qatomic_i386.h>
#endif

QT_END_HEADER

#endif // MACOSX_QATOMIC_H

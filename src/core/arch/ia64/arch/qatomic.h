/****************************************************************************
**
** Definition of q_atomic_test_and_set_* functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef IA64_QATOMIC_H
#define IA64_QATOMIC_H

#ifndef QT_H
#  include <qglobal.h>
#endif // QT_H

extern "C" {

#if defined(Q_CC_GNU) && !defined(Q_CC_INTEL)

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
        int ret;
        asm volatile("mov ar.ccv=%2\n"
                     ";;\n"
                     "cmpxchg4.acq %0=%1,%3,ar.ccv\n"
                     : "=r" (ret), "+m" (*ptr)
                     : "r" (expected), "r" (newval)
                     : "memory");
        return ret == expected;
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval) {
        void *ret;
        asm volatile("mov ar.ccv=%2\n"
                     ";;\n"
                     "cmpxchg8.acq %0=%1,%3,ar.ccv\n"
                     : "=r" (ret), "+m" (*ptr)
                     : "r" (expected), "r" (newval)
                     : "memory");
        return ret == expected;
    }

#else

    Q_CORE_EXPORT
    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);

    Q_CORE_EXPORT
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);

#endif // Q_CC_GNU && !Q_CC_INTEL

} // extern "C"

#endif // IA64_QATOMIC_H

/****************************************************************************
**
** Definition/Implementation of q_atomic_* functions.
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

#ifndef QATOMIC_P_H
#define QATOMIC_P_H

extern "C" {

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

#define Q_HAVE_ATOMIC_INCDEC

    inline int q_atomic_increment(volatile int *ptr)
    { return ++(*ptr); }

    inline int q_atomic_decrement(volatile int *ptr)
    { return --(*ptr); }

#define Q_HAVE_ATOMIC_SET

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

} // extern "C"

#endif // QATOMIC_P_H

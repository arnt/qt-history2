/****************************************************************************
**
** Definition/Implementation of q_atomic_* functions.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

inline int q_atomic_test_and_set_ptr(void * volatile *ptr, void *expected, void *newval)
{
    if (*ptr == expected) {
	*ptr = newval;
	return 1;
    }
    return 0;
}

} // extern "C"

#endif // QATOMIC_P_H

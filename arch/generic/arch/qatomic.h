/****************************************************************************
**
** Definition/Implementation of q_cas_* functions.
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

inline int q_cas_32(volatile int *ptr, int expected, int newval)
{
    int p = *ptr;
    if (p == expected) {
        *ptr = newval;
        return expected;
    }
    return p;
}

inline void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval)
{
    void *p = *ptr;
    if (p == expected) {
        *ptr = newval;
        return expected;
    }
    return p;
}

#endif // QATOMIC_P_H


/****************************************************************************
**
** Definition of q_cas_* functions.
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
    int q_cas_32(int * volatile ptr, int expected, int newval);
    void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval);
}

#endif // QATOMIC_P_H


/****************************************************************************
**
** Implementation of q_atomic_test_and_set_* functions.
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

#include "arch/qatomic.h"

extern "C" {

    int q_atomic_test_and_set_int(volatile int *pointer, int expected, int newval)
    {
        __asm {
            mov EBX,pointer
                mov EAX,expected
                mov ECX,newval
                lock cmpxchg dword ptr[EBX],ECX
                mov EAX,0
                sete AL
                mov newval,EAX
                }
        return newval;
    }

    int q_atomic_test_and_set_ptr(void * volatile *pointer, void *expected, void *newval)
    {
        return q_atomic_test_and_set_int(reinterpret_cast<volatile int *>(pointer),
                                         reinterpret_cast<int>(expected),
                                         reinterpret_cast<int>(newval));
    }

} // extern "C"

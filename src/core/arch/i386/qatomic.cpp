/****************************************************************************
**
** Implementation of q_cas_* functions.
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

#include "arch/qatomic.h"

extern "C" {

    /*
     * int q_cas_32(volatile int *ptr, int expected, int newval);
     *
     * Atomic compare-and-set for 32-bit integers.
     *
     * This function atomically compares the contents of \a ptr with \a
     * expected.  If they are equal, the contents of \a ptr and \a newval
     * are swapped and \a newval (which now contains the previous contents
     * of \a ptr) is returned; otherwise nothing happens and the current
     * contents of \a ptr is returned.
     *
     * \sa q_cas_ptr()
     */
    int q_cas_32(volatile int *pointer, int expected, int newval)
    {
	__asm {
	    mov EBX,pointer
	    mov EAX,expected
	    mov ECX,newval
	    lock cmpxchg dword ptr[EBX],ECX
	    mov newval,EAX
	}
	return newval;
    }

    /*! \internal
     *  \function void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval)
     *
     * Atomic compare-and-set for pointers.
     *
     * This function atomically compares the contents of \a ptr with \a
     * expected.  If they are equal, the contents of \a ptr and \a newval
     * are swapped and \a newval (which now contains the previous contents
     * of \a ptr) is returned; otherwise nothing happens and the current
     * contents of \a ptr is returned.
     *
     * \sa q_cas_32()
     */
    void *q_cas_ptr(void * volatile *pointer, void *expected, void *newval)
    {
	__asm {
	    mov EBX,pointer
	    mov EAX,expected
	    mov ECX,newval
	    lock cmpxchg dword ptr[EBX],ECX
	    mov newval,EAX
	}
	return newval;
    }

} // extern "C"

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
.section ".text"
.align 4
.type q_cas_32,#function
.global q_cas_32
q_cas_32:
q_cas_32_spin:
    ld [%o0],%l0
    cmp %l0,-1
    be q_cas_32_spin

    mov -1,%l0
    swap [%o0],%l0
    cmp %l0,-1
    be q_cas_32_spin

    cmp %l0,%o1
    bne q_cas_32_failed
    stbar
    st %o2,[%o0]
    retl
    mov %l0,%o0

q_cas_32_failed:
    stbar
    st %l0,[%o0]
    retl
    mov %l0,%o0
.size q_cas_32,.-q_cas_32

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
.section ".text"
.align 4
.type q_cas_ptr,#function
.global q_cas_ptr
q_cas_ptr:
q_cas_ptr_spin:
    ld [%o0],%l0
    cmp %l0,-1
    be q_cas_ptr_spin

    mov -1,%l0
    swap [%o0],%l0
    cmp %l0,-1
    be q_cas_ptr_spin

    cmp %l0,%o1
    bne q_cas_ptr_failed
    stbar
    st %o2,[%o0]
    retl
    mov %l0,%o0

q_cas_ptr_failed:
    stbar
    st %l0,[%o0]
    retl
    mov %l0,%o0
.size q_cas_ptr,.-q_cas_ptr

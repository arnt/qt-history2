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
    int q_cas_32(int *volatile ptr, int expected, int newval)
    {
	__asm {
	    mov ECX,ptr
	    mov EAX,expected
	    mov EDX,newval
	    lock cmpxchg dword EDX,ptr[ECX]
	    mov value,EAX
	}
	return newval;
    }

    /*! \internal
     *  \function void *q_cas_ptr(volatile void **ptr, void *expected, void *newval)
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
    void *q_cas_ptr(void *volatile *ptr, void *expected, void *newval)
    {
	__asm {
	    mov ECX,ptr
	    mov EAX,expected
	    mov EDX,newval
	    lock cmpxchg dword EDX,ptr[ECX]
	    mov newval,EAX
	}
	return newval;
    }

} // extern "C"

/****************************************************************************
**
** Definition of q_atomic_* functions.
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

#ifndef I386_QATOMIC_H
#define I386_QATOMIC_H

#ifndef QT_H
#  include <qglobal.h>
#endif // QT_H

extern "C" {

#if defined(Q_CC_GNU) || (defined(Q_OS_UNIX) && defined(Q_CC_INTEL))

    inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
	unsigned char ret;
	asm volatile("lock cmpxchgl %2,%3\n"
		     "sete %1\n"
		     : "=a" (newval), "=qm" (ret)
		     : "r" (newval), "m" (*ptr), "0" (expected)
		     : "memory");
	return static_cast<int>(ret);
    }

    inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
	return q_atomic_test_and_set_int(reinterpret_cast<volatile int *>(ptr),
					 reinterpret_cast<int>(expected),
					 reinterpret_cast<int>(newval));
    }

#define Q_HAVE_ATOMIC_INCDEC

    inline int q_atomic_increment(volatile int *ptr)
    {
	unsigned char ret;
	asm volatile("lock incl %0\n"
		     "setne %1"
		     : "=m" (*ptr), "=qm" (ret)
		     : "m" (*ptr)
		     : "memory");
	return static_cast<int>(ret);
    }

    inline int q_atomic_decrement(volatile int *ptr)
    {
	unsigned char ret;
	asm volatile("lock decl %0\n"
		     "setne %1"
		     : "=m" (*ptr), "=qm" (ret)
		     : "m" (*ptr)
		     : "memory");
	return static_cast<int>(ret);
    }

#define Q_HAVE_ATOMIC_SET

    inline int q_atomic_set_int(volatile int *ptr, int newval)
    {
	asm volatile("xchgl %0,%1"
		     : "=r" (newval)
		     : "m" (*ptr), "0" (newval)
		     : "memory" );
	return newval;
    }

    inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
	return reinterpret_cast<void *>(q_atomic_set_int(reinterpret_cast<volatile int *>(ptr),
							 reinterpret_cast<int>(newval)));
    }

#elif defined(Q_OS_WIN) && (defined(Q_CC_MSVC) || defined(Q_CC_INTEL))

    inline int q_atomic_test_and_set_int(volatile int *pointer, int expected, int newval)
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

#define Q_HAVE_ATOMIC_SET

    inline int q_atomic_test_and_set_ptr(volatile void *pointer, void *expected, void *newval)
    {
	return q_atomic_test_and_set_int(reinterpret_cast<volatile int *>(pointer),
					 reinterpret_cast<int>(expected),
					 reinterpret_cast<int>(newval));
    }

    inline int q_atomic_set_int(volatile int *pointer, int newval)
    {
	__asm {
	    mov EBX,pointer
	    mov ECX,newval
	    xchg dword ptr[EBX],ECX
	    mov newval,ECX
	}
	return newval;
    }

    inline void *q_atomic_set_ptr(volatile void *pointer, void *newval)
    {
	return reinterpret_cast<void *>(q_atomic_set_int(reinterpret_cast<volatile int *>(pointer),
							 reinterpret_cast<int>(newval)));
    }

#else

    // compiler doesn't support inline assembly

    Q_CORE_EXPORT
    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);

    Q_CORE_EXPORT
    void *q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);

#endif

} // extern "C"

#endif // I386_QATOMIC_H

#ifndef QATOMIC_P_H
#define QATOMIC_P_H

#ifndef QT_H
#  include <qglobal.h>
#endif // QT_H

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

inline int q_atomic_test_and_set_ptr(void * volatile *ptr, void *expected, void *newval) {
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

extern "C" {

Q_CORE_EXPORT
int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);

Q_CORE_EXPORT
int q_atomic_test_and_set_ptr(void * volatile *ptr, void *expected, void *newval);

} // extern "C"

#endif // Q_CC_GNU

#endif // QATOMIC_P_H


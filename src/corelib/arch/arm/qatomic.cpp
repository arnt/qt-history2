#include "arch/qatomic.h"

#ifdef Q_CC_GNU

extern "C" {
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
        static int lock = 0;
        int ret = 0;
        while (q_atomic_swp(&lock, ~0) != 0)
            ;
        if (*reinterpret_cast<void * volatile *>(ptr) == expected) {
            *reinterpret_cast<void * volatile *>(ptr) = newval;
            ret = 1;
        }
        (void) q_atomic_swp(&lock, 0);
        return ret;
    }

    void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        return reinterpret_cast<void *>(q_atomic_swp(reinterpret_cast<volatile int *>(ptr),
                                                     reinterpret_cast<int>(newval)));
    }
}

#endif

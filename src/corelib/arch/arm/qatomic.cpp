#include "arch/qatomic.h"

#ifdef Q_CC_GNU

extern "C" {
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
        static int lock = 0;
        int ret = 0;
        while (q_atomic_lock(&lock) != 0)
            ;
        if (*reinterpret_cast<void * volatile *>(ptr) == expected) {
            *reinterpret_cast<void * volatile *>(ptr) = newval;
            ret = 1;
        }
        q_atomic_unlock(&lock);
        return ret;
    }

    void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        register void *ret;
        asm("swp %0,%1,[%2]"
            : "=r"(ret)
            : "r"(newval), "r"(ptr)
            : "cc");
        return ret;
    }
}

#endif

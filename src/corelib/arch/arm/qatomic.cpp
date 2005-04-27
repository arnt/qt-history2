#include "arch/qatomic.h"

static char locks[256];

char *getLock(volatile void *addr)
{ return &locks[(ulong(addr) >> 2) & 0xff]; }

#ifdef Q_CC_GNU

inline char q_atomic_swp(volatile char *ptr, char newval)
{
    register int ret;
    asm volatile("swpb %0,%1,[%2]"
                 : "=r"(ret)
                 : "r"(newval), "r"(ptr)
                 : "cc", "memory");
    return ret;
}

#endif // Q_CC_GNU

extern "C" {

    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
    {
        char *lock = getLock(ptr);
        int returnValue = 0;
        while (q_atomic_swp(lock, ~0) != 0)
            ;
        if (*ptr == expected) {
            *ptr = newval;
            returnValue = 1;
        }
        (void) q_atomic_swp(lock, 0);
        return returnValue;
    }

    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
        char *lock = getLock(ptr);
        int returnValue = 0;
        while (q_atomic_swp(lock, ~0) != 0)
            ;
        if (*reinterpret_cast<void * volatile *>(ptr) == expected) {
            *reinterpret_cast<void * volatile *>(ptr) = newval;
            returnValue = 1;
        }
        (void) q_atomic_swp(lock, 0);
        return returnValue;
    }

    int q_atomic_increment(volatile int *ptr)
    {
        char *lock = getLock(ptr);
        while (q_atomic_swp(lock, ~0) != 0)
            ;
        int originalValue = *ptr;
        *ptr = originalValue + 1;
        (void) q_atomic_swp(lock, 0);
        return originalValue != -1;
    }

    int q_atomic_decrement(volatile int *ptr)
    {
        char *lock = getLock(ptr);
        while (q_atomic_swp(lock, ~0) != 0)
            ;
        int originalValue = *ptr;
        *ptr = originalValue - 1;
        (void) q_atomic_swp(lock, 0);
        return originalValue != 1;
    }

    int q_atomic_set_int(volatile int *ptr, int newval)
    {
        char *lock = getLock(ptr);
        while (q_atomic_swp(lock, ~0) != 0)
            ;
        int originalValue = *ptr;
        *ptr = newval;
        (void) q_atomic_swp(lock, 0);
        return originalValue;
    }

    void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        char *lock = getLock(ptr);
        while (q_atomic_swp(lock, ~0) != 0)
            ;
	void *originalValue = *reinterpret_cast<void * volatile *>(ptr);
        *reinterpret_cast<void * volatile *>(ptr) = newval;
        (void) q_atomic_swp(lock, 0);
        return originalValue;
    }
}

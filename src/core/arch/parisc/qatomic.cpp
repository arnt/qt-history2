#include <qhash.h>

#define UNLOCKED    {-1,-1,-1,-1}
#define UNLOCKED2      UNLOCKED,UNLOCKED
#define UNLOCKED4     UNLOCKED2,UNLOCKED2
#define UNLOCKED8     UNLOCKED4,UNLOCKED4
#define UNLOCKED16    UNLOCKED8,UNLOCKED8
#define UNLOCKED32   UNLOCKED16,UNLOCKED16
#define UNLOCKED64   UNLOCKED32,UNLOCKED32
#define UNLOCKED128  UNLOCKED64,UNLOCKED64
#define UNLOCKED256 UNLOCKED128,UNLOCKED128

// use a 4k page for locks
static int locks[256][4] = { UNLOCKED256 };

int *getLock(volatile void *addr) 
{ return locks[qHash(const_cast<void *>(addr)) % 256]; }

static int *align16(int *lock)
{
    long off = (((long) lock) % 16);
    return (int *)(long(lock) + off);
}

extern "C" {

    int q_ldcw(volatile int *addr);

    void q_atomic_lock(int *lock)
    {
        // ldcw requires a 16-byte aligned address
        volatile int *x = align16(lock);
        while (q_ldcw(x) == 0)
	    ;
    }

    void q_atomic_unlock(int *lock)
    { lock[0] = lock[1] = lock[2] = lock[3] = -1; }

    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
    {
	int *lock = getLock(ptr);
	q_atomic_lock(lock);
        if (*reinterpret_cast<void * volatile *>(ptr) == expected) {
	    *reinterpret_cast<void * volatile *>(ptr) = newval;
	    q_atomic_unlock(lock);
	    return 1;
        }
	q_atomic_unlock(lock);
	return 0;
    }

    void *q_atomic_set_ptr(volatile void *ptr, void *newval)
    {
        int *lock = getLock(ptr);
	q_atomic_lock(lock);
	void *oldval = *reinterpret_cast<void * volatile *>(ptr);
        *reinterpret_cast<void * volatile *>(ptr) = newval;
	q_atomic_unlock(lock);
        return oldval;
    }

}


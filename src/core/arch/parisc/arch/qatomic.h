#ifndef PARISC_QATOMIC_H
#define PARISC_QATOMIC_H

#include <qglobal.h>

extern "C" {
    Q_CORE_EXPORT void q_atomic_lock(int *lock);
    Q_CORE_EXPORT void q_atomic_unlock(int *lock);
    Q_CORE_EXPORT int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
    Q_CORE_EXPORT void *q_atomic_set_ptr(volatile void *ptr, void *newval);
}

#define Q_SPECIALIZED_QATOMIC

struct QAtomic
{
    int lock[4];
    int atomic;

    inline bool operator++()
    {
	q_atomic_lock(lock);
	bool ret = (++atomic != 0);
	q_atomic_unlock(lock);
	return ret;
    }

    inline bool operator--()
    {
	q_atomic_lock(lock);
	bool ret = (--atomic != 0);
	q_atomic_unlock(lock);
	return ret;
    }

    inline bool operator==(int x) const
    { return atomic == x; }

    inline bool operator!=(int x) const
    { return atomic != x; }

    inline bool operator!() const
    { return atomic == 0; }

    inline void operator=(int x)
    { lock[0] = lock[1] = lock[2] = lock[3] = -1; atomic = x; }

    inline operator int() const
    { return atomic; }

    inline bool testAndSet(int expected, int newval)
    {
	q_atomic_lock(lock);
	if (atomic == expected) {
            atomic = newval;
	    q_atomic_unlock(lock);
	    return true;
        }
	q_atomic_unlock(lock);
	return false;
    }

    inline int exchange(int newval)
    {
	q_atomic_lock(lock);
	int oldval = atomic;
	atomic = newval;
	q_atomic_unlock(lock);
	return oldval;
    }
};

template <typename T>
struct QAtomicPointer
{
    int lock[4];
    T *atomic;

    inline bool operator==(T *x) const
    {
	const T * const volatile * const ptr = &atomic;
	return *ptr == x;
    }

    inline bool operator!=(T *x) const
    {
	const T * const volatile * const ptr = &atomic;
	return *ptr != x;
    }

    inline bool operator!() const
    { return operator==(0); }

    inline void operator=(T *x)
    { lock[0] = lock[1] = lock[2] = lock[3] = -1; atomic = x; }

    inline T *operator->()
    { return atomic; }
    inline const T *operator->() const
    { return atomic; }

    inline operator T*() const
    { return const_cast<T *>(atomic); }

    inline bool testAndSet(T* expected, T *newval)
    {
	q_atomic_lock(lock);
	if (atomic == expected) {
	    atomic = newval;
	    q_atomic_unlock(lock);
	    return true;
	}
	q_atomic_unlock(lock);
	return false;
    }

    inline T *exchange(T *newval)
    {
	q_atomic_lock(lock);
	T *oldval = atomic;
	atomic = newval;
	q_atomic_unlock(lock);
	return oldval;
    }
};

#define Q_ATOMIC_INIT(a) {{-1,-1,-1,-1},(a)}

#endif // PARISC_QATOMIC_H

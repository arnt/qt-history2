#ifndef QATOMIC_WIN_H
#ifndef QT_H
#include "qglobal.h"
#endif // QT_H
#define QATOMIC_WIN_H

#if defined(Q_CC_BOR)
static void *qAtomicSetPtr_helper(void ** volatile pointer, void* value)
#else
inline void *qAtomicSetPtr_helper(void ** volatile pointer, void* value)
#endif
{
    __asm {
	mov EAX, pointer
	mov EDX, dword ptr [value]
	xchg dword ptr[EAX], EDX
	mov value, EDX
     }
    return value;
}

template <typename T>
inline T qAtomicSetPtr(T * volatile pointer, T value)
{
    Q_ASSERT(sizeof(T) == 4);
    return static_cast<T>(qAtomicSetPtr_helper((void**volatile)pointer, value));
}

template <typename T>
inline bool qAtomicCompareAndSetPtr(T * volatile pointer, T compare, T value)
{
    Q_ASSERT(sizeof(T) == 4);
    unsigned char result;
    __asm {
        mov ECX, pointer
	mov EAX, compare
	mov EDX, value
	lock cmpxchg dword ptr[ECX], EDX
	sete result
    }
    return (result != 0);
}

#if defined(Q_CC_BOR)
static bool qAtomicInc_helper(volatile int * pointer)
#else
inline bool qAtomicInc_helper(volatile int * pointer)
#endif
{
	unsigned char result;
	__asm {
            mov EDX,dword ptr [pointer]
	    lock inc dword ptr [EDX]
	    sete result
	}
	return (result == 0);
}

#if defined(Q_CC_BOR)
static bool qAtomicDec_helper(volatile int * pointer)
#else
inline bool qAtomicDec_helper(volatile int * pointer)
#endif
{
	unsigned char result;
	__asm {
            mov EDX,dword ptr [pointer]
	    lock dec dword ptr [EDX]
	    sete result
	}
	return (result == 0);
}

struct QAtomic {
    int atomic;

    inline bool operator++()
    {
	volatile int *pointer = &atomic;
	return qAtomicInc_helper( pointer );
    }

    inline bool operator--()
    {
	volatile int *pointer = &atomic;
	return qAtomicDec_helper( pointer );
    }

    inline bool operator==(int x) const
    {
	volatile const int *ptr = &atomic;
	return *ptr == x;
    }

    inline bool operator!=(int x) const
    {
	volatile const int *ptr = &atomic;
	return *ptr != x;
    }

    inline void operator=(int x)
    {
	volatile int *ptr = &atomic;
	*ptr = x;
    }
};

#define Q_ATOMIC_INIT(a) { (a) }

#endif

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QATOMIC_WINDOWS_H
#define QATOMIC_WINDOWS_H

QT_BEGIN_HEADER

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_WAIT_FREE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{ return true; }
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{ return true; }

#define Q_ATOMIC_INT_TEST_AND_SET_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_TEST_AND_SET_IS_WAIT_FREE

inline bool QBasicAtomicInt::isTestAndSetNative()
{ return true; }
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{ return true; }

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_WAIT_FREE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{ return true; }

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_WAIT_FREE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{ return true; }

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{ return true; }

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{ return true; }

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{ return true; }

#if defined(Q_CC_MSVC)

// MSVC++ 6.0 doesn't generate correct code when optimization are turned on!
#if _MSC_VER < 1300 && defined (_M_IX86)

inline bool QBasicAtomicInt::ref()
{
    volatile int *pointer = &_q_value;
    unsigned char retVal;
    __asm {
        mov ECX,pointer
        lock inc DWORD ptr[ECX]
        setne retVal
    }
    return retVal != 0;
}

inline bool QBasicAtomicInt::deref()
{
    volatile int *pointer = &_q_value;
    unsigned char retVal;
    __asm {
        mov ECX,pointer
        lock dec DWORD ptr[ECX]
        setne retVal
    }
    return retVal != 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
    volatile int *pointer = &_q_value;
    __asm {
        mov EDX,pointer
        mov EAX,expectedValue
        mov ECX,newValue
        lock cmpxchg dword ptr[EDX],ECX
        mov newValue,EAX
    }
    return newValue == expectedValue;
}


inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    volatile int *pointer = &_q_value;
    __asm {
        mov EDX,pointer
        mov ECX,newValue
        lock xchg dword ptr[EDX],ECX
        mov newValue,ECX
    }
    return newValue;
}


inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
    volatile int *pointer = &_q_value;
    __asm {
        mov EDX,pointer
        mov ECX,valueToAdd
        lock xadd dword ptr[EDX],ECX
        mov valueToAdd,ECX
    }
    return valueToAdd;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    volatile void *pointer = &_q_value;
    __asm {
        mov EDX,pointer
        mov EAX,expectedValue
        mov ECX,newValue
        lock cmpxchg dword ptr[EDX],ECX
        mov newValue,EAX
    }
    return newValue == expectedValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    volatile void *pointer = &_q_value;
    __asm {
        mov EDX,pointer
        mov ECX,newValue
        lock xchg dword ptr[EDX],ECX
        mov newValue,ECX
    }
    return reinterpret_cast<T *>(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
    volatile void *pointer = &_q_value;
    valueToAdd *= sizeof(T);
    __asm {
        mov EDX,pointer
        mov ECX,valueToAdd
        lock xadd dword ptr[EDX],ECX
        mov pointer,ECX
    }
    return reinterpret_cast<T *>(pointer);
}

#else

// use compiler intrinsics for all atomic functions
extern "C" {
    long _InterlockedIncrement(volatile long *);
    long _InterlockedDecrement(volatile long *);
    long _InterlockedExchange(volatile long *, long);
    long _InterlockedCompareExchange(volatile long *, long, long);
    long _InterlockedExchangeAdd(volatile long *, long);
}
#  pragma intrinsic (_InterlockedIncrement)
#  pragma intrinsic (_InterlockedDecrement)
#  pragma intrinsic (_InterlockedExchange)
#  pragma intrinsic (_InterlockedCompareExchange)
#  pragma intrinsic (_InterlockedExchangeAdd)

#  ifndef _M_IX86
extern "C" {
    void *_InterlockedCompareExchangePointer(void * volatile *, void *, void *);
    void *_InterlockedExchangePointer(void * volatile *, void *);
    __int64 _InterlockedExchangeAdd64(__int64 volatile * Addend, __int64 Value);
}
#    pragma intrinsic (_InterlockedCompareExchangePointer)
#    pragma intrinsic (_InterlockedExchangePointer)
#    pragma intrinsic (_InterlockedExchangeAdd64)
#    define _InterlockedExchangeAddPointer(a,b)
        _InterlockedExchangeAdd64(reinterpret_cast<volatile __int64 *>(a), __int64(b)))
#  else
#    define _InterlockedCompareExchangePointer(a,b,c) \
        _InterlockedCompareExchange(reinterpret_cast<volatile long *>(a), long(b), long(c))
#    define _InterlockedExchangePointer(a, b) \
        _InterlockedExchange(reinterpret_cast<volatile long *>(a), long(b))
#    define _InterlockedExchangeAddPointer(a,b) \
        _InterlockedExchangeAdd(reinterpret_cast<volatile long *>(a), long(b))
#  endif

inline bool QBasicAtomicInt::ref()
{
    return _InterlockedIncrement(reinterpret_cast<volatile long *>(&_q_value)) != 0;
}

inline bool QBasicAtomicInt::deref()
{
    return _InterlockedDecrement(reinterpret_cast<volatile long *>(&_q_value)) != 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
    return _InterlockedCompareExchange(reinterpret_cast<volatile long *>(&_q_value), newValue, expectedValue) == expectedValue;
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    return _InterlockedExchange(reinterpret_cast<volatile long *>(&_q_value), newValue);
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
    return _InterlockedExchangeAdd(reinterpret_cast<volatile long *>(&_q_value), valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    return _InterlockedCompareExchangePointer(reinterpret_cast<void * volatile *>(&_q_value),
                                              newValue, expectedValue) == long(expectedValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    return reinterpret_cast<T *>(_InterlockedExchangePointer(reinterpret_cast<void * volatile *>(&_q_value), newValue));
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
    return reinterpret_cast<T *>(_InterlockedExchangeAddPointer(reinterpret_cast<void * volatile *>(&_q_value), valueToAdd * sizeof(T)));
}

#endif // _MSC_VER ...

#else

#if !(defined Q_CC_BOR) || (__BORLANDC__ < 0x560)

extern "C" {
    __declspec(dllimport) long __stdcall InterlockedCompareExchange(long *, long, long);
    __declspec(dllimport) long __stdcall InterlockedIncrement(long *);
    __declspec(dllimport) long __stdcall InterlockedDecrement(long *);
    __declspec(dllimport) long __stdcall InterlockedExchange(long *, long);
    __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long *, long);
}

#else

extern "C" {
    __declspec(dllimport) long __stdcall InterlockedCompareExchange(long volatile*, long, long);
    __declspec(dllimport) long __stdcall InterlockedIncrement(long volatile*);
    __declspec(dllimport) long __stdcall InterlockedDecrement(long volatile*);
    __declspec(dllimport) long __stdcall InterlockedExchange(long volatile*, long);
    __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long volatile*, long);
}

#endif

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{ return InterlockedCompareExchange(reinterpret_cast<long *>(const_cast<int *>(ptr)), newval, expected) == expected; }

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{ return InterlockedCompareExchange(reinterpret_cast<long *>(const_cast<void *>(ptr)),
                                    reinterpret_cast<long>(newval),
                                    reinterpret_cast<long>(expected)) == reinterpret_cast<long>(expected); }

inline int q_atomic_increment(volatile int *ptr)
{ return InterlockedIncrement(reinterpret_cast<long *>(const_cast<int *>(ptr))); }

inline int q_atomic_decrement(volatile int *ptr)
{ return InterlockedDecrement(reinterpret_cast<long *>(const_cast<int *>(ptr))); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{ return InterlockedExchange(reinterpret_cast<long *>(const_cast<int *>(ptr)), newval); }

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{ return reinterpret_cast<void *>(InterlockedExchange(reinterpret_cast<long *>(const_cast<void *>(ptr)),
                                  reinterpret_cast<long>(newval))); }

inline int q_atomic_fetch_and_add_int(volatile int *ptr, int value)
{
    return InterlockedExchangeAdd(reinterpret_cast<long *>(const_cast<int *>(ptr)), value);
}

#endif // Q_CC_GNU

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
    return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
    return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
    return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
    return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
    return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
    return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

QT_END_HEADER

#endif // QATOMIC_WINDOWS_H

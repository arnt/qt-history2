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

#ifndef QATOMIC_MACOSX_H
#define QATOMIC_MACOSX_H

#if defined(__x86_64__)
#  include <QtCore/qatomic_x86_64.h>
#elif defined(__i386__)
#  include <QtCore/qatomic_i386.h>
#else // !__x86_64 && !__i386__

// PowerPC

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{ return true; }
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{ return false; }

#define Q_ATOMIC_INT_TEST_AND_SET_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isTestAndSetNative()
{ return true; }
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{ return false; }

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{ return false; }

// Use the functions in OSAtomic.h if we are in 64-bit mode. This header is
// unfortunately not available on 10.3, so we can't use it in 32-bit
// mode. (64-bit is not supported on 10.3.)
#if defined (__LP64__)
#include <libkern/OSAtomic.h>
#endif

QT_BEGIN_HEADER

#if defined (__LP64__)

inline bool QBasicAtomicInt::ref()
{
    return OSAtomicIncrement32(&_q_value) != 0;
}

inline bool QBasicAtomicInt::deref()
{
    return OSAtomicDecrement32(&_q_value) != 0;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
     return OSAtomicCompareAndSwap32(expectedValue, newValue, &_q_value);
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
    return OSAtomicCompareAndSwap32Barrier(expectedValue, newValue, &_q_value);
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
    int returnValue;
    for (;;) {
        returnValue = _q_value;
        if (testAndSetRelaxed(returnValue, newValue))
            break;
    }
    return returnValue;
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    int returnValue;
    for (;;) {
        returnValue = _q_value;
        if (testAndSetOrdered(returnValue, newValue))
            break;
    }
    return returnValue;
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
    int returnValue;
    for (;;) {
        returnValue = _q_value;
        if (testAndSetRelaxed(returnValue, _q_value + valueToAdd))
            break;
    }
    return returnValue;
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
    int returnValue;
    for (;;) {
        returnValue = _q_value;
        if (testAndSetOrdered(returnValue, _q_value + valueToAdd))
            break;
    }
    return returnValue;
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
    return OSAtomicCompareAndSwap64(reinterpret_cast<int64_t>(expectedValue),
                                    reinterpret_cast<int64_t>(newValue),
                                    reinterpret_cast<int64_t *>(const_cast<T **>(&_q_value)));
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    return OSAtomicCompareAndSwap64Barrier(reinterpret_cast<int64_t>(expectedValue),
                                           reinterpret_cast<int64_t>(newValue),
                                           reinterpret_cast<int64_t *>(const_cast<T **>(&_q_value)));
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
    T *returnValue;
    for (;;) {
        returnValue = const_cast<T *>(_q_value);
        if (testAndSetRelaxed(returnValue, newValue))
            break;
    }
    return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    T *returnValue;
    for (;;) {
        returnValue = const_cast<T *>(_q_value);
        if (testAndSetOrdered(returnValue, newValue))
            break;
    }
    return returnValue;
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
    T *returnValue;
    for (;;) {
        returnValue = const_cast<T *>(_q_value);
        if (testAndSetRelaxed(returnValue, returnValue + valueToAdd))
            break;
    }
    return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
    T *returnValue;
    for (;;) {
        returnValue = const_cast<T *>(_q_value);
        if (testAndSetOrdered(returnValue, returnValue + valueToAdd))
            break;
    }
    return returnValue;
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

#else

#if defined(Q_CC_GNU)
#ifdef __64BIT__
#  define LPARX "ldarx"
#  define CMPP  "cmpd"
#  define STPCX "stdcx."
#else
#  define LPARX "lwarx"
#  define CMPP  "cmpw"
#  define STPCX "stwcx."
#endif

inline bool QBasicAtomicInt::ref()
{
    register int ret;
    register int one = 1;
    asm volatile("lwarx  %0, 0, %1\n"
                 "add    %0, %2, %0\n"
                 "stwcx. %0, 0, %1\n"
                 "bne-   $-12\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (one)
                 : "cc", "memory");
    return ret != 0;
}

inline bool QBasicAtomicInt::deref()
{
    register int ret;
    register int one = -1;
    asm volatile("lwarx  %0, 0, %1\n"
                 "add    %0, %2, %0\n"
                 "stwcx. %0, 0, %1\n"
                 "bne-   $-12\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (one)
                 : "cc", "memory");
    return ret != 0;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0,0,%2\n"
                 "cmpw   %0,%3\n"
                 "bne-   $+20\n"
                 "stwcx. %4,0,%2\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret != 0;
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0,0,%2\n"
                 "cmpw   %0,%3\n"
                 "bne-   $+20\n"
                 "stwcx. %4,0,%2\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 "eieio\n"
                 : "=&r" (tmp), "=&r" (ret)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret != 0;
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
    register int tmp;
    register int ret;
    asm volatile("eieio\n"
                 "lwarx  %0,0,%2\n"
                 "cmpw   %0,%3\n"
                 "bne-   $+20\n"
                 "stwcx. %4,0,%2\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret != 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
    return testAndSetAcquire(expectedValue, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
    register int ret;
    asm volatile("lwarx  %0, 0, %1\n"
                 "stwcx. %2, 0, %1\n"
                 "bne-   $-8\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
    register int ret;
    asm volatile("lwarx  %0, 0, %1\n"
                 "stwcx. %2, 0, %1\n"
                 "bne-   $-8\n"
                 "eieio\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
    register int ret;
    asm volatile("eieio\n"
                 "lwarx  %0, 0, %1\n"
                 "stwcx. %2, 0, %1\n"
                 "bne-   $-8\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    return fetchAndStoreAcquire(newValue);
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 "stwcx. %1, 0, %3\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (_q_value)
                 : "r" (&_q_value), "r" (valueToAdd)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
    register int tmp;
    register int ret;
    asm volatile("lwarx  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 "stwcx. %1, 0, %3\n"
                 "bne-   $-12\n"
                 "eieio\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (_q_value)
                 : "r" (&_q_value), "r" (valueToAdd)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
    register int tmp;
    register int ret;
    asm volatile("eieio\n"
                 "lwarx  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 "stwcx. %1, 0, %3\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (_q_value)
                 : "r" (&_q_value), "r" (valueToAdd)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
    return fetchAndAddAcquire(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
    register void *tmp;
    register int ret;
    asm volatile(LPARX"  %0,0,%2\n"
                 CMPP"   %0,%3\n"
                 "bne-   $+20\n"
                 STPCX"  %4,0,%2\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
    register void *tmp;
    register int ret;
    asm volatile(LPARX"  %0,0,%2\n"
                 CMPP"   %0,%3\n"
                 "bne-   $+20\n"
                 STPCX"  %4,0,%2\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 "eieio\n"
                 : "=&r" (tmp), "=&r" (ret)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
    register void *tmp;
    register int ret;
    asm volatile("eieio\n"
                 LPARX"  %0,0,%2\n"
                 CMPP"   %0,%3\n"
                 "bne-   $+20\n"
                 STPCX"  %4,0,%2\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    return testAndSetAcquire(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
    register void *ret;
    asm volatile(LPARX"  %0, 0, %1\n"
                 STPCX"  %2, 0, %1\n"
                 "bne-   $-8\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
    register void *ret;
    asm volatile(LPARX"  %0, 0, %1\n"
                 STPCX"  %2, 0, %1\n"
                 "bne-   $-8\n"
                 "eieio\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
    register void *ret;
    asm volatile("eieio\n"
                 LPARX"  %0, 0, %1\n"
                 STPCX"  %2, 0, %1\n"
                 "bne-   $-8\n"
                 : "=&r" (ret)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    return fetchAndStoreAcquire(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
    register T *tmp;
    register T *ret;
    asm volatile(LPARX"  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 STPCX"  %1, 0, %3\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (_q_value)
                 : "r" (&_q_value), "r" (valueToAdd)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
    register T *tmp;
    register T *ret;
    asm volatile(LPARX"  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 STPCX"  %1, 0, %3\n"
                 "bne-   $-12\n"
                 "eieio\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (_q_value)
                 : "r" (&_q_value), "r" (valueToAdd)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
    register T *tmp;
    register T *ret;
    asm volatile("eieio\n"
                 LPARX"  %0, 0, %3\n"
                 "add    %1, %4, %0\n"
                 STPCX"  %1, 0, %3\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=&r" (tmp), "=m" (_q_value)
                 : "r" (&_q_value), "r" (valueToAdd)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
    return fetchAndAddAcquire(valueToAdd);
}

#undef LPARX
#undef CMPP
#undef STPCX

#endif // Q_CC_GNU

#endif // __LP64__

QT_END_HEADER

#endif // !__x86_64__ && !__i386__

#endif // QATOMIC_MACOSX_H

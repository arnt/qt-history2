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

#ifndef QATOMIC_POWERPC_H
#define QATOMIC_POWERPC_H

QT_BEGIN_HEADER

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

#if defined(Q_CC_GNU)
#if defined(__64BIT__) || defined(__powerpc64__)
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
    asm volatile("lwarx  %0, 0, %2\n"
                 "add    %0, %3, %0\n"
                 "stwcx. %0, 0, %2\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=m" (_q_value)
                 : "r" (&_q_value), "r" (one)
                 : "cc", "memory");
    return ret != 0;
}

inline bool QBasicAtomicInt::deref()
{
    register int ret;
    register int one = -1;
    asm volatile("lwarx  %0, 0, %2\n"
                 "add    %0, %3, %0\n"
                 "stwcx. %0, 0, %2\n"
                 "bne-   $-12\n"
                 : "=&r" (ret), "=m" (_q_value)
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
    asm volatile("lwarx  %0,0,%3\n"
                 "cmpw   %0,%4\n"
                 "bne-   $+20\n"
                 "stwcx. %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 "eieio\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (_q_value)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret != 0;
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
    register int tmp;
    register int ret;
    asm volatile("eieio\n"
                 "lwarx  %0,0,%3\n"
                 "cmpw   %0,%4\n"
                 "bne-   $+20\n"
                 "stwcx. %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (_q_value)
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret != 0;
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
    register int ret;
    asm volatile("lwarx  %0, 0, %2\n"
                 "stwcx. %3, 0, %2\n"
                 "bne-   $-8\n"
                 : "=&r" (ret), "=m" (_q_value)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
    register int ret;
    asm volatile("lwarx  %0, 0, %2\n"
                 "stwcx. %3, 0, %2\n"
                 "bne-   $-8\n"
                 "eieio\n"
                 : "=&r" (ret), "=m" (_q_value)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
    register int ret;
    asm volatile("eieio\n"
                 "lwarx  %0, 0, %2\n"
                 "stwcx. %3, 0, %2\n"
                 "bne-   $-8\n"
                 : "=&r" (ret), "=m" (_q_value)
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
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
                 : "r" (&_q_value), "r" (value)
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
                 : "r" (&_q_value), "r" (value)
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
                 : "r" (&_q_value), "r" (value)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
    register void *tmp;
    register int ret;
    asm volatile(LPARX"  %0,0,%3\n"
                 CMPP"   %0,%4\n"
                 "bne-   $+20\n"
                 STPCX"  %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(&_q_value))
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
    register void *tmp;
    register int ret;
    asm volatile(LPARX"  %0,0,%3\n"
                 CMPP"   %0,%4\n"
                 "bne-   $+20\n"
                 STPCX"  %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 "eieio\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(&_q_value))
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
    register void *tmp;
    register int ret;
    asm volatile("eieio\n"
                 LPARX"  %0,0,%3\n"
                 CMPP"   %0,%4\n"
                 "bne-   $+20\n"
                 STPCX"  %5,0,%3\n"
                 "bne-   $-16\n"
                 "li     %1,1\n"
                 "b      $+8\n"
                 "li     %1,0\n"
                 : "=&r" (tmp), "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(&_q_value))
                 : "r" (&_q_value), "r" (expectedValue), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
    register void *ret;
    asm volatile(LPARX"  %0, 0, %2\n"
                 STPCX"  %3, 0, %2\n"
                 "bne-   $-8\n"
                 : "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(&_q_value))
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
    register void *ret;
    asm volatile(LPARX"  %0, 0, %2\n"
                 STPCX"  %3, 0, %2\n"
                 "bne-   $-8\n"
                 "eieio\n"
                 : "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(&_q_value))
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
    register void *ret;
    asm volatile("eieio\n"
                 LPARX"  %0, 0, %2\n"
                 STPCX"  %3, 0, %2\n"
                 "bne-   $-8\n"
                 : "=&r" (ret), "=m" (*reinterpret_cast<volatile long *>(&_q_value))
                 : "r" (&_q_value), "r" (newValue)
                 : "cc", "memory");
    return ret;
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
                 : "r" (&_q_value), "r" (value)
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
                 : "r" (&_q_value), "r" (value)
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
                 : "r" (&_q_value), "r" (value)
                 : "cc", "memory");
    return ret;
}

#undef LPARX
#undef CMPP
#undef STPCX

#else

extern "C" {
    int q_atomic_test_and_set_int(volatile int *ptr, int expectedValue, int newValue);
    int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expectedValue, int newValue);
    int q_atomic_test_and_set_release_int(volatile int *ptr, int expectedValue, int newValue);
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expectedValue, void *newValue);
    int q_atomic_test_and_set_acquire_ptr(volatile void *ptr, void *expectedValue, void *newValue);
    int q_atomic_test_and_set_release_ptr(volatile void *ptr, void *expectedValue, void *newValue);
    int q_atomic_increment(volatile int *);
    int q_atomic_decrement(volatile int *);
    int q_atomic_set_int(volatile int *, int);
    int q_atomic_fetch_and_store_acquire_int(volatile int *ptr, int newValue);
    int q_atomic_fetch_and_store_release_int(volatile int *ptr, int newValue);
    void *q_atomic_set_ptr(volatile void *, void *);
    int q_atomic_fetch_and_store_acquire_ptr(volatile void *ptr, void *newValue);
    int q_atomic_fetch_and_store_release_ptr(volatile void *ptr, void *newValue);
    int q_atomic_fetch_and_add_int(volatile int *ptr, int valueToAdd);
    int q_atomic_fetch_and_add_acquire_int(volatile int *ptr, int valueToAdd);
    int q_atomic_fetch_and_add_release_int(volatile int *ptr, int valueToAdd);
    void *q_atomic_fetch_and_add_ptr(volatile void *ptr, qptrdiff valueToAdd);
    void *q_atomic_fetch_and_add_acquire_ptr(volatile void *ptr, qptrdiff valueToAdd);
    void *q_atomic_fetch_and_add_release_ptr(volatile void *ptr, qptrdiff valueToAdd);
} // extern "C"


inline bool QBasicAtomicInt::ref()
{
    return q_atomic_increment(&_q_value) != 0;
}

inline bool QBasicAtomicInt::deref()
{
    return q_atomic_decrement(&_q_value) != 0;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
    return q_atomic_test_and_set_int(&_q_value, expectedValue, newValue) != 0;
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
    return q_atomic_test_and_set_acquire_int(&_q_value, expectedValue, newValue) != 0;
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
    return q_atomic_test_and_set_release_int(&_q_value, expectedValue, newValue) != 0;
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
    return q_atomic_set_int(&_q_value, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
    return q_atomic_fetch_and_store_acquire_int(&_q_value, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
    return q_atomic_fetch_and_store_release_int(&_q_value, newValue);
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
    return q_atomic_fetch_and_add_int(&_q_value, valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
    return q_atomic_fetch_and_add_acquire_int(&_q_value, valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
    return q_atomic_fetch_and_add_release_int(&_q_value, valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
    return q_atomic_test_and_set_ptr(&_q_value, expectedValue, newValue) != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
    return q_atomic_test_and_set_acquire_ptr(&_q_value, expectedValue, newValue) != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
    return q_atomic_test_and_set_release_ptr(&_q_value, expectedValue, newValue) != 0;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
    return reinterpret_cast<T *>(q_atomic_set_ptr(&_q_value, newValue));
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
    return reinterpret_cast<T *>(q_atomic_fetch_and_store_acquire_ptr(&_q_value, newValue));
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
    return reinterpret_cast<T *>(q_atomic_fetch_and_store_release_ptr(&_q_value, newValue));
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
    return reinterpret_cast<T *>(q_atomic_fetch_and_add_ptr(&_q_value, valueToAdd * sizeof(T)));
}
template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
    return reinterpret_cast<T *>(q_atomic_fetch_and_add_acquire_ptr(&_q_value, valueToAdd * sizeof(T)));
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
    return reinterpret_cast<T *>(q_atomic_fetch_and_add_release_ptr(&_q_value, valueToAdd * sizeof(T)));
}

#endif

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
    return testAndSetAcquire(expectedValue, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    return fetchAndStoreAcquire(newValue);
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
    return fetchAndAddAcquire(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    return testAndSetAcquire(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    return fetchAndStoreAcquire(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
    return fetchAndAddAcquire(valueToAdd);
}

QT_END_HEADER

#endif // QATOMIC_POWERPC_H

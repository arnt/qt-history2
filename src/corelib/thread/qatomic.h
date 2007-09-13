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

#ifndef QATOMIC_H
#define QATOMIC_H

#include <QtCore/qglobal.h>
#include <QtCore/qbasicatomic.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE
QT_MODULE(Core)

// High-level atomic integer operations
class Q_CORE_EXPORT QAtomicInt : public QBasicAtomicInt
{
public:
    inline QAtomicInt(int value = 0)
    {
#ifdef QT_ARCH_PARISC
        _q_lock[0] = _q_lock[1] = _q_lock[2] = _q_lock[3] = -1;
#endif
        _q_value = value;
    }
    inline QAtomicInt(const QAtomicInt &other)
    {
#ifdef QT_ARCH_PARISC
        _q_lock[0] = _q_lock[1] = _q_lock[2] = _q_lock[3] = -1;
#endif
        _q_value = other._q_value;
    }

    inline QAtomicInt &operator=(int value)
    {
        (void) QBasicAtomicInt::operator=(value);
        return *this;
    }

    inline QAtomicInt &operator=(const QAtomicInt &other)
    {
        (void) QBasicAtomicInt::operator=(other);
        return *this;
    }

#ifdef qdoc
    bool operator==(int value) const;
    bool operator!=(int value) const;
    bool operator!() const;
    operator int() const;

    static bool isReferenceCountingNative();
    static bool isReferenceCountingWaitFree();

    bool ref();
    bool deref();

    static bool isTestAndSetNative();
    static bool isTestAndSetWaitFree();

    bool testAndSetRelaxed(int expectedValue, int newValue);
    bool testAndSetAcquire(int expectedValue, int newValue);
    bool testAndSetRelease(int expectedValue, int newValue);
    bool testAndSetOrdered(int expectedValue, int newValue);

    static bool isFetchAndStoreNative();
    static bool isFetchAndStoreWaitFree();

    int fetchAndStoreRelaxed(int newValue);
    int fetchAndStoreAcquire(int newValue);
    int fetchAndStoreRelease(int newValue);
    int fetchAndStoreOrdered(int newValue);

    static bool isFetchAndAddNative();
    static bool isFetchAndAddWaitFree();

    int fetchAndAddRelaxed(int valueToAdd);
    int fetchAndAddAcquire(int valueToAdd);
    int fetchAndAddRelease(int valueToAdd);
    int fetchAndAddOrdered(int valueToAdd);
#endif
};

// High-level atomic pointer operations
template <typename T>
class QAtomicPointer : public QBasicAtomicPointer<T>
{
public:
    inline QAtomicPointer(T *value = 0)
    {
#ifdef QT_ARCH_PARISC
        _q_lock[0] = _q_lock[1] = _q_lock[2] = _q_lock[3] = -1;
#endif
        QBasicAtomicPointer<T>::_q_value = value;
    }
    inline QAtomicPointer(const QAtomicPointer<T> &other)
    {
#ifdef QT_ARCH_PARISC
        _q_lock[0] = _q_lock[1] = _q_lock[2] = _q_lock[3] = -1;
#endif
        QBasicAtomicPointer<T>::_q_value = other._q_value;
    }

    inline QAtomicPointer<T> &operator=(T *value)
    {
        (void) QBasicAtomicPointer<T>::operator=(value);
        return *this;
    }

    inline QAtomicPointer<T> &operator=(const QAtomicPointer<T> &other)
    {
        (void) QBasicAtomicPointer<T>::operator=(other);
        return *this;
    }

#ifdef qdoc
    bool operator==(T *value) const;
    bool operator!=(T *value) const;
    bool operator!() const;
    operator T *() const;
    T *operator->() const;

    static bool isTestAndSetNative();
    static bool isTestAndSetWaitFree();

    bool testAndSetRelaxed(T *expectedValue, T *newValue);
    bool testAndSetAcquire(T *expectedValue, T *newValue);
    bool testAndSetRelease(T *expectedValue, T *newValue);
    bool testAndSetOrdered(T *expectedValue, T *newValue);

    static bool isFetchAndStoreNative();
    static bool isFetchAndStoreWaitFree();

    T *fetchAndStoreRelaxed(T *newValue);
    T *fetchAndStoreAcquire(T *newValue);
    T *fetchAndStoreRelease(T *newValue);
    T *fetchAndStoreOrdered(T *newValue);

    static bool isFetchAndAddNative();
    static bool isFetchAndAddWaitFree();

    T *fetchAndAddRelaxed(qptrdiff valueToAdd);
    T *fetchAndAddAcquire(qptrdiff valueToAdd);
    T *fetchAndAddRelease(qptrdiff valueToAdd);
    T *fetchAndAddOrdered(qptrdiff valueToAdd);
#endif
};

/*!
    This is a helper for the assignment operators of implicitly
    shared classes. Your assignment operator should look like this:

    \code
        MyClass &MyClass:operator=(const MyClass &other)
        { qAtomicAssign(d, other.d); return *this; }
    \endcode
*/
template <typename T>
inline void qAtomicAssign(T *&d, T *x)
{
    if (d == x)
        return;
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

/*!
    This is a helper for the detach method of implicitly shared
    classes. Your private class needs a copy constructor which copies
    the members and sets the refcount to 1. After that, your detach
    function should look like this:

    \code
        void MyClass::detach()
        { qAtomicDetach(d); }
    \endcode
*/
template <typename T>
inline void qAtomicDetach(T *&d)
{
    if (d->ref == 1)
        return;
    T *x = d;
    d = new T(*d);
    if (!x->ref.deref())
        delete x;
}

QT_END_NAMESPACE
QT_END_HEADER

#endif // QATOMIC_H

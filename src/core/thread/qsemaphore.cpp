/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsemaphore.h"

#include "qmutex.h"
#include "qwaitcondition.h"

/*!
    \class QSemaphore qsemaphore.h
    \threadsafe
    \brief The QSemaphore class provides a robust integer semaphore.

    \ingroup thread
    \ingroup environment
    \mainclass

    A QSemaphore can be used to serialize thread execution, in a
    similar way to a QMutex. A semaphore differs from a mutex, in
    that a semaphore can be accessed by more than one thread at a
    time.

    For example, suppose we have an application that stores data in a
    large tree structure. The application creates 10 threads
    (commonly called a thread pool) to perform searches on the tree.
    When the application searches the tree for some piece of data, it
    uses one thread per base node to do the searching. A semaphore
    could be used to make sure that two threads don't try to search
    the same branch of the tree at the same time.

    A non-computing example of a semaphore would be dining at a
    restaurant. A semaphore is initialized to have a maximum count
    equal to the number of chairs in the restaurant. As people
    arrive, they want a seat. As seats are filled, the semaphore is
    accessed, once per person. As people leave, the access is
    released, allowing more people to enter. If a party of 10 people
    want to be seated, but there are only 9 seats, those 10 people
    will wait, but a party of 4 people would be seated (taking the
    available seats to 5, making the party of 10 people wait longer).

    When a semaphore is created it is given a number which is the
    maximum number of concurrent accesses it will permit. This amount
    may be changed using operator++(), operator--(), operator+=() and
    operator-=(). The number of accesses allowed is retrieved with
    available(), and the total number with total(). Note that the
    incrementing functions will block if there aren't enough available
    accesses. Use tryAccess() if you want to acquire accesses without
    blocking.
*/

class QSemaphorePrivate {
public:
    inline QSemaphorePrivate(int n)
        : mutex(false), value(n)
    { }

    QMutex mutex;
    QWaitCondition cond;

    int value;
};

/*!
    Creates a new semaphore, initialize its value to \a n.
*/
QSemaphore::QSemaphore(int n)
{
    Q_ASSERT_X(n >= 0, "QSemaphore", "parameter 'n' must be non-negative");
    d = new QSemaphorePrivate(n);
}

/*!
    Destroys the semaphore.

    \warning Destroying a semaphore that is in use results in
    undefined behavior.
*/
QSemaphore::~QSemaphore()
{ delete d; }

/*!
    Try to get access to the semaphore. If \l available() \< \a n,
    this call will block until it can get all the accesses it wants,
    i.e.  until available() \>= \a n.
*/
void QSemaphore::acquire(int n)
{
    Q_ASSERT_X(n >= 0, "QSemaphore::acquire", "parameter 'n' must be non-negative");
    QMutexLocker locker(&d->mutex);
    while (n > d->value)
        d->cond.wait(locker.mutex());
    d->value -= n;
}

/*!
    Release \a n accesses to the semaphore.
*/
void QSemaphore::release(int n)
{
    Q_ASSERT_X(n >= 0, "QSemaphore::release", "parameter 'n' must be non-negative");
    QMutexLocker locker(&d->mutex);
    d->value += n;
    d->cond.wakeAll();
}

/*!
    Returns the number of accesses currently available to the
    semaphore.
*/
int QSemaphore::value() const
{
    QMutexLocker locker(&d->mutex);
    return d->value;
}

/*!
    Try to get access to the semaphore. If \l available() \< \a n,
    this function will return false immediately. If \l available() \>=
    \a n, this function will take \a n accesses and return true. This
    function does \e not block.
*/
bool QSemaphore::tryAcquire(int n)
{
    Q_ASSERT_X(n >= 0, "QSemaphore::tryAcquire", "parameter 'n' must be non-negative");
    QMutexLocker locker(&d->mutex);
    if (n > d->value)
        return false;
    d->value -= n;
    return true;
}

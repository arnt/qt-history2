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

#include "q3semaphore.h"

#include "qmutex.h"
#include "qwaitcondition.h"




/*!
    \class Q3Semaphore q3semaphore.h
    \threadsafe
    \brief The Q3Semaphore class provides a robust integer semaphore.

    \compat

    A Q3Semaphore can be used to serialize thread execution, in a
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

#ifdef max
#undef max
#endif

class Q3SemaphorePrivate {
public:
    Q3SemaphorePrivate(int);

    QMutex mutex;
    QWaitCondition cond;

    int value, max;
};


Q3SemaphorePrivate::Q3SemaphorePrivate(int m)
    : value(0), max(m)
{
}


/*!
    Creates a new semaphore. The semaphore can be concurrently
    accessed at most \a maxcount times.
*/
Q3Semaphore::Q3Semaphore(int maxcount)
{
    d = new Q3SemaphorePrivate(maxcount);
}


/*!
    Destroys the semaphore.

    \warning If you destroy a semaphore that has accesses in use the
    resultant behavior is undefined.
*/
Q3Semaphore::~Q3Semaphore()
{
    delete d;
}


/*!
    Postfix ++ operator.

    Try to get access to the semaphore. If \l available() == 0, this
    call will block until it can get access, i.e. until available() \>
    0.
*/
int Q3Semaphore::operator++(int)
{
    QMutexLocker locker(&d->mutex);
    while (d->value >= d->max)
        d->cond.wait(locker.mutex());

    ++d->value;
    if (d->value > d->max)
        d->value = d->max;

    return d->value;
}


/*!
    Postfix -- operator.

    Release access of the semaphore. This wakes all threads waiting
    for access to the semaphore.
*/
int Q3Semaphore::operator--(int)
{
    QMutexLocker locker(&d->mutex);

    --d->value;
    if (d->value < 0)
        d->value = 0;

    d->cond.wakeAll();

    return d->value;
}


/*!
    Try to get access to the semaphore. If \l available() \< \a n, this
    call will block until it can get all the accesses it wants, i.e.
    until available() \>= \a n.
*/
int Q3Semaphore::operator+=(int n)
{
    QMutexLocker locker(&d->mutex);

    if (n < 0 || n > d->max) {
        qWarning("Q3Semaphore::operator+=: parameter %d out of range", n);
        n = n < 0 ? 0 : d->max;
    }

    while (d->value + n > d->max)
        d->cond.wait(locker.mutex());

    d->value += n;

    return d->value;
}


/*!
    Release \a n accesses to the semaphore.
*/
int Q3Semaphore::operator-=(int n)
{
    QMutexLocker locker(&d->mutex);

    if (n < 0 || n > d->value) {
        qWarning("Q3Semaphore::operator-=: parameter %d out of range", n);
        n = n < 0 ? 0 : d->value;
    }

    d->value -= n;
    d->cond.wakeAll();

    return d->value;
}


/*!
    Returns the number of accesses currently available to the
    semaphore.
*/
int Q3Semaphore::available() const
{
    QMutexLocker locker(&d->mutex);
    return d->max - d->value;
}


/*!
    Returns the total number of accesses to the semaphore.
*/
int Q3Semaphore::total() const
{
    QMutexLocker locker(&d->mutex);
    return d->max;
}


/*!
    Try to get access to the semaphore. If \l available() \< \a n, this
    function will return false immediately. If \l available() \>= \a n,
    this function will take \a n accesses and return true. This
    function does \e not block.
*/
bool Q3Semaphore::tryAccess(int n)
{
    QMutexLocker locker(&d->mutex);

    if (d->value + n > d->max)
        return false;

    d->value += n;

    return true;
}

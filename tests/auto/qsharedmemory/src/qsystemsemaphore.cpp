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

#include "qsystemsemaphore.h"
#include "qsystemsemaphore_p.h"
#include <qglobal.h>

/*! \class QSystemSemaphore

    \brief The QSystemSemaphore class provides a general system counting semaphore.

    A semaphore is a generalization of a mutex. While a mutex can only be locked once, it's possible to acquire a semaphore multiple times. Semaphores are typically used to protect a certain number of identical resources.  With a system semaphore multiple threads or processes can access the same semaphore.

    There are some platform difference that should be known when using this class:

    * On Windows once all of the QSystemSemaphore have been destroyed or crashed
      the semaphore is automatically removed.
    * On Unix if the process that owns the QSystemSemaphore crashes
      the semaphore is not automatically removed.  When setting the key
      you can force QSystemSemaphore to take ownership if it already exists
      and reset the number of resources to the requested amount.
    * On Unix once the process exists Unix will automatically undo any
      operations that occurred. So if a process acquires and then exits Unix
      will automatically release one.

    Semaphores support two fundamental operations, acquire() and release():

    acquire() tries to acquire 1 resource. If there aren't that many resources available, the call will block until this is the case.

    release(n) releases n resources.

    A system semaphore needs a key that every process can use to access the same semaphore.

    Example:
     QSemaphore sem(QLatin1String("market"), 3);      // semaphore available == 3

     sem.acquire();         // semaphores available == 2
     sem.acquire();         // semaphores available == 1
     sem.acquire();         // semaphores available == 0
     sem.release(5);         // semaphores available == 5
     sem.release(5);         // semaphores available == 10

    A typical application of system semaphores is for controlling access to a circular buffer shared by a producer process and a consumer processes.

    See also QSharedMemory, QSemaphore
 */

/*!
    Creates a new system semaphore with \a key and initializes the number
    of resources it guards to \a initialValue (by default, 0) if it didn't already
    exists.

    \sa acquire(), key().
 */
QSystemSemaphore::QSystemSemaphore(const QString &key, int initialValue, OpenMode mode)
{
    d = new QSystemSemaphorePrivate;
    setKey(key, initialValue, mode);
}

/*!
    Destroys a system semaphore.

    warning: On Windows if it has been acquired it will not automatically release.
    warning: On Unix if it has been acquired, but not released
             it will automatically release once the process exits.
*/
QSystemSemaphore::~QSystemSemaphore()
{
    d->cleanHandle();
    delete d;
}

/*!
    \enum QSystemSemaphore::OpenMode

    \value Open If the semaphore already exists initialValue is not set.
                If the semaphore doesn't exists it will be created and the
                initialValue is set. On Unix after creating QSystemSemaphore
                will take ownership of the semaphore and remove it when
                QSystemSemaphore is destroyed.


    \value Create On Unix QSystemSemaphore will take ownership of the
                  semaphore and set initialValue even if it already exists.
                  This is used when the first semaphore for this key is
                  constructed and you know that any existing semaphore could
                  only exists from a crash.  This applies to Unix where system
                  semaphores will survive a crash.  On windows Create does
                  the exact same behavior as Open as semaphores do not survive
                  a crash.
*/

/*!
    Sets a new key to this system semaphore and initializes the
    number of resources it guards to initialValue (by default, 0) if the semaphore
    didn't previously exists.

    \sa key(), acquire()
 */
void QSystemSemaphore::setKey(const QString &key, int initialValue, OpenMode mode)
{
    if (key == d->key && mode == Open)
        return;
#ifndef Q_OS_WIN
    // optimization to not destroy/create the file & semaphore
    if (key == d->key && mode == Create && d->createdSemaphore && d->createdFile) {
        d->initialValue = initialValue;
        d->unix_key = -1;
        d->handle(mode);
        return;
    }
#endif
    d->cleanHandle();
    d->key = key;
    d->initialValue = initialValue;
    // cache the file name so it doesn't have to be generated all the time.
    d->fileName = d->makeKeyFileName();
    d->handle(mode);
}

/*!
    Returns the key assigned to this system semaphore.

    \sa setKey()
 */
QString QSystemSemaphore::key() const
{
    return d->key;
}

/*!
    Tries to acquire 1 resource guarded by the semaphore.
    If not available, this call will block until enough resources are available.

    \sa release()
  */
bool QSystemSemaphore::acquire()
{
    return d->modifySemaphore(-1);
}

/*!
    Releases \a n resources guarded by the semaphore.
    This function can be used to "create" resources. For example:

    QSystemSemaphore sem(5);  // a semaphore that guards 5 resources
    sem.acquire(5);           // acquire all 5 resources
    sem.release(5);           // release the 5 resources
    sem.release(10);          // "create" 10 new resources

    \sa acquire()
 */
bool QSystemSemaphore::release(int n)
{
    if (n == 0)
        return true;
    if (n < 0) {
        qWarning("QSystemSemaphore::release: n is negative.");
        return false;
    }
    return d->modifySemaphore(n);
}


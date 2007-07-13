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

/*!
  \class QSystemSemaphore
  \since 4.4

  \brief The QSystemSemaphore class provides a general system counting semaphore.

  A semaphore is a generalization of a mutex. While a mutex can be
  locked only once, a semaphore can be acquired multiple times.
  Typically, a semaphore is used to protect a certain number of
  identical resources.

  Like its lighter counterpart QSemaphore, a QSystemSemaphore can be
  accessed from multiple \l {QThread} {threads}. But unlike
  QSemaphore, the much heavier QSystemSemaphore can also be accessed
  from multiple \l {QProcess} {processes}. If your application
  doesn't need to access semaphores from multiple processes, you
  probably want to use QSemaphore.

  When using this class, be aware of the following platform differences:

  \list

  \o Windows: When all instances of QSystemSemaphore for a particular
  key have been deleted, or when all processes having instances of
  QSystemSemaphore for a particular key have terminated or crashed,
  Windows automatically removes its underlying system semaphore.

  \o Unix: If the last process having an instance of QSystemSemaphore
  for a particular key terminates or crashes, Unix does not
  automatically remove its underlying system semaphore. However, when
  a later process then creates its first instance of QSystemSemaphore
  for that same key, it can specify that it wants to \c Create the
  semaphore, in case one already exists for that key due to a crash,
  and it can reset the resource count to the desired value. If the
  \c Open flag is used in that case, QSystemSemaphore will be given
  the exisating system semaphore, but the resource count will not be
  reset.

  \o Unix: Once the process exits Unix will automatically undo any
  operations that occurred. So if a process acquires and then exits
  Unix will automatically release one.

  \endlist

  Semaphores support two fundamental operations, acquire() and release():

  acquire() tries to acquire 1 resource. If there aren't that many
  resources available, the call will block until this is the case.

  release(n) releases n resources.

  A system semaphore needs a key that every process can use to access
  the same semaphore.

  Example:
    QSemaphore sem("market", 3);      // semaphore available == 3

    sem.acquire();         // semaphores available == 2
    sem.acquire();         // semaphores available == 1
    sem.acquire();         // semaphores available == 0
    sem.release(5);         // semaphores available == 5
    sem.release(5);         // semaphores available == 10

  A typical application of system semaphores is for controlling access
  to a circular buffer shared by a producer process and a consumer
  processes.

  See also QSharedMemory, QSemaphore
 */

/*!
  Creates a new system semaphore with \a key and initializes the
  number of resources it guards to \a initialValue (by default, 0) if
  it didn't already exists.  When initializing the key it uses the
  access mode \a mode.
  
  \sa acquire(), key()
 */
QSystemSemaphore::QSystemSemaphore(const QString &key, int initialValue, AccessMode mode)
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
  \enum QSystemSemaphore::AccessMode

  \value Open If the semaphore already exists initialValue is not set.
  If the semaphore doesn't exists it will be created and the
  initialValue is set. On Unix after creating QSystemSemaphore will
  take ownership of the semaphore and remove it when QSystemSemaphore
  is destroyed.


  \value Create On Unix QSystemSemaphore will take ownership of the
  semaphore and set initialValue even if it already exists.  This is
  used when the first semaphore for this key is constructed and you
  know that any existing semaphore could only exists from a crash.
  This applies to Unix where system semaphores will survive a crash.
  On windows Create does the exact same behavior as Open as semaphores
  do not survive a crash.
*/

/*!
  Sets a new \a key to this system semaphore and initializes the
  number of resources it guards to \a initialValue (by default, 0) if
  the semaphore didn't previously exists.  When initializing the key
  it uses the access mode \a mode.

  \sa key(), acquire()
 */
void QSystemSemaphore::setKey(const QString &key, int initialValue, AccessMode mode)
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
  Tries to acquire 1 resource guarded by the semaphore.  If not
  available, this call will block until enough resources are
  available.

  Returns true on success; otherwise returns false.

  \sa release()
 */
bool QSystemSemaphore::acquire()
{
    return d->modifySemaphore(-1);
}

/*!
  Releases \a n resources guarded by the semaphore.  This function
  can be used to "create" resources. For example: Returns true on
  success; otherwise returns false.

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


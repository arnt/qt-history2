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
  accessed from multiple \l {QThread} {threads}. Unlike QSemaphore, a
  QSystemSemaphore can also be accessed from multiple \l {QProcess}
  {processes}. This means QSystemSemaphore is a much heavier class, so
  if your application doesn't need to access your semaphores across
  multiple processes, you will probably want to use QSemaphore.

  When using this class, be aware of the following platform
  differences:

  \list

  \o Windows: When all instances of QSystemSemaphore for a particular
  key have been deleted, or when all processes having instances of
  QSystemSemaphore for a particular key have terminated normally or
  crashed, Windows will automatically remove its underlying system
  semaphore.

  \o Unix: If the last process having an instance of QSystemSemaphore
  for a particular key crashes, Unix does not automatically remove its
  underlying system semaphore. A subsequent process that constructs a
  QSystemSemaphore with that same key will then be allocated the
  existing system semaphore. If the QSystemSemaphore constructor
  specifies \l {QSystemSemaphore::} {Open} as its \l
  {QSystemSemaphore::AccessMode} {access mode}, its resource request
  will not be honored, and the number of resources will remain as it
  was set in the crashed process. Use \l {QSystemSemaphore::} {Create}
  as the \l {QSystemSemaphore::AccessMode} {access mode} to force Unix
  to reset the resource count in the underlying system semaphore.

  \o Unix: When a process using QSystemSemaphore terminates for any
  reason, Unix automatically reverses any acquire operations that
  occurred in that process that were not released. Thus if the process
  acquires a resource and then exits, Unix will release that resource.

  \endlist

  Semaphores support two fundamental operations, acquire() and release():

  acquire() tries to acquire one resource. If there isn't a resource
  available, the call blocks until a resource becomes available. Then
  the resource is acquired and the call returns.

  release() releases one resource so it can be acquired by another
  process. The function can also be called with a parameter n > 1,
  which releases n resources.

  A system semaphore is created with a string key that other processes
  can use to use the same semaphore.

  Example: Create a system semaphore 
  \code
    QSystemSemaphore sem("market", 3, QSystemSemaphore::Create);
                                 // resources available == 3
    sem.acquire();               // resources available == 2
    sem.acquire();               // resources available == 1
    sem.acquire();               // resources available == 0
    sem.release();               // resources available == 1 
    sem.release(2);              // resources available == 3
  \endcode

  A typical application of system semaphores is for controlling access
  to a circular buffer shared by a producer process and a consumer
  processes.

  See also QSharedMemory, QSemaphore
 */

/*!

  Requests a system semaphore for the specified \a key. The parameters
  \a initialValue and \a mode are used according to the following
  rules, which are system dependent.

  In Unix, if the \a mode is \l {QSystemSemaphore::} {Open} and the
  system already has a semaphore identified by \a key, that semaphore
  is used, and the semaphore's resource count is not changed, i.e., \a
  initialValue is ignored. But if the system does not already have a
  semaphore identified by \a key, it creates a new semphore for that
  key and sets its resource count to \a initialValue.

  In Unix, if the \a mode is \l {QSystemSemaphore::} {Create} and the
  system already has a semaphore identified by \a key, that semaphore
  is used, and its resource count is set to \a initialValue. If the
  system does not already have a semaphore identified by \a key, it
  creates a new semphore for that key and sets its resource count to
  \a initialValue.

  In Windows, \a mode is ignored, and the system always tries to
  create a semaphore for the specified \a key. If the system does not
  already have a semaphore identified as \a key, it creates the
  semaphore and sets its resource count to \a initialValue. But if the
  system already has a semaphore identified as \a key it uses that
  semaphore and ignores \a initialValue.

  The \l {QSystemSemaphore::AccessMode} {mode} parameter is only used
  in Unix systems to handle the case where a semaphore survives a
  process crash. In that case, the next process to allocate a
  semaphore with the same \a key will get the semaphore that survived
  the crash, and unless \a mode is \l {QSystemSempahore::} {Create},
  the resource count will not be reset to \a initialValue but will
  retain the initial value it had been given by the crashed process.
  
  \sa acquire(), key()
 */
QSystemSemaphore::QSystemSemaphore(const QString &key, int initialValue, AccessMode mode)
{
    d = new QSystemSemaphorePrivate;
    setKey(key, initialValue, mode);
}

/*!
  The destructor destroys the QSystemSemaphore object, but the
  underlying system semaphore is not deallocated and removed from the
  system unless this QSystemSemaphore is the last one using the
  underlying system semaphore.

  Two important side effects of the destructor depend on the system.
  In Windows, if acquire() has been called for this semaphore but not
  release(), release() will not be called by the destructor, nor will
  the resource be released when the process exits normally. This would
  be a program bug which could be the cause of a deadlock in another
  process trying to acquire the same resource. In Unix, acquired
  resources that are not released before the destructor is called are
  automatically released when the process exits.
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


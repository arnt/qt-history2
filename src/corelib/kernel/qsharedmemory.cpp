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

#include "qsharedmemory.h"
#include "qsharedmemory_p.h"
#include "qsystemsemaphore.h"
#include <qdir.h>
#include <qdebug.h>

/*!
    \internal

    Generate a string from the key which can be any unicode string into
    the subset that the win/unix kernel allows.

    On Unix this will be a file name
 */
QString QSharedMemoryPrivate::makePlatformSafeKey(const QString &key, const QString &prefix)
{
    if (key.isEmpty())
        return QString();

    QString allowableKey;
    for (int i = 0; i < key.count(); ++i)
        allowableKey += QString::number(key.at(i).unicode());
    QString strippedKey = key;
    strippedKey.replace(QRegExp(QLatin1String("[^A-Za-z]")), QString());

#ifdef Q_OS_WIN
    return prefix + strippedKey + allowableKey;
#else
    return QDir::tempPath() + QLatin1Char('/') + prefix + strippedKey + allowableKey;
#endif
}

/*!
  \class QSharedMemory
  \since 4.4

  \brief The QSharedMemory class provides access to a shared memory segment.

  QSharedMemory provides access to a shared memory segment by multiple
  threads and processes. It also provides a way for a single thread or
  process to lock the memory for exclusive access.

  When using this class, be aware of the following platform
  differences:

  \list

  \o Windows: QSharedMemory does not "own" the shared memory segment.
  When all threads or processes that have an instance of QSharedMemory
  attached to a particular shared memory segment have either destroyed
  their instance of QSharedMemory or exited, the Windows kernel
  releases the shared memory segment automatically.

  \o Unix: QSharedMemory "owns" the shared memory segment. When the
  last thread or process that has an instance of QSharedMemory
  attached to a particular shared memory segment detaches from the
  segment by destroying its instance of QSharedMemory, the Unix kernel
  release the shared memory segment. But if that last thread or
  process crashes without running the QSharedMemory destructor, the
  shared memory segment survives the crash.

  \o HP-UX: Only one attach to a shared memory segment is allowed per
  process. This means that QSharedMemeory should not be used across
  multiple threads in the same process in HP-UX.

  \endlist

  ### Add code snippit after example is made.

  Unlike QtSharedMemory, QSharedMemory will automatically destroy the
  shared memory once there are no references to the shared memory.  It
  is advisable to not mix QtSharedMemory and QSharedMemory, but to
  port everything to QSharedMemory.

 */


/*!
    \overload

    Constructs an shared memory with a given \a parent.
    The key is not set and must be set before create() or attach() are used.

    \sa setKey()
 */
QSharedMemory::QSharedMemory(QObject *parent) : QObject(*new QSharedMemoryPrivate, parent)
{
}

/*!
    Constructs a shared memory module with a given \a parent
    where \a key is set.

    \sa setKey(), create(), attach()
 */
QSharedMemory::QSharedMemory(const QString &key, QObject *parent)
    : QObject(*new QSharedMemoryPrivate, parent)
{
    setKey(key);
}

/*!
    Destructor will detach if it is currently attached.
    If there are no more attachments the memory segment will be destroyed.

    \sa detach() isAttached()
 */
QSharedMemory::~QSharedMemory()
{
    setKey(QString());
}

/*!
    Sets a new \a key to this shared memory.  If the shared memory is
    attached it will detach before setting the new key.

    \sa key() isAttached()
 */
void QSharedMemory::setKey(const QString &key)
{
    Q_D(QSharedMemory);
    if (key == d->key)
        return;

    if (isAttached())
        detach();
    d->cleanHandle();
    d->key = key;
    d->systemSemaphore.setKey(key, 1);
    d->errorString = QString();
    d->error = QSharedMemory::NoError;
}

/*!
    Returns the key assigned to this shared memory.

    \sa setKey()
 */
QString QSharedMemory::key() const
{
    Q_D(const QSharedMemory);
    return d->key;
}

/*!
    Creates a shared memory segment of \a size bytes and then attaches
    to the shared memory segment with access mode \a mode.  If the memory
    segment already exists create will return false and not be attached.

    Returns true on success; otherwise returns false.

    \sa error()
 */
bool QSharedMemory::create(int size, AccessMode mode)
{
    Q_D(QSharedMemory);
    QSharedMemoryLocker lock(this);
    QString function = QLatin1String("QSharedMemory::create");
    if (!d->tryLocker(&lock, function))
        return false;

    if (size <= 0) {
        d->error = QSharedMemory::InvalidSize;
        d->errorString = QSharedMemory::tr("%1: create size is less then 0").arg(function);
        return false;
    }

    if (!d->create(size))
        return false;

    return d->attach(mode);
}

/*!
    Returns the size of the shared memory if attached; otherwise 0.

    \sa create() attach()
 */
int QSharedMemory::size() const
{
    Q_D(const QSharedMemory);
    return d->size;
}

/*!
    \enum QSharedMemory::AccessMode

    \value ReadOnly The shared memory can only be read from.  If an attempt is
                    made to write to the memory the program will segfault.
    \value ReadWrite The shared memory can be read from and written to.
*/

/*!
    Attempts to attach to the shared memory segment with the current key
    and access mode \a mode.  Returns true on success; otherwise returns false.
    After attaching the shared memory can be accessed by data().

    \sa isAttached(), detach(), create()
 */
bool QSharedMemory::attach(AccessMode mode)
{
    Q_D(QSharedMemory);
    QSharedMemoryLocker lock(this);
    if (!d->tryLocker(&lock, QLatin1String("QSharedMemory::attach")))
        return false;

    if (isAttached() || !d->handle())
        return false;

    return d->attach(mode);
}

/*!
    Returns true if attached to the shared memory segment otherwise false.

    \sa attach(), detach()
 */
bool QSharedMemory::isAttached() const
{
    Q_D(const QSharedMemory);
    return (0 != d->memory);
}

/*!
    Detaches from the shared memory.  If there is no one attached to
    the memory segment then memory is destroyed.
    Returns true on success; otherwise returns false.

    \sa attach(), isAttached()
 */
bool QSharedMemory::detach()
{
    Q_D(QSharedMemory);
    if (!isAttached())
        return false;

    QSharedMemoryLocker lock(this);
    if (!d->tryLocker(&lock, QLatin1String("QSharedMemory::detach")))
        return false;

    return d->detach();
}

/*!
    Returns a pointer to the shared memory if attached otherwise 0.

    \sa attach() create()
  */
void *QSharedMemory::data()
{
    Q_D(QSharedMemory);
    return d->memory;
}

/*!
    This is an overloaded member function, provided for convenience.
*/
const void* QSharedMemory::constData() const
{
    Q_D(const QSharedMemory);
    return d->memory;
}

/*!
    This is an overloaded member function, provided for convenience.
*/
const void *QSharedMemory::data() const
{
    Q_D(const QSharedMemory);
    return d->memory;
}

/*!
    This will lock the memory segment.  If another process has locked the
    memory segment lock() will block until it is unlocked.
    Returns true on success; otherwise returns false.

    \sa unlock(), data()
 */
bool QSharedMemory::lock()
{
    Q_D(QSharedMemory);
    if (d->systemSemaphore.acquire()) {
        d->lockedByMe = true;
        return true;
    }
    QString function = QLatin1String("QSharedMemory::lock");
    d->errorString = QSharedMemory::tr("%1: unable to lock").arg(function);
    d->error = QSharedMemory::LockError;
    return false;
}

/*!
    Unlocks the memory segment.
    Returns true on success; otherwise returns false.

    \sa lock()
 */
bool QSharedMemory::unlock()
{
    Q_D(QSharedMemory);
    if (!d->lockedByMe)
        return false;
    d->lockedByMe = false;
    if (d->systemSemaphore.release())
        return true;
    QString function = QLatin1String("QSharedMemory::unlock");
    d->errorString = QSharedMemory::tr("%1: unable to unlock").arg(function);
    d->error = QSharedMemory::LockError;
    return false;
}

/*! \enum QSharedMemory::SharedMemoryError

    \value NoError No error occurred.

    \value PermissionDenied Permission for doing the operation has been denied.

    \value InvalidSize Unable to create a shared memory of because of the size.

    \value KeyError Error with the shared memory key.

    \value AlreadyExists Error creating shared memory because it already existed.

    \value NotFound Unable to attach to shared memory because it doesn't exists.

    \value LockError Error because it was unable to get a lock

    \value OutOfResources The operating system ran out of a resource.

    \value UnknownError Something else happened.. :)
*/

/*!
    Returns the type of error that occurred last or NoError.

    \sa errorString()
 */
QSharedMemory::SharedMemoryError QSharedMemory::error() const
{
    Q_D(const QSharedMemory);
    return d->error;
}

/*!
    Returns the human-readable message appropriate to the current error
    reported by error(). If no suitable string is available, an empty
    string is returned.

    \sa error()
 */
QString QSharedMemory::errorString() const
{
    Q_D(const QSharedMemory);
    return d->errorString;
}


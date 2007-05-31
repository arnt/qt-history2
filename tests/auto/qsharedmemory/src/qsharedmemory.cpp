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

/*! \class QSharedMemory

    \brief The QSharedMemory class provides a shared memory segment.

    QSharedMemory provides a way for multiple threads and processes to
    access the same memory on a system.  QSharedMemory provides a way
    to lock the memory so it can be accessed by one process or thread
    at a time.

    There are some platform difference that should be known when using this class:

    * On Windows once all of the QSharedMemory have been destroyed or the processes
      exits the shared memory is automatically removed.
    * On Unix the last QSharedMemory to detach will remove the shared memory
      so if there is one QSharedMemory that is attached and the program crashes or
      the QSharedMemory is not destroyed the shared memory is not removed from
      the system.
    * On HP-UX you can only attach once to a shared memory per process.

    ### Add code snippit after example is made.

 */


/*!
    \overload

    Constructs an shared memory where the key is not set.
    The key must be set before create() or attach() are used.

    \sa setKey()
 */
QSharedMemory::QSharedMemory(QObject *parent) : QObject(*new QSharedMemoryPrivate, parent)
{
}

/*!
    Constructs a shared memory module where key is set.

    \sa setKey(), create(), attach()
 */
QSharedMemory::QSharedMemory(const QString &key, QObject *parent) : QObject(*new QSharedMemoryPrivate, parent)
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
    Sets a new key to this shared memory.  If the shared memory is
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
    to the shared memory segment.  If the memory segment already exists
    create will return false and not be attached.

    Returns true on success; otherwise returns false.

    \sa error()
 */
bool QSharedMemory::create(int size, OpenMode mode)
{
    Q_D(QSharedMemory);
    QSharedMemoryLocker lock(this);
    if (!d->checkLocker(&lock, QLatin1String("QSharedMemory::create")))
        return false;

    if (size <= 0) {
        d->error = QSharedMemory::InvalidSize;
        d->errorString = QLatin1String("QSharedMemory::create: ") + tr("create size is less then 0");
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
    \enum QSharedMemory::OpenMode

    \value ReadOnly The shared memory can only be read from.  If an attempt is
                    made to write to the memory the program will segfault.
    \value ReadWrite The shared memory can be read from and written to.
*/

/*!
    Attempts to attach to the shared memory segment with the current key.
    Returns true on success; otherwise returns false.  After attaching the
    shared memory can be accessed by data().

    \sa isAttached(), detach(), create()
 */
bool QSharedMemory::attach(OpenMode mode)
{
    Q_D(QSharedMemory);
    QSharedMemoryLocker lock(this);
    if (!d->checkLocker(&lock, QLatin1String("QSharedMemory::attach")))
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
    if (!d->checkLocker(&lock, QLatin1String("QSharedMemory::detach")))
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

    \sa unlock(), data()
 */
bool QSharedMemory::lock()
{
    Q_D(QSharedMemory);
    if (d->systemSemaphore.acquire()) {
        d->lockedByMe = true;
        return true;
    }
    return false;
}

/*!
    Unlocks the memory segment.

    \sa lock()
 */
bool QSharedMemory::unlock()
{
    Q_D(QSharedMemory);
    if (!d->lockedByMe)
        return false;
    d->lockedByMe = false;
    return d->systemSemaphore.release();
}

/*! \enum QtSharedMemory::Error

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


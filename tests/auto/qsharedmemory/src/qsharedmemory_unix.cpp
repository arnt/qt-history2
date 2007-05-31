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
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

QSharedMemoryPrivate::QSharedMemoryPrivate() : QObjectPrivate(),
        memory(0), size(0), error(QSharedMemory::NoError), systemSemaphore(QString()), lockedByMe(false), unix_key(0)
{
}

void QSharedMemoryPrivate::setErrorString(const QString &function)
{
    Q_Q(QSharedMemory);
    // EINVAL is handled in functions so they can give better error strings
    switch (errno) {
    case EACCES:
        errorString = function + QLatin1String(": ") + q->tr("permission denied");
        error = QSharedMemory::PermissionDenied;
        break;
    case EEXIST:
        errorString = function + QLatin1String(": ") + q->tr("already exists");
        error = QSharedMemory::AlreadyExists;
        break;
    case ENOENT:
        errorString = function + QLatin1String(": ") + q->tr("doesn't exists");
        error = QSharedMemory::NotFound;
        break;
    case EMFILE:
    case ENOMEM:
    case ENOSPC:
        errorString = function + QLatin1String(": ") + q->tr("out of resources");
        error = QSharedMemory::OutOfResources;
        break;
    default:
        errorString = function + QLatin1String(": ") + q->tr("unknown error")
            + QLatin1Char(' ') + errno;
        error = QSharedMemory::UnknownError;
#if defined QSHAREDMEMORY_DEBUG
        qDebug() << errorString << "key" << key << "errno" << errno << EINVAL;
#endif
    }
}

/*!
    \internal

    If not already made create the handle used for accessing the shared memory.
*/
key_t QSharedMemoryPrivate::handle()
{
    Q_Q(QSharedMemory);
    // already made
    if (unix_key)
        return unix_key;

    // don't allow making handles on empty keys
    if (key.isEmpty()) {
        errorString = QLatin1String("QSharedMemory::handle: ") + q->tr("key is empty");
        error = QSharedMemory::KeyError;
        return -1;
    }

    // ftok requires that an actual file exists somewhere
    QString fileName = makePlatformSafeKey(key);
    if (!QFile::exists(fileName)) {
        errorString = QLatin1String("QSharedMemory::handle: ") + q->tr("unix key file doesn't exists");
        error = QSharedMemory::NotFound;
        return -1;
    }

    unix_key = ftok(QFile::encodeName(fileName).constData(), 'Q');
    if (-1 == unix_key) {
        errorString = QLatin1String("QSharedMemory::handle: ") + q->tr("ftok failed");
        error = QSharedMemory::KeyError;
        unix_key = 0;
    }
    return unix_key;
}

/*!
    \internal
    Creates the unix file if needed.
    returns true if the unix file was created.

    -1 error
     0 already existed
     1 created
  */
int QSharedMemoryPrivate::createUnixKeyFile(const QString &fileName)
{
    if (QFile::exists(fileName))
        return 0;

    int fd = open(QFile::encodeName(fileName).constData(), O_EXCL | O_CREAT | O_RDWR, 0660);
    if (-1 == fd) {
        if (errno == EEXIST)
            return 0;
        return -1;
    } else {
        close(fd);
    }
    return 1;
}

bool QSharedMemoryPrivate::cleanHandle()
{
    unix_key = 0;
    return true;
}

bool QSharedMemoryPrivate::create(int size)
{
    Q_Q(QSharedMemory);
    // build file if needed
    bool createdFile = false;
    int built = createUnixKeyFile(makePlatformSafeKey(key));
    if (built == -1) {
        errorString = QLatin1String("QSharedMemory::handle: ") + q->tr("unable to make key");
        error = QSharedMemory::KeyError;
        return false;
    }
    if (built == 1) {
        createdFile = true;
    }

    // get handle
    if (!handle()) {
        if (createdFile)
            QFile::remove(makePlatformSafeKey(key));
        return false;
    }

    // create
    if (-1 == shmget(handle(), size, 0666 | IPC_CREAT | IPC_EXCL)) {
        QString function = QLatin1String("QSharedMemory::create");
        switch (errno) {
        case EINVAL:
            errorString = function + QLatin1String(": ") + q->tr("system-imposed size restrictions");
            error = QSharedMemory::InvalidSize;
            break;
        default:
            setErrorString(function);
        }
        if (createdFile && error != QSharedMemory::AlreadyExists)
            QFile::remove(makePlatformSafeKey(key));
        return false;
    }

    // Take ownership and force set initialValue because the semaphore
    // might have already existed from a previous crash.
    systemSemaphore.setKey(key, 1, QSystemSemaphore::Create);
    systemSemaphore.acquire();

    return true;
}

bool QSharedMemoryPrivate::attach(QSharedMemory::OpenMode mode)
{
    // grab the shared memory segment id
    int id = shmget(handle(), 0, (mode == QSharedMemory::ReadOnly ? 0444 : 0660));
    if (-1 == id) {
        setErrorString(QLatin1String("QSharedMemory::attach"));
        return false;
    }

    // grab the memory
    memory = shmat(id, 0, (mode == QSharedMemory::ReadOnly ? SHM_RDONLY : 0));
    if ((void*) - 1 == memory) {
        memory = 0;
        setErrorString(QLatin1String("QSharedMemory::attach"));
        return false;
    }

    // grab the size
    shmid_ds shmid_ds;
    if (!shmctl(id, IPC_STAT, &shmid_ds)) {
        size = (int)shmid_ds.shm_segsz;
    } else {
        setErrorString(QLatin1String("QSharedMemory::attach"));
        return false;
    }

    return true;
}

bool QSharedMemoryPrivate::detach()
{
    Q_Q(QSharedMemory);
    // detach from the memory segment
    if (-1 == shmdt(memory)) {
        QString function = QLatin1String("QSharedMemory::detach");
        switch (errno) {
        case EINVAL:
            errorString = function + QLatin1String(": ") + q->tr("not attached");
            error = QSharedMemory::NotFound;
            break;
        default:
            setErrorString(function);
        }
        return false;
    }
    memory = 0;

    // Get the number of current attachments
    int id = shmget(handle(), 0, 0444);
    struct shmid_ds shmid_ds;
    if (0 != shmctl(id, IPC_STAT, &shmid_ds)) {
        setErrorString(QLatin1String("QSharedMemory::detach shmget"));
        return false;
    }
    // If there are no attachments then remove it.
    if (shmid_ds.shm_nattch == 0) {
        // mark for removal
        struct shmid_ds shmid_ds;
        if (-1 == shmctl(id, IPC_RMID, &shmid_ds)) {
            setErrorString(QLatin1String("QSharedMemory::remove"));
            return false;
        }

        // remove file
        if (!QFile::remove(makePlatformSafeKey(key)))
            return false;
    }
    return true;
}


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

#include <qdebug.h>
#include <qfile.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/shm.h>

#include <sys/sem.h>
// We have to define this as on some sem.h will have it
union qt_semun {
    int val;                    /* value for SETVAL */
    struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array;      /* array for GETALL, SETALL */
};

QSystemSemaphorePrivate::QSystemSemaphorePrivate() :
        semaphore(-1), createdFile(false), createdSemaphore(false), unix_key(-1)
{
}

/*!
    \internal

    Setup unix_key
 */
key_t QSystemSemaphorePrivate::handle(QSystemSemaphore::AccessMode mode)
{
    if (key.isEmpty())
        return -1;

    // ftok requires that an actual file exists somewhere
    if (-1 != unix_key)
        return unix_key;

    // Create the file needed for ftok
    int built = QSharedMemoryPrivate::createUnixKeyFile(fileName);
    if (-1 == built)
        return -1;
    createdFile = (1 == built);

    // Get the unix key for the created file
    unix_key = ftok(QFile::encodeName(fileName).constData(), 'Q');
    if (-1 == unix_key) {
        return -1;
    }

    // Get semaphore
    semaphore = semget(unix_key, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (-1 == semaphore) {
        if (errno == EEXIST)
            semaphore = semget(unix_key, 1, 0666 | IPC_CREAT);
        if (-1 == semaphore) {
            cleanHandle();
            return -1;
        }
    } else {
        createdSemaphore = true;
        // Force cleanup of file, it is possible that it can be left over from a crash
        createdFile = true;
    }

    if (mode == QSystemSemaphore::Create) {
        createdSemaphore = true;
        createdFile = true;
    }

    // Created semaphore so initialize its value.
    if (createdSemaphore && initialValue >= 0) {
        qt_semun init_op;
        init_op.val = initialValue;
        if (-1 == semctl(semaphore, 0, SETVAL, init_op)) {
            cleanHandle();
            return -1;
        }
    }

    return unix_key;
}

/*!
    \internal

    Cleanup the unix_key
 */
void QSystemSemaphorePrivate::cleanHandle()
{
    unix_key = -1;

    // remove the file if we made it
    if (createdFile) {
        QFile::remove(fileName);
        createdFile = false;
    }

    if (createdSemaphore) {
        if (-1 != semaphore) {
            if (-1 == semctl(semaphore, 0, IPC_RMID)) {
#if defined QSYSTEMSEMAPHORE_DEBUG
                qDebug() << QLatin1String("QSystemSemaphore::cleanHandle semctl failed.");
#endif
            }
            semaphore = -1;
        }
        createdSemaphore = false;
    }
}

/*!
    \internal
 */
bool QSystemSemaphorePrivate::modifySemaphore(int count)
{
    if (-1 == handle())
        return false;

    struct sembuf operation;
    operation.sem_num = 0;
    operation.sem_op = count;
    operation.sem_flg = SEM_UNDO;
    if (-1 == semop(semaphore, &operation, 1)) {
        // If the semaphore was removed be nice and create it and then modifySemaphore again
        if (errno == EINVAL || errno == EIDRM) {
            semaphore = -1;
            cleanHandle();
            handle();
            return modifySemaphore(count);
        }
#if defined QSYSTEMSEMAPHORE_DEBUG
        qDebug() << QLatin1String("QSystemSempahore::modify failed") << count << semctl(semaphore, 0, GETVAL) << errno << EIDRM << EINVAL;
#endif
        return false;
    }

    return true;
}


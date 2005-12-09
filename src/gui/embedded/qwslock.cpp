#include "qwslock_p.h"

#include <qglobal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifdef QT_NO_SEMAPHORE
#error QWSLock currently requires semaphores
#endif

QWSLock::QWSLock()
{
    semId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    if (semId == -1) {
        perror("QWSLock::QWSLock");
        qFatal("Unable to create semaphore");
    }

    if (semctl(semId, 0, SETVAL, 1) == -1) {
        perror("QWSLock::QWSLock");
        qFatal("Unable to initialize semaphore");
    }
}

QWSLock::QWSLock(int id)
{
    semId = id;
}

QWSLock::~QWSLock()
{
    if (semId == -1)
        return;
    semctl(semId, 0, IPC_RMID, 0);
}

bool QWSLock::lock(int /*timeout*/)
{
    int ret;
    do {
        sembuf sops = { 0, -1, 0 };
        ret = semop(semId, &sops, 1);
        if (ret == -1 && errno != EINTR)
            qDebug("QWSLock::lock: %s", strerror(errno));
    } while (ret == -1 && errno == EINTR);

    return (ret != -1);
}

bool QWSLock::isLocked()
{
    int ret;
    do {
        ret = semctl(semId, 0, GETVAL);
        if (ret == -1 && errno != EINTR)
            qDebug("QWSLock::isLocked: %s", strerror(errno));
    } while (ret == -1 && errno == EINTR);

    return (ret == 0);
}

void QWSLock::unlock()
{
#ifdef QT_DEBUG
    if (!isLocked())
        qWarning("QWSLock::unlock(): semaphore %d not locked", semId);
#endif
    int ret;
    do {
        sembuf sops = {0, 1, 0};
        ret = semop(semId, &sops, 1);
        if (ret == -1 && errno != EINTR)
            qDebug("QWSLock::unlock: %s", strerror(errno));
    } while (ret == -1 && errno == EINTR);
}

void QWSLock::wait()
{
    bool ok = lock();
    if (ok)
        unlock();
}


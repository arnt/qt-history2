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

#ifndef QSYSTEMLOCK_P_H
#define QSYSTEMLOCK_P_H

#ifndef QT_NO_SYSTEMLOCK

#include "qsystemlock.h"
#include "private/qsharedmemory_p.h"

#define MAX_LOCKS 64

class QSystemLockPrivate
{

public:
    QSystemLockPrivate();

    QString makeKeyFileName()
    {
        return QSharedMemoryPrivate::makePlatformSafeKey(key, QLatin1String("qipc_systemlock_"));
    }

    void setErrorString(const QString &function);

#ifdef Q_OS_WIN
    HANDLE handle();
    bool lock(HANDLE, int count);
    bool unlock(HANDLE, int count);
#else
    key_t handle();
#endif
    void cleanHandle();

    enum Operation {
        Lock,
        Unlock
    };
    bool modifySemaphore(Operation op, QSystemLock::LockMode mode = QSystemLock::ReadOnly);

    QString key;
    QString fileName;
#ifdef Q_OS_WIN
    HANDLE semaphore;
    HANDLE semaphoreLock;
#else
    int semaphore;
#endif
    int lockCount;
    QSystemLock::LockMode lockedMode;

    QSystemLock::SystemLockError error;
    QString errorString;

private:
#ifndef Q_OS_WIN
    key_t unix_key;
    bool createdFile;
    bool createdSemaphore;
#endif
};

#endif // QT_NO_SYSTEMLOCK

#endif // QSYSTEMLOCK_P_H


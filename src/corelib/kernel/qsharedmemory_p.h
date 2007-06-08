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

#ifndef QSHAREDMEMORY_P_H
#define QSHAREDMEMORY_P_H

#ifndef QT_NO_SHAREDMEMORY

#include "qsharedmemory.h"
#include "qsystemsemaphore.h"
#include "private/qobject_p.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <sys/sem.h>
#endif

/*!
  Helper class
  */
class QSharedMemoryLocker
{

public:
    inline QSharedMemoryLocker(QSharedMemory *sharedMemory) : q_sm(sharedMemory)
    {
        Q_ASSERT(q_sm);
    }

    inline ~QSharedMemoryLocker()
    {
        if (q_sm)
            q_sm->unlock();
    }

    inline bool lock()
    {
        if (q_sm && q_sm->lock())
            return true;
        q_sm = 0;
        return false;
    }

private:
    QSharedMemory *q_sm;
};

class Q_AUTOTEST_EXPORT QSharedMemoryPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSharedMemory)

public:
    QSharedMemoryPrivate();

    void *memory;
    int size;
    QString key;
    QSharedMemory::SharedMemoryError error;
    QString errorString;
    QSystemSemaphore systemSemaphore;
    bool lockedByMe;

    static int createUnixKeyFile(const QString &fileName);
    static QString makePlatformSafeKey(const QString &key,
            const QString &prefix = QLatin1String("qipc_sharedmemory_"));
#ifdef Q_OS_WIN
    HANDLE handle();
#else
    key_t handle();
#endif
    bool cleanHandle();
    bool create(int size);
    bool attach(QSharedMemory::AccessMode mode);
    bool detach();

    void setErrorString(const QString &function);

    bool tryLocker(QSharedMemoryLocker *locker, const QString function) {
        if (!locker->lock()) {
            errorString = QSharedMemory::tr("%1: unable to lock").arg(function);
            error = QSharedMemory::LockError;
            return false;
        }
        return true;
    }

private:
#ifdef Q_OS_WIN
    HANDLE hand;
#else
    key_t unix_key;
#endif
};

#endif // QT_NO_SHAREDMEMORY

#endif // QSHAREDMEMORY_P_H


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

#include "qsystemlock.h"
#include "qsystemlock_p.h"
#include <qdebug.h>
#include <QtCore>
QSystemLockPrivate::QSystemLockPrivate() :
        semaphore(0), semaphoreLock(0),
	lockCount(0), error(QSystemLock::NoError)
{
}

void QSystemLockPrivate::setErrorString(const QString &function)
{
    BOOL windowsError = GetLastError();
    if (windowsError == 0)
        return;
    errorString = function + QLatin1String(": ")
                    + QLatin1String("Unknown error");
    error = QSystemLock::UnknownError;
    qWarning() << errorString << "key" << key << (int)windowsError << semaphore << semaphoreLock;
}

/*!
    \internal

    Setup the semaphore
 */
HANDLE QSystemLockPrivate::handle()
{
    // don't allow making handles on empty keys
    if (key.isEmpty())
        return 0;

    // Create it if it doesn't already exists.
    if (semaphore == 0) {
        QString safeName = makeKeyFileName();
        QT_WA({
            semaphore = CreateSemaphoreW(0, MAX_LOCKS, MAX_LOCKS, (TCHAR*)safeName.utf16());
        }, {
            semaphore = CreateSemaphoreA(0, MAX_LOCKS, MAX_LOCKS, safeName.toLocal8Bit().constData());
        });

        if (semaphore == 0) {
            setErrorString(QLatin1String("QSystemLockPrivate::handle"));
	    return 0;
	}
    }

    if (semaphoreLock == 0) {
	QString safeLockName = QSharedMemoryPrivate::makePlatformSafeKey(key + QLatin1String("lock"), QLatin1String("qipc_systemlock_"));
        QT_WA({
            semaphoreLock = CreateSemaphoreW(0,
                 1, 1, (TCHAR*)safeLockName.utf16());
        }, {
            semaphoreLock = CreateSemaphoreA(0,
                 1, 1, safeLockName.toLocal8Bit().constData());
        });
        if (semaphoreLock == 0) {
            setErrorString(QLatin1String("QSystemLockPrivate::handle"));
	    return 0;
	}
    }
    return semaphore;
}

/*!
    \internal

    Cleanup the semaphore
 */
void QSystemLockPrivate::cleanHandle()
{
    if (semaphore && !CloseHandle(semaphore))
        setErrorString(QLatin1String("QSystemLockPrivate::cleanHandle:"));
    if (semaphoreLock && !CloseHandle(semaphoreLock))
        setErrorString(QLatin1String("QSystemLockPrivate::cleanHandle:"));
    semaphore = 0;
    semaphoreLock = 0;
}

bool QSystemLockPrivate::lock(HANDLE handle, int count)
{
    if (count == 1) {
	WaitForSingleObject(handle, INFINITE);
	return true;
    }

    int i = count;
    while (i > 0) {
	if (WAIT_OBJECT_0 == WaitForSingleObject(handle, 0)) {
	    --i;
	} else {
	    // undo what we have done, sleep and then try again later
	    ReleaseSemaphore(handle, (count - i), 0);
	    i = count;
	    ReleaseSemaphore(semaphoreLock, 1, 0);
	    Sleep(1);
	    WaitForSingleObject(semaphoreLock, INFINITE);
	}
    }
    return true;
}

bool QSystemLockPrivate::unlock(HANDLE handle, int count)
{
    if (0 == ReleaseSemaphore(handle, count, 0)) {
        setErrorString(QLatin1String("QSystemLockPrivate::unlock"));
        return false;
    }
    return true;
}

/*!
    \internal

    modifySemaphore handles recursive behavior and modifies the semaphore.
 */
bool QSystemLockPrivate::modifySemaphore(QSystemLockPrivate::Operation op,
        QSystemLock::LockMode mode)
{
    if (0 == handle())
        return false;

    if ((lockCount == 0 && op == Lock) || (lockCount > 0 && op == Unlock)) {
        if (op == Unlock) {
            --lockCount;
            Q_ASSERT(lockCount >= 0);
            if (lockCount > 0)
                return true;
        }

        int count = (mode == QSystemLock::ReadWrite) ? MAX_LOCKS : 1;
        if (op == Lock) {
            lock(semaphoreLock, 1);
            lock(semaphore, count);
            if (count != MAX_LOCKS) unlock(semaphoreLock, 1);
	    lockedMode = mode;
        } else {
            if (count == MAX_LOCKS) unlock(semaphoreLock, 1);
	    unlock(semaphore, count);
        }

    }
    if (op == Lock)
        lockCount++;

    return true;
}


/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QREADWRITELOCK_H
#define QREADWRITELOCK_H

#include <qglobal.h>
#include <limits.h>

struct QReadWriteLockPrivate;

class Q_CORE_EXPORT QReadWriteLock
{
public:
    explicit QReadWriteLock(const int maxReaders=INT_MAX);
    ~QReadWriteLock();

    void lockForRead();
    bool tryLockForRead();

    void lockForWrite();
    bool tryLockForWrite();

    void unlock();

private:
    Q_DISABLE_COPY(QReadWriteLock)
    QReadWriteLockPrivate *d;
};

class Q_CORE_EXPORT QReadLocker
{
public:
    inline QReadLocker(QReadWriteLock *readWriteLock)
        : lock(readWriteLock)
    { relock(); }
    inline ~QReadLocker()
    { unlock(); }

    inline void unlock()
    { if (lock) lock->unlock(); }

    inline void relock()
    { if (lock) lock->lockForRead(); }

private:
    Q_DISABLE_COPY(QReadLocker)
    QReadWriteLock *lock;
};

class Q_CORE_EXPORT QWriteLocker
{
public:
    inline QWriteLocker(QReadWriteLock *readWriteLock)
        : lock(readWriteLock)
    { relock(); }
    inline ~QWriteLocker()
    { unlock(); }

    inline void unlock()
    { if (lock) lock->unlock(); }

    inline void relock()
    { if (lock) lock->lockForWrite(); }

private:
    Q_DISABLE_COPY(QWriteLocker)
    QReadWriteLock *lock;
};

#endif

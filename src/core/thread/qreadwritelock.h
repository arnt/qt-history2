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

#include <QtCore/qglobal.h>
#include <limits.h>

struct QReadWriteLockPrivate;

class Q_CORE_EXPORT QReadWriteLock
{
public:
    QReadWriteLock();
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
        : q_lock(readWriteLock)
    { relock(); }
    inline ~QReadLocker()
    { unlock(); }

    inline void unlock()
    { if (q_lock) q_lock->unlock(); }

    inline void relock()
    { if (q_lock) q_lock->lockForRead(); }

    inline QReadWriteLock *readWriteLock() const
    { return q_lock; }

private:
    Q_DISABLE_COPY(QReadLocker)
    QReadWriteLock *q_lock;
};

class Q_CORE_EXPORT QWriteLocker
{
public:
    inline QWriteLocker(QReadWriteLock *readWriteLock)
        : q_lock(readWriteLock)
    { relock(); }
    inline ~QWriteLocker()
    { unlock(); }

    inline void unlock()
    { if (q_lock) q_lock->unlock(); }

    inline void relock()
    { if (q_lock) q_lock->lockForWrite(); }

    inline QReadWriteLock *readWriteLock() const
    { return q_lock; }

private:
    Q_DISABLE_COPY(QWriteLocker)
    QReadWriteLock *q_lock;
};

#endif // QREADWRITELOCK_H

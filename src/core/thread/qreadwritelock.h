#ifndef QREADWRITELOCK_H
#define QREADWRITELOCK_H

#include <qglobal.h>

struct QReadWriteLockPrivate;

class Q_CORE_EXPORT QReadWriteLock
{
public:
     enum AccessMode {
        ReadAccess = 0,
        WriteAccess
    };

    explicit QReadWriteLock(const int maxReaders=INT_MAX);
    ~QReadWriteLock();

    void lock(AccessMode mode);
    bool tryLock(AccessMode mode);
    void unlock();
private:
    Q_DISABLE_COPY(QReadWriteLock)

    QReadWriteLockPrivate * d;
};

class Q_CORE_EXPORT QReadWriteLockLocker
{
public:
    inline QReadWriteLockLocker(QReadWriteLock *readWriteLock,
                                QReadWriteLock::AccessMode accessMode)
        : lock(readWriteLock), access(accessMode)
    { relock(); }
    inline ~QReadWriteLockLocker()
    { unlock(); }

    inline void unlock()
    { if (lock) lock->unlock(); }

    inline void relock()
    { if (lock) lock->lock(access); }

private:
    Q_DISABLE_COPY(QReadWriteLockLocker)

    QReadWriteLock *lock;
    QReadWriteLock::AccessMode access;
};

#endif


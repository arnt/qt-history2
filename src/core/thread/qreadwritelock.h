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

#endif


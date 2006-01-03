#ifndef QWSLOCK_H
#define QWSLOCK_H

class QWSLock
{
public:
    enum LockType { BackingStore, Communication };

    QWSLock();
    QWSLock(int lockId);
    ~QWSLock();

    bool lock(LockType type, int timeout = -1);
    void unlock(LockType type);
    bool wait(LockType type, int timeout = -1);
    int id() const { return semId; }

private:
    int semId;
    int lockCount[2];

    bool hasLock(LockType type);
};

#endif // QWSLOCK_H

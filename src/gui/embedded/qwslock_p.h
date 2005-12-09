#ifndef QWSLOCK_H
#define QWSLOCK_H

class QWSLock
{
public:
    QWSLock();
    QWSLock(int id);
    ~QWSLock();
    bool lock(int timeout = -1);
    bool isLocked();
    void unlock();
    void wait();
    int id() { return semId; }

private:
    int semId;
};

#endif // QWSLOCK_H

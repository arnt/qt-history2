#include <QMutex>

#include "threads.h"

void MyThread::run()
{
}

#define Counter ReentrantCounter

class Counter
{
public:
    Counter() { n = 0; }

    void increment() { ++n; }
    void decrement() { --n; }
    int value() const { return n; }

private:
    int n;
};

#undef Counter
#define Counter ThreadSafeCounter

class Counter
{
public:
    Counter() { n = 0; }

    void increment() { QMutexLocker locker(&mutex); ++n; }
    void decrement() { QMutexLocker locker(&mutex); --n; }
    int value() const { QMutexLocker locker(&mutex); return n; }

private:
    mutable QMutex mutex;
    int n;
};

int main()
{
    return 0;
}

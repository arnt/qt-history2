#include <QCache>
#include <QMutex>
#include <QThreadStorage>

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

typedef int SomeClass;

QThreadStorage<QCache<QString, SomeClass> *> caches;

void cacheObject(const QString &key, SomeClass *object)
{
    if (!caches.hasLocalData())
        caches.setLocalData(new QCache<QString, SomeClass>);

    caches.localData()->insert(key, object);
}

void removeFromCache(const QString &key)
{
    if (!caches.hasLocalData())
        return;

    caches.localData()->remove(key);
}

int main()
{
    return 0;
}

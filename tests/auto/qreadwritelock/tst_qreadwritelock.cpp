/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qcoreapplication.h>


#include <qreadwritelock.h>
#include <qmutex.h>
#include <qthread.h>
#include <qwaitcondition.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#ifdef Q_OS_WIN32
#include <windows.h>
#define sleep(X) Sleep(X)
#endif

//on solaris, threads that loop one the release bool variable
//needs to sleep more than 1 usec.
#ifdef Q_OS_SOLARIS
# define RWTESTSLEEP usleep(10);
#else
# define RWTESTSLEEP usleep(1);
#endif

#include <stdio.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/thread/qmutex.h corelib/thread/qmutex.cpp

class tst_QReadWriteLock : public QObject
{
    Q_OBJECT
public:
    tst_QReadWriteLock();
    virtual ~tst_QReadWriteLock();


/*
    Singlethreaded tests
*/
private slots:
void constructDestruct();
void readLockUnlock();
void writeLockUnlock();
void readLockUnlockLoop();
void writeLockUnlockLoop();
void readLockLoop();
void writeLockLoop();
void readWriteLockUnlockLoop();
void tryReadLock();
void tryWriteLock();
/*
    Multithreaded tests
*/
private slots:

void readLockBlockRelease();
void writeLockBlockRelease();
void multipleReadersBlockRelease();
void multipleReadersLoop();
void multipleWritersLoop();
void multipleReadersWritersLoop();
void countingTest();
void limitedReaders();
void deleteOnUnlock();

/*
    Performance tests
*/
private slots:
void uncontendedLocks();

};

tst_QReadWriteLock::tst_QReadWriteLock()
{

}

tst_QReadWriteLock::~tst_QReadWriteLock()
{

}

void tst_QReadWriteLock::constructDestruct()
{
    {
        QReadWriteLock rwlock;
    }
}

void tst_QReadWriteLock::readLockUnlock()
{
     QReadWriteLock rwlock;
     rwlock.lockForRead();
     rwlock.unlock();
}

void tst_QReadWriteLock::writeLockUnlock()
{
     QReadWriteLock rwlock;
     rwlock.lockForWrite();
     rwlock.unlock();
}

void tst_QReadWriteLock::readLockUnlockLoop()
{
    QReadWriteLock rwlock;
    int runs=10000;
    int i;
    for (i=0; i<runs; ++i) {
        rwlock.lockForRead();
        rwlock.unlock();
    }
}

void tst_QReadWriteLock::writeLockUnlockLoop()
{
    QReadWriteLock rwlock;
    int runs=10000;
    int i;
    for (i=0; i<runs; ++i) {
        rwlock.lockForWrite();
        rwlock.unlock();
    }
}


void tst_QReadWriteLock::readLockLoop()
{
    QReadWriteLock rwlock;
    int runs=10000;
    int i;
    for (i=0; i<runs; ++i) {
        rwlock.lockForRead();
    }
    for (i=0; i<runs; ++i) {
        rwlock.unlock();
    }
}

void tst_QReadWriteLock::writeLockLoop()
{
    /*
        If you include this, the test should print one line
        and then block.
    */
#if 0
    QReadWriteLock rwlock;
    int runs=10000;
    int i;
    for (i=0; i<runs; ++i) {
        rwlock.lockForWrite();
        qDebug("I am going to block now.");
    }
#endif
}

void tst_QReadWriteLock::readWriteLockUnlockLoop()
{
    QReadWriteLock rwlock;
    int runs=10000;
    int i;
    for (i=0; i<runs; ++i) {
        rwlock.lockForRead();
        rwlock.unlock();
        rwlock.lockForWrite();
        rwlock.unlock();
    }

}

void tst_QReadWriteLock::tryReadLock()
{
    QReadWriteLock rwlock;
    QVERIFY(rwlock.tryLockForRead());
    rwlock.unlock();
    QVERIFY(rwlock.tryLockForRead());
    rwlock.unlock();

    rwlock.lockForRead();
    rwlock.lockForRead();
    QVERIFY(rwlock.tryLockForRead());
    rwlock.unlock();
    rwlock.unlock();
    rwlock.unlock();

    rwlock.lockForWrite();
    QVERIFY(!rwlock.tryLockForRead());
    rwlock.unlock();

}

void tst_QReadWriteLock::tryWriteLock()
{
    QReadWriteLock rwlock;
    QVERIFY(rwlock.tryLockForWrite());
    rwlock.unlock();
    QVERIFY(rwlock.tryLockForWrite());
    rwlock.unlock();

    rwlock.lockForWrite();
    QVERIFY(!rwlock.tryLockForWrite());
    rwlock.unlock();

    rwlock.lockForRead();
    QVERIFY(!rwlock.tryLockForWrite());
    rwlock.unlock();

}

bool threadDone;
volatile bool release;

/*
    write-lock
    unlock
    set threadone
*/
class WriteLockThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    inline WriteLockThread(QReadWriteLock &l) : testRwlock(l) { }
    void run()
    {
        testRwlock.lockForWrite();
        testRwlock.unlock();
        threadDone=true;
    }
};

/*
    read-lock
    unlock
    set threadone
*/
class ReadLockThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    inline ReadLockThread(QReadWriteLock &l) : testRwlock(l) { }
    void run()
    {
        testRwlock.lockForRead();
        testRwlock.unlock();
        threadDone=true;
    }
};
/*
    write-lock
    wait for release==true
    unlock
*/
class WriteLockReleasableThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    inline WriteLockReleasableThread(QReadWriteLock &l) : testRwlock(l) { }
    void run()
    {
        testRwlock.lockForWrite();
        while(release==false) {
            RWTESTSLEEP
        }
        testRwlock.unlock();
    }
};

/*
    read-lock
    wait for release==true
    unlock
*/
class ReadLockReleasableThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    inline ReadLockReleasableThread(QReadWriteLock &l) : testRwlock(l) { }
    void run()
    {
        testRwlock.lockForRead();
        while(release==false) {
            RWTESTSLEEP
        }
        testRwlock.unlock();
    }
};


/*
    for(runTime msecs)
        read-lock
        msleep(holdTime msecs)
        release lock
        msleep(waitTime msecs)
*/
class ReadLockLoopThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    int runTime;
    int holdTime;
    int waitTime;
    bool print;
    QTime t;
    inline ReadLockLoopThread(QReadWriteLock &l, int runTime, int holdTime=0, int waitTime=0, bool print=false)
    :testRwlock(l)
    ,runTime(runTime)
    ,holdTime(holdTime)
    ,waitTime(waitTime)
    ,print(print)
    { }
    void run()
    {
        t.start();
        while (t.elapsed()<runTime)  {
            testRwlock.lockForRead();
            if(print) printf("reading\n");
            if (holdTime) msleep(holdTime);
            testRwlock.unlock();
            if (waitTime) msleep(waitTime);
        }
    }
};

/*
    for(runTime msecs)
        write-lock
        msleep(holdTime msecs)
        release lock
        msleep(waitTime msecs)
*/
class WriteLockLoopThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    int runTime;
    int holdTime;
    int waitTime;
    bool print;
    QTime t;
    inline WriteLockLoopThread(QReadWriteLock &l, int runTime, int holdTime=0, int waitTime=0, bool print=false)
    :testRwlock(l)
    ,runTime(runTime)
    ,holdTime(holdTime)
    ,waitTime(waitTime)
    ,print(print)
    { }
    void run()
    {
        t.start();
        while (t.elapsed() < runTime)  {
            testRwlock.lockForWrite();
            if (print) printf(".");
            if (holdTime) msleep(holdTime);
            testRwlock.unlock();
            if (waitTime) msleep(waitTime);
        }
    }
};

volatile int count=0;

/*
    for(runTime msecs)
        write-lock
        count to maxval
        set count to 0
        release lock
        msleep waitTime
*/
class WriteLockCountThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    int runTime;
    int waitTime;
    int maxval;
    QTime t;
    inline WriteLockCountThread(QReadWriteLock &l, int runTime, int waitTime, int maxval)
    :testRwlock(l)
    ,runTime(runTime)
    ,waitTime(waitTime)
    ,maxval(maxval)
    { }
    void run()
    {
        t.start();
        while (t.elapsed() < runTime)  {
            testRwlock.lockForWrite();
            if(count)
                qFatal("Non-zero count at start of write! (%d)",count );
//            printf(".");
            int i;
            for(i=0; i<maxval; ++i) {
                volatile int lc=count;
                ++lc;
                count=lc;
            }
            count=0;
            testRwlock.unlock();
            msleep(waitTime);
        }
    }
};

/*
    for(runTime msecs)
        read-lock
        verify count==0
        release lock
        msleep waitTime
*/
class ReadLockCountThread : public QThread
{
public:
    QReadWriteLock &testRwlock;
    int runTime;
    int waitTime;
    QTime t;
    inline ReadLockCountThread(QReadWriteLock &l, int runTime, int waitTime)
    :testRwlock(l)
    ,runTime(runTime)
    ,waitTime(waitTime)
    { }
    void run()
    {
        t.start();
        while (t.elapsed() < runTime)  {
            testRwlock.lockForRead();
            if(count)
                qFatal("Non-zero count at Read! (%d)",count );
            testRwlock.unlock();
            msleep(waitTime);
        }
    }
};


/*
    A writer aquires a read-lock, a reader locks
    the writer releases the lock, the reader gets the lock
*/
void tst_QReadWriteLock::readLockBlockRelease()
{
    QReadWriteLock testLock;
    testLock.lockForWrite();
    threadDone=false;
    ReadLockThread rlt(testLock);
    rlt.start();
    sleep(1);
    testLock.unlock();
    rlt.wait();
    QVERIFY(threadDone);
}

/*
    writer1 aquires a read-lock, writer2 blocks,
    writer1 releases the lock, writer2 gets the lock
*/
void tst_QReadWriteLock::writeLockBlockRelease()
{
    QReadWriteLock testLock;
    testLock.lockForWrite();
    threadDone=false;
    WriteLockThread wlt(testLock);
    wlt.start();
    sleep(1);
    testLock.unlock();
    wlt.wait();
    QVERIFY(threadDone);
}
/*
    Two readers aquire a read-lock, one writer attempts a write block,
    the readers release their locks, the writer gets the lock.
*/
void tst_QReadWriteLock::multipleReadersBlockRelease()
{

    QReadWriteLock testLock;
    release=false;
    threadDone=false;
    ReadLockReleasableThread rlt1(testLock);
    ReadLockReleasableThread rlt2(testLock);
    rlt1.start();
    rlt2.start();
    sleep(1);
    WriteLockThread wlt(testLock);
    wlt.start();
    sleep(1);
    release=true;
    wlt.wait();
    rlt1.wait();
    rlt2.wait();
    QVERIFY(threadDone);
}

/*
    Multiple readers locks and unlocks a lock.
*/
void tst_QReadWriteLock::multipleReadersLoop()
{
    int time=500;
    int hold=250;
    int wait=0;
#if defined (Q_OS_HPUX)
    const int numthreads=50;
#else
    const int numthreads=75;
#endif
    QReadWriteLock testLock;
    ReadLockLoopThread *threads[numthreads];
    int i;
    for (i=0; i<numthreads; ++i)
        threads[i] = new ReadLockLoopThread(testLock, time, hold, wait);
    for (i=0; i<numthreads; ++i)
        threads[i]->start();
    for (i=0; i<numthreads; ++i)
        threads[i]->wait();
    for (i=0; i<numthreads; ++i)
        delete threads[i];
}

/*
    Multiple writers locks and unlocks a lock.
*/
void tst_QReadWriteLock::multipleWritersLoop()
{
        int time=500;
        int wait=0;
        int hold=0;
        const int numthreads=50;
        QReadWriteLock testLock;
        WriteLockLoopThread *threads[numthreads];
        int i;
        for (i=0; i<numthreads; ++i)
            threads[i] = new WriteLockLoopThread(testLock, time, hold, wait);
        for (i=0; i<numthreads; ++i)
            threads[i]->start();
        for (i=0; i<numthreads; ++i)
            threads[i]->wait();
        for (i=0; i<numthreads; ++i)
            delete threads[i];
}

/*
    Multiple readers and writers locks and unlocks a lock.
*/
void tst_QReadWriteLock::multipleReadersWritersLoop()
{
        //int time=INT_MAX;
        int time=10000;
        int readerThreads=20;
        int readerWait=0;
        int readerHold=1;

        int writerThreads=2;
        int writerWait=500;
        int writerHold=50;

        QReadWriteLock testLock;
        ReadLockLoopThread  *readers[1024];
        WriteLockLoopThread *writers[1024];
        int i;

        for (i=0; i<readerThreads; ++i)
            readers[i] = new ReadLockLoopThread(testLock, time, readerHold, readerWait, false);
        for (i=0; i<writerThreads; ++i)
            writers[i] = new WriteLockLoopThread(testLock, time, writerHold, writerWait, false);

        for (i=0; i<readerThreads; ++i)
            readers[i]->start(QThread::NormalPriority);
        for (i=0; i<writerThreads; ++i)
            writers[i]->start(QThread::IdlePriority);

        for (i=0; i<readerThreads; ++i)
            readers[i]->wait();
        for (i=0; i<writerThreads; ++i)
            writers[i]->wait();

        for (i=0; i<readerThreads; ++i)
            delete readers[i];
        for (i=0; i<writerThreads; ++i)
            delete writers[i];
}

/*
    Writers increment a variable from 0 to maxval, then reset it to 0.
    Readers verify that the variable remains at 0.
*/
void tst_QReadWriteLock::countingTest()
{
        //int time=INT_MAX;
        int time=10000;
        int readerThreads=20;
        int readerWait=1;

        int writerThreads=3;
        int writerWait=150;
        int maxval=10000;

        QReadWriteLock testLock;
        ReadLockCountThread  *readers[1024];
        WriteLockCountThread *writers[1024];
        int i;

        for (i=0; i<readerThreads; ++i)
            readers[i] = new ReadLockCountThread(testLock, time,  readerWait);
        for (i=0; i<writerThreads; ++i)
            writers[i] = new WriteLockCountThread(testLock, time,  writerWait, maxval);

        for (i=0; i<readerThreads; ++i)
            readers[i]->start(QThread::NormalPriority);
        for (i=0; i<writerThreads; ++i)
            writers[i]->start(QThread::LowestPriority);

        for (i=0; i<readerThreads; ++i)
            readers[i]->wait();
        for (i=0; i<writerThreads; ++i)
            writers[i]->wait();

        for (i=0; i<readerThreads; ++i)
            delete readers[i];
        for (i=0; i<writerThreads; ++i)
            delete writers[i];
}

void tst_QReadWriteLock::limitedReaders()
{

};

/*
    Test a race-condition that may happen if one thread is in unlock() while
    another thread deletes the rw-lock.
    
    MainThread              DeleteOnUnlockThread
                        
    write-lock
    unlock
      |                     write-lock
      |                     unlock
      |                     delete lock
    deref d inside unlock
*/
class DeleteOnUnlockThread : public QThread
{
public:
    DeleteOnUnlockThread(QReadWriteLock **lock, QWaitCondition *startup, QMutex *waitMutex)
    :m_lock(lock), m_startup(startup), m_waitMutex(waitMutex) {}
    void run()
    {
        m_waitMutex->lock();
        m_startup->wakeAll();
        m_waitMutex->unlock();
        
        // DeleteOnUnlockThread and the main thread will race from this point
        (*m_lock)->lockForWrite();
        (*m_lock)->unlock();
        delete *m_lock; 
    }
private:
    QReadWriteLock **m_lock;
    QWaitCondition *m_startup;
    QMutex *m_waitMutex;
};

void tst_QReadWriteLock::deleteOnUnlock()
{
    QReadWriteLock *lock = 0;
    QWaitCondition startup;
    QMutex waitMutex;

    DeleteOnUnlockThread thread2(&lock, &startup, &waitMutex);
    
    QTime t;
    t.start();
    while(t.elapsed() < 4000) {
        lock = new QReadWriteLock();
        waitMutex.lock();
        lock->lockForWrite();
        thread2.start();
        startup.wait(&waitMutex);
        waitMutex.unlock();
        
        // DeleteOnUnlockThread and the main thread will race from this point
        lock->unlock();
        
        thread2.wait();
    }
}


void tst_QReadWriteLock::uncontendedLocks()
{

    uint read=0;
    uint write=0;
    uint count=0;
    int millisecs=200;
    {
        QTime t;
        t.start();
        while(t.elapsed() <millisecs)
        {
            ++count;
        }
    }
    {
        QReadWriteLock rwlock;
        QTime t;
        t.start();
        while(t.elapsed() <millisecs)
        {
            rwlock.lockForRead();
            rwlock.unlock();
            ++read;
        }
    }
    {
        QReadWriteLock rwlock;
        QTime t;
        t.start();
        while(t.elapsed() <millisecs)
        {
            rwlock.lockForWrite();
            rwlock.unlock();
            ++write;
        }
    }

    printf("during %d millisecs:\n", millisecs);
    printf("counted to %u\n", count);
    printf("%u uncontended read locks/unlocks\n", read);
    printf("%u uncontended write locks/unlocks\n", write);
}
QTEST_MAIN(tst_QReadWriteLock)

#include "tst_qreadwritelock.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QMutexLocker>
#include <QSemaphore>
#include <QThread>

// TESTED_CLASS=QMutexLocker
// TESTED_FILES=corelib/thread/qmutex.h

class tst_QMutexLockerThread : public QThread
{
public:
    QMutex mutex;
    QSemaphore semaphore, testSemaphore;

    void waitForTest()
    {
        semaphore.release();
        testSemaphore.acquire();
    }

    tst_QMutexLockerThread()
        : mutex(QMutex::Recursive)
    {
    }
};

class tst_QMutexLocker : public QObject
{
    Q_OBJECT

public:
    tst_QMutexLocker();
    ~tst_QMutexLocker();

    tst_QMutexLockerThread *thread;

    void waitForThread()
    {
        thread->semaphore.acquire();
    }
    void releaseThread()
    {
        thread->testSemaphore.release();
    }

private slots:
    void scopeTest();
    void unlockAndRelockTest();
    void lockerStateTest();
};

tst_QMutexLocker::tst_QMutexLocker()
{
}

tst_QMutexLocker::~tst_QMutexLocker()
{
}

void tst_QMutexLocker::scopeTest()
{
    class ScopeTestThread : public tst_QMutexLockerThread
    {
    public:
        void run()
        {
            waitForTest();

            {
                QMutexLocker locker(&mutex);
                waitForTest();
            }

            waitForTest();
        }
    };

    thread = new ScopeTestThread;
    thread->start();

    waitForThread();
    // mutex should be unlocked before entering the scope that creates the QMutexLocker
    QVERIFY(thread->mutex.tryLock());
    thread->mutex.unlock();
    releaseThread();

    waitForThread();
    // mutex should be locked by the QMutexLocker
    QVERIFY(!thread->mutex.tryLock());
    releaseThread();

    waitForThread();
    // mutex should be unlocked when the QMutexLocker goes out of scope
    QVERIFY(thread->mutex.tryLock());
    thread->mutex.unlock();
    releaseThread();

    QVERIFY(thread->wait());

    delete thread;
    thread = 0;
}


void tst_QMutexLocker::unlockAndRelockTest()
{
    class UnlockAndRelockThread : public tst_QMutexLockerThread
    {
    public:
        void run()
        {
            QMutexLocker locker(&mutex);

            waitForTest();

            locker.unlock();

            waitForTest();

            locker.relock();

            waitForTest();
        }
    };

    thread = new UnlockAndRelockThread;
    thread->start();

    waitForThread();
    // mutex should be locked by the QMutexLocker
    QVERIFY(!thread->mutex.tryLock());
    releaseThread();

    waitForThread();
    // mutex has been explicitly unlocked via QMutexLocker
    QVERIFY(thread->mutex.tryLock());
    thread->mutex.unlock();
    releaseThread();

    waitForThread();
    // mutex has been explicity relocked via QMutexLocker
    QVERIFY(!thread->mutex.tryLock());
    releaseThread();

    QVERIFY(thread->wait());

    delete thread;
    thread = 0;
}

void tst_QMutexLocker::lockerStateTest()
{
#if QT_VERSION < 0x040200
    QSKIP("QMutexLocker doesn't keep state in Qt < 4.2", SkipAll);
#else
    class LockerStateThread : public tst_QMutexLockerThread
    {
    public:
        void run()
        {
            {
                QMutexLocker locker(&mutex);
                locker.relock();
                locker.unlock();

                waitForTest();
            }

            waitForTest();
        }
    };

    thread = new LockerStateThread;
    thread->start();

    waitForThread();
    // even though we relock() after creating the QMutexLocker, it shouldn't lock the mutex more than once
    QVERIFY(thread->mutex.tryLock());
    thread->mutex.unlock();
    releaseThread();

    waitForThread();
    // if we call QMutexLocker::unlock(), its destructor should do nothing
    QVERIFY(thread->mutex.tryLock());
    thread->mutex.unlock();
    releaseThread();

    QVERIFY(thread->wait());

    delete thread;
    thread = 0;
#endif
}

QTEST_MAIN(tst_QMutexLocker)
#include "tst_qmutexlocker.moc"

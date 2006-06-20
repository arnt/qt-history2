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
#include <QWriteLocker>
#include <QSemaphore>
#include <QThread>

// TESTED_CLASS=QWriteLocker
// TESTED_FILES=corelib/thread/qreadwritelock.h

class tst_QWriteLockerThread : public QThread
{
public:
    QReadWriteLock lock;
    QSemaphore semaphore, testSemaphore;

    void waitForTest()
    {
        semaphore.release();
        testSemaphore.acquire();
    }
};

class tst_QWriteLocker : public QObject
{
    Q_OBJECT

public:
    tst_QWriteLocker();
    ~tst_QWriteLocker();

    tst_QWriteLockerThread *thread;

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

tst_QWriteLocker::tst_QWriteLocker()
{
}

tst_QWriteLocker::~tst_QWriteLocker()
{
}

void tst_QWriteLocker::scopeTest()
{
    class ScopeTestThread : public tst_QWriteLockerThread
    {
    public:
        void run()
        {
            waitForTest();

            {
                QWriteLocker locker(&lock);
                waitForTest();
            }

            waitForTest();
        }
    };

    thread = new ScopeTestThread;
    thread->start();

    waitForThread();
    // lock should be unlocked before entering the scope that creates the QWriteLocker
    QVERIFY(thread->lock.tryLockForWrite());
    thread->lock.unlock();
    releaseThread();

    waitForThread();
    // lock should be locked by the QWriteLocker
    QVERIFY(!thread->lock.tryLockForWrite());
    releaseThread();

    waitForThread();
    // lock should be unlocked when the QWriteLocker goes out of scope
    QVERIFY(thread->lock.tryLockForWrite());
    thread->lock.unlock();
    releaseThread();

    QVERIFY(thread->wait());

    delete thread;
    thread = 0;
}


void tst_QWriteLocker::unlockAndRelockTest()
{
    class UnlockAndRelockThread : public tst_QWriteLockerThread
    {
    public:
        void run()
        {
            QWriteLocker locker(&lock);

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
    // lock should be locked by the QWriteLocker
    QVERIFY(!thread->lock.tryLockForWrite());
    releaseThread();

    waitForThread();
    // lock has been explicitly unlocked via QWriteLocker
    QVERIFY(thread->lock.tryLockForWrite());
    thread->lock.unlock();
    releaseThread();

    waitForThread();
    // lock has been explicity relocked via QWriteLocker
    QVERIFY(!thread->lock.tryLockForWrite());
    releaseThread();

    QVERIFY(thread->wait());

    delete thread;
    thread = 0;
}

void tst_QWriteLocker::lockerStateTest()
{
#if QT_VERSION < 0x040200
    QSKIP("QWriteLocker doesn't keep state in Qt < 4.2", SkipAll);
#else
    class LockerStateThread : public tst_QWriteLockerThread
    {
    public:
        void run()
        {
            {
                QWriteLocker locker(&lock);
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
    // even though we relock() after creating the QWriteLocker, it shouldn't lock the lock more than once
    QVERIFY(thread->lock.tryLockForWrite());
    thread->lock.unlock();
    releaseThread();

    waitForThread();
    // if we call QWriteLocker::unlock(), its destructor should do nothing
    QVERIFY(thread->lock.tryLockForWrite());
    thread->lock.unlock();
    releaseThread();

    QVERIFY(thread->wait());

    delete thread;
    thread = 0;
#endif
}

QTEST_MAIN(tst_QWriteLocker)
#include "tst_qwritelocker.moc"

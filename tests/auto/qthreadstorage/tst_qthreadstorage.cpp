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
#include <qmutex.h>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qthreadstorage.h>

#ifdef Q_OS_UNIX
#include <pthread.h>
#endif
#ifdef Q_OS_WIN
#include <process.h>
#include <windows.h>
#endif

//TESTED_CLASS=
//TESTED_FILES=corelib/thread/qthreadstorage.h corelib/thread/qthreadstorage.cpp

class tst_QThreadStorage : public QObject
{
    Q_OBJECT

public:
    tst_QThreadStorage();

private slots:
    void hasLocalData();
    void localData();
    void localData_const();
    void setLocalData();
    void autoDelete();
    void adoptedThreads();
};

class Pointer
{
public:
    static int count;
    inline Pointer() { ++count; }
    inline ~Pointer() { --count; }
};
int Pointer::count = 0;

tst_QThreadStorage::tst_QThreadStorage()

{ }

void tst_QThreadStorage::hasLocalData()
{
    QThreadStorage<Pointer *> pointers;
    QVERIFY(!pointers.hasLocalData());
    pointers.setLocalData(new Pointer);
    QVERIFY(pointers.hasLocalData());
    pointers.setLocalData(0);
    QVERIFY(!pointers.hasLocalData());
}

void tst_QThreadStorage::localData()
{
    QThreadStorage<Pointer*> pointers;
    Pointer *p = new Pointer;
    QVERIFY(!pointers.hasLocalData());
    pointers.setLocalData(p);
    QVERIFY(pointers.hasLocalData());
    QCOMPARE(pointers.localData(), p);
    pointers.setLocalData(0);
    QCOMPARE(pointers.localData(), (Pointer *)0);
    QVERIFY(!pointers.hasLocalData());
}

void tst_QThreadStorage::localData_const()
{
    QThreadStorage<Pointer *> pointers;
    const QThreadStorage<Pointer *> &const_pointers = pointers;
    Pointer *p = new Pointer;
    QVERIFY(!pointers.hasLocalData());
    pointers.setLocalData(p);
    QVERIFY(pointers.hasLocalData());
    QCOMPARE(const_pointers.localData(), p);
    pointers.setLocalData(0);
    QCOMPARE(const_pointers.localData(), (Pointer *)0);
    QVERIFY(!pointers.hasLocalData());
}

void tst_QThreadStorage::setLocalData()
{
    QThreadStorage<Pointer *> pointers;
    QVERIFY(!pointers.hasLocalData());
    pointers.setLocalData(new Pointer);
    QVERIFY(pointers.hasLocalData());
    pointers.setLocalData(0);
    QVERIFY(!pointers.hasLocalData());
}

class Thread : public QThread
{
public:
    QThreadStorage<Pointer *> &pointers;

    QMutex mutex;
    QWaitCondition cond;

    Thread(QThreadStorage<Pointer *> &p)
        : pointers(p)
    { }

    void run()
    {
        pointers.setLocalData(new Pointer);

        QMutexLocker locker(&mutex);
        cond.wakeOne();
        cond.wait(&mutex);
    }
};

void tst_QThreadStorage::autoDelete()
{
    QThreadStorage<Pointer *> pointers;
    QVERIFY(!pointers.hasLocalData());

    Thread thread(pointers);
    int c = Pointer::count;
    {
        QMutexLocker locker(&thread.mutex);
        thread.start();
        thread.cond.wait(&thread.mutex);
        // QCOMPARE(Pointer::count, c + 1);
        thread.cond.wakeOne();
    }
    thread.wait();
    QCOMPARE(Pointer::count, c);
}

bool threadStorageOk;
void testAdoptedThreadStorageWin(void *p)
{
    QThreadStorage<Pointer *>  *pointers = reinterpret_cast<QThreadStorage<Pointer *> *>(p);
    if (pointers->hasLocalData()) {
        threadStorageOk = false;
        return;
    }

    Pointer *pointer = new Pointer();
    pointers->setLocalData(pointer);

    if (pointers->hasLocalData() == false) {
        threadStorageOk = false;
        return;
    }

    if (pointers->localData() != pointer) {
        threadStorageOk = false;
        return;
    }
    QObject::connect(QThread::currentThread(), SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
}
void *testAdoptedThreadStorageUnix(void *pointers)
{
    testAdoptedThreadStorageWin(pointers);
    return 0;
}
void tst_QThreadStorage::adoptedThreads()
{
    QTestEventLoop::instance(); // Make sure the instance is created in this thread.
    QThreadStorage<Pointer *> pointers;
    int c = Pointer::count;
    threadStorageOk = true;
    {
#ifdef Q_OS_UNIX
        pthread_t thread;
        const int state = pthread_create(&thread, 0, testAdoptedThreadStorageUnix, &pointers);
        QCOMPARE(state, 0);
        pthread_join(thread, 0);
#elif defined Q_OS_WIN
        HANDLE thread;
        thread = (HANDLE)_beginthread(testAdoptedThreadStorageWin, 0, &pointers);
        QVERIFY(thread);
        WaitForSingleObject(thread, INFINITE);
#endif
    }
    QVERIFY(threadStorageOk);

    QTestEventLoop::instance().enterLoop(2);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(Pointer::count, c);
}

QTEST_MAIN(tst_QThreadStorage)
#include "tst_qthreadstorage.moc"

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
#include <qdatetime.h>
#include <qmutex.h>
#include <qthread.h>
#include <qtimer.h>
#include <qwaitcondition.h>
#include <qdebug.h>

#ifdef Q_OS_UNIX
#include <pthread.h>
#endif
#ifdef Q_OS_WIN
#include <process.h>
#include <windows.h>
#endif

//TESTED_CLASS=QThread
//TESTED_FILES=corelib/thread/qthread.h corelib/thread/qthread.cpp

class tst_QThread : public QObject
{
    Q_OBJECT

public:
    tst_QThread();
    virtual ~tst_QThread();

private slots:
    void currentThreadId();
    void currentThread();
    void isFinished();
    void isRunning();
    void setPriority();
    void priority();
    void setStackSize();
    void stackSize();
    void exit();
    void start();
    void terminate();
    void quit();
    void wait();
    void started();
    void finished();
    void terminated();
    void run();
    void exec();
    void setTerminationEnabled();
    void sleep();
    void msleep();
    void usleep();

    void nativeThreadAdoption();
    void adoptedThreadAffinity();
    void adoptedThreadSetPriority();
    void adoptedThreadExit();
    void adoptedThreadExec();
    void adoptedThreadFinished();
    void adoptedThreadTerminated();
    void adoptMultipleThreads();

    void stressTest();
};

enum { one_minute = 60 * 1000, five_minutes = 5 * one_minute };

class SignalRecorder : public QObject
{
    Q_OBJECT
public:
    QAtomic activationCount;

    inline SignalRecorder()
    { activationCount = 0; }

    bool wasActivated()
    { return activationCount > 0; }

public slots:
    void slot();
};

void SignalRecorder::slot()
{ activationCount.ref(); }

class Current_Thread : public QThread
{
public:
    Qt::HANDLE id;
    QThread *thread;

    void run()
    {
        id = QThread::currentThreadId();
        thread = QThread::currentThread();
    }
};

class Simple_Thread : public QThread
{
public:
    QMutex mutex;
    QWaitCondition cond;

    void run()
    {
        QMutexLocker locker(&mutex);
        cond.wakeOne();
    }
};

class Exit_Object : public QObject
{
    Q_OBJECT
public:
    QThread *thread;
    int code;
public slots:
    void slot()
    { thread->exit(code); }
};

class Exit_Thread : public Simple_Thread
{
public:
    int code;
    int result;

    void run()
    {
        Simple_Thread::run();
        Exit_Object o;
        o.thread = this;
        o.code = code;
        QTimer::singleShot(100, &o, SLOT(slot()));
        result = exec();
    }
};

class Terminate_Thread : public Simple_Thread
{
public:
    void run()
    {
        setTerminationEnabled(false);
        {
            QMutexLocker locker(&mutex);
            cond.wakeOne();
            cond.wait(&mutex, five_minutes);
        }
        setTerminationEnabled(true);
        Q_ASSERT_X(false, "tst_QThread", "test case hung");
    }
};

class Quit_Object : public QObject
{
    Q_OBJECT
public:
    QThread *thread;
public slots:
    void slot()
    { thread->quit(); }
};

class Quit_Thread : public Simple_Thread
{
public:
    int result;

    void run()
    {
        {
            QMutexLocker locker(&mutex);
            cond.wakeOne();
        }
        Quit_Object o;
        o.thread = this;
        QTimer::singleShot(100, &o, SLOT(slot()));
        result = exec();
    }
};

class Sleep_Thread : public Simple_Thread
{
public:
    enum SleepType { Second, Millisecond, Microsecond };

    SleepType sleepType;
    int interval;

    int elapsed; // result, in *MILLISECONDS*

    void run()
    {
        QMutexLocker locker(&mutex);

        elapsed = 0;
        QTime time;
        time.start();
        switch (sleepType) {
        case Second:
            sleep(interval);
            break;
        case Millisecond:
            msleep(interval);
            break;
        case Microsecond:
            usleep(interval);
            break;
        }
        elapsed = time.elapsed();

        cond.wakeOne();
    }
};

tst_QThread::tst_QThread()

{
}

tst_QThread::~tst_QThread()
{

}

void tst_QThread::currentThreadId()
{
    Current_Thread thread;
    thread.id = 0;
    thread.thread = 0;
    thread.start();
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(thread.id != 0);
    QVERIFY(thread.id != QThread::currentThreadId());
}

void tst_QThread::currentThread()
{
    QVERIFY(QThread::currentThread() != 0);
    QCOMPARE(QThread::currentThread(), thread());

    Current_Thread thread;
    thread.id = 0;
    thread.thread = 0;
    thread.start();
    QVERIFY(thread.wait(five_minutes));
    QCOMPARE(thread.thread, (QThread *)&thread);
}

void tst_QThread::isFinished()
{
    Simple_Thread thread;
    QVERIFY(!thread.isFinished());
    QMutexLocker locker(&thread.mutex);
    thread.start();
    QVERIFY(!thread.isFinished());
    thread.cond.wait(locker.mutex());
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(thread.isFinished());
}

void tst_QThread::isRunning()
{
    Simple_Thread thread;
    QVERIFY(!thread.isRunning());
    QMutexLocker locker(&thread.mutex);
    thread.start();
    QVERIFY(thread.isRunning());
    thread.cond.wait(locker.mutex());
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(!thread.isRunning());
}

void tst_QThread::setPriority()
{
#if QT_VERSION < 0x040100
    QSKIP("QThread::setPriority() was introduced in 4.1.0, you are testing " QT_VERSION_STR,
         SkipAll);
#else
    Simple_Thread thread;

    // cannot change the priority, since the thread is not running
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::IdlePriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::LowestPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::LowPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::NormalPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::HighPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::HighestPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::TimeCriticalPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);

    QCOMPARE(thread.priority(), QThread::InheritPriority);
    QMutexLocker locker(&thread.mutex);
    thread.start();

    // change the priority of a running thread
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::IdlePriority);
    QCOMPARE(thread.priority(), QThread::IdlePriority);
    thread.setPriority(QThread::LowestPriority);
    QCOMPARE(thread.priority(), QThread::LowestPriority);
    thread.setPriority(QThread::LowPriority);
    QCOMPARE(thread.priority(), QThread::LowPriority);
    thread.setPriority(QThread::NormalPriority);
    QCOMPARE(thread.priority(), QThread::NormalPriority);
    thread.setPriority(QThread::HighPriority);
    QCOMPARE(thread.priority(), QThread::HighPriority);
    thread.setPriority(QThread::HighestPriority);
    QCOMPARE(thread.priority(), QThread::HighestPriority);
    thread.setPriority(QThread::TimeCriticalPriority);
    QCOMPARE(thread.priority(), QThread::TimeCriticalPriority);
    thread.cond.wait(locker.mutex());
    QVERIFY(thread.wait(five_minutes));

    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::IdlePriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::LowestPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::LowPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::NormalPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::HighPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::HighestPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
    thread.setPriority(QThread::TimeCriticalPriority);
    QCOMPARE(thread.priority(), QThread::InheritPriority);
#endif
}

void tst_QThread::priority()
{ DEPENDS_ON("setPriority"); }

void tst_QThread::setStackSize()
{
    Simple_Thread thread;
    QCOMPARE(thread.stackSize(), 0u);
    thread.setStackSize(8192u);
    QCOMPARE(thread.stackSize(), 8192u);
    thread.setStackSize(0u);
    QCOMPARE(thread.stackSize(), 0u);
}

void tst_QThread::stackSize()
{
    DEPENDS_ON("setStackSize");
}

void tst_QThread::exit()
{
    Exit_Thread thread;
    thread.code = 42;
    thread.result = 0;
    QVERIFY(!thread.isFinished());
    QVERIFY(!thread.isRunning());
    QMutexLocker locker(&thread.mutex);
    thread.start();
    QVERIFY(thread.isRunning());
    QVERIFY(!thread.isFinished());
    thread.cond.wait(locker.mutex());
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(thread.isFinished());
    QVERIFY(!thread.isRunning());
    QCOMPARE(thread.result, thread.code);
}

void tst_QThread::start()
{
    QThread::Priority priorities[] = {
	QThread::IdlePriority,
	QThread::LowestPriority,
	QThread::LowPriority,
	QThread::NormalPriority,
	QThread::HighPriority,
	QThread::HighestPriority,
	QThread::TimeCriticalPriority,
	QThread::InheritPriority
    };
    const int prio_count = sizeof(priorities) / sizeof(QThread::Priority);

    for (int i = 0; i < prio_count; ++i) {
        Simple_Thread thread;
        QVERIFY(!thread.isFinished());
        QVERIFY(!thread.isRunning());
        QMutexLocker locker(&thread.mutex);
        thread.start(priorities[i]);
        QVERIFY(thread.isRunning());
        QVERIFY(!thread.isFinished());
        thread.cond.wait(locker.mutex());
        QVERIFY(thread.wait(five_minutes));
        QVERIFY(thread.isFinished());
        QVERIFY(!thread.isRunning());
    }
}

void tst_QThread::terminate()
{
    Terminate_Thread thread;
    {
        QMutexLocker locker(&thread.mutex);
        thread.start();
        QVERIFY(thread.cond.wait(locker.mutex(), five_minutes));
        thread.terminate();
        thread.cond.wakeOne();
    }
    QVERIFY(thread.wait(five_minutes));
}

void tst_QThread::quit()
{
    Quit_Thread thread;
    QVERIFY(!thread.isFinished());
    QVERIFY(!thread.isRunning());
    QMutexLocker locker(&thread.mutex);
    thread.start();
    QVERIFY(thread.isRunning());
    QVERIFY(!thread.isFinished());
    thread.cond.wait(locker.mutex());
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(thread.isFinished());
    QVERIFY(!thread.isRunning());
    QCOMPARE(thread.result, 0);
}

void tst_QThread::wait()
{
    DEPENDS_ON("isRunning");
    DEPENDS_ON("isFinished");
}

void tst_QThread::started()
{
    SignalRecorder recorder;
    Simple_Thread thread;
    connect(&thread, SIGNAL(started()), &recorder, SLOT(slot()), Qt::DirectConnection);
    thread.start();
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(recorder.wasActivated());
}

void tst_QThread::finished()
{
    SignalRecorder recorder;
    Simple_Thread thread;
    connect(&thread, SIGNAL(finished()), &recorder, SLOT(slot()), Qt::DirectConnection);
    thread.start();
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(recorder.wasActivated());
}

void tst_QThread::terminated()
{
    SignalRecorder recorder;
    Terminate_Thread thread;
    connect(&thread, SIGNAL(terminated()), &recorder, SLOT(slot()), Qt::DirectConnection);
    {
        QMutexLocker locker(&thread.mutex);
        thread.start();
        thread.cond.wait(locker.mutex());
        thread.terminate();
        thread.cond.wakeOne();
    }
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(recorder.wasActivated());
}

void tst_QThread::run()
{ DEPENDS_ON("wait()"); }

void tst_QThread::exec()
{
    DEPENDS_ON("exit()");
    DEPENDS_ON("quit()");
}

void tst_QThread::setTerminationEnabled()
{ DEPENDS_ON("terminate"); }

void tst_QThread::sleep()
{
    Sleep_Thread thread;
    thread.sleepType = Sleep_Thread::Second;
    thread.interval = 2;
    thread.start();
    QVERIFY(thread.wait(five_minutes));
    QVERIFY(thread.elapsed >= 2000);
}

void tst_QThread::msleep()
{
    Sleep_Thread thread;
    thread.sleepType = Sleep_Thread::Millisecond;
    thread.interval = 120;
    thread.start();
    QVERIFY(thread.wait(five_minutes));
#if defined (Q_OS_WIN)
    // Since the resolution of QTime is so coarse...
    QVERIFY(thread.elapsed >= 100);
#else
    QVERIFY(thread.elapsed >= 120);
#endif
}

void tst_QThread::usleep()
{
    Sleep_Thread thread;
    thread.sleepType = Sleep_Thread::Microsecond;
    thread.interval = 120000;
    thread.start();
    QVERIFY(thread.wait(five_minutes));
#if defined (Q_OS_WIN)
    // Since the resolution of QTime is so coarse...
    QVERIFY(thread.elapsed >= 100);
#else
    QVERIFY(thread.elapsed >= 120);
#endif
}

typedef void (*FunctionPointer)(void *);
void noop(void*) { }

#ifdef Q_OS_UNIX
    typedef pthread_t ThreadHandle;
#elif defined Q_OS_WIN
    typedef HANDLE ThreadHandle;
#endif

class NativeThreadWrapper
{
public:
    NativeThreadWrapper() : qthread(0), waitForStop(false) {}
    void start(FunctionPointer functionPointer = noop, void *data = 0);
    void startAndWait(FunctionPointer functionPointer = noop, void *data = 0);
    void join();
    void setWaitForStop() { waitForStop = true; }
    void stop();

    ThreadHandle nativeThread;
    QThread *qthread;
    QWaitCondition startCondition;
    QMutex mutex;
    bool waitForStop;
    QWaitCondition stopCondition;
protected:
    static void *runUnix(void *data);
    static void runWin(void *data);

    FunctionPointer functionPointer;
    void *data;
};

void NativeThreadWrapper::start(FunctionPointer functionPointer, void *data)
{
    this->functionPointer = functionPointer;
    this->data = data;
#ifdef Q_OS_UNIX
    const int state = pthread_create(&nativeThread, 0, NativeThreadWrapper::runUnix, this);
    Q_UNUSED(state);
#elif defined Q_OS_WIN
    nativeThread = (HANDLE)_beginthread(NativeThreadWrapper::runWin, 0, this);
#endif
}

void NativeThreadWrapper::startAndWait(FunctionPointer functionPointer, void *data)
{
    QMutexLocker locker(&mutex);
    start(functionPointer, data);
    startCondition.wait(locker.mutex());
}

void NativeThreadWrapper::join()
{
#ifdef Q_OS_UNIX
    pthread_join(nativeThread, 0);
#elif defined Q_OS_WIN
    WaitForSingleObject(nativeThread, INFINITE);
#endif
}

void *NativeThreadWrapper::runUnix(void *that)
{
    NativeThreadWrapper *nativeThreadWrapper = reinterpret_cast<NativeThreadWrapper*>(that);

    // Adoppt thread, create QThread object.
    nativeThreadWrapper->qthread = QThread::currentThread();

    // Release main thread.
    {
        QMutexLocker lock(&nativeThreadWrapper->mutex);
        nativeThreadWrapper->startCondition.wakeOne();
    }

    // Run function.
    nativeThreadWrapper->functionPointer(nativeThreadWrapper->data);

    // Wait for stop.
    {
        QMutexLocker lock(&nativeThreadWrapper->mutex);
        if (nativeThreadWrapper->waitForStop)
            nativeThreadWrapper->stopCondition.wait(lock.mutex());
    }

    return 0;
}

void NativeThreadWrapper::runWin(void *data)
{
    runUnix(data);
}

void NativeThreadWrapper::stop()
{
    QMutexLocker lock(&mutex);
    waitForStop = false;
    stopCondition.wakeOne();
}

bool threadAdoptedOk = false;
QThread *mainThread;
void testNativeThreadAdoption(void *)
{
    threadAdoptedOk = (QThread::currentThreadId() != 0
                       && QThread::currentThread() != 0
                       && QThread::currentThread() != mainThread);
}
void tst_QThread::nativeThreadAdoption()
{
    threadAdoptedOk = false;
    mainThread = QThread::currentThread();
    NativeThreadWrapper nativeThread;
    nativeThread.setWaitForStop();
    nativeThread.startAndWait(testNativeThreadAdoption);
    QVERIFY(nativeThread.qthread);
    
    nativeThread.stop();
    nativeThread.join();
    
    QVERIFY(threadAdoptedOk);
}

void adoptedThreadAffinityFunction(void *arg)
{
    QThread **affinity = reinterpret_cast<QThread **>(arg);
    QThread *current = QThread::currentThread();
    affinity[0] = current;
    affinity[1] = current->thread();
}

void tst_QThread::adoptedThreadAffinity()
{
    QThread *affinity[2] = { 0, 0 };

    NativeThreadWrapper thread;
    thread.startAndWait(adoptedThreadAffinityFunction, affinity);
    thread.join();

    // adopted thread should have affinity to itself
    QCOMPARE(affinity[0], affinity[1]);
}

void tst_QThread::adoptedThreadSetPriority()
{

    NativeThreadWrapper nativeThread;
    nativeThread.setWaitForStop();
    nativeThread.startAndWait();

    // change the priority of a running thread
    QCOMPARE(nativeThread.qthread->priority(), QThread::InheritPriority);
    nativeThread.qthread->setPriority(QThread::IdlePriority);
    QCOMPARE(nativeThread.qthread->priority(), QThread::IdlePriority);
    nativeThread.qthread->setPriority(QThread::LowestPriority);
    QCOMPARE(nativeThread.qthread->priority(), QThread::LowestPriority);
    nativeThread.qthread->setPriority(QThread::LowPriority);
    QCOMPARE(nativeThread.qthread->priority(), QThread::LowPriority);
    nativeThread.qthread->setPriority(QThread::NormalPriority);
    QCOMPARE(nativeThread.qthread->priority(), QThread::NormalPriority);
    nativeThread.qthread->setPriority(QThread::HighPriority);
    QCOMPARE(nativeThread.qthread->priority(), QThread::HighPriority);
    nativeThread.qthread->setPriority(QThread::HighestPriority);
    QCOMPARE(nativeThread.qthread->priority(), QThread::HighestPriority);
    nativeThread.qthread->setPriority(QThread::TimeCriticalPriority);
    QCOMPARE(nativeThread.qthread->priority(), QThread::TimeCriticalPriority);

    nativeThread.stop();
    nativeThread.join();
}

void tst_QThread::adoptedThreadExit()
{
    NativeThreadWrapper nativeThread;
    nativeThread.setWaitForStop();

    nativeThread.startAndWait();
    QVERIFY(nativeThread.qthread);
    QVERIFY(nativeThread.qthread->isRunning());
    QVERIFY(!nativeThread.qthread->isFinished());

    nativeThread.stop();
    nativeThread.join();
}

void adoptedThreadExecFunction(void *)
{
    QThread  * const adoptedThread = QThread::currentThread();
    QEventLoop eventLoop(adoptedThread);

    const int code = 1;
    Exit_Object o;
    o.thread = adoptedThread;
    o.code = code;
    QTimer::singleShot(100, &o, SLOT(slot()));
    
    const int result = eventLoop.exec();
    QCOMPARE(result, code);
}

void tst_QThread::adoptedThreadExec()
{
    NativeThreadWrapper nativeThread;
    nativeThread.start(adoptedThreadExecFunction);
    nativeThread.join();
}

/*
    Test that you get the finished signal when an adopted thread exits.
*/
void tst_QThread::adoptedThreadFinished()
{
    NativeThreadWrapper nativeThread;
    nativeThread.setWaitForStop();
    nativeThread.startAndWait();

    QObject::connect(nativeThread.qthread, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));

    nativeThread.stop();
    nativeThread.join();

    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(!QTestEventLoop::instance().timeout());
}

void tst_QThread::adoptedThreadTerminated()
{
    NativeThreadWrapper nativeThread;
    nativeThread.setWaitForStop();
    nativeThread.startAndWait();

    QObject::connect(nativeThread.qthread, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));

    nativeThread.qthread->terminate();
    nativeThread.stop();
    nativeThread.join();
   
    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(!QTestEventLoop::instance().timeout());
}

void tst_QThread::adoptMultipleThreads()
{
    const int numThreads = 5;
    QVector<NativeThreadWrapper*> nativeThreads;

    SignalRecorder recorder;
    
    for (int i = 0; i < numThreads; ++i) {
        nativeThreads.append(new NativeThreadWrapper());
        nativeThreads.at(i)->setWaitForStop();
        nativeThreads.at(i)->startAndWait();  
        QObject::connect(nativeThreads.at(i)->qthread, SIGNAL(finished()), &recorder, SLOT(slot()));
    }

    QObject::connect(nativeThreads.at(numThreads - 1)->qthread, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    
    for (int i = 0; i < numThreads; ++i) {
        nativeThreads.at(i)->stop();
        nativeThreads.at(i)->join();  
    }

    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(!QTestEventLoop::instance().timeout());
    QCOMPARE(int(recorder.activationCount), numThreads);
}

void tst_QThread::stressTest()
{
    QTime t;
    t.start();
    while (t.elapsed() < one_minute) {
        Current_Thread t;
        t.start();
        t.wait();
    }
}


QTEST_MAIN(tst_QThread)
#include "tst_qthread.moc"

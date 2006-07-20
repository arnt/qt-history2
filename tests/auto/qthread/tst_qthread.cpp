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

    void stressTest();
};

enum { one_minute = 60 * 1000, five_minutes = 5 * one_minute };

class SignalRecorder : public QObject
{
    Q_OBJECT

    bool activated;

public:
    inline SignalRecorder()
    { activated = false; }

    bool wasActivated()
    { return activated; }

public slots:
    void slot();
};

void SignalRecorder::slot()
{ activated = true; }

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

bool threadAdoptedOk = false;
QThread *mainThread;
void testThreadAdoptionWin(void *)
{
    threadAdoptedOk = (QThread::currentThreadId() != 0
                       && QThread::currentThread() != 0
                       && QThread::currentThread() != mainThread);
}
void *testThreadAdoptionUnix(void *)
{
    testThreadAdoptionWin(0);
    return 0;
}

void tst_QThread::nativeThreadAdoption()
{
    threadAdoptedOk = false;
    mainThread = QThread::currentThread();
#ifdef Q_OS_UNIX
    pthread_t thread;
    const int state = pthread_create(&thread, 0, testThreadAdoptionUnix, 0);
    QCOMPARE(state, 0);
    pthread_join(thread, 0);
#elif defined Q_OS_WIN
    HANDLE thread;
    thread = (HANDLE)_beginthread(testThreadAdoptionWin, 0, NULL);
    QVERIFY(thread);
    WaitForSingleObject(thread, INFINITE);
#endif
	QVERIFY(threadAdoptedOk);
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

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>
#include <qcoreevent.h>
#include <qeventloop.h>
#include <qmutex.h>
#include <qthread.h>
#include <qtimer.h>
#include <qwaitcondition.h>
#include <QTcpServer>
#include <QTcpSocket>

//TESTED_CLASS=QEventLoop
//TESTED_FILES=corelib/kernel/qeventloop.h corelib/kernel/qeventloop.cpp

class EventLoopExiter : public QObject
{
    Q_OBJECT
    QEventLoop *eventLoop;
public:
    inline EventLoopExiter(QEventLoop *el)
        : eventLoop(el)
    { }
public slots:
    void exit();
    void exit1();
    void exit2();
};

void EventLoopExiter::exit()
{ eventLoop->exit(); }

void EventLoopExiter::exit1()
{ eventLoop->exit(1); }

void EventLoopExiter::exit2()
{ eventLoop->exit(2); }

class EventLoopThread : public QThread
{
    Q_OBJECT
signals:
    void checkPoint();
public:
    QEventLoop *eventLoop;
    void run();
};

void EventLoopThread::run()
{
    eventLoop = new QEventLoop;
    emit checkPoint();
    (void) eventLoop->exec();
    delete eventLoop;
    eventLoop = 0;
}

class MultipleExecThread : public QThread
{
    Q_OBJECT
signals:
    void checkPoint();
public:
    QMutex mutex;
    QWaitCondition cond;
    void run()
    {
        QMutexLocker locker(&mutex);
        // this exec should work

        cond.wakeOne();
        cond.wait(&mutex);

        QTimer timer;
        connect(&timer, SIGNAL(timeout()), SLOT(quit()), Qt::DirectConnection);
        timer.setInterval(1000);
        timer.start();
        (void) exec();

        // this should return immediately, since exit() has been called
        cond.wakeOne();
        cond.wait(&mutex);
        QEventLoop eventLoop;
        (void) eventLoop.exec();
    }
};

class StartStopEvent: public QEvent
{
public:
    StartStopEvent(int type, QEventLoop *loop = 0)
        : QEvent(Type(type)), el(loop)
    { }

    QEventLoop *el;
};

class EventLoopExecutor : public QObject
{
    Q_OBJECT
    QEventLoop *eventLoop;
public:
    int returnCode;
    EventLoopExecutor(QEventLoop *eventLoop)
        : QObject(), eventLoop(eventLoop), returnCode(-42)
    {
    }
public slots:
    void exec()
    {
        QTimer::singleShot(100, eventLoop, SLOT(quit()));
        // this should return immediately, and the timer event should be delivered to
        // tst_QEventLoop::exec() test, letting the test complete
        returnCode = eventLoop->exec();
    }
};

class tst_QEventLoop : public QObject
{
    Q_OBJECT
public:
    tst_QEventLoop();
    ~tst_QEventLoop();
public slots:
    void init();
    void cleanup();
private slots:
    void processEvents();
    void exec();
    void exit();
    void wakeUp();
    void quit();
    void processEventsExcludeSocket();

    // keep this test last:
    void nestedLoops();

protected:
    void customEvent(QEvent *e);
};

tst_QEventLoop::tst_QEventLoop()
{ }

tst_QEventLoop::~tst_QEventLoop()
{ }

void tst_QEventLoop::init()
{ }

void tst_QEventLoop::cleanup()
{ }

void tst_QEventLoop::processEvents()
{
    QSignalSpy spy1(QAbstractEventDispatcher::instance(), SIGNAL(aboutToBlock()));
    QSignalSpy spy2(QAbstractEventDispatcher::instance(), SIGNAL(awake()));

    QEventLoop eventLoop;

    QCoreApplication::postEvent(&eventLoop, new QEvent(QEvent::User));

    // process posted events, QEventLoop::processEvents() should return
    // true
    QVERIFY(eventLoop.processEvents());
    QCOMPARE(spy1.count(), 0);
    QCOMPARE(spy2.count(), 1);

    // allow any session manager to complete its handshake, so that
    // there are no pending events left.
    while (eventLoop.processEvents())
        ;

    // On mac we get application started events at this point,
    // so process events one more time just to be sure.
    eventLoop.processEvents();

    // no events to process, QEventLoop::processEvents() should return
    // false
    spy1.clear();
    spy2.clear();
    QVERIFY(!eventLoop.processEvents());
    QCOMPARE(spy1.count(), 0);
    QCOMPARE(spy2.count(), 1);

    // make sure the test doesn't block forever
    int timerId = startTimer(100);

    // wait for more events to process, QEventLoop::processEvents()
    // should return true
    spy1.clear();
    spy2.clear();
    QVERIFY(eventLoop.processEvents(QEventLoop::WaitForMoreEvents));

    // Verify that the eventloop has blocked and woken up. Some eventloops
    // may block and wake up multiple times.
    QVERIFY(spy1.count() > 0);
    QVERIFY(spy2.count() > 0);
    // We should get one awake for each aboutToBlock, plus one awake when
    // processEvents is entered.
    QVERIFY(spy2.count() >= spy1.count());

    killTimer(timerId);
}

void tst_QEventLoop::exec()
{
    {
        QEventLoop eventLoop;
        EventLoopExiter exiter(&eventLoop);
        int returnCode;

        QTimer::singleShot(100, &exiter, SLOT(exit()));
        returnCode = eventLoop.exec();
        QCOMPARE(returnCode, 0);

        QTimer::singleShot(100, &exiter, SLOT(exit1()));
        returnCode = eventLoop.exec();
        QCOMPARE(returnCode, 1);

        QTimer::singleShot(100, &exiter, SLOT(exit2()));
        returnCode = eventLoop.exec();
        QCOMPARE(returnCode, 2);
    }

    {
        // calling exec() after exit()/quit() should return immediately
        MultipleExecThread thread;

        // start thread and wait for checkpoint
        thread.mutex.lock();
        thread.start();
        thread.cond.wait(&thread.mutex);

        // make sure the eventloop runs
        QSignalSpy spy(QAbstractEventDispatcher::instance(&thread), SIGNAL(awake()));
        thread.cond.wakeOne();
        thread.cond.wait(&thread.mutex);
        QVERIFY(spy.count() > 0);

        // exec should return immediately
        spy.clear();
        thread.cond.wakeOne();
        thread.mutex.unlock();
        thread.wait();
        QCOMPARE(spy.count(), 0);
    }

    {
        // a single instance of QEventLoop should not be allowed to recurse into exec()
        QEventLoop eventLoop;
        EventLoopExecutor executor(&eventLoop);

        QTimer::singleShot(100, &executor, SLOT(exec()));
        int returnCode = eventLoop.exec();
        QCOMPARE(returnCode, 0);
        QCOMPARE(executor.returnCode, -1);
    }
}

void tst_QEventLoop::exit()
{ DEPENDS_ON(exec()); }

void tst_QEventLoop::wakeUp()
{
    EventLoopThread thread;
    QEventLoop eventLoop;
    connect(&thread, SIGNAL(checkPoint()), &eventLoop, SLOT(quit()));
    connect(&thread, SIGNAL(finished()), &eventLoop, SLOT(quit()));

    thread.start();
    (void) eventLoop.exec();

    QSignalSpy spy(QAbstractEventDispatcher::instance(&thread), SIGNAL(awake()));
    thread.eventLoop->wakeUp();

    // give the thread time to wake up
    QTimer::singleShot(1000, &eventLoop, SLOT(quit()));
    (void) eventLoop.exec();

    QVERIFY(spy.count() > 0);

    thread.quit();
    (void) eventLoop.exec();
}

void tst_QEventLoop::quit()
{
    QEventLoop eventLoop;
    int returnCode;

    QTimer::singleShot(100, &eventLoop, SLOT(quit()));
    returnCode = eventLoop.exec();
    QCOMPARE(returnCode, 0);
}


void tst_QEventLoop::nestedLoops()
{
    QCoreApplication::postEvent(this, new StartStopEvent(QEvent::User));
    QCoreApplication::postEvent(this, new StartStopEvent(QEvent::User));
    QCoreApplication::postEvent(this, new StartStopEvent(QEvent::User));

    // without the fix, this will *wedge* and never return
    QTest::qWait(1000);
}

void tst_QEventLoop::customEvent(QEvent *e)
{
    if (e->type() == QEvent::User) {
        QEventLoop loop;
        QCoreApplication::postEvent(this, new StartStopEvent(int(QEvent::User) + 1, &loop));
        loop.exec();
    } else {
        static_cast<StartStopEvent *>(e)->el->exit();
    }
}

class SocketEventsTester: public QObject
{
    Q_OBJECT
public:
    SocketEventsTester()
    {
        socket = 0;
        server = 0;
        dataArrived = false;
        testResult = false;
    }
    ~SocketEventsTester()
    {
        delete socket;
        delete server;
    }
    bool init()
    {
        bool ret = false;
        server = new QTcpServer();
        socket = new QTcpSocket();
        connect(server, SIGNAL(newConnection()), this, SLOT(sendHello()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(sendAck()), Qt::DirectConnection);
        if((ret = server->listen(QHostAddress::LocalHost, 0))) {
            socket->connectToHost(server->serverAddress(), server->serverPort());
            socket->waitForConnected();
        }
        return ret;
    }

    QTcpSocket *socket;
    QTcpServer *server;
    bool dataArrived;
    bool testResult;
public slots:
    void sendAck()
    {
        dataArrived = true;
    }
    void sendHello()
    {
        char data[10] ="HELLO";
        qint64 size = sizeof(data);

        QTcpSocket *serverSocket = server->nextPendingConnection();
        serverSocket->write(data, size);
        serverSocket->flush();
        QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        testResult = dataArrived;
        serverSocket->close();
        QThread::currentThread()->exit(0);
    }
};

class SocketTestThread : public QThread
{
    Q_OBJECT
public:
    SocketTestThread():QThread(0),testResult(false){};
    void run()
    {
        SocketEventsTester *tester = new SocketEventsTester();
        if (tester->init())
            exec();
        testResult = tester->testResult;
        delete tester;
    }
     bool testResult;
};

void tst_QEventLoop::processEventsExcludeSocket()
{
    SocketTestThread thread;
    thread.start();
    QVERIFY(thread.wait());
    QVERIFY(!thread.testResult);
}

QTEST_MAIN(tst_QEventLoop)
#include "tst_qeventloop.moc"































// previous test

#if 0

#include <qwidget.h>

#ifdef Q_WS_WIN
#include <windows.h>
#endif



//TESTED_CLASS=
//TESTED_FILES=corelib/kernel/qeventloop.h corelib/kernel/qeventloop.cpp

class EventHandlerWidget : public QWidget
{
public:
    EventHandlerWidget( QWidget* parent = 0, const char* name = 0 ) : QWidget( parent, name )
    {
	installEventFilter( this );
	recievedPaintEvent = FALSE;
	recievedMouseEvent = FALSE;
    }
    ~EventHandlerWidget() {}

    // Sucky I know...
    bool recievedPaintEvent;
    bool recievedMouseEvent;

protected:
    bool eventFilter( QObject* o, QEvent* e )
    {
	if ( e->type() == QEvent::Paint )
	    recievedPaintEvent = TRUE;
	else if ( e->type() == QEvent::MouseButtonPress )
	    recievedMouseEvent = TRUE;
	return QWidget::eventFilter( o, e );
    }
};

class InBetweenObject : public QObject
{
    Q_OBJECT
public:
    InBetweenObject(QObject *parent, QObject *child)
    : QObject(parent), childObject(child)
    {
        childObject->setParent(this);
        ++instanceCounter;
    }

    ~InBetweenObject()
    {
        --instanceCounter;
    }

    static int instanceCounter;

protected:
    void childEvent(QChildEvent *e)
    {
        if (e->removed() && e->child() == childObject) {
            deleteLater();
        }
    }

private:
    QObject *childObject;
};

class ObjectContainer : public QObject
{
public:
    ObjectContainer(QObject *parent = 0)
    : QObject(parent)
    {
    }

protected:
    void childEvent(QChildEvent *e)
    {
        if (e->inserted() && !::qobject_cast<InBetweenObject*>(e->child())) {
            InBetweenObject *inBetween = new InBetweenObject(this, e->child());
        }
    }
};

class tst_QEventLoop : public QObject
{
    Q_OBJECT
public:
    tst_QEventLoop();
    ~tst_QEventLoop();
public slots:
    void init();
    void cleanup();
private slots:
    void processEvents();
    void eventHandlerPostsEvent();
};

tst_QEventLoop::tst_QEventLoop()
{
}

tst_QEventLoop::~tst_QEventLoop()
{
}

void tst_QEventLoop::init()
{
}

void tst_QEventLoop::cleanup()
{
}


void tst_QEventLoop::processEvents()
{
    EventHandlerWidget *mainWidget = new EventHandlerWidget( 0 );
    mainWidget->show();
    qApp->setMainWidget( mainWidget );

#ifdef Q_WS_WIN
    QEventLoop* eventLoop = qApp->eventLoop();
    eventLoop->processEvents( QEventLoop::AllEvents );
    QVERIFY( !eventLoop->hasPendingEvents() );

    // Make sure the flag is cleared first
    mainWidget->recievedPaintEvent = FALSE;
#ifdef Q_WS_WIN
    InvalidateRect( mainWidget->winId(), 0, TRUE );
#endif
    QVERIFY( !mainWidget->recievedPaintEvent );
    eventLoop->processEvents( QEventLoop::AllEvents );
    QVERIFY( mainWidget->recievedPaintEvent );

#ifdef Q_WS_WIN
    // Hardcoded for now...
    LPARAM lParam = MAKELPARAM( 10, 10 );
    PostMessage( mainWidget->winId(), WM_LBUTTONDOWN, 0, lParam );
#endif

    mainWidget->recievedMouseEvent = FALSE;
    eventLoop->processEvents( QEventLoop::ExcludeUserInput );
    QVERIFY( !mainWidget->recievedMouseEvent );
#else
    QSKIP( "QEventLoop test is not implememented on X11 yet", SkipAll);
#endif
}

int InBetweenObject::instanceCounter = 0;

void tst_QEventLoop::eventHandlerPostsEvent()
{
    ObjectContainer container;

    QObject *object = new QObject(&container);
    qApp->processEvents();
    qApp->processEvents();

    QCOMPARE(InBetweenObject::instanceCounter, 1);

    object->deleteLater();

    qApp->processEvents();
    QCOMPARE(InBetweenObject::instanceCounter, 0);
};

QTEST_MAIN(tst_QEventLoop)
#include "tst_qeventloop.moc"

#endif

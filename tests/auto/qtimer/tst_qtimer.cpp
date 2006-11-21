/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qtimer.h>





#if defined Q_OS_UNIX
#include <unistd.h>
#endif


//TESTED_CLASS=
//TESTED_FILES=corelib/kernel/qtimer.h corelib/kernel/qtimer.cpp

class tst_QTimer : public QObject
{
    Q_OBJECT

public:
    tst_QTimer();
    virtual ~tst_QTimer();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void singleShotTimeout();
    void timeout();
    void livelock_data();
    void livelock();
    void timerInfiniteRecursion_data();
    void timerInfiniteRecursion();
    void recurringTimer_data();
    void recurringTimer();
    void deleteLaterOnQTimer(); // long name, don't want to shadow QObject::deleteLater()
};

class TimerHelper : public QObject
{
    Q_OBJECT
public:
    TimerHelper() : QObject(), count(0)
    {
    }

    int count;

public slots:
    void timeout();
};

void TimerHelper::timeout()
{
    ++count;
}

tst_QTimer::tst_QTimer()
{
}

tst_QTimer::~tst_QTimer()
{
}

void tst_QTimer::initTestCase()
{
}

void tst_QTimer::cleanupTestCase()
{
}

void tst_QTimer::init()
{
}

void tst_QTimer::cleanup()
{
}

void tst_QTimer::singleShotTimeout()
{
    TimerHelper helper;
    QTimer timer;
    timer.setSingleShot(true);

    connect(&timer, SIGNAL(timeout()), &helper, SLOT(timeout()));
    timer.start(100);

    QTest::qWait(500);
    QCOMPARE(helper.count, 1);
    QTest::qWait(500);
    QCOMPARE(helper.count, 1);
}

void tst_QTimer::timeout()
{
    TimerHelper helper;
    QTimer timer;

    connect(&timer, SIGNAL(timeout()), &helper, SLOT(timeout()));
    timer.start(100);

    QCOMPARE(helper.count, 0);

    QTest::qWait(200);
    QVERIFY(helper.count > 0);
    int oldCount = helper.count;

    QTest::qWait(200);
    QVERIFY(helper.count > oldCount);
}


void tst_QTimer::livelock_data()
{
    QTest::addColumn<int>("interval");
    QTest::newRow("zero timer") << 0;
    QTest::newRow("non-zero timer") << 1;
    QTest::newRow("longer than sleep") << 20;
}

/*!
 *
 * DO NOT "FIX" THIS TEST!  it is written like this for a reason, do
 * not *change it without first dicussing it with Brad, Pascal, and
 * Paul
 *
*/
class LiveLockTester : public QObject
{
public:
    LiveLockTester(int i)
        : interval(i),
          timeoutsForFirst(0), timeoutsForExtra(0), timeoutsForSecond(0),
          postEventAtRightTime(false)
    {
        firstTimerId = startTimer(interval);
        extraTimerId = startTimer(interval + 80);
        secondTimerId = -1; // started later
    }

    bool event(QEvent *e) {
        if (e->type() == 4002) {
            // got the posted event
            if (timeoutsForFirst == 1 && timeoutsForSecond == 0)
                postEventAtRightTime = true;
            return true;
        }
        return QObject::event(e);
    }

    void timerEvent(QTimerEvent *te) {
        if (te->timerId() == firstTimerId) {
            if (++timeoutsForFirst == 1) {
                killTimer(extraTimerId);
                extraTimerId = -1;
		QApplication::postEvent(this, new QEvent(static_cast<QEvent::Type>(4002)));
                secondTimerId = startTimer(interval);
            }
        } else if (te->timerId() == secondTimerId) {
            ++timeoutsForSecond;
        } else if (te->timerId() == extraTimerId) {
            ++timeoutsForExtra;
        }

        // sleep for 2ms
        QTest::qSleep(2);

        killTimer(te->timerId());
    }

    const int interval;
    int firstTimerId;
    int secondTimerId;
    int extraTimerId;
    int timeoutsForFirst;
    int timeoutsForExtra;
    int timeoutsForSecond;
    bool postEventAtRightTime;
};

void tst_QTimer::livelock()
{
    /*
      New timers created in timer event handlers should not be sent
      until the next iteration of the eventloop.  Note: this test
      depends on the fact that we send posted events before timer
      events (since new posted events are not sent until the next
      iteration of the eventloop either).
    */
    QFETCH(int, interval);
    LiveLockTester tester(interval);
    QTest::qWait(180); // we have to use wait here, since we're testing timers with a non-zero timeout
    QCOMPARE(tester.timeoutsForFirst, 1);
    QCOMPARE(tester.timeoutsForExtra, 0);
    QCOMPARE(tester.timeoutsForSecond, 1);
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    QEXPECT_FAIL("zero timer", "", Continue);
    QEXPECT_FAIL("non-zero timer", "", Continue);
#endif
    QVERIFY(tester.postEventAtRightTime);
}

class TimerInfiniteRecursionObject : public QObject
{
public:
    bool inTimerEvent;
    bool timerEventRecursed;

    TimerInfiniteRecursionObject()
        : inTimerEvent(false), timerEventRecursed(false)
    { }

    void timerEvent(QTimerEvent *timerEvent)
    {
        timerEventRecursed = inTimerEvent;
        if (timerEventRecursed) {
            // bug detected!
            return;
        }

        inTimerEvent = true;
        QApplication::processEvents();
        inTimerEvent = false;

        killTimer(timerEvent->timerId());
    }
};

void tst_QTimer::timerInfiniteRecursion_data()
{
    QTest::addColumn<int>("interval");
    QTest::newRow("zero timer") << 0;
    QTest::newRow("non-zero timer") << 1;
}


void tst_QTimer::timerInfiniteRecursion()
{
    QFETCH(int, interval);
    TimerInfiniteRecursionObject object;
    (void) object.startTimer(interval);
    QApplication::processEvents();
    QVERIFY(!object.timerEventRecursed);
}

class RecurringTimerObject : public QObject
{
Q_OBJECT
public:
    int times;
    int target;

    RecurringTimerObject(int target)
        : times(0), target(target) { }

    void timerEvent(QTimerEvent *timerEvent)
    {
        if (++times == target) {
            killTimer(timerEvent->timerId());
            emit done();
        }
    }

    signals:
    void done();
};

void tst_QTimer::recurringTimer_data()
{
    QTest::addColumn<int>("interval");
    QTest::newRow("zero timer") << 0;
    QTest::newRow("non-zero timer") << 1;
}

void tst_QTimer::recurringTimer()
{
    const int target = 5;
    QFETCH(int, interval);
    RecurringTimerObject object(target);
    QObject::connect(&object, SIGNAL(done()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    (void) object.startTimer(interval);
    QTestEventLoop::instance().enterLoop(5);

    QCOMPARE(object.times, target);
}

void tst_QTimer::deleteLaterOnQTimer()
{
    QTimer *timer = new QTimer;
    connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
    connect(timer, SIGNAL(destroyed()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    timer->setInterval(1);
    timer->setSingleShot(true);
    timer->start();
    QPointer<QTimer> pointer = timer;
    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(pointer.isNull());
}

QTEST_MAIN(tst_QTimer)
#include "tst_qtimer.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifdef Q_OS_WIN
#include <private/qwineventnotifier_p.h>


#include <qtimer.h>

//TESTED_CLASS=QWinEventNotifier
//TESTED_FILES=

class tst_QWinEventNotifier : public QObject
{
    Q_OBJECT

public:
    tst_QWinEventNotifier();
    ~tst_QWinEventNotifier();

    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

protected slots:
    void simple_activated();
    void simple_timerSet();
private slots:

    void simple();

private:
    HANDLE simpleHEvent;
    bool simpleActivated;
};

tst_QWinEventNotifier::tst_QWinEventNotifier()
{}

tst_QWinEventNotifier::~tst_QWinEventNotifier()
{ }

void tst_QWinEventNotifier::initTestCase()
{ }

void tst_QWinEventNotifier::cleanupTestCase()
{ }

void tst_QWinEventNotifier::init()
{ }

void tst_QWinEventNotifier::cleanup()
{
}


void tst_QWinEventNotifier::simple_activated()
{
    simpleActivated = true;
    ResetEvent((HANDLE)simpleHEvent);
    QTestEventLoop::instance().exitLoop();
}

void tst_QWinEventNotifier::simple_timerSet()
{
    SetEvent((HANDLE)simpleHEvent);
}

void tst_QWinEventNotifier::simple()
{
    QT_WA({
	simpleHEvent = CreateEventW(0, TRUE, FALSE, 0);
    }, {
	simpleHEvent = CreateEventA(0, TRUE, FALSE, 0);
    });
    QVERIFY(simpleHEvent);
    QWinEventNotifier n(simpleHEvent);
    QObject::connect(&n, SIGNAL(activated(HANDLE)), this, SLOT(simple_activated()));
    simpleActivated = false;

    SetEvent((HANDLE)simpleHEvent);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out");

    QVERIFY(simpleActivated);


    simpleActivated = false;

    QTimer::singleShot(3000, this, SLOT(simple_timerSet()));

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out");

    QVERIFY(simpleActivated);
}

QTEST_MAIN(tst_QWinEventNotifier)

#include "tst_qwineventnotifier.moc"
#else // non-windows systems
QTEST_NOOP_MAIN
#endif


/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

class tst_QCoreApplication: public QObject
{
    Q_OBJECT
private slots:
    void qAppName();
    void argc();
    void postEvent();
    void removePostedEvents();
};

void tst_QCoreApplication::qAppName()
{
    int argc = 1;
    char *argv[] = { "tst_qcoreapplication" };
    QCoreApplication app(argc, argv);
    QVERIFY(!::qAppName().isEmpty());
}

void tst_QCoreApplication::argc()
{
    {
        int argc = 1;
        char *argv[] = { "tst_qcoreapplication" };
        QCoreApplication app(argc, argv);
        QCOMPARE(argc, 1);
        QCOMPARE(app.argc(), 1);
    }

    {
        int argc = 4;
        char *argv[] = { "tst_qcoreapplication", "arg1", "arg2", "arg3" };
        QCoreApplication app(argc, argv);
        QCOMPARE(argc, 4);
        QCOMPARE(app.argc(), 4);
    }

    {
        int argc = 0;
        char **argv = 0;
        QCoreApplication app(argc, argv);
        QCOMPARE(argc, 0);
        QCOMPARE(app.argc(), 0);
    }
}

class EventSpy : public QObject
{
   Q_OBJECT

public:
    QList<int> recordedEvents;
    bool eventFilter(QObject *, QEvent *event)
    {
        recordedEvents.append(event->type());
        return false;
    }
};

class EventGenerator : public QObject
{
    Q_OBJECT

public:
    QObject *other;

    bool event(QEvent *e)
    {
        if (e->type() == QEvent::MaxUser) {
            QCoreApplication::sendPostedEvents(other, 0);
        } else if (e->type() <= QEvent::User + 999) {
            // post a new event in response to this posted event
            int offset = e->type() - QEvent::User;
            offset = (offset * 10 + offset % 10);
            QCoreApplication::postEvent(this, new QEvent(QEvent::Type(QEvent::User + offset)), offset);
        }

        return QObject::event(e);
    }
};

void tst_QCoreApplication::postEvent()
{
    int argc = 1;
    char *argv[] = { "tst_qcoreapplication" };
    QCoreApplication app(argc, argv);

    EventSpy spy;
    EventGenerator odd, even;
    odd.other = &even;
    odd.installEventFilter(&spy);
    even.other = &odd;
    even.installEventFilter(&spy);

    QCoreApplication::postEvent(&odd,  new QEvent(QEvent::Type(QEvent::User + 1)));
    QCoreApplication::postEvent(&even, new QEvent(QEvent::Type(QEvent::User + 2)));

    QCoreApplication::postEvent(&odd,  new QEvent(QEvent::Type(QEvent::User + 3)), 1);
    QCoreApplication::postEvent(&even, new QEvent(QEvent::Type(QEvent::User + 4)), 2);

    QCoreApplication::postEvent(&odd,  new QEvent(QEvent::Type(QEvent::User + 5)), -2);
    QCoreApplication::postEvent(&even, new QEvent(QEvent::Type(QEvent::User + 6)), -1);

    QList<int> expected;
    expected << QEvent::User + 4
             << QEvent::User + 3
             << QEvent::User + 1
             << QEvent::User + 2
             << QEvent::User + 6
             << QEvent::User + 5;

    QCoreApplication::sendPostedEvents();
    // live lock protection ensures that we only send the initial events
    QCOMPARE(spy.recordedEvents, expected);

    expected.clear();
    expected << QEvent::User + 66
             << QEvent::User + 55
             << QEvent::User + 44
             << QEvent::User + 33
             << QEvent::User + 22
             << QEvent::User + 11;

    spy.recordedEvents.clear();
    QCoreApplication::sendPostedEvents();
    // expect next sequence events
    QCOMPARE(spy.recordedEvents, expected);

    // have the generators call sendPostedEvents() on each other in
    // response to an event
    QCoreApplication::postEvent(&odd, new QEvent(QEvent::MaxUser), INT_MAX);
    QCoreApplication::postEvent(&even, new QEvent(QEvent::MaxUser), INT_MAX);

    expected.clear();
    expected << int(QEvent::MaxUser)
             << int(QEvent::MaxUser)
             << QEvent::User + 555
             << QEvent::User + 333
             << QEvent::User + 111
             << QEvent::User + 666
             << QEvent::User + 444
             << QEvent::User + 222;

    spy.recordedEvents.clear();
    QCoreApplication::sendPostedEvents();
    QCOMPARE(spy.recordedEvents, expected);

    expected.clear();
    expected << QEvent::User + 6666
             << QEvent::User + 5555
             << QEvent::User + 4444
             << QEvent::User + 3333
             << QEvent::User + 2222
             << QEvent::User + 1111;

    spy.recordedEvents.clear();
    QCoreApplication::sendPostedEvents();
    QCOMPARE(spy.recordedEvents, expected);

    // no more events
    expected.clear();
    spy.recordedEvents.clear();
    QCoreApplication::sendPostedEvents();
    QCOMPARE(spy.recordedEvents, expected);
}

void tst_QCoreApplication::removePostedEvents()
{
    int argc = 1;
    char *argv[] = { "tst_qcoreapplication" };
    QCoreApplication app(argc, argv);

    EventSpy spy;
    QObject one, two;
    one.installEventFilter(&spy);
    two.installEventFilter(&spy);

    QList<int> expected;

    // remove all events for one object
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 1)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 2)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 3)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 4)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 5)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 6)));
    QCoreApplication::removePostedEvents(&one);
    expected << QEvent::User + 4
             << QEvent::User + 5
             << QEvent::User + 6;
    QCoreApplication::sendPostedEvents();
    QCOMPARE(spy.recordedEvents, expected);
    spy.recordedEvents.clear();
    expected.clear();

    // remove all events for all objects
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 7)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 8)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 9)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 10)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 11)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 12)));
    QCoreApplication::removePostedEvents(0);
    QCoreApplication::sendPostedEvents();
    QVERIFY(spy.recordedEvents.isEmpty());

    // remove a specific type of event for one object
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 13)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 14)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 15)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 16)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 17)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 18)));
    QCoreApplication::removePostedEvents(&one, QEvent::User + 13);
    QCoreApplication::removePostedEvents(&two, QEvent::User + 18);
    QCoreApplication::sendPostedEvents();
    expected << QEvent::User + 14
             << QEvent::User + 15
             << QEvent::User + 16
             << QEvent::User + 17;
    QCOMPARE(spy.recordedEvents, expected);
    spy.recordedEvents.clear();
    expected.clear();

    // remove a specific type of event for all objects
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 19)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 19)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 20)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 20)));
    QCoreApplication::postEvent(&one, new QEvent(QEvent::Type(QEvent::User + 21)));
    QCoreApplication::postEvent(&two, new QEvent(QEvent::Type(QEvent::User + 21)));
    QCoreApplication::removePostedEvents(0, QEvent::User + 20);
    QCoreApplication::sendPostedEvents();
    expected << QEvent::User + 19
             << QEvent::User + 19
             << QEvent::User + 21
             << QEvent::User + 21;
    QCOMPARE(spy.recordedEvents, expected);
    spy.recordedEvents.clear();
    expected.clear();
}

QTEST_APPLESS_MAIN(tst_QCoreApplication)
#include "tst_qcoreapplication.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcoreevent.h>

//TESTED_CLASS=QEvent
//TESTED_FILES=corelib/kernel/qcoreevent.h corelib/kernel/qcoreevent.cpp

class tst_QEvent : public QObject
{
    Q_OBJECT
public:
    tst_QEvent();
    ~tst_QEvent();

private slots:
    void registerEventType_data();
    void registerEventType();
};

tst_QEvent::tst_QEvent()
{ }

tst_QEvent::~tst_QEvent()
{ }

void tst_QEvent::registerEventType_data()
{
    QTest::addColumn<int>("hint");
    QTest::addColumn<int>("expected");

    // default argument
    QTest::newRow("default") << -1 << int(QEvent::MaxUser);
    // hint not valid
    QTest::newRow("User-1") << int(QEvent::User - 1) << int(QEvent::MaxUser - 1);
    // hint valid, but already taken
    QTest::newRow("MaxUser-1") << int(QEvent::MaxUser - 1) << int(QEvent::MaxUser - 2);
    // hint valid, but not taken
    QTest::newRow("User + 1000") << int(QEvent::User + 1000) << int(QEvent::User + 1000);
}

void tst_QEvent::registerEventType()
{
    QFETCH(int, hint);
    QFETCH(int, expected);
    QCOMPARE(QEvent::registerEventType(hint), expected);
}

QTEST_MAIN(tst_QEvent)
#include "tst_qevent.moc"

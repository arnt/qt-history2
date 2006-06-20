/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtTest/QtTest>

class tst_Sleep: public QObject
{
    Q_OBJECT

private slots:
    void sleep();
};

void tst_Sleep::sleep()
{
    QTime t;
    t.start();

    QTest::qSleep(100);
    QVERIFY(t.elapsed() > 90);

    QTest::qSleep(1000);
    QVERIFY(t.elapsed() > 1000);

    QTest::qSleep(1000 * 10); // 10 seconds
    QVERIFY(t.elapsed() > 1000 * 10);
}

QTEST_MAIN(tst_Sleep)

#include "tst_sleep.moc"

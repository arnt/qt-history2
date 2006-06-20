/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <math.h>

class tst_QNumeric: public QObject
{
    Q_OBJECT

private slots:
    void qIsEqual_data();
    void qIsEqual();
};

static bool qIsEqual(double d1, double d2, double delta = 0.0000000000001)
{
    int exp;
    frexp(qAbs(d1) < qAbs(d2) ? d2 : d1, &exp);
    return qAbs(d1 - d2) < ldexp(delta, exp);
}

void tst_QNumeric::qIsEqual_data()
{
    QTest::addColumn<double>("val1");
    QTest::addColumn<double>("val2");
    QTest::addColumn<bool>("isEqual");

    QTest::newRow("zero") << 0.0 << 0.0 << true;
    QTest::newRow("ten") << 10.0 << 10.0 << true;
    QTest::newRow("large") << 1000000000.0 << 1000000000.0 << true;
    QTest::newRow("small") << 0.00000000001 << 0.00000000001 << true;
    QTest::newRow("eps") << 10.000000000000001 << 10.00000000000002 << true;
    QTest::newRow("eps2") << 10.000000000000001 << 10.000000000000009 << true;

    QTest::newRow("mis1") << 0.0 << 1.0 << false;
    QTest::newRow("mis2") << 0.0 << 10000000.0 << false;
    QTest::newRow("mis3") << 0.0 << 0.000000001 << false;
    QTest::newRow("mis4") << 100000000.0 << 0.000000001 << false;
    QTest::newRow("mis4") << 0.0000000001 << 0.000000001 << false;
}

void tst_QNumeric::qIsEqual()
{
    QFETCH(double, val1);
    QFETCH(double, val2);
    QFETCH(bool, isEqual);

    QCOMPARE(::qIsEqual(val1, val2), isEqual);
    QCOMPARE(::qIsEqual(val2, val1), isEqual);
    QCOMPARE(::qIsEqual(-val1, -val2), isEqual);
    QCOMPARE(::qIsEqual(-val2, -val1), isEqual);
}

QTEST_MAIN(tst_QNumeric)
#include "tst_qnumeric.moc"

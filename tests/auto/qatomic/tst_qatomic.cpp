/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#if QT_VERSION >= 0x040200
#  include <qatomic_arch.h>
#else
#  include <arch/qatomic.h>
#endif
#include <qatomic.h>

#include <limits.h>




//TESTED_CLASS=QAtomic
//TESTED_FILES=

class tst_QAtomic : public QObject
{
    Q_OBJECT

public:
    tst_QAtomic();
    ~tst_QAtomic();

private slots:
    // QAtomic members
    void constructor_data();
    void constructor();
    void copy_constructor_data();
    void copy_constructor();
    void ref_data();
    void ref();
    void deref_data();
    void deref();
    void equality_operator_data();
    void equality_operator();
    void inequality_operator_data();
    void inequality_operator();
    void not_operator_data();
    void not_operator();
    void cast_operator_data();
    void cast_operator();
    void assignment_operator_data();
    void assignment_operator();
    void testAndSet_data();
    void testAndSet();
    void exchange_data();
    void exchange();
    void fetchAndAdd_data();
    void fetchAndAdd();

    // stress tests
    void testAndSet_loop();
    void fetchAndAdd_loop();
    void fetchAndAdd_threadedLoop();
};

tst_QAtomic::tst_QAtomic()
{ }

tst_QAtomic::~tst_QAtomic()
{ }

void tst_QAtomic::constructor_data()
{
    QTest::addColumn<int>("value");

    QTest::newRow("0") << 31337;
    QTest::newRow("1") << 0;
    QTest::newRow("2") << 1;
    QTest::newRow("3") << -1;
    QTest::newRow("4") << 2;
    QTest::newRow("5") << -2;
    QTest::newRow("6") << 3;
    QTest::newRow("7") << -3;
    QTest::newRow("8") << INT_MAX;
    QTest::newRow("9") << INT_MIN+1;
}

void tst_QAtomic::constructor()
{
    QFETCH(int, value);
    QAtomic atomic1(value);
    QCOMPARE(int(atomic1), value);
    QAtomic atomic2 = value;
    QCOMPARE(int(atomic2), value);
}

void tst_QAtomic::copy_constructor_data()
{ constructor_data(); }

void tst_QAtomic::copy_constructor()
{
    QFETCH(int, value);
    QAtomic atomic1(value);
    QCOMPARE(int(atomic1), value);

    QAtomic atomic2(atomic1);
    QCOMPARE(int(atomic2), value);
    QAtomic atomic3 = atomic1;
    QCOMPARE(int(atomic3), value);
    QAtomic atomic4(atomic2);
    QCOMPARE(int(atomic4), value);
    QAtomic atomic5 = atomic2;
    QCOMPARE(int(atomic5), value);
}

void tst_QAtomic::ref_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<int>("result");
    QTest::addColumn<int>("expected");

    QTest::newRow("data0") <<  0 << 1 << 1;
    QTest::newRow("data1") << -1 << 0 << 0;
    QTest::newRow("data2") <<  1 << 1 << 2;
}

void tst_QAtomic::ref()
{
    QFETCH(int, value);
    QAtomic x = value;
    QTEST(x.ref() ? 1 : 0, "result");
    QTEST(int(x), "expected");
}

void tst_QAtomic::deref_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<int>("result");
    QTest::addColumn<int>("expected");

    QTest::newRow("data0") <<  0 << 1 << -1;
    QTest::newRow("data1") <<  1 << 0 <<  0;
    QTest::newRow("data2") <<  2 << 1 <<  1;
}

void tst_QAtomic::deref()
{
    QFETCH(int, value);
    QAtomic x = value;
    QTEST(x.deref() ? 1 : 0, "result");
    QTEST(int(x), "expected");
}

void tst_QAtomic::equality_operator_data()
{
    QTest::addColumn<int>("value1");
    QTest::addColumn<int>("value2");
    QTest::addColumn<int>("result");

    QTest::newRow("success0") <<  1 <<  1 << 1;
    QTest::newRow("success1") << -1 << -1 << 1;
    QTest::newRow("failure0") <<  0 <<  1 << 0;
    QTest::newRow("failure1") <<  1 <<  0 << 0;
    QTest::newRow("failure2") <<  0 << -1 << 0;
    QTest::newRow("failure3") << -1 <<  0 << 0;
}

void tst_QAtomic::equality_operator()
{
    QFETCH(int, value1);
    QFETCH(int, value2);
    QAtomic x = value1;
    QTEST(x == value2 ? 1 : 0, "result");
}

void tst_QAtomic::inequality_operator_data()
{
    QTest::addColumn<int>("value1");
    QTest::addColumn<int>("value2");
    QTest::addColumn<int>("result");

    QTest::newRow("failure0") <<  1 <<  1 << 0;
    QTest::newRow("failure1") << -1 << -1 << 0;
    QTest::newRow("success0") <<  0 <<  1 << 1;
    QTest::newRow("success1") <<  1 <<  0 << 1;
    QTest::newRow("success2") <<  0 << -1 << 1;
    QTest::newRow("success3") << -1 <<  0 << 1;
}

void tst_QAtomic::inequality_operator()
{
    QFETCH(int, value1);
    QFETCH(int, value2);
    QAtomic x = value1;
    QTEST(x != value2 ? 1 : 0, "result");
}

void tst_QAtomic::not_operator_data()
{ constructor_data(); }

void tst_QAtomic::not_operator()
{
    QFETCH(int, value);
    QAtomic atomic = value;
    QCOMPARE(!atomic, !value);
}

void tst_QAtomic::cast_operator_data()
{ constructor_data(); }

void tst_QAtomic::cast_operator()
{
    QFETCH(int, value);
    QAtomic atomic = value;
    int copy = atomic;
    QCOMPARE(copy, value);
}

void tst_QAtomic::assignment_operator_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<int>("newval");

    QTest::newRow("value0") <<  0 <<  1;
    QTest::newRow("value1") <<  1 <<  0;
    QTest::newRow("value2") <<  0 << -1;
    QTest::newRow("value3") << -1 <<  0;
    QTest::newRow("value4") << -1 <<  1;
    QTest::newRow("value5") <<  1 << -1;
}

void tst_QAtomic::assignment_operator()
{
    QFETCH(int, value);
    QFETCH(int, newval);

    {
        QAtomic atomic1 = value;
        atomic1 = newval;
        QCOMPARE(int(atomic1), newval);
        atomic1 = value;
        QCOMPARE(int(atomic1), value);
        QAtomic atomic2 = newval;
        atomic1 = atomic2;
        QCOMPARE(atomic1, atomic2);
    }
}

void tst_QAtomic::testAndSet_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<int>("expected");
    QTest::addColumn<int>("newval");
    QTest::addColumn<int>("result");

    // these should succeed
    QTest::newRow("success0") <<         0 <<         0 <<         0 << 1;
    QTest::newRow("success1") <<         0 <<         0 <<         1 << 1;
    QTest::newRow("success2") <<         0 <<         0 <<        -1 << 1;
    QTest::newRow("success3") <<         1 <<         1 <<         0 << 1;
    QTest::newRow("success4") <<         1 <<         1 <<         1 << 1;
    QTest::newRow("success5") <<         1 <<         1 <<        -1 << 1;
    QTest::newRow("success6") <<        -1 <<        -1 <<         0 << 1;
    QTest::newRow("success7") <<        -1 <<        -1 <<         1 << 1;
    QTest::newRow("success8") <<        -1 <<        -1 <<        -1 << 1;
    QTest::newRow("success9") << INT_MIN+1 << INT_MIN+1 << INT_MIN+1 << 1;
    QTest::newRow("successA") << INT_MIN+1 << INT_MIN+1 <<         1 << 1;
    QTest::newRow("successB") << INT_MIN+1 << INT_MIN+1 <<        -1 << 1;
    QTest::newRow("successC") << INT_MAX   << INT_MAX   << INT_MAX   << 1;
    QTest::newRow("successD") << INT_MAX   << INT_MAX   <<         1 << 1;
    QTest::newRow("successE") << INT_MAX   << INT_MAX   <<        -1 << 1;

    // these should fail
    QTest::newRow("failure0") <<       0   <<       1   <<        ~0 << 0;
    QTest::newRow("failure1") <<       0   <<      -1   <<        ~0 << 0;
    QTest::newRow("failure2") <<       1   <<       0   <<        ~0 << 0;
    QTest::newRow("failure3") <<      -1   <<       0   <<        ~0 << 0;
    QTest::newRow("failure4") <<       1   <<      -1   <<        ~0 << 0;
    QTest::newRow("failure5") <<      -1   <<       1   <<        ~0 << 0;
    QTest::newRow("failure6") << INT_MIN+1 << INT_MAX   <<        ~0 << 0;
    QTest::newRow("failure7") << INT_MAX   << INT_MIN+1 <<        ~0 << 0;
}

void tst_QAtomic::testAndSet()
{
    QFETCH(int, value);
    QFETCH(int, expected);
    QFETCH(int, newval);
    QAtomic atomic = value;
    QTEST(atomic.testAndSet(expected, newval) ? 1 : 0, "result");
}

void tst_QAtomic::exchange_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<int>("newval");

    QTest::newRow("data0") << 0 << 1;
    QTest::newRow("data1") << 1 << 2;
    QTest::newRow("data2") << 3 << 8;
}

void tst_QAtomic::exchange()
{
    QFETCH(int, value);
    QFETCH(int, newval);
    QAtomic atomic = value;
    QCOMPARE(atomic.exchange(newval), value);
    QCOMPARE(int(atomic), newval);
}

void tst_QAtomic::fetchAndAdd_data()
{
    QTest::addColumn<int>("value1");
    QTest::addColumn<int>("value2");

    QTest::newRow("0+1") << 0 << 1;
    QTest::newRow("1+0") << 1 << 0;
    QTest::newRow("1+2") << 1 << 2;
    QTest::newRow("2+1") << 2 << 1;
    QTest::newRow("10+21") << 10 << 21;
    QTest::newRow("31+40") << 31 << 40;
    QTest::newRow("51+62") << 51 << 62;
    QTest::newRow("72+81") << 72 << 81;
    QTest::newRow("810+721") << 810 << 721;
    QTest::newRow("631+540") << 631 << 540;
    QTest::newRow("451+362") << 451 << 362;
    QTest::newRow("272+181") << 272 << 181;
    QTest::newRow("1810+8721") << 810 << 721;
    QTest::newRow("3631+6540") << 631 << 540;
    QTest::newRow("5451+4362") << 451 << 362;
    QTest::newRow("7272+2181") << 272 << 181;
}

void tst_QAtomic::fetchAndAdd()
{
    QFETCH(int, value1);
    QFETCH(int, value2);
    int result;

    {
        QAtomic atomic = value1;
        result = atomic.fetchAndAdd(value2);
        QCOMPARE(result, value1);
        QCOMPARE(int(atomic), value1 + value2);
    }

    {
        QAtomic atomic = value1;
        result = atomic.fetchAndAddAcquire(value2);
        QCOMPARE(result, value1);
        QCOMPARE(int(atomic), value1 + value2);
    }

    {
        QAtomic atomic = value1;
        result = atomic.fetchAndAddRelease(value2);
        QCOMPARE(result, value1);
        QCOMPARE(int(atomic), value1 + value2);
    }
}

void tst_QAtomic::testAndSet_loop()
{
    QTime stopWatch;
    stopWatch.start();

    int iterations = 10000000;

    QAtomic val=0;
    for (int i = 0; i < iterations; ++i) {
        QVERIFY(val.testAndSet(val, val+1));
        if ((i % 1000) == 999) {
            if (stopWatch.elapsed() > 60 * 1000) {
                // This test shouldn't run for more than two minutes.
                qDebug("Interrupted test after %d iterations (%.2f iterations/sec)",
                       i, (i * 1000.0) / double(stopWatch.elapsed()));
                break;
            }
        }
    }
}

void tst_QAtomic::fetchAndAdd_loop()
{
    int iterations = 10000000;
#if defined (Q_OS_HPUX)
    iterations = 1000000;
#endif

    QAtomic val=0;
    for (int i = 0; i < iterations; ++i) {
        const int prev = val.fetchAndAdd(1);
        QCOMPARE(prev, int(val) -1);
    }
}

class FetchAndAddThread : public QThread
{
public:
    void run()
    {

        for (int i = 0; i < iterations; ++i)
            val->fetchAndAddAcquire(1);

        for (int i = 0; i < iterations; ++i)
            val->fetchAndAddAcquire(-1);

    }
QAtomic *val;
int iterations;
};


void tst_QAtomic::fetchAndAdd_threadedLoop()
{
    QAtomic val;
    FetchAndAddThread t1;
    t1.val = &val;
    t1.iterations = 1000000;

    FetchAndAddThread t2;
    t2.val = &val;
    t2.iterations = 2000000;
    
    t1.start();
    t2.start();
    t1.wait();
    t2.wait();
    
    QCOMPARE(int(val), 0);
}

QTEST_APPLESS_MAIN(tst_QAtomic)
#include "tst_qatomic.moc"

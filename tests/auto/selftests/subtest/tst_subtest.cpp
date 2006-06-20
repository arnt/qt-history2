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

class tst_Subtest: public QObject
{
    Q_OBJECT
public slots:
    void init();
    void initTestCase();

    void cleanup();
    void cleanupTestCase();

private slots:
    void test1();
    void test2_data();
    void test2();
    void test3_data();
    void test3();
};


void tst_Subtest::initTestCase()
{
    printf("initTestCase %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");
}

void tst_Subtest::cleanupTestCase()
{
    printf("cleanupTestCase %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");
}

void tst_Subtest::init()
{
    printf("init %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");
}

void tst_Subtest::cleanup()
{
    printf("cleanup %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");
}

void tst_Subtest::test1()
{
    printf("test1 %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");
}

void tst_Subtest::test2_data()
{
    printf("test2_data %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");

    QTest::addColumn<QString>("str");

    QTest::newRow("data0") << QString("hello0");
    QTest::newRow("data1") << QString("hello1");
    QTest::newRow("data2") << QString("hello2");

    printf("test2_data end\n");
}

void tst_Subtest::test2()
{
    printf("test2 %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");

    static int count = 0;

    QFETCH(QString, str);
    QCOMPARE(str, QString("hello%1").arg(count++));

    printf("test2 end\n");
}

void tst_Subtest::test3_data()
{
    printf("test3_data %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");

    QTest::addColumn<QString>("str");

    QTest::newRow("data0") << QString("hello0");
    QTest::newRow("data1") << QString("hello1");
    QTest::newRow("data2") << QString("hello2");

    printf("test3_data end\n");
}

void tst_Subtest::test3()
{
    printf("test2 %s %s\n",
            QTest::currentTestFunction() ? QTest::currentTestFunction() : "(null)",
            QTest::currentDataTag() ? QTest::currentDataTag() : "(null)");

    QFETCH(QString, str);

    // second and third time we call this it shoud FAIL
    QCOMPARE(str, QString("hello0"));

    printf("test2 end\n");
}

QTEST_MAIN(tst_Subtest)

#include "tst_subtest.moc"

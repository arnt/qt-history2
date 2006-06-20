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

class tst_Skip: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();

    void emptytest_data();
    void emptytest();

    void singleSkip_data();
    void singleSkip();
};


void tst_Skip::test_data()
{
    QTest::addColumn<bool>("booll");
    QTest::newRow("local 1") << false;
    QTest::newRow("local 2") << true;

    QSKIP("skipping all", SkipAll);
}

void tst_Skip::test()
{
    printf("this line should never be reached, since we skip in the _data function\n");
}

void tst_Skip::emptytest_data()
{
    QSKIP("skipping all", SkipAll);
}

void tst_Skip::emptytest()
{
    printf("this line should never be reached, since we skip in the _data function\n");
}

void tst_Skip::singleSkip_data()
{
    QTest::addColumn<bool>("booll");
    QTest::newRow("local 1") << false;
    QTest::newRow("local 2") << true;
}

void tst_Skip::singleSkip()
{
    QFETCH(bool, booll);
    if (!booll)
        QSKIP("skipping one", SkipSingle);
    printf("this line should only be reached once (%s)\n", booll ? "true" : "false");
}

QTEST_MAIN(tst_Skip)

#include "tst_skip.moc"

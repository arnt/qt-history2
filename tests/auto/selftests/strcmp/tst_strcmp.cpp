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

class tst_StrCmp: public QObject
{
    Q_OBJECT

private slots:
    void compare_charstars();
    void compare_bytearray();
};

void tst_StrCmp::compare_charstars()
{
    QCOMPARE("foo", "foo");

    const char *str1 = "foo";
    QCOMPARE("foo", str1);
    QCOMPARE(str1, "foo");
    QCOMPARE(str1, str1);

    char *str2 = "foo";
    QCOMPARE("foo", str2);
    QCOMPARE(str2, "foo");
    QCOMPARE(str2, str2);
    QCOMPARE(str1, str2);
    QCOMPARE(str2, str1);

    const char str3[] = "foo";
    QCOMPARE(str3, "foo");
    QCOMPARE("foo", str3);
    QCOMPARE((const char *)str3, str1);
    QCOMPARE((const char *)str3, str2);
    QCOMPARE(str1, (const char *)str3);
    QCOMPARE(str2, (const char *)str3);
}

void tst_StrCmp::compare_bytearray()
{
    QByteArray ba = "foo";
    QEXPECT_FAIL("", "Next test should fail", Continue);
    QCOMPARE(ba.constData(), "bar");
    QCOMPARE(ba.constData(), "foo");

    char *bar = "bar";
    char *foo = "foo";

    QEXPECT_FAIL("", "Next test should fail", Continue);
    QCOMPARE(ba.data(), bar);
    QCOMPARE(ba.data(), foo);

    const char *cbar = "bar";
    const char *cfoo = "foo";

    QEXPECT_FAIL("", "Next test should fail", Continue);
    QCOMPARE(ba.constData(), cbar);
    QCOMPARE(ba.constData(), cfoo);
}

QTEST_MAIN(tst_StrCmp)

#include "tst_strcmp.moc"

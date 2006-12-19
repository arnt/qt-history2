/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qvarlengtharray.h>

const int N = 1;

//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qvarlengtharray.h corelib/tools/qvarlengtharray.cpp

class tst_QVarLengthArray : public QObject
{
    Q_OBJECT

public:
    tst_QVarLengthArray() {}
    virtual ~tst_QVarLengthArray() {}

private slots:
    void oldTests();
};

int fooCtor = 0;
int fooDtor = 0;

struct Foo
{
    int *p;

    Foo() { p = new int; ++fooCtor; }
    Foo(const Foo &other) { p = new int; ++fooCtor; }

    void operator=(const Foo & /* other */) { }

    ~Foo() { delete p; ++fooDtor; }
};

void tst_QVarLengthArray::oldTests()
{
    {
	QVarLengthArray<int, 256> sa(128);
	QVERIFY(sa.data() == &sa[0]);
	sa[0] = 0xfee;
	sa[10] = 0xff;
	QVERIFY(sa[0] == 0xfee);
	QVERIFY(sa[10] == 0xff);
	sa.resize(512);
	QVERIFY(sa.data() == &sa[0]);
	QVERIFY(sa[0] == 0xfee);
	QVERIFY(sa[10] == 0xff);
	QVERIFY(sa.size() == 512);
	sa.reserve(1024);
	QVERIFY(sa.capacity() == 1024);
	QVERIFY(sa.size() == 512);
    }
    {
	QVarLengthArray<QString> sa(10);
	sa[0] = "Hello";
	sa[9] = "World";
	QVERIFY(*sa.data() == "Hello");
	QVERIFY(sa[9] == "World");
	sa.reserve(512);
	QVERIFY(*sa.data() == "Hello");
	QVERIFY(sa[9] == "World");
	sa.resize(512);
	QVERIFY(*sa.data() == "Hello");
	QVERIFY(sa[9] == "World");
    }
    {
        int arr[2] = {1, 2};
        QVarLengthArray<int> sa(10);
        QCOMPARE(sa.size(), 10);
        sa.append(arr, 2);
        QCOMPARE(sa.size(), 12);
        QCOMPARE(sa[10], 1);
        QCOMPARE(sa[11], 2);
    }
    {
        QString arr[2] = { QString("hello"), QString("world") };
        QVarLengthArray<QString> sa(10);
        QCOMPARE(sa.size(), 10);
        sa.append(arr, 2);
        QCOMPARE(sa.size(), 12);
        QCOMPARE(sa[10], QString("hello"));
        QCOMPARE(sa[11], QString("world"));

        sa.append(arr, 1);
        QCOMPARE(sa.size(), 13);
        QCOMPARE(sa[12], QString("hello"));

        sa.append(arr, 0);
        QCOMPARE(sa.size(), 13);
    }
    {
        // assignment operator and copy constructor

        QVarLengthArray<int> sa(10);
        sa[5] = 5;

        QVarLengthArray<int> sa2(10);
        sa2[5] = 6;
        sa2 = sa;
        QCOMPARE(sa2[5], 5);

        QVarLengthArray<int> sa3(sa);
        QCOMPARE(sa3[5], 5);
    }

    {
        QVarLengthArray<Foo> a;
        const int N = 0x7fffffff / sizeof(Foo);
        const int Prealloc = a.capacity();
        const Foo *data0 = a.constData();

        a.resize(N);
        if (a.size() == N) {
            // it's a miracle!
            QVERIFY(a.capacity() >= N);
            QCOMPARE(fooCtor, N);
            QCOMPARE(fooDtor, 0);

            for (int i = 0; i < N; i += 35000)
                a[i] = Foo();
        } else {
            // this is the case we're actually testing
            QCOMPARE(a.size(), 0);
            QCOMPARE(a.capacity(), Prealloc);
            QCOMPARE(a.constData(), data0);
            QCOMPARE(fooCtor, 0);
            QCOMPARE(fooDtor, 0);

            a.resize(5);
            QCOMPARE(a.size(), 5);
            QCOMPARE(a.capacity(), Prealloc);
            QCOMPARE(a.constData(), data0);
            QCOMPARE(fooCtor, 5);
            QCOMPARE(fooDtor, 0);

            a.resize(Prealloc + 1);
            QCOMPARE(a.size(), Prealloc + 1);
            QVERIFY(a.capacity() >= Prealloc + 1);
            QVERIFY(a.constData() != data0);
            QCOMPARE(fooCtor, Prealloc + 6);
            QCOMPARE(fooDtor, 5);

            const Foo *data1 = a.constData();

            a.resize(0x10000000);
            QCOMPARE(a.size(), 0);
            QVERIFY(a.capacity() >= Prealloc + 1);
            QVERIFY(a.constData() == data1);
            QCOMPARE(fooCtor, Prealloc + 6);
            QCOMPARE(fooDtor, Prealloc + 6);
        }
    }
}

QTEST_APPLESS_MAIN(tst_QVarLengthArray)
#include "tst_qvarlengtharray.moc"

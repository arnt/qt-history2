/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <q3strlist.h>




#include <qdatetime.h>

//TESTED_CLASS=
//TESTED_FILES=compat/tools/q3ptrlist.h

class tst_Q3PtrList : public QObject
{
Q_OBJECT

public:
    tst_Q3PtrList();
    virtual ~tst_Q3PtrList();


public slots:
    void init();
    void cleanup();
private slots:
    void replace();
    void replaceStrDeep();
    void replaceStrShallow();
    void take();
    void removeType();
};

tst_Q3PtrList::tst_Q3PtrList()
{
}

tst_Q3PtrList::~tst_Q3PtrList()
{

}

void tst_Q3PtrList::init()
{
}

void tst_Q3PtrList::cleanup()
{
}

void tst_Q3PtrList::replace()
{
#if QT_VERSION >= 0x030100
    Q3PtrList<int> list;
    int foo = 4;
    list.setAutoDelete( TRUE );
    QCOMPARE( list.insert(0, new int(1)), (bool)TRUE );
    QCOMPARE( list.insert(1, new int(2)), (bool)TRUE );
    QCOMPARE( list.insert(2, new int(4)), (bool)TRUE );

    QCOMPARE( *(list.at(2)), 4 );
    QCOMPARE( list.replace(2, new int(3)), (bool)TRUE );
    QCOMPARE( *(list.at(2)), 3 );
    uint count = list.count();
    QCOMPARE( list.replace(3, &foo), (bool)FALSE );
    QCOMPARE( list.count(), count );

    int *p = new int(7);
    QCOMPARE( list.insert(2, p), (bool)TRUE );
    QCOMPARE( list.replace(2, p), (bool)TRUE );

#else
    QSKIP( "Not tested with Qt versions < 3.1", SkipAll);
#endif
}

void tst_Q3PtrList::replaceStrDeep()
{
#if QT_VERSION >= 0x030100
    Q3StrList list;
    const char *str;

    QCOMPARE( list.insert(0, "This is string 1"), (bool)TRUE );
    QCOMPARE( list.insert(1, "This is string 2"), (bool)TRUE );
    QCOMPARE( list.insert(2, "This is string 3"), (bool)TRUE );

    QCOMPARE( strcmp(list.at(2), "This is string 3"), 0 );
    QCOMPARE( list.replace(2, "Replaced String"), (bool)TRUE );
    QCOMPARE( strcmp(list.at(2), "Replaced String"), 0 );
    uint count = list.count();

    str = "TEST";
    QCOMPARE( list.replace(3, str), (bool)FALSE );
    QCOMPARE( list.count(), count );

    QCOMPARE( list.insert(2, str), (bool)TRUE );
    QCOMPARE( list.replace(2, str), (bool)TRUE );
#else
    QSKIP( "Not tested with Qt versions < 3.1", SkipAll);
#endif
}

void tst_Q3PtrList::replaceStrShallow()
{
#if QT_VERSION >= 0x030100
    Q3StrList list( FALSE );
    char str1[] = "This is string 1";
    char str2[] = "This is string 2";
    char str3[] = "This is string 3";
    char str4[] = "Replace";

    QCOMPARE( list.insert(0, str1), (bool)TRUE );
    QCOMPARE( list.insert(1, str2), (bool)TRUE );
    QCOMPARE( list.insert(2, str3), (bool)TRUE );

    QCOMPARE( strcmp(list.at(2), str3), 0 );
    QCOMPARE( list.replace(2, str4), (bool)TRUE );
    QCOMPARE( strcmp(list.at(2), str4), 0 );
    uint count = list.count();

    char str[] = "TEST";
    QCOMPARE( list.replace(3, str), (bool)FALSE );
    QCOMPARE( list.count(), count );

    QCOMPARE( list.insert(2, str), (bool)TRUE );
    QCOMPARE( list.replace(2, str), (bool)TRUE );
#else
    QSKIP( "Not tested with Qt versions < 3.1", SkipAll);
#endif
}

void tst_Q3PtrList::take()
{
    Q3PtrList<int> list;
    QVERIFY(list.take(0) == 0);
    QVERIFY(list.take(list.count()) == 0);
}

void tst_Q3PtrList::removeType()
{
    Q3PtrList<QString> items;
    items.append(new QString("first"));
    QString *second = new QString("second");
    items.append(second);
    QString *third = new QString("third");
    items.append(third);
    QString *fourth = new QString("fourth");
    items.append(fourth);

    QVERIFY(items.current() == fourth);
    items.setAutoDelete(FALSE);

    // this test an undocumented feature of remove( NULL )
    // in QGList::remove if the ptr is 0 it removes the current item
    // ie. it removes the fourth item from the list in this case
    QString *nullPointer = NULL;
    items.remove( nullPointer );
    QVERIFY(items.count() == 3);
    QVERIFY(items.current() == third);

    // this tests that remove updates the current item also
    // when it removes the _end_ item in the list
    items.remove(third);
    QVERIFY(items.current() == second);

    // test that the removed items are not in the list, then deletes them
    QVERIFY(third && items.find(third) == -1 );
    QVERIFY(fourth && items.find(fourth) == -1);
    delete third;
    delete fourth;
    fourth = third = 0;

    items.setAutoDelete(TRUE);
}

QTEST_APPLESS_MAIN(tst_Q3PtrList)
#include "tst_q3ptrlist.moc"

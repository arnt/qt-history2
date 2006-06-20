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




//TESTED_CLASS=QAtomicPointer
//TESTED_FILES=

class tst_QAtomicPointer : public QObject
{
    Q_OBJECT

public:
    tst_QAtomicPointer();
    ~tst_QAtomicPointer();

private slots:
    void constructor();
    void copy_constructor();
    void equality_operator();
    void inequality_operator();
    void assignment_operator();
    void star_operator();
    void dereference_operator();
    void testAndSet();
    void exchange();
};

tst_QAtomicPointer::tst_QAtomicPointer()
{ }

tst_QAtomicPointer::~tst_QAtomicPointer()
{ }

void tst_QAtomicPointer::constructor()
{
    void *one = this;
    QAtomicPointer<void> atomic1 = one;
    QVERIFY(atomic1 == one);

    void *two = &one;
    QAtomicPointer<void> atomic2 = two;
    QVERIFY(atomic2 == two);

    void *three = &two;
    QAtomicPointer<void> atomic3 = three;
    QVERIFY(atomic3 == three);
}

void tst_QAtomicPointer::copy_constructor()
{
    void *one = this;
    QAtomicPointer<void> atomic1 = one;
    QAtomicPointer<void> atomic1_copy = atomic1;
    QVERIFY(atomic1_copy == one);
    QCOMPARE(atomic1_copy, atomic1);

    void *two = &one;
    QAtomicPointer<void> atomic2 = two;
    QAtomicPointer<void> atomic2_copy = atomic2;
    QVERIFY(atomic2_copy == two);
    QCOMPARE(atomic2_copy, atomic2);

    void *three = &two;
    QAtomicPointer<void> atomic3 = three;
    QAtomicPointer<void> atomic3_copy = atomic3;
    QVERIFY(atomic3_copy == three);
    QCOMPARE(atomic3_copy, atomic3);
}

void tst_QAtomicPointer::equality_operator()
{
    void *one = this;
    void *two = &one;
    void *three = &two;

    QAtomicPointer<void> atomic1 = one;
    QAtomicPointer<void> atomic2 = two;
    QAtomicPointer<void> atomic3 = three;

    QVERIFY(atomic1 == one);
    QVERIFY(!(atomic1 == two));
    QVERIFY(!(atomic1 == three));

    QVERIFY(!(atomic2 == one));
    QVERIFY(atomic2 == two);
    QVERIFY(!(atomic2 == three));

    QVERIFY(!(atomic3 == one));
    QVERIFY(!(atomic3 == two));
    QVERIFY(atomic3 == three);
}

void tst_QAtomicPointer::inequality_operator()
{
    void *one = this;
    void *two = &one;
    void *three = &two;

    QAtomicPointer<void> atomic1 = one;
    QAtomicPointer<void> atomic2 = two;
    QAtomicPointer<void> atomic3 = three;

    QVERIFY(!(atomic1 != one));
    QVERIFY(atomic1 != two);
    QVERIFY(atomic1 != three);

    QVERIFY(atomic2 != one);
    QVERIFY(!(atomic2 != two));
    QVERIFY(atomic2 != three);

    QVERIFY(atomic3 != one);
    QVERIFY(atomic3 != two);
    QVERIFY(!(atomic3 != three));
}

void tst_QAtomicPointer::assignment_operator()
{
    void *one = this;
    void *two = &one;
    void *three = &two;

    QAtomicPointer<void> atomic1 = one;
    QAtomicPointer<void> atomic2 = two;
    QAtomicPointer<void> atomic3 = three;

    QVERIFY(atomic1 == one);
    QVERIFY(atomic2 == two);
    QVERIFY(atomic3 == three);

    atomic1 = two;
    atomic2 = three;
    atomic3 = one;

    QVERIFY(atomic1 == two);
    QVERIFY(atomic2 == three);
    QVERIFY(atomic3 == one);
}

struct Type
{
    inline const Type *self() const
    { return this; }
};

void tst_QAtomicPointer::star_operator()
{
    Type t;
    QAtomicPointer<Type> p = &t;
    QCOMPARE((*p).self(), t.self());
}

void tst_QAtomicPointer::dereference_operator()
{
    Type t;
    QAtomicPointer<Type> p = &t;
    QCOMPARE(p->self(), t.self());
}

void tst_QAtomicPointer::testAndSet()
{
    void *one = this;
    void *two = &one;
    void *three = &two;

    QAtomicPointer<void> atomic1 = one;
    QAtomicPointer<void> atomic2 = two;
    QAtomicPointer<void> atomic3 = three;

    QVERIFY(atomic1 == one);
    QVERIFY(atomic2 == two);
    QVERIFY(atomic3 == three);

    QVERIFY(atomic1.testAndSet(one, two));
    QVERIFY(atomic2.testAndSet(two, three));
    QVERIFY(atomic3.testAndSet(three, one));

    QVERIFY(atomic1 == two);
    QVERIFY(atomic2 == three);
    QVERIFY(atomic3 == one);
}

void tst_QAtomicPointer::exchange()
{
    void *one = this;
    void *two = &one;
    void *three = &two;

    QAtomicPointer<void> atomic1 = one;
    QAtomicPointer<void> atomic2 = two;
    QAtomicPointer<void> atomic3 = three;

    QVERIFY(atomic1 == one);
    QVERIFY(atomic2 == two);
    QVERIFY(atomic3 == three);

    QCOMPARE(atomic1.exchange(two), one);
    QCOMPARE(atomic2.exchange(three), two);
    QCOMPARE(atomic3.exchange(one), three);

    QVERIFY(atomic1 == two);
    QVERIFY(atomic2 == three);
    QVERIFY(atomic3 == one);
}

QTEST_APPLESS_MAIN(tst_QAtomicPointer)
#include "tst_qatomicpointer.moc"

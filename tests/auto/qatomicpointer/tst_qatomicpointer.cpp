/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

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

    void isTestAndSetNative();
    void isTestAndSetWaitFree();
    void testAndSet();

    void isFetchAndStoreNative();
    void isFetchAndStoreWaitFree();
    void fetchAndStore();

    void isFetchAndAddNative();
    void isFetchAndAddWaitFree();
    void fetchAndAdd_data();
    void fetchAndAdd();
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

void tst_QAtomicPointer::isTestAndSetNative()
{
#if defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE)
    // the runtime test should say the same thing
    QVERIFY(QAtomicPointer<void>::isTestAndSetNative());

#  if (defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE)     \
       || defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_NOT_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_TEST_AND_SET_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#elif defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE)
    // could be either, just want to make sure the function is implemented
    QVERIFY(QAtomicPointer<void>::isTestAndSetNative()
            || !QAtomicPointer<void>::isTestAndSetNative());

#  if (defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE) \
       || defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_NOT_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_TEST_AND_SET_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#elif defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_NOT_NATIVE)
    // the runtime test should say the same thing
    QVERIFY(!QAtomicPointer<void>::isTestAndSetNative());

#  if (defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE) \
       || defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_TEST_AND_SET_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#else
#  error "Q_ATOMIC_POINTER_TEST_AND_SET_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE is not defined"
#endif
}

void tst_QAtomicPointer::isTestAndSetWaitFree()
{
#if defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_WAIT_FREE)
    // the runtime test should say the same thing
    QVERIFY(QAtomicPointer<void>::isTestAndSetWaitFree());

    // enforce some invariants
    QVERIFY(QAtomicPointer<void>::isTestAndSetNative());
#  if defined(Q_ATOMIC_POINTER_TEST_AND_SET_IS_NOT_NATIVE)
#    error "Reference counting cannot be wait-free and unsupported at the same time!"
#  endif
#else
    // the runtime test should say the same thing
    QVERIFY(!QAtomicPointer<void>::isTestAndSetWaitFree());
#endif
}

void tst_QAtomicPointer::testAndSet()
{
    void *one = this;
    void *two = &one;
    void *three = &two;

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QVERIFY(atomic1.testAndSetRelaxed(one, two));
        QVERIFY(atomic2.testAndSetRelaxed(two, three));
        QVERIFY(atomic3.testAndSetRelaxed(three, one));

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QVERIFY(atomic1.testAndSetAcquire(one, two));
        QVERIFY(atomic2.testAndSetAcquire(two, three));
        QVERIFY(atomic3.testAndSetAcquire(three, one));

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QVERIFY(atomic1.testAndSetRelease(one, two));
        QVERIFY(atomic2.testAndSetRelease(two, three));
        QVERIFY(atomic3.testAndSetRelease(three, one));

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QVERIFY(atomic1.testAndSetOrdered(one, two));
        QVERIFY(atomic2.testAndSetOrdered(two, three));
        QVERIFY(atomic3.testAndSetOrdered(three, one));

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }
}

void tst_QAtomicPointer::isFetchAndStoreNative()
{
#if defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE)
    // the runtime test should say the same thing
    QVERIFY(QAtomicPointer<void>::isFetchAndStoreNative());

#  if (defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_SOMETIMES_NATIVE)     \
       || defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_NOT_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#elif defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_SOMETIMES_NATIVE)
    // could be either, just want to make sure the function is implemented
    QVERIFY(QAtomicPointer<void>::isFetchAndStoreNative()
            || !QAtomicPointer<void>::isFetchAndStoreNative());

#  if (defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE) \
       || defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_NOT_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#elif defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_NOT_NATIVE)
    // the runtime test should say the same thing
    QVERIFY(!QAtomicPointer<void>::isFetchAndStoreNative());

#  if (defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE) \
       || defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_SOMETIMES_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#else
#  error "Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE is not defined"
#endif
}

void tst_QAtomicPointer::isFetchAndStoreWaitFree()
{
#if defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_WAIT_FREE)
    // the runtime test should say the same thing
    QVERIFY(QAtomicPointer<void>::isFetchAndStoreWaitFree());

    // enforce some invariants
    QVERIFY(QAtomicPointer<void>::isFetchAndStoreNative());
#  if defined(Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_NOT_NATIVE)
#    error "Reference counting cannot be wait-free and unsupported at the same time!"
#  endif
#else
    // the runtime test should say the same thing
    QVERIFY(!QAtomicPointer<void>::isFetchAndStoreWaitFree());
#endif
}

void tst_QAtomicPointer::fetchAndStore()
{
    void *one = this;
    void *two = &one;
    void *three = &two;

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QCOMPARE(atomic1.fetchAndStoreRelaxed(two), one);
        QCOMPARE(atomic2.fetchAndStoreRelaxed(three), two);
        QCOMPARE(atomic3.fetchAndStoreRelaxed(one), three);

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QCOMPARE(atomic1.fetchAndStoreAcquire(two), one);
        QCOMPARE(atomic2.fetchAndStoreAcquire(three), two);
        QCOMPARE(atomic3.fetchAndStoreAcquire(one), three);

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QCOMPARE(atomic1.fetchAndStoreRelease(two), one);
        QCOMPARE(atomic2.fetchAndStoreRelease(three), two);
        QCOMPARE(atomic3.fetchAndStoreRelease(one), three);

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }

    {
        QAtomicPointer<void> atomic1 = one;
        QAtomicPointer<void> atomic2 = two;
        QAtomicPointer<void> atomic3 = three;

        QVERIFY(atomic1 == one);
        QVERIFY(atomic2 == two);
        QVERIFY(atomic3 == three);

        QCOMPARE(atomic1.fetchAndStoreOrdered(two), one);
        QCOMPARE(atomic2.fetchAndStoreOrdered(three), two);
        QCOMPARE(atomic3.fetchAndStoreOrdered(one), three);

        QVERIFY(atomic1 == two);
        QVERIFY(atomic2 == three);
        QVERIFY(atomic3 == one);
    }
}

void tst_QAtomicPointer::isFetchAndAddNative()
{
#if defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE)
    // the runtime test should say the same thing
    QVERIFY(QAtomicPointer<void>::isFetchAndAddNative());

#  if (defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_SOMETIMES_NATIVE)     \
       || defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_NOT_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#elif defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_SOMETIMES_NATIVE)
    // could be either, just want to make sure the function is implemented
    QVERIFY(QAtomicPointer<void>::isFetchAndAddNative()
            || !QAtomicPointer<void>::isFetchAndAddNative());

#  if (defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE) \
       || defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_NOT_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#elif defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_NOT_NATIVE)
    // the runtime test should say the same thing
    QVERIFY(!QAtomicPointer<void>::isFetchAndAddNative());

#  if (defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE) \
       || defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_SOMETIMES_NATIVE))
#    error "Define only one of Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE"
#  endif
#else
#  error "Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_{ALWAYS,SOMTIMES,NOT}_NATIVE is not defined"
#endif
}

void tst_QAtomicPointer::isFetchAndAddWaitFree()
{
#if defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_WAIT_FREE)
    // the runtime test should say the same thing
    QVERIFY(QAtomicPointer<void>::isFetchAndAddWaitFree());

    // enforce some invariants
    QVERIFY(QAtomicPointer<void>::isFetchAndAddNative());
#  if defined(Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_NOT_NATIVE)
#    error "Reference counting cannot be wait-free and unsupported at the same time!"
#  endif
#else
    // the runtime test should say the same thing
    QVERIFY(!QAtomicPointer<void>::isFetchAndAddWaitFree());
#endif
}

void tst_QAtomicPointer::fetchAndAdd_data()
{
    QTest::addColumn<int>("valueToAdd");

    QTest::newRow("0") << 0;
    QTest::newRow("1") << 1;
    QTest::newRow("2") << 2;
    QTest::newRow("10") << 10;
    QTest::newRow("31") << 31;
    QTest::newRow("51") << 51;
    QTest::newRow("72") << 72;
    QTest::newRow("810") << 810;
    QTest::newRow("631") << 631;
    QTest::newRow("451") << 451;
    QTest::newRow("272") << 272;
    QTest::newRow("1810") << 1810;
    QTest::newRow("3631") << 3631;
    QTest::newRow("5451") << 5451;
    QTest::newRow("7272") << 7272;
    QTest::newRow("-1") << -1;
    QTest::newRow("-2") << -2;
    QTest::newRow("-10") << -10;
    QTest::newRow("-31") << -31;
    QTest::newRow("-51") << -51;
    QTest::newRow("-72") << -72;
    QTest::newRow("-810") << -810;
    QTest::newRow("-631") << -631;
    QTest::newRow("-451") << -451;
    QTest::newRow("-272") << -272;
    QTest::newRow("-1810") << -1810;
    QTest::newRow("-3631") << -3631;
    QTest::newRow("-5451") << -5451;
    QTest::newRow("-7272") << -7272;
}

void tst_QAtomicPointer::fetchAndAdd()
{
    QFETCH(int, valueToAdd);

    char c;
    char *pc = &c;
    short s;
    short *ps = &s;
    int i;
    int *pi = &i;

    {
        QAtomicPointer<char> pointer1 = pc;
        QCOMPARE(quintptr(pointer1.fetchAndAddRelaxed(valueToAdd)), quintptr(pc));
        QCOMPARE(quintptr(pointer1.fetchAndAddRelaxed(-valueToAdd)), quintptr(pc + valueToAdd));
        QCOMPARE(quintptr(static_cast<char *>(pointer1)), quintptr(pc));
        QAtomicPointer<short> pointer2 = ps;
        QCOMPARE(quintptr(pointer2.fetchAndAddRelaxed(valueToAdd)), quintptr(ps));
        QCOMPARE(quintptr(pointer2.fetchAndAddRelaxed(-valueToAdd)), quintptr(ps + valueToAdd));
        QCOMPARE(quintptr(static_cast<short *>(pointer2)), quintptr(ps));
        QAtomicPointer<int> pointer3 = pi;
        QCOMPARE(quintptr(pointer3.fetchAndAddRelaxed(valueToAdd)), quintptr(pi));
        QCOMPARE(quintptr(pointer3.fetchAndAddRelaxed(-valueToAdd)), quintptr(pi + valueToAdd));
        QCOMPARE(quintptr(static_cast<int *>(pointer3)), quintptr(pi));
    }

    {
        QAtomicPointer<char> pointer1 = pc;
        QCOMPARE(quintptr(pointer1.fetchAndAddAcquire(valueToAdd)), quintptr(pc));
        QCOMPARE(quintptr(pointer1.fetchAndAddAcquire(-valueToAdd)), quintptr(pc + valueToAdd));
        QCOMPARE(quintptr(static_cast<char *>(pointer1)), quintptr(pc));
        QAtomicPointer<short> pointer2 = ps;
        QCOMPARE(quintptr(pointer2.fetchAndAddAcquire(valueToAdd)), quintptr(ps));
        QCOMPARE(quintptr(pointer2.fetchAndAddAcquire(-valueToAdd)), quintptr(ps + valueToAdd));
        QCOMPARE(quintptr(static_cast<short *>(pointer2)), quintptr(ps));
        QAtomicPointer<int> pointer3 = pi;
        QCOMPARE(quintptr(pointer3.fetchAndAddAcquire(valueToAdd)), quintptr(pi));
        QCOMPARE(quintptr(pointer3.fetchAndAddAcquire(-valueToAdd)), quintptr(pi + valueToAdd));
        QCOMPARE(quintptr(static_cast<int *>(pointer3)), quintptr(pi));
    }

    {
        QAtomicPointer<char> pointer1 = pc;
        QCOMPARE(quintptr(pointer1.fetchAndAddRelease(valueToAdd)), quintptr(pc));
        QCOMPARE(quintptr(pointer1.fetchAndAddRelease(-valueToAdd)), quintptr(pc + valueToAdd));
        QCOMPARE(quintptr(static_cast<char *>(pointer1)), quintptr(pc));
        QAtomicPointer<short> pointer2 = ps;
        QCOMPARE(quintptr(pointer2.fetchAndAddRelease(valueToAdd)), quintptr(ps));
        QCOMPARE(quintptr(pointer2.fetchAndAddRelease(-valueToAdd)), quintptr(ps + valueToAdd));
        QCOMPARE(quintptr(static_cast<short *>(pointer2)), quintptr(ps));
        QAtomicPointer<int> pointer3 = pi;
        QCOMPARE(quintptr(pointer3.fetchAndAddRelease(valueToAdd)), quintptr(pi));
        QCOMPARE(quintptr(pointer3.fetchAndAddRelease(-valueToAdd)), quintptr(pi + valueToAdd));
        QCOMPARE(quintptr(static_cast<int *>(pointer3)), quintptr(pi));
    }

    {
        QAtomicPointer<char> pointer1 = pc;
        QCOMPARE(quintptr(pointer1.fetchAndAddOrdered(valueToAdd)), quintptr(pc));
        QCOMPARE(quintptr(pointer1.fetchAndAddOrdered(-valueToAdd)), quintptr(pc + valueToAdd));
        QCOMPARE(quintptr(static_cast<char *>(pointer1)), quintptr(pc));
        QAtomicPointer<short> pointer2 = ps;
        QCOMPARE(quintptr(pointer2.fetchAndAddOrdered(valueToAdd)), quintptr(ps));
        QCOMPARE(quintptr(pointer2.fetchAndAddOrdered(-valueToAdd)), quintptr(ps + valueToAdd));
        QCOMPARE(quintptr(static_cast<short *>(pointer2)), quintptr(ps));
        QAtomicPointer<int> pointer3 = pi;
        QCOMPARE(quintptr(pointer3.fetchAndAddOrdered(valueToAdd)), quintptr(pi));
        QCOMPARE(quintptr(pointer3.fetchAndAddOrdered(-valueToAdd)), quintptr(pi + valueToAdd));
        QCOMPARE(quintptr(static_cast<int *>(pointer3)), quintptr(pi));
    }
}

QTEST_APPLESS_MAIN(tst_QAtomicPointer)
#include "tst_qatomicpointer.moc"

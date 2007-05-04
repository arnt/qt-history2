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

#ifdef Q_OS_LINUX
# include <pthread.h>
#endif

Q_DECLARE_METATYPE(QMetaType::Type)

class tst_QMetaType: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QVariant> prop READ prop WRITE setProp)

public:
    tst_QMetaType() { propList << 42 << "Hello"; }

    QList<QVariant> prop() const { return propList; }
    void setProp(const QList<QVariant> &list) { propList = list; }

private:
    QList<QVariant> propList;

private slots:
    void defined();
    void threadSafety();
    void namespaces();
    void qMetaTypeId();
    void properties();
    void normalizedTypes();
    void typeName_data();
    void typeName();
    void construct();
    void typedefs();
    void isRegistered_data();
    void isRegistered();
};

struct Foo { int i; };

void tst_QMetaType::defined()
{
    QCOMPARE(int(QMetaTypeId2<QString>::Defined), 1);
    QCOMPARE(int(QMetaTypeId2<Foo>::Defined), 0);
    QCOMPARE(int(QMetaTypeId2<void*>::Defined), 1);
    QCOMPARE(int(QMetaTypeId2<int*>::Defined), 0);
}

struct Bar
{
    Bar()
    {
        // check re-entrancy
        Q_ASSERT(QMetaType::isRegistered(qRegisterMetaType<Foo>("Foo")));
    }
};

class MetaTypeTorturer: public QThread
{
    Q_OBJECT
protected:
    void run()
    {
        for (int i = 0; i < 1000; ++i) {
            const QByteArray name = QString("Bar%1_%2").arg(i).arg((size_t)QThread::currentThreadId()).toLatin1();
            const char *nm = name.constData();
            int tp = qRegisterMetaType<Bar>(nm);
#ifdef Q_OS_LINUX
            pthread_yield();
#endif
            Q_ASSERT(QMetaType::isRegistered(tp));
            Q_ASSERT(QMetaType::type(nm) == tp);
            Q_ASSERT(QMetaType::typeName(tp) == name);
            void *buf = QMetaType::construct(tp, 0);
            void *buf2 = QMetaType::construct(tp, buf);
            Q_ASSERT(buf);
            Q_ASSERT(buf2);
            QMetaType::destroy(tp, buf);
            QMetaType::destroy(tp, buf2);
        }
    }
};

void tst_QMetaType::threadSafety()
{
#if QT_VERSION >= 0x040100
    MetaTypeTorturer t1;
    MetaTypeTorturer t2;
    MetaTypeTorturer t3;

    t1.start();
    t2.start();
    t3.start();

    QVERIFY(t1.wait());
    QVERIFY(t2.wait());
    QVERIFY(t3.wait());
#endif
}

namespace TestSpace
{
    struct Foo { double d; };

}
Q_DECLARE_METATYPE(TestSpace::Foo)

void tst_QMetaType::namespaces()
{
    TestSpace::Foo nf = { 11.12 };
    QVariant v = qVariantFromValue(nf);
    QCOMPARE(qvariant_cast<TestSpace::Foo>(v).d, 11.12);
}

void tst_QMetaType::qMetaTypeId()
{
    QCOMPARE(::qMetaTypeId<QString>(), int(QMetaType::QString));
    QCOMPARE(::qMetaTypeId<int>(), int(QMetaType::Int));
    QCOMPARE(::qMetaTypeId<TestSpace::Foo>(), QMetaType::type("TestSpace::Foo"));
}

void tst_QMetaType::properties()
{
    qRegisterMetaType<QList<QVariant> >("QList<QVariant>");

    QVariant v = property("prop");

    QCOMPARE(v.typeName(), "QVariantList");

    QList<QVariant> values = v.toList();
    QCOMPARE(values.count(), 2);
    QCOMPARE(values.at(0).toInt(), 42);

    values << 43 << "world";

    QVERIFY(setProperty("prop", values));
    v = property("prop");
    QCOMPARE(v.toList().count(), 4);
}

template <typename T>
struct Whity { T t; };

Q_DECLARE_METATYPE( Whity < int > )
Q_DECLARE_METATYPE(Whity<double>)

void tst_QMetaType::normalizedTypes()
{
    int WhityIntId = ::qMetaTypeId<Whity<int> >();
    int WhityDoubleId = ::qMetaTypeId<Whity<double> >();

    QCOMPARE(QMetaType::type("Whity<int>"), WhityIntId);
    QCOMPARE(QMetaType::type(" Whity < int > "), WhityIntId);
    QCOMPARE(QMetaType::type("Whity<int >"), WhityIntId);

    QCOMPARE(QMetaType::type("Whity<double>"), WhityDoubleId);
    QCOMPARE(QMetaType::type(" Whity< double > "), WhityDoubleId);
    QCOMPARE(QMetaType::type("Whity<double >"), WhityDoubleId);

    QCOMPARE(qRegisterMetaType<Whity<int> >(" Whity < int > "), WhityIntId);
    QCOMPARE(qRegisterMetaType<Whity<int> >("Whity<int>"), WhityIntId);
    QCOMPARE(qRegisterMetaType<Whity<int> >("Whity<int > "), WhityIntId);

    QCOMPARE(qRegisterMetaType<Whity<double> >(" Whity < double > "), WhityDoubleId);
    QCOMPARE(qRegisterMetaType<Whity<double> >("Whity<double>"), WhityDoubleId);
    QCOMPARE(qRegisterMetaType<Whity<double> >("Whity<double > "), WhityDoubleId);
}

void tst_QMetaType::typeName_data()
{
    QTest::addColumn<QMetaType::Type>("aType");
    QTest::addColumn<QString>("aTypeName");

    QTest::newRow("void") << QMetaType::Void << "void";
    QTest::newRow("int") << QMetaType::Int << "int";
    QTest::newRow("double") << QMetaType::Double << "double";
    QTest::newRow("qlonglong") << QMetaType::LongLong << "qlonglong";
    QTest::newRow("QRegExp") << QMetaType::QRegExp << "QRegExp";
    QTest::newRow("QColorGroup") << QMetaType::Type(63) << "QColorGroup";
    QTest::newRow("void*") << QMetaType::VoidStar << "void*";
    QTest::newRow("ulong") << QMetaType::ULong << "ulong";
    QTest::newRow("QWidget*") << QMetaType::QWidgetStar << "QWidget*";
}

void tst_QMetaType::typeName()
{
    QFETCH(QMetaType::Type, aType);
    QFETCH(QString, aTypeName);

    QCOMPARE(QString::fromLatin1(QMetaType::typeName(aType)), aTypeName);
}

void tst_QMetaType::construct()
{
    QSize x(1, 1);
    void *size = QMetaType::construct(QMetaType::QSize, &x);
    QVERIFY(size);
    QCOMPARE(static_cast<QSize *>(size)->width(), 1);
    QMetaType::destroy(QMetaType::QSize, size);
}

void tst_QMetaType::typedefs()
{
    QCOMPARE(QMetaType::type("long long"), int(QMetaType::LongLong));
    QCOMPARE(QMetaType::type("unsigned long long"), int(QMetaType::ULongLong));
    QCOMPARE(QMetaType::type("qint8"), int(QMetaType::Char));
    QCOMPARE(QMetaType::type("quint8"), int(QMetaType::UChar));
    QCOMPARE(QMetaType::type("qint16"), int(QMetaType::Short));
    QCOMPARE(QMetaType::type("quint16"), int(QMetaType::UShort));
    QCOMPARE(QMetaType::type("qint32"), int(QMetaType::Int));
    QCOMPARE(QMetaType::type("quint32"), int(QMetaType::UInt));
    QCOMPARE(QMetaType::type("qint64"), int(QMetaType::LongLong));
    QCOMPARE(QMetaType::type("quint64"), int(QMetaType::ULongLong));

    // make sure the qreal typeId is the type id of the type it's defined to
    QCOMPARE(QMetaType::type("qreal"), ::qMetaTypeId<qreal>());
}

class IsRegisteredDummyType { };

void tst_QMetaType::isRegistered_data()
{
    QTest::addColumn<int>("typeId");
    QTest::addColumn<bool>("registered");

    // predefined/custom types
    QTest::newRow("QMetaType::Void") << int(QMetaType::Void) << true;
    QTest::newRow("QMetaType::Int") << int(QMetaType::Int) << true;

    int dummyTypeId = qRegisterMetaType<IsRegisteredDummyType>("IsRegisteredDummyType");

    QTest::newRow("IsRegisteredDummyType") << dummyTypeId << true;

    // unknown types
    QTest::newRow("-1") << -1 << false;
    QTest::newRow("-42") << -42 << false;
    QTest::newRow("IsRegisteredDummyType + 1") << (dummyTypeId + 1) << false;
}

void tst_QMetaType::isRegistered()
{
    QFETCH(int, typeId);
    QFETCH(bool, registered);
    QCOMPARE(QMetaType::isRegistered(typeId), registered);
}

QTEST_MAIN(tst_QMetaType)
#include "tst_qmetatype.moc"

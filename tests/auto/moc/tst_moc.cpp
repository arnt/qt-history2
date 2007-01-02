/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <QtTest/QtTest>
#include <stdio.h>
#include <qobject.h>

#include "using-namespaces.h"
#include "assign-namespace.h"
#include "no-keywords.h"
#include "backslash-newlines.h"
#include "slots-with-void-template.h"
#include "qinvokable.h"

#if defined(PARSE_BOOST)
#include "parse-boost.h"
#endif

// No such thing as "long long" in Microsoft's compiler 13.0 and before
#if defined Q_CC_MSVC && _MSC_VER <= 1310
#  define NOLONGLONG
#endif

struct MyStruct {};
struct MyStruct2 {};

struct SuperClass {};

// Try to avoid inserting for instance a comment with a quote between the following line and the Q_OBJECT
// That will make the test give a false positive.
const char* test_multiple_number_of_escapes =   "\\\"";
namespace MyNamespace
{
    class TestSuperClass : public QObject
    {
        Q_OBJECT
        public:
            inline TestSuperClass() {}
    };
}

namespace String
{
    typedef QString Type;
}

namespace Int
{
    typedef int Type;
}

typedef struct {
    int doNotConfuseMoc;
} OldStyleCStruct;

class Sender : public QObject
{
    Q_OBJECT

public:
    void sendValue(const String::Type& value)
    {
        emit send(value);
    }
    void sendValue(const Int::Type& value)
    {
        emit send(value);
    }

signals:
    void send(const String::Type&);
    void send(const Int::Type&);
};

class Receiver : public QObject
{
    Q_OBJECT
public:
    Receiver() : stringCallCount(0), intCallCount(0) {}

    int stringCallCount;
    int intCallCount;

public slots:
    void receive(const String::Type&) { stringCallCount++; }
    void receive(const Int::Type&)    { intCallCount++; }
};

#define MACRO_WITH_POSSIBLE_COMPILER_SPECIFIC_ATTRIBUTES

#define DONT_CONFUSE_MOC(klass) klass
#define DONT_CONFUSE_MOC_EVEN_MORE(klass, dummy, dummy2) klass

Q_DECLARE_METATYPE(MyStruct)
Q_DECLARE_METATYPE(MyStruct*)

#if QT_VERSION >= 0x040200
namespace myNS {
    struct Points
    {
        Points() : p1(0xBEEF), p2(0xBABE) { }
        int p1, p2;
    };
}

Q_DECLARE_METATYPE(myNS::Points)
#endif

class TestClassinfoWithEscapes: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("escaped", "\"bar\"")
    Q_CLASSINFO("\"escaped\"", "foo")
public slots:
    void slotWithAReallyLongName(int)
    { }
};

struct ForwardDeclaredStruct;

struct StructQObject : public QObject
{
    Q_OBJECT
public:
    void foo(struct ForwardDeclaredStruct *);
};

void StructQObject::foo(struct ForwardDeclaredStruct *)
{
    struct Inner {
        bool field;
    };

    struct Inner unusedVariable;
}

class TestClass : public MyNamespace::TestSuperClass, public DONT_CONFUSE_MOC(MyStruct),
                  public DONT_CONFUSE_MOC_EVEN_MORE(MyStruct2, dummy, ignored)
{
    Q_OBJECT
#if QT_VERSION >= 0x040101
    Q_CLASSINFO("help", QT_TR_NOOP("Opening this will let you configure something"))
#endif
#if QT_VERSION >= 0x040102
    Q_PROPERTY(short int shortIntProperty READ shortIntProperty)
    Q_PROPERTY(unsigned short int unsignedShortIntProperty READ unsignedShortIntProperty)
    Q_PROPERTY(signed short int signedShortIntProperty READ signedShortIntProperty)
    Q_PROPERTY(long int longIntProperty READ longIntProperty)
    Q_PROPERTY(unsigned long int unsignedLongIntProperty READ unsignedLongIntProperty)
    Q_PROPERTY(signed long int signedLongIntProperty READ signedLongIntProperty)
    Q_PROPERTY(long double longDoubleProperty READ longDoubleProperty)
#endif
#if QT_VERSION >= 0x040200
    Q_PROPERTY(myNS::Points points READ points WRITE setPoints)
#endif

    Q_CLASSINFO("Multi"
                "line",
                ""
                "This is a "
                "multiline Q_CLASSINFO"
                "")

    // a really really long string that we have to cut into pieces in the generated stringdata
    // table, otherwise msvc craps out
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.KCookieServer\" >\n"
"    <method name=\"findCookies\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\" />\n"
"      <arg direction=\"in\" type=\"x\" name=\"windowId\" />\n"
"      <arg direction=\"out\" type=\"s\" name=\"cookies\" />\n"
"    </method>\n"
"    <method name=\"findDomains\" >\n"
"      <arg direction=\"out\" type=\"as\" name=\"domains\" />\n"
"    </method>\n"
"    <method name=\"findCookies\" >\n"
"      <arg direction=\"in\" type=\"ai\" name=\"fields\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"domain\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"fqdn\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\" />\n"
"      <arg direction=\"out\" type=\"as\" name=\"cookies\" />\n"
"      <annotation value=\"QList&lt;int>\" name=\"com.trolltech.QtDBus.QtTypeName.In0\" />\n"
"    </method>\n"
"    <method name=\"findDOMCookies\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\" />\n"
"      <arg direction=\"in\" type=\"x\" name=\"windowId\" />\n"
"      <arg direction=\"out\" type=\"s\" name=\"cookies\" />\n"
"    </method>\n"
"    <method name=\"addCookies\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\" />\n"
"      <arg direction=\"in\" type=\"ay\" name=\"cookieHeader\" />\n"
"      <arg direction=\"in\" type=\"x\" name=\"windowId\"  />\n"
"    </method>\n"
"    <method name=\"deleteCookie\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"domain\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"fqdn\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\" />\n"
"    </method>\n"
"    <method name=\"deleteCookiesFromDomain\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"domain\" />\n"
"    </method>\n"
"    <method name=\"deleteSessionCookies\" >\n"
"      <arg direction=\"in\" type=\"x\" name=\"windowId\" />\n"
"    </method>\n"
"    <method name=\"deleteSessionCookiesFor\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"fqdn\" />\n"
"      <arg direction=\"in\" type=\"x\" name=\"windowId\" />\n"
"    </method>\n"
"    <method name=\"deleteAllCookies\" />\n"
"    <method name=\"addDOMCookies\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\" />\n"
"      <arg direction=\"in\" type=\"ay\" name=\"cookieHeader\" />\n"
"      <arg direction=\"in\" type=\"x\" name=\"windowId\" />\n"
"    </method>\n"
"    <method name=\"setDomainAdvice\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"advice\" />\n"
"    </method>\n"
"    <method name=\"getDomainAdvice\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\" />\n"
"      <arg direction=\"out\" type=\"s\" name=\"advice\" />\n"
"    </method>\n"
"    <method name=\"reloadPolicy\" />\n"
"    <method name=\"shutdown\" />\n"
"  </interface>\n"
        "")

public:
    inline TestClass() {}

private slots:
    inline void dummy1() MACRO_WITH_POSSIBLE_COMPILER_SPECIFIC_ATTRIBUTES {}
    inline void dummy2() MACRO_WITH_POSSIBLE_COMPILER_SPECIFIC_ATTRIBUTES const {}
    inline void dummy3() const MACRO_WITH_POSSIBLE_COMPILER_SPECIFIC_ATTRIBUTES {}

#ifndef NOLONGLONG
    void slotWithULongLong(unsigned long long) {}
#if QT_VERSION >= 0x040101
    void slotWithULongLongP(unsigned long long*) {}
#endif
    void slotWithULong(unsigned long) {}
    void slotWithLongLong(long long) {}
    void slotWithLong(long) {}
#endif

    void slotWithColonColonType(::Int::Type) {}

#if QT_VERSION >= 0x040101
    TestClass &slotWithReferenceReturnType() { return *this; }
#endif

#if (0 && 1) || 1
    void expressionEvaluationShortcut1() {}
#endif
#if (1 || 0) && 0
#else
    void expressionEvaluationShortcut2() {}
#endif

public slots:
    void slotWithArray(const double[3]) {}
    void slotWithNamedArray(const double namedArray[3]) {}
    void slotWithMultiArray(const double[3][4]) {}

#if QT_VERSION >= 0x040102
    short int shortIntProperty() { return 0; }
    unsigned short int unsignedShortIntProperty() { return 0; }
    signed short int signedShortIntProperty() { return 0; }
    long int longIntProperty() { return 0; }
    unsigned long int unsignedLongIntProperty() { return 0; }
    signed long int signedLongIntProperty() { return 0; }
    long double longDoubleProperty() { return 0.0; }
#endif

#if QT_VERSION >= 0x040200
    myNS::Points points() { return m_points; }
    void setPoints(myNS::Points points) { m_points = points; }
#endif

signals:
    void signalWithArray(const double[3]);
    void signalWithNamedArray(const double namedArray[3]);

private slots:
    // for tst_Moc::preprocessorConditionals
#if 0
    void invalidSlot() {}
#else
    void slotInElse() {}
#endif

#if 1
    void slotInIf() {}
#else
    void invalidSlot() {}
#endif

#if 0
    void invalidSlot() {}
#elif 0
#else
    void slotInLastElse() {}
#endif

#if 0
    void invalidSlot() {}
#elif 1
    void slotInElif() {}
#else
    void invalidSlot() {}
#endif


    friend class Receiver; // task #85783
signals:
    friend class Sender; // task #85783

public slots:
    void const slotWithSillyConst() {}

public:
    Q_INVOKABLE void const slotWithSillyConst2() {}

    // that one however should be fine
public slots:
    void slotWithVoidStar(void *) {}

#if QT_VERSION >= 0x040200
private:
     myNS::Points m_points;
#endif

private slots:
     inline virtual void blub1() {}
     virtual inline void blub2() {}
};

class PropertyTestClass : public QObject
{
    Q_OBJECT
public:

    enum TestEnum { One, Two, Three };

    Q_ENUMS(TestEnum)
};

class PropertyUseClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PropertyTestClass::TestEnum foo READ foo)
public:

    inline PropertyTestClass::TestEnum foo() const { return PropertyTestClass::One; }
};

#if defined(Q_MOC_RUN)
// Task #119503
#define _TASK_119503
#if !_TASK_119503
#endif
#endif

static QString srcify(const char *path)
{
#ifndef Q_OS_IRIX
    return QString(SRCDIR) + QLatin1Char('/') + QLatin1String(path);
#else
    return QString(QLatin1String(path));
#endif
}

class tst_Moc : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool user1 READ user1 USER true )
    Q_PROPERTY(bool user2 READ user2 USER false)
    Q_PROPERTY(bool user3 READ user3 USER userFunction())

public:
    inline tst_Moc() {}

private slots:
    void slotWithException() throw(MyStruct);
    void dontStripNamespaces();
    void oldStyleCasts();
    void warnOnExtraSignalSlotQualifiaction();
    void uLongLong();
    void inputFileNameWithDotsButNoExtension();
    void userProperties();
    void supportConstSignals();
    void task87883();
    void multilineComments();
    void classinfoWithEscapes();
    void trNoopInClassInfo();
    void ppExpressionEvaluation();
    void arrayArguments();
    void preprocessorConditionals();
    void blackslashNewlines();
    void slotWithSillyConst();
    void testExtraData();
    void namespaceTypeProperty();
    void slotsWithVoidTemplate();
    void structQObject();
    void namespacedFlags();
    void warnOnMultipleInheritance();
    void forgottenQInterface();

signals:
    void sigWithUnsignedArg(unsigned foo);
    void sigWithSignedArg(signed foo);
    void sigWithConstSignedArg(const signed foo);
    void sigWithVolatileConstSignedArg(volatile const signed foo);
    void sigWithCustomType(const MyStruct);
    void constSignal1() const;
    void constSignal2(int arg) const;

private:
    bool user1() { return true; };
    bool user2() { return false; };
    bool user3() { return false; };
    bool userFunction(){ return false; };
};

void tst_Moc::slotWithException() throw(MyStruct)
{
    // be happy
    QVERIFY(true);
}

void tst_Moc::dontStripNamespaces()
{
    Sender sender;
    Receiver receiver;

    connect(&sender, SIGNAL(send(const String::Type &)),
            &receiver, SLOT(receive(const String::Type &)));
    connect(&sender, SIGNAL(send(const Int::Type &)),
            &receiver, SLOT(receive(const Int::Type &)));

    sender.sendValue(String::Type("Hello"));
    QCOMPARE(receiver.stringCallCount, 1);
    QCOMPARE(receiver.intCallCount, 0);
    sender.sendValue(Int::Type(42));
    QCOMPARE(receiver.stringCallCount, 1);
    QCOMPARE(receiver.intCallCount, 1);
}


void tst_Moc::oldStyleCasts()
{
#if defined(Q_OS_LINUX) && defined(Q_CC_GNU)
    QVERIFY(!qgetenv("QTDIR").isNull());

    QProcess proc;
    proc.start("moc", QStringList(srcify("/oldstyle-casts.h")));
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitCode(), 0);
    QByteArray mocOut = proc.readAllStandardOutput();
    QVERIFY(!mocOut.isEmpty());
    QCOMPARE(proc.readAllStandardError(), QByteArray());

    QStringList args;
    args << "-c" << "-x" << "c++" << "-Wold-style-cast" << "-I" << "."
         << "-I" << qgetenv("QTDIR") + "/include" << "-o" << "/dev/null" << "-";
    proc.start("gcc", args);
    QVERIFY(proc.waitForStarted());
    proc.write(mocOut);
    proc.closeWriteChannel();

    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitCode(), 0);
    QCOMPARE(QString::fromLocal8Bit(proc.readAllStandardError()), QString());
#else
    QSKIP("Only tested on linux/gcc", SkipAll);
#endif
}

void tst_Moc::warnOnExtraSignalSlotQualifiaction()
{
#if defined(Q_OS_LINUX) && defined(Q_CC_GNU)
    QVERIFY(!qgetenv("QTDIR").isNull());

    QProcess proc;
    proc.start("moc", QStringList(srcify("extraqualification.h")));
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitCode(), 0);
    QByteArray mocOut = proc.readAllStandardOutput();
    QVERIFY(!mocOut.isEmpty());
    QString mocWarning = QString::fromLocal8Bit(proc.readAllStandardError());
    QCOMPARE(mocWarning, QString(SRCDIR) +
                QString("/extraqualification.h:13: Warning: Function declaration Test::badFunctionDeclaration contains extra qualification. Ignoring as signal or slot.\n") +
                QString(SRCDIR) + QString("/extraqualification.h:16: Warning: parsemaybe: Function declaration Test::anotherOne contains extra qualification. Ignoring as signal or slot.\n"));
#else
    QSKIP("Only tested on linux/gcc", SkipAll);
#endif
}

void tst_Moc::uLongLong()
{
#ifndef NOLONGLONG
    TestClass tst;
    const QMetaObject *mobj = tst.metaObject();
    int idx = mobj->indexOfSlot("slotWithULong(ulong)");
    QVERIFY(idx != -1);
    idx = mobj->indexOfSlot("slotWithULongLong(unsigned long long)");
    QVERIFY(idx != -1);
#if QT_VERSION >= 0x040101
    idx = mobj->indexOfSlot("slotWithULongLongP(unsigned long long*)");
    QVERIFY(idx != -1);
#endif

    idx = mobj->indexOfSlot("slotWithLong(long)");
    QVERIFY(idx != -1);
    idx = mobj->indexOfSlot("slotWithLongLong(long long)");
    QVERIFY(idx != -1);
#else
    QSKIP("long long doesn't work on MSVC6 & .NET 2002, also skipped on 2003 due to compiler version issue with moc", SkipAll);
#endif
}

void tst_Moc::inputFileNameWithDotsButNoExtension()
{
#if defined(Q_OS_LINUX) && defined(Q_CC_GNU)
    QVERIFY(!qgetenv("QTDIR").isNull());

    QProcess proc;
    proc.setWorkingDirectory(QString(SRCDIR) + "/task71021");
    proc.start("moc", QStringList("../Header"));
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitCode(), 0);
    QByteArray mocOut = proc.readAllStandardOutput();
    QVERIFY(!mocOut.isEmpty());
    QCOMPARE(proc.readAllStandardError(), QByteArray());

    QStringList args;
    args << "-c" << "-x" << "c++" << "-I" << ".."
         << "-I" << qgetenv("QTDIR") + "/include" << "-o" << "/dev/null" << "-";
    proc.start("gcc", args);
    QVERIFY(proc.waitForStarted());
    proc.write(mocOut);
    proc.closeWriteChannel();

    QVERIFY(proc.waitForFinished());
    QCOMPARE(QString::fromLocal8Bit(proc.readAllStandardError()), QString());
    QCOMPARE(proc.exitCode(), 0);
#else
    QSKIP("Only tested on linux/gcc", SkipAll);
#endif
}

void tst_Moc::userProperties()
{
    const QMetaObject *mobj = metaObject();
    QMetaProperty property = mobj->property(mobj->indexOfProperty("user1"));
    QVERIFY(property.isValid());
    QVERIFY(property.isUser());

    property = mobj->property(mobj->indexOfProperty("user2"));
    QVERIFY(property.isValid());
    QVERIFY(!property.isUser());

    property = mobj->property(mobj->indexOfProperty("user3"));
    QVERIFY(property.isValid());
    QVERIFY(!property.isUser(this));
}

void tst_Moc::supportConstSignals()
{
    QSignalSpy spy1(this, SIGNAL(constSignal1()));
    QVERIFY(spy1.isEmpty());
    emit constSignal1();
    QCOMPARE(spy1.count(), 1);

    QSignalSpy spy2(this, SIGNAL(constSignal2(int)));
    QVERIFY(spy2.isEmpty());
    emit constSignal2(42);
    QCOMPARE(spy2.count(), 1);
    QCOMPARE(spy2.at(0).at(0).toInt(), 42);
}

#include "task87883.h"

void tst_Moc::task87883()
{
#if QT_VERSION >= 0x040101
    QVERIFY(Task87883::staticMetaObject.className());
#else
    QSKIP("Fixed in >= 4.1.1", SkipAll);
#endif
}

#include "c-comments.h"

void tst_Moc::multilineComments()
{
#if QT_VERSION >= 0x040101
    QVERIFY(IfdefedClass::staticMetaObject.className());
#else
    QSKIP("Fixed in >= 4.1.1", SkipAll);
#endif
}

void tst_Moc::classinfoWithEscapes()
{
    const QMetaObject *mobj = &TestClassinfoWithEscapes::staticMetaObject;
    QCOMPARE(mobj->methodCount() - mobj->methodOffset(), 1);

    QMetaMethod mm = mobj->method(mobj->methodOffset());
    QCOMPARE(mm.signature(), "slotWithAReallyLongName(int)");
}

void tst_Moc::trNoopInClassInfo()
{
    TestClass t;
    const QMetaObject *mobj = t.metaObject();
    QVERIFY(mobj);
    QCOMPARE(mobj->classInfoCount(), 3);
    QCOMPARE(mobj->indexOfClassInfo("help"), 0);
    QCOMPARE(QString(mobj->classInfo(0).value()), QString("Opening this will let you configure something"));
}

void tst_Moc::ppExpressionEvaluation()
{
    TestClass tst;
    const QMetaObject *mobj = tst.metaObject();
    int idx = mobj->indexOfSlot("expressionEvaluationShortcut1()");
    QVERIFY(idx != -1);

    idx = mobj->indexOfSlot("expressionEvaluationShortcut2()");
    QVERIFY(idx != -1);
}

void tst_Moc::arrayArguments()
{
    TestClass tst;
    const QMetaObject *mobj = tst.metaObject();
    QVERIFY(mobj->indexOfSlot("slotWithArray(const double[3])") != -1);
    QVERIFY(mobj->indexOfSlot("slotWithNamedArray(const double[3])") != -1);
    QVERIFY(mobj->indexOfSlot("slotWithMultiArray(const double[3][4])") != -1);
    QVERIFY(mobj->indexOfSignal("signalWithArray(const double[3])") != -1);
    QVERIFY(mobj->indexOfSignal("signalWithNamedArray(const double[3])") != -1);
}

void tst_Moc::preprocessorConditionals()
{
    TestClass tst;
    const QMetaObject *mobj = tst.metaObject();
    QVERIFY(mobj->indexOfSlot("slotInElse()") != -1);
    QVERIFY(mobj->indexOfSlot("slotInIf()") != -1);
    QVERIFY(mobj->indexOfSlot("slotInLastElse()") != -1);
    QVERIFY(mobj->indexOfSlot("slotInElif()") != -1);
}

void tst_Moc::blackslashNewlines()
{
    BackslashNewlines tst;
    const QMetaObject *mobj = tst.metaObject();
    QVERIFY(mobj->indexOfSlot("works()") != -1);
    QVERIFY(mobj->indexOfSlot("buggy()") == -1);
}

void tst_Moc::slotWithSillyConst()
{
    TestClass tst;
    const QMetaObject *mobj = tst.metaObject();
    QVERIFY(mobj->indexOfSlot("slotWithSillyConst()") != -1);
    QVERIFY(mobj->indexOfMethod("slotWithSillyConst2()") != -1);
    QVERIFY(mobj->indexOfSlot("slotWithVoidStar(void*)") != -1);
}

void tst_Moc::testExtraData()
{
    const QMetaObject *mobj = &PropertyTestClass::staticMetaObject;
    QCOMPARE(mobj->enumeratorCount(), 1);
    QCOMPARE(QByteArray(mobj->enumerator(0).name()), QByteArray("TestEnum"));

    mobj = &PropertyUseClass::staticMetaObject;
    const int idx = mobj->indexOfProperty("foo");
    QVERIFY(idx != -1);
    const QMetaProperty prop = mobj->property(idx);
    QVERIFY(prop.isValid());
    QVERIFY(prop.isEnumType());
    const QMetaEnum en = prop.enumerator();
    QCOMPARE(QByteArray(en.name()), QByteArray("TestEnum"));
}

void tst_Moc::namespaceTypeProperty()
{
    qRegisterMetaType<myNS::Points>("myNS::Points");
    TestClass tst;
    QByteArray ba = QByteArray("points");
    QVariant v = tst.property(ba);
    QVERIFY(v.isValid());
    myNS::Points p = qVariantValue<myNS::Points>(v);
    QCOMPARE(p.p1, 0xBEEF);
    QCOMPARE(p.p2, 0xBABE);
    p.p1 = 0xCAFE;
    p.p2 = 0x1EE7;
    QVERIFY(tst.setProperty(ba, qVariantFromValue(p)));
    myNS::Points pp = qVariantValue<myNS::Points>(tst.property(ba));
    QCOMPARE(p.p1, pp.p1);
    QCOMPARE(p.p2, pp.p2);
}

void tst_Moc::slotsWithVoidTemplate()
{
    SlotsWithVoidTemplateTest test;
    QVERIFY(QObject::connect(&test, SIGNAL(myVoidSignal(void)),
                             &test, SLOT(dummySlot(void))));
    QVERIFY(QObject::connect(&test, SIGNAL(mySignal(const TestTemplate<void> &)),
                             &test, SLOT(anotherSlot(const TestTemplate<void> &))));
}

void tst_Moc::structQObject()
{
    StructQObject o;
    QCOMPARE(QByteArray(o.metaObject()->className()), QByteArray("StructQObject"));
}

#include "namespaced-flags.h"

void tst_Moc::namespacedFlags()
{
    Foo::Baz baz;
    Foo::Bar bar;

    bar.setFlags(Foo::Bar::Read | Foo::Bar::Write);
    QVERIFY(baz.flags() != bar.flags());

    const QVariant v = bar.property("flags");
    QVERIFY(v.isValid());
    QVERIFY(baz.setProperty("flags", v));
    QVERIFY(baz.flags() == bar.flags());
}

void tst_Moc::warnOnMultipleInheritance()
{
#if defined(Q_OS_LINUX) && defined(Q_CC_GNU)
    QVERIFY(!qgetenv("QTDIR").isNull());

    QProcess proc;
    proc.start("moc", QStringList(srcify("warn-on-multiple-qobject-subclasses.h")));
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitCode(), 0);
    QByteArray mocOut = proc.readAllStandardOutput();
    QVERIFY(!mocOut.isEmpty());
    QString mocWarning = QString::fromLocal8Bit(proc.readAllStandardError());
    QCOMPARE(mocWarning, QString(SRCDIR) +
                QString("/warn-on-multiple-qobject-subclasses.h:13: Warning: Class Bar inherits from two QObject subclasses QWidget and Foo. This is not supported!\n"));
#else
    QSKIP("Only tested on linux/gcc", SkipAll);
#endif
}

void tst_Moc::forgottenQInterface()
{
#if defined(Q_OS_LINUX) && defined(Q_CC_GNU)
    QVERIFY(!qgetenv("QTDIR").isNull());

    QProcess proc;
    proc.start("moc", QStringList(srcify("forgotten-qinterface.h")));
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitCode(), 0);
    QByteArray mocOut = proc.readAllStandardOutput();
    QVERIFY(!mocOut.isEmpty());
    QString mocWarning = QString::fromLocal8Bit(proc.readAllStandardError());
    QCOMPARE(mocWarning, QString(SRCDIR) +
                QString("/forgotten-qinterface.h:13: Warning: Class Test implements the interface MyInterface but does not list it in Q_INTERFACES. qobject_cast to MyInterface will not work!\n"));
#else
    QSKIP("Only tested on linux/gcc", SkipAll);
#endif
}

QTEST_MAIN(tst_Moc)
#include "tst_moc.moc"




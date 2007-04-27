/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qscriptengine.h>
#include <qgraphicsitem.h>
#include <qstandarditemmodel.h>

//TESTED_CLASS=
//TESTED_FILES=qscriptengine.h qscriptengine.cpp

class tst_QScriptEngine : public QObject
{
    Q_OBJECT

public:
    tst_QScriptEngine();
    virtual ~tst_QScriptEngine();

private slots:
    void currentContext();
    void newFunction();
    void newObject();
    void newArray();
    void newVariant();
    void newRegExp();
    void newDate();
    void newQObject();
    void newQMetaObject();
    void newActivationObject();
    void globalObject();
    void canEvaluate_data();
    void canEvaluate();
    void evaluate_data();
    void evaluate();
    void nestedEvaluate();
    void uncaughtException();
    void getSetDefaultPrototype();
    void valueConversion();
    void importExtension();
    void infiniteRecursion();
    void castWithPrototypeChain();
    void castWithMultipleInheritance();
    void gc();
    void gcWithNestedDataStructure();
    void processEventsWhileRunning();
};

tst_QScriptEngine::tst_QScriptEngine()
{
}

tst_QScriptEngine::~tst_QScriptEngine()
{
}

void tst_QScriptEngine::currentContext()
{
    QScriptEngine eng;
    QVERIFY(eng.currentContext() != 0);
    QVERIFY(eng.currentContext()->parentContext() == 0);
}

static QScriptValue myFunction(QScriptContext *, QScriptEngine *eng)
{
    return eng->nullValue();
}

void tst_QScriptEngine::newFunction()
{
    QScriptEngine eng;
    {
        QScriptValue fun = eng.newFunction(myFunction);
        QCOMPARE(fun.isValid(), true);
        QCOMPARE(fun.isFunction(), true);
        QCOMPARE(fun.isObject(), true);
        // prototype should be Function.prototype
        QCOMPARE(fun.prototype().isValid(), true);
        QCOMPARE(fun.prototype().isFunction(), true);
        QCOMPARE(fun.prototype().strictEqualTo(eng.evaluate("Function.prototype")), true);
        
        QCOMPARE(fun.call().isNull(), true);
        QCOMPARE(fun.construct().isObject(), true);
    }

    // the overload that takes a prototype
    {
        QScriptValue proto = eng.newObject();
        QScriptValue fun = eng.newFunction(myFunction, proto);
        QCOMPARE(fun.isValid(), true);
        QCOMPARE(fun.isFunction(), true);
        QCOMPARE(fun.isObject(), true);
        // internal prototype should be Function.prototype
        QCOMPARE(fun.prototype().isValid(), true);
        QCOMPARE(fun.prototype().isFunction(), true);
        QCOMPARE(fun.prototype().strictEqualTo(eng.evaluate("Function.prototype")), true);
        // public prototype should be the one we passed
        QCOMPARE(fun.property("prototype").strictEqualTo(proto), true);
        QCOMPARE(fun.propertyFlags("prototype"), QScriptValue::Undeletable);
        QCOMPARE(proto.property("constructor").strictEqualTo(fun), true);
        QCOMPARE(proto.propertyFlags("constructor"),
                 QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);

        QCOMPARE(fun.call().isNull(), true);
        QCOMPARE(fun.construct().isObject(), true);
    }
}

void tst_QScriptEngine::newObject()
{
    QScriptEngine eng;
    QScriptValue object = eng.newObject();
    QCOMPARE(object.isValid(), true);
    QCOMPARE(object.isObject(), true);
    QCOMPARE(object.isFunction(), false);
    // prototype should be Object.prototype
    QCOMPARE(object.prototype().isValid(), true);
    QCOMPARE(object.prototype().isObject(), true);
    QCOMPARE(object.prototype().strictEqualTo(eng.evaluate("Object.prototype")), true);
}

void tst_QScriptEngine::newArray()
{
    QScriptEngine eng;
    QScriptValue array = eng.newArray();
    QCOMPARE(array.isValid(), true);
    QCOMPARE(array.isArray(), true);
    QCOMPARE(array.isObject(), true);
    // prototype should be Array.prototype
    QCOMPARE(array.prototype().isValid(), true);
    QCOMPARE(array.prototype().isArray(), true);
    QCOMPARE(array.prototype().strictEqualTo(eng.evaluate("Array.prototype")), true);
}

void tst_QScriptEngine::newVariant()
{
    QScriptEngine eng;
    QScriptValue opaque = eng.newVariant(QVariant());
    QCOMPARE(opaque.isValid(), true);
    QCOMPARE(opaque.isVariant(), true);
    QCOMPARE(opaque.isObject(), true);
    QCOMPARE(opaque.prototype().isValid(), true);
    QCOMPARE(opaque.prototype().isVariant(), true);
}

void tst_QScriptEngine::newRegExp()
{
    QScriptEngine eng;
    for (int x = 0; x < 2; ++x) {
        QScriptValue rexp;
        if (x == 0)
            rexp = eng.newRegExp("foo", "bar");
        else
            rexp = eng.newRegExp(QRegExp("foo"));
        QCOMPARE(rexp.isValid(), true);
        QCOMPARE(rexp.isRegExp(), true);
        QCOMPARE(rexp.isObject(), true);
        // prototype should be RegExp.prototype
        QCOMPARE(rexp.prototype().isValid(), true);
        QCOMPARE(rexp.prototype().isRegExp(), true);
        QCOMPARE(rexp.prototype().strictEqualTo(eng.evaluate("RegExp.prototype")), true);

        QCOMPARE(rexp.toRegExp().pattern(), QRegExp("foo").pattern());
    }
}

void tst_QScriptEngine::newDate()
{
    QScriptEngine eng;

    {
        QScriptValue date = eng.newDate(0);
        QCOMPARE(date.isValid(), true);
        QCOMPARE(date.isDate(), true);
        QCOMPARE(date.isObject(), true);
        // prototype should be Date.prototype
        QCOMPARE(date.prototype().isValid(), true);
        QCOMPARE(date.prototype().isDate(), true);
        QCOMPARE(date.prototype().strictEqualTo(eng.evaluate("Date.prototype")), true);
    }

    {
        QDateTime dt = QDateTime(QDate(1, 2, 3), QTime(4, 5, 6, 7));
        QScriptValue date = eng.newDate(dt);
        QCOMPARE(date.isValid(), true);
        QCOMPARE(date.isDate(), true);
        QCOMPARE(date.isObject(), true);
        // prototype should be Date.prototype
        QCOMPARE(date.prototype().isValid(), true);
        QCOMPARE(date.prototype().isDate(), true);
        QCOMPARE(date.prototype().strictEqualTo(eng.evaluate("Date.prototype")), true);

        QCOMPARE(date.toDateTime(), dt);
    }
}

void tst_QScriptEngine::newQObject()
{
    QScriptEngine eng;

    {
        QScriptValue qobject = eng.newQObject(0);
        QCOMPARE(qobject.isValid(), true);
        QCOMPARE(qobject.isNull(), true);
        QCOMPARE(qobject.isObject(), false);
        QCOMPARE(qobject.toQObject(), (QObject *)0);
    }
    {
        QScriptValue qobject = eng.newQObject(this);
        QCOMPARE(qobject.isValid(), true);
        QCOMPARE(qobject.isQObject(), true);
        QCOMPARE(qobject.isObject(), true);
        QCOMPARE(qobject.toQObject(), (QObject *)this);
        // prototype should be QObject.prototype
        QCOMPARE(qobject.prototype().isValid(), true);
        QCOMPARE(qobject.prototype().isQObject(), true);
    }

    // test ownership
    {
        QPointer<QObject> ptr = new QObject();
        QVERIFY(ptr != 0);
        {
            QScriptValue v = eng.newQObject(ptr, QScriptEngine::ScriptOwnership);
        }
        eng.evaluate("gc()");
        QVERIFY(ptr == 0);
    }
    {
        QPointer<QObject> ptr = new QObject();
        QVERIFY(ptr != 0);
        {
            QScriptValue v = eng.newQObject(ptr, QScriptEngine::QtOwnership);
        }
        QObject *before = ptr;
        eng.evaluate("gc()");
        QVERIFY(ptr == before);
        delete ptr;
    }
    {
        QObject *parent = new QObject();
        QObject *child = new QObject(parent);
        QScriptValue v = eng.newQObject(child, QScriptEngine::QtOwnership);
        QCOMPARE(v.toQObject(), child);
        delete parent;
        QCOMPARE(v.toQObject(), (QObject *)0);
    }
    {
        QPointer<QObject> ptr = new QObject();
        QVERIFY(ptr != 0);
        {
            QScriptValue v = eng.newQObject(ptr, QScriptEngine::AutoOwnership);
        }
        eng.evaluate("gc()");
        // no parent, so it should be like ScriptOwnership
        QVERIFY(ptr == 0);
    }
    {
        QObject *parent = new QObject();
        QPointer<QObject> child = new QObject(parent);
        QVERIFY(child != 0);
        {
            QScriptValue v = eng.newQObject(child, QScriptEngine::AutoOwnership);
        }
        eng.evaluate("gc()");
        // has parent, so it should be like QtOwnership
        QVERIFY(child != 0);
        delete parent;
    }
}

Q_SCRIPT_DECLARE_QMETAOBJECT(QObject, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QWidget, QWidget*)

void tst_QScriptEngine::newQMetaObject()
{
    QScriptEngine eng;
#if 0
    QScriptValue qclass = eng.newQMetaObject<QObject>();
    QScriptValue qclass2 = eng.newQMetaObject<QWidget>();
#else
    QScriptValue qclass = qScriptValueFromQMetaObject<QObject>(&eng);
    QScriptValue qclass2 = qScriptValueFromQMetaObject<QWidget>(&eng);
#endif
    QCOMPARE(qclass.isValid(), true);
    QCOMPARE(qclass.isQMetaObject(), true);
    QCOMPARE(qclass.toQMetaObject(), &QObject::staticMetaObject);
    QCOMPARE(qclass.isFunction(), true);

    QCOMPARE(qclass2.isValid(), true);
    QCOMPARE(qclass2.isQMetaObject(), true);
    QCOMPARE(qclass2.toQMetaObject(), &QWidget::staticMetaObject);
    QCOMPARE(qclass2.isFunction(), true);

    // prototype should be QMetaObject.prototype
    QCOMPARE(qclass.prototype().isValid(), true);
    QCOMPARE(qclass2.prototype().isValid(), true);

    QScriptValue instance = qclass.construct();
    QCOMPARE(instance.isQObject(), true);
    QCOMPARE(instance.toQObject()->metaObject(), qclass.toQMetaObject());

    QScriptValue instance2 = qclass2.construct();
    QCOMPARE(instance2.isQObject(), true);
    QCOMPARE(instance2.toQObject()->metaObject(), qclass2.toQMetaObject());

    QScriptValueList args;
    args << instance;
    QScriptValue instance3 = qclass.construct(args);
    QCOMPARE(instance3.isQObject(), true);
    QCOMPARE(instance3.toQObject()->parent(), instance.toQObject());

    // ### must explicitly delete these, since the engine will not
    delete instance.toQObject();
    delete instance2.toQObject();
}

void tst_QScriptEngine::newActivationObject()
{
    QScriptEngine eng;
    QScriptValue act = eng.newActivationObject();
    QCOMPARE(act.isValid(), true);
    QCOMPARE(act.isObject(), true);
    QScriptValue v(&eng, 123);
    act.setProperty("prop", v);
    QCOMPARE(act.property("prop").strictEqualTo(v), true);
    QCOMPARE(act.scope().isValid(), false);
}

void tst_QScriptEngine::globalObject()
{
    QScriptEngine eng;
    QScriptValue glob = eng.globalObject();
    QCOMPARE(glob.isValid(), true);
    QCOMPARE(glob.isObject(), true);
    // prototype should be Object.prototype
    QCOMPARE(glob.prototype().isValid(), true);
    QCOMPARE(glob.prototype().isObject(), true);
    QCOMPARE(glob.prototype().strictEqualTo(eng.evaluate("Object.prototype")), true);
}

void tst_QScriptEngine::canEvaluate_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<bool>("expectSuccess");

    QTest::newRow("") << QString("") << true;
    QTest::newRow("0") << QString("0") << true;
    QTest::newRow("!") << QString("!\n") << false;
    QTest::newRow("if (") << QString("if (\n") << false;
    QTest::newRow("a = 1; if (") << QString("a = 1;\nif (\n") << false;
    QTest::newRow("./test.js") << QString("./test.js\n") << true;
    QTest::newRow("if (0) print(1)") << QString("if (0)\nprint(1)\n") << true;
    QTest::newRow("0 = ") << QString("0 = \n") << false;
    QTest::newRow("0 = 0") << QString("0 = 0\n") << true;
    QTest::newRow("foo[") << QString("foo[") << true; // automatic semicolon will be inserted
    QTest::newRow("foo[") << QString("foo[\n") << false;
    QTest::newRow("foo['bar']") << QString("foo['bar']") << true;
}

void tst_QScriptEngine::canEvaluate()
{
    QFETCH(QString, code);
    QFETCH(bool, expectSuccess);

    QScriptEngine eng;
    QCOMPARE(eng.canEvaluate(code), expectSuccess);
}

void tst_QScriptEngine::evaluate_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<int>("lineNumber");
    QTest::addColumn<bool>("expectHadError");
    QTest::addColumn<int>("expectErrorLineNumber");

    QTest::newRow("0")     << QString("0")       << -1 << false << 0;
    QTest::newRow("0=1")   << QString("\n0=1\n") << -1 << true  << 1;
    QTest::newRow("a=1")   << QString("a=1\n")   << -1 << false << 0;
    QTest::newRow("a=1;K") << QString("a=1;\nK") << -1 << true  << 1;

    QTest::newRow("f()") << QString("function f()\n"
                                    "{\n"
                                    "  var a;\n"
                                    "  var b=\";\n" // here's the error
                                    "}\n"
                                    "f();\n")
                         << -1 << true << 3;

    QTest::newRow("0")     << QString("0")       << 10 << false << 0;
    QTest::newRow("0=1")   << QString("\n\n0=1\n") << 10 << true  << 12;
    QTest::newRow("a=1")   << QString("a=1\n")   << 10 << false << 0;
    QTest::newRow("a=1;K") << QString("a=1;\n\nK") << 10 << true  << 12;

    QTest::newRow("f()") << QString("function f()\n"
                                    "{\n"
                                    "  var a;\n"
                                    "\n\n"
                                    "  var b=\";\n" // here's the error
                                    "}\n"
                                    "f();\n")
                         << 10 << true << 15;
}

void tst_QScriptEngine::evaluate()
{
    QFETCH(QString, code);
    QFETCH(int, lineNumber);
    QFETCH(bool, expectHadError);
    QFETCH(int, expectErrorLineNumber);

    QScriptEngine eng;
    if (lineNumber != -1)
        (void)eng.evaluate(code, lineNumber);
    else
        (void)eng.evaluate(code);
    QCOMPARE(eng.hasUncaughtException(), expectHadError);
    QCOMPARE(eng.uncaughtExceptionLineNumber(), expectErrorLineNumber);
}

static QScriptValue eval_nested(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue result = eng->newObject();
    result.setProperty("thisObjectIdBefore", ctx->thisObject().property("id"));
    QScriptValue evaluatedThisObject = eng->evaluate("this");
    result.setProperty("thisObjectIdAfter", ctx->thisObject().property("id"));
    result.setProperty("evaluatedThisObjectId", evaluatedThisObject.property("id"));
    return result;
}

void tst_QScriptEngine::nestedEvaluate()
{
    QScriptEngine eng;
    eng.globalObject().setProperty("fun", eng.newFunction(eval_nested));
    QScriptValue result = eng.evaluate("o = { id:'foo'}; o.fun = fun; o.fun()");
    QCOMPARE(result.property("thisObjectIdBefore").toString(), QString("foo"));
    QCOMPARE(result.property("thisObjectIdAfter").toString(), QString("foo"));
    QCOMPARE(result.property("evaluatedThisObjectId").toString(), QString("foo"));
}

void tst_QScriptEngine::uncaughtException()
{
    QScriptEngine eng;
    for (int x = 0; x < 2; ++x) {
        {
            QScriptValue ret = eng.evaluate("a = 10;\nb = 20;\n0 = 0;\n", /*lineNumber=*/x);
            QVERIFY(eng.hasUncaughtException());
            QCOMPARE(eng.uncaughtExceptionLineNumber(), x+2);
            QVERIFY(eng.uncaughtException().strictEqualTo(ret));
            (void)ret.toString();
            QVERIFY(!eng.hasUncaughtException());
            QCOMPARE(eng.uncaughtExceptionLineNumber(), x+2);
            QVERIFY(!eng.uncaughtException().isValid());
        }
        {
            QScriptValue ret = eng.evaluate("a = 10");
            QVERIFY(!eng.hasUncaughtException());
            QVERIFY(!eng.uncaughtException().isValid());
        }
    }
}

struct Foo {
public:
    int x, y;
    Foo() : x(-1), y(-1) { }
};

Q_DECLARE_METATYPE(Foo)
Q_DECLARE_METATYPE(Foo*)

void tst_QScriptEngine::getSetDefaultPrototype()
{
    QScriptEngine eng;
    {
        QScriptValue object = eng.newObject();
        QCOMPARE(eng.defaultPrototype(qMetaTypeId<int>()).isValid(), false);
        eng.setDefaultPrototype(qMetaTypeId<int>(), object);
        QCOMPARE(eng.defaultPrototype(qMetaTypeId<int>()).strictEqualTo(object), true);
        QScriptValue value = eng.newVariant(int(123));
        QCOMPARE(value.prototype().isObject(), true);
        QCOMPARE(value.prototype().strictEqualTo(object), true);

        eng.setDefaultPrototype(qMetaTypeId<int>(), QScriptValue());
        QCOMPARE(eng.defaultPrototype(qMetaTypeId<int>()).isValid(), false);
        QScriptValue value2 = eng.newVariant(int(123));
        QCOMPARE(value2.prototype().strictEqualTo(object), false);
    }
    {
        QScriptValue object = eng.newObject();
        QCOMPARE(eng.defaultPrototype(qMetaTypeId<Foo>()).isValid(), false);
        eng.setDefaultPrototype(qMetaTypeId<Foo>(), object);
        QCOMPARE(eng.defaultPrototype(qMetaTypeId<Foo>()).strictEqualTo(object), true);
        QScriptValue value = eng.newVariant(qVariantFromValue(Foo()));
        QCOMPARE(value.prototype().isObject(), true);
        QCOMPARE(value.prototype().strictEqualTo(object), true);

        eng.setDefaultPrototype(qMetaTypeId<Foo>(), QScriptValue());
        QCOMPARE(eng.defaultPrototype(qMetaTypeId<Foo>()).isValid(), false);
        QScriptValue value2 = eng.newVariant(qVariantFromValue(Foo()));
        QCOMPARE(value2.prototype().strictEqualTo(object), false);
    }
}

static QScriptValue fooToScriptValue(QScriptEngine *eng, const Foo &foo)
{
    QScriptValue obj = eng->newObject();
    obj.setProperty("x", QScriptValue(eng, foo.x));
    obj.setProperty("y", QScriptValue(eng, foo.y));
    return obj;
}

static void fooFromScriptValue(const QScriptValue &value, Foo &foo)
{
    foo.x = value.property("x").toInt32();
    foo.y = value.property("y").toInt32();
}

Q_DECLARE_METATYPE(QLinkedList<QString>)
Q_DECLARE_METATYPE(QList<Foo>)
Q_DECLARE_METATYPE(QVector<QChar>)
Q_DECLARE_METATYPE(QStack<int>)
Q_DECLARE_METATYPE(QQueue<char>)
Q_DECLARE_METATYPE(QLinkedList<QStack<int> >)

void tst_QScriptEngine::valueConversion()
{
    QScriptEngine eng;
    {
        QScriptValue num = qScriptValueFromValue(&eng, 123);
        QCOMPARE(num.isNumber(), true);
        QCOMPARE(num.strictEqualTo(QScriptValue(&eng, 123)), true);

        int inum = qScriptValueToValue<int>(num);
        QCOMPARE(inum, 123);

        QString snum = qScriptValueToValue<QString>(num);
        QCOMPARE(snum, QLatin1String("123"));
    }
#ifndef QT_NO_MEMBER_TEMPLATES
    {
        QScriptValue num = eng.toScriptValue(123);
        QCOMPARE(num.isNumber(), true);
        QCOMPARE(num.strictEqualTo(QScriptValue(&eng, 123)), true);

        int inum = eng.fromScriptValue<int>(num);
        QCOMPARE(inum, 123);

        QString snum = eng.fromScriptValue<QString>(num);
        QCOMPARE(snum, QLatin1String("123"));
    }
#endif
    {
        QScriptValue num(&eng, 123);
        QCOMPARE(qScriptValueToValue<char>(num), char(123));
        QCOMPARE(qScriptValueToValue<unsigned char>(num), (unsigned char)(123));
        QCOMPARE(qScriptValueToValue<short>(num), short(123));
        QCOMPARE(qScriptValueToValue<unsigned short>(num), (unsigned short)(123));
        QCOMPARE(qScriptValueToValue<float>(num), float(123));
        QCOMPARE(qScriptValueToValue<double>(num), double(123));
    }

    {
        QChar c = QLatin1Char('c');
        QScriptValue str = QScriptValue(&eng, "ciao");
        QCOMPARE(qScriptValueToValue<QChar>(str), c);
        QScriptValue code = QScriptValue(&eng, c.unicode());
        QCOMPARE(qScriptValueToValue<QChar>(code), c);
        QCOMPARE(qScriptValueToValue<QChar>(qScriptValueFromValue(&eng, c)), c);
    }

    {
        // a type that we don't have built-in conversion of
        // (it's stored as a variant)
        QTime tm(1, 2, 3, 4);
        QScriptValue val = qScriptValueFromValue(&eng, tm);
        QCOMPARE(qScriptValueToValue<QTime>(val), tm);
    }

    {
        Foo foo;
        foo.x = 12;
        foo.y = 34;
        QScriptValue fooVal = qScriptValueFromValue(&eng, foo);
        QCOMPARE(fooVal.isVariant(), true);

        Foo foo2 = qScriptValueToValue<Foo>(fooVal);
        QCOMPARE(foo2.x, foo.x);
        QCOMPARE(foo2.y, foo.y);
    }

    qScriptRegisterMetaType<Foo>(&eng, fooToScriptValue, fooFromScriptValue);

    {
        Foo foo;
        foo.x = 12;
        foo.y = 34;
        QScriptValue fooVal = qScriptValueFromValue(&eng, foo);
        QCOMPARE(fooVal.isObject(), true);
        QCOMPARE(fooVal.property("x").strictEqualTo(QScriptValue(&eng, 12)), true);
        QCOMPARE(fooVal.property("y").strictEqualTo(QScriptValue(&eng, 34)), true);
        fooVal.setProperty("x", QScriptValue(&eng, 56));
        fooVal.setProperty("y", QScriptValue(&eng, 78));

        Foo foo2 = qScriptValueToValue<Foo>(fooVal);
        QCOMPARE(foo2.x, 56);
        QCOMPARE(foo2.y, 78);
    }

    qScriptRegisterSequenceMetaType<QLinkedList<QString> >(&eng);

    {
        QLinkedList<QString> lst;
        lst << QLatin1String("foo") << QLatin1String("bar");
        QScriptValue lstVal = qScriptValueFromValue(&eng, lst);
        QCOMPARE(lstVal.isArray(), true);
        QCOMPARE(lstVal.property("length").toInt32(), 2);
        QCOMPARE(lstVal.property("0").isString(), true);
        QCOMPARE(lstVal.property("0").toString(), QLatin1String("foo"));
        QCOMPARE(lstVal.property("1").isString(), true);
        QCOMPARE(lstVal.property("1").toString(), QLatin1String("bar"));
    }

    qScriptRegisterSequenceMetaType<QList<Foo> >(&eng);
    qScriptRegisterSequenceMetaType<QStack<int> >(&eng);
    qScriptRegisterSequenceMetaType<QVector<QChar> >(&eng);
    qScriptRegisterSequenceMetaType<QQueue<char> >(&eng);
    qScriptRegisterSequenceMetaType<QLinkedList<QStack<int> > >(&eng);

    {
        QLinkedList<QStack<int> > lst;
        QStack<int> first; first << 13 << 49; lst << first;
        QStack<int> second; second << 99999;lst << second;
        QScriptValue lstVal = qScriptValueFromValue(&eng, lst);
        QCOMPARE(lstVal.isArray(), true);
        QCOMPARE(lstVal.property("length").toInt32(), 2);
        QCOMPARE(lstVal.property("0").isArray(), true);
        QCOMPARE(lstVal.property("0").property("length").toInt32(), 2);
        QCOMPARE(lstVal.property("0").property("0").toInt32(), first.at(0));
        QCOMPARE(lstVal.property("0").property("1").toInt32(), first.at(1));
        QCOMPARE(lstVal.property("1").isArray(), true);
        QCOMPARE(lstVal.property("1").property("length").toInt32(), 1);
        QCOMPARE(lstVal.property("1").property("0").toInt32(), second.at(0));
        QCOMPARE(qscriptvalue_cast<QStack<int> >(lstVal.property("0")), first);
        QCOMPARE(qscriptvalue_cast<QStack<int> >(lstVal.property("1")), second);
        QCOMPARE(qscriptvalue_cast<QLinkedList<QStack<int> > >(lstVal), lst);
    }

    // pointers
    {
        Foo foo;
        {
            QScriptValue v = qScriptValueFromValue(&eng, &foo);
            Foo *pfoo = qscriptvalue_cast<Foo*>(v);
            QCOMPARE(pfoo, &foo);
        }
        {
            Foo *pfoo = 0;
            QScriptValue v = qScriptValueFromValue(&eng, pfoo);
            QCOMPARE(v.isNull(), true);
            QVERIFY(qscriptvalue_cast<Foo*>(v) == 0);
        }
    }
}

static QScriptValue __import__(QScriptContext *ctx, QScriptEngine *eng)
{
    return eng->importExtension(ctx->argument(0).toString());
}

void tst_QScriptEngine::importExtension()
{
    QStringList libPaths = QCoreApplication::instance()->libraryPaths();
    QCoreApplication::instance()->setLibraryPaths(QStringList() << ".");

    // try to import something that doesn't exist
    {
        QScriptEngine eng;
        QScriptValue ret = eng.importExtension("this.extension.does.not.exist");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(ret.isError(), true);
    }

    {
        QScriptEngine eng;
        for (int x = 0; x < 2; ++x) {
            QCOMPARE(eng.globalObject().property("com").isValid(), x == 1);
            QScriptValue ret = eng.importExtension("com.trolltech");
            QCOMPARE(eng.hasUncaughtException(), false);
            QCOMPARE(ret.isUndefined(), true);

            QScriptValue com = eng.globalObject().property("com");
            QCOMPARE(com.isObject(), true);
            QCOMPARE(com.property("wasDefinedAlready")
                     .strictEqualTo(QScriptValue(&eng, false)), true);
            QCOMPARE(com.property("name")
                     .strictEqualTo(QScriptValue(&eng, "com")), true);
            QCOMPARE(com.property("level")
                     .strictEqualTo(QScriptValue(&eng, 1)), true);

            QScriptValue trolltech = com.property("trolltech");
            QCOMPARE(trolltech.isObject(), true);
            QCOMPARE(trolltech.property("wasDefinedAlready")
                     .strictEqualTo(QScriptValue(&eng, false)), true);
            QCOMPARE(trolltech.property("name")
                     .strictEqualTo(QScriptValue(&eng, "com.trolltech")), true);
            QCOMPARE(trolltech.property("level")
                     .strictEqualTo(QScriptValue(&eng, 2)), true);
        }
    }

    // recursive import should throw an error
    {
        QScriptEngine eng;
        eng.globalObject().setProperty("__import__", eng.newFunction(__import__));
        QScriptValue ret = eng.importExtension("com.trolltech.recursive");
        QCOMPARE(eng.hasUncaughtException(), true);
    }

    QCoreApplication::instance()->setLibraryPaths(libPaths);
}

static QScriptValue recurse(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng);
    return ctx->callee().call();
}

static QScriptValue recurse2(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng);
    return ctx->callee().construct();
}

void tst_QScriptEngine::infiniteRecursion()
{
    QScriptEngine eng;
    {
        QScriptValue ret = eng.evaluate("function foo() { foo(); }; foo();");
        QCOMPARE(ret.isError(), true);
        QCOMPARE(ret.toString(), QLatin1String("Error: call stack overflow"));
    }
    {
        QScriptValue fun = eng.newFunction(recurse);
        QScriptValue ret = fun.call();
        QCOMPARE(ret.isError(), true);
        QCOMPARE(ret.toString(), QLatin1String("Error: call stack overflow"));
    }
    {
        QScriptValue fun = eng.newFunction(recurse2);
        QScriptValue ret = fun.construct();
        QCOMPARE(ret.isError(), true);
        QCOMPARE(ret.toString(), QLatin1String("Error: call stack overflow"));
    }
}

struct Bar {
    int a;
};

struct Baz : public Bar {
    int b;
};

Q_DECLARE_METATYPE(Bar*)
Q_DECLARE_METATYPE(Baz*)

Q_DECLARE_METATYPE(QGradient)
Q_DECLARE_METATYPE(QGradient*)
Q_DECLARE_METATYPE(QLinearGradient)

void tst_QScriptEngine::castWithPrototypeChain()
{
    QScriptEngine eng;
    Bar bar;
    Baz baz;
    QScriptValue barProto = qScriptValueFromValue(&eng, &bar);
    QScriptValue bazProto = qScriptValueFromValue(&eng, &baz);
    bazProto.setPrototype(barProto); // establish chain
    eng.setDefaultPrototype(qMetaTypeId<Bar*>(), barProto);
    eng.setDefaultPrototype(qMetaTypeId<Baz*>(), bazProto);

    Baz baz2;
    baz2.a = 123;
    baz2.b = 456;
    QScriptValue baz2Value = qScriptValueFromValue(&eng, &baz2);
    {
        Baz *pbaz = qscriptvalue_cast<Baz*>(baz2Value);
        QVERIFY(pbaz != 0);
        QCOMPARE(pbaz->b, baz2.b);
        // this is the essential part
        Bar *pbar = qscriptvalue_cast<Bar*>(baz2Value);
        QVERIFY(pbar != 0);
        QCOMPARE(pbar->a, baz2.a);
    }

    bazProto.setPrototype(barProto.prototype()); // kill chain
    {
        Baz *pbaz = qscriptvalue_cast<Baz*>(baz2Value);
        QVERIFY(pbaz != 0);
        // should not work anymore
        Bar *pbar = qscriptvalue_cast<Bar*>(baz2Value);
        QVERIFY(pbar == 0);
    }

    bazProto.setPrototype(eng.newQObject(this));
    {
        Baz *pbaz = qscriptvalue_cast<Baz*>(baz2Value);
        QVERIFY(pbaz != 0);
        // should not work now either
        Bar *pbar = qscriptvalue_cast<Bar*>(baz2Value);
        QVERIFY(pbar == 0);
    }

    {
        QScriptValue b = qScriptValueFromValue(&eng, QBrush());
        b.setPrototype(barProto);
        // this shows that a "wrong" cast is possible, if you
        // don't play by the rules (the pointer is actually a QBrush*)...
        Bar *pbar = qscriptvalue_cast<Bar*>(b);
        QVERIFY(pbar != 0);
    }

    {
        QScriptValue gradientProto = qScriptValueFromValue(&eng, QGradient());
        QScriptValue linearGradientProto = qScriptValueFromValue(&eng, QLinearGradient());
        linearGradientProto.setPrototype(gradientProto);
        QLinearGradient lg(10, 20, 30, 40);
        QScriptValue linearGradient = qScriptValueFromValue(&eng, lg);
        {
            QGradient *pgrad = qscriptvalue_cast<QGradient*>(linearGradient);
            QVERIFY(pgrad == 0);
        }
        linearGradient.setPrototype(linearGradientProto);
        {
            QGradient *pgrad = qscriptvalue_cast<QGradient*>(linearGradient);
            QVERIFY(pgrad != 0);
            QCOMPARE(pgrad->type(), QGradient::LinearGradient);
            QLinearGradient *plingrad = static_cast<QLinearGradient*>(pgrad);
            QCOMPARE(plingrad->start(), lg.start());
            QCOMPARE(plingrad->finalStop(), lg.finalStop());
        }
    }
}

class Klazz : public QWidget,
              public QStandardItem,
              public QGraphicsItem
{
    Q_OBJECT
public:
    Klazz(QWidget *parent = 0) : QWidget(parent) { }
    virtual QRectF boundingRect() const { return QRectF(); }
    virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) { }
};

Q_DECLARE_METATYPE(Klazz*)
Q_DECLARE_METATYPE(QStandardItem*)

void tst_QScriptEngine::castWithMultipleInheritance()
{
    QScriptEngine eng;
    Klazz klz;
    QScriptValue v = eng.newQObject(&klz);

    QCOMPARE(qscriptvalue_cast<Klazz*>(v), &klz);
    QCOMPARE(qscriptvalue_cast<QWidget*>(v), (QWidget *)&klz);
    QCOMPARE(qscriptvalue_cast<QObject*>(v), (QObject *)&klz);
    QCOMPARE(qscriptvalue_cast<QStandardItem*>(v), (QStandardItem *)&klz);
    QCOMPARE(qscriptvalue_cast<QGraphicsItem*>(v), (QGraphicsItem *)&klz);
}

void tst_QScriptEngine::gc()
{
    QScriptEngine eng;
    eng.evaluate("a = new Object(); a = new Object(); a = new Object()");
    QScriptValue a = eng.newObject();
    a = eng.newObject();
    a = eng.newObject();
    QPointer<QObject> ptr = new QObject();
    QVERIFY(ptr != 0);
    {
        QScriptValue v = eng.newQObject(ptr, QScriptEngine::ScriptOwnership);
    }
    eng.gc();
    QVERIFY(ptr == 0);
}

void tst_QScriptEngine::gcWithNestedDataStructure()
{
    QScriptEngine eng;
    eng.evaluate(
        "function makeList(size)"
        "{"
        "  var head = { };"
        "  var l = head;"
        "  for (var i = 0; i < size; ++i) {"
        "    l.data = i + \"\";"
        "    l.next = { }; l = l.next;"
        "  }"
        "  l.next = null;"
        "  return head;"
        "}");
    QCOMPARE(eng.hasUncaughtException(), false);
    const int size = 200;
    QScriptValue head = eng.evaluate(QString::fromLatin1("makeList(%0)").arg(size));
    QCOMPARE(eng.hasUncaughtException(), false);
    for (int x = 0; x < 2; ++x) {
        if (x == 1)
            eng.evaluate("gc()");
        QScriptValue l = head;
        for (int i = 0; i < 200; ++i) {
            QCOMPARE(l.property("data").toString(), QString::number(i));
            l = l.property("next");
        }
    }
}

class EventReceiver : public QObject
{
public:
    EventReceiver() {
        received = false;
    }

    bool event(QEvent *e) {
        received |= (e->type() == QEvent::User + 1);
        return QObject::event(e);
    }

    bool received;
};

void tst_QScriptEngine::processEventsWhileRunning()
{
    QString script = QString::fromLatin1(
        "var end = Number(new Date()) + 1000;"
        "var x = 0;"
        "while (Number(new Date()) < end) {"
        "    ++x;"
        "}");

    QScriptEngine eng;

    EventReceiver receiver;
    QCoreApplication::postEvent(&receiver, new QEvent(QEvent::Type(QEvent::User+1)));

    eng.evaluate(script);
    QVERIFY(!eng.hasUncaughtException());
    QVERIFY(!receiver.received);

    eng.setProcessEventsInterval(100);
    eng.evaluate(script);
    QVERIFY(!eng.hasUncaughtException());
    QVERIFY(receiver.received);
}

QTEST_MAIN(tst_QScriptEngine)
#include "tst_qscriptengine.moc"

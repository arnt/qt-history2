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

//TESTED_CLASS=
//TESTED_FILES=qscriptengine.h qscriptengine.cpp

class tst_QScriptEngine : public QObject
{
    Q_OBJECT

public:
    tst_QScriptEngine();
    virtual ~tst_QScriptEngine();

private slots:
    void createNull();
    void createUndefined();
    void createBoolean();
    void createNumber();
    void createFunction();
    void createObject();
    void createString();
    void createArray();
    void createOpaque();
    void createRegExp();
    void createDate();
    void createQObject();
    void createQClass();
    void globalObject();
    void canEvaluate_data();
    void canEvaluate();
    void evaluate_data();
    void evaluate();
    void nestedEvaluate();
    void nameId();
    void getSetDefaultPrototype();
    void valueConversion();
};

tst_QScriptEngine::tst_QScriptEngine()
{
}

tst_QScriptEngine::~tst_QScriptEngine()
{
}

void tst_QScriptEngine::createNull()
{
    QScriptEngine eng;
    QScriptValue null = eng.nullValue();
    QCOMPARE(null.isValid(), true);
    QCOMPARE(null.isNull(), true);
    QCOMPARE(null.isObject(), false);
    QCOMPARE(null.prototype().isValid(), false);
}

void tst_QScriptEngine::createUndefined()
{
    QScriptEngine eng;
    QScriptValue undefined = eng.undefinedValue();
    QCOMPARE(undefined.isValid(), true);
    QCOMPARE(undefined.isUndefined(), true);
    QCOMPARE(undefined.isObject(), false);
    QCOMPARE(undefined.prototype().isValid(), false);
}

void tst_QScriptEngine::createBoolean()
{
    QScriptEngine eng;
    QScriptValue falskt = QScriptValue(&eng, false);
    QCOMPARE(falskt.isValid(), true);
    QCOMPARE(falskt.isBoolean(), true);
    QCOMPARE(falskt.isObject(), false);
    QCOMPARE(falskt.prototype().isValid(), false);
}

void tst_QScriptEngine::createNumber()
{
    QScriptEngine eng;
    QScriptValue number = QScriptValue(&eng, 123);
    QCOMPARE(number.isValid(), true);
    QCOMPARE(number.isNumber(), true);
    QCOMPARE(number.isObject(), false);
    QCOMPARE(number.prototype().isValid(), false);
}

static QScriptValue myFunction(QScriptContext *, QScriptEngine *eng)
{
    return eng->undefinedValue();
}

void tst_QScriptEngine::createFunction()
{
    QScriptEngine eng;
    QScriptValue fun = eng.newFunction(myFunction);
    QCOMPARE(fun.isValid(), true);
    QCOMPARE(fun.isFunction(), true);
    QCOMPARE(fun.isObject(), true);
    // prototype should be Function.prototype
    QCOMPARE(fun.prototype().isValid(), true);
    QCOMPARE(fun.prototype().isFunction(), true);
    QCOMPARE(fun.prototype().strictEqualTo(eng.evaluate("Function.prototype")), true);
}

void tst_QScriptEngine::createObject()
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

void tst_QScriptEngine::createString()
{
    QScriptEngine eng;
    QScriptValue str = QScriptValue(&eng, "ciao");
    QCOMPARE(str.isValid(), true);
    QCOMPARE(str.isString(), true);
    QCOMPARE(str.isObject(), false);
    QCOMPARE(str.prototype().isValid(), false);
}

void tst_QScriptEngine::createArray()
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

void tst_QScriptEngine::createOpaque()
{
    QScriptEngine eng;
    QScriptValue opaque = eng.newVariant(QVariant());
    QCOMPARE(opaque.isValid(), true);
    QCOMPARE(opaque.isVariant(), true);
    QCOMPARE(opaque.isObject(), true);
    QCOMPARE(opaque.prototype().isValid(), true);
    QCOMPARE(opaque.prototype().isVariant(), true);
}

void tst_QScriptEngine::createRegExp()
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

void tst_QScriptEngine::createDate()
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

void tst_QScriptEngine::createQObject()
{
    QScriptEngine eng;

    {
        QScriptValue qobject = eng.newQObject(0);
        QCOMPARE(qobject.isValid(), true);
        QCOMPARE(qobject.isQObject(), true);
        QCOMPARE(qobject.isObject(), true);
        QCOMPARE(qobject.toQObject(), (QObject *)0);
    }
    {
        QScriptValue qobject = eng.newQObject(this);
        QCOMPARE(qobject.isValid(), true);
        QCOMPARE(qobject.isQObject(), true);
        QCOMPARE(qobject.isObject(), true);
        QCOMPARE(qobject.toQObject(), this);
        // prototype should be QObject.prototype
        QCOMPARE(qobject.prototype().isValid(), true);
        QCOMPARE(qobject.prototype().isQObject(), true);
    }
}

Q_SCRIPT_DECLARE_QMETAOBJECT(QObject, QObject*)

void tst_QScriptEngine::createQClass()
{
    QScriptEngine eng;
#if 0
    QScriptValue qclass = eng.createQClass<QObject>();
#else
    QScriptValue qclass = qScriptValueFromQMetaObject<QObject>(&eng);
#endif
    QCOMPARE(qclass.isValid(), true);
    QCOMPARE(qclass.isObject(), true);
    // prototype should be QClass.prototype
    QCOMPARE(qclass.prototype().isValid(), true);
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
    QTest::addColumn<bool>("expectHadError");
    QTest::addColumn<int>("expectErrorLineNumber");

    QTest::newRow("0")      << QString("0")             << false        << 0;
    QTest::newRow("0=1")    << QString("\n0=1\n")       << true         << 1;
    QTest::newRow("a=1")    << QString("a=1\n")         << false        << 0;
    QTest::newRow("a=1;K")  << QString("a=1;\nK")       << true         << 2; /// ### checkme
}

void tst_QScriptEngine::evaluate()
{
    QFETCH(QString, code);
    QFETCH(bool, expectHadError);
    QFETCH(int, expectErrorLineNumber);

    QScriptEngine eng;
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

void tst_QScriptEngine::nameId()
{
    QScriptEngine eng;
    QScriptNameId invalidId;
    QCOMPARE(invalidId.isValid(), false);
    QScriptNameId id = eng.nameId(QLatin1String("Object"));
    QCOMPARE(id.isValid(), true);
    QScriptNameId sameId = eng.nameId(QLatin1String("Object"));
    QCOMPARE(id, sameId);
    QScriptNameId otherId = eng.nameId(QLatin1String("object"));
    QCOMPARE(otherId.isValid(), true);
    QVERIFY(id != otherId);
    otherId = eng.nameId(QLatin1String("Function"));
    QVERIFY(id != otherId);
}

struct Foo {
public:
    int x, y;
    Foo() : x(-1), y(-1) { }
};

Q_DECLARE_METATYPE(Foo)

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
}

QTEST_MAIN(tst_QScriptEngine)
#include "tst_qscriptengine.moc"

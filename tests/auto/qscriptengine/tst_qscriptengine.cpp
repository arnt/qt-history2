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
    void addRemoveRootObject();
    void nameId();
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
    QScriptValue null = eng.nullScriptValue();
    QCOMPARE(null.isValid(), true);
    QCOMPARE(null.isNull(), true);
    QCOMPARE(null.isObject(), false);
    QCOMPARE(null.prototype().isValid(), false);
}

void tst_QScriptEngine::createUndefined()
{
    QScriptEngine eng;
    QScriptValue undefined = eng.undefinedScriptValue();
    QCOMPARE(undefined.isValid(), true);
    QCOMPARE(undefined.isUndefined(), true);
    QCOMPARE(undefined.isObject(), false);
    QCOMPARE(undefined.prototype().isValid(), false);
}

void tst_QScriptEngine::createBoolean()
{
    QScriptEngine eng;
    QScriptValue falskt = eng.scriptValue(false);
    QCOMPARE(falskt.isValid(), true);
    QCOMPARE(falskt.isBoolean(), true);
    QCOMPARE(falskt.isObject(), false);
    QCOMPARE(falskt.prototype().isValid(), false);
}

void tst_QScriptEngine::createNumber()
{
    QScriptEngine eng;
    QScriptValue number = eng.scriptValue(123);
    QCOMPARE(number.isValid(), true);
    QCOMPARE(number.isNumber(), true);
    QCOMPARE(number.isObject(), false);
    QCOMPARE(number.prototype().isValid(), false);
}

static QScriptValue myFunction(QScriptContext *, QScriptEngine *eng)
{
    return eng->undefinedScriptValue();
}

void tst_QScriptEngine::createFunction()
{
    QScriptEngine eng;
    QScriptValue fun = eng.scriptValue(myFunction);
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
    QScriptValue str = eng.scriptValue("ciao");
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
    QScriptValue opaque = eng.scriptValueFromVariant(QVariant());
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
            rexp = eng.scriptValue(QRegExp("foo"));
        QCOMPARE(rexp.isValid(), true);
        QCOMPARE(rexp.isRegExp(), true);
        QCOMPARE(rexp.isObject(), true);
        // prototype should be RegExp.prototype
        QCOMPARE(rexp.prototype().isValid(), true);
        QCOMPARE(rexp.prototype().isRegExp(), true);
        QCOMPARE(rexp.prototype().strictEqualTo(eng.evaluate("RegExp.prototype")), true);
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
        QScriptValue qobject = eng.scriptValueFromQObject(0);
        QCOMPARE(qobject.isValid(), true);
        QCOMPARE(qobject.isQObject(), true);
        QCOMPARE(qobject.isObject(), true);
        QCOMPARE(qobject.toQObject(), (QObject *)0);
    }
    {
        QScriptValue qobject = eng.scriptValueFromQObject(this);
        QCOMPARE(qobject.isValid(), true);
        QCOMPARE(qobject.isQObject(), true);
        QCOMPARE(qobject.isObject(), true);
        QCOMPARE(qobject.toQObject(), this);
        // prototype should be QObject.prototype
        QCOMPARE(qobject.prototype().isValid(), true);
        QCOMPARE(qobject.prototype().isQObject(), true);
    }
}

Q_SCRIPT_DECLARE_QCLASS(QObject, QObject*)

void tst_QScriptEngine::createQClass()
{
    QScriptEngine eng;
#if 0
    QScriptValue qclass = eng.createQClass<QObject>();
#else
    QScriptValue qclass = qScriptValueFromQClass<QObject>(&eng);
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
    QTest::newRow("a=1;K")  << QString("a=1;\nK")       << true         << 1;
}

void tst_QScriptEngine::evaluate()
{
    QFETCH(QString, code);
    QFETCH(bool, expectHadError);
    QFETCH(int, expectErrorLineNumber);

    QScriptEngine eng;
    (void)eng.evaluate(code);
    QCOMPARE(eng.uncaughtException(), expectHadError);
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
    eng.globalObject().setProperty("fun", eng.scriptValue(eval_nested));
    QScriptValue result = eng.evaluate("o = { id:'foo'}; o.fun = fun; o.fun()");
    QCOMPARE(result.property("thisObjectIdBefore").toString(), QString("foo"));
    QCOMPARE(result.property("thisObjectIdAfter").toString(), QString("foo"));
    QCOMPARE(result.property("evaluatedThisObjectId").toString(), QString("foo"));
}

void tst_QScriptEngine::addRemoveRootObject()
{
    QScriptEngine eng;
    QCOMPARE(eng.rootObjects().size(), 0);

    QScriptValue object = eng.newObject();
    eng.addRootObject(object);
    QCOMPARE(eng.rootObjects().size(), 1);
    QCOMPARE(eng.rootObjects().at(0).strictEqualTo(object), true);

    QScriptValue object2 = eng.newObject();
    eng.addRootObject(object2);
    QCOMPARE(eng.rootObjects().size(), 2);
    QCOMPARE(eng.rootObjects().at(0).strictEqualTo(object), true);
    QCOMPARE(eng.rootObjects().at(1).strictEqualTo(object2), true);

    eng.removeRootObject(object);
    QCOMPARE(eng.rootObjects().size(), 1);
    QCOMPARE(eng.rootObjects().at(0).strictEqualTo(object2), true);

    eng.removeRootObject(object2);
    QCOMPARE(eng.rootObjects().size(), 0);

    // test refcounting
    eng.addRootObject(object);
    QCOMPARE(eng.rootObjects().size(), 1);
    QCOMPARE(eng.rootObjects().at(0).strictEqualTo(object), true);

    eng.addRootObject(object);
    QCOMPARE(eng.rootObjects().size(), 1);
    QCOMPARE(eng.rootObjects().at(0).strictEqualTo(object), true);

    eng.removeRootObject(object);
    QCOMPARE(eng.rootObjects().size(), 1);
    QCOMPARE(eng.rootObjects().at(0).strictEqualTo(object), true);
    eng.removeRootObject(object);
    QCOMPARE(eng.rootObjects().size(), 0);
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

QTEST_MAIN(tst_QScriptEngine)
#include "tst_qscriptengine.moc"

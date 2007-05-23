/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>

//TESTED_CLASS=
//TESTED_FILES=qscriptcontext.h qscriptcontext.cpp

class tst_QScriptContext : public QObject
{
    Q_OBJECT

public:
    tst_QScriptContext();
    virtual ~tst_QScriptContext();

private slots:
    void callee();
    void arguments();
    void thisObject();
    void returnValue();
    void throwError();
    void throwValue();
    void evaluateInFunction();
    void pushAndPopContext();
    void lineNumber();
    void backtrace();
};

tst_QScriptContext::tst_QScriptContext()
{
}

tst_QScriptContext::~tst_QScriptContext()
{
}

static QScriptValue get_callee(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->callee();
}

void tst_QScriptContext::callee()
{
    QScriptEngine eng;

    QScriptValue fun = eng.newFunction(get_callee);
    fun.setProperty("foo", QScriptValue(&eng, "bar"));
    eng.globalObject().setProperty("get_callee", fun);

    QScriptValue result = eng.evaluate("get_callee()");
    QCOMPARE(result.isFunction(), true);
    QCOMPARE(result.property("foo").toString(), QString("bar"));
}

static QScriptValue get_arguments(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue array = eng->newArray();
    for (int i = 0; i < ctx->argumentCount(); ++i)
        array.setProperty(QString::number(i), ctx->argument(i));
    return array;
}

static QScriptValue get_argumentsObject(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->argumentsObject();
}

void tst_QScriptContext::arguments()
{
    QScriptEngine eng;

    {
        QScriptValue fun = eng.newFunction(get_arguments);
        eng.globalObject().setProperty("get_arguments", fun);
    }

    {
        QScriptValue result = eng.evaluate("get_arguments()");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property("length").toUInt32(), quint32(0));
    }

    {
        QScriptValue result = eng.evaluate("get_arguments(123)");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property("length").toUInt32(), quint32(1));
        QCOMPARE(result.property("0").isNumber(), true);
        QCOMPARE(result.property("0").toNumber(), 123.0);
    }

    {
        QScriptValue result = eng.evaluate("get_arguments(\"ciao\", null, true, undefined)");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property("length").toUInt32(), quint32(4));
        QCOMPARE(result.property("0").isString(), true);
        QCOMPARE(result.property("0").toString(), QString("ciao"));
        QCOMPARE(result.property("1").isNull(), true);
        QCOMPARE(result.property("2").isBoolean(), true);
        QCOMPARE(result.property("2").toBoolean(), true);
        QCOMPARE(result.property("3").isUndefined(), true);
    }

    {
        QScriptValue fun = eng.newFunction(get_argumentsObject);
        eng.globalObject().setProperty("get_argumentsObject", fun);
    }

    {
        QScriptValue fun = eng.evaluate("get_argumentsObject");
        QCOMPARE(fun.isFunction(), true);
        QScriptValue result = eng.evaluate("get_argumentsObject()");
        QCOMPARE(result.isArray(), false);
        QCOMPARE(result.property("length").toUInt32(), quint32(0));
        QCOMPARE(result.property("callee").strictlyEquals(fun), true);
    }

    {
        QScriptValue result = eng.evaluate("get_argumentsObject(123)");
        QCOMPARE(result.isArray(), false);
        QCOMPARE(result.property("length").toUInt32(), quint32(1));
        QCOMPARE(result.property("0").isNumber(), true);
        QCOMPARE(result.property("0").toNumber(), 123.0);
    }

    {
        QScriptValue result = eng.evaluate("get_argumentsObject(\"ciao\", null, true, undefined)");
        QCOMPARE(result.isArray(), false);
        QCOMPARE(result.property("length").toUInt32(), quint32(4));
        QCOMPARE(result.property("0").isString(), true);
        QCOMPARE(result.property("0").toString(), QString("ciao"));
        QCOMPARE(result.property("1").isNull(), true);
        QCOMPARE(result.property("2").isBoolean(), true);
        QCOMPARE(result.property("2").toBoolean(), true);
        QCOMPARE(result.property("3").isUndefined(), true);
    }
}

static QScriptValue get_thisObject(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->thisObject();
}

void tst_QScriptContext::thisObject()
{
    QScriptEngine eng;

    QScriptValue fun = eng.newFunction(get_thisObject);
    eng.globalObject().setProperty("get_thisObject", fun);

    {
        QScriptValue result = eng.evaluate("get_thisObject()");
        QCOMPARE(result.isObject(), true);
        QCOMPARE(result.toString(), QString("[object global]"));
    }

    {
        QScriptValue result = eng.evaluate("get_thisObject.apply(new Number(123))");
        QCOMPARE(result.isObject(), true);
        QCOMPARE(result.toNumber(), 123.0);
    }
}

void tst_QScriptContext::returnValue()
{
    QScriptEngine eng;
    eng.evaluate("123");
    QCOMPARE(eng.currentContext()->returnValue().toNumber(), 123.0);
    eng.evaluate("\"ciao\"");
    QCOMPARE(eng.currentContext()->returnValue().toString(), QString("ciao"));
}

static QScriptValue throw_Error(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError(QScriptContext::UnknownError, "foo");
}

static QScriptValue throw_TypeError(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError(QScriptContext::TypeError, "foo");
}

static QScriptValue throw_ReferenceError(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError(QScriptContext::ReferenceError, "foo");
}

static QScriptValue throw_SyntaxError(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError(QScriptContext::SyntaxError, "foo");
}

static QScriptValue throw_RangeError(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError(QScriptContext::RangeError, "foo");
}

static QScriptValue throw_URIError(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError(QScriptContext::URIError, "foo");
}

void tst_QScriptContext::throwError()
{
    QScriptEngine eng;

    {
        QScriptValue fun = eng.newFunction(throw_Error);
        eng.globalObject().setProperty("throw_Error", fun);
        QScriptValue result = eng.evaluate("throw_Error()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("Error: foo"));
    }

    {
        QScriptValue fun = eng.newFunction(throw_TypeError);
        eng.globalObject().setProperty("throw_TypeError", fun);
        QScriptValue result = eng.evaluate("throw_TypeError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("TypeError: foo"));
    }

    {
        QScriptValue fun = eng.newFunction(throw_ReferenceError);
        eng.globalObject().setProperty("throw_ReferenceError", fun);
        QScriptValue result = eng.evaluate("throw_ReferenceError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("ReferenceError: foo"));
    }

    {
        QScriptValue fun = eng.newFunction(throw_SyntaxError);
        eng.globalObject().setProperty("throw_SyntaxError", fun);
        QScriptValue result = eng.evaluate("throw_SyntaxError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("SyntaxError: foo"));
    }

    {
        QScriptValue fun = eng.newFunction(throw_RangeError);
        eng.globalObject().setProperty("throw_RangeError", fun);
        QScriptValue result = eng.evaluate("throw_RangeError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("RangeError: foo"));
    }

    {
        QScriptValue fun = eng.newFunction(throw_URIError);
        eng.globalObject().setProperty("throw_URIError", fun);
        QScriptValue result = eng.evaluate("throw_URIError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("URIError: foo"));
    }
}

static QScriptValue throw_value(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwValue(ctx->argument(0));
}

void tst_QScriptContext::throwValue()
{
    QScriptEngine eng;

    QScriptValue fun = eng.newFunction(throw_value);
    eng.globalObject().setProperty("throw_value", fun);

    {
        QScriptValue result = eng.evaluate("throw_value(123)");
        QCOMPARE(result.isError(), false);
        QCOMPARE(result.toNumber(), 123.0);
        QCOMPARE(eng.hasUncaughtException(), true);
    }
}

static QScriptValue evaluate(QScriptContext *, QScriptEngine *eng)
{
    return eng->evaluate("a = 123; a");
//    return eng->evaluate("a");
}

void tst_QScriptContext::evaluateInFunction()
{
    QScriptEngine eng;

    QScriptValue fun = eng.newFunction(evaluate);
    eng.globalObject().setProperty("evaluate", fun);

    QScriptValue result = eng.evaluate("evaluate()");
    QCOMPARE(result.isError(), false);
    QCOMPARE(result.isNumber(), true);
    QCOMPARE(result.toNumber(), 123.0);
    QCOMPARE(eng.hasUncaughtException(), false);

    QCOMPARE(eng.evaluate("a").toNumber(), 123.0);
}

void tst_QScriptContext::pushAndPopContext()
{
    QScriptEngine eng;
    QScriptContext *topLevel = eng.currentContext();
    QCOMPARE(topLevel->engine(), &eng);

    QScriptContext *ctx = eng.pushContext();
    QCOMPARE(ctx->parentContext(), topLevel);
    QCOMPARE(eng.currentContext(), ctx);
    QCOMPARE(ctx->engine(), &eng);
    QCOMPARE(ctx->state(), QScriptContext::NormalState);
    QCOMPARE(ctx->isCalledAsConstructor(), false);
    QCOMPARE(ctx->argumentCount(), 0);
    QCOMPARE(ctx->argument(0).isUndefined(), true);
    QCOMPARE(ctx->argumentsObject().isObject(), true);
    QCOMPARE(ctx->activationObject().isObject(), true);
    QCOMPARE(ctx->callee().isValid(), false);
    QCOMPARE(ctx->thisObject().strictlyEquals(eng.globalObject()), true);

    QScriptContext *ctx2 = eng.pushContext();
    QCOMPARE(ctx2->parentContext(), ctx);
    QCOMPARE(eng.currentContext(), ctx2);

    eng.popContext();
    QCOMPARE(eng.currentContext(), ctx);
    eng.popContext();
    QCOMPARE(eng.currentContext(), topLevel);

    // popping the top-level context is not allowed
    eng.popContext();
    QCOMPARE(eng.currentContext(), topLevel);
}

void tst_QScriptContext::lineNumber()
{
    QScriptEngine eng;

    QScriptValue result = eng.evaluate("try { eval(\"foo = 123;\\n this[is{a{syntax|error@#$%@#% \"); } catch (e) { return e.lineNumber; } return \"not reached!\";");
    QVERIFY(!eng.hasUncaughtException());
    QVERIFY(result.isNumber());
    QCOMPARE(result.toInt32(), 1);

    result = eng.evaluate("foo = 123;\n bar = 42\n0 = 0");
    QVERIFY(eng.hasUncaughtException());
    QCOMPARE(eng.uncaughtExceptionLineNumber(), 3);
    QCOMPARE(result.property("lineNumber").toInt32(), 3);
}

static QScriptValue getBacktrace(QScriptContext *ctx, QScriptEngine *eng)
{
    return eng->toScriptValue(ctx->backtrace());
}

void tst_QScriptContext::backtrace()
{
    QScriptEngine eng;
    eng.globalObject().setProperty("bt", eng.newFunction(getBacktrace));

    QString fileName = "testfile";
    QStringList expected;
    expected << "<native>(123)@:0"
             << "foo(hello,[object Object])@testfile:2"
             << "<global>()@testfile:4";

    QScriptValue ret = eng.evaluate(
        "function foo() {\n"
        "  return bt(123);\n"
        "}\n"
        "foo('hello', { })", fileName);

    QVERIFY(ret.isArray());
    QStringList slist = qscriptvalue_cast<QStringList>(ret);
    QCOMPARE(slist, expected);
}

QTEST_MAIN(tst_QScriptContext)
#include "tst_qscriptcontext.moc"

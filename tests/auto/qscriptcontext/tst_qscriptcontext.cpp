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
    void recoverFromException();
    void evaluateInFunction();
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

    QScriptValue fun = eng.scriptValue(get_callee);
    fun.setProperty("foo", eng.scriptValue("bar"));
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

void tst_QScriptContext::arguments()
{
    QScriptEngine eng;

    QScriptValue fun = eng.scriptValue(get_arguments);
    eng.globalObject().setProperty("get_arguments", fun);

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
}

static QScriptValue get_thisObject(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->thisObject();
}

void tst_QScriptContext::thisObject()
{
    QScriptEngine eng;

    QScriptValue fun = eng.scriptValue(get_thisObject);
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
    return ctx->throwError(QScriptContext::GenericError, "foo");
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
        QScriptValue fun = eng.scriptValue(throw_Error);
        eng.globalObject().setProperty("throw_Error", fun);
        QScriptValue result = eng.evaluate("throw_Error()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("Error: foo"));
    }

    {
        QScriptValue fun = eng.scriptValue(throw_TypeError);
        eng.globalObject().setProperty("throw_TypeError", fun);
        QScriptValue result = eng.evaluate("throw_TypeError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("TypeError: foo"));
    }

    {
        QScriptValue fun = eng.scriptValue(throw_ReferenceError);
        eng.globalObject().setProperty("throw_ReferenceError", fun);
        QScriptValue result = eng.evaluate("throw_ReferenceError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("ReferenceError: foo"));
    }

    {
        QScriptValue fun = eng.scriptValue(throw_SyntaxError);
        eng.globalObject().setProperty("throw_SyntaxError", fun);
        QScriptValue result = eng.evaluate("throw_SyntaxError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("SyntaxError: foo"));
    }

    {
        QScriptValue fun = eng.scriptValue(throw_RangeError);
        eng.globalObject().setProperty("throw_RangeError", fun);
        QScriptValue result = eng.evaluate("throw_RangeError()");
        QCOMPARE(eng.hasUncaughtException(), true);
        QCOMPARE(result.isError(), true);
        QCOMPARE(result.toString(), QString("RangeError: foo"));
    }

    {
        QScriptValue fun = eng.scriptValue(throw_URIError);
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

    QScriptValue fun = eng.scriptValue(throw_value);
    eng.globalObject().setProperty("throw_value", fun);

    {
        QScriptValue result = eng.evaluate("throw_value(123)");
        QCOMPARE(result.isError(), false);
        QCOMPARE(result.toNumber(), 123.0);
        QCOMPARE(eng.hasUncaughtException(), true);
    }
}

static QScriptValue throw_and_recover(QScriptContext *ctx, QScriptEngine *eng)
{
    ctx->throwValue(ctx->argument(0));
    ctx->recoverFromException();
    return eng->scriptValue(456.0);
}

void tst_QScriptContext::recoverFromException()
{
    QScriptEngine eng;

    QScriptValue fun = eng.scriptValue(throw_and_recover);
    eng.globalObject().setProperty("throw_and_recover", fun);
    QScriptValue result = eng.evaluate("throw_and_recover(123)");
    QCOMPARE(eng.hasUncaughtException(), false);
    QCOMPARE(result.toNumber(), 456.0);
}

static QScriptValue evaluate(QScriptContext *, QScriptEngine *eng)
{
    return eng->evaluate("a = 123; a");
//    return eng->evaluate("a");
}

void tst_QScriptContext::evaluateInFunction()
{
    QScriptEngine eng;

    QScriptValue fun = eng.scriptValue(evaluate);
    eng.globalObject().setProperty("evaluate", fun);

    QScriptValue result = eng.evaluate("evaluate()");
    QCOMPARE(result.isError(), false);
    QCOMPARE(result.isNumber(), true);
    QCOMPARE(result.toNumber(), 123.0);
    QCOMPARE(eng.hasUncaughtException(), false);

    QCOMPARE(eng.evaluate("a").toNumber(), 123.0);
}

QTEST_MAIN(tst_QScriptContext)
#include "tst_qscriptcontext.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtScript/qscriptcontextinfo.h>
#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptable.h>

Q_DECLARE_METATYPE(QScriptValue)
Q_DECLARE_METATYPE(QScriptContextInfo)
Q_DECLARE_METATYPE(QList<QScriptContextInfo>)

//TESTED_CLASS=
//TESTED_FILES=script/qscriptcontextinfo.h script/qscriptcontextinfo.cpp

class tst_QScriptContextInfo : public QObject, public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QScriptValue testProperty READ testProperty WRITE setTestProperty)
public:
    tst_QScriptContextInfo();
    virtual ~tst_QScriptContextInfo();

    QScriptValue testProperty() const
    {
        return engine()->globalObject().property("getContextInfoList").call();
    }

    QScriptValue setTestProperty(const QScriptValue &) const
    {
        return engine()->globalObject().property("getContextInfoList").call();
    }

public slots:
    QScriptValue testSlot(double a, double b)
    {
        Q_UNUSED(a); Q_UNUSED(b);
        return engine()->globalObject().property("getContextInfoList").call();
    }

    QScriptValue testSlot(const QString &s)
    {
        Q_UNUSED(s);
        return engine()->globalObject().property("getContextInfoList").call();
    }

private slots:
    void nativeFunction();
    void scriptFunction();
    void qtFunction();
    void qtPropertyFunction();
};

tst_QScriptContextInfo::tst_QScriptContextInfo()
{
}

tst_QScriptContextInfo::~tst_QScriptContextInfo()
{
}

static QScriptValue getContextInfoList(QScriptContext *ctx, QScriptEngine *eng)
{
    QList<QScriptContextInfo> lst;
    while (ctx) {
        QScriptContextInfo info(ctx);
        lst.append(info);
        ctx = ctx->parentContext();
    }
    return eng->toScriptValue(lst);
}

void tst_QScriptContextInfo::nativeFunction()
{
    QScriptEngine eng;
    eng.globalObject().setProperty("getContextInfoList", eng.newFunction(getContextInfoList));

    QString fileName = "foo.qs";
    int lineNumber = 123;
    QScriptValue ret = eng.evaluate("getContextInfoList()", fileName, lineNumber);
    QList<QScriptContextInfo> lst = qscriptvalue_cast<QList<QScriptContextInfo> >(ret);
    QCOMPARE(lst.size(), 2);

    {
        // getContextInfoList()
        QScriptContextInfo info = lst.at(0);
        QCOMPARE(info.functionType(), QScriptContextInfo::NativeFunction);
        QCOMPARE(info.fileName(), QString());
        QCOMPARE(info.lineNumber(), -1);
        QCOMPARE(info.columnNumber(), -1);
        QCOMPARE(info.functionName(), QString());
        QCOMPARE(info.functionEndLineNumber(), -1);
        QCOMPARE(info.functionStartLineNumber(), -1);
        QCOMPARE(info.functionParameterNames().size(), 0);
        QCOMPARE(info.functionMetaIndex(), -1);
    }

    {
        // evaluate()
        QScriptContextInfo info = lst.at(1);
        QCOMPARE(info.functionType(), QScriptContextInfo::NativeFunction);
        QCOMPARE(info.fileName(), fileName);
        QCOMPARE(info.lineNumber(), lineNumber);
        QCOMPARE(info.columnNumber(), 1);
        QCOMPARE(info.functionName(), QString());
        QCOMPARE(info.functionEndLineNumber(), -1);
        QCOMPARE(info.functionStartLineNumber(), -1);
        QCOMPARE(info.functionParameterNames().size(), 0);
        QCOMPARE(info.functionMetaIndex(), -1);
    }
}

void tst_QScriptContextInfo::scriptFunction()
{
    QScriptEngine eng;
    eng.globalObject().setProperty("getContextInfoList", eng.newFunction(getContextInfoList));

    QString fileName = "ciao.qs";
    int lineNumber = 456;
    QScriptValue ret = eng.evaluate("function bar(a, b, c) {\n return getContextInfoList();\n}\nreturn bar()",
                                    fileName, lineNumber);
    QList<QScriptContextInfo> lst = qscriptvalue_cast<QList<QScriptContextInfo> >(ret);
    QCOMPARE(lst.size(), 3);

    // getContextInfoList()
    QCOMPARE(lst.at(0).functionType(), QScriptContextInfo::NativeFunction);

    {
        // bar()
        QScriptContextInfo info = lst.at(1);
        QCOMPARE(info.functionType(), QScriptContextInfo::ScriptFunction);
        QCOMPARE(info.fileName(), fileName);
        QCOMPARE(info.lineNumber(), lineNumber + 1);
        QCOMPARE(info.columnNumber(), 2);
        QCOMPARE(info.functionName(), QString::fromLatin1("bar"));
        QCOMPARE(info.functionStartLineNumber(), lineNumber);
        QCOMPARE(info.functionEndLineNumber(), lineNumber + 2);
        QCOMPARE(info.functionParameterNames().size(), 3);
        QCOMPARE(info.functionParameterNames().at(0), QString::fromLatin1("a"));
        QCOMPARE(info.functionParameterNames().at(1), QString::fromLatin1("b"));
        QCOMPARE(info.functionParameterNames().at(2), QString::fromLatin1("c"));
        QCOMPARE(info.functionMetaIndex(), -1);
    }

    {
        // evaluate()
        QScriptContextInfo info = lst.at(2);
        QCOMPARE(info.functionType(), QScriptContextInfo::NativeFunction);
        QCOMPARE(info.fileName(), fileName);
        QCOMPARE(info.lineNumber(), lineNumber + 3);
        QCOMPARE(info.columnNumber(), 1);
        QCOMPARE(info.functionName(), QString());
        QCOMPARE(info.functionEndLineNumber(), -1);
        QCOMPARE(info.functionStartLineNumber(), -1);
        QCOMPARE(info.functionParameterNames().size(), 0);
        QCOMPARE(info.functionMetaIndex(), -1);
    }
}

void tst_QScriptContextInfo::qtFunction()
{
    QScriptEngine eng;
    eng.globalObject().setProperty("getContextInfoList", eng.newFunction(getContextInfoList));
    eng.globalObject().setProperty("qobj", eng.newQObject(this));

    for (int x = 0; x < 2; ++x) {
        QString code;
        const char *sig;
        QStringList pnames;
        if (x == 0) {
            code = "qobj.testSlot(1, 2)";
            sig = "testSlot(double,double)";
            pnames << "a" << "b";
        } else {
            code = "qobj.testSlot('ciao')";
            sig = "testSlot(QString)";
            pnames << "s";
        }
        QScriptValue ret = eng.evaluate(code);
        QList<QScriptContextInfo> lst = qscriptvalue_cast<QList<QScriptContextInfo> >(ret);
        QCOMPARE(lst.size(), 3);

        // getContextInfoList()
        QCOMPARE(lst.at(0).functionType(), QScriptContextInfo::NativeFunction);

        {
            // testSlot(double a, double b)
            QScriptContextInfo info = lst.at(1);
            QCOMPARE(info.functionType(), QScriptContextInfo::QtFunction);
            QCOMPARE(info.fileName(), QString());
            QCOMPARE(info.lineNumber(), -1);
            QCOMPARE(info.columnNumber(), -1);
            QCOMPARE(info.functionName(), QString::fromLatin1("testSlot"));
            QCOMPARE(info.functionEndLineNumber(), -1);
            QCOMPARE(info.functionStartLineNumber(), -1);
            QCOMPARE(info.functionParameterNames().size(), pnames.size());
            QCOMPARE(info.functionParameterNames(), pnames);
            QCOMPARE(info.functionMetaIndex(), metaObject()->indexOfMethod(sig));
        }

        // evaluate()
        QCOMPARE(lst.at(0).functionType(), QScriptContextInfo::NativeFunction);
    }
}

void tst_QScriptContextInfo::qtPropertyFunction()
{
    QScriptEngine eng;
    eng.globalObject().setProperty("getContextInfoList", eng.newFunction(getContextInfoList));
    eng.globalObject().setProperty("qobj", eng.newQObject(this));

    QScriptValue ret = eng.evaluate("qobj.testProperty()");
    QList<QScriptContextInfo> lst = qscriptvalue_cast<QList<QScriptContextInfo> >(ret);
    QCOMPARE(lst.size(), 3);

    // getContextInfoList()
    QCOMPARE(lst.at(0).functionType(), QScriptContextInfo::NativeFunction);

    {
        // testProperty()
        QScriptContextInfo info = lst.at(1);
        QCOMPARE(info.functionType(), QScriptContextInfo::QtPropertyFunction);
        QCOMPARE(info.fileName(), QString());
        QCOMPARE(info.lineNumber(), -1);
        QCOMPARE(info.columnNumber(), -1);
        QCOMPARE(info.functionName(), QString::fromLatin1("testProperty"));
        QCOMPARE(info.functionEndLineNumber(), -1);
        QCOMPARE(info.functionStartLineNumber(), -1);
        QCOMPARE(info.functionParameterNames().size(), 0);
        QCOMPARE(info.functionMetaIndex(), metaObject()->indexOfProperty("testProperty"));
    }

    // evaluate()
    QCOMPARE(lst.at(0).functionType(), QScriptContextInfo::NativeFunction);
}

QTEST_MAIN(tst_QScriptContextInfo)
#include "tst_qscriptcontextinfo.moc"

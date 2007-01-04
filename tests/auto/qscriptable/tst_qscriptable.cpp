/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qdebug.h>

#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptable.h>

//TESTED_CLASS=
//TESTED_FILES=

class tst_QScriptableObject : public QObject
{
    Q_OBJECT

public:
    tst_QScriptableObject();
    virtual ~tst_QScriptableObject();

private slots:
    void test();
};

tst_QScriptableObject::tst_QScriptableObject()
{
}

tst_QScriptableObject::~tst_QScriptableObject()
{
}

class MyScriptable : public QObject, protected QScriptable
{
    Q_OBJECT
    Q_PROPERTY(int baz READ baz WRITE setBaz)
public:
    MyScriptable(QObject *parent = 0)
        : QObject(parent), m_engineWasNonZero(false)
        { }
    ~MyScriptable() { }

public slots:
    void foo();
    void setX(int x);
    void setX2(int x);
    bool isBar() const;
    int baz();
    void setBaz(int x);
    bool engineWasNonZero() const;

private:
    bool m_engineWasNonZero;
};

bool MyScriptable::engineWasNonZero() const
{
    return m_engineWasNonZero;
}

int MyScriptable::baz()
{
    m_engineWasNonZero = (engine() != 0);
    return 123;
}

void MyScriptable::setBaz(int)
{
}

void MyScriptable::foo()
{
    QVERIFY(engine() != 0);
    QCOMPARE(argumentCount(), 0);
    context()->throwError("MyScriptable.foo");
}

void MyScriptable::setX(int x)
{
    QCOMPARE(argumentCount(), 1);
    thisObject().setProperty("x", engine()->scriptValue(x));
}

void MyScriptable::setX2(int)
{
    thisObject().setProperty("x", argument(0));
}

bool MyScriptable::isBar() const
{
    QString str = thisObject().toString();
    return str.contains(QLatin1Char('@'));
}

void tst_QScriptableObject::test()
{
    QScriptEngine eng;
    MyScriptable scriptable;
    QScriptValue obj = eng.scriptValueFromQObject(&scriptable);
    eng.globalObject().setProperty("scriptable", obj);

    {
        QScriptValue ret = eng.evaluate("scriptable.foo()");
        QCOMPARE(ret.isError(), true);
        QCOMPARE(ret.toString(), QString("Error: MyScriptable.foo"));
    }

    eng.evaluate("o = { }; o.__proto__ = scriptable");

    {
        QScriptValue ret = eng.evaluate("o.setX(123); o.x");
        QCOMPARE(ret.toNumber(), 123.0);
    }

    {
        QScriptValue ret = eng.evaluate("o.setX2(456); o.x");
        QCOMPARE(ret.toNumber(), 456.0);
    }

    {
        QScriptValue ret = eng.evaluate("o.isBar()");
        QCOMPARE(ret.toBoolean(), false);
    }

    {
        QScriptValue ret = eng.evaluate("o.toString() = function() { return 'foo@bar'; }; o.isBar()");
        QCOMPARE(ret.toBoolean(), true);
    }

    QCOMPARE(scriptable.engineWasNonZero(), false);
    eng.evaluate("scriptable.baz");
    QCOMPARE(scriptable.engineWasNonZero(), true);
    (void)scriptable.property("baz");
    QCOMPARE(scriptable.engineWasNonZero(), false);
}

QTEST_MAIN(tst_QScriptableObject)
#include "tst_qscriptable.moc"

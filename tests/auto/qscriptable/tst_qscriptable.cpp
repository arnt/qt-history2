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

class MyScriptable : public QObject, protected QScriptable
{
    Q_OBJECT
    Q_PROPERTY(int baz READ baz WRITE setBaz)
    Q_PROPERTY(QObject* zab READ zab WRITE setZab)
    Q_PROPERTY(int 0 READ baz)
    Q_PROPERTY(QObject* 1 READ zab)
    Q_PROPERTY(int oof WRITE setOof)
public:
    MyScriptable(QObject *parent = 0)
        : QObject(parent), m_lastEngine(0)
        { }
    ~MyScriptable() { }

    QScriptEngine *lastEngine() const;

    void setOof(int)
        { m_oofThisObject = context()->thisObject(); }
    QScriptValue oofThisObject() const
        { return m_oofThisObject; }

public slots:
    void foo();
    void setX(int x);
    void setX(const QString &x);
    void setX2(int x);
    bool isBar();
    int baz();
    void setBaz(int x);
    void evalIsBar();
    bool useInAnotherEngine();
    void setOtherEngine();
    QObject *zab();
    QObject *setZab(QObject *);

private:
    QScriptEngine *m_lastEngine;
    QScriptEngine *m_otherEngine;
    QScriptValue m_oofThisObject;
};

QScriptEngine *MyScriptable::lastEngine() const
{
    return m_lastEngine;
}

int MyScriptable::baz()
{
    m_lastEngine = engine();
    return 123;
}

void MyScriptable::setBaz(int)
{
    m_lastEngine = engine();
}

QObject *MyScriptable::zab()
{
    return thisObject().toQObject();
}

QObject *MyScriptable::setZab(QObject *)
{
    return thisObject().toQObject();
}

void MyScriptable::foo()
{
    m_lastEngine = engine();
    QVERIFY(engine() != 0);
    context()->throwError("MyScriptable.foo");
}

void MyScriptable::evalIsBar()
{
    engine()->evaluate("this.isBar()");
    m_lastEngine = engine();
}

bool MyScriptable::useInAnotherEngine()
{
    QScriptEngine eng;
    eng.globalObject().setProperty("foo", eng.newQObject(this));
    eng.evaluate("foo.baz()");
    m_lastEngine = engine();
    return (m_otherEngine == &eng);
}

void MyScriptable::setOtherEngine()
{
    m_otherEngine = engine();
}

void MyScriptable::setX(int x)
{
    m_lastEngine = engine();
    thisObject().setProperty("x", QScriptValue(engine(), x));
}

void MyScriptable::setX(const QString &x)
{
    m_lastEngine = engine();
    thisObject().setProperty("x", QScriptValue(engine(), x));
}

void MyScriptable::setX2(int)
{
    m_lastEngine = engine();
    thisObject().setProperty("x", argument(0));
}

bool MyScriptable::isBar()
{
    m_lastEngine = engine();
    QString str = thisObject().toString();
    return str.contains(QLatin1Char('@'));
}

class tst_QScriptable : public QObject
{
    Q_OBJECT

public:
    tst_QScriptable();
    virtual ~tst_QScriptable();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void engine();
    void thisObject();
    void throwError();

private:
    QScriptEngine m_engine;
    MyScriptable m_scriptable;
};

tst_QScriptable::tst_QScriptable()
{
}

tst_QScriptable::~tst_QScriptable()
{
}

void tst_QScriptable::initTestCase()
{
    QScriptValue obj = m_engine.newQObject(&m_scriptable);
    m_engine.globalObject().setProperty("scriptable", obj);
}

void tst_QScriptable::cleanupTestCase()
{
}

void tst_QScriptable::engine()
{
    QCOMPARE(m_scriptable.lastEngine(), (QScriptEngine *)0);

    // reading property
    {
        QScriptValue ret = m_engine.evaluate("scriptable.baz");
        QCOMPARE(ret.strictEqualTo(QScriptValue(&m_engine, 123)), true);
    }
    QCOMPARE(m_scriptable.lastEngine(), &m_engine);
    {
        QScriptValue ret = m_engine.evaluate("scriptable[0]");
        QCOMPARE(ret.strictEqualTo(QScriptValue(&m_engine, 123)), true);
    }
    QCOMPARE(m_scriptable.lastEngine(), &m_engine);
    // when reading from C++, engine() should be 0
    (void)m_scriptable.property("baz");
    QCOMPARE(m_scriptable.lastEngine(), (QScriptEngine *)0);

    // writing property
    m_engine.evaluate("scriptable.baz = 123");
    QCOMPARE(m_scriptable.lastEngine(), &m_engine);
    (void)m_scriptable.setProperty("baz", 123);
    QCOMPARE(m_scriptable.lastEngine(), (QScriptEngine *)0);

    // calling slot
    m_engine.evaluate("scriptable.setX(123)");
    QCOMPARE(m_scriptable.lastEngine(), &m_engine);
    QCOMPARE(m_engine.evaluate("scriptable.x")
             .strictEqualTo(QScriptValue(&m_engine, 123)), true);
    (void)m_scriptable.setProperty("baz", 123);
    QCOMPARE(m_scriptable.lastEngine(), (QScriptEngine *)0);

    // calling overloaded slot
    m_engine.evaluate("scriptable.setX('123')");
    QCOMPARE(m_scriptable.lastEngine(), &m_engine);
    QCOMPARE(m_engine.evaluate("scriptable.x")
             .strictEqualTo(QScriptValue(&m_engine, QLatin1String("123"))), true);

    // calling a slot from another slot
    m_engine.evaluate("scriptable.evalIsBar()");
    QCOMPARE(m_scriptable.lastEngine(), &m_engine);

    // calling a slot that registers m_scriptable in a different engine
    // and calls evaluate()
    {
        QScriptValue ret = m_engine.evaluate("scriptable.useInAnotherEngine()");
        QCOMPARE(m_scriptable.lastEngine(), &m_engine);
    }
}

void tst_QScriptable::thisObject()
{
    m_engine.evaluate("o = { }; o.__proto__ = scriptable");
    {
        QScriptValue ret = m_engine.evaluate("o.setX(123); o.x");
        QCOMPARE(ret.strictEqualTo(QScriptValue(&m_engine, 123)), true);
    }
    {
        QScriptValue ret = m_engine.evaluate("o.setX2(456); o.x");
        QCOMPARE(ret.strictEqualTo(QScriptValue(&m_engine, 456)), true);
    }
    {
        QScriptValue ret = m_engine.evaluate("o.isBar()");
        QCOMPARE(ret.strictEqualTo(QScriptValue(&m_engine, false)), true);
    }
    {
        QScriptValue ret = m_engine.evaluate("o.toString = function() { return 'foo@bar'; }; o.isBar()");
        QCOMPARE(ret.strictEqualTo(QScriptValue(&m_engine, true)), true);
    }

    // property getter
    {
        QScriptValue ret = m_engine.evaluate("scriptable.zab");
        QCOMPARE(ret.isQObject(), true);
        QCOMPARE(ret.toQObject(), (QObject *)&m_scriptable);
    }
    {
        QScriptValue ret = m_engine.evaluate("scriptable[1]");
        QCOMPARE(ret.isQObject(), true);
        QCOMPARE(ret.toQObject(), (QObject *)&m_scriptable);
    }
    {
        QScriptValue ret = m_engine.evaluate("o.zab");
        QCOMPARE(ret.toQObject(), (QObject *)0);
    }
    // property setter
    {
        QScriptValue ret = m_engine.evaluate("scriptable.setZab(null)");
        QCOMPARE(ret.isQObject(), true);
        QCOMPARE(ret.toQObject(), (QObject *)&m_scriptable);
    }
    {
        QVERIFY(!m_scriptable.oofThisObject().isValid());
        m_engine.evaluate("o.oof = 123");
        QVERIFY(m_scriptable.oofThisObject().strictEqualTo(m_engine.evaluate("o")));
    }
    {
        m_engine.evaluate("scriptable.oof = 123");
        QVERIFY(m_scriptable.oofThisObject().strictEqualTo(m_engine.evaluate("scriptable")));
    }

    m_engine.evaluate("delete o");
}

void tst_QScriptable::throwError()
{
    QScriptValue ret = m_engine.evaluate("scriptable.foo()");
    QCOMPARE(ret.isError(), true);
    QCOMPARE(ret.toString(), QString("Error: MyScriptable.foo"));
}

QTEST_MAIN(tst_QScriptable)
#include "tst_qscriptable.moc"

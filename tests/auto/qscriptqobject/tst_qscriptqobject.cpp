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
#include <qscriptcontext.h>

//TESTED_CLASS=
//TESTED_FILES=qscriptextqobject.h qscriptextqobject.cpp

QScriptValue fromLongLong(QScriptEngine *eng, const qlonglong &ll)
{
    return eng->scriptValue(ll);
}
    
void toLongLong(const QScriptValue &value, qlonglong &ll)
{
    ll = qlonglong(value.toNumber());
}

class MyQObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int intProperty READ intProperty WRITE setIntProperty)
    Q_PROPERTY(QVariant variantProperty READ variantProperty WRITE setVariantProperty)
    Q_PROPERTY(double hiddenProperty READ hiddenProperty WRITE setHiddenProperty SCRIPTABLE false)
    Q_ENUMS(Policy Strategy)
    Q_FLAGS(Ability)

public:
    enum Policy {
        FooPolicy = 0,
        BarPolicy,
        BazPolicy
    };

    enum Strategy {
        FooStrategy = 10,
        BarStrategy,
        BazStrategy
    };

    enum AbilityFlag {
        NoAbility  = 0x000,
        FooAbility = 0x001,
        BarAbility = 0x080,
        BazAbility = 0x200,
        AllAbility = FooAbility | BarAbility | BazAbility
    };

    Q_DECLARE_FLAGS(Ability, AbilityFlag)

    MyQObject(QObject *parent = 0)
        : QObject(parent),
          m_intValue(123),
          m_variantValue(QString("foo")),
          m_hiddenValue(456.0),
          m_qtFunctionInvoked(-1)
        { }

    int intProperty() const
        { return m_intValue; }
    void setIntProperty(int value)
        { m_intValue = value; }

    QVariant variantProperty() const
        { return m_variantValue; }
    void setVariantProperty(const QVariant &value)
        { m_variantValue = value; }

    double hiddenProperty() const
        { return m_hiddenValue; }
    void setHiddenProperty(double value)
        { m_hiddenValue = value; }

    int qtFunctionInvoked() const
        { return m_qtFunctionInvoked; }

    QVariantList qtFunctionActuals() const
        { return m_actuals; }

    void resetQtFunctionInvoked()
        { m_qtFunctionInvoked = -1; m_actuals.clear(); }

    Q_INVOKABLE void myInvokable()
        { m_qtFunctionInvoked = 0; }
    Q_INVOKABLE void myInvokableWithIntArg(int arg)
        { m_qtFunctionInvoked = 1; m_actuals << arg; }
    Q_INVOKABLE void myInvokableWithLonglongArg(qlonglong arg)
        { m_qtFunctionInvoked = 2; m_actuals << arg; }
    Q_INVOKABLE void myInvokableWithFloatArg(float arg)
        { m_qtFunctionInvoked = 3; m_actuals << arg; }
    Q_INVOKABLE void myInvokableWithDoubleArg(double arg)
        { m_qtFunctionInvoked = 4; m_actuals << arg; }
    Q_INVOKABLE void myInvokableWithStringArg(const QString &arg)
        { m_qtFunctionInvoked = 5; m_actuals << arg; }
    Q_INVOKABLE void myInvokableWithIntArgs(int arg1, int arg2)
        { m_qtFunctionInvoked = 6; m_actuals << arg1 << arg2; }
    Q_INVOKABLE int myInvokableReturningInt()
        { m_qtFunctionInvoked = 7; return 123; }
    Q_INVOKABLE QString myInvokableReturningString()
        { m_qtFunctionInvoked = 8; return QLatin1String("ciao"); }
    Q_INVOKABLE void myInvokableWithIntArg(int arg1, int arg2) // overload
        { m_qtFunctionInvoked = 9; m_actuals << arg1 << arg2; }
    Q_INVOKABLE void myInvokableWithEnumArg(Policy policy)
        { m_qtFunctionInvoked = 10; m_actuals << policy; }
    Q_INVOKABLE QVector<int> myInvokableReturningVectorOfInt()
        { m_qtFunctionInvoked = 11; return QVector<int>(); }
    Q_INVOKABLE void myInvokableWithVectorOfIntArg(const QVector<int> &)
        { m_qtFunctionInvoked = 12; }
    Q_INVOKABLE QObject *myInvokableReturningQObjectStar()
        { m_qtFunctionInvoked = 13; return this; }

    void emitMySignal()
        { emit mySignal(); }
    void emitMySignalWithIntArg(int arg)
        { emit mySignalWithIntArg(arg); }

public Q_SLOTS:
    void mySlot()
        { m_qtFunctionInvoked = 10; }

Q_SIGNALS:
    void mySignal();
    void mySignalWithIntArg(int arg);

private:
    int m_intValue;
    QVariant m_variantValue;
    double m_hiddenValue;
    int m_qtFunctionInvoked;
    QVariantList m_actuals;
};

class MyOtherQObject : public MyQObject
{
public:
    MyOtherQObject(QObject *parent = 0)
        : MyQObject(parent)
        { }
};

class tst_QScriptExtQObject : public QObject
{
    Q_OBJECT

public:
    tst_QScriptExtQObject();
    virtual ~tst_QScriptExtQObject();

public slots:
    void init();
    void cleanup();

private slots:
    void getSetStaticProperty();
    void getSetDynamicProperty();
    void getSetChildren();
    void callQtInvokable();
    void connectAndDisconnect();
    void classEnums();
    void classConstructor();
    void overrideInvokable();
    void transferInvokable();

private:
    QScriptEngine *m_engine;
    MyQObject *m_myObject;
};

tst_QScriptExtQObject::tst_QScriptExtQObject()
{
}

tst_QScriptExtQObject::~tst_QScriptExtQObject()
{
}

void tst_QScriptExtQObject::init()
{
    m_engine = new QScriptEngine();
    m_myObject = new MyQObject();
    m_engine->globalObject().setProperty("myObject", m_engine->scriptValueFromQObject(m_myObject));
    m_engine->globalObject().setProperty("global", m_engine->globalObject());
}

void tst_QScriptExtQObject::cleanup()
{
    delete m_engine;
    delete m_myObject;
}

void tst_QScriptExtQObject::getSetStaticProperty()
{
    QCOMPARE(m_engine->evaluate("myObject.noSuchProperty").isUndefined(), true);

    // initial value (set in MyQObject constructor)
    QCOMPARE(m_engine->evaluate("myObject.intProperty").toNumber(), 123.0);
    QCOMPARE(m_engine->evaluate("myObject.variantProperty").toString(), QLatin1String("foo"));

    // property change in C++ should be reflected in script
    m_myObject->setIntProperty(456);
    QCOMPARE(m_engine->evaluate("myObject.intProperty").toNumber(), 456.0);
    m_myObject->setIntProperty(789);
    QCOMPARE(m_engine->evaluate("myObject.intProperty").toNumber(), 789.0);

    m_myObject->setVariantProperty(QLatin1String("bar"));
    QCOMPARE(m_engine->evaluate("myObject.variantProperty").toString(), QLatin1String("bar"));
    m_myObject->setVariantProperty(42);
    QCOMPARE(m_engine->evaluate("myObject.variantProperty").toNumber(), 42.0);

    // property change in script should be reflected in C++
    QCOMPARE(m_engine->evaluate("myObject.intProperty = 123;"
                          "myObject.intProperty").toNumber(), 123.0);
    QCOMPARE(m_myObject->intProperty(), 123);
    QCOMPARE(m_engine->evaluate("myObject.intProperty = \"ciao!\";"
                          "myObject.intProperty").toNumber(), 0.0);
    QCOMPARE(m_myObject->intProperty(), 0);
    QCOMPARE(m_engine->evaluate("myObject.intProperty = \"123\";"
                          "myObject.intProperty").toNumber(), 123.0);
    QCOMPARE(m_myObject->intProperty(), 123);

    QCOMPARE(m_engine->evaluate("myObject.variantProperty = \"foo\";"
                          "myObject.variantProperty").toString(), QLatin1String("foo"));
    QCOMPARE(m_myObject->variantProperty().toString(), QLatin1String("foo"));
    QCOMPARE(m_engine->evaluate("myObject.variantProperty = 42;"
                          "myObject.variantProperty").toNumber(), 42.0);
    QCOMPARE(m_myObject->variantProperty().toDouble(), 42.0);

    // try to delete
    QCOMPARE(m_engine->evaluate("delete myObject.intProperty").toBoolean(), false);
    QCOMPARE(m_engine->evaluate("myObject.intProperty").toNumber(), 123.0);

    QCOMPARE(m_engine->evaluate("delete myObject.variantProperty").toBoolean(), false);
    QCOMPARE(m_engine->evaluate("myObject.variantProperty").toNumber(), 42.0);

    // non-scriptable property
    QCOMPARE(m_myObject->hiddenProperty(), 456.0);
    QCOMPARE(m_engine->evaluate("myObject.hiddenProperty").isUndefined(), true);
    QCOMPARE(m_engine->evaluate("myObject.hiddenProperty = 123;"
                                "myObject.hiddenProperty").toInt32(), 123);
    QCOMPARE(m_myObject->hiddenProperty(), 456.0);
}

void tst_QScriptExtQObject::getSetDynamicProperty()
{
    // initially the object does not have the property
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"dynamicProperty\")").toBoolean(), false);

    // add a dynamic property in C++
    QCOMPARE(m_myObject->setProperty("dynamicProperty", 123), false);
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"dynamicProperty\")").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty").toNumber(), 123.0);

    // property change in script should be reflected in C++
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty = \"foo\";"
                             "myObject.dynamicProperty").toString(), QLatin1String("foo"));
    QCOMPARE(m_myObject->property("dynamicProperty").toString(), QLatin1String("foo"));

    // delete the property
    QCOMPARE(m_engine->evaluate("delete myObject.dynamicProperty").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty").isUndefined(), true);
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"foooo\")").toBoolean(), false);
}

void tst_QScriptExtQObject::getSetChildren()
{
    // initially the object does not have the child
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")").toBoolean(), false);

    // add a child
    MyQObject *child = new MyQObject(m_myObject);
    child->setObjectName("child");
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")").toBoolean(), true);

    // add a grandchild
    MyQObject *grandChild = new MyQObject(child);
    grandChild->setObjectName("grandChild");
    QCOMPARE(m_engine->evaluate("myObject.child.hasOwnProperty(\"grandChild\")").toBoolean(), true);

    // delete grandchild
    delete grandChild;
    QCOMPARE(m_engine->evaluate("myObject.child.hasOwnProperty(\"grandChild\")").toBoolean(), false);

    // delete child
    delete child;
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")").toBoolean(), false);

}

template <class Container>
QScriptValue fromContainer(QScriptEngine *eng, const Container &cont)
{
    QScriptValue a = eng->newArray();
    typename Container::const_iterator begin = cont.begin();
    typename Container::const_iterator end = cont.end();
    typename Container::const_iterator it;
    for (it = begin; it != end; ++it)
        a.setProperty(eng->scriptValue(it - begin).toString(), qScriptValueFromValue(eng, *it));
    return a;
}

template <class Container>
void toContainer(const QScriptValue &value, Container &cont)
{
    QScriptEngine *eng = value.engine();
    Q_ASSERT(eng);
    quint32 len = value.property("length").toUInt32();
    for (quint32 i = 0; i < len; ++i) {
        QScriptValue item = value.property(eng->scriptValue(i).toString());
        cont.push_back(qscript_cast<typename Container::value_type>(item));
    }
}

Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<double>)
Q_DECLARE_METATYPE(QVector<QString>)
    
void tst_QScriptExtQObject::callQtInvokable()
{
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokable()").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 0);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);

    m_myObject->resetQtFunctionInvoked();
    qScriptRegisterMetaType<qlonglong>(m_engine, fromLongLong, toLongLong);
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithLonglongArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 2);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toLongLong(), qlonglong(123));

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithFloatArg(123.5)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 3);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toDouble(), 123.5);

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithDoubleArg(123.5)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 4);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toDouble(), 123.5);

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithStringArg(\"ciao\")").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 5);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toString(), QLatin1String("ciao"));

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithIntArgs(123, 456)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 6);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 2);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
    QCOMPARE(m_myObject->qtFunctionActuals().at(1).toInt(), 456);

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningInt()").toNumber(), 123.0);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 7);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningString()").toString(), QLatin1String("ciao"));
    QCOMPARE(m_myObject->qtFunctionInvoked(), 8);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithIntArg(123, 456)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 9);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 2);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
    QCOMPARE(m_myObject->qtFunctionActuals().at(1).toInt(), 456);

    // first time we expect failure because the metatype is not registered
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningVectorOfInt()").isError(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), -1);

    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithVectorOfIntArg(0)").isError(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), -1);

    // now we register it, and it should work
    qScriptRegisterMetaType<QVector<int> >(m_engine, fromContainer, toContainer);
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableReturningVectorOfInt()");
        QCOMPARE(ret.isArray(), true);
        QCOMPARE(m_myObject->qtFunctionInvoked(), 11);
    }

    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithVectorOfIntArg(myObject.myInvokableReturningVectorOfInt())");
        QCOMPARE(ret.isUndefined(), true);
        QCOMPARE(m_myObject->qtFunctionInvoked(), 12);
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableReturningQObjectStar()");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 13);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 0);
        QCOMPARE(ret.isQObject(), true);
        QCOMPARE(ret.toQObject(), m_myObject);
    }
}

void tst_QScriptExtQObject::connectAndDisconnect()
{
    m_engine->evaluate("myHandler = function() { gotSignal = true; global.signalArgs = arguments; }");
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(myHandler)").toBoolean(), true);

    m_engine->evaluate("gotSignal = false");
    m_engine->evaluate("myObject.mySignal()");
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 0.0);

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 0.0);

    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myHandler)").toBoolean(), true);

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignalWithIntArg(123);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("signalArgs[0]").toNumber(), 123.0);

    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(myHandler)").toBoolean(), true);
}

void tst_QScriptExtQObject::classEnums()
{
    QScriptValue myClass = m_engine->scriptValue(m_myObject->metaObject(), m_engine->undefinedScriptValue());
    m_engine->globalObject().setProperty("MyQObject", myClass);

    QCOMPARE(static_cast<MyQObject::Policy>(m_engine->evaluate("MyQObject.FooPolicy").toInt32()),
             MyQObject::FooPolicy);
    QCOMPARE(static_cast<MyQObject::Policy>(m_engine->evaluate("MyQObject.BarPolicy").toInt32()),
             MyQObject::BarPolicy);
    QCOMPARE(static_cast<MyQObject::Policy>(m_engine->evaluate("MyQObject.BazPolicy").toInt32()),
             MyQObject::BazPolicy);

    QCOMPARE(static_cast<MyQObject::Strategy>(m_engine->evaluate("MyQObject.FooStrategy").toInt32()),
             MyQObject::FooStrategy);
    QCOMPARE(static_cast<MyQObject::Strategy>(m_engine->evaluate("MyQObject.BarStrategy").toInt32()),
             MyQObject::BarStrategy);
    QCOMPARE(static_cast<MyQObject::Strategy>(m_engine->evaluate("MyQObject.BazStrategy").toInt32()),
             MyQObject::BazStrategy);

    QCOMPARE(static_cast<MyQObject::Ability>(m_engine->evaluate("MyQObject.NoAbility").toInt32()),
             MyQObject::NoAbility);
    QCOMPARE(static_cast<MyQObject::Ability>(m_engine->evaluate("MyQObject.FooAbility").toInt32()),
             MyQObject::FooAbility);
    QCOMPARE(static_cast<MyQObject::Ability>(m_engine->evaluate("MyQObject.BarAbility").toInt32()),
             MyQObject::BarAbility);
    QCOMPARE(static_cast<MyQObject::Ability>(m_engine->evaluate("MyQObject.BazAbility").toInt32()),
             MyQObject::BazAbility);
    QCOMPARE(static_cast<MyQObject::Ability>(m_engine->evaluate("MyQObject.AllAbility").toInt32()),
             MyQObject::AllAbility);

    qRegisterMetaType<MyQObject::Policy>("Policy");

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithEnumArg(MyQObject.BazPolicy)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 10);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), int(MyQObject::BazPolicy));
}

Q_SCRIPT_DECLARE_QCLASS(MyQObject, QObject*)
Q_SCRIPT_DECLARE_QCLASS(QObject, QObject*)

void tst_QScriptExtQObject::classConstructor()
{
    QScriptValue myClass = qScriptValueFromQClass<MyQObject>(m_engine);
    m_engine->globalObject().setProperty("MyQObject", myClass);

    QScriptValue myObj = m_engine->evaluate("myObj = MyQObject()");
    QObject *qobj = myObj.toQObject();
    QVERIFY(qobj != 0);
    QCOMPARE(qobj->metaObject()->className(), "MyQObject");
    QCOMPARE(qobj->parent(), (QObject *)0);

    QScriptValue qobjectClass = qScriptValueFromQClass<QObject>(m_engine);
    m_engine->globalObject().setProperty("QObject", qobjectClass);

    QScriptValue otherObj = m_engine->evaluate("otherObj = QObject(myObj)");
    QObject *qqobj = otherObj.toQObject();
    QVERIFY(qqobj != 0);
    QCOMPARE(qqobj->metaObject()->className(), "QObject");
    QCOMPARE(qqobj->parent(), qobj);
}

void tst_QScriptExtQObject::overrideInvokable()
{
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myInvokable = function() { global.a = 123; }");
    m_engine->evaluate("myObject.myInvokable()");
    QCOMPARE(m_myObject->qtFunctionInvoked(), -1);
    QCOMPARE(m_engine->evaluate("global.a").toNumber(), 123.0);
    m_engine->evaluate("myObject.myInvokable = function() { global.a = 456; }");
    m_engine->evaluate("myObject.myInvokable()");
    QCOMPARE(m_myObject->qtFunctionInvoked(), -1);
    QCOMPARE(m_engine->evaluate("global.a").toNumber(), 456.0);
    m_engine->evaluate("delete myObject.myInvokable");
    m_engine->evaluate("myObject.myInvokable()");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 0);
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myInvokable = myObject.myInvokableWithIntArg");
    m_engine->evaluate("myObject.myInvokable(123)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 1);
}

void tst_QScriptExtQObject::transferInvokable()
{
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.foozball = myObject.myInvokable");
    m_engine->evaluate("myObject.foozball()");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 0);
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.foozball = myObject.myInvokableWithIntArg");
    m_engine->evaluate("myObject.foozball(123)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 1);
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myInvokable = myObject.myInvokableWithIntArg");
    m_engine->evaluate("myObject.myInvokable(123)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 1);

    MyOtherQObject other;
    m_engine->globalObject().setProperty(
        "myOtherObject", m_engine->scriptValueFromQObject(&other));
    m_engine->evaluate("myOtherObject.foo = myObject.foozball");
    other.resetQtFunctionInvoked();
    m_engine->evaluate("myOtherObject.foo(456)");
    QCOMPARE(other.qtFunctionInvoked(), 1);
}

QTEST_MAIN(tst_QScriptExtQObject)
#include "tst_qscriptqobject.moc"

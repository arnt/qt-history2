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
    return QScriptValue(eng, ll);
}
    
void toLongLong(const QScriptValue &value, qlonglong &ll)
{
    ll = qlonglong(value.toNumber());
}

struct CustomType
{
    QString string;
};
Q_DECLARE_METATYPE(CustomType)

class MyQObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int intProperty READ intProperty WRITE setIntProperty)
    Q_PROPERTY(QVariant variantProperty READ variantProperty WRITE setVariantProperty)
    Q_PROPERTY(QVariantList variantListProperty READ variantListProperty WRITE setVariantListProperty)
    Q_PROPERTY(QString stringProperty READ stringProperty WRITE setStringProperty)
    Q_PROPERTY(QStringList stringListProperty READ stringListProperty WRITE setStringListProperty)
    Q_PROPERTY(QByteArray byteArrayProperty READ byteArrayProperty WRITE setByteArrayProperty)
    Q_PROPERTY(QBrush brushProperty READ brushProperty WRITE setBrushProperty)
    Q_PROPERTY(double hiddenProperty READ hiddenProperty WRITE setHiddenProperty SCRIPTABLE false)
    Q_PROPERTY(int writeOnlyProperty WRITE setWriteOnlyProperty)
    Q_PROPERTY(int readOnlyProperty READ readOnlyProperty)
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut)
    Q_PROPERTY(CustomType propWithCustomType READ propWithCustomType WRITE setPropWithCustomType)
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
          m_variantValue(QLatin1String("foo")),
          m_variantListValue(QVariantList() << QVariant(123) << QVariant(QLatin1String("foo"))),
          m_stringValue(QLatin1String("bar")),
          m_stringListValue(QStringList() << QLatin1String("zig") << QLatin1String("zag")),
          m_brushValue(QColor(10, 20, 30, 40)),
          m_hiddenValue(456.0),
          m_writeOnlyValue(789),
          m_readOnlyValue(987),
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

    QVariantList variantListProperty() const
        { return m_variantListValue; }
    void setVariantListProperty(const QVariantList &value)
        { m_variantListValue = value; }

    QString stringProperty() const
        { return m_stringValue; }
    void setStringProperty(const QString &value)
        { m_stringValue = value; }

    QStringList stringListProperty() const
        { return m_stringListValue; }
    void setStringListProperty(const QStringList &value)
        { m_stringListValue = value; }

    QByteArray byteArrayProperty() const
        { return m_byteArrayValue; }
    void setByteArrayProperty(const QByteArray &value)
        { m_byteArrayValue = value; }

    QBrush brushProperty() const
        { return m_brushValue; }
    void setBrushProperty(const QBrush &value)
        { m_brushValue = value; }

    double hiddenProperty() const
        { return m_hiddenValue; }
    void setHiddenProperty(double value)
        { m_hiddenValue = value; }

    int writeOnlyProperty() const
        { return m_writeOnlyValue; }
    void setWriteOnlyProperty(int value)
        { m_writeOnlyValue = value; }

    int readOnlyProperty() const
        { return m_readOnlyValue; }

    QKeySequence shortcut() const
        { return m_shortcut; }
    void setShortcut(const QKeySequence &seq)
        { m_shortcut = seq; }

    CustomType propWithCustomType() const
        { return m_customType; }
    void setPropWithCustomType(const CustomType &c)
        { m_customType = c; }

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
    Q_INVOKABLE QObjectList myInvokableWithQObjectListArg(const QObjectList &lst)
        { m_qtFunctionInvoked = 14; m_actuals << qVariantFromValue(lst); return lst; }
    Q_INVOKABLE QVariant myInvokableWithVariantArg(const QVariant &v)
        { m_qtFunctionInvoked = 15; m_actuals << v; return v; }
    Q_INVOKABLE QVariantMap myInvokableWithVariantMapArg(const QVariantMap &vm)
        { m_qtFunctionInvoked = 16; m_actuals << vm; return vm; }
    Q_INVOKABLE QList<int> myInvokableWithListOfIntArg(const QList<int> &lst)
        { m_qtFunctionInvoked = 17; m_actuals << qVariantFromValue(lst); return lst; }
    Q_INVOKABLE QObject* myInvokableWithQObjectStarArg(QObject *obj)
        { m_qtFunctionInvoked = 18; m_actuals << qVariantFromValue(obj); return obj; }
    Q_INVOKABLE QBrush myInvokableWithQBrushArg(const QBrush &brush)
        { m_qtFunctionInvoked = 19; m_actuals << qVariantFromValue(brush); return brush; }

    void emitMySignal()
        { emit mySignal(); }
    void emitMySignalWithIntArg(int arg)
        { emit mySignalWithIntArg(arg); }
    void emitMySignal2(bool arg)
        { emit mySignal2(arg); }
    void emitMySignal2()
        { emit mySignal2(); }

public Q_SLOTS:
    void mySlot()
        { m_qtFunctionInvoked = 20; }
    void mySlotWithIntArg(int arg)
        { m_qtFunctionInvoked = 21; m_actuals << arg; }
    void mySlotWithDoubleArg(double arg)
        { m_qtFunctionInvoked = 22; m_actuals << arg; }
    void mySlotWithStringArg(const QString &arg)
        { m_qtFunctionInvoked = 23; m_actuals << arg; }

Q_SIGNALS:
    void mySignal();
    void mySignalWithIntArg(int arg);
    void mySignalWithDoubleArg(double arg);
    void mySignal2(bool arg = false);

private:
    int m_intValue;
    QVariant m_variantValue;
    QVariantList m_variantListValue;
    QString m_stringValue;
    QStringList m_stringListValue;
    QByteArray m_byteArrayValue;
    QBrush m_brushValue;
    double m_hiddenValue;
    int m_writeOnlyValue;
    int m_readOnlyValue;
    QKeySequence m_shortcut;
    CustomType m_customType;
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
    void findChild();
    void findChildren();

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
    m_engine->globalObject().setProperty("myObject", m_engine->newQObject(m_myObject));
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
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictEqualTo(QScriptValue(m_engine, 123.0)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantProperty")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("foo"))), true);
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("bar"))), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty").isArray(), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty.length")
             .strictEqualTo(QScriptValue(m_engine, 2)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[0]")
             .strictEqualTo(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[1]")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("foo"))), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty").isArray(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty.length")
             .strictEqualTo(QScriptValue(m_engine, 2)), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[0]").isString(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[0]").toString(),
             QLatin1String("zig"));
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[1]").isString(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[1]").toString(),
             QLatin1String("zag"));

    // property change in C++ should be reflected in script
    m_myObject->setIntProperty(456);
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictEqualTo(QScriptValue(m_engine, 456)), true);
    m_myObject->setIntProperty(789);
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictEqualTo(QScriptValue(m_engine, 789)), true);

    m_myObject->setVariantProperty(QLatin1String("bar"));
    QCOMPARE(m_engine->evaluate("myObject.variantProperty")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("bar"))), true);
    m_myObject->setVariantProperty(42);
    QCOMPARE(m_engine->evaluate("myObject.variantProperty")
             .strictEqualTo(QScriptValue(m_engine, 42)), true);

    m_myObject->setStringProperty(QLatin1String("baz"));
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .equalTo(QScriptValue(m_engine, QLatin1String("baz"))), true);
    m_myObject->setStringProperty(QLatin1String("zab"));
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .equalTo(QScriptValue(m_engine, QLatin1String("zab"))), true);

    // property change in script should be reflected in C++
    QCOMPARE(m_engine->evaluate("myObject.intProperty = 123")
             .strictEqualTo(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictEqualTo(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_myObject->intProperty(), 123);
    QCOMPARE(m_engine->evaluate("myObject.intProperty = \"ciao!\";"
                                "myObject.intProperty")
             .strictEqualTo(QScriptValue(m_engine, 0)), true);
    QCOMPARE(m_myObject->intProperty(), 0);
    QCOMPARE(m_engine->evaluate("myObject.intProperty = \"123\";"
                                "myObject.intProperty")
             .strictEqualTo(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_myObject->intProperty(), 123);

    QCOMPARE(m_engine->evaluate("myObject.stringProperty = 'ciao'")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("ciao"))), true);
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("ciao"))), true);
    QCOMPARE(m_myObject->stringProperty(), QLatin1String("ciao"));
    QCOMPARE(m_engine->evaluate("myObject.stringProperty = 123;"
                                "myObject.stringProperty")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("123"))), true);
    QCOMPARE(m_myObject->stringProperty(), QLatin1String("123"));

    QCOMPARE(m_engine->evaluate("myObject.variantProperty = \"foo\";"
                                "myObject.variantProperty").toString(), QLatin1String("foo"));
    QCOMPARE(m_myObject->variantProperty(), QVariant(QLatin1String("foo")));
    QCOMPARE(m_engine->evaluate("myObject.variantProperty = 42;"
                                "myObject.variantProperty").toNumber(), 42.0);
    QCOMPARE(m_myObject->variantProperty().toDouble(), 42.0);

    QCOMPARE(m_engine->evaluate("myObject.variantListProperty = [1, 'two', true];"
                                "myObject.variantListProperty.length")
             .strictEqualTo(QScriptValue(m_engine, 3)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[0]")
             .strictEqualTo(QScriptValue(m_engine, 1)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[1]")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("two"))), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[2]")
             .strictEqualTo(QScriptValue(m_engine, true)), true);
    {
        QVariantList vl = qscriptvalue_cast<QVariantList>(m_engine->evaluate("myObject.variantListProperty"));
        QCOMPARE(vl, QVariantList()
                 << QVariant(1)
                 << QVariant(QLatin1String("two"))
                 << QVariant(true));
    }

    QCOMPARE(m_engine->evaluate("myObject.stringListProperty = [1, 'two', true];"
                                "myObject.stringListProperty.length")
             .strictEqualTo(QScriptValue(m_engine, 3)), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[0]").isString(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[0]").toString(),
             QLatin1String("1"));
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[1]").isString(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[1]").toString(),
             QLatin1String("two"));
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[2]").isString(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[2]").toString(),
             QLatin1String("true"));
    {
        QStringList sl = qscriptvalue_cast<QStringList>(m_engine->evaluate("myObject.stringListProperty"));
        QCOMPARE(sl, QStringList()
                 << QLatin1String("1")
                 << QLatin1String("two")
                 << QLatin1String("true"));
    }

    // test setting properties where we can't convert the type natively but where the
    // types happen to be compatible variant types already
    {
        QKeySequence sequence(Qt::ControlModifier + Qt::AltModifier + Qt::Key_Delete);
        QScriptValue mobj = m_engine->globalObject().property("myObject");

        QVERIFY(m_myObject->shortcut().isEmpty());
        mobj.setProperty("shortcut", m_engine->newVariant(sequence));
        QVERIFY(m_myObject->shortcut() == sequence);
    }
    {
        CustomType t; t.string = "hello";
        QScriptValue mobj = m_engine->globalObject().property("myObject");

        QVERIFY(m_myObject->propWithCustomType().string.isEmpty());
        mobj.setProperty("propWithCustomType", m_engine->newVariant(qVariantFromValue(t)));
        QVERIFY(m_myObject->propWithCustomType().string == t.string);
    }

    // test that we do value conversion if necessary when setting properties
    {
        QScriptValue br = m_engine->evaluate("myObject.brushProperty");
        QCOMPARE(qscriptvalue_cast<QBrush>(br), m_myObject->brushProperty());
        QCOMPARE(qscriptvalue_cast<QColor>(br), m_myObject->brushProperty().color());

        QColor newColor(40, 30, 20, 10);
        QScriptValue val = qScriptValueFromValue(m_engine, newColor);
        m_engine->globalObject().setProperty("myColor", val);
        QScriptValue ret = m_engine->evaluate("myObject.brushProperty = myColor");
        QCOMPARE(ret.strictEqualTo(val), true);
        br = m_engine->evaluate("myObject.brushProperty");
        QCOMPARE(qscriptvalue_cast<QBrush>(br), QBrush(newColor));
        QCOMPARE(qscriptvalue_cast<QColor>(br), newColor);

        m_engine->globalObject().setProperty("myColor", QScriptValue());
    }

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

    // write-only property
    QCOMPARE(m_myObject->writeOnlyProperty(), 789);
    QCOMPARE(m_engine->evaluate("myObject.writeOnlyProperty").isUndefined(), true);
    QCOMPARE(m_engine->evaluate("myObject.writeOnlyProperty = 123;"
                                "myObject.writeOnlyProperty").isUndefined(), true);
    QCOMPARE(m_myObject->writeOnlyProperty(), 123);

    // read-only property
    QCOMPARE(m_myObject->readOnlyProperty(), 987);
    QCOMPARE(m_engine->evaluate("myObject.readOnlyProperty").toInt32(), 987);
    QCOMPARE(m_engine->evaluate("myObject.readOnlyProperty = 654;"
                                "myObject.readOnlyProperty").toInt32(), 987);
    QCOMPARE(m_myObject->readOnlyProperty(), 987);
}

void tst_QScriptExtQObject::getSetDynamicProperty()
{
    // initially the object does not have the property
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"dynamicProperty\")")
             .strictEqualTo(QScriptValue(m_engine, false)), true);

    // add a dynamic property in C++
    QCOMPARE(m_myObject->setProperty("dynamicProperty", 123), false);
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"dynamicProperty\")")
             .strictEqualTo(QScriptValue(m_engine, true)), true);
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty")
             .strictEqualTo(QScriptValue(m_engine, 123)), true);

    // property change in script should be reflected in C++
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty = \"foo\";"
                                "myObject.dynamicProperty")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("foo"))), true);
    QCOMPARE(m_myObject->property("dynamicProperty").toString(), QLatin1String("foo"));

    // delete the property
    QCOMPARE(m_engine->evaluate("delete myObject.dynamicProperty").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty").isUndefined(), true);
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"foooo\")").toBoolean(), false);
}

void tst_QScriptExtQObject::getSetChildren()
{
    // initially the object does not have the child
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")")
             .strictEqualTo(QScriptValue(m_engine, false)), true);

    // add a child
    MyQObject *child = new MyQObject(m_myObject);
    child->setObjectName("child");
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")")
             .strictEqualTo(QScriptValue(m_engine, true)), true);

    // add a grandchild
    MyQObject *grandChild = new MyQObject(child);
    grandChild->setObjectName("grandChild");
    QCOMPARE(m_engine->evaluate("myObject.child.hasOwnProperty(\"grandChild\")")
             .strictEqualTo(QScriptValue(m_engine, true)), true);

    // delete grandchild
    delete grandChild;
    QCOMPARE(m_engine->evaluate("myObject.child.hasOwnProperty(\"grandChild\")")
             .strictEqualTo(QScriptValue(m_engine, false)), true);

    // delete child
    delete child;
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")")
             .strictEqualTo(QScriptValue(m_engine, false)), true);

}

Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<double>)
Q_DECLARE_METATYPE(QVector<QString>)
Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(QList<int>)

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
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningInt()")
             .strictEqualTo(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 7);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningString()")
             .strictEqualTo(QScriptValue(m_engine, QLatin1String("ciao"))), true);
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
    qScriptRegisterSequenceMetaType<QVector<int> >(m_engine);
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

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithQObjectListArg([myObject])");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 14);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
        QCOMPARE(ret.isArray(), true);
        QCOMPARE(ret.property(QLatin1String("length"))
                 .strictEqualTo(QScriptValue(m_engine, 1)), true);
        QCOMPARE(ret.property(QLatin1String("0")).isQObject(), true);
        QCOMPARE(ret.property(QLatin1String("0")).toQObject(), m_myObject);
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithVariantArg(myObject.variantProperty)");
        QCOMPARE(ret.isVariant(), true);
        QCOMPARE(m_myObject->qtFunctionInvoked(), 15);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
        QCOMPARE(m_myObject->qtFunctionActuals().at(0), m_myObject->variantProperty());
        QCOMPARE(ret.toVariant(), m_myObject->variantProperty());
    }

    m_engine->globalObject().setProperty("fishy", m_engine->newVariant(123));
    m_engine->evaluate("myObject.myInvokableWithStringArg(fishy)");

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithVariantMapArg({ a:123, b:'ciao' })");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 16);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
        QVariant v = m_myObject->qtFunctionActuals().at(0);
        QCOMPARE(v.userType(), int(QMetaType::QVariantMap));
        QVariantMap vmap = qvariant_cast<QVariantMap>(v);
        QCOMPARE(vmap.keys().size(), 2);
        QCOMPARE(vmap.keys().at(0), QLatin1String("a"));
        QCOMPARE(vmap.value("a"), QVariant(123));
        QCOMPARE(vmap.keys().at(1), QLatin1String("b"));
        QCOMPARE(vmap.value("b"), QVariant("ciao"));

        QCOMPARE(ret.isObject(), true);
        QCOMPARE(ret.property("a").strictEqualTo(QScriptValue(m_engine, 123)), true);
        QCOMPARE(ret.property("b").strictEqualTo(QScriptValue(m_engine, "ciao")), true);
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithListOfIntArg([1, 5])");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 17);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
        QVariant v = m_myObject->qtFunctionActuals().at(0);
        QCOMPARE(v.userType(), qMetaTypeId<QList<int> >());
        QList<int> ilst = qvariant_cast<QList<int> >(v);
        QCOMPARE(ilst.size(), 2);
        QCOMPARE(ilst.at(0), 1);
        QCOMPARE(ilst.at(1), 5);

        QCOMPARE(ret.isArray(), true);
        QCOMPARE(ret.property("0").strictEqualTo(QScriptValue(m_engine, 1)), true);
        QCOMPARE(ret.property("1").strictEqualTo(QScriptValue(m_engine, 5)), true);
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithQObjectStarArg(myObject)");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 18);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
        QVariant v = m_myObject->qtFunctionActuals().at(0);
        QCOMPARE(v.userType(), int(QMetaType::QObjectStar));
        QCOMPARE(qvariant_cast<QObject*>(v), (QObject *)m_myObject);

        QCOMPARE(ret.isQObject(), true);
        QCOMPARE(qscriptvalue_cast<QObject*>(ret), (QObject *)m_myObject);
    }

    m_myObject->resetQtFunctionInvoked();
    {
        // no implicit conversion from integer to QObject*
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithQObjectStarArg(123)");
        QCOMPARE(ret.isError(), true);
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue fun = m_engine->evaluate("myObject.myInvokableWithQBrushArg");
        Q_ASSERT(fun.isFunction());
        QColor color(10, 20, 30, 40);
        // QColor should be converted to a QBrush
        QScriptValue ret = fun.call(QScriptValue(), QScriptValueList()
                                    << qScriptValueFromValue(m_engine, color));
        QCOMPARE(m_myObject->qtFunctionInvoked(), 19);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
        QVariant v = m_myObject->qtFunctionActuals().at(0);
        QCOMPARE(v.userType(), int(QMetaType::QBrush));
        QCOMPARE(qvariant_cast<QColor>(v), color);

        QCOMPARE(qscriptvalue_cast<QColor>(ret), color);
    }
}

void tst_QScriptExtQObject::connectAndDisconnect()
{
    // connect(function)
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(123)").isError(), true);

    m_engine->evaluate("myHandler = function() { global.gotSignal = true; global.signalArgs = arguments; global.slotThisObject = this; global.signalSender = this.sender; }");
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(myHandler)").toBoolean(), true);

    m_engine->evaluate("gotSignal = false");
    m_engine->evaluate("myObject.mySignal()");
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 0.0);
    QCOMPARE(m_engine->evaluate("signalSender").toQObject(), (QObject *)0);
    QCOMPARE(m_engine->evaluate("slotThisObject").toQObject(), m_myObject);

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
    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(myHandler)").toBoolean(), false);

    QCOMPARE(m_engine->evaluate("myObject.mySignal2.connect(myHandler)").toBoolean(), true);

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal2(false);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("signalArgs[0]").toBoolean(), false);

    m_engine->evaluate("gotSignal = false");
    QCOMPARE(m_engine->evaluate("myObject.mySignal2.connect(myHandler)").toBoolean(), true);
    m_myObject->emitMySignal2(true);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("signalArgs[0]").toBoolean(), true);

    QCOMPARE(m_engine->evaluate("myObject.mySignal2.disconnect(myHandler)").toBoolean(), true);

    QCOMPARE(m_engine->evaluate("myObject.['mySignal2()'].connect(myHandler)").toBoolean(), true);

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal2();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);

    QCOMPARE(m_engine->evaluate("myObject.['mySignal2()'].disconnect(myHandler)").toBoolean(), true);

    // connect(object, function)
    m_engine->evaluate("otherObject = { name:'foo' }");
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(otherObject, myHandler)").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(otherObject, myHandler)").toBoolean(), true);
    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), false);

    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(otherObject, myHandler)").toBoolean(), false);

    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(otherObject, myHandler)").toBoolean(), true);
    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 0.0);
    QCOMPARE(m_engine->evaluate("signalSender").toQObject(), (QObject *)m_myObject);
    QCOMPARE(m_engine->evaluate("slotThisObject").property("name").toString(), QLatin1String("foo"));
    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(otherObject, myHandler)").toBoolean(), true);

    m_engine->evaluate("yetAnotherObject = { name:'bar', func : function() { } }");
    QCOMPARE(m_engine->evaluate("myObject.mySignal2.connect(yetAnotherObject, myHandler)").toBoolean(), true);
    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal2(true);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("signalSender").toQObject(), (QObject *)m_myObject);
    QCOMPARE(m_engine->evaluate("slotThisObject").property("name").toString(), QLatin1String("bar"));
    QCOMPARE(m_engine->evaluate("myObject.mySignal2.disconnect(yetAnotherObject, myHandler)").toBoolean(), true);

    // connect(obj, string)
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(yetAnotherObject, 'func')").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(myObject, 'mySlot')").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(yetAnotherObject, 'func')").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(myObject, 'mySlot')").toBoolean(), true);

    // check that emitting signals from script works

    // no arguments
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(myObject.mySlot)").toBoolean(), true);
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignal()").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 20);
    QCOMPARE(m_engine->evaluate("myObject.mySignal.disconnect(myObject.mySlot)").toBoolean(), true);

    // one argument
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject.mySlotWithIntArg)").toBoolean(), true);
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 21);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject.mySlotWithIntArg)").toBoolean(), true);

    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject.mySlotWithDoubleArg)").toBoolean(), true);
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 22);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QEXPECT_FAIL("", "missing value conversion", Continue);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toDouble(), 123.0);
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject.mySlotWithDoubleArg)").toBoolean(), true);

// ### this will crash!
#if 0
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject.mySlotWithStringArg)").toBoolean(), true);
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 23);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toString(), QLatin1String("123"));
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject.mySlotWithStringArg)").toBoolean(), true);
#endif
}

void tst_QScriptExtQObject::classEnums()
{
    QScriptValue myClass = m_engine->newQMetaObject(m_myObject->metaObject(), m_engine->undefinedValue());
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

Q_SCRIPT_DECLARE_QMETAOBJECT(MyQObject, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QObject, QObject*)

void tst_QScriptExtQObject::classConstructor()
{
    QScriptValue myClass = qScriptValueFromQMetaObject<MyQObject>(m_engine);
    m_engine->globalObject().setProperty("MyQObject", myClass);

    QScriptValue myObj = m_engine->evaluate("myObj = MyQObject()");
    QObject *qobj = myObj.toQObject();
    QVERIFY(qobj != 0);
    QCOMPARE(qobj->metaObject()->className(), "MyQObject");
    QCOMPARE(qobj->parent(), (QObject *)0);

    QScriptValue qobjectClass = qScriptValueFromQMetaObject<QObject>(m_engine);
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
        "myOtherObject", m_engine->newQObject(&other));
    m_engine->evaluate("myOtherObject.foo = myObject.foozball");
    other.resetQtFunctionInvoked();
    m_engine->evaluate("myOtherObject.foo(456)");
    QCOMPARE(other.qtFunctionInvoked(), 1);
}

void tst_QScriptExtQObject::findChild()
{
    QObject *child = new QObject(m_myObject);
    child->setObjectName(QLatin1String("myChildObject"));

    {
        QScriptValue result = m_engine->evaluate("myObject.findChild('noSuchChild')");
        QCOMPARE(result.isNull(), true);
    }

    {    
        QScriptValue result = m_engine->evaluate("myObject.findChild('myChildObject')");
        QCOMPARE(result.isQObject(), true);
        QCOMPARE(result.toQObject(), child);
    }

    delete child;
}

void tst_QScriptExtQObject::findChildren()
{
    QObject *child = new QObject(m_myObject);
    child->setObjectName(QLatin1String("myChildObject"));

    {
        QScriptValue result = m_engine->evaluate("myObject.findChildren('noSuchChild')");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property(QLatin1String("length")).toNumber(), 0.0);
    }

    {
        QScriptValue result = m_engine->evaluate("myObject.findChildren('myChildObject')");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property(QLatin1String("length")).toNumber(), 1.0);
        QCOMPARE(result.property(QLatin1String("0")).toQObject(), child);
    }

    QObject *namelessChild = new QObject(m_myObject);

    {
        QScriptValue result = m_engine->evaluate("myObject.findChildren('myChildObject')");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property(QLatin1String("length")).toNumber(), 1.0);
        QCOMPARE(result.property(QLatin1String("0")).toQObject(), child);
    }

    QObject *anotherChild = new QObject(m_myObject);
    anotherChild->setObjectName(QLatin1String("anotherChildObject"));

    {
        QScriptValue result = m_engine->evaluate("myObject.findChildren('anotherChildObject')");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property(QLatin1String("length")).toNumber(), 1.0);
        QCOMPARE(result.property(QLatin1String("0")).toQObject(), anotherChild);
    }

    anotherChild->setObjectName(QLatin1String("myChildObject"));
    {
        QScriptValue result = m_engine->evaluate("myObject.findChildren('myChildObject')");
        QCOMPARE(result.isArray(), true);
        QCOMPARE(result.property(QLatin1String("length")).toNumber(), 2.0);
        QObject *o1 = result.property(QLatin1String("0")).toQObject();
        QObject *o2 = result.property(QLatin1String("1")).toQObject();
        if (o1 != child) {
            QCOMPARE(o1, anotherChild);
            QCOMPARE(o2, child);
        } else {
            QCOMPARE(o1, child);
            QCOMPARE(o2, anotherChild);
        }
    }

    delete anotherChild;
    delete namelessChild;
    delete child;
}

QTEST_MAIN(tst_QScriptExtQObject)
#include "tst_qscriptqobject.moc"

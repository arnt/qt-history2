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
#include <qscriptvalueiterator.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>

//TESTED_CLASS=
//TESTED_FILES=qscriptextqobject.h qscriptextqobject.cpp

struct CustomType
{
    QString string;
};
Q_DECLARE_METATYPE(CustomType)

Q_DECLARE_METATYPE(QBrush*)
Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(Qt::BrushStyle)

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
    Q_INVOKABLE void setBrushProperty(const QBrush &value)
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
    Q_INVOKABLE qlonglong myInvokableReturningLongLong()
        { m_qtFunctionInvoked = 39; return 456; }
    Q_INVOKABLE QString myInvokableReturningString()
        { m_qtFunctionInvoked = 8; return QLatin1String("ciao"); }
    Q_INVOKABLE void myInvokableWithIntArg(int arg1, int arg2) // overload
        { m_qtFunctionInvoked = 9; m_actuals << arg1 << arg2; }
    Q_INVOKABLE void myInvokableWithEnumArg(Policy policy)
        { m_qtFunctionInvoked = 10; m_actuals << policy; }
    Q_INVOKABLE void myInvokableWithQualifiedEnumArg(MyQObject::Policy policy)
        { m_qtFunctionInvoked = 36; m_actuals << policy; }
    Q_INVOKABLE Policy myInvokableReturningEnum()
        { m_qtFunctionInvoked = 37; return BazPolicy; }
    Q_INVOKABLE MyQObject::Policy myInvokableReturningQualifiedEnum()
        { m_qtFunctionInvoked = 38; return BazPolicy; }
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
    Q_INVOKABLE void myInvokableWithBrushStyleArg(Qt::BrushStyle style)
        { m_qtFunctionInvoked = 43; m_actuals << qVariantFromValue(style); }
    Q_INVOKABLE void myInvokableWithVoidStarArg(void *arg)
        { m_qtFunctionInvoked = 44; m_actuals << qVariantFromValue(arg); }
    Q_INVOKABLE void myInvokableWithAmbiguousArg(int arg)
        { m_qtFunctionInvoked = 45; m_actuals << qVariantFromValue(arg); }
    Q_INVOKABLE void myInvokableWithAmbiguousArg(uint arg)
        { m_qtFunctionInvoked = 46; m_actuals << qVariantFromValue(arg); }
    Q_INVOKABLE void myInvokableWithDefaultArgs(int arg1, const QString &arg2 = "")
        { m_qtFunctionInvoked = 47; m_actuals << qVariantFromValue(arg1) << qVariantFromValue(arg2); }
    Q_INVOKABLE QObject& myInvokableReturningRef()
        { m_qtFunctionInvoked = 48; return *this; }
    Q_INVOKABLE const QObject& myInvokableReturningConstRef() const
        { const_cast<MyQObject*>(this)->m_qtFunctionInvoked = 49; return *this; }
    Q_INVOKABLE void myInvokableWithPointArg(const QPoint &arg)
        { const_cast<MyQObject*>(this)->m_qtFunctionInvoked = 50; m_actuals << qVariantFromValue(arg); }
    Q_INVOKABLE void myInvokableWithPointArg(const QPointF &arg)
        { const_cast<MyQObject*>(this)->m_qtFunctionInvoked = 51; m_actuals << qVariantFromValue(arg); }

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

    void myOverloadedSlot()
        { m_qtFunctionInvoked = 24; }
    void myOverloadedSlot(QObject *arg)
        { m_qtFunctionInvoked = 41; m_actuals << arg; }
    void myOverloadedSlot(bool arg)
        { m_qtFunctionInvoked = 25; m_actuals << arg; }
    void myOverloadedSlot(const QStringList &arg)
        { m_qtFunctionInvoked = 42; m_actuals << arg; }
    void myOverloadedSlot(double arg)
        { m_qtFunctionInvoked = 26; m_actuals << arg; }
    void myOverloadedSlot(float arg)
        { m_qtFunctionInvoked = 27; m_actuals << arg; }
    void myOverloadedSlot(int arg)
        { m_qtFunctionInvoked = 28; m_actuals << arg; }
    void myOverloadedSlot(const QString &arg)
        { m_qtFunctionInvoked = 29; m_actuals << arg; }
    void myOverloadedSlot(const QColor &arg)
        { m_qtFunctionInvoked = 30; m_actuals << arg; }
    void myOverloadedSlot(const QBrush &arg)
        { m_qtFunctionInvoked = 31; m_actuals << arg; }
    void myOverloadedSlot(const QDateTime &arg)
        { m_qtFunctionInvoked = 32; m_actuals << arg; }
    void myOverloadedSlot(const QDate &arg)
        { m_qtFunctionInvoked = 33; m_actuals << arg; }
    void myOverloadedSlot(const QRegExp &arg)
        { m_qtFunctionInvoked = 34; m_actuals << arg; }
    void myOverloadedSlot(const QVariant &arg)
        { m_qtFunctionInvoked = 35; m_actuals << arg; }

    void qscript_call()
        { m_qtFunctionInvoked = 40; }

protected Q_SLOTS:
    void myProtectedSlot() { m_qtFunctionInvoked = 36; }

private Q_SLOTS:
    void myPrivateSlot() { }

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

class MyEnumTestQObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString p1 READ p1)
    Q_PROPERTY(QString p2 READ p2)
    Q_PROPERTY(QString p3 READ p3 SCRIPTABLE false)
    Q_PROPERTY(QString p4 READ p4)
    Q_PROPERTY(QString p5 READ p5 SCRIPTABLE false)
    Q_PROPERTY(QString p6 READ p6)
public:
    MyEnumTestQObject(QObject *parent = 0)
        : QObject(parent) { }
    QString p1() const { return QLatin1String("p1"); }
    QString p2() const { return QLatin1String("p2"); }
    QString p3() const { return QLatin1String("p3"); }
    QString p4() const { return QLatin1String("p4"); }
    QString p5() const { return QLatin1String("p5"); }
    QString p6() const { return QLatin1String("p5"); }
public Q_SLOTS:
    void mySlot() { }
    void myOtherSlot() { }
Q_SIGNALS:
    void mySignal();
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

protected slots:
    void onSignalHandlerException(const QScriptValue &exception)
    {
        m_signalHandlerException = exception;
    }

private slots:
    void getSetStaticProperty();
    void getSetDynamicProperty();
    void getSetChildren();
    void callQtInvokable();
    void connectAndDisconnect();
    void cppConnectAndDisconnect();
    void classEnums();
    void classConstructor();
    void overrideInvokable();
    void transferInvokable();
    void findChild();
    void findChildren();
    void overloadedSlots();
    void enumerate_data();
    void enumerate();
    void wrapOptions();
    void prototypes();
    void objectDeleted();

private:
    QScriptEngine *m_engine;
    MyQObject *m_myObject;
    QScriptValue m_signalHandlerException;
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

static QScriptValue getSetProperty(QScriptContext *ctx, QScriptEngine *)
{
    if (ctx->argumentCount() != 0)
        ctx->callee().setProperty("value", ctx->argument(0));
    return ctx->callee().property("value");
}

void tst_QScriptExtQObject::getSetStaticProperty()
{
    QCOMPARE(m_engine->evaluate("myObject.noSuchProperty").isUndefined(), true);

    // initial value (set in MyQObject constructor)
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictlyEquals(QScriptValue(m_engine, 123.0)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantProperty")
             .toVariant(), QVariant(QLatin1String("foo")));
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("bar"))), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty").isArray(), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty.length")
             .strictlyEquals(QScriptValue(m_engine, 2)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[0]")
             .strictlyEquals(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[1]")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("foo"))), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty").isArray(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty.length")
             .strictlyEquals(QScriptValue(m_engine, 2)), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[0]").isString(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[0]").toString(),
             QLatin1String("zig"));
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[1]").isString(), true);
    QCOMPARE(m_engine->evaluate("myObject.stringListProperty[1]").toString(),
             QLatin1String("zag"));

    // default flags for "normal" properties
    {
        QScriptValue mobj = m_engine->globalObject().property("myObject");
        QVERIFY(!(mobj.propertyFlags("intProperty") & QScriptValue::ReadOnly));
        QVERIFY(mobj.propertyFlags("intProperty") & QScriptValue::Undeletable);
        QVERIFY(mobj.propertyFlags("intProperty") & QScriptValue::PropertyGetter);
        QVERIFY(mobj.propertyFlags("intProperty") & QScriptValue::PropertySetter);
        QVERIFY(!(mobj.propertyFlags("intProperty") & QScriptValue::SkipInEnumeration));
        QVERIFY(mobj.propertyFlags("intProperty") & QScriptValue::QObjectMember);

        QVERIFY(!(mobj.propertyFlags("mySlot") & QScriptValue::ReadOnly));
        QVERIFY(!(mobj.propertyFlags("mySlot") & QScriptValue::Undeletable));
        QVERIFY(!(mobj.propertyFlags("mySlot") & QScriptValue::SkipInEnumeration));
        QVERIFY(mobj.propertyFlags("mySlot") & QScriptValue::QObjectMember);
    }

    // property change in C++ should be reflected in script
    m_myObject->setIntProperty(456);
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictlyEquals(QScriptValue(m_engine, 456)), true);
    m_myObject->setIntProperty(789);
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictlyEquals(QScriptValue(m_engine, 789)), true);

    m_myObject->setVariantProperty(QLatin1String("bar"));
    QCOMPARE(m_engine->evaluate("myObject.variantProperty")
             .toVariant(), QVariant(QLatin1String("bar")));
    m_myObject->setVariantProperty(42);
    QCOMPARE(m_engine->evaluate("myObject.variantProperty")
             .toVariant(), QVariant(42));

    m_myObject->setStringProperty(QLatin1String("baz"));
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .equals(QScriptValue(m_engine, QLatin1String("baz"))), true);
    m_myObject->setStringProperty(QLatin1String("zab"));
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .equals(QScriptValue(m_engine, QLatin1String("zab"))), true);

    // property change in script should be reflected in C++
    QCOMPARE(m_engine->evaluate("myObject.intProperty = 123")
             .strictlyEquals(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_engine->evaluate("myObject.intProperty")
             .strictlyEquals(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_myObject->intProperty(), 123);
    QCOMPARE(m_engine->evaluate("myObject.intProperty = \"ciao!\";"
                                "myObject.intProperty")
             .strictlyEquals(QScriptValue(m_engine, 0)), true);
    QCOMPARE(m_myObject->intProperty(), 0);
    QCOMPARE(m_engine->evaluate("myObject.intProperty = \"123\";"
                                "myObject.intProperty")
             .strictlyEquals(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_myObject->intProperty(), 123);

    QCOMPARE(m_engine->evaluate("myObject.stringProperty = 'ciao'")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("ciao"))), true);
    QCOMPARE(m_engine->evaluate("myObject.stringProperty")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("ciao"))), true);
    QCOMPARE(m_myObject->stringProperty(), QLatin1String("ciao"));
    QCOMPARE(m_engine->evaluate("myObject.stringProperty = 123;"
                                "myObject.stringProperty")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("123"))), true);
    QCOMPARE(m_myObject->stringProperty(), QLatin1String("123"));

    QCOMPARE(m_engine->evaluate("myObject.variantProperty = \"foo\";"
                                "myObject.variantProperty.valueOf()").toString(), QLatin1String("foo"));
    QCOMPARE(m_myObject->variantProperty(), QVariant(QLatin1String("foo")));
    QCOMPARE(m_engine->evaluate("myObject.variantProperty = 42;"
                                "myObject.variantProperty").toNumber(), 42.0);
    QCOMPARE(m_myObject->variantProperty().toDouble(), 42.0);

    QCOMPARE(m_engine->evaluate("myObject.variantListProperty = [1, 'two', true];"
                                "myObject.variantListProperty.length")
             .strictlyEquals(QScriptValue(m_engine, 3)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[0]")
             .strictlyEquals(QScriptValue(m_engine, 1)), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[1]")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("two"))), true);
    QCOMPARE(m_engine->evaluate("myObject.variantListProperty[2]")
             .strictlyEquals(QScriptValue(m_engine, true)), true);
    {
        QVariantList vl = qscriptvalue_cast<QVariantList>(m_engine->evaluate("myObject.variantListProperty"));
        QCOMPARE(vl, QVariantList()
                 << QVariant(1)
                 << QVariant(QLatin1String("two"))
                 << QVariant(true));
    }

    QCOMPARE(m_engine->evaluate("myObject.stringListProperty = [1, 'two', true];"
                                "myObject.stringListProperty.length")
             .strictlyEquals(QScriptValue(m_engine, 3)), true);
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
        QCOMPARE(ret.strictlyEquals(val), true);
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
    {
        QScriptValue mobj = m_engine->globalObject().property("myObject");
        QCOMPARE(mobj.propertyFlags("readOnlyProperty") & QScriptValue::ReadOnly,
                 QScriptValue::ReadOnly);
    }

    // auto-dereferencing of pointers
    {
        QBrush b = QColor(0xCA, 0xFE, 0xBA, 0xBE);
        QBrush *bp = &b;
        QScriptValue bpValue = m_engine->newVariant(qVariantFromValue(bp));
        m_engine->globalObject().setProperty("brushPointer", bpValue);
        {
            QScriptValue ret = m_engine->evaluate("myObject.setBrushProperty(brushPointer)");
            QCOMPARE(ret.isUndefined(), true);
            QCOMPARE(qscriptvalue_cast<QBrush>(m_engine->evaluate("myObject.brushProperty")), b);
        }
        {
            b = QColor(0xDE, 0xAD, 0xBE, 0xEF);
            QScriptValue ret = m_engine->evaluate("myObject.brushProperty = brushPointer");
            QCOMPARE(ret.strictlyEquals(bpValue), true);
            QCOMPARE(qscriptvalue_cast<QBrush>(m_engine->evaluate("myObject.brushProperty")), b);
        }
        m_engine->globalObject().setProperty("brushPointer", QScriptValue());
    }

    // try to install custom property getter+setter
    {
        QScriptValue mobj = m_engine->globalObject().property("myObject");
        QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setProperty() failed: "
                             "cannot set getter or setter of native property "
                             "`intProperty'");
        mobj.setProperty("intProperty", m_engine->newFunction(getSetProperty),
                         QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    }
}

void tst_QScriptExtQObject::getSetDynamicProperty()
{
    // initially the object does not have the property
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"dynamicProperty\")")
             .strictlyEquals(QScriptValue(m_engine, false)), true);

    // add a dynamic property in C++
    QCOMPARE(m_myObject->setProperty("dynamicProperty", 123), false);
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"dynamicProperty\")")
             .strictlyEquals(QScriptValue(m_engine, true)), true);
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty")
             .strictlyEquals(QScriptValue(m_engine, 123)), true);

    // check the flags
    {
        QScriptValue mobj = m_engine->globalObject().property("myObject");
        QVERIFY(!(mobj.propertyFlags("dynamicProperty") & QScriptValue::ReadOnly));
        QVERIFY(!(mobj.propertyFlags("dynamicProperty") & QScriptValue::Undeletable));
        QVERIFY(!(mobj.propertyFlags("dynamicProperty") & QScriptValue::SkipInEnumeration));
        QVERIFY(mobj.propertyFlags("dynamicProperty") & QScriptValue::QObjectMember);
    }

    // property change in script should be reflected in C++
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty = \"foo\";"
                                "myObject.dynamicProperty")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("foo"))), true);
    QCOMPARE(m_myObject->property("dynamicProperty").toString(), QLatin1String("foo"));

    // delete the property
    QCOMPARE(m_engine->evaluate("delete myObject.dynamicProperty").toBoolean(), true);
    QCOMPARE(m_myObject->property("dynamicProperty").isValid(), false);
    QCOMPARE(m_engine->evaluate("myObject.dynamicProperty").isUndefined(), true);
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"dynamicProperty\")").toBoolean(), false);
}

void tst_QScriptExtQObject::getSetChildren()
{
    // initially the object does not have the child
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")")
             .strictlyEquals(QScriptValue(m_engine, false)), true);

    // add a child
    MyQObject *child = new MyQObject(m_myObject);
    child->setObjectName("child");
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")")
             .strictlyEquals(QScriptValue(m_engine, true)), true);

    // add a grandchild
    MyQObject *grandChild = new MyQObject(child);
    grandChild->setObjectName("grandChild");
    QCOMPARE(m_engine->evaluate("myObject.child.hasOwnProperty(\"grandChild\")")
             .strictlyEquals(QScriptValue(m_engine, true)), true);

    // delete grandchild
    delete grandChild;
    QCOMPARE(m_engine->evaluate("myObject.child.hasOwnProperty(\"grandChild\")")
             .strictlyEquals(QScriptValue(m_engine, false)), true);

    // delete child
    delete child;
    QCOMPARE(m_engine->evaluate("myObject.hasOwnProperty(\"child\")")
             .strictlyEquals(QScriptValue(m_engine, false)), true);

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

    // extra arguments should silently be ignored
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokable(10, 20, 30)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 0);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithIntArg('123')").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);

    m_myObject->resetQtFunctionInvoked();
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
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithStringArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 5);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toString(), QLatin1String("123"));

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithIntArgs(123, 456)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 6);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 2);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
    QCOMPARE(m_myObject->qtFunctionActuals().at(1).toInt(), 456);

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningInt()")
             .strictlyEquals(QScriptValue(m_engine, 123)), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 7);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningLongLong()")
             .strictlyEquals(QScriptValue(m_engine, 456)), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 39);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableReturningString()")
             .strictlyEquals(QScriptValue(m_engine, QLatin1String("ciao"))), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 8);
    QCOMPARE(m_myObject->qtFunctionActuals(), QVariantList());

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithIntArg(123, 456)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 9);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 2);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
    QCOMPARE(m_myObject->qtFunctionActuals().at(1).toInt(), 456);

    m_myObject->resetQtFunctionInvoked();
    QVERIFY(m_engine->evaluate("myObject.myInvokableWithVoidStarArg(null)").isUndefined());
    QCOMPARE(m_myObject->qtFunctionInvoked(), 44);
    m_myObject->resetQtFunctionInvoked();
    QVERIFY(m_engine->evaluate("myObject.myInvokableWithVoidStarArg(123)").isError());
    QCOMPARE(m_myObject->qtFunctionInvoked(), -1);

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithAmbiguousArg(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: ambiguous call of overloaded function myInvokableWithAmbiguousArg(); candidates were\n    myInvokableWithAmbiguousArg(int)\n    myInvokableWithAmbiguousArg(uint)"));
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithDefaultArgs(123, 'hello')");
        QVERIFY(ret.isUndefined());
        QCOMPARE(m_myObject->qtFunctionInvoked(), 47);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 2);
        QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
        QCOMPARE(m_myObject->qtFunctionActuals().at(1).toString(), QLatin1String("hello"));
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithDefaultArgs(456)");
        QVERIFY(ret.isUndefined());
        QCOMPARE(m_myObject->qtFunctionInvoked(), 47);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 2);
        QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 456);
        QCOMPARE(m_myObject->qtFunctionActuals().at(1).toString(), QString());
    }

    {
        QScriptValue fun = m_engine->evaluate("myObject.myInvokableWithPointArg");
        QVERIFY(fun.isFunction());
        m_myObject->resetQtFunctionInvoked();
        {
            QScriptValue ret = fun.call(m_engine->evaluate("myObject"),
                                        QScriptValueList() << m_engine->toScriptValue(QPoint(10, 20)));
            QVERIFY(ret.isUndefined());
            QCOMPARE(m_myObject->qtFunctionInvoked(), 50);
            QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
            QCOMPARE(m_myObject->qtFunctionActuals().at(0).toPoint(), QPoint(10, 20));
        }
        m_myObject->resetQtFunctionInvoked();
        {
            QScriptValue ret = fun.call(m_engine->evaluate("myObject"),
                                        QScriptValueList() << m_engine->toScriptValue(QPointF(30, 40)));
            QVERIFY(ret.isUndefined());
            QCOMPARE(m_myObject->qtFunctionInvoked(), 51);
            QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
            QCOMPARE(m_myObject->qtFunctionActuals().at(0).toPointF(), QPointF(30, 40));
        }
    }

    // calling function that returns (const)ref
    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableReturningRef()");
        QVERIFY(ret.isUndefined());
        QVERIFY(!m_engine->hasUncaughtException());
        QCOMPARE(m_myObject->qtFunctionInvoked(), 48);
    }
    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableReturningConstRef()");
        QVERIFY(ret.isUndefined());
        QVERIFY(!m_engine->hasUncaughtException());
        QCOMPARE(m_myObject->qtFunctionInvoked(), 49);
    }

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
        QCOMPARE(ret.toQObject(), (QObject *)m_myObject);
    }

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithQObjectListArg([myObject])");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 14);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
        QCOMPARE(ret.isArray(), true);
        QCOMPARE(ret.property(QLatin1String("length"))
                 .strictlyEquals(QScriptValue(m_engine, 1)), true);
        QCOMPARE(ret.property(QLatin1String("0")).isQObject(), true);
        QCOMPARE(ret.property(QLatin1String("0")).toQObject(), (QObject *)m_myObject);
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
        QCOMPARE(ret.property("a").strictlyEquals(QScriptValue(m_engine, 123)), true);
        QCOMPARE(ret.property("b").strictlyEquals(QScriptValue(m_engine, "ciao")), true);
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
        QCOMPARE(ret.property("0").strictlyEquals(QScriptValue(m_engine, 1)), true);
        QCOMPARE(ret.property("1").strictlyEquals(QScriptValue(m_engine, 5)), true);
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

    // private slots should not be part of the QObject binding
    QCOMPARE(m_engine->evaluate("myObject.myPrivateSlot").isUndefined(), true);

    // protected slots should be fine
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myProtectedSlot()");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 36);

    // call with too few arguments
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithIntArg()");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("SyntaxError: too few arguments in call to myInvokableWithIntArg(); candidates are\n    myInvokableWithIntArg(int,int)\n    myInvokableWithIntArg(int)"));
    }

    // call function where not all types have been registered
    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithBrushStyleArg(0)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: cannot call myInvokableWithBrushStyleArg(): unknown type `Qt::BrushStyle'"));
        QCOMPARE(m_myObject->qtFunctionInvoked(), -1);
    }

    // call function with incompatible argument type
    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableWithQBrushArg(null)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: incompatible type of argument(s) in call to myInvokableWithQBrushArg(); candidates were\n    myInvokableWithQBrushArg(QBrush)"));
        QCOMPARE(m_myObject->qtFunctionInvoked(), -1);
    }
}

void tst_QScriptExtQObject::connectAndDisconnect()
{
    // connect(function)
    QCOMPARE(m_engine->evaluate("myObject.mySignal.connect(123)").isError(), true);

    m_engine->evaluate("myHandler = function() { global.gotSignal = true; global.signalArgs = arguments; global.slotThisObject = this; global.signalSender = __qt_sender__; }");
    QVERIFY(m_engine->evaluate("myObject.mySignal.connect(myHandler)").isUndefined());

    m_engine->evaluate("gotSignal = false");
    m_engine->evaluate("myObject.mySignal()");
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 0.0);
    QCOMPARE(m_engine->evaluate("signalSender").toQObject(), (QObject *)m_myObject);
    QVERIFY(m_engine->evaluate("slotThisObject").strictlyEquals(m_engine->globalObject()));

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 0.0);

    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myHandler)").isUndefined());

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignalWithIntArg(123);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("signalArgs[0]").toNumber(), 123.0);

    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(myHandler)").isUndefined());
    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(myHandler)").isError());

    QVERIFY(m_engine->evaluate("myObject.mySignal2.connect(myHandler)").isUndefined());

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal2(false);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("signalArgs[0]").toBoolean(), false);

    m_engine->evaluate("gotSignal = false");
    QVERIFY(m_engine->evaluate("myObject.mySignal2.connect(myHandler)").isUndefined());
    m_myObject->emitMySignal2(true);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("signalArgs[0]").toBoolean(), true);

    QVERIFY(m_engine->evaluate("myObject.mySignal2.disconnect(myHandler)").isUndefined());

    QCOMPARE(m_engine->evaluate("myObject.['mySignal2()'].connect(myHandler)").toBoolean(), true);

    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal2();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);

    QCOMPARE(m_engine->evaluate("myObject.['mySignal2()'].disconnect(myHandler)").toBoolean(), true);

    // connect(object, function)
    m_engine->evaluate("otherObject = { name:'foo' }");
    QVERIFY(m_engine->evaluate("myObject.mySignal.connect(otherObject, myHandler)").isUndefined());
    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(otherObject, myHandler)").isUndefined());
    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), false);

    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(otherObject, myHandler)").isError());

    QVERIFY(m_engine->evaluate("myObject.mySignal.connect(otherObject, myHandler)").isUndefined());
    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal();
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 0.0);
    QVERIFY(m_engine->evaluate("slotThisObject").strictlyEquals(m_engine->evaluate("otherObject")));
    QVERIFY(m_engine->evaluate("signalSender").strictlyEquals(m_engine->evaluate("myObject")));
    QCOMPARE(m_engine->evaluate("slotThisObject").property("name").toString(), QLatin1String("foo"));
    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(otherObject, myHandler)").isUndefined());

    m_engine->evaluate("yetAnotherObject = { name:'bar', func : function() { } }");
    QVERIFY(m_engine->evaluate("myObject.mySignal2.connect(yetAnotherObject, myHandler)").isUndefined());
    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal2(true);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QVERIFY(m_engine->evaluate("slotThisObject").strictlyEquals(m_engine->evaluate("yetAnotherObject")));
    QVERIFY(m_engine->evaluate("signalSender").strictlyEquals(m_engine->evaluate("myObject")));
    QCOMPARE(m_engine->evaluate("slotThisObject").property("name").toString(), QLatin1String("bar"));
    QVERIFY(m_engine->evaluate("myObject.mySignal2.disconnect(yetAnotherObject, myHandler)").isUndefined());

    QVERIFY(m_engine->evaluate("myObject.mySignal2.connect(myObject, myHandler)").isUndefined());
    m_engine->evaluate("gotSignal = false");
    m_myObject->emitMySignal2(true);
    QCOMPARE(m_engine->evaluate("gotSignal").toBoolean(), true);
    QCOMPARE(m_engine->evaluate("signalArgs.length").toNumber(), 1.0);
    QCOMPARE(m_engine->evaluate("slotThisObject").toQObject(), (QObject *)m_myObject);
    QVERIFY(m_engine->evaluate("signalSender").strictlyEquals(m_engine->evaluate("myObject")));
    QVERIFY(m_engine->evaluate("myObject.mySignal2.disconnect(myObject, myHandler)").isUndefined());

    // connect(obj, string)
    QVERIFY(m_engine->evaluate("myObject.mySignal.connect(yetAnotherObject, 'func')").isUndefined());
    QVERIFY(m_engine->evaluate("myObject.mySignal.connect(myObject, 'mySlot')").isUndefined());
    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(yetAnotherObject, 'func')").isUndefined());
    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(myObject, 'mySlot')").isUndefined());

    // check that emitting signals from script works

    // no arguments
    QVERIFY(m_engine->evaluate("myObject.mySignal.connect(myObject.mySlot)").isUndefined());
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignal()").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 20);
    QVERIFY(m_engine->evaluate("myObject.mySignal.disconnect(myObject.mySlot)").isUndefined());

    // one argument
    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject.mySlotWithIntArg)").isUndefined());
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 21);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject.mySlotWithIntArg)").isUndefined());

    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject.mySlotWithDoubleArg)").isUndefined());
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 22);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toDouble(), 123.0);
    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject.mySlotWithDoubleArg)").isUndefined());

    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject.mySlotWithStringArg)").isUndefined());
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 23);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toString(), QLatin1String("123"));
    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject.mySlotWithStringArg)").isUndefined());

    // connecting to overloaded slot
    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject.myOverloadedSlot)").isUndefined());
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(123)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 26); // double overload
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 123);
    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject.myOverloadedSlot)").isUndefined());

    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.connect(myObject['myOverloadedSlot(int)'])").isUndefined());
    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.mySignalWithIntArg(456)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 28); // int overload
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), 456);
    QVERIFY(m_engine->evaluate("myObject.mySignalWithIntArg.disconnect(myObject['myOverloadedSlot(int)'])").isUndefined());

    // erroneous input
    {
        QScriptValue ret = m_engine->evaluate("(function() { }).connect()");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: Function.prototype.connect: no arguments given"));
    }
    {
        QScriptValue ret = m_engine->evaluate("var o = { }; o.connect = Function.prototype.connect;  o.connect()");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: Function.prototype.connect: no arguments given"));
    }

    {
        QScriptValue ret = m_engine->evaluate("(function() { }).connect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.connect: this object is not a signal"));
    }
    {
        QScriptValue ret = m_engine->evaluate("var o = { }; o.connect = Function.prototype.connect;  o.connect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.connect: this object is not a signal"));
    }

    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokable.connect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.connect: MyQObject::myInvokable() is not a signal"));
    }
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokable.connect(function() { })");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.connect: MyQObject::myInvokable() is not a signal"));
    }

    {
        QScriptValue ret = m_engine->evaluate("myObject.mySignal.connect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.connect: target is not a function"));
    }

    {
        QScriptValue ret = m_engine->evaluate("(function() { }).disconnect()");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: Function.prototype.disconnect: no arguments given"));
    }
    {
        QScriptValue ret = m_engine->evaluate("var o = { }; o.disconnect = Function.prototype.disconnect;  o.disconnect()");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: Function.prototype.disconnect: no arguments given"));
    }

    {
        QScriptValue ret = m_engine->evaluate("(function() { }).disconnect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.disconnect: this object is not a signal"));
    }
    {
        QScriptValue ret = m_engine->evaluate("var o = { }; o.disconnect = Function.prototype.disconnect;  o.disconnect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.disconnect: this object is not a signal"));
    }

    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokable.disconnect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.disconnect: MyQObject::myInvokable() is not a signal"));
    }
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokable.disconnect(function() { })");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.disconnect: MyQObject::myInvokable() is not a signal"));
    }

    {
        QScriptValue ret = m_engine->evaluate("myObject.mySignal.disconnect(123)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("TypeError: Function.prototype.disconnect: target is not a function"));
    }

    {
        QScriptValue ret = m_engine->evaluate("myObject.mySignal.disconnect(function() { })");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: Function.prototype.disconnect: failed to disconnect from MyQObject::mySignal()"));
    }

    // when the wrapper dies, the connection stays alive
    QVERIFY(m_engine->evaluate("myObject.mySignal.connect(myObject.mySlot)").isUndefined());
    m_myObject->resetQtFunctionInvoked();
    m_myObject->emitMySignal();
    QCOMPARE(m_myObject->qtFunctionInvoked(), 20);
    m_engine->evaluate("myObject = null");
    m_engine->collectGarbage();
    m_myObject->resetQtFunctionInvoked();
    m_myObject->emitMySignal();
    QCOMPARE(m_myObject->qtFunctionInvoked(), 20);
}

void tst_QScriptExtQObject::cppConnectAndDisconnect()
{
    QScriptEngine eng;
    QLineEdit edit;
    QLineEdit edit2;
    QScriptValue fun = eng.evaluate("function fun(text) { signalObject = this; signalArg = text; }; return fun");
    for (int z = 0; z < 2; ++z) {
        QScriptValue receiver;
        if (z == 0)
            receiver = QScriptValue();
        else
            receiver = eng.newObject();
        for (int y = 0; y < 2; ++y) {
            QVERIFY(qScriptConnect(&edit, SIGNAL(textChanged(const QString &)), receiver, fun));
            QVERIFY(qScriptConnect(&edit2, SIGNAL(textChanged(const QString &)), receiver, fun));
            // check signal emission
            for (int x = 0; x < 4; ++x) {
                QLineEdit *ed = (x < 2) ? &edit : &edit2;
                ed->setText((x % 2) ? "foo" : "bar");
                {
                    QScriptValue ret = eng.globalObject().property("signalObject");
                    if (receiver.isObject())
                        QVERIFY(ret.strictlyEquals(receiver));
                    else
                        QVERIFY(ret.strictlyEquals(eng.globalObject()));
                }
                {
                    QScriptValue ret = eng.globalObject().property("signalArg");
                    QVERIFY(ret.isString());
                    QCOMPARE(ret.toString(), ed->text());
                }
                eng.collectGarbage();
            }

            // check disconnect
            QVERIFY(qScriptDisconnect(&edit, SIGNAL(textChanged(const QString &)), receiver, fun));
            eng.globalObject().setProperty("signalObject", QScriptValue());
            eng.globalObject().setProperty("signalArg", QScriptValue());
            edit.setText("something else");
            QVERIFY(!eng.globalObject().property("signalObject").isValid());
            QVERIFY(!eng.globalObject().property("signalArg").isValid());
            QVERIFY(!qScriptDisconnect(&edit, SIGNAL(textChanged(const QString &)), receiver, fun));

            // other object's connection should remain
            edit2.setText(edit.text());
            {
                QScriptValue ret = eng.globalObject().property("signalObject");
                if (receiver.isObject())
                    QVERIFY(ret.strictlyEquals(receiver));
                else
                    QVERIFY(ret.strictlyEquals(eng.globalObject()));
            }
            {
                QScriptValue ret = eng.globalObject().property("signalArg");
                QVERIFY(ret.isString());
                QCOMPARE(ret.toString(), edit2.text());
            }

            // disconnect other object too
            QVERIFY(qScriptDisconnect(&edit2, SIGNAL(textChanged(const QString &)), receiver, fun));
            eng.globalObject().setProperty("signalObject", QScriptValue());
            eng.globalObject().setProperty("signalArg", QScriptValue());
            edit2.setText("even more different");
            QVERIFY(!eng.globalObject().property("signalObject").isValid());
            QVERIFY(!eng.globalObject().property("signalArg").isValid());
            QVERIFY(!qScriptDisconnect(&edit2, SIGNAL(textChanged(const QString &)), receiver, fun));
        }
    }

    // make sure we don't crash when engine is deleted
    {
        QScriptEngine *eng2 = new QScriptEngine;
        QScriptValue fun2 = eng2->evaluate("function(text) { signalObject = this; signalArg = text; }");
        QVERIFY(qScriptConnect(&edit, SIGNAL(textChanged(const QString &)), QScriptValue(), fun2));
        delete eng2;
        edit.setText("ciao");
        QVERIFY(!qScriptDisconnect(&edit, SIGNAL(textChanged(const QString &)), QScriptValue(), fun2));
    }

    // mixing script-side and C++-side connect
    {
        eng.globalObject().setProperty("edit", eng.newQObject(&edit));
        QVERIFY(eng.evaluate("edit.textChanged.connect(fun)").isUndefined());
        QVERIFY(qScriptDisconnect(&edit, SIGNAL(textChanged(const QString &)), QScriptValue(), fun));

        QVERIFY(qScriptConnect(&edit, SIGNAL(textChanged(const QString &)), QScriptValue(), fun));
        QVERIFY(eng.evaluate("edit.textChanged.disconnect(fun)").isUndefined());
    }

    // signalHandlerException()
    {
        connect(&eng, SIGNAL(signalHandlerException(QScriptValue)),
                this, SLOT(onSignalHandlerException(QScriptValue)));

        eng.globalObject().setProperty("edit", eng.newQObject(&edit));
        QScriptValue fun = eng.evaluate("function() { nonExistingFunction(); }");
        QVERIFY(qScriptConnect(&edit, SIGNAL(textChanged(const QString &)), QScriptValue(), fun));

        m_signalHandlerException = QScriptValue();
        QScriptValue ret = eng.evaluate("edit.text = 'trigger a signal handler exception from script'");
        QVERIFY(ret.isError());
        QVERIFY(m_signalHandlerException.strictlyEquals(ret));

        m_signalHandlerException = QScriptValue();
        edit.setText("trigger a signal handler exception from C++");
        QVERIFY(m_signalHandlerException.isError());

        QVERIFY(qScriptDisconnect(&edit, SIGNAL(textChanged(const QString &)), QScriptValue(), fun));

        m_signalHandlerException = QScriptValue();
        eng.evaluate("edit.text = 'no more exception from script'");
        QVERIFY(!m_signalHandlerException.isValid());
        edit.setText("no more exception from C++");
        QVERIFY(!m_signalHandlerException.isValid());

        disconnect(&eng, SIGNAL(signalHandlerException(QScriptValue)),
                   this, SLOT(onSignalHandlerException(QScriptValue)));
    }
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

    QCOMPARE(MyQObject::Ability(m_engine->evaluate("MyQObject.NoAbility").toInt32()),
             MyQObject::NoAbility);
    QCOMPARE(MyQObject::Ability(m_engine->evaluate("MyQObject.FooAbility").toInt32()),
             MyQObject::FooAbility);
    QCOMPARE(MyQObject::Ability(m_engine->evaluate("MyQObject.BarAbility").toInt32()),
             MyQObject::BarAbility);
    QCOMPARE(MyQObject::Ability(m_engine->evaluate("MyQObject.BazAbility").toInt32()),
             MyQObject::BazAbility);
    QCOMPARE(MyQObject::Ability(m_engine->evaluate("MyQObject.AllAbility").toInt32()),
             MyQObject::AllAbility);

    // enums from Qt are inherited through prototype
    QCOMPARE(static_cast<Qt::FocusPolicy>(m_engine->evaluate("MyQObject.StrongFocus").toInt32()),
             Qt::StrongFocus);
    QCOMPARE(static_cast<Qt::Key>(m_engine->evaluate("MyQObject.Key_Left").toInt32()),
             Qt::Key_Left);

    QCOMPARE(m_engine->evaluate("MyQObject.className()").toString(), QLatin1String("MyQObject"));

    qRegisterMetaType<MyQObject::Policy>("Policy");

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithEnumArg(MyQObject.BazPolicy)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 10);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), int(MyQObject::BazPolicy));

    m_myObject->resetQtFunctionInvoked();
    QCOMPARE(m_engine->evaluate("myObject.myInvokableWithQualifiedEnumArg(MyQObject.BazPolicy)").isUndefined(), true);
    QCOMPARE(m_myObject->qtFunctionInvoked(), 36);
    QCOMPARE(m_myObject->qtFunctionActuals().size(), 1);
    QCOMPARE(m_myObject->qtFunctionActuals().at(0).toInt(), int(MyQObject::BazPolicy));

    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableReturningEnum()");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 37);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 0);
        QCOMPARE(ret.isVariant(), true);
    }
    m_myObject->resetQtFunctionInvoked();
    {
        QScriptValue ret = m_engine->evaluate("myObject.myInvokableReturningQualifiedEnum()");
        QCOMPARE(m_myObject->qtFunctionInvoked(), 38);
        QCOMPARE(m_myObject->qtFunctionActuals().size(), 0);
        QCOMPARE(ret.isNumber(), true);
    }
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

    delete qobj;
}

void tst_QScriptExtQObject::overrideInvokable()
{
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myInvokable()");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 0);

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

    m_engine->evaluate("delete myObject.myInvokable");
    m_myObject->resetQtFunctionInvoked();
    // this form (with the '()') is read-only
    m_engine->evaluate("myObject['myInvokable()'] = function() { global.a = 123; }");
    m_engine->evaluate("myObject.myInvokable()");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 0);
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

    // find all
    {
        QScriptValue result = m_engine->evaluate("myObject.findChildren()");
        QVERIFY(result.isArray());
        int count = 3;
        QCOMPARE(result.property("length").toInt32(), count);
        for (int i = 0; i < 3; ++i) {
            QObject *o = result.property(i).toQObject();
            if (o == namelessChild || o == child || o == anotherChild)
                --count;
        }
        QVERIFY(count == 0);
    }

    delete anotherChild;
    delete namelessChild;
    delete child;
}

void tst_QScriptExtQObject::overloadedSlots()
{
    // should pick myOverloadedSlot(double)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(10)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 26);

    // should pick myOverloadedSlot(double)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(10.0)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 26);

    // should pick myOverloadedSlot(QString)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot('10')");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 29);

    // should pick myOverloadedSlot(bool)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(true)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 25);

    // should pick myOverloadedSlot(QDateTime)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(new Date())");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 32);

    // should pick myOverloadedSlot(QRegExp)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(new RegExp())");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 34);

    // should pick myOverloadedSlot(QVariant)
    m_myObject->resetQtFunctionInvoked();
    QScriptValue f = m_engine->evaluate("myObject.myOverloadedSlot");
    f.call(QScriptValue(), QScriptValueList() << m_engine->newVariant(QVariant("ciao")));
    QCOMPARE(m_myObject->qtFunctionInvoked(), 35);

    // should pick myOverloadedSlot(QObject*)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(myObject)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 41);

    // should pick myOverloadedSlot(QObject*)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(null)");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 41);

    // should pick myOverloadedSlot(QStringList)
    m_myObject->resetQtFunctionInvoked();
    m_engine->evaluate("myObject.myOverloadedSlot(['hello'])");
    QCOMPARE(m_myObject->qtFunctionInvoked(), 42);
}

void tst_QScriptExtQObject::enumerate_data()
{
    QTest::addColumn<int>("wrapOptions");
    QTest::addColumn<QStringList>("expectedNames");

    QTest::newRow( "enumerate all" )
        << 0
        << (QStringList()
            // meta-object-defined properties:
            //   inherited
            << "objectName"
            //   non-inherited
            << "p1" << "p2" << "p4" << "p6"
            // dynamic properties
            << "dp1" << "dp2" << "dp3"
            // inherited slots
            << "destroyed(QObject*)" << "destroyed()"
            << "deleteLater()"
            // not included because it's private:
            // << "_q_reregisterTimers(void*)"
            // signals
            << "mySignal()"
            // slots
            << "mySlot()" << "myOtherSlot()");

    QTest::newRow( "don't enumerate inherited properties" )
        << int(QScriptEngine::ExcludeSuperClassProperties)
        << (QStringList()
            // meta-object-defined properties:
            //   non-inherited
            << "p1" << "p2" << "p4" << "p6"
            // dynamic properties
            << "dp1" << "dp2" << "dp3"
            // inherited slots
            << "destroyed(QObject*)" << "destroyed()"
            << "deleteLater()"
            // not included because it's private:
            // << "_q_reregisterTimers(void*)"
            // signals
            << "mySignal()"
            // slots
            << "mySlot()" << "myOtherSlot()");

    QTest::newRow( "don't enumerate inherited methods" )
        << int(QScriptEngine::ExcludeSuperClassMethods)
        << (QStringList()
            // meta-object-defined properties:
            //   inherited
            << "objectName"
            //   non-inherited
            << "p1" << "p2" << "p4" << "p6"
            // dynamic properties
            << "dp1" << "dp2" << "dp3"
            // signals
            << "mySignal()"
            // slots
            << "mySlot()" << "myOtherSlot()");

    QTest::newRow( "don't enumerate inherited members" )
        << int(QScriptEngine::ExcludeSuperClassMethods
               | QScriptEngine::ExcludeSuperClassProperties)
        << (QStringList()
            // meta-object-defined properties
            << "p1" << "p2" << "p4" << "p6"
            // dynamic properties
            << "dp1" << "dp2" << "dp3"
            // signals
            << "mySignal()"
            // slots
            << "mySlot()" << "myOtherSlot()");
}

void tst_QScriptExtQObject::enumerate()
{
    QFETCH( int, wrapOptions );
    QFETCH( QStringList, expectedNames );

    QScriptEngine eng;
    MyEnumTestQObject enumQObject;
    // give it some dynamic properties
    enumQObject.setProperty("dp1", "dp1");
    enumQObject.setProperty("dp2", "dp2");
    enumQObject.setProperty("dp3", "dp3");
    QScriptValue obj = eng.newQObject(&enumQObject, QScriptEngine::QtOwnership,
                                      QScriptEngine::QObjectWrapOptions(wrapOptions));

    // enumerate in script
    {
        eng.globalObject().setProperty("myEnumObject", obj);
        eng.evaluate("var enumeratedProperties = []");
        eng.evaluate("for (var p in myEnumObject) { enumeratedProperties.push(p); }");
        QStringList result = qscriptvalue_cast<QStringList>(eng.evaluate("enumeratedProperties"));
        QCOMPARE(result.size(), expectedNames.size());
        for (int i = 0; i < expectedNames.size(); ++i)
            QCOMPARE(result.at(i), expectedNames.at(i));
    }
    // enumerate in C++
    {
        QScriptValueIterator it(obj);
        QStringList result;
        while (it.hasNext()) {
            it.next();
            result.append(it.name());
        }
        QCOMPARE(result.size(), expectedNames.size());
        for (int i = 0; i < expectedNames.size(); ++i)
            QCOMPARE(result.at(i), expectedNames.at(i));
    }
}

void tst_QScriptExtQObject::wrapOptions()
{
    QCOMPARE(m_myObject->setProperty("dynamicProperty", 123), false);
    MyQObject *child = new MyQObject(m_myObject);
    child->setObjectName("child");
    // exclude child objects
    {
        QScriptValue obj = m_engine->newQObject(m_myObject, QScriptEngine::QtOwnership,
                                                QScriptEngine::ExcludeChildObjects);
        QCOMPARE(obj.property("child").isValid(), false);
        obj.setProperty("child", QScriptValue(m_engine, 123));
        QCOMPARE(obj.property("child")
                 .strictlyEquals(QScriptValue(m_engine, 123)), true);
    }
    // don't auto-create dynamic properties
    {
        QScriptValue obj = m_engine->newQObject(m_myObject);
        QVERIFY(!m_myObject->dynamicPropertyNames().contains("anotherDynamicProperty"));
        obj.setProperty("anotherDynamicProperty", QScriptValue(m_engine, 123));
        QVERIFY(!m_myObject->dynamicPropertyNames().contains("anotherDynamicProperty"));
        QCOMPARE(obj.property("anotherDynamicProperty")
                 .strictlyEquals(QScriptValue(m_engine, 123)), true);
    }
    // auto-create dynamic properties
    {
        QScriptValue obj = m_engine->newQObject(m_myObject, QScriptEngine::QtOwnership,
                                                QScriptEngine::AutoCreateDynamicProperties);
        QVERIFY(!m_myObject->dynamicPropertyNames().contains("anotherDynamicProperty"));
        obj.setProperty("anotherDynamicProperty", QScriptValue(m_engine, 123));
        QVERIFY(m_myObject->dynamicPropertyNames().contains("anotherDynamicProperty"));
        QCOMPARE(obj.property("anotherDynamicProperty")
                 .strictlyEquals(QScriptValue(m_engine, 123)), true);
    }
    // don't exclude super-class properties
    {
        QScriptValue obj = m_engine->newQObject(m_myObject);
        QVERIFY(obj.property("objectName").isValid());
        QVERIFY(obj.propertyFlags("objectName") & QScriptValue::QObjectMember);
    }
    // exclude super-class properties
    {
        QScriptValue obj = m_engine->newQObject(m_myObject, QScriptEngine::QtOwnership,
                                                QScriptEngine::ExcludeSuperClassProperties);
        QVERIFY(!obj.property("objectName").isValid());
        QVERIFY(!(obj.propertyFlags("objectName") & QScriptValue::QObjectMember));
        QVERIFY(obj.property("intProperty").isValid());
        QVERIFY(obj.propertyFlags("intProperty") & QScriptValue::QObjectMember);
    }
    // don't exclude super-class methods
    {
        QScriptValue obj = m_engine->newQObject(m_myObject);
        QVERIFY(obj.property("deleteLater").isValid());
        QVERIFY(obj.propertyFlags("deleteLater") & QScriptValue::QObjectMember);
    }
    // exclude super-class methods
    {
        QScriptValue obj = m_engine->newQObject(m_myObject, QScriptEngine::QtOwnership,
                                                QScriptEngine::ExcludeSuperClassMethods);
        QVERIFY(!obj.property("deleteLater").isValid());
        QVERIFY(!(obj.propertyFlags("deleteLater") & QScriptValue::QObjectMember));
        QVERIFY(obj.property("mySlot").isValid());
        QVERIFY(obj.propertyFlags("mySlot") & QScriptValue::QObjectMember);
    }
    // exclude all that we can
    {
        QScriptValue obj = m_engine->newQObject(m_myObject, QScriptEngine::QtOwnership,
                                                QScriptEngine::ExcludeSuperClassMethods
                                                | QScriptEngine::ExcludeSuperClassProperties
                                                | QScriptEngine::ExcludeChildObjects);
        QVERIFY(!obj.property("deleteLater").isValid());
        QVERIFY(!(obj.propertyFlags("deleteLater") & QScriptValue::QObjectMember));
        QVERIFY(obj.property("mySlot").isValid());
        QVERIFY(obj.propertyFlags("mySlot") & QScriptValue::QObjectMember);

        QVERIFY(!obj.property("objectName").isValid());
        QVERIFY(!(obj.propertyFlags("objectName") & QScriptValue::QObjectMember));
        QVERIFY(obj.property("intProperty").isValid());
        QVERIFY(obj.propertyFlags("intProperty") & QScriptValue::QObjectMember);

        QCOMPARE(obj.property("child").isValid(), false);
        obj.setProperty("child", QScriptValue(m_engine, 123));
        QCOMPARE(obj.property("child")
                 .strictlyEquals(QScriptValue(m_engine, 123)), true);
    }

    delete child;
}

Q_DECLARE_METATYPE(QWidget*)
Q_DECLARE_METATYPE(QPushButton*)

void tst_QScriptExtQObject::prototypes()
{
    QScriptEngine eng;
    QScriptValue widgetProto = eng.newQObject(new QWidget(), QScriptEngine::ScriptOwnership);
    eng.setDefaultPrototype(qMetaTypeId<QWidget*>(), widgetProto);
    QPushButton *pbp = new QPushButton();
    QScriptValue buttonProto = eng.newQObject(pbp, QScriptEngine::ScriptOwnership);
    buttonProto.setPrototype(widgetProto);
    eng.setDefaultPrototype(qMetaTypeId<QPushButton*>(), buttonProto);
    QPushButton *pb = new QPushButton();
    QScriptValue button = eng.newQObject(pb, QScriptEngine::ScriptOwnership);
    QVERIFY(button.prototype().strictlyEquals(buttonProto));

    buttonProto.setProperty("text", QScriptValue(&eng, "prototype button"));
    QCOMPARE(pbp->text(), QLatin1String("prototype button"));
    button.setProperty("text", QScriptValue(&eng, "not the prototype button"));
    QCOMPARE(pb->text(), QLatin1String("not the prototype button"));
    QCOMPARE(pbp->text(), QLatin1String("prototype button"));

    buttonProto.setProperty("objectName", QScriptValue(&eng, "prototype button"));
    QCOMPARE(pbp->objectName(), QLatin1String("prototype button"));
    button.setProperty("objectName", QScriptValue(&eng, "not the prototype button"));
    QCOMPARE(pb->objectName(), QLatin1String("not the prototype button"));
    QCOMPARE(pbp->objectName(), QLatin1String("prototype button"));
}

void tst_QScriptExtQObject::objectDeleted()
{
    QScriptEngine eng;
    MyQObject *qobj = new MyQObject();
    QScriptValue v = eng.newQObject(qobj);
    v.setProperty("objectName", QScriptValue(&eng, "foo"));
    QCOMPARE(qobj->objectName(), QLatin1String("foo"));
    v.setProperty("intProperty", QScriptValue(&eng, 123));
    QCOMPARE(qobj->intProperty(), 123);
    qobj->resetQtFunctionInvoked();
    v.property("myInvokable").call(v);
    QCOMPARE(qobj->qtFunctionInvoked(), 0);

    // now delete the object
    delete qobj;

    // the documented behavior is: isQObject() should still return true,
    // but toQObject() should return 0
    QVERIFY(v.isQObject());
    QCOMPARE(v.toQObject(), (QObject *)0);

    // any attempt to access properties of the object should result in an exception
    {
        QScriptValue ret = v.property("objectName");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot access member `objectName' of deleted QObject"));
    }
    {
        eng.evaluate("Object");
        QVERIFY(!eng.hasUncaughtException());
        v.setProperty("objectName", QScriptValue(&eng, "foo"));
        QVERIFY(eng.hasUncaughtException());
        QVERIFY(eng.uncaughtException().isError());
        QCOMPARE(eng.uncaughtException().toString(), QLatin1String("Error: cannot access member `objectName' of deleted QObject"));
    }

    {
        QScriptValue ret = v.call();
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot call function of deleted QObject"));
    }

    // myInvokable is stored in member table (since we've accessed it before deletion)
    QVERIFY(v.property("myInvokable").isFunction());
    {
        QScriptValue ret = v.property("myInvokable").call(v);
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot call function of deleted QObject"));
    }
    // myInvokableWithIntArg is not stored in member table (since we've not accessed it)
    {
        QScriptValue ret = v.property("myInvokableWithIntArg");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot access member `myInvokableWithIntArg' of deleted QObject"));
    }

    // access from script
    eng.globalObject().setProperty("o", v);
    {
        QScriptValue ret = eng.evaluate("o()");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot call function of deleted QObject"));
    }
    {
        QScriptValue ret = eng.evaluate("o.objectName");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot access member `objectName' of deleted QObject"));
    }
    {
        QScriptValue ret = eng.evaluate("o.myInvokable()");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot call function of deleted QObject"));
    }
    {
        QScriptValue ret = eng.evaluate("o.myInvokableWithIntArg(10)");
        QVERIFY(ret.isError());
        QCOMPARE(ret.toString(), QLatin1String("Error: cannot access member `myInvokableWithIntArg' of deleted QObject"));
    }
}

QTEST_MAIN(tst_QScriptExtQObject)
#include "tst_qscriptqobject.moc"

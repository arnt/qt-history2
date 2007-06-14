/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QPushButton>
#include <QtCore/qnumeric.h>

#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptengine.h>

//TESTED_CLASS=
//TESTED_FILES=qscriptvalue.h qscriptvalue.cpp

class tst_QScriptValue : public QObject
{
    Q_OBJECT

public:
    tst_QScriptValue();
    virtual ~tst_QScriptValue();

private slots:
    void ctor();
    void engine();
    void toString();
    void toNumber();
    void toBoolean();
    void toInteger();
    void toInt32();
    void toUInt32();
    void toUInt16();
    void toVariant();
    void toQObject();
    void toObject();
    void instanceOf();
    void getSetPrototype();
    void getSetScope();
    void getSetProperty();
    void call();
    void construct();
    void lessThan();
    void equals();
    void strictlyEquals();
    void castToPointer();
};

tst_QScriptValue::tst_QScriptValue()
{
}

tst_QScriptValue::~tst_QScriptValue()
{
}

void tst_QScriptValue::ctor()
{
    QScriptEngine eng;
    {
        QScriptValue v;
        QCOMPARE(v.isValid(), false);
        QCOMPARE(v.engine(), (QScriptEngine *)0);
    }
    {
        QScriptValue v(&eng, QScriptValue::UndefinedValue);
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isUndefined(), true);
        QCOMPARE(v.isObject(), false);
    }
    {
        QScriptValue v(&eng, QScriptValue::NullValue);
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isNull(), true);
        QCOMPARE(v.isObject(), false);
    }
    {
        QScriptValue v(&eng, false);
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isBoolean(), true);
        QCOMPARE(v.isObject(), false);
        QCOMPARE(v.toBoolean(), false);
    }
    {
        QScriptValue v(&eng, int(1));
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isNumber(), true);
        QCOMPARE(v.isObject(), false);
        QCOMPARE(v.toNumber(), 1.0);
    }
    {
        QScriptValue v(&eng, uint(1));
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isNumber(), true);
        QCOMPARE(v.isObject(), false);
        QCOMPARE(v.toNumber(), 1.0);
    }
    {
        QScriptValue v(&eng, 1.0);
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isNumber(), true);
        QCOMPARE(v.isObject(), false);
        QCOMPARE(v.toNumber(), 1.0);
    }
    {
        QScriptValue v(&eng, "ciao");
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isString(), true);
        QCOMPARE(v.isObject(), false);
        QCOMPARE(v.toString(), QLatin1String("ciao"));
    }
    {
        QScriptValue v(&eng, QString("ciao"));
        QCOMPARE(v.isValid(), true);
        QCOMPARE(v.isString(), true);
        QCOMPARE(v.isObject(), false);
        QCOMPARE(v.toString(), QLatin1String("ciao"));
    }
    // copy constructor, operator=
    {
        QScriptValue v(&eng, 1.0);
        QScriptValue v2(v);
        QCOMPARE(v2.strictlyEquals(v), true);

        QScriptValue v3(v);
        QCOMPARE(v3.strictlyEquals(v), true);
        QCOMPARE(v3.strictlyEquals(v2), true);

        QScriptValue v4(&eng, 2.0);
        QCOMPARE(v4.strictlyEquals(v), false);
        v3 = v4;
        QCOMPARE(v3.strictlyEquals(v), false);
        QCOMPARE(v3.strictlyEquals(v4), true);

        v2 = QScriptValue();
        QCOMPARE(v2.strictlyEquals(v), false);
        QCOMPARE(v.toNumber(), 1.0);

        QScriptValue v5(v);
        QCOMPARE(v5.strictlyEquals(v), true);
        v = QScriptValue();
        QCOMPARE(v5.strictlyEquals(v), false);
        QCOMPARE(v5.toNumber(), 1.0);
    }
}

void tst_QScriptValue::engine()
{
    QScriptEngine eng;
    QScriptValue object = eng.newObject();
    QCOMPARE(object.engine(), &eng);
}

static QScriptValue myFunction(QScriptContext *, QScriptEngine *eng)
{
    return eng->undefinedValue();
}

void tst_QScriptValue::toString()
{
    QScriptEngine eng;

    QScriptValue undefined = eng.undefinedValue();
    QCOMPARE(undefined.toString(), QString("undefined"));
    QCOMPARE(qscriptvalue_cast<QString>(undefined), QString("undefined"));

    QScriptValue null = eng.nullValue();
    QCOMPARE(null.toString(), QString("null"));
    QCOMPARE(qscriptvalue_cast<QString>(null), QString("null"));

    QScriptValue falskt = QScriptValue(&eng, false);
    QCOMPARE(falskt.toString(), QString("false"));
    QCOMPARE(qscriptvalue_cast<QString>(falskt), QString("false"));

    QScriptValue sant = QScriptValue(&eng, true);
    QCOMPARE(sant.toString(), QString("true"));
    QCOMPARE(qscriptvalue_cast<QString>(sant), QString("true"));

    QScriptValue number = QScriptValue(&eng, 123);
    QCOMPARE(number.toString(), QString("123"));
    QCOMPARE(qscriptvalue_cast<QString>(number), QString("123"));

    QScriptValue str = QScriptValue(&eng, QString("ciao"));
    QCOMPARE(str.toString(), QString("ciao"));
    QCOMPARE(qscriptvalue_cast<QString>(str), QString("ciao"));

    QScriptValue object = eng.newObject();
    QCOMPARE(object.toString(), QString("[object Object]"));
    QCOMPARE(qscriptvalue_cast<QString>(object), QString("[object Object]"));

    QScriptValue fun = eng.newFunction(myFunction);
    QCOMPARE(fun.toString(), QString("function () { [native] }"));
    QCOMPARE(qscriptvalue_cast<QString>(fun), QString("function () { [native] }"));

    // toString() that throws exception
    {
        QScriptValue objectObject = eng.evaluate(
            "(function(){"
            "  o = { };"
            "  o.toString = function() { throw new Error('toString'); };"
            "  return o;"
            "})()");
        QCOMPARE(objectObject.toString(), QLatin1String("Object"));
        QVERIFY(eng.hasUncaughtException());
        QCOMPARE(eng.uncaughtException().toString(), QLatin1String("Error: toString"));
    }

    QScriptValue inv = QScriptValue();
    QCOMPARE(inv.toString(), QString());
}

void tst_QScriptValue::toNumber()
{
    QScriptEngine eng;

    QScriptValue undefined = eng.undefinedValue();
    QCOMPARE(qIsNaN(undefined.toNumber()), true);
    QCOMPARE(qIsNaN(qscriptvalue_cast<qsreal>(undefined)), true);

    QScriptValue null = eng.nullValue();
    QCOMPARE(null.toNumber(), 0.0);
    QCOMPARE(qscriptvalue_cast<qsreal>(null), 0.0);

    QScriptValue falskt = QScriptValue(&eng, false);
    QCOMPARE(falskt.toNumber(), 0.0);
    QCOMPARE(qscriptvalue_cast<qsreal>(falskt), 0.0);

    QScriptValue sant = QScriptValue(&eng, true);
    QCOMPARE(sant.toNumber(), 1.0);
    QCOMPARE(qscriptvalue_cast<qsreal>(sant), 1.0);

    QScriptValue number = QScriptValue(&eng, 123.0);
    QCOMPARE(number.toNumber(), 123.0);
    QCOMPARE(qscriptvalue_cast<qsreal>(number), 123.0);

    QScriptValue str = QScriptValue(&eng, QString("ciao"));
    QCOMPARE(qIsNaN(str.toNumber()), true);
    QCOMPARE(qIsNaN(qscriptvalue_cast<qsreal>(str)), true);

    QScriptValue str2 = QScriptValue(&eng, QString("123"));
    QCOMPARE(str2.toNumber(), 123.0);
    QCOMPARE(qscriptvalue_cast<qsreal>(str2), 123.0);

    QScriptValue object = eng.newObject();
    QCOMPARE(qIsNaN(object.toNumber()), true);
    QCOMPARE(qIsNaN(qscriptvalue_cast<qsreal>(object)), true);

    QScriptValue fun = eng.newFunction(myFunction);
    QCOMPARE(qIsNaN(fun.toNumber()), true);
    QCOMPARE(qIsNaN(qscriptvalue_cast<qsreal>(fun)), true);

    QScriptValue inv = QScriptValue();
    QCOMPARE(inv.toNumber(), 0.0);
    QCOMPARE(qscriptvalue_cast<qsreal>(inv), 0.0);
}

void tst_QScriptValue::toBoolean()
{
    QScriptEngine eng;

    QScriptValue undefined = eng.undefinedValue();
    QCOMPARE(undefined.toBoolean(), false);
    QCOMPARE(qscriptvalue_cast<bool>(undefined), false);

    QScriptValue null = eng.nullValue();
    QCOMPARE(null.toBoolean(), false);
    QCOMPARE(qscriptvalue_cast<bool>(null), false);

    QScriptValue falskt = QScriptValue(&eng, false);
    QCOMPARE(falskt.toBoolean(), false);
    QCOMPARE(qscriptvalue_cast<bool>(falskt), false);

    QScriptValue sant = QScriptValue(&eng, true);
    QCOMPARE(sant.toBoolean(), true);
    QCOMPARE(qscriptvalue_cast<bool>(sant), true);

    QScriptValue number = QScriptValue(&eng, 0.0);
    QCOMPARE(number.toBoolean(), false);
    QCOMPARE(qscriptvalue_cast<bool>(number), false);

    QScriptValue number2 = QScriptValue(&eng, qSNaN());
    QCOMPARE(number2.toBoolean(), false);
    QCOMPARE(qscriptvalue_cast<bool>(number2), false);

    QScriptValue number3 = QScriptValue(&eng, 123.0);
    QCOMPARE(number3.toBoolean(), true);
    QCOMPARE(qscriptvalue_cast<bool>(number3), true);

    QScriptValue number4 = QScriptValue(&eng, -456.0);
    QCOMPARE(number4.toBoolean(), true);
    QCOMPARE(qscriptvalue_cast<bool>(number4), true);

    QScriptValue str = QScriptValue(&eng, QString(""));
    QCOMPARE(str.toBoolean(), false);
    QCOMPARE(qscriptvalue_cast<bool>(str), false);

    QScriptValue str2 = QScriptValue(&eng, QString("123"));
    QCOMPARE(str2.toBoolean(), true);
    QCOMPARE(qscriptvalue_cast<bool>(str2), true);

    QScriptValue object = eng.newObject();
    QCOMPARE(object.toBoolean(), true);
    QCOMPARE(qscriptvalue_cast<bool>(object), true);

    QScriptValue fun = eng.newFunction(myFunction);
    QCOMPARE(fun.toBoolean(), true);
    QCOMPARE(qscriptvalue_cast<bool>(fun), true);

    QScriptValue inv = QScriptValue();
    QCOMPARE(inv.toBoolean(), false);
    QCOMPARE(qscriptvalue_cast<bool>(inv), false);
}

void tst_QScriptValue::toInteger()
{
    QScriptEngine eng;

    QScriptValue number = QScriptValue(&eng, 123.0);
    QCOMPARE(number.toInteger(), 123.0);

    QScriptValue number2 = QScriptValue(&eng, qSNaN());
    QCOMPARE(number2.toInteger(), 0.0);

    QScriptValue number3 = QScriptValue(&eng, qInf());
    QCOMPARE(qIsInf(number3.toInteger()), true);

    QScriptValue number4 = QScriptValue(&eng, 0.5);
    QCOMPARE(number4.toInteger(), 0.0);

    QScriptValue number5 = QScriptValue(&eng, 123.5);
    QCOMPARE(number5.toInteger(), 123.0);

    QScriptValue number6 = QScriptValue(&eng, -456.5);
    QCOMPARE(number6.toInteger(), -456.0);

    QScriptValue inv;
    QCOMPARE(inv.toInteger(), 0.0);
}

void tst_QScriptValue::toInt32()
{
    QScriptEngine eng;

    QScriptValue zer0 = QScriptValue(&eng, 0.0);
    QCOMPARE(zer0.toInt32(), 0);
    QCOMPARE(qscriptvalue_cast<qint32>(zer0), 0);

    QScriptValue number = QScriptValue(&eng, 123.0);
    QCOMPARE(number.toInt32(), 123);
    QCOMPARE(qscriptvalue_cast<qint32>(number), 123);

    QScriptValue number2 = QScriptValue(&eng, qSNaN());
    QCOMPARE(number2.toInt32(), 0);
    QCOMPARE(qscriptvalue_cast<qint32>(number2), 0);

    QScriptValue number3 = QScriptValue(&eng, +qInf());
    QCOMPARE(number3.toInt32(), 0);
    QCOMPARE(qscriptvalue_cast<qint32>(number3), 0);

    QScriptValue number3_2 = QScriptValue(&eng, -qInf());
    QCOMPARE(number3_2.toInt32(), 0);
    QCOMPARE(qscriptvalue_cast<qint32>(number3_2), 0);

    QScriptValue number4 = QScriptValue(&eng, 0.5);
    QCOMPARE(number4.toInt32(), 0);
    QCOMPARE(qscriptvalue_cast<qint32>(number4), 0);

    QScriptValue number5 = QScriptValue(&eng, 123.5);
    QCOMPARE(number5.toInt32(), 123);
    QCOMPARE(qscriptvalue_cast<qint32>(number5), 123);

    QScriptValue number6 = QScriptValue(&eng, -456.5);
    QCOMPARE(number6.toInt32(), -456);
    QCOMPARE(qscriptvalue_cast<qint32>(number6), -456);

    QScriptValue inv;
    QCOMPARE(inv.toInt32(), 0);
    QCOMPARE(qscriptvalue_cast<qint32>(inv), 0);
}

void tst_QScriptValue::toUInt32()
{
    QScriptEngine eng;

    QScriptValue zer0 = QScriptValue(&eng, 0.0);
    QCOMPARE(zer0.toUInt32(), quint32(0));
    QCOMPARE(qscriptvalue_cast<quint32>(zer0), quint32(0));

    QScriptValue number = QScriptValue(&eng, 123.0);
    QCOMPARE(number.toUInt32(), quint32(123));
    QCOMPARE(qscriptvalue_cast<quint32>(number), quint32(123));

    QScriptValue number2 = QScriptValue(&eng, qSNaN());
    QCOMPARE(number2.toUInt32(), quint32(0));
    QCOMPARE(qscriptvalue_cast<quint32>(number2), quint32(0));

    QScriptValue number3 = QScriptValue(&eng, +qInf());
    QCOMPARE(number3.toUInt32(), quint32(0));
    QCOMPARE(qscriptvalue_cast<quint32>(number3), quint32(0));

    QScriptValue number3_2 = QScriptValue(&eng, -qInf());
    QCOMPARE(number3_2.toUInt32(), quint32(0));
    QCOMPARE(qscriptvalue_cast<quint32>(number3_2), quint32(0));

    QScriptValue number4 = QScriptValue(&eng, 0.5);
    QCOMPARE(number4.toUInt32(), quint32(0));

    QScriptValue number5 = QScriptValue(&eng, 123.5);
    QCOMPARE(number5.toUInt32(), quint32(123));

    QScriptValue number6 = QScriptValue(&eng, -456.5);
    QCOMPARE(number6.toUInt32(), quint32(-456));
    QCOMPARE(qscriptvalue_cast<quint32>(number6), quint32(-456));

    QScriptValue inv;
    QCOMPARE(inv.toUInt32(), quint32(0));
    QCOMPARE(qscriptvalue_cast<quint32>(inv), quint32(0));
}

void tst_QScriptValue::toUInt16()
{
    QScriptEngine eng;

    QScriptValue zer0 = QScriptValue(&eng, 0.0);
    QCOMPARE(zer0.toUInt16(), quint16(0));
    QCOMPARE(qscriptvalue_cast<quint16>(zer0), quint16(0));

    QScriptValue number = QScriptValue(&eng, 123.0);
    QCOMPARE(number.toUInt16(), quint16(123));
    QCOMPARE(qscriptvalue_cast<quint16>(number), quint16(123));

    QScriptValue number2 = QScriptValue(&eng, qSNaN());
    QCOMPARE(number2.toUInt16(), quint16(0));
    QCOMPARE(qscriptvalue_cast<quint16>(number2), quint16(0));

    QScriptValue number3 = QScriptValue(&eng, +qInf());
    QCOMPARE(number3.toUInt16(), quint16(0));
    QCOMPARE(qscriptvalue_cast<quint16>(number3), quint16(0));

    QScriptValue number3_2 = QScriptValue(&eng, -qInf());
    QCOMPARE(number3_2.toUInt16(), quint16(0));
    QCOMPARE(qscriptvalue_cast<quint16>(number3_2), quint16(0));

    QScriptValue number4 = QScriptValue(&eng, 0.5);
    QCOMPARE(number4.toUInt16(), quint16(0));

    QScriptValue number5 = QScriptValue(&eng, 123.5);
    QCOMPARE(number5.toUInt16(), quint16(123));

    QScriptValue number6 = QScriptValue(&eng, -456.5);
    QCOMPARE(number6.toUInt16(), quint16(-456));
    QCOMPARE(qscriptvalue_cast<quint16>(number6), quint16(-456));

    QScriptValue number7 = QScriptValue(&eng, 0x10000);
    QCOMPARE(number7.toUInt16(), quint16(0));
    QCOMPARE(qscriptvalue_cast<quint16>(number7), quint16(0));

    QScriptValue number8 = QScriptValue(&eng, 0x10001);
    QCOMPARE(number8.toUInt16(), quint16(1));
    QCOMPARE(qscriptvalue_cast<quint16>(number8), quint16(1));

    QScriptValue inv;
    QCOMPARE(inv.toUInt16(), quint16(0));
    QCOMPARE(qscriptvalue_cast<quint16>(inv), quint16(0));
}

Q_DECLARE_METATYPE(QVariant)

void tst_QScriptValue::toVariant()
{
    QScriptEngine eng;

    QScriptValue undefined = eng.undefinedValue();
    QCOMPARE(undefined.toVariant(), QVariant());
    QCOMPARE(qscriptvalue_cast<QVariant>(undefined), QVariant());

    QScriptValue null = eng.nullValue();
    QCOMPARE(null.toVariant(), QVariant());
    QCOMPARE(qscriptvalue_cast<QVariant>(null), QVariant());

    QScriptValue number = QScriptValue(&eng, 123.0);
    QCOMPARE(number.toVariant(), QVariant(123.0));
    QCOMPARE(qscriptvalue_cast<QVariant>(number), QVariant(123.0));

    QScriptValue falskt = QScriptValue(&eng, false);
    QCOMPARE(falskt.toVariant(), QVariant(false));
    QCOMPARE(qscriptvalue_cast<QVariant>(falskt), QVariant(false));

    QScriptValue sant = QScriptValue(&eng, true);
    QCOMPARE(sant.toVariant(), QVariant(true));
    QCOMPARE(qscriptvalue_cast<QVariant>(sant), QVariant(true));

    QScriptValue str = QScriptValue(&eng, QString("ciao"));
    QCOMPARE(str.toVariant(), QVariant(QString("ciao")));
    QCOMPARE(qscriptvalue_cast<QVariant>(str), QVariant(QString("ciao")));

    QVariant var(QChar(0x007A));
    QScriptValue opaque = eng.newVariant(var);
    QCOMPARE(opaque.toVariant(), var);

    QScriptValue object = eng.newObject();
    QCOMPARE(object.toVariant(), QVariant(QString("[object Object]")));

    QScriptValue qobject = eng.newQObject(this);
    {
        QVariant var = qobject.toVariant();
        QCOMPARE(var.userType(), int(QMetaType::QObjectStar));
        QCOMPARE(qVariantValue<QObject*>(var), (QObject *)this);
    }

    {
        QDateTime dateTime = QDateTime(QDate(1980, 10, 4));
        QScriptValue dateObject = eng.newDate(dateTime);
        QVariant var = dateObject.toVariant();
        QCOMPARE(var, QVariant(dateTime));
    }

    {
        QRegExp rx = QRegExp("[0-9a-z]+");
        QScriptValue rxObject = eng.newRegExp(rx);
        QVariant var = rxObject.toVariant();
        QCOMPARE(var, QVariant(rx));
    }

    QScriptValue inv;
    QCOMPARE(inv.toVariant(), QVariant());
    QCOMPARE(qscriptvalue_cast<QVariant>(inv), QVariant());
}

// unfortunately, this is necessary in order to do qscriptvalue_cast<QPushButton*>(...)
Q_DECLARE_METATYPE(QPushButton*)

void tst_QScriptValue::toQObject()
{
    QScriptEngine eng;

    QScriptValue undefined = eng.undefinedValue();
    QCOMPARE(undefined.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(undefined), (QObject *)0);

    QScriptValue null = eng.nullValue();
    QCOMPARE(null.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(null), (QObject *)0);

    QScriptValue falskt = QScriptValue(&eng, false);
    QCOMPARE(falskt.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(falskt), (QObject *)0);

    QScriptValue sant = QScriptValue(&eng, true);
    QCOMPARE(sant.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(sant), (QObject *)0);

    QScriptValue number = QScriptValue(&eng, 123.0);
    QCOMPARE(number.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(number), (QObject *)0);

    QScriptValue str = QScriptValue(&eng, QString("ciao"));
    QCOMPARE(str.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(str), (QObject *)0);

    QScriptValue object = eng.newObject();
    QCOMPARE(object.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(object), (QObject *)0);

    QScriptValue qobject = eng.newQObject(this);
    QCOMPARE(qobject.toQObject(), (QObject *)this);
    QCOMPARE(qscriptvalue_cast<QObject*>(qobject), (QObject *)this);

    QScriptValue qobject2 = eng.newQObject(0);
    QCOMPARE(qobject2.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(qobject2), (QObject *)0);

    QWidget widget;
    QScriptValue qwidget = eng.newQObject(&widget);
    QCOMPARE(qwidget.toQObject(), (QObject *)&widget);
    QCOMPARE(qscriptvalue_cast<QObject*>(qwidget), (QObject *)&widget);
    QCOMPARE(qscriptvalue_cast<QWidget*>(qwidget), &widget);

    QPushButton button;
    QScriptValue qbutton = eng.newQObject(&button);
    QCOMPARE(qbutton.toQObject(), (QObject *)&button);
    QCOMPARE(qscriptvalue_cast<QObject*>(qbutton), (QObject *)&button);
    QCOMPARE(qscriptvalue_cast<QWidget*>(qbutton), (QWidget *)&button);
    QCOMPARE(qscriptvalue_cast<QPushButton*>(qbutton), &button);

    QScriptValue inv;
    QCOMPARE(inv.toQObject(), (QObject *)0);
    QCOMPARE(qscriptvalue_cast<QObject*>(inv), (QObject *)0);
}

void tst_QScriptValue::toObject()
{
    QScriptEngine eng;

    QScriptValue undefined = eng.undefinedValue();
    QCOMPARE(undefined.toObject().isValid(), false);

    QScriptValue null = eng.nullValue();
    QCOMPARE(null.toObject().isValid(), false);

    QScriptValue falskt = QScriptValue(&eng, false);
    {
        QScriptValue tmp = falskt.toObject();
        QCOMPARE(tmp.isObject(), true);
        QCOMPARE(tmp.toNumber(), falskt.toNumber());
    }

    QScriptValue sant = QScriptValue(&eng, true);
    {
        QScriptValue tmp = sant.toObject();
        QCOMPARE(tmp.isObject(), true);
        QCOMPARE(tmp.toNumber(), sant.toNumber());
    }

    QScriptValue number = QScriptValue(&eng, 123.0);
    {
        QScriptValue tmp = number.toObject();
        QCOMPARE(tmp.isObject(), true);
        QCOMPARE(tmp.toNumber(), number.toNumber());
    }

    QScriptValue str = QScriptValue(&eng, QString("ciao"));
    {
        QScriptValue tmp = str.toObject();
        QCOMPARE(tmp.isObject(), true);
        QCOMPARE(tmp.toString(), str.toString());
    }

    QScriptValue object = eng.newObject();
    {
        QScriptValue tmp = object.toObject();
        QCOMPARE(tmp.isObject(), true);
    }

    QScriptValue qobject = eng.newQObject(this);
    QCOMPARE(qobject.toObject().isValid(), true);

    QScriptValue inv;
    QCOMPARE(inv.toObject().isValid(), false);
}

void tst_QScriptValue::instanceOf()
{
    QScriptEngine eng;
    QScriptValue obj = eng.newObject();
    QCOMPARE(obj.instanceOf(eng.evaluate("Object.prototype")), true);
    QCOMPARE(obj.instanceOf(eng.evaluate("Array.prototype")), false);
    QCOMPARE(obj.instanceOf(eng.evaluate("Function.prototype")), false);
    QCOMPARE(obj.instanceOf(eng.evaluate("QObject.prototype")), false);
    QCOMPARE(obj.instanceOf(QScriptValue(&eng, 123)), false);
    QCOMPARE(obj.instanceOf(eng.undefinedValue()), false);
    QCOMPARE(obj.instanceOf(eng.nullValue()), false);
    QCOMPARE(obj.instanceOf(QScriptValue()), false);

    QCOMPARE(obj.instanceOf(eng.evaluate("Object")), true);
    QCOMPARE(obj.instanceOf(eng.evaluate("Array")), false);
    QCOMPARE(obj.instanceOf(eng.evaluate("Function")), false);
    QCOMPARE(obj.instanceOf(eng.evaluate("QObject")), false);

    QScriptValue arr = eng.newArray();
    QCOMPARE(arr.instanceOf(eng.evaluate("Object.prototype")), true);
    QCOMPARE(arr.instanceOf(eng.evaluate("Array.prototype")), true);
    QCOMPARE(arr.instanceOf(eng.evaluate("Function.prototype")), false);
    QCOMPARE(arr.instanceOf(eng.evaluate("QObject.prototype")), false);
    QCOMPARE(arr.instanceOf(eng.evaluate("Object")), true);
    QCOMPARE(arr.instanceOf(eng.evaluate("Array")), true);
    QCOMPARE(arr.instanceOf(eng.evaluate("Function")), false);
    QCOMPARE(arr.instanceOf(eng.evaluate("QObject")), false);

    QCOMPARE(QScriptValue().instanceOf(arr), false);

    QScriptEngine otherEngine;
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::instanceof: cannot perform operation on a value created in a different engine");
    QCOMPARE(obj.instanceOf(otherEngine.globalObject().property("Object")), false);
}

static QScriptValue getter(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->thisObject().property("x");
}

static QScriptValue setter(QScriptContext *ctx, QScriptEngine *)
{
    ctx->thisObject().setProperty("x", ctx->argument(0));
    return ctx->argument(0);
}

static QScriptValue getterSetter(QScriptContext *ctx, QScriptEngine *)
{
    if (ctx->argumentCount() > 0)
        ctx->thisObject().setProperty("x", ctx->argument(0));
    return ctx->thisObject().property("x");
}

static QScriptValue getterSetterThrowingError(QScriptContext *ctx, QScriptEngine *)
{
    if (ctx->argumentCount() > 0)
        return ctx->throwError("set foo");
    else
        return ctx->throwError("get foo");
}

static QScriptValue getSet__proto__(QScriptContext *ctx, QScriptEngine *)
{
    if (ctx->argumentCount() > 0)
        ctx->callee().setProperty("value", ctx->argument(0));
    return ctx->callee().property("value");
}

void tst_QScriptValue::getSetProperty()
{
    QScriptEngine eng;

    QScriptValue object = eng.newObject();

    QScriptValue str = QScriptValue(&eng, "bar");
    object.setProperty("foo", str);
    QCOMPARE(object.property("foo").toString(), str.toString());

    QScriptValue num = QScriptValue(&eng, 123.0);
    object.setProperty("baz", num);
    QCOMPARE(object.property("baz").toNumber(), num.toNumber());

    QScriptValue inv;
    inv.setProperty("foo", num);
    QCOMPARE(inv.property("foo").isValid(), false);

    QScriptValue array = eng.newArray();
    array.setProperty(0, num);
    QCOMPARE(array.property(0).toNumber(), num.toNumber());
    QCOMPARE(array.property("length").toUInt32(), quint32(1));
    array.setProperty(1, str);
    QCOMPARE(array.property(1).toString(), str.toString());
    QCOMPARE(array.property("length").toUInt32(), quint32(2));
    array.setProperty("length", QScriptValue(&eng, 1));
    QCOMPARE(array.property("length").toUInt32(), quint32(1));
    QCOMPARE(array.property(1).isValid(), false);

    // task 162051 -- detecting whether the property is an array index or not
    QVERIFY(eng.evaluate("a = []; a['00'] = 123; a['00']").strictlyEquals(QScriptValue(&eng, 123)));
    QVERIFY(eng.evaluate("a.length").strictlyEquals(QScriptValue(&eng, 0)));
    QVERIFY(eng.evaluate("a.hasOwnProperty('00')").strictlyEquals(QScriptValue(&eng, true)));
    QEXPECT_FAIL("", "hasOwnProperty() is buggy for Array objects", Continue);
    QVERIFY(eng.evaluate("a.hasOwnProperty('0')").strictlyEquals(QScriptValue(&eng, false)));
    QVERIFY(eng.evaluate("a[0]").isUndefined());
    QVERIFY(eng.evaluate("a[0.5] = 456; a[0.5]").strictlyEquals(QScriptValue(&eng, 456)));
    QVERIFY(eng.evaluate("a.length").strictlyEquals(QScriptValue(&eng, 0)));
    QVERIFY(eng.evaluate("a.hasOwnProperty('0.5')").strictlyEquals(QScriptValue(&eng, true)));
    QVERIFY(eng.evaluate("a[0]").isUndefined());
    QVERIFY(eng.evaluate("a[0] = 789; a[0]").strictlyEquals(QScriptValue(&eng, 789)));
    QVERIFY(eng.evaluate("a.length").strictlyEquals(QScriptValue(&eng, 1)));

    QScriptEngine otherEngine;
    QScriptValue otherNum = QScriptValue(&otherEngine, 123);
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setProperty(oof) failed: cannot set value created in a different engine");
    object.setProperty("oof", otherNum);
    QCOMPARE(object.property("oof").isValid(), false);

    // test ResolveMode
    QScriptValue object2 = eng.newObject();
    object.setPrototype(object2);
    QScriptValue num2 = QScriptValue(&eng, 456.0);
    object2.setProperty("propertyInPrototype", num2);
    // default is ResolvePrototype
    QCOMPARE(object.property("propertyInPrototype")
             .strictlyEquals(num2), true);
    QCOMPARE(object.property("propertyInPrototype", QScriptValue::ResolvePrototype)
             .strictlyEquals(num2), true);
    QCOMPARE(object.property("propertyInPrototype", QScriptValue::ResolveLocal)
             .isValid(), false);
    QCOMPARE(object.property("propertyInPrototype", QScriptValue::ResolveScope)
             .strictlyEquals(num2), false);
    QCOMPARE(object.property("propertyInPrototype", QScriptValue::ResolveFull)
             .strictlyEquals(num2), true);

    // test property removal (setProperty(QScriptValue()))
    QScriptValue object3 = eng.newObject();
    object3.setProperty("foo", num);
    QCOMPARE(object3.property("foo").strictlyEquals(num), true);
    object3.setProperty("bar", str);
    QCOMPARE(object3.property("bar").strictlyEquals(str), true);
    object3.setProperty("foo", QScriptValue());
    QCOMPARE(object3.property("foo").isValid(), false);
    QCOMPARE(object3.property("bar").strictlyEquals(str), true);
    object3.setProperty("foo", num);
    QCOMPARE(object3.property("foo").strictlyEquals(num), true);
    QCOMPARE(object3.property("bar").strictlyEquals(str), true);
    object3.setProperty("bar", QScriptValue());
    QCOMPARE(object3.property("bar").isValid(), false);
    QCOMPARE(object3.property("foo").strictlyEquals(num), true);
    object3.setProperty("foo", QScriptValue());
    object3.setProperty("foo", QScriptValue());

    eng.globalObject().setProperty("object3", object3);
    QCOMPARE(eng.evaluate("object3.hasOwnProperty('foo')")
             .strictlyEquals(QScriptValue(&eng, false)), true);
    object3.setProperty("foo", num);
    QCOMPARE(eng.evaluate("object3.hasOwnProperty('foo')")
             .strictlyEquals(QScriptValue(&eng, true)), true);
    eng.globalObject().setProperty("object3", QScriptValue());
    QCOMPARE(eng.evaluate("this.hasOwnProperty('object3')")
             .strictlyEquals(QScriptValue(&eng, false)), true);

    // getters and setters
    {
        QScriptValue object4 = eng.newObject();
        for (int x = 0; x < 2; ++x) {
            object4.setProperty("foo", QScriptValue());
            // getter() returns this.x
            object4.setProperty("foo", eng.newFunction(getter),
                                QScriptValue::PropertyGetter | QScriptValue::UserRange);
            QCOMPARE(object4.propertyFlags("foo"),
                     QScriptValue::PropertyGetter | QScriptValue::UserRange);
            object4.setProperty("x", num);
            QCOMPARE(object4.property("foo").strictlyEquals(num), true);
            
            // setter() sets this.x
            object4.setProperty("foo", eng.newFunction(setter),
                                QScriptValue::PropertySetter | QScriptValue::UserRange);
            QCOMPARE(object4.propertyFlags("foo"),
                     QScriptValue::PropertySetter | QScriptValue::UserRange);
            object4.setProperty("foo", str);
            QCOMPARE(object4.property("x").strictlyEquals(str), true);
            QCOMPARE(object4.property("foo").strictlyEquals(str), true);
            
            // kill the getter
            object4.setProperty("foo", QScriptValue(), QScriptValue::PropertyGetter);
            QCOMPARE(object4.property("foo").isValid(), false);
            
            // setter should still work
            object4.setProperty("foo", num);
            QCOMPARE(object4.property("x").strictlyEquals(num), true);
            
            // kill the setter too
            object4.setProperty("foo", QScriptValue(), QScriptValue::PropertySetter);
            // now foo is just a regular property
            object4.setProperty("foo", str);
            QCOMPARE(object4.property("x").strictlyEquals(num), true);
            QCOMPARE(object4.property("foo").strictlyEquals(str), true);
        }

        for (int x = 0; x < 2; ++x) {
            object4.setProperty("foo", QScriptValue());
            // setter() sets this.x
            object4.setProperty("foo", eng.newFunction(setter), QScriptValue::PropertySetter);
            object4.setProperty("foo", str);
            QCOMPARE(object4.property("x").strictlyEquals(str), true);
            QCOMPARE(object4.property("foo").isValid(), false);
            
            // getter() returns this.x
            object4.setProperty("foo", eng.newFunction(getter), QScriptValue::PropertyGetter);
            object4.setProperty("x", num);
            QCOMPARE(object4.property("foo").strictlyEquals(num), true);
            
            // kill the setter
            object4.setProperty("foo", QScriptValue(), QScriptValue::PropertySetter);
            QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setProperty() failed: property 'foo' has a getter but no setter");
            object4.setProperty("foo", str);
            
            // getter should still work
            QCOMPARE(object4.property("foo").strictlyEquals(num), true);
            
            // kill the getter too
            object4.setProperty("foo", QScriptValue(), QScriptValue::PropertyGetter);
            // now foo is just a regular property
            object4.setProperty("foo", str);
            QCOMPARE(object4.property("x").strictlyEquals(num), true);
            QCOMPARE(object4.property("foo").strictlyEquals(str), true);
        }

        // use a single function as both getter and setter
        object4.setProperty("foo", QScriptValue());
        object4.setProperty("foo", eng.newFunction(getterSetter),
                            QScriptValue::PropertyGetter | QScriptValue::PropertySetter
                            | QScriptValue::UserRange);
        QCOMPARE(object4.propertyFlags("foo"),
                 QScriptValue::PropertyGetter | QScriptValue::PropertySetter
                 | QScriptValue::UserRange);
        object4.setProperty("x", num);
        QCOMPARE(object4.property("foo").strictlyEquals(num), true);

        // killing the getter will also kill the setter, since they are the same function
        object4.setProperty("foo", QScriptValue(), QScriptValue::PropertyGetter);
        QCOMPARE(object4.property("foo").isValid(), false);
        // now foo is just a regular property
        object4.setProperty("foo", str);
        QCOMPARE(object4.property("x").strictlyEquals(num), true);
        QCOMPARE(object4.property("foo").strictlyEquals(str), true);

        // getter/setter that throws an error
        {
            QScriptValue object5 = eng.newObject();
            object5.setProperty("foo", eng.newFunction(getterSetterThrowingError),
                                QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
            QVERIFY(!eng.hasUncaughtException());
            QScriptValue ret = object5.property("foo");
            QVERIFY(ret.isError());
            QVERIFY(eng.hasUncaughtException());
            QVERIFY(ret.strictlyEquals(eng.uncaughtException()));
            eng.evaluate("Object"); // clear exception state...
            QVERIFY(!eng.hasUncaughtException());
            object5.setProperty("foo", str);
            QVERIFY(eng.hasUncaughtException());
            QCOMPARE(eng.uncaughtException().toString(), QLatin1String("Error: set foo"));
        }

        // attempt to install getter+setter on built-in (native) property
        {
            QScriptValue object6 = eng.newObject();
            QVERIFY(object6.property("__proto__").strictlyEquals(object6.prototype()));

            QScriptValue fun = eng.newFunction(getSet__proto__);
            fun.setProperty("value", QScriptValue(&eng, "boo"));
            QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setProperty() failed: "
                                 "cannot set getter or setter of native property "
                                 "`__proto__'");
            object6.setProperty("__proto__", fun,
                                QScriptValue::PropertyGetter | QScriptValue::PropertySetter
                                | QScriptValue::UserRange);
            QVERIFY(object6.property("__proto__").strictlyEquals(object6.prototype()));

            object6.setProperty("__proto__", QScriptValue(),
                                QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
            QVERIFY(object6.property("__proto__").strictlyEquals(object6.prototype()));
        }
    }

    eng.globalObject().setProperty("object", object);

  // ReadOnly
    object.setProperty("readOnlyProperty", num, QScriptValue::ReadOnly);
    QCOMPARE(object.propertyFlags("readOnlyProperty"), QScriptValue::ReadOnly);
    QCOMPARE(object.property("readOnlyProperty").strictlyEquals(num), true);
    eng.evaluate("object.readOnlyProperty = !object.readOnlyProperty");
    QCOMPARE(object.property("readOnlyProperty").strictlyEquals(num), true);
    // should still be part of enumeration
    {
        QScriptValue ret = eng.evaluate(
            "found = false;"
            "for (var p in object) {"
            "  if (p == 'readOnlyProperty') {"
            "    found = true; break;"
            "  }"
            "} found");
        QCOMPARE(ret.strictlyEquals(QScriptValue(&eng, true)), true);
    }
    // should still be deletable
    {
        QScriptValue ret = eng.evaluate("delete object.readOnlyProperty");
        QCOMPARE(ret.strictlyEquals(QScriptValue(&eng, true)), true);
        QCOMPARE(object.property("readOnlyProperty").isValid(), false);
    }

  // Undeletable
    object.setProperty("undeletableProperty", num, QScriptValue::Undeletable);
    QCOMPARE(object.propertyFlags("undeletableProperty"), QScriptValue::Undeletable);
    QCOMPARE(object.property("undeletableProperty").strictlyEquals(num), true);
    {
        QScriptValue ret = eng.evaluate("delete object.undeletableProperty");
        QCOMPARE(ret.strictlyEquals(QScriptValue(&eng, true)), false);
        QCOMPARE(object.property("undeletableProperty").strictlyEquals(num), true);
    }
    // should still be writable
    eng.evaluate("object.undeletableProperty = object.undeletableProperty + 1");
    QCOMPARE(object.property("undeletableProperty").toNumber(), num.toNumber() + 1);
    // should still be part of enumeration
    {
        QScriptValue ret = eng.evaluate(
            "found = false;"
            "for (var p in object) {"
            "  if (p == 'undeletableProperty') {"
            "    found = true; break;"
            "  }"
            "} found");
        QCOMPARE(ret.strictlyEquals(QScriptValue(&eng, true)), true);
    }

  // SkipInEnumeration
    object.setProperty("dontEnumProperty", num, QScriptValue::SkipInEnumeration);
    QCOMPARE(object.propertyFlags("dontEnumProperty"), QScriptValue::SkipInEnumeration);
    QCOMPARE(object.property("dontEnumProperty").strictlyEquals(num), true);
    // should not be part of enumeration
    {
        QScriptValue ret = eng.evaluate(
            "found = false;"
            "for (var p in object) {"
            "  if (p == 'dontEnumProperty') {"
            "    found = true; break;"
            "  }"
            "} found");
        QCOMPARE(ret.strictlyEquals(QScriptValue(&eng, false)), true);
    }
    // should still be writable
    eng.evaluate("object.dontEnumProperty = object.dontEnumProperty + 1");
    QCOMPARE(object.property("dontEnumProperty").toNumber(), num.toNumber() + 1);
    // should still be deletable
    {
        QScriptValue ret = eng.evaluate("delete object.dontEnumProperty");
        QCOMPARE(ret.strictlyEquals(QScriptValue(&eng, true)), true);
        QCOMPARE(object.property("dontEnumProperty").isValid(), false);
    }

    // change flags
    object.setProperty("flagProperty", str);
    QCOMPARE(object.propertyFlags("flagProperty"), static_cast<QScriptValue::PropertyFlags>(0));

    object.setProperty("flagProperty", str, QScriptValue::ReadOnly);
    QCOMPARE(object.propertyFlags("flagProperty"), QScriptValue::ReadOnly);

    object.setProperty("flagProperty", str, object.propertyFlags("flagProperty") | QScriptValue::Undeletable);
    QCOMPARE(object.propertyFlags("flagProperty"), QScriptValue::ReadOnly | QScriptValue::Undeletable);

    object.setProperty("flagProperty", str, QScriptValue::KeepExistingFlags);
    QCOMPARE(object.propertyFlags("flagProperty"), QScriptValue::ReadOnly | QScriptValue::Undeletable);

    object.setProperty("flagProperty", str, QScriptValue::UserRange);
    QCOMPARE(object.propertyFlags("flagProperty"), QScriptValue::UserRange);
}

void tst_QScriptValue::getSetPrototype()
{
    QScriptEngine eng;

    QScriptValue object = eng.newObject();

    QScriptValue object2 = eng.newObject();
    object2.setPrototype(object);

    QCOMPARE(object2.prototype().strictlyEquals(object), true);

    QScriptValue inv;
    inv.setPrototype(object);
    QCOMPARE(inv.prototype().isValid(), false);

    QScriptEngine otherEngine;
    QScriptValue object3 = otherEngine.newObject();
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setPrototype() failed: cannot set a prototype created in a different engine");
    object2.setPrototype(object3);
    QCOMPARE(object2.prototype().strictlyEquals(object), true);

    // cyclic prototypes
    QScriptValue old = object.prototype();
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setPrototype() failed: cyclic prototype value");
    object.setPrototype(object);
    QCOMPARE(object.prototype().strictlyEquals(old), true);

    object2.setPrototype(object);
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setPrototype() failed: cyclic prototype value");
    object.setPrototype(object2);
    QCOMPARE(object.prototype().strictlyEquals(old), true);

    {
        QScriptValue ret = eng.evaluate("o = { }; p = { }; o.__proto__ = p; p.__proto__ = o");
        QCOMPARE(eng.hasUncaughtException(), true);
        QVERIFY(ret.strictlyEquals(eng.uncaughtException()));
        QCOMPARE(ret.isError(), true);
        QCOMPARE(ret.toString(), QLatin1String("Error: cycle in prototype chain"));
    }
    {
        QScriptValue ret = eng.evaluate("p.__proto__ = { }");
        QCOMPARE(eng.hasUncaughtException(), false);
        QCOMPARE(ret.isError(), false);
    }
}

void tst_QScriptValue::getSetScope()
{
    QScriptEngine eng;

    QScriptValue object = eng.newObject();
    QCOMPARE(object.scope().isValid(), false);

    QScriptValue object2 = eng.newObject();
    object2.setScope(object);

    QCOMPARE(object2.scope().strictlyEquals(object), true);

    QScriptValue inv;
    inv.setScope(object);
    QCOMPARE(inv.scope().isValid(), false);

    QScriptEngine otherEngine;
    QScriptValue object3 = otherEngine.newObject();
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::setScope() failed: cannot set a scope object created in a different engine");
    object2.setScope(object3);
    QCOMPARE(object2.scope().strictlyEquals(object), true);
}

static QScriptValue getArg(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->argument(0);
}

static QScriptValue evaluateArg(QScriptContext *, QScriptEngine *eng)
{
    return eng->evaluate("arguments[0]");
}

static QScriptValue addArgs(QScriptContext *, QScriptEngine *eng)
{
    return eng->evaluate("arguments[0] + arguments[1]");
}

static QScriptValue returnInvalidValue(QScriptContext *, QScriptEngine *)
{
    return QScriptValue();
}

void tst_QScriptValue::call()
{
    QScriptEngine eng;

    QScriptValue Object = eng.evaluate("Object");
    QCOMPARE(Object.isFunction(), true);
    {
        QScriptValue result = Object.call(Object);
        QCOMPARE(result.isObject(), true);
    }

    // test that call() doesn't construct new objects
    QScriptValue Number = eng.evaluate("Number");
    QCOMPARE(Object.isFunction(), true);
    {
        QScriptValueList args;
        args << QScriptValue(&eng, 123);
        QScriptValue result = Number.call(Object, args);
        QCOMPARE(result.strictlyEquals(args.at(0)), true);
    }

    // test that correct "this" object is used
    {
        QScriptValue fun = eng.evaluate("function() { return this; }");
        QCOMPARE(fun.isFunction(), true);

        {
            QScriptValue numberObject = QScriptValue(&eng, 123.0).toObject();
            QScriptValue result = fun.call(numberObject);
            QCOMPARE(result.isObject(), true);
            QCOMPARE(result.toNumber(), 123.0);
        }
    }

    // test that correct arguments are passed
    {
        QScriptValue fun = eng.evaluate("function() { return arguments[0]; }");
        QCOMPARE(fun.isFunction(), true);

        {
            QScriptValue result = fun.call(eng.undefinedValue());
            QCOMPARE(result.isUndefined(), true);
        }

        {
            QScriptValueList args;
            args << QScriptValue(&eng, 123.0);
            QScriptValue result = fun.call(eng.undefinedValue(), args);
            QCOMPARE(result.isNumber(), true);
            QCOMPARE(result.toNumber(), 123.0);
        }
    }

    {
        QScriptValue fun = eng.evaluate("function() { return arguments[1]; }");
        QCOMPARE(fun.isFunction(), true);

        {
            QScriptValueList args;
            args << QScriptValue(&eng, 123.0) << QScriptValue(&eng, 456.0);
            QScriptValue result = fun.call(eng.undefinedValue(), args);
            QCOMPARE(result.isNumber(), true);
            QCOMPARE(result.toNumber(), 456.0);
        }
    }

    {
        QScriptValue fun = eng.evaluate("function() { throw new Error('foo'); }");
        QCOMPARE(fun.isFunction(), true);

        {
            QScriptValue result = fun.call();
            QCOMPARE(result.isError(), true);
            QCOMPARE(eng.hasUncaughtException(), true);
            QVERIFY(result.strictlyEquals(eng.uncaughtException()));
        }
    }

    {
        QScriptValue fun = eng.newFunction(getArg);
        {
            QScriptValueList args;
            args << QScriptValue(&eng, 123.0);
            QScriptValue result = fun.call(eng.undefinedValue(), args);
            QCOMPARE(result.isNumber(), true);
            QCOMPARE(result.toNumber(), 123.0);
        }
    }

    {
        QScriptValue fun = eng.newFunction(evaluateArg);
        {
            QScriptValueList args;
            args << QScriptValue(&eng, 123.0);
            QScriptValue result = fun.call(eng.undefinedValue(), args);
            QCOMPARE(result.isNumber(), true);
            QCOMPARE(result.toNumber(), 123.0);
        }
    }

    QScriptValue inv;
    QCOMPARE(inv.call().isValid(), false);

    // test that invalid arguments are handled gracefully
    {
        QScriptValue fun = eng.newFunction(getArg);
        {
            QScriptValueList args;
            args << QScriptValue();
            QScriptValue ret = fun.call(QScriptValue(), args);
            QCOMPARE(ret.isValid(), true);
            QCOMPARE(ret.isUndefined(), true);
        }
    }
    {
        QScriptValue fun = eng.newFunction(evaluateArg);
        {
            QScriptValueList args;
            args << QScriptValue();
            QScriptValue ret = fun.call(QScriptValue(), args);
            QCOMPARE(ret.isValid(), true);
            QCOMPARE(ret.isUndefined(), true);
        }
    }
    {
        QScriptValue fun = eng.newFunction(addArgs);
        {
            QScriptValueList args;
            args << QScriptValue() << QScriptValue();
            QScriptValue ret = fun.call(QScriptValue(), args);
            QCOMPARE(ret.isValid(), true);
            QCOMPARE(ret.isNumber(), true);
            QCOMPARE(qIsNaN(ret.toNumber()), true);
        }
    }

    // test that invalid return value is handled gracefully
    {
        QScriptValue fun = eng.newFunction(returnInvalidValue);
        eng.globalObject().setProperty("returnInvalidValue", fun);
        QScriptValue ret = eng.evaluate("returnInvalidValue() + returnInvalidValue()");
        QCOMPARE(ret.isValid(), true);
        QCOMPARE(ret.isNumber(), true);
        QCOMPARE(qIsNaN(ret.toNumber()), true);
    }

    {
        QScriptEngine otherEngine;
        QScriptValue fun = otherEngine.evaluate("function() { return 1; }");
        QTest::ignoreMessage(QtWarningMsg, "QScriptValue::call() failed: "
                             "cannot call function with thisObject created in "
                             "a different engine");
        QCOMPARE(fun.call(Object).isValid(), false);
        QTest::ignoreMessage(QtWarningMsg, "QScriptValue::call() failed: "
                             "cannot call function with argument created in "
                             "a different engine");
        QCOMPARE(fun.call(QScriptValue(), QScriptValueList() << QScriptValue(&eng, 123)).isValid(), false);
    }

    {
        QScriptValue fun = eng.evaluate("function() { return arguments; }");
        QScriptValue array = eng.newArray(3);
        array.setProperty(0, QScriptValue(&eng, 123.0));
        array.setProperty(1, QScriptValue(&eng, 456.0));
        array.setProperty(2, QScriptValue(&eng, 789.0));
        // call with single array object as arguments
        QScriptValue ret = fun.call(QScriptValue(), array);
        QCOMPARE(ret.isError(), false);
        QCOMPARE(ret.property(0).strictlyEquals(array.property(0)), true);
        QCOMPARE(ret.property(1).strictlyEquals(array.property(1)), true);
        QCOMPARE(ret.property(2).strictlyEquals(array.property(2)), true);
        // call with arguments object as arguments
        QScriptValue ret2 = fun.call(QScriptValue(), ret);
        QCOMPARE(ret2.isError(), false);
        QCOMPARE(ret2.property(0).strictlyEquals(ret.property(0)), true);
        QCOMPARE(ret2.property(1).strictlyEquals(ret.property(1)), true);
        QCOMPARE(ret2.property(2).strictlyEquals(ret.property(2)), true);
        // call with null as arguments
        QScriptValue ret3 = fun.call(QScriptValue(), eng.nullValue());
        QCOMPARE(ret3.isError(), false);
        QCOMPARE(ret3.property("length").isNumber(), true);
        QCOMPARE(ret3.property("length").toNumber(), 0.0);
        // call with undefined as arguments
        QScriptValue ret4 = fun.call(QScriptValue(), eng.undefinedValue());
        QCOMPARE(ret4.isError(), false);
        QCOMPARE(ret4.property("length").isNumber(), true);
        QCOMPARE(ret4.property("length").toNumber(), 0.0);
        // call with something else as arguments
        QScriptValue ret5 = fun.call(QScriptValue(), QScriptValue(&eng, 123.0));
        QCOMPARE(ret5.isError(), true);
    }
}

void tst_QScriptValue::construct()
{
    QScriptEngine eng;

    QScriptValue Number = eng.evaluate("Number");
    QCOMPARE(Number.isFunction(), true);
    {
        QScriptValueList args;
        args << QScriptValue(&eng, 123);
        QScriptValue ret = Number.construct(args);
        QCOMPARE(ret.isObject(), true);
        QCOMPARE(ret.toNumber(), args.at(0).toNumber());
    }

    // test that internal prototype is set correctly
    {
        QScriptValue fun = eng.evaluate("function() { return this.__proto__; }");
        QCOMPARE(fun.isFunction(), true);
        QCOMPARE(fun.property("prototype").isObject(), true);
        QScriptValue ret = fun.construct();
        QCOMPARE(fun.property("prototype").strictlyEquals(ret), true);
    }

    // test that we return the new object even if a non-object value is returned from the function
    {
        QScriptValue fun = eng.evaluate("function() { return 123; }");
        QCOMPARE(fun.isFunction(), true);
        QScriptValue ret = fun.construct();
        QCOMPARE(ret.isObject(), true);
    }

    {
        QScriptValue fun = eng.evaluate("function() { throw new Error('foo'); }");
        QCOMPARE(fun.isFunction(), true);
        QScriptValue ret = fun.construct();
        QCOMPARE(ret.isError(), true);
        QCOMPARE(eng.hasUncaughtException(), true);
        QVERIFY(ret.strictlyEquals(eng.uncaughtException()));
    }

    QScriptValue inv;
    QCOMPARE(inv.construct().isValid(), false);

    {
        QScriptValue fun = eng.evaluate("function() { return arguments; }");
        QScriptValue array = eng.newArray(3);
        array.setProperty(0, QScriptValue(&eng, 123.0));
        array.setProperty(1, QScriptValue(&eng, 456.0));
        array.setProperty(2, QScriptValue(&eng, 789.0));
        // construct with single array object as arguments
        QScriptValue ret = fun.construct(array);
        QCOMPARE(ret.property(0).strictlyEquals(array.property(0)), true);
        QCOMPARE(ret.property(1).strictlyEquals(array.property(1)), true);
        QCOMPARE(ret.property(2).strictlyEquals(array.property(2)), true);
        // construct with arguments object as arguments
        QScriptValue ret2 = fun.construct(ret);
        QCOMPARE(ret2.property(0).strictlyEquals(ret.property(0)), true);
        QCOMPARE(ret2.property(1).strictlyEquals(ret.property(1)), true);
        QCOMPARE(ret2.property(2).strictlyEquals(ret.property(2)), true);
        // construct with null as arguments
        QScriptValue ret3 = fun.construct(eng.nullValue());
        QCOMPARE(ret3.isError(), false);
        QCOMPARE(ret3.property("length").isNumber(), true);
        QCOMPARE(ret3.property("length").toNumber(), 0.0);
        // construct with undefined as arguments
        QScriptValue ret4 = fun.construct(eng.undefinedValue());
        QCOMPARE(ret4.isError(), false);
        QCOMPARE(ret4.property("length").isNumber(), true);
        QCOMPARE(ret4.property("length").toNumber(), 0.0);
        // construct with something else as arguments
        QScriptValue ret5 = fun.construct(QScriptValue(&eng, 123.0));
        QCOMPARE(ret5.isError(), true);
    }
}

void tst_QScriptValue::lessThan()
{
    QScriptEngine eng;

    QScriptValue num = QScriptValue(&eng, 123);
    QCOMPARE(num.lessThan(QScriptValue(&eng, 124)), true);
    QCOMPARE(num.lessThan(QScriptValue(&eng, 122)), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, 123)), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, "124")), true);
    QCOMPARE(num.lessThan(QScriptValue(&eng, "122")), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, "123")), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, qSNaN())), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, +qInf())), true);
    QCOMPARE(num.lessThan(QScriptValue(&eng, -qInf())), false);
    QCOMPARE(num.lessThan(num), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, 124).toObject()), true);
    QCOMPARE(num.lessThan(QScriptValue(&eng, 122).toObject()), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, 123).toObject()), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, "124").toObject()), true);
    QCOMPARE(num.lessThan(QScriptValue(&eng, "122").toObject()), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, "123").toObject()), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, qSNaN()).toObject()), false);
    QCOMPARE(num.lessThan(QScriptValue(&eng, +qInf()).toObject()), true);
    QCOMPARE(num.lessThan(QScriptValue(&eng, -qInf()).toObject()), false);
    QCOMPARE(num.lessThan(num.toObject()), false);
    QCOMPARE(num.lessThan(QScriptValue()), false);

    QScriptValue str = QScriptValue(&eng, "123");
    QCOMPARE(str.lessThan(QScriptValue(&eng, "124")), true);
    QCOMPARE(str.lessThan(QScriptValue(&eng, "122")), false);
    QCOMPARE(str.lessThan(QScriptValue(&eng, "123")), false);
    QCOMPARE(str.lessThan(QScriptValue(&eng, 124)), true);
    QCOMPARE(str.lessThan(QScriptValue(&eng, 122)), false);
    QCOMPARE(str.lessThan(QScriptValue(&eng, 123)), false);
    QCOMPARE(str.lessThan(str), false);
    QCOMPARE(str.lessThan(QScriptValue(&eng, "124").toObject()), true);
    QCOMPARE(str.lessThan(QScriptValue(&eng, "122").toObject()), false);
    QCOMPARE(str.lessThan(QScriptValue(&eng, "123").toObject()), false);
    QCOMPARE(str.lessThan(QScriptValue(&eng, 124).toObject()), true);
    QCOMPARE(str.lessThan(QScriptValue(&eng, 122).toObject()), false);
    QCOMPARE(str.lessThan(QScriptValue(&eng, 123).toObject()), false);
    QCOMPARE(str.lessThan(str.toObject()), false);
    QCOMPARE(str.lessThan(QScriptValue()), false);

    QScriptValue obj1 = eng.newObject();
    QScriptValue obj2 = eng.newObject();
    QCOMPARE(obj1.lessThan(obj2), false);
    QCOMPARE(obj2.lessThan(obj1), false);
    QCOMPARE(obj1.lessThan(obj1), false);
    QCOMPARE(obj2.lessThan(obj2), false);

    QScriptValue date1 = eng.newDate(QDateTime(QDate(2000, 1, 1)));
    QScriptValue date2 = eng.newDate(QDateTime(QDate(1999, 1, 1)));
    QCOMPARE(date1.lessThan(date2), false);
    QCOMPARE(date2.lessThan(date1), true);
    QCOMPARE(date1.lessThan(date1), false);
    QCOMPARE(date2.lessThan(date2), false);
    QCOMPARE(date1.lessThan(QScriptValue()), false);

    QCOMPARE(QScriptValue().lessThan(date2), false);

    QScriptEngine otherEngine;
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::lessThan: "
                         "cannot compare to a value created in "
                         "a different engine");
    QCOMPARE(date1.lessThan(QScriptValue(&otherEngine, 123)), false);
}

void tst_QScriptValue::equals()
{
    QScriptEngine eng;

    QScriptValue num = QScriptValue(&eng, 123);
    QCOMPARE(num.equals(QScriptValue(&eng, 123)), true);
    QCOMPARE(num.equals(QScriptValue(&eng, 321)), false);
    QCOMPARE(num.equals(QScriptValue(&eng, "123")), true);
    QCOMPARE(num.equals(QScriptValue(&eng, "321")), false);
    QCOMPARE(num.equals(QScriptValue(&eng, 123).toObject()), true);
    QCOMPARE(num.equals(QScriptValue(&eng, 321).toObject()), false);
    QCOMPARE(num.equals(QScriptValue(&eng, "123").toObject()), true);
    QCOMPARE(num.equals(QScriptValue(&eng, "321").toObject()), false);
    QCOMPARE(num.equals(QScriptValue()), false);

    QScriptValue str = QScriptValue(&eng, "123");
    QCOMPARE(str.equals(QScriptValue(&eng, "123")), true);
    QCOMPARE(str.equals(QScriptValue(&eng, "321")), false);
    QCOMPARE(str.equals(QScriptValue(&eng, 123)), true);
    QCOMPARE(str.equals(QScriptValue(&eng, 321)), false);
    QCOMPARE(str.equals(QScriptValue(&eng, "123").toObject()), true);
    QCOMPARE(str.equals(QScriptValue(&eng, "321").toObject()), false);
    QCOMPARE(str.equals(QScriptValue(&eng, 123).toObject()), true);
    QCOMPARE(str.equals(QScriptValue(&eng, 321).toObject()), false);
    QCOMPARE(str.equals(QScriptValue()), false);

    QScriptValue date1 = eng.newDate(QDateTime(QDate(2000, 1, 1)));
    QScriptValue date2 = eng.newDate(QDateTime(QDate(1999, 1, 1)));
    QCOMPARE(date1.equals(date2), false);
    QCOMPARE(date1.equals(date1), true);
    QCOMPARE(date2.equals(date2), true);

    QScriptValue undefined = eng.undefinedValue();
    QScriptValue null = eng.nullValue();
    QCOMPARE(undefined.equals(undefined), true);
    QCOMPARE(null.equals(null), true);
    QCOMPARE(undefined.equals(null), true);
    QCOMPARE(null.equals(undefined), true);
    QCOMPARE(undefined.equals(QScriptValue()), false);
    QCOMPARE(null.equals(QScriptValue()), false);

    QScriptValue obj1 = eng.newObject();
    QScriptValue obj2 = eng.newObject();
    QCOMPARE(obj1.equals(obj2), false);
    QCOMPARE(obj2.equals(obj1), false);
    QCOMPARE(obj1.equals(obj1), true);
    QCOMPARE(obj2.equals(obj2), true);

    QScriptEngine otherEngine;
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::equals: "
                         "cannot compare to a value created in "
                         "a different engine");
    QCOMPARE(date1.equals(QScriptValue(&otherEngine, 123)), false);
}

void tst_QScriptValue::strictlyEquals()
{
    QScriptEngine eng;

    QScriptValue num = QScriptValue(&eng, 123);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, 123)), true);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, 321)), false);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, "123")), false);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, "321")), false);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, 123).toObject()), false);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, 321).toObject()), false);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, "123").toObject()), false);
    QCOMPARE(num.strictlyEquals(QScriptValue(&eng, "321").toObject()), false);

    QScriptValue str = QScriptValue(&eng, "123");
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, "123")), true);
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, "321")), false);
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, 123)), false);
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, 321)), false);
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, "123").toObject()), false);
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, "321").toObject()), false);
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, 123).toObject()), false);
    QCOMPARE(str.strictlyEquals(QScriptValue(&eng, 321).toObject()), false);

    QScriptValue date1 = eng.newDate(QDateTime(QDate(2000, 1, 1)));
    QScriptValue date2 = eng.newDate(QDateTime(QDate(1999, 1, 1)));
    QCOMPARE(date1.strictlyEquals(date2), false);
    QCOMPARE(date1.strictlyEquals(date1), true);
    QCOMPARE(date2.strictlyEquals(date2), true);

    QScriptValue undefined = eng.undefinedValue();
    QScriptValue null = eng.nullValue();
    QCOMPARE(undefined.strictlyEquals(undefined), true);
    QCOMPARE(null.strictlyEquals(null), true);
    QCOMPARE(undefined.strictlyEquals(null), false);
    QCOMPARE(null.strictlyEquals(undefined), false);

    QScriptValue obj1 = eng.newObject();
    QScriptValue obj2 = eng.newObject();
    QCOMPARE(obj1.strictlyEquals(obj2), false);
    QCOMPARE(obj2.strictlyEquals(obj1), false);
    QCOMPARE(obj1.strictlyEquals(obj1), true);
    QCOMPARE(obj2.strictlyEquals(obj2), true);

    QScriptEngine otherEngine;
    QTest::ignoreMessage(QtWarningMsg, "QScriptValue::strictlyEquals: "
                         "cannot compare to a value created in "
                         "a different engine");
    QCOMPARE(date1.strictlyEquals(QScriptValue(&otherEngine, 123)), false);
}

Q_DECLARE_METATYPE(int*)
Q_DECLARE_METATYPE(double*)
Q_DECLARE_METATYPE(QColor*)
Q_DECLARE_METATYPE(QBrush*)

void tst_QScriptValue::castToPointer()
{
    QScriptEngine eng;
    {
        QScriptValue v = eng.newVariant(int(123));
        int *ip = qscriptvalue_cast<int*>(v);
        QVERIFY(ip != 0);
        QCOMPARE(*ip, 123);
        *ip = 456;
        QCOMPARE(qscriptvalue_cast<int>(v), 456);

        double *dp = qscriptvalue_cast<double*>(v);
        QVERIFY(dp == 0);

        QScriptValue v2 = eng.newVariant(qVariantFromValue(ip));
        QCOMPARE(qscriptvalue_cast<int*>(v2), ip);
    }
    {
        QColor c(123, 210, 231);
        QScriptValue v = eng.newVariant(c);
        QColor *cp = qscriptvalue_cast<QColor*>(v);
        QVERIFY(cp != 0);
        QCOMPARE(*cp, c);

        QBrush *bp = qscriptvalue_cast<QBrush*>(v);
        QVERIFY(bp == 0);

        QScriptValue v2 = eng.newVariant(qVariantFromValue(cp));
        QCOMPARE(qscriptvalue_cast<QColor*>(v2), cp);
    }
}

QTEST_MAIN(tst_QScriptValue)
#include "tst_qscriptvalue.moc"

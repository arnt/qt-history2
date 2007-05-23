/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptvalueiterator.h>

//TESTED_CLASS=
//TESTED_FILES=qscriptvalueiterator.h qscriptvalueiterator.cpp

class tst_QScriptValueIterator : public QObject
{
    Q_OBJECT

public:
    tst_QScriptValueIterator();
    virtual ~tst_QScriptValueIterator();

private slots:
    void iterateForward_data();
    void iterateForward();
    void iterateBackward_data();
    void iterateBackward();
    void iterateArray();
    void iterateBackAndForth();
    void setValue();
    void remove();
};

tst_QScriptValueIterator::tst_QScriptValueIterator()
{
}

tst_QScriptValueIterator::~tst_QScriptValueIterator()
{
}

void tst_QScriptValueIterator::iterateForward_data()
{
    QTest::addColumn<QStringList>("propertyNames");
    QTest::addColumn<QStringList>("propertyValues");

    QTest::newRow("no properties")
        << QStringList() << QStringList();
    QTest::newRow("foo=bar")
        << (QStringList() << "foo")
        << (QStringList() << "bar");
    QTest::newRow("foo=bar, baz=123")
        << (QStringList() << "foo" << "baz")
        << (QStringList() << "bar" << "123");
    QTest::newRow("foo=bar, baz=123, rab=oof")
        << (QStringList() << "foo" << "baz" << "rab")
        << (QStringList() << "bar" << "123" << "oof");
}

void tst_QScriptValueIterator::iterateForward()
{
    QFETCH(QStringList, propertyNames);
    QFETCH(QStringList, propertyValues);
    QMap<QString, QString> pmap;
    Q_ASSERT(propertyNames.size() == propertyValues.size());

    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    for (int i = 0; i < propertyNames.size(); ++i) {
        QString name = propertyNames.at(i);
        QString value = propertyValues.at(i);
        pmap.insert(name, value);
        object.setProperty(name, QScriptValue(&engine, value));
    }

    QStringList lst;
    QScriptValueIterator it(object);
    while (!pmap.isEmpty()) {
        QCOMPARE(it.hasNext(), true);
        QCOMPARE(it.hasNext(), true);
        it.next();
        QString name = it.name();
        QCOMPARE(pmap.contains(name), true);
        QCOMPARE(it.name(), name);
        QCOMPARE(it.value().strictlyEquals(QScriptValue(&engine, pmap.value(name))), true);
        pmap.remove(name);
        lst.append(name);
    }

    QCOMPARE(it.hasNext(), false);
    QCOMPARE(it.hasNext(), false);

    it.toFront();
    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasNext(), true);
        it.next();
        QCOMPARE(it.name(), lst.at(i));
    }

    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasPrevious(), true);
        it.previous();
        QCOMPARE(it.name(), lst.at(lst.count()-1-i));
    }
    QCOMPARE(it.hasPrevious(), false);
    it.previous();
    QCOMPARE(it.name(), QString());
    QCOMPARE(it.hasPrevious(), false);
}

void tst_QScriptValueIterator::iterateBackward_data()
{
    iterateForward_data();
}

void tst_QScriptValueIterator::iterateBackward()
{
    QFETCH(QStringList, propertyNames);
    QFETCH(QStringList, propertyValues);
    QMap<QString, QString> pmap;
    Q_ASSERT(propertyNames.size() == propertyValues.size());

    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    for (int i = 0; i < propertyNames.size(); ++i) {
        QString name = propertyNames.at(i);
        QString value = propertyValues.at(i);
        pmap.insert(name, value);
        object.setProperty(name, QScriptValue(&engine, value));
    }

    QStringList lst;
    QScriptValueIterator it(object);
    it.toBack();
    while (!pmap.isEmpty()) {
        QCOMPARE(it.hasPrevious(), true);
        QCOMPARE(it.hasPrevious(), true);
        it.previous();
        QString name = it.name();
        QCOMPARE(pmap.contains(name), true);
        QCOMPARE(it.name(), name);
        QCOMPARE(it.value().strictlyEquals(QScriptValue(&engine, pmap.value(name))), true);
        pmap.remove(name);
        lst.append(name);
    }

    QCOMPARE(it.hasPrevious(), false);
    QCOMPARE(it.hasPrevious(), false);

    it.toBack();
    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasPrevious(), true);
        it.previous();
        QCOMPARE(it.name(), lst.at(i));
    }

    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasNext(), true);
        it.next();
        QCOMPARE(it.name(), lst.at(lst.count()-1-i));
    }
    QCOMPARE(it.hasNext(), false);
    it.next();
    QCOMPARE(it.name(), QString());
//    QEXPECT_FAIL("", "iterator wraps around", Continue);
//    QCOMPARE(it.hasNext(), false);
}

void tst_QScriptValueIterator::iterateArray()
{
    QScriptEngine engine;
    QScriptValue array = engine.newArray();
    array.setProperty("0", QScriptValue(&engine, 123));
    array.setProperty("1", QScriptValue(&engine, 456));
    array.setProperty("2", QScriptValue(&engine, 789));
    int length = array.property("length").toInt32();
    QScriptValueIterator it(array);
    for (int i = 0; i < length; ++i) {
        QCOMPARE(it.hasNext(), true);
        QString indexStr = QScriptValue(&engine, i).toString();
        it.next();
        QCOMPARE(it.name(), indexStr);
        QCOMPARE(it.value().strictlyEquals(array.property(indexStr)), true);
    }
    QCOMPARE(it.hasNext(), false);
}

void tst_QScriptValueIterator::iterateBackAndForth()
{
    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    object.setProperty("foo", QScriptValue(&engine, "bar"));
    object.setProperty("rab", QScriptValue(&engine, "oof"),
                       QScriptValue::SkipInEnumeration); // should not affect iterator
    QScriptValueIterator it(object);
    it.next();
    QCOMPARE(it.name(), QLatin1String("foo"));
    it.previous();
    QCOMPARE(it.name(), QLatin1String("foo"));
    it.next();
    QCOMPARE(it.name(), QLatin1String("foo"));
    it.previous();
    QCOMPARE(it.name(), QLatin1String("foo"));
    it.next();
    QCOMPARE(it.name(), QLatin1String("foo"));
    it.next();
    QCOMPARE(it.name(), QLatin1String("rab"));
    it.previous();
    QCOMPARE(it.name(), QLatin1String("rab"));
    it.next();
    QCOMPARE(it.name(), QLatin1String("rab"));
    it.previous();
    QCOMPARE(it.name(), QLatin1String("rab"));
}

void tst_QScriptValueIterator::setValue()
{
    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    object.setProperty("foo", QScriptValue(&engine, "bar"));
    QScriptValueIterator it(object);
    it.next();
    QCOMPARE(it.name(), QLatin1String("foo"));
    it.setValue(QScriptValue(&engine, "baz"));
    QCOMPARE(it.value().strictlyEquals(QScriptValue(&engine, QLatin1String("baz"))), true);
    QCOMPARE(object.property("foo").toString(), QLatin1String("baz"));
    it.setValue(QScriptValue(&engine, "zab"));
    QCOMPARE(it.value().strictlyEquals(QScriptValue(&engine, QLatin1String("zab"))), true);
    QCOMPARE(object.property("foo").toString(), QLatin1String("zab"));
}

void tst_QScriptValueIterator::remove()
{
    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    object.setProperty("foo", QScriptValue(&engine, "bar"),
                       QScriptValue::SkipInEnumeration); // should not affect iterator
    object.setProperty("rab", QScriptValue(&engine, "oof"));
    QScriptValueIterator it(object);
    it.next();
    QCOMPARE(it.name(), QLatin1String("foo"));
    it.remove();
    QCOMPARE(it.hasPrevious(), false);
    QCOMPARE(object.property("foo").isValid(), false);
    QCOMPARE(object.property("rab").toString(), QLatin1String("oof"));
    it.next();
    QCOMPARE(it.name(), QLatin1String("rab"));
    QCOMPARE(it.value().toString(), QLatin1String("oof"));
    QCOMPARE(it.hasNext(), false);
    it.remove();
    QCOMPARE(object.property("rab").isValid(), false);
    QCOMPARE(it.hasPrevious(), false);
    QCOMPARE(it.hasNext(), false);
}

QTEST_MAIN(tst_QScriptValueIterator)
#include "tst_qscriptvalueiterator.moc"

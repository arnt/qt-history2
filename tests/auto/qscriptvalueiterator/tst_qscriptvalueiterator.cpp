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
        object.setProperty(name, engine.scriptValue(value));
    }

    QStringList lst;
    QScriptValueIterator it(object);
    while (!pmap.isEmpty()) {
        QCOMPARE(it.hasNext(), true);
        QCOMPARE(it.hasNext(), true);
        QString name = it.next();
        QCOMPARE(pmap.contains(name), true);
        QCOMPARE(it.name(), name);
        QCOMPARE(it.value().toString(), pmap.value(name));
        pmap.remove(name);
        lst.append(name);
    }

    QCOMPARE(it.hasNext(), false);
    QCOMPARE(it.hasNext(), false);

    it.toFront();
    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasNext(), true);
        QCOMPARE(it.next(), lst.at(i));
    }

    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasPrevious(), true);
        QCOMPARE(it.previous(), lst.at(lst.count()-1-i));
    }
    QCOMPARE(it.hasPrevious(), false);
    QCOMPARE(it.previous(), QString());
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
        object.setProperty(name, engine.scriptValue(value));
    }

    QStringList lst;
    QScriptValueIterator it(object);
    it.toBack();
    while (!pmap.isEmpty()) {
        QCOMPARE(it.hasPrevious(), true);
        QCOMPARE(it.hasPrevious(), true);
        QString name = it.previous();
        QCOMPARE(pmap.contains(name), true);
        QCOMPARE(it.name(), name);
        QCOMPARE(it.value().toString(), pmap.value(name));
        pmap.remove(name);
        lst.append(name);
    }

    QCOMPARE(it.hasPrevious(), false);
    QCOMPARE(it.hasPrevious(), false);

    it.toBack();
    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasPrevious(), true);
        QCOMPARE(it.previous(), lst.at(i));
    }

    for (int i = 0; i < lst.count(); ++i) {
        QCOMPARE(it.hasNext(), true);
        QCOMPARE(it.next(), lst.at(lst.count()-1-i));
    }
    QCOMPARE(it.hasNext(), false);
    QCOMPARE(it.next(), QString());
//    QEXPECT_FAIL("", "iterator wraps around", Continue);
//    QCOMPARE(it.hasNext(), false);
}

void tst_QScriptValueIterator::iterateArray()
{
    QScriptEngine engine;
    QScriptValue array = engine.newArray();
    array.setProperty("0", engine.scriptValue(123));
    array.setProperty("1", engine.scriptValue(456));
    array.setProperty("2", engine.scriptValue(789));
    int length = array.property("length").toInt32();
    QScriptValueIterator it(array);
    for (int i = 0; i < length; ++i) {
        QCOMPARE(it.hasNext(), true);
        QString indexStr = engine.scriptValue(i).toString();
        QCOMPARE(it.next(), indexStr);
        QCOMPARE(it.value().toString(), array.property(indexStr).toString());
    }
    QCOMPARE(it.hasNext(), false);
}

void tst_QScriptValueIterator::iterateBackAndForth()
{
    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    object.setProperty("foo", engine.scriptValue("bar"));
    object.setProperty("rab", engine.scriptValue("oof"));
    QScriptValueIterator it(object);
    QCOMPARE(it.next(), QLatin1String("foo"));
    QCOMPARE(it.previous(), QLatin1String("foo"));
    QCOMPARE(it.next(), QLatin1String("foo"));
    QCOMPARE(it.previous(), QLatin1String("foo"));
    QCOMPARE(it.next(), QLatin1String("foo"));
    QCOMPARE(it.next(), QLatin1String("rab"));
    QCOMPARE(it.previous(), QLatin1String("rab"));
    QCOMPARE(it.next(), QLatin1String("rab"));
    QCOMPARE(it.previous(), QLatin1String("rab"));
}

void tst_QScriptValueIterator::setValue()
{
    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    object.setProperty("foo", engine.scriptValue("bar"));
    QScriptValueIterator it(object);
    QCOMPARE(it.next(), QLatin1String("foo"));
    it.setValue(engine.scriptValue("baz"));
    QCOMPARE(it.value().toString(), QLatin1String("baz"));
    QCOMPARE(object.property("foo").toString(), QLatin1String("baz"));
    it.setValue(engine.scriptValue("zab"));
    QCOMPARE(it.value().toString(), QLatin1String("zab"));
    QCOMPARE(object.property("foo").toString(), QLatin1String("zab"));
}

void tst_QScriptValueIterator::remove()
{
    QScriptEngine engine;
    QScriptValue object = engine.newObject();
    object.setProperty("foo", engine.scriptValue("bar"));
    object.setProperty("rab", engine.scriptValue("oof"));
    QScriptValueIterator it(object);
    QCOMPARE(it.next(), QLatin1String("foo"));
    it.remove();
    QCOMPARE(it.hasPrevious(), false);
    QCOMPARE(object.property("foo").isValid(), false);
    QCOMPARE(object.property("rab").toString(), QLatin1String("oof"));
    QCOMPARE(it.next(), QLatin1String("rab"));
    QCOMPARE(it.value().toString(), QLatin1String("oof"));
    QCOMPARE(it.hasNext(), false);
    it.remove();
    QCOMPARE(object.property("rab").isValid(), false);
    QCOMPARE(it.hasPrevious(), false);
    QCOMPARE(it.hasNext(), false);
}

QTEST_MAIN(tst_QScriptValueIterator)
#include "tst_qscriptvalueiterator.moc"

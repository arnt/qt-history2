/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include "qxmlquery.h"
#include "qxmlname.h"

/*!
 \class tst_QXmlName
 \internal
 \short
 \since 4.4
 \brief Tests class QXmlName.

 This test is not intended for testing the engine, but the functionality specific
 to the QXmlName class.

 In other words, if you have an engine bug; don't add it here because it won't be
 tested properly. Instead add it to the test suite.

 */
class tst_QXmlName : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void defaultConstructor() const;

    void isNull() const;
    void operatorEqual() const;
    void operatorEqual_data() const;
    void operatorNotEqual() const;
    void operatorNotEqual_data() const;
    void constCorrectness() const;
    void qHash() const;
};

void tst_QXmlName::defaultConstructor() const
{
    /* Allocate instance in different orders. */
    {
        QXmlName name;
    }

    {
        QXmlName name1;
        QXmlName name2;
        QXmlName name3;
    }
}

void tst_QXmlName::isNull() const
{
    /* Check default value. */
    QXmlName name;
    QVERIFY(name.isNull());
}

Q_DECLARE_METATYPE(QXmlName)
void tst_QXmlName::operatorEqual() const
{
    QFETCH(QXmlName, op1);
    QFETCH(QXmlName, op2);
    QFETCH(bool, expected);
    
    QCOMPARE(op1 == op2, expected);
}

void tst_QXmlName::operatorEqual_data() const
{
    QTest::addColumn<QXmlName>("op1");
    QTest::addColumn<QXmlName>("op2");
    QTest::addColumn<bool>("expected");

    QXmlQuery query;
    const QXmlName n1(query.createName(QString::fromLatin1("localName1"),
                                       QString::fromLatin1("http://example.com/Namespace1"),
                                       QString::fromLatin1("prefix1")));

    const QXmlName n2(query.createName(QString::fromLatin1("localName2"),
                                       QString::fromLatin1("http://example.com/Namespace1"),
                                       QString::fromLatin1("prefix1")));

    const QXmlName n3(query.createName(QString::fromLatin1("localName2"),
                                       QString::fromLatin1("http://example.com/Namespace1"),
                                       QString::fromLatin1("prefix2")));

    const QXmlName n4(query.createName(QString::fromLatin1("localName3"),
                                       QString::fromLatin1("http://example.com/Namespace2")));

    const QXmlName n5(query.createName(QString::fromLatin1("localName4"),
                                       QString::fromLatin1("http://example.com/Namespace2")));

    const QXmlName n6(query.createName(QString::fromLatin1("localName4"),
                                       QString::fromLatin1("http://example.com/Namespace2"),
                                       QString::fromLatin1("prefix3")));

    const QXmlName n7(query.createName(QString::fromLatin1("localName2"),
                                       QString::fromLatin1("http://example.com/Namespace2"),
                                       QString::fromLatin1("prefix3")));

    QTest::newRow(qPrintable(n1.toClarkName(query)))
        << n1
        << n1
        << true;

    QTest::newRow(qPrintable(n2.toClarkName(query)))
        << n2
        << n2
        << true;

    QTest::newRow(qPrintable(n3.toClarkName(query)))
        << n3
        << n3
        << true;

    QTest::newRow(qPrintable(n4.toClarkName(query)))
        << n4
        << n4
        << true;

    QTest::newRow(qPrintable(n5.toClarkName(query)))
        << n5
        << n5
        << true;

    QTest::newRow(qPrintable(n6.toClarkName(query)))
        << n6
        << n6
        << true;

    QTest::newRow(qPrintable(n7.toClarkName(query)))
        << n7
        << n7
        << true;

    QTest::newRow("Prefix differs")
        << n2
        << n3
        << true;

    QTest::newRow("No prefix vs. prefix")
        << n5
        << n6
        << true;

    QTest::newRow("Local name differs")
        << n1
        << n2
        << false;

    QTest::newRow("Namespace differs")
        << n2
        << n7
        << false;
}

void tst_QXmlName::operatorNotEqual() const
{
    QFETCH(QXmlName, op1);
    QFETCH(QXmlName, op2);
    QFETCH(bool, expected);
    
    QCOMPARE(op1 != op2, !expected);
}

void tst_QXmlName::operatorNotEqual_data() const
{
    operatorEqual_data();
}

/*!
  Check that functions have the correct const qualification.
 */
void tst_QXmlName::constCorrectness() const
{
    const QXmlName name;

    /* isNull() */
    QVERIFY(name.isNull());

    /* operator==() */
    QVERIFY(name == name);

    /* operator!=() */
    QVERIFY(!(name != name));

    QXmlQuery query;
    const QXmlName name2(query.createName(QLatin1String("localName"), QLatin1String("http://example.com/"), QLatin1String("prefix")));

    /* namespaceUri(). */
    QCOMPARE(name2.namespaceUri(query), QLatin1String("http://example.com/"));

    /* localName(). */
    QCOMPARE(name2.localName(query), QLatin1String("localName"));

    /* prefix(). */
    QCOMPARE(name2.prefix(query), QLatin1String("prefix"));

    /* toClarkname(). */
    QCOMPARE(name2.toClarkName(query), QLatin1String("{http://example.com/}prefix:localName"));
}

void tst_QXmlName::qHash() const
{
    /* Just call it, so we know it exist and that we don't trigger undefined
     * behavior. We can't test the return value, since it's opaque. */
    QXmlName name;
    ::qHash(name);
}

QTEST_MAIN(tst_QXmlName)

#include "tst_qxmlname.moc"
// vim: et:ts=4:sw=4:sts=4

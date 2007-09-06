/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include "qsourcelocation.h"

/*!
 \class tst_QSourceLocation
 \internal
 \since 4.4
 \brief Tests QSourceLocation

 */
class tst_QSourceLocation : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void isNull() const;
    void defaultConstructor() const;
    void valueConstructor() const;
    void copyConstructor() const;
    void assignmentOperator() const;
    void equalnessOperator() const;
    void equalnessOperator_data() const;
    void defaultValues() const;
    void constCorrectness() const;
    void setLine() const;
    void setColumn() const;
    void setUri() const;
};

/*!
  We allocate a couple to catch reference counting bugs.
 */
void tst_QSourceLocation::defaultConstructor() const
{
    QSourceLocation def1;
    QSourceLocation def2;
    QSourceLocation def3;
}

void tst_QSourceLocation::copyConstructor() const
{
    {
        QSourceLocation def;
        QSourceLocation copy(def);

        QCOMPARE(def.line(), qint64(-1));
        QCOMPARE(def.column(), qint64(-1));
        QCOMPARE(def.uri(), QUrl());
    }

    {
        QSourceLocation val;
        val.setLine(5);
        val.setColumn(600);
        val.setUri(QUrl(QLatin1String("http://example.com/")));

        QSourceLocation copy(val);
        QCOMPARE(copy.line(), qint64(5));
        QCOMPARE(copy.column(), qint64(600));
        QCOMPARE(copy.uri(), QUrl(QLatin1String("http://example.com/")));
    }

    {
        /* Construct from a const object. */
        const QSourceLocation val;
        const QSourceLocation val2(val);
    }
}

void tst_QSourceLocation::valueConstructor() const
{
    const QSourceLocation sl(QUrl(QLatin1String("http://example.com/")), 5, 4);

    QCOMPARE(sl.uri(), QUrl(QLatin1String("http://example.com/")));
    QCOMPARE(sl.line(), qint64(5));
    QCOMPARE(sl.column(), qint64(4));
}

void tst_QSourceLocation::assignmentOperator() const
{
    /* Assign to self. */
    {
        QSourceLocation def;

        def = def;

        QVERIFY(def.isNull());
        QCOMPARE(def.line(), qint64(-1));
        QCOMPARE(def.column(), qint64(-1));
        QCOMPARE(def.uri(), QUrl());
    }

    /* Assign to default constructed object. */
    {
        QSourceLocation val;
        val.setLine(3);
        val.setColumn(4);
        val.setUri(QUrl(QLatin1String("http://example.com/2")));

        QSourceLocation assigned;
        assigned = val;

        QCOMPARE(assigned.line(), qint64(3));
        QCOMPARE(assigned.column(), qint64(4));
        QCOMPARE(assigned.uri(), QUrl(QLatin1String("http://example.com/2")));
    }

    /* Assign to modified object. */
    {
        QSourceLocation val;
        val.setLine(3);
        val.setColumn(4);
        val.setUri(QUrl(QLatin1String("http://example.com/2")));

        QSourceLocation assigned;
        assigned.setLine(700);
        assigned.setColumn(4000);
        assigned.setUri(QUrl(QLatin1String("http://example.com/3")));

        assigned = val;

        QCOMPARE(assigned.line(), qint64(3));
        QCOMPARE(assigned.column(), qint64(4));
        QCOMPARE(assigned.uri(), QUrl(QLatin1String("http://example.com/2")));
    }
}

Q_DECLARE_METATYPE(QSourceLocation)

void tst_QSourceLocation::equalnessOperator() const
{
    QFETCH(QSourceLocation, v1);
    QFETCH(QSourceLocation, v2);
    QFETCH(bool, True);

    QCOMPARE(v1 == v2, True);
    QCOMPARE(v1 != v2, True);
}

void tst_QSourceLocation::equalnessOperator_data() const
{
    QTest::addColumn<QSourceLocation>("v1");
    QTest::addColumn<QSourceLocation>("v2");
    QTest::addColumn<bool>("True");

    {
        QTest::newRow("Default constructed values")
                << QSourceLocation()
                << QSourceLocation()
                << true;
    }

    {
        QSourceLocation modified;
        modified.setColumn(4);

        QTest::newRow("Default constructed, against column-modified")
            << QSourceLocation()
            << modified
            << false;
    }

    {
        QSourceLocation modified;
        modified.setLine(5);

        QTest::newRow("Default constructed, against line-modified")
            << QSourceLocation()
            << modified
            << false;
    }

    {
        QSourceLocation modified;
        modified.setUri(QUrl(QLatin1String("http://example.com/")));

        QTest::newRow("Default constructed, against line-modified")
            << QSourceLocation()
            << modified
            << false;
    }

    {
        QSourceLocation modified;
        modified.setUri(QUrl(QLatin1String("http://example.com/")));
        modified.setLine(5);
        modified.setColumn(4);

        QTest::newRow("Default constructed, against all-modified")
            << QSourceLocation()
            << modified
            << false;
    }
}

void tst_QSourceLocation::defaultValues() const
{
    QSourceLocation def;

    QCOMPARE(def.line(), qint64(-1));
    QCOMPARE(def.column(), qint64(-1));
    QCOMPARE(def.uri(), QUrl());
}

/*!
  Call functions that must be const.
 */
void tst_QSourceLocation::constCorrectness() const
{
    const QSourceLocation def;

    def.line();
    def.column();
    def.uri();
    def.isNull();

    const QSourceLocation def2;

    /* Equalness operator. */
    QVERIFY(def == def2);
    QCOMPARE(def, def2);

    /* Inverse equalness operator. */
    QVERIFY(def != def2);
}

void tst_QSourceLocation::isNull() const
{
    {
        QSourceLocation def;
        QVERIFY(def.isNull());

        def.setColumn(4);
        QVERIFY(def.isNull());
    }

    {
        QSourceLocation def2;
        def2.setLine(4);
        QVERIFY(def2.isNull());
    }

    {
        QSourceLocation def3;
        def3.setUri(QUrl(QLatin1String("http://example.com/")));
        QVERIFY(!def3.isNull());
    }
}

void tst_QSourceLocation::setLine() const
{
    QSourceLocation sl;
    sl.setLine(8);
    QCOMPARE(sl.line(), qint64(8));
}

void tst_QSourceLocation::setColumn() const
{
    QSourceLocation sl;
    sl.setColumn(5);
    QCOMPARE(sl.column(), qint64(5));
}

void tst_QSourceLocation::setUri() const
{
    QSourceLocation sl;
    sl.setUri(QUrl(QLatin1String("http://example.com/")));
    QCOMPARE(sl.uri(), QUrl(QLatin1String("http://example.com/")));
}

// TODO How do we test the debug stream?

QTEST_MAIN(tst_QSourceLocation)

#include "tst_qsourcelocation.moc"
// vim: et:ts=4:sw=4:sts=4

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtTest/QtTest>

#include <QDateTime>

/*!
  \internal
 */
class tst_DateTime: public QObject
{
    Q_OBJECT

private slots:
    void dateTime() const;
    void qurl() const;
    void qurl_data() const;
};

void tst_DateTime::dateTime() const
{
    const QDateTime base(QDate(2000, 5, 3), QTime(4, 3, 4));
    const QDateTime local(base.toLocalTime());
    QDateTime utc(base.toUTC());
    utc.setDate(utc.date().addDays(1));

    QCOMPARE(local, utc);
}

void tst_DateTime::qurl() const
{
    QFETCH(QUrl, operandA);
    QFETCH(QUrl, operandB);
    
    QCOMPARE(operandA, operandB);
}

void tst_DateTime::qurl_data() const
{
    QTest::addColumn<QUrl>("operandA");
    QTest::addColumn<QUrl>("operandB");

    QTest::newRow("") << QUrl() << QUrl();
    QTest::newRow("") << QUrl(QLatin1String("http://example.com")) << QUrl();
    QTest::newRow("") << QUrl() << QUrl(QLatin1String("http://example.com"));
    QTest::newRow("") << QUrl(QLatin1String("http://example.com")) << QUrl(QLatin1String("http://example.com"));
}

QTEST_MAIN(tst_DateTime)

#include "tst_datetime.moc"

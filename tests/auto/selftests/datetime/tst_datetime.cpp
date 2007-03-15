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
};

void tst_DateTime::dateTime() const
{
    const QDateTime base(QDate(2000, 5, 3), QTime(4, 3, 4));
    const QDateTime local(base.toLocalTime());
    QDateTime utc(base.toUTC());
    utc.setDate(utc.date().addDays(1));

    QCOMPARE(local, utc);
}

QTEST_MAIN(tst_DateTime)

#include "tst_datetime.moc"

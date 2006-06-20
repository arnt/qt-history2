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

class tst_FetchBogus: public QObject
{
    Q_OBJECT

private slots:
    void fetchBogus_data();
    void fetchBogus();
};

void tst_FetchBogus::fetchBogus_data()
{
    QTest::addColumn<QString>("string");
    QTest::newRow("foo") << QString("blah");
}

void tst_FetchBogus::fetchBogus()
{
    QFETCH(QString, bubu);
}

QTEST_MAIN(tst_FetchBogus)

#include "tst_fetchbogus.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifdef QT_NO_SOLUTIONS
QTEST_NOOP_MAIN
#else

#include "qtmd5.h"

class tst_QtMD5: public QObject
{
    Q_OBJECT

private slots:
    void computeMD5_data();
    void computeMD5();
};

void tst_QtMD5::computeMD5_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QString>("expected");

    QTest::newRow("data0") << QByteArray("") << QString("d41d8cd98f00b204e9800998ecf8427e");
    QTest::newRow("data1") << QByteArray("a") << QString("0cc175b9c0f1b6a831c399e269772661");
    QTest::newRow("data2") << QByteArray("abc") << QString("900150983cd24fb0d6963f7d28e17f72");
    QTest::newRow("data3") << QByteArray("message digest") << QString("f96b697d7cb7938d525a2f31aaf161d0");
    QTest::newRow("data4") << QByteArray("abcdefghijklmnopqrstuvwxyz") << QString("c3fcd3d76192e4007dfb496cca67e13b");
    QTest::newRow("data5") << QByteArray("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") << QString("d174ab98d277d9f5a5611c2c9f419d9f");
    QTest::newRow("data6") << QByteArray("12345678901234567890123456789012345678901234567890123456789012345678901234567890") << QString("57edf4a22be3c955ac49da2e2107b67a");
}

void tst_QtMD5::computeMD5()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    QCOMPARE(qtMD5(data), expected);
}

QTEST_MAIN(tst_QtMD5)
#include "tst_qtmd5.moc"
#endif

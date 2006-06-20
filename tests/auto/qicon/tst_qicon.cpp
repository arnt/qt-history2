/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qicon.h>

Q_DECLARE_METATYPE(QSize)

//TESTED_CLASS=QIcon
//TESTED_FILES=gui/image/qicon.h gui/image/qicon.cpp

class tst_QIcon : public QObject
{
    Q_OBJECT
public:
    tst_QIcon();
private slots:
    void actualSize_data(); // test with 1 pixmap
    void actualSize();
    void actualSize2_data(); // test with 2 pixmaps with different aspect ratio
    void actualSize2();
    void isNull();
};

tst_QIcon::tst_QIcon()
{
}

void tst_QIcon::actualSize_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<QSize>("argument");
    QTest::addColumn<QSize>("result");

    // square image
    QTest::newRow("resource0") << ":/image.png" << QSize(128, 128) << QSize(128, 128);
    QTest::newRow("resource1") << ":/image.png" << QSize( 64,  64) << QSize( 64,  64);
    QTest::newRow("resource2") << ":/image.png" << QSize( 32,  64) << QSize( 32,  32);
    QTest::newRow("resource3") << ":/image.png" << QSize( 16,  64) << QSize( 16,  16);
    QTest::newRow("resource4") << ":/image.png" << QSize( 16,  128) << QSize( 16,  16);
    QTest::newRow("resource5") << ":/image.png" << QSize( 128,  16) << QSize( 16,  16);
    QTest::newRow("resource6") << ":/image.png" << QSize( 150,  150) << QSize( 128,  128);
    // rect image
    QTest::newRow("resource7") << ":/rect.png" << QSize( 20,  40) << QSize( 20,  40);
    QTest::newRow("resource8") << ":/rect.png" << QSize( 10,  20) << QSize( 10,  20);
    QTest::newRow("resource9") << ":/rect.png" << QSize( 15,  50) << QSize( 15,  30);
    QTest::newRow("resource10") << ":/rect.png" << QSize( 25,  50) << QSize( 20,  40);

    QTest::newRow("external0") << "image.png" << QSize(128, 128) << QSize(128, 128);
    QTest::newRow("external1") << "image.png" << QSize( 64,  64) << QSize( 64,  64);
    QTest::newRow("external2") << "image.png" << QSize( 32,  64) << QSize( 32,  32);
    QTest::newRow("external3") << "image.png" << QSize( 16,  64) << QSize( 16,  16);
    QTest::newRow("external4") << "image.png" << QSize( 16,  128) << QSize( 16,  16);
    QTest::newRow("external5") << "image.png" << QSize( 128,  16) << QSize( 16,  16);
    QTest::newRow("external6") << "image.png" << QSize( 150,  150) << QSize( 128,  128);
    // rect image
    QTest::newRow("external7") << ":/rect.png" << QSize( 20,  40) << QSize( 20,  40);
    QTest::newRow("external8") << ":/rect.png" << QSize( 10,  20) << QSize( 10,  20);
    QTest::newRow("external9") << ":/rect.png" << QSize( 15,  50) << QSize( 15,  30);
    QTest::newRow("external10") << ":/rect.png" << QSize( 25,  50) << QSize( 20,  40);

}

void tst_QIcon::actualSize()
{
    QFETCH(QString, source);
    QFETCH(QSize, argument);
    QFETCH(QSize, result);

    {
        QPixmap pixmap(source);
        QIcon icon(pixmap);
        QCOMPARE(icon.actualSize(argument), result);
        QCOMPARE(icon.pixmap(argument).size(), result);
    }

    {
        QIcon icon(source);
        QCOMPARE(icon.actualSize(argument), result);
        QCOMPARE(icon.pixmap(argument).size(), result);
    }
}

void tst_QIcon::actualSize2_data()
{
    QTest::addColumn<QSize>("argument");
    QTest::addColumn<QSize>("result");

    // two images - 128x128 and 20x40. Let the games begin
    QTest::newRow("trivial1") << QSize( 128,  128) << QSize( 128,  128);
    QTest::newRow("trivial2") << QSize( 20,  40) << QSize( 20,  40);

    // QIcon chooses the one with the smallest area to choose the pixmap
    QTest::newRow("best1") << QSize( 100,  100) << QSize( 100,  100);
    QTest::newRow("best2") << QSize( 20,  20) << QSize( 10,  20);
    QTest::newRow("best3") << QSize( 15,  30) << QSize( 15,  30);
    QTest::newRow("best4") << QSize( 5,  5) << QSize( 2,  5);
    QTest::newRow("best5") << QSize( 10,  15) << QSize( 7,  15);
}

void tst_QIcon::actualSize2()
{
    QIcon icon;
    icon.addPixmap(QPixmap("image.png"));
    icon.addPixmap(QPixmap("rect.png"));

    QFETCH(QSize, argument);
    QFETCH(QSize, result);

#if QT_VERSION >= 0x040200
    QCOMPARE(icon.actualSize(argument), result);
    QCOMPARE(icon.pixmap(argument).size(), result);
#endif
}


void tst_QIcon::isNull() {
    // test default constructor
    QIcon defaultConstructor;
    QVERIFY(defaultConstructor.isNull());

    // test copy constructor
    QVERIFY(QIcon(defaultConstructor).isNull());

    // test pixmap constructor
    QPixmap nullPixmap;
    QVERIFY(QIcon(nullPixmap).isNull());

    // test string constructor with empty string
    QIcon iconEmptyString = QIcon(QString());
    QVERIFY(iconEmptyString.isNull());
    QVERIFY(!iconEmptyString.actualSize(QSize(32, 32)).isValid());;

    // test string constructor with non-existing file
    QIcon iconNoFile = QIcon("imagedoesnotexist");
    QVERIFY(!iconNoFile.isNull());
    QVERIFY(!iconNoFile.actualSize(QSize(32, 32)).isValid());

    // test string constructor with non-existing file with suffix
    QIcon iconNoFileSuffix = QIcon("imagedoesnotexist.png");
    QVERIFY(!iconNoFileSuffix.isNull());
    QVERIFY(!iconNoFileSuffix.actualSize(QSize(32, 32)).isValid());

    // test string constructor with existing file but unsupported format
    QIcon iconUnsupportedFormat = QIcon("image.tga");
    QVERIFY(!iconUnsupportedFormat.isNull());
    QVERIFY(!iconUnsupportedFormat.actualSize(QSize(32, 32)).isValid());

    // test string constructor with existing file and supported format
    QIcon iconSupportedFormat = QIcon("image.png");
    QVERIFY(!iconSupportedFormat.isNull());
    QVERIFY(iconSupportedFormat.actualSize(QSize(32, 32)).isValid());
}

QTEST_MAIN(tst_QIcon)
#include "tst_qicon.moc"

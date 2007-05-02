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
    void bestMatch();
    void cacheKey();
    void detach();
    void operatorEqual() const;
    void operatorEqual_data() const;

    void operatorNotEqual() const;
    void operatorNotEqual_data() const;
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

    const QString prefix = QLatin1String(SRCDIR) + QLatin1String("/");

    QTest::newRow("external0") << prefix + "image.png" << QSize(128, 128) << QSize(128, 128);
    QTest::newRow("external1") << prefix + "image.png" << QSize( 64,  64) << QSize( 64,  64);
    QTest::newRow("external2") << prefix + "image.png" << QSize( 32,  64) << QSize( 32,  32);
    QTest::newRow("external3") << prefix + "image.png" << QSize( 16,  64) << QSize( 16,  16);
    QTest::newRow("external4") << prefix + "image.png" << QSize( 16,  128) << QSize( 16,  16);
    QTest::newRow("external5") << prefix + "image.png" << QSize( 128,  16) << QSize( 16,  16);
    QTest::newRow("external6") << prefix + "image.png" << QSize( 150,  150) << QSize( 128,  128);
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

    const QString prefix = QLatin1String(SRCDIR) + QLatin1String("/");

    icon.addPixmap(QPixmap(prefix + "image.png"));
    icon.addPixmap(QPixmap(prefix + "rect.png"));

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

    const QString prefix = QLatin1String(SRCDIR) + QLatin1String("/");

    // test string constructor with existing file but unsupported format
    QIcon iconUnsupportedFormat = QIcon(prefix + "image.tga");
    QVERIFY(!iconUnsupportedFormat.isNull());
    QVERIFY(!iconUnsupportedFormat.actualSize(QSize(32, 32)).isValid());

    // test string constructor with existing file and supported format
    QIcon iconSupportedFormat = QIcon(prefix + "image.png");
    QVERIFY(!iconSupportedFormat.isNull());
    QVERIFY(iconSupportedFormat.actualSize(QSize(32, 32)).isValid());
}

void tst_QIcon::bestMatch()
{
    QPixmap p1(1, 1);
    QPixmap p2(2, 2);
    QPixmap p3(3, 3);
    QPixmap p4(4, 4);
    QPixmap p5(5, 5);
    QPixmap p6(6, 6);
    QPixmap p7(7, 7);
    QPixmap p8(8, 8);

    p1.fill(Qt::black);
    p2.fill(Qt::black);
    p3.fill(Qt::black);
    p4.fill(Qt::black);
    p5.fill(Qt::black);
    p6.fill(Qt::black);
    p7.fill(Qt::black);
    p8.fill(Qt::black);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            QIcon::State state = (j == 0) ? QIcon::On : QIcon::Off;
            QIcon::State oppositeState = (state == QIcon::On) ? QIcon::Off
                                                              : QIcon::On;
            QIcon::Mode mode;
            QIcon::Mode oppositeMode;

            QIcon icon;

            switch (i) {
            case 0:
            default:
                mode = QIcon::Normal;
                oppositeMode = QIcon::Active;
                break;
            case 1:
                mode = QIcon::Active;
                oppositeMode = QIcon::Normal;
                break;
            case 2:
                mode = QIcon::Disabled;
                oppositeMode = QIcon::Selected;
                break;
            case 3:
                mode = QIcon::Selected;
                oppositeMode = QIcon::Disabled;
            }

            /*
                The test mirrors the code in
                QPixmapIconEngine::bestMatch(), to make sure that
                nobody breaks QPixmapIconEngine by mistake. Before
                you change this test or the code that it tests,
                please talk to Jasmin if possible.
            */
            if (mode == QIcon::Disabled || mode == QIcon::Selected) {
                icon.addPixmap(p1, oppositeMode, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p1.size());

                icon.addPixmap(p2, oppositeMode, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p2.size());

                icon.addPixmap(p3, QIcon::Active, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p3.size());

                icon.addPixmap(p4, QIcon::Normal, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p4.size());

                icon.addPixmap(p5, mode, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p5.size());

                icon.addPixmap(p6, QIcon::Active, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p6.size());

                icon.addPixmap(p7, QIcon::Normal, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p7.size());

                icon.addPixmap(p8, mode, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p8.size());
            } else {
                icon.addPixmap(p1, QIcon::Selected, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p1.size());

                icon.addPixmap(p2, QIcon::Disabled, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p2.size());

                icon.addPixmap(p3, QIcon::Selected, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p3.size());

                icon.addPixmap(p4, QIcon::Disabled, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p4.size());

                icon.addPixmap(p5, oppositeMode, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p5.size());

                icon.addPixmap(p6, mode, oppositeState);
                QVERIFY(icon.pixmap(100, mode, state).size() == p6.size());

                icon.addPixmap(p7, oppositeMode, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p7.size());

                icon.addPixmap(p8, mode, state);
                QVERIFY(icon.pixmap(100, mode, state).size() == p8.size());
            }
        }
    }
}

void tst_QIcon::cacheKey()
{
    QIcon icon1("image.png");
    qint64 icon1_key = icon1.cacheKey();
    QIcon icon2 = icon1;

    QVERIFY(icon2.cacheKey() == icon1.cacheKey());
    icon2.detach();
    QVERIFY(icon2.cacheKey() != icon1.cacheKey());
    QVERIFY(icon1.cacheKey() == icon1_key);
}

void tst_QIcon::detach()
{
    QImage img(32, 32, QImage::Format_ARGB32_Premultiplied);
    img.fill(0xffff0000);
    QIcon icon1(QPixmap::fromImage(img));
    QIcon icon2 = icon1;
    icon2.addFile("image.png", QSize(64, 64));

    QImage img1 = icon1.pixmap(64, 64).toImage();
    QImage img2 = icon2.pixmap(64, 64).toImage();
    QVERIFY(img1 != img2);

    img1 = icon1.pixmap(32, 32).toImage();
    img2 = icon2.pixmap(32, 32).toImage();
    QVERIFY(img1 == img2);
}

void tst_QIcon::operatorEqual() const
{
    QFETCH(QIcon, operandA);
    QFETCH(QIcon, operandB);
    QFETCH(bool, result);

    QCOMPARE(operandA == operandB, result);
}

void tst_QIcon::operatorEqual_data() const
{
    QImage img(32, 32, QImage::Format_ARGB32_Premultiplied);
    img.fill(0xffff0000);
    const QIcon fromImage(QPixmap::fromImage(img));
    QVERIFY(!fromImage.isNull()); // Check that we're actually populated.

    QTest::addColumn<QIcon>("operandA");
    QTest::addColumn<QIcon>("operandB");
    QTest::addColumn<bool>("result");

    QTest::newRow("Both are null") << QIcon() << QIcon() << true;
    QTest::newRow("Left is null") << QIcon() << fromImage << false;
    QTest::newRow("Right is null") << fromImage << QIcon() << false;

    QTest::newRow("Two identical, filled") << fromImage << fromImage << true;

    const QIcon fromFile("image.png");
    QVERIFY(!fromFile.isNull()); // Check that loading the file didn't fail.

    QTest::newRow("Two from same file") << fromFile << fromFile << true;
    QTest::newRow("Different icons") << fromFile << fromImage << false;
    QTest::newRow("Different icons") << fromImage << fromFile << false;
}

/*!
  \internal
  The exact same code as for operatorEqual(), but we simply negate the result.
 */
void tst_QIcon::operatorNotEqual() const
{
    QFETCH(QIcon, operandA);
    QFETCH(QIcon, operandB);
    QFETCH(bool, result);

    QCOMPARE(operandA != operandB, !result);
}

void tst_QIcon::operatorNotEqual_data() const
{
    operatorEqual_data();
}

QTEST_MAIN(tst_QIcon)
#include "tst_qicon.moc"

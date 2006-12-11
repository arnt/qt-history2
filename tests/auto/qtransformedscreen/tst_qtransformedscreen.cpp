/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifdef Q_WS_QWS

#include <qwindowsystem_qws.h>
#include <qscreen_qws.h>
#include <qscreendriverfactory_qws.h>
#include <qscreentransformed_qws.h>

class tst_QTransformedScreen : public QObject
{
    Q_OBJECT

public:
    tst_QTransformedScreen() {}
    ~tst_QTransformedScreen() { }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void setTransformation_data();
    void setTransformation();

private:
    QTransformedScreen *screen;
    QScreen *oldScreen;
    int id;
};

Q_DECLARE_METATYPE(QTransformedScreen::Transformation);

void tst_QTransformedScreen::initTestCase()
{
    oldScreen = qt_screen;

    QVERIFY(QScreenDriverFactory::keys().contains(QLatin1String("Transformed")));
    QVERIFY(QScreenDriverFactory::keys().contains(QLatin1String("VNC")));

    id = 10;
    screen = static_cast<QTransformedScreen*>(QScreenDriverFactory::create("Transformed", id));
    QVERIFY(screen);
    QVERIFY(screen->connect(QString("Transformed:Rot90:VNC:%1").arg(id)));
    QVERIFY(screen->initDevice());
}

void tst_QTransformedScreen::cleanupTestCase()
{
    screen->shutdownDevice();
    screen->disconnect();
    delete screen;
    screen = 0;

    qt_screen = oldScreen;
}

void tst_QTransformedScreen::setTransformation_data()
{
    QTest::addColumn<QTransformedScreen::Transformation>("transformation");
    QTest::addColumn<bool>("swap");

    QTest::newRow("Rot0") << QTransformedScreen::None << false;
    QTest::newRow("Rot90") << QTransformedScreen::Rot90 << true;
    QTest::newRow("Rot180") << QTransformedScreen::Rot180 << false;
    QTest::newRow("Rot270") << QTransformedScreen::Rot270 << true;
}

void tst_QTransformedScreen::setTransformation()
{
    // Not really failures but equal height and width makes this test useless
    QVERIFY(screen->deviceWidth() != screen->deviceHeight());

    screen->setTransformation(QTransformedScreen::None);
    int dw = screen->deviceWidth();
    int dh = screen->deviceHeight();
    int mmw = screen->physicalWidth();
    int mmh = screen->physicalHeight();

    QFETCH(QTransformedScreen::Transformation, transformation);
    QFETCH(bool, swap);

    screen->setTransformation(transformation);
    QCOMPARE(screen->deviceWidth(), dw);
    QCOMPARE(screen->deviceHeight(), dh);

    if (swap) {
        QCOMPARE(screen->width(), dh);
        QCOMPARE(screen->height(), dw);
        QCOMPARE(screen->physicalWidth(), mmh);
        QCOMPARE(screen->physicalHeight(), mmw);
    } else {
        QCOMPARE(screen->width(), dw);
        QCOMPARE(screen->height(), dh);
        QCOMPARE(screen->physicalWidth(), mmw);
        QCOMPARE(screen->physicalHeight(), mmh);
    }
}

QTEST_MAIN(tst_QTransformedScreen)

#include "tst_qtransformedscreen.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

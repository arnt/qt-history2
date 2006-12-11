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

class tst_QTransformedScreen : public QObject
{
    Q_OBJECT

public:
    tst_QTransformedScreen() {}
    ~tst_QTransformedScreen() { }

private slots:
    void initTestCase();
    void cleanupTestCase();

private:
    QScreen *screen;
    QScreen *oldScreen;
    int id;
};

void tst_QTransformedScreen::initTestCase()
{
    oldScreen = qt_screen;

    QVERIFY(QScreenDriverFactory::keys().contains(QLatin1String("Transformed")));
    QVERIFY(QScreenDriverFactory::keys().contains(QLatin1String("VNC")));

    id = 10;
    screen = QScreenDriverFactory::create("Transformed", id);
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

QTEST_MAIN(tst_QTransformedScreen)

#include "tst_qtransformedscreen.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

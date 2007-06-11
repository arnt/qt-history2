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

#include <qdesktopwidget.h>
#include <qscreen_qws.h>
#include <qscreendriverfactory_qws.h>

class tst_QMultiScreen : public QObject
{
    Q_OBJECT

public:
    tst_QMultiScreen() : screen(0), oldScreen(0) {}
    ~tst_QMultiScreen() {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void widgetSetFixedSize();

private:
    QScreen *screen;
    QScreen *oldScreen;
};

void tst_QMultiScreen::cleanupTestCase()
{
    screen->shutdownDevice();
    screen->disconnect();
    delete screen;
    screen = 0;

    qt_screen = oldScreen;
}

void tst_QMultiScreen::initTestCase()
{
    oldScreen = qt_screen;

    QVERIFY(QScreenDriverFactory::keys().contains(QLatin1String("Multi")));
    QVERIFY(QScreenDriverFactory::keys().contains(QLatin1String("VNC")));

    const int id = 10;
    screen = QScreenDriverFactory::create("Multi", id);
    QVERIFY(screen);
    QVERIFY(screen->connect(QString("Multi: "
                                    "VNC:size=640x480:offset=0,0:%1 "
                                    "VNC:size=640x480:offset=640,0:%2 "
                                    "VNC:size=640x480:offset=0,480:%3 "
                                    ":%4")
                            .arg(id+1).arg(id+2).arg(id+3).arg(id)));
    QVERIFY(screen->initDevice());

    QDesktopWidget desktop;
    QCOMPARE(desktop.numScreens(), 3);
}

void tst_QMultiScreen::widgetSetFixedSize()
{
    QDesktopWidget desktop;
    QRect maxRect;
    for (int i = 0; i < desktop.numScreens(); ++i)
        maxRect |= desktop.availableGeometry(i);

    maxRect = maxRect.adjusted(50, 50, -50, -50);

    // make sure we can set a size larger than a single screen (task 166368)
    QWidget w;
    w.setFixedSize(maxRect.size());
    w.show();
    QApplication::processEvents();
    QCOMPARE(w.geometry().size(), maxRect.size());
}

QTEST_MAIN(tst_QMultiScreen)

#include "tst_qmultiscreen.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

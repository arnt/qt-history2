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
#include <qlabel.h>

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
    void grabWindow();

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
                                    "VNC:size=640x480:depth=32:offset=0,0:%1 "
                                    "VNC:size=640x480:depth=32:offset=640,0:%2 "
                                    "VNC:size=640x480:depth=16:offset=0,480:%3 "
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

void tst_QMultiScreen::grabWindow()
{
    QDesktopWidget desktop;

    QVERIFY(desktop.numScreens() >= 2);

    const QRect r0 = desktop.availableGeometry(0).adjusted(50, 50, -50, -50);
    const QRect r1 = desktop.availableGeometry(1).adjusted(60, 60, -60, -60);

    QWidget w;
    w.setGeometry(r0);
    w.show();

    QLabel l("hi there");
    l.setGeometry(r1);
    l.show();

    QApplication::processEvents();
    QVERIFY(desktop.screenNumber(&w) == 0);
    QVERIFY(desktop.screenNumber(&l) == 1);

    const QPixmap p0 = QPixmap::grabWindow(w.winId());
    const QPixmap p1 = QPixmap::grabWindow(l.winId());

//     p0.save("w.png", "PNG");
//     p1.save("l.png", "PNG");
    QCOMPARE(p0.size(), w.size());
    QCOMPARE(p1.size(), l.size());

    const QImage img0 = p0.toImage();
    const QImage img1 = p1.toImage();

//     QPixmap::grabWidget(&w).toImage().convertToFormat(img0.format()).save("w_img.png", "PNG");
//     QPixmap::grabWidget(&l).toImage().convertToFormat(img1.format()).save("l_img.png", "PNG");

    QImage::Format format = QImage::Format_RGB16;
    QCOMPARE(img0.convertToFormat(format),
             QPixmap::grabWidget(&w).toImage().convertToFormat(format));
    QCOMPARE(img1.convertToFormat(format),
             QPixmap::grabWidget(&l).toImage().convertToFormat(format));
}

QTEST_MAIN(tst_QMultiScreen)

#include "tst_qmultiscreen.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

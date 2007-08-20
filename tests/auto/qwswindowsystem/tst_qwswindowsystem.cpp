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

//TESTED_CLASS=
//TESTED_FILES=gui/embedded/qwindowsystem_qws.h gui/embedded/qwindowsystem_qws.cpp

#include <qwindowsystem_qws.h>
#include <qpainter.h>
#include <qdesktopwidget.h>
#include <qdirectpainter_qws.h>
#include <qscreen_qws.h>
#include <private/qwindowsurface_qws_p.h>

class tst_QWSWindowSystem : public QObject
{
    Q_OBJECT

public:
    tst_QWSWindowSystem() {}
    ~tst_QWSWindowSystem() {}

private slots:
    void initTestCase();
    void showHideWindow();
    void raiseLowerWindow();
    void windowOpacity();
    void directPainter();
    void setMaxWindowRect();
    void initialGeometry();
    void WA_PaintOnScreen();
    void toplevelMove();

private:
    QWSWindow* getWindow(int windId);
    QColor bgColor;
};

class ColorWidget : public QWidget
{
public:
    ColorWidget(const QColor &color = QColor(Qt::red))
        : QWidget(0, Qt::FramelessWindowHint), c(color) {}

    QColor color() { return c; }

protected:
    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        p.fillRect(rect(), QBrush(c));
    }

private:
    QColor c;
};

void tst_QWSWindowSystem::initTestCase()
{
    bgColor = QColor(Qt::green);

    QWSServer *server = QWSServer::instance();
    server->setBackground(bgColor);
}

QWSWindow* tst_QWSWindowSystem::getWindow(int winId)
{
    QWSServer *server = QWSServer::instance();
    foreach (QWSWindow *w, server->clientWindows()) {
        if (w->winId() == winId)
            return w;
    }
    return 0;
}

#define VERIFY_COLOR(rect, color) {                                     \
    const QPixmap pixmap = QPixmap::grabWindow(QDesktopWidget().winId(), \
                                               rect.left(), rect.top(), \
                                               rect.width(), rect.height()); \
    QCOMPARE(pixmap.size(), rect.size());                               \
    QPixmap expectedPixmap(pixmap); /* ensure equal formats */          \
    expectedPixmap.fill(color);                                         \
    QCOMPARE(pixmap, expectedPixmap);                                   \
}

void tst_QWSWindowSystem::showHideWindow()
{
    ColorWidget w;

    const QRect rect(100, 100, 100, 100);

    w.setGeometry(rect);
    QApplication::processEvents();

    QWSWindow *win = getWindow(w.winId());
    QVERIFY(win);
    QCOMPARE(win->requestedRegion(), QRegion());
    QCOMPARE(win->allocatedRegion(), QRegion());
    VERIFY_COLOR(rect, bgColor);

    w.show();
    QApplication::processEvents();
    QCOMPARE(win->requestedRegion(), QRegion(rect));
    QCOMPARE(win->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w.color());

    w.hide();
    QApplication::processEvents();
    QCOMPARE(win->requestedRegion(), QRegion());
    QCOMPARE(win->allocatedRegion(), QRegion());
    VERIFY_COLOR(rect, bgColor);
}

void tst_QWSWindowSystem::raiseLowerWindow()
{
    const QRect rect(100, 100, 100, 100);

    ColorWidget w1(Qt::red);
    w1.setGeometry(rect);
    w1.show();
    QApplication::processEvents();

    ColorWidget w2(Qt::blue);
    w2.setGeometry(rect);
    w2.show();

    QWSWindow *win1 = getWindow(w1.winId());
    QWSWindow *win2 = getWindow(w2.winId());

    QApplication::processEvents();
    QCOMPARE(win1->requestedRegion(), QRegion(rect));
    QCOMPARE(win2->requestedRegion(), QRegion(rect));
    QCOMPARE(win1->allocatedRegion(), QRegion());
    QCOMPARE(win2->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w2.color());

    w1.raise();
    QApplication::processEvents();
    QCOMPARE(win1->requestedRegion(), QRegion(rect));
    QCOMPARE(win2->requestedRegion(), QRegion(rect));
    QCOMPARE(win1->allocatedRegion(), QRegion(rect));
    QCOMPARE(win2->allocatedRegion(), QRegion());
    VERIFY_COLOR(rect, w1.color());

    w1.lower();
    QApplication::processEvents();
    QCOMPARE(win1->requestedRegion(), QRegion(rect));
    QCOMPARE(win2->requestedRegion(), QRegion(rect));
    QCOMPARE(win1->allocatedRegion(), QRegion());
    QCOMPARE(win2->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w2.color());
}

void tst_QWSWindowSystem::windowOpacity()
{
    const QRect rect(100, 100, 100, 100);

    ColorWidget w1(Qt::red);
    w1.setGeometry(rect);
    w1.show();

    QWidget w2(0, Qt::FramelessWindowHint);
    w2.setGeometry(rect);
    w2.show();
    w2.raise();

    QWSWindow *win1 = getWindow(w1.winId());
    QWSWindow *win2 = getWindow(w2.winId());

    QApplication::processEvents();
    QCOMPARE(win1->allocatedRegion(), QRegion());
    QCOMPARE(win2->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w2.palette().color(w2.backgroundRole()));

    // Make w2 transparent so both widgets are shown.

    w2.setWindowOpacity(0.0);
    QApplication::processEvents();
    QCOMPARE(win1->allocatedRegion(), QRegion(rect));
    QCOMPARE(win2->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w1.color());

    w2.setWindowOpacity(1.0);
    QApplication::processEvents();
    QCOMPARE(win1->allocatedRegion(), QRegion());
    QCOMPARE(win2->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w2.palette().color(w2.backgroundRole()));

    // Use the palette to make w2 transparent
    QPalette palette = w2.palette();
    palette.setBrush(QPalette::All, QPalette::Background,
                     QColor(255, 255, 255, 0));
    w2.setPalette(palette);
    QApplication::processEvents();
    QCOMPARE(win1->allocatedRegion(), QRegion(rect));
    QCOMPARE(win2->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w1.color());

    palette.setBrush(QPalette::All, QPalette::Background,
                     QColor(255, 255, 255, 255));
    w2.setPalette(palette);
    QApplication::processEvents();
    QApplication::processEvents();
    QApplication::processEvents();
    QCOMPARE(win1->allocatedRegion(), QRegion());
    QCOMPARE(win2->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, QColor(255, 255, 255, 255));
}

void tst_QWSWindowSystem::directPainter()
{
    const QRect rect(100, 100, 100, 100);

    ColorWidget w(Qt::red);
    w.setGeometry(rect);
    w.show();

    QWSWindow *win = getWindow(w.winId());

    QApplication::processEvents();
    QCOMPARE(win->allocatedRegion(), QRegion(rect));

    // reserve screen area using the static functions

    QDirectPainter::reserveRegion(QRegion(rect));
    QApplication::processEvents();
    QCOMPARE(win->allocatedRegion(), QRegion());
    QCOMPARE(QDirectPainter::reservedRegion(), QRegion(rect));

    QDirectPainter::reserveRegion(QRegion());
    QApplication::processEvents();
    QCOMPARE(win->allocatedRegion(), QRegion(rect));
    QCOMPARE(QDirectPainter::reservedRegion(), QRegion());

    // reserve screen area using a QDirectPainter object
    {
        QDirectPainter dp;
        dp.setRegion(QRegion(rect));
        dp.lower();

        QWSWindow *dpWin = getWindow(dp.winId());

        QApplication::processEvents();
        QCOMPARE(win->allocatedRegion(), QRegion(rect));
        QCOMPARE(dpWin->allocatedRegion(), QRegion());

        w.lower();
        QApplication::processEvents();
        QCOMPARE(win->allocatedRegion(), QRegion());
        QCOMPARE(dpWin->allocatedRegion(), QRegion(rect));
    }

    QApplication::processEvents();
    QCOMPARE(win->allocatedRegion(), QRegion(rect));
    VERIFY_COLOR(rect, w.color());
}

void tst_QWSWindowSystem::setMaxWindowRect()
{
    QDesktopWidget desktop;
    const QRect screenRect = desktop.screenGeometry();

    QWidget w;
    w.showMaximized();
    QApplication::processEvents();

    QCOMPARE(w.frameGeometry(), screenRect);

    QRect available = QRect(screenRect.left(), screenRect.top(),
                            screenRect.right() + 1, screenRect.bottom() - 20 + 1);
    QWSServer::setMaxWindowRect(available);
    QApplication::processEvents();

    QCOMPARE(desktop.availableGeometry(), available);
    QCOMPARE(w.frameGeometry(), desktop.availableGeometry());

    w.hide();
    QApplication::processEvents();

    QWSServer::setMaxWindowRect(screenRect);
    w.show();
    QVERIFY(w.isMaximized());
    QCOMPARE(desktop.availableGeometry(), screenRect);
    QCOMPARE(w.frameGeometry(), desktop.availableGeometry());
}

void tst_QWSWindowSystem::initialGeometry()
{
    ColorWidget w(Qt::red);
    w.setGeometry(100, 0, 50, 50);
    w.show();

    const QRect rect(10, 200, 100, 100);
    w.setGeometry(rect);

    QApplication::processEvents();

    QCOMPARE(w.frameGeometry(), rect);
    VERIFY_COLOR(rect, QColor(Qt::red));
}

void tst_QWSWindowSystem::WA_PaintOnScreen()
{
    ColorWidget w(Qt::red);
    w.setAttribute(Qt::WA_PaintOnScreen);

    QRect rect;

    QVERIFY(w.testAttribute(Qt::WA_PaintOnScreen));
    rect = QRect(10, 0, 50, 50);
    w.setGeometry(rect);
    w.show();

    QApplication::processEvents();
    QWSWindowSurface *surface = static_cast<QWSWindowSurface*>(w.windowSurface());
    QCOMPARE(surface->key(), QLatin1String("OnScreen"));
    QVERIFY(w.testAttribute(Qt::WA_PaintOnScreen));
    VERIFY_COLOR(rect, QColor(Qt::red));

    // move
    rect = QRect(10, 100, 50, 50);
    w.setGeometry(rect);
    QApplication::processEvents();
    VERIFY_COLOR(rect, QColor(Qt::red));

    // resize
    rect = QRect(10, 100, 60, 60);
    w.setGeometry(rect);
    QApplication::processEvents();
    VERIFY_COLOR(rect, QColor(Qt::red));
}

class DummyMoveSurface : public QWSSharedMemSurface
{
public:
    DummyMoveSurface(QWidget *w) : QWSSharedMemSurface(w) {}
    DummyMoveSurface() : QWSSharedMemSurface() {}

    // doesn't do any move
    QRegion move(const QPoint &, const QRegion &) {
        return QRegion();
    }

    QString key() const { return QLatin1String("dummy"); }
};

class DummyScreen : public QScreen
{
private:
    QScreen *s;

public:

    DummyScreen() : QScreen(0), s(qt_screen) {
        qt_screen = this;
        w = s->width();
        h = s->height();
        dw = s->deviceWidth();
        dh = s->deviceHeight();
        d = s->depth();
        data = s->base();
        lstep = s->linestep();
        physWidth = s->physicalWidth();
        physHeight = s->physicalHeight();
        setPixelFormat(s->pixelFormat());
    }

    ~DummyScreen() {
        qt_screen = s;
    }

    bool initDevice() { return s->initDevice(); }
    bool connect(const QString &displaySpec) {
        return s->connect(displaySpec);
    }
    void disconnect() { s->disconnect(); }
    void setMode(int w, int h, int d) { s->setMode(w, h, d); }
    void exposeRegion(QRegion r, int changing) {
        s->exposeRegion(r, changing);
    }
    void blit(const QImage &img, const QPoint &topLeft, const QRegion &r) {
        s->blit(img, topLeft, r);
    }
    void solidFill(const QColor &color, const QRegion &region) {
        s->solidFill(color, region);
    }
    QWSWindowSurface* createSurface(const QString &key) const {
        if (key == QLatin1String("dummy"))
            return new DummyMoveSurface;
        return s->createSurface(key);
    }
};

void tst_QWSWindowSystem::toplevelMove()
{
    { // default move implementation, opaque window
        ColorWidget w(Qt::red);
        w.show();

        w.setGeometry(50, 50, 50, 50);
        QApplication::processEvents();
        VERIFY_COLOR(QRect(50, 50, 50, 50), w.color());
        VERIFY_COLOR(QRect(100, 100, 50, 50), bgColor);

        w.move(100, 100);
        QApplication::processEvents();

        VERIFY_COLOR(QRect(100, 100, 50, 50), w.color());
        VERIFY_COLOR(QRect(50, 50, 50, 50), bgColor);
    }

    { // default move implementation, non-opaque window
        ColorWidget w(Qt::red);
        w.setWindowOpacity(0.5);
        w.show();

        w.setGeometry(50, 50, 50, 50);
        QApplication::processEvents();
//        VERIFY_COLOR(QRect(50, 50, 50, 50), w.color());
        VERIFY_COLOR(QRect(100, 100, 50, 50), bgColor);

        w.move(100, 100);
        QApplication::processEvents();

//        VERIFY_COLOR(QRect(100, 100, 50, 50), w.color());
        VERIFY_COLOR(QRect(50, 50, 50, 50), bgColor);
    }

    DummyScreen *screen = new DummyScreen;
    { // dummy accelerated move

        ColorWidget w(Qt::red);
        w.setWindowSurface(new DummyMoveSurface(&w));
        w.show();

        w.setGeometry(50, 50, 50, 50);
        QApplication::processEvents();
        VERIFY_COLOR(QRect(50, 50, 50, 50), w.color());
        VERIFY_COLOR(QRect(100, 100, 50, 50), bgColor);

        w.move(100, 100);
        QApplication::processEvents();
        // QEXPECT_FAIL("", "Task 169976", Continue);
        //VERIFY_COLOR(QRect(50, 50, 50, 50), w.color()); // unchanged
        VERIFY_COLOR(QRect(100, 100, 50, 50), bgColor); // unchanged
    }
    delete screen;
}

QTEST_MAIN(tst_QWSWindowSystem)

#include "tst_qwswindowsystem.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

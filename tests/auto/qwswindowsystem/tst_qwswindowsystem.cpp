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
#include <qpainter.h>
#include <qdesktopwidget.h>
#include <qdirectpainter_qws.h>

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

    // Painting is not necessarily enabled when running the vnc server.
    // Since this test is testing framebuffer memory we explictly enable it.
    server->enablePainting(true);
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

QTEST_MAIN(tst_QWSWindowSystem)

#include "tst_qwswindowsystem.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifndef Q_WS_MAC

#include <private/qwindowsurface_p.h>
#include <QDesktopWidget>

#ifdef Q_WS_X11
extern void qt_x11_wait_for_window_manager( QWidget* w );
#endif

class tst_QWindowSurface : public QObject
{
    Q_OBJECT

public:
    tst_QWindowSurface() {}
    ~tst_QWindowSurface() {}

private slots:
    void getSetWindowSurface();
    void flushOutsidePaintEvent();
};

class MyWindowSurface : public QWindowSurface
{
public:
    MyWindowSurface(QWidget *w) : QWindowSurface(w) {}

    QPaintDevice *paintDevice() {
        return &image;
    }

    void flush(QWidget*, const QRegion&, const QPoint&) {
        /* nothing */
    }

private:
    QImage image;
};

class ColorWidget : public QWidget
{
public:
    ColorWidget(QWidget *parent = 0, const QColor &c = QColor(Qt::red))
        : QWidget(parent, Qt::FramelessWindowHint), color(c)
    {
        QPalette opaquePalette = palette();
        opaquePalette.setColor(backgroundRole(), color);
        setPalette(opaquePalette);
        setAutoFillBackground(true);
    }

    void paintEvent(QPaintEvent *e) {
        r += e->region();
    }

    void reset() {
        r = QRegion();
    }

    QColor color;
    QRegion r;
};

#define VERIFY_COLOR(region, color) {                                   \
    const QRegion r = QRegion(region);                                  \
    for (int i = 0; i < r.rects().size(); ++i) {                        \
        const QRect rect = r.rects().at(i);                             \
        const QPixmap pixmap = QPixmap::grabWindow(QDesktopWidget().winId(), \
                                                   rect.left(), rect.top(), \
                                                   rect.width(), rect.height()); \
        QCOMPARE(pixmap.size(), rect.size());                           \
        QPixmap expectedPixmap(pixmap); /* ensure equal formats */      \
        expectedPixmap.fill(color);                                     \
        QCOMPARE(pixmap, expectedPixmap);                               \
    }                                                                   \
}

void tst_QWindowSurface::getSetWindowSurface()
{
    QWidget w;
    QVERIFY(!w.windowSurface());

    w.show();
    QApplication::processEvents();
    QVERIFY(w.windowSurface());

    for (int i = 0; i < 2; ++i) {
        MyWindowSurface *surface = new MyWindowSurface(&w);
        QCOMPARE(w.windowSurface(), (QWindowSurface *)surface);

        w.setWindowSurface(surface);
        QCOMPARE(w.windowSurface(), (QWindowSurface *)surface);
    }
}

void tst_QWindowSurface::flushOutsidePaintEvent()
{
    ColorWidget w(0, Qt::red);
    w.setGeometry(10, 10, 50, 50);
    w.show();

    QApplication::processEvents();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&w);
#endif
    VERIFY_COLOR(w.geometry(), w.color);
    w.reset();

    // trigger a paintEvent() the next time the event loop is entered
    w.update();

    // draw a black rectangle inside the widget
    QWindowSurface *surface = w.windowSurface();
    QVERIFY(surface);
    const QRect rect = surface->rect(&w);
    surface->beginPaint(rect);
    QImage *img = surface->buffer(&w);
    if (img)
        img->fill(0);
    surface->endPaint(rect);
    surface->flush(&w, rect, QPoint());

    // the paintEvent() should overwrite the painted rectangle
    QApplication::processEvents();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&w);
#endif
    VERIFY_COLOR(w.geometry(), w.color);
    QCOMPARE(QRegion(w.rect()), w.r);
    w.reset();
}

QTEST_MAIN(tst_QWindowSurface)

#else // Q_WS_MAC

QTEST_NOOP_MAIN

#endif

#include "tst_qwindowsurface.moc"

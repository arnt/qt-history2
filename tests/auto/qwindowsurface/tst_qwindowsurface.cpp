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

class tst_QWindowSurface : public QObject
{
    Q_OBJECT

public:
    tst_QWindowSurface() {}
    ~tst_QWindowSurface() {}

private slots:
    void getSetWindowSurface();
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

void tst_QWindowSurface::getSetWindowSurface()
{
    QWidget w;
    QVERIFY(!w.windowSurface());

    w.show();
    QApplication::processEvents();
    QVERIFY(w.windowSurface());

    MyWindowSurface *surface = new MyWindowSurface(&w);
    QCOMPARE(w.windowSurface(), surface);

    w.setWindowSurface(surface);
    QCOMPARE(w.windowSurface(), surface);
}

QTEST_MAIN(tst_QWindowSurface)

#else // Q_WS_MAC

QTEST_NOOP_MAIN

#endif

#include "tst_qwindowsurface.moc"

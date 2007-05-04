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
#include <private/qwindowsurface_qws_p.h>
#include <private/qdrawhelper_p.h>

class tst_QDirectPainter : public QObject
{
    Q_OBJECT

public:
    tst_QDirectPainter() {}
    ~tst_QDirectPainter() {}

private slots:
    void initTestCase();
    void setGeometry_data();
    void setGeometry();

private:
    QWSWindow* getWindow(int windId);
    QColor bgColor;
};

class ColorPainter : public QDirectPainter
{
public:
    ColorPainter(SurfaceFlag flag = NonReserved,
                 const QColor &color = QColor(Qt::red))
        : QDirectPainter(0, flag), c(color) {}

    QColor color() { return c; }

protected:
    void regionChanged(const QRegion &region) {
        QScreen::instance()->solidFill(c, region);
    }

private:
    QColor c;
    QRegion r;
};

Q_DECLARE_METATYPE(QDirectPainter::SurfaceFlag)

void tst_QDirectPainter::initTestCase()
{
    bgColor = QColor(Qt::green);

    QWSServer *server = QWSServer::instance();
    server->setBackground(bgColor);
}

QWSWindow* tst_QDirectPainter::getWindow(int winId)
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

void tst_QDirectPainter::setGeometry_data()
{
    QTest::addColumn<QDirectPainter::SurfaceFlag>("flag");

    QTest::newRow("NonReserved") << QDirectPainter::NonReserved;
    QTest::newRow("Reserved") << QDirectPainter::Reserved;
    QTest::newRow("ReservedSynchronous") << QDirectPainter::ReservedSynchronous;
}

void tst_QDirectPainter::setGeometry()
{
    QFETCH(QDirectPainter::SurfaceFlag, flag);

    const QRect rect(100, 100, 100, 100);
    {
        ColorPainter w(flag);

        w.setGeometry(rect);
        QApplication::processEvents();
        QCOMPARE(w.geometry(), rect);
        VERIFY_COLOR(rect, w.color());
    }
    QApplication::processEvents();
    VERIFY_COLOR(rect, bgColor);
}

QTEST_MAIN(tst_QDirectPainter)

#include "tst_qdirectpainter.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

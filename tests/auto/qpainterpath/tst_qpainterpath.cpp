/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qfile.h>
#include <qpainterpath.h>

#define _USE_MATH_DEFINES
#include <math.h>

class tst_QPainterPath : public QObject
{
    Q_OBJECT

public:

private slots:
    void getSetCheck();
    void contains_QPointF_data();
    void contains_QPointF();

    void contains_QRectF_data();
    void contains_QRectF();

    void currentPosition();

    void testOperatorEquals();
    void testOperatorDatastream();

#if QT_VERSION >= 0x040200
    void testArcMoveTo_data();
    void testArcMoveTo();
    void setElementPositionAt();
#endif

    void testOnPath_data();
    void testOnPath();
    
    void closing();
};

// Testing get/set functions
void tst_QPainterPath::getSetCheck()
{
    QPainterPathStroker obj1;
    // qreal QPainterPathStroker::width()
    // void QPainterPathStroker::setWidth(qreal)
    obj1.setWidth(0.0);
    QCOMPARE(1.0, obj1.width()); // Pathstroker sets with to 1 if <= 0
    obj1.setWidth(0.5);
    QCOMPARE(0.5, obj1.width());
    obj1.setWidth(1.1);
    QCOMPARE(1.1, obj1.width());

    // qreal QPainterPathStroker::miterLimit()
    // void QPainterPathStroker::setMiterLimit(qreal)
    obj1.setMiterLimit(0.0);
    QCOMPARE(0.0, obj1.miterLimit());
    obj1.setMiterLimit(1.1);
    QCOMPARE(1.1, obj1.miterLimit());

    // qreal QPainterPathStroker::curveThreshold()
    // void QPainterPathStroker::setCurveThreshold(qreal)
    obj1.setCurveThreshold(0.0);
    QCOMPARE(0.0, obj1.curveThreshold());
    obj1.setCurveThreshold(1.1);
    QCOMPARE(1.1, obj1.curveThreshold());
}

Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPointF)
Q_DECLARE_METATYPE(QRectF)

void tst_QPainterPath::currentPosition()
{
    QPainterPath p;

    QCOMPARE(p.currentPosition(), QPointF());

    p.moveTo(100, 100);
    QCOMPARE(p.currentPosition(), QPointF(100, 100));

    p.lineTo(200, 200);
    QCOMPARE(p.currentPosition(), QPointF(200, 200));

    p.cubicTo(300, 200, 200, 300, 500, 500);
    QCOMPARE(p.currentPosition(), QPointF(500, 500));
}

void tst_QPainterPath::contains_QPointF_data()
{
    QTest::addColumn<QPainterPath>("path");
    QTest::addColumn<QPointF>("pt");
    QTest::addColumn<bool>("contained");

    QPainterPath path;
    path.addRect(0, 0, 100, 100);

    // #####
    // #   #
    // #   #
    // #   #
    // #####

    QTest::newRow("[0,0] in [0,0,100,100]") << path << QPointF(0, 0) << true;

    QTest::newRow("[99,0] in [0,0,100,100]") << path << QPointF(99, 0) << true;
    QTest::newRow("[0,99] in [0,0,100,100]") << path << QPointF(0, 99) << true;
    QTest::newRow("[99,99] in [0,0,100,100]") << path << QPointF(99, 99) << true;

    QTest::newRow("[99.99,0] in [0,0,100,100]") << path << QPointF(99.99, 0) << true;
    QTest::newRow("[0,99.99] in [0,0,100,100]") << path << QPointF(0, 99.99) << true;
    QTest::newRow("[99.99,99.99] in [0,0,100,100]") << path << QPointF(99.99, 99.99) << true;

    QTest::newRow("[0.01,0.01] in [0,0,100,100]") << path << QPointF(0.01, 0.01) << true;
    QTest::newRow("[0,0.01] in [0,0,100,100]") << path << QPointF(0, 0.01) << true;
    QTest::newRow("[0.01,0] in [0,0,100,100]") << path << QPointF(0.01, 0) << true;

    QTest::newRow("[-0.01,-0.01] in [0,0,100,100]") << path << QPointF(-0.01, -0.01) << false;
    QTest::newRow("[-0,-0.01] in [0,0,100,100]") << path << QPointF(0, -0.01) << false;
    QTest::newRow("[-0.01,0] in [0,0,100,100]") << path << QPointF(-0.01, 0) << false;


    QTest::newRow("[-10,0] in [0,0,100,100]") << path << QPointF(-10, 0) << false;
    QTest::newRow("[100,0] in [0,0,100,100]") << path << QPointF(100, 0) << false;

    QTest::newRow("[0,-10] in [0,0,100,100]") << path << QPointF(0, -10) << false;
    QTest::newRow("[0,100] in [0,0,100,100]") << path << QPointF(0, 100) << false;

    QTest::newRow("[100.1,0] in [0,0,100,100]") << path << QPointF(100.1, 0) << false;
    QTest::newRow("[0,100.1] in [0,0,100,100]") << path << QPointF(0, 100.1) << false;

    path.addRect(50, 50, 100, 100);

    // #####
    // #   #
    // # #####
    // # # # #
    // ##### #
    //   #   #
    //   #####

    QTest::newRow("[49,49] in 2 rects") << path << QPointF(49,49) << true;
    QTest::newRow("[50,50] in 2 rects") << path << QPointF(50,50) << false;
    QTest::newRow("[100,100] in 2 rects") << path << QPointF(100,100) << true;

    path.setFillRule(Qt::WindingFill);
    QTest::newRow("[50,50] in 2 rects (winding)") << path << QPointF(50,50) << true;

    path.addEllipse(0, 0, 150, 150);

    // #####
    // ##  ##
    // # #####
    // # # # #
    // ##### #
    //  ##  ##
    //   #####

    QTest::newRow("[50,50] in complex (winding)") << path << QPointF(50, 50) << true;

    path.setFillRule(Qt::OddEvenFill);
    QTest::newRow("[50,50] in complex (windinf)") << path << QPointF(50, 50) << true;
    QTest::newRow("[49,49] in complex") << path << QPointF(49,49) << false;
    QTest::newRow("[100,100] in complex") << path << QPointF(49,49) << false;


    // unclosed triangle
    path = QPainterPath();
    path.moveTo(100, 100);
    path.lineTo(130, 70);
    path.lineTo(150, 110);

    QTest::newRow("[100,100] in triangle") << path << QPointF(100, 100) << true;
    QTest::newRow("[140,100] in triangle") << path << QPointF(140, 100) << true;
    QTest::newRow("[130,80] in triangle") << path << QPointF(130, 80) << true;

    QTest::newRow("[110,80] in triangle") << path << QPointF(110, 80) << false;
    QTest::newRow("[150,100] in triangle") << path << QPointF(150, 100) << false;
    QTest::newRow("[120,110] in triangle") << path << QPointF(120, 110) << false;

    QRectF base_rect(0, 0, 20, 20);

    path = QPainterPath();
    path.addEllipse(base_rect);

    // not strictly precise, but good enougth to verify fair precision.
    QPainterPath inside;
    inside.addEllipse(base_rect.adjusted(5, 5, -5, -5));
    QPolygonF inside_poly = inside.toFillPolygon();
    for (int i=0; i<inside_poly.size(); ++i)
        QTest::newRow("inside_ellipse") << path << inside_poly.at(i) << true;

    QPainterPath outside;
    outside.addEllipse(base_rect.adjusted(-5, -5, 5, 5));
    QPolygonF outside_poly = outside.toFillPolygon();
    for (int i=0; i<outside_poly.size(); ++i)
        QTest::newRow("outside_ellipse") << path << outside_poly.at(i) << false;

    path = QPainterPath();
    base_rect = QRectF(50, 50, 200, 200);
    path.addEllipse(base_rect);
    path.setFillRule(Qt::WindingFill);

    QTest::newRow("topleft outside ellipse") << path << base_rect.topLeft() << false;
    QTest::newRow("topright outside ellipse") << path << base_rect.topRight() << false;
    QTest::newRow("bottomright outside ellipse") << path << base_rect.bottomRight() << false;
    QTest::newRow("bottomleft outside ellipse") << path << base_rect.bottomLeft() << false;

    // Test horizontal curve segment
    path = QPainterPath();
    path.moveTo(100, 100);
    path.cubicTo(120, 100, 180, 100, 200, 100);
    path.lineTo(150, 200);
    path.closeSubpath();

    QTest::newRow("horizontal cubic, out left") << path << QPointF(0, 100) << false;
    QTest::newRow("horizontal cubic, out right") << path << QPointF(300, 100) <<false;
    QTest::newRow("horizontal cubic, in mid") << path << QPointF(150, 100) << true;
}

void tst_QPainterPath::contains_QPointF()
{
    QFETCH(QPainterPath, path);
    QFETCH(QPointF, pt);
    QFETCH(bool, contained);

    QCOMPARE(path.contains(pt), contained);
}

void tst_QPainterPath::contains_QRectF_data()
{
    QTest::addColumn<QPainterPath>("path");
    QTest::addColumn<QRectF>("rect");
    QTest::addColumn<bool>("contained");

    QPainterPath path;
    path.addRect(0, 0, 100, 100);

    QTest::newRow("same rect") << path << QRectF(0.1, 0.1, 99, 99) << true; // ###
    QTest::newRow("outside") << path << QRectF(-1, -1, 100, 100) << false;
    QTest::newRow("covers") << path << QRectF(-1, -1, 102, 102) << false;
    QTest::newRow("left") << path << QRectF(-10, 50, 5, 5) << false;
    QTest::newRow("top") << path << QRectF(50, -10, 5, 5) << false;
    QTest::newRow("right") << path << QRectF(110, 50, 5, 5) << false;
    QTest::newRow("bottom") << path << QRectF(50, 110, 5, 5) << false;

    path.addRect(50, 50, 100, 100);

    QTest::newRow("r1 top") << path << QRectF(0.1, 0.1, 99, 49) << true;
    QTest::newRow("r1 left") << path << QRectF(0.1, 0.1, 49, 99) << true;
    QTest::newRow("r2 right") << path << QRectF(100.01, 50.1, 49, 99) << true;
    QTest::newRow("r2 bottom") << path << QRectF(50.1, 100.1, 99, 49) << true;
    QTest::newRow("inside 2 rects") << path << QRectF(51, 51, 48, 48) << false;
    QTest::newRow("topRight 2 rects") << path << QRectF(100, 0, 49, 49) << false;
    QTest::newRow("bottomLeft 2 rects") << path << QRectF(0, 100, 49, 49) << false;

    path.setFillRule(Qt::WindingFill);
    QTest::newRow("inside 2 rects (winding)") << path << QRectF(51, 51, 48, 48) << true;

    path.addEllipse(0, 0, 150, 150);
    QTest::newRow("topRight 2 rects") << path << QRectF(100, 25, 24, 24) << true;
    QTest::newRow("bottomLeft 2 rects") << path << QRectF(25, 100, 24, 24) << true;

    path.setFillRule(Qt::OddEvenFill);
    QTest::newRow("inside 2 rects") << path << QRectF(50, 50, 49, 49) << false;
}

void tst_QPainterPath::contains_QRectF()
{
    QFETCH(QPainterPath, path);
    QFETCH(QRectF, rect);
    QFETCH(bool, contained);

    QCOMPARE(path.contains(rect), contained);
}

void tst_QPainterPath::testOperatorEquals()
{
    QPainterPath empty1;
    QPainterPath empty2;
    QVERIFY(empty1 == empty2);

    QPainterPath rect1;
    rect1.addRect(100, 100, 100, 100);
    QVERIFY(rect1 == rect1);
    QVERIFY(rect1 != empty1);

    QPainterPath rect2;
    rect2.addRect(100, 100, 100, 100);
    QVERIFY(rect1 == rect2);

    rect2.setFillRule(Qt::WindingFill);
    QVERIFY(rect1 != rect2);

    QPainterPath ellipse1;
    ellipse1.addEllipse(50, 50, 100, 100);
    QVERIFY(rect1 != ellipse1);

    QPainterPath ellipse2;
    ellipse2.addEllipse(50, 50, 100, 100);
    QVERIFY(ellipse1 == ellipse2);
}



void tst_QPainterPath::testOperatorDatastream()
{
    QPainterPath path;
    path.addEllipse(0, 0, 100, 100);
    path.addRect(0, 0, 100, 100);
    path.setFillRule(Qt::WindingFill);

    // Write out
    {
        QFile data("data");
        bool ok = data.open(QFile::WriteOnly);
        QVERIFY(ok);
        QDataStream stream(&data);
        stream << path;
    }

    QPainterPath other;
    // Read in
    {
        QFile data("data");
        bool ok = data.open(QFile::ReadOnly);
        QVERIFY(ok);
        QDataStream stream(&data);
        stream >> other;
    }

    QVERIFY(other == path);
}

void tst_QPainterPath::closing()
{
    // lineto's
    {
        QPainterPath triangle(QPoint(100, 100));

        triangle.lineTo(200, 100);
        triangle.lineTo(200, 200);
        QCOMPARE(triangle.elementCount(), 3);

        triangle.closeSubpath();
        QCOMPARE(triangle.elementCount(), 4);
        QCOMPARE(triangle.elementAt(3).type, QPainterPath::LineToElement);

        triangle.moveTo(300, 300);
        QCOMPARE(triangle.elementCount(), 5);
        QCOMPARE(triangle.elementAt(4).type, QPainterPath::MoveToElement);

        triangle.lineTo(400, 300);
        triangle.lineTo(400, 400);
        QCOMPARE(triangle.elementCount(), 7);

        triangle.closeSubpath();
        QCOMPARE(triangle.elementCount(), 8);

        // this will should trigger implicit moveto...
        triangle.lineTo(600, 300);
        QCOMPARE(triangle.elementCount(), 10);
        QCOMPARE(triangle.elementAt(8).type, QPainterPath::MoveToElement);
        QCOMPARE(triangle.elementAt(9).type, QPainterPath::LineToElement);

        triangle.lineTo(600, 700);
        QCOMPARE(triangle.elementCount(), 11);
    }

    // curveto's
    {
        QPainterPath curves(QPoint(100, 100));

        curves.cubicTo(200, 100, 100, 200, 200, 200);
        QCOMPARE(curves.elementCount(), 4);

        curves.closeSubpath();
        QCOMPARE(curves.elementCount(), 5);
        QCOMPARE(curves.elementAt(4).type, QPainterPath::LineToElement);

        curves.moveTo(300, 300);
        QCOMPARE(curves.elementCount(), 6);
        QCOMPARE(curves.elementAt(5).type, QPainterPath::MoveToElement);

        curves.cubicTo(400, 300, 300, 400, 400, 400);
        QCOMPARE(curves.elementCount(), 9);

        curves.closeSubpath();
        QCOMPARE(curves.elementCount(), 10);

        // should trigger implicit moveto..
        curves.cubicTo(100, 800, 800, 100, 800, 800);
        QCOMPARE(curves.elementCount(), 14);
        QCOMPARE(curves.elementAt(10).type, QPainterPath::MoveToElement);
        QCOMPARE(curves.elementAt(11).type, QPainterPath::CurveToElement);
    }

    {
        QPainterPath rects;
        rects.addRect(100, 100, 100, 100);

        QCOMPARE(rects.elementCount(), 5);
        QCOMPARE(rects.elementAt(0).type, QPainterPath::MoveToElement);
        QCOMPARE(rects.elementAt(4).type, QPainterPath::LineToElement);

        rects.addRect(300, 100, 100,100);
        QCOMPARE(rects.elementCount(), 10);
        QCOMPARE(rects.elementAt(5).type, QPainterPath::MoveToElement);
        QCOMPARE(rects.elementAt(9).type, QPainterPath::LineToElement);

        rects.lineTo(0, 0);
        QCOMPARE(rects.elementCount(), 12);
        QCOMPARE(rects.elementAt(10).type, QPainterPath::MoveToElement);
        QCOMPARE(rects.elementAt(11).type, QPainterPath::LineToElement);
    }

    {
        QPainterPath ellipses;
        ellipses.addEllipse(100, 100, 100, 100);

        QCOMPARE(ellipses.elementCount(), 13);
        QCOMPARE(ellipses.elementAt(0).type, QPainterPath::MoveToElement);
        QCOMPARE(ellipses.elementAt(10).type, QPainterPath::CurveToElement);

        ellipses.addEllipse(300, 100, 100,100);
        QCOMPARE(ellipses.elementCount(), 26);
        QCOMPARE(ellipses.elementAt(13).type, QPainterPath::MoveToElement);
        QCOMPARE(ellipses.elementAt(23).type, QPainterPath::CurveToElement);

        ellipses.lineTo(0, 0);
        QCOMPARE(ellipses.elementCount(), 28);
        QCOMPARE(ellipses.elementAt(26).type, QPainterPath::MoveToElement);
        QCOMPARE(ellipses.elementAt(27).type, QPainterPath::LineToElement);
    }
}

#if QT_VERSION >= 0x040200
void tst_QPainterPath::testArcMoveTo_data()
{
    QTest::addColumn<QRectF>("rect");
    QTest::addColumn<qreal>("angle");

    QList<QRectF> rects;
    rects << QRectF(100, 100, 100, 100)
          << QRectF(100, 100, -100, 100)
          << QRectF(100, 100, 100, -100)
          << QRectF(100, 100, -100, -100);

    for (int domain=0; domain<rects.size(); ++domain) {
        for (int i=-360; i<=360; ++i) {
            QTest::newRow("test") << rects.at(domain) << (qreal) i;
        }
    }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define ANGLE(t) ((t) * 2 * M_PI / 360.0)

void tst_QPainterPath::testArcMoveTo()
{
    QFETCH(QRectF, rect);
    QFETCH(qreal, angle);

    QPainterPath path;
    path.arcMoveTo(rect, angle);

    QPointF pos = path.elementAt(0);


    qreal x_radius = rect.width() / 2.0;
    qreal y_radius = rect.height() / 2.0;

    QPointF shouldBe = rect.center()
                       + QPointF(x_radius * cos(ANGLE(angle)), -y_radius * sin(ANGLE(angle)));

    QVERIFY(qFuzzyCompare(pos.x(), shouldBe.x()));
    QVERIFY(qFuzzyCompare(pos.y(), shouldBe.y()));
}

void tst_QPainterPath::testOnPath_data()
{
    QTest::addColumn<QPainterPath>("path");
    QTest::addColumn<qreal>("start");
    QTest::addColumn<qreal>("middle");
    QTest::addColumn<qreal>("end");

    QPainterPath path = QPainterPath(QPointF(153, 199));
    path.cubicTo(QPointF(147, 61), QPointF(414, 18),
                 QPointF(355, 201));

    QTest::newRow("First case") << path
                                << -93.0
                                << -4.0
                                << 107.87;

    path = QPainterPath(QPointF(328, 197));
    path.cubicTo(QPointF(150, 50), QPointF(401, 50),
                 QPointF(225, 197));
    QTest::newRow("Second case") << path
                                << -140.0
                                << 0.0
                                << 140.0;

    path = QPainterPath(QPointF(328, 197));
    path.cubicTo(QPointF(101 , 153), QPointF(596, 151),
                 QPointF(353, 197));
    QTest::newRow("Third case") << path
                                << -169.0
                                << -0.22
                                <<  169.0;

    path = QPainterPath(QPointF(153, 199));
    path.cubicTo(QPointF(59, 53), QPointF(597, 218),
                  QPointF(355, 201));
    QTest::newRow("Fourth case") << path
                                 << -122.0
                                 <<  12.0
                                 << -175.0;
    
}

#define SIGN(x) ((x < 0)?-1:1)
void tst_QPainterPath::testOnPath()
{
    QFETCH(QPainterPath, path);
    QFETCH(qreal, start);
    QFETCH(qreal, middle);
    QFETCH(qreal, end);

    int signStart = SIGN(start);
    int signMid   = SIGN(middle);
    int signEnd   = SIGN(end);

    static const qreal diff = 3;

    qreal angle = path.angleAtPercent(0);
    QVERIFY(SIGN(angle) == signStart);
    QVERIFY(qAbs(angle-start) < diff);
    
    angle = path.angleAtPercent(0.5);
    QVERIFY(SIGN(angle) == signMid);
    QVERIFY(qAbs(angle-middle) < diff);
    
    angle = path.angleAtPercent(1);
    QVERIFY(SIGN(angle) == signEnd);
    QVERIFY(qAbs(angle-end) < diff);
}

void tst_QPainterPath::setElementPositionAt()
{
    QPainterPath path(QPointF(42., 42.));
    QCOMPARE(path.elementCount(), 1);
    QVERIFY(path.elementAt(0).type == QPainterPath::MoveToElement);
    QCOMPARE(path.elementAt(0).x, qreal(42.));
    QCOMPARE(path.elementAt(0).y, qreal(42.));

    QPainterPath copy = path;
    copy.setElementPositionAt(0, qreal(0), qreal(0));
    QCOMPARE(copy.elementCount(), 1);
    QVERIFY(copy.elementAt(0).type == QPainterPath::MoveToElement);
    QCOMPARE(copy.elementAt(0).x, qreal(0));
    QCOMPARE(copy.elementAt(0).y, qreal(0));

    QCOMPARE(path.elementCount(), 1);
    QVERIFY(path.elementAt(0).type == QPainterPath::MoveToElement);
    QCOMPARE(path.elementAt(0).x, qreal(42.));
    QCOMPARE(path.elementAt(0).y, qreal(42.));
}

#endif

QTEST_APPLESS_MAIN(tst_QPainterPath)

#include "tst_qpainterpath.moc"

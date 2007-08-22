/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qline.h>

#include <math.h>

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif





//TESTED_CLASS=
//TESTED_FILES=corelib/io/QLine.h corelib/io/QLine.cpp

class tst_QLine : public QObject
{
Q_OBJECT

public:
    tst_QLine();

private slots:
    void testIntersection();
    void testIntersection_data();

    void testLength();
    void testLength_data();

    void testNormalVector();
    void testNormalVector_data();

    void testAngle();
    void testAngle_data();

    void testSet();
};

// Square root of two
#define SQRT2 1.4142135623731

// Length of unit vector projected to x from 45 degrees
#define UNITX_45 0.707106781186547

tst_QLine::tst_QLine()

{
}

void tst_QLine::testSet()
{
    {
        QLine l;
        l.setP1(QPoint(1, 2));
        l.setP2(QPoint(3, 4));

        QCOMPARE(l.x1(), 1);
        QCOMPARE(l.y1(), 2);
        QCOMPARE(l.x2(), 3);
        QCOMPARE(l.y2(), 4);

        l.setPoints(QPoint(5, 6), QPoint(7, 8));
        QCOMPARE(l.x1(), 5);
        QCOMPARE(l.y1(), 6);
        QCOMPARE(l.x2(), 7);
        QCOMPARE(l.y2(), 8);

        l.setLine(9, 10, 11, 12);
        QCOMPARE(l.x1(), 9);
        QCOMPARE(l.y1(), 10);
        QCOMPARE(l.x2(), 11);
        QCOMPARE(l.y2(), 12);
    }

    {
        QLineF l;
        l.setP1(QPointF(1, 2));
        l.setP2(QPointF(3, 4));

        QCOMPARE(l.x1(), 1.0);
        QCOMPARE(l.y1(), 2.0);
        QCOMPARE(l.x2(), 3.0);
        QCOMPARE(l.y2(), 4.0);

        l.setPoints(QPointF(5, 6), QPointF(7, 8));
        QCOMPARE(l.x1(), 5.0);
        QCOMPARE(l.y1(), 6.0);
        QCOMPARE(l.x2(), 7.0);
        QCOMPARE(l.y2(), 8.0);

        l.setLine(9.0, 10.0, 11.0, 12.0);
        QCOMPARE(l.x1(), 9.0);
        QCOMPARE(l.y1(), 10.0);
        QCOMPARE(l.x2(), 11.0);
        QCOMPARE(l.y2(), 12.0);
    }

}

void tst_QLine::testIntersection_data()
{
    QTest::addColumn<double>("xa1");
    QTest::addColumn<double>("ya1");
    QTest::addColumn<double>("xa2");
    QTest::addColumn<double>("ya2");
    QTest::addColumn<double>("xb1");
    QTest::addColumn<double>("yb1");
    QTest::addColumn<double>("xb2");
    QTest::addColumn<double>("yb2");
    QTest::addColumn<int>("type");
    QTest::addColumn<double>("ix");
    QTest::addColumn<double>("iy");

    QTest::newRow("parallel") << 1.0 << 1.0 << 3.0 << 4.0
                           << 5.0 << 6.0 << 7.0 << 9.0
                           << int(QLineF::NoIntersection) << 0.0 << 0.0;
    QTest::newRow("unbounded") << 1.0 << 1.0 << 5.0 << 5.0
                            << 0.0 << 4.0 << 3.0 << 4.0
                            << int(QLineF::UnboundedIntersection) << 4.0 << 4.0;
    QTest::newRow("bounded") << 1.0 << 1.0 << 5.0 << 5.0
                          << 0.0 << 4.0 << 5.0 << 4.0
                          << int(QLineF::BoundedIntersection) << 4.0 << 4.0;

    QTest::newRow("almost vertical") << 0.0 << 10.0 << 20.0000000000001 << 10.0
                                     << 10.0 << 0.0 << 10.0 << 20.0
                                     << int(QLineF::BoundedIntersection) << 10.0 << 10.0;

    QTest::newRow("almost horizontal") << 0.0 << 10.0 << 20.0 << 10.0
                                       << 10.0000000000001 << 0.0 << 10.0 << 20.0
                                       << int(QLineF::BoundedIntersection) << 10.0 << 10.0;
}

void tst_QLine::testIntersection()
{
    QFETCH(double, xa1);
    QFETCH(double, ya1);
    QFETCH(double, xa2);
    QFETCH(double, ya2);
    QFETCH(double, xb1);
    QFETCH(double, yb1);
    QFETCH(double, xb2);
    QFETCH(double, yb2);
    QFETCH(int, type);
    QFETCH(double, ix);
    QFETCH(double, iy);

    QLineF a(xa1, ya1, xa2, ya2);
    QLineF b(xb1, yb1, xb2, yb2);


    QPointF ip;
    QLineF::IntersectType itype = a.intersect(b, &ip);

    QCOMPARE(int(itype), type);
    if (type != QLineF::NoIntersection) {
        QCOMPARE(ip.x(), qreal(ix));
        QCOMPARE(ip.y(), qreal(iy));
    }
}

void tst_QLine::testLength_data()
{
    QTest::addColumn<double>("x1");
    QTest::addColumn<double>("y1");
    QTest::addColumn<double>("x2");
    QTest::addColumn<double>("y2");
    QTest::addColumn<double>("length");
    QTest::addColumn<double>("lengthToSet");
    QTest::addColumn<double>("vx");
    QTest::addColumn<double>("vy");

    QTest::newRow("[1,0]*2") << 0.0 << 0.0 << 1.0 << 0.0 << 1.0 << 2.0 << 2.0 << 0.0;
    QTest::newRow("[0,1]*2") << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 2.0 << 0.0 << 2.0;
    QTest::newRow("[-1,0]*2") << 0.0 << 0.0 << -1.0 << 0.0 << 1.0 << 2.0 << -2.0 << 0.0;
    QTest::newRow("[0,-1]*2") << 0.0 << 0.0 << 0.0 << -1.0 << 1.0 << 2.0 << 0.0 << -2.0;
    QTest::newRow("[1,1]->|1|") << 0.0 << 0.0 << 1.0 << 1.0
                             << double(SQRT2) << 1.0 << double(UNITX_45) << double(UNITX_45);
    QTest::newRow("[-1,1]->|1|") << 0.0 << 0.0 << -1.0 << 1.0
                             << double(SQRT2) << 1.0 << double(-UNITX_45) << double(UNITX_45);
    QTest::newRow("[1,-1]->|1|") << 0.0 << 0.0 << 1.0 << -1.0
                             << double(SQRT2) << 1.0 << double(UNITX_45) << double(-UNITX_45);
    QTest::newRow("[-1,-1]->|1|") << 0.0 << 0.0 << -1.0 << -1.0
                             << double(SQRT2) << 1.0 << double(-UNITX_45) << double(-UNITX_45);
    QTest::newRow("[1,0]*2 (2,2)") << 2.0 << 2.0 << 3.0 << 2.0 << 1.0 << 2.0 << 2.0 << 0.0;
    QTest::newRow("[0,1]*2 (2,2)") << 2.0 << 2.0 << 2.0 << 3.0 << 1.0 << 2.0 << 0.0 << 2.0;
    QTest::newRow("[-1,0]*2 (2,2)") << 2.0 << 2.0 << 1.0 << 2.0 << 1.0 << 2.0 << -2.0 << 0.0;
    QTest::newRow("[0,-1]*2 (2,2)") << 2.0 << 2.0 << 2.0 << 1.0 << 1.0 << 2.0 << 0.0 << -2.0;
    QTest::newRow("[1,1]->|1| (2,2)") << 2.0 << 2.0 << 3.0 << 3.0
                                   << double(SQRT2) << 1.0 << double(UNITX_45) << double(UNITX_45);
    QTest::newRow("[-1,1]->|1| (2,2)") << 2.0 << 2.0 << 1.0 << 3.0
                                    << double(SQRT2) << 1.0 << double(-UNITX_45) << double(UNITX_45);
    QTest::newRow("[1,-1]->|1| (2,2)") << 2.0 << 2.0 << 3.0 << 1.0
                                    << double(SQRT2) << 1.0 << double(UNITX_45) << double(-UNITX_45);
    QTest::newRow("[-1,-1]->|1| (2,2)") << 2.0 << 2.0 << 1.0 << 1.0
                                     << double(SQRT2) << 1.0 << double(-UNITX_45) << double(-UNITX_45);
}

void tst_QLine::testLength()
{
    QFETCH(double, x1);
    QFETCH(double, y1);
    QFETCH(double, x2);
    QFETCH(double, y2);
    QFETCH(double, length);
    QFETCH(double, lengthToSet);
    QFETCH(double, vx);
    QFETCH(double, vy);

    QLineF l(x1, y1, x2, y2);
    QCOMPARE(l.length(), qreal(length));

    l.setLength(lengthToSet);
    QCOMPARE(l.length(), qreal(lengthToSet));
    QCOMPARE(l.dx(), qreal(vx));
    QCOMPARE(l.dy(), qreal(vy));
}


void tst_QLine::testNormalVector_data()
{
    QTest::addColumn<double>("x1");
    QTest::addColumn<double>("y1");
    QTest::addColumn<double>("x2");
    QTest::addColumn<double>("y2");
    QTest::addColumn<double>("nvx");
    QTest::addColumn<double>("nvy");

    QTest::newRow("[1, 0]") << 0.0 << 0.0 << 1.0 << 0.0 << 0.0 << -1.0;
    QTest::newRow("[-1, 0]") << 0.0 << 0.0 << -1.0 << 0.0 << 0.0 << 1.0;
    QTest::newRow("[0, 1]") << 0.0 << 0.0 << 0.0 << 1.0 << 1.0 << 0.0;
    QTest::newRow("[0, -1]") << 0.0 << 0.0 << 0.0 << -1.0 << -1.0 << 0.0;
    QTest::newRow("[2, 3]") << 2.0 << 3.0 << 4.0 << 6.0 << 3.0 << -2.0;
}

void tst_QLine::testNormalVector()
{
    QFETCH(double, x1);
    QFETCH(double, y1);
    QFETCH(double, x2);
    QFETCH(double, y2);
    QFETCH(double, nvx);
    QFETCH(double, nvy);

    QLineF l(x1, y1, x2, y2);
    QLineF n = l.normalVector();

    QCOMPARE(l.x1(), n.x1());
    QCOMPARE(l.y1(), n.y1());

    QCOMPARE(n.dx(), qreal(nvx));
    QCOMPARE(n.dy(), qreal(nvy));
}

void tst_QLine::testAngle_data()
{
    QTest::addColumn<double>("xa1");
    QTest::addColumn<double>("ya1");
    QTest::addColumn<double>("xa2");
    QTest::addColumn<double>("ya2");
    QTest::addColumn<double>("xb1");
    QTest::addColumn<double>("yb1");
    QTest::addColumn<double>("xb2");
    QTest::addColumn<double>("yb2");
    QTest::addColumn<double>("angle");

    QTest::newRow("parallel") << 1.0 << 1.0 << 3.0 << 4.0
                           << 5.0 << 6.0 << 7.0 << 9.0
                           << 0.0;
    QTest::newRow("[4,4]-[4,0]") << 1.0 << 1.0 << 5.0 << 5.0
                              << 0.0 << 4.0 << 3.0 << 4.0
                              << 45.0;
    QTest::newRow("[4,4]-[-4,0]") << 1.0 << 1.0 << 5.0 << 5.0
                              << 3.0 << 4.0 << 0.0 << 4.0
                              << 135.0;

    for (int i=0; i<180; ++i) {
        QTest::newRow(QString("angle:%1").arg(i).toLatin1())
            << 0.0 << 0.0 << double(cos(i*M_2PI/360)) << double(sin(i*M_2PI/360))
            << 0.0 << 0.0 << 1.0 << 0.0
            << double(i);
    }
}

void tst_QLine::testAngle()
{
    QFETCH(double, xa1);
    QFETCH(double, ya1);
    QFETCH(double, xa2);
    QFETCH(double, ya2);
    QFETCH(double, xb1);
    QFETCH(double, yb1);
    QFETCH(double, xb2);
    QFETCH(double, yb2);
    QFETCH(double, angle);

    QLineF a(xa1, ya1, xa2, ya2);
    QLineF b(xb1, yb1, xb2, yb2);

    double resultAngle = a.angle(b);
    QCOMPARE(qRound(resultAngle), qRound(angle));
}

QTEST_MAIN(tst_QLine)
#include "tst_qline.moc"

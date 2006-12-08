/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qregion.h>

#include <qbitmap.h>
#include <qpolygon.h>


//TESTED_CLASS=
//TESTED_FILES=gui/painting/qregion.h gui/painting/qregion.cpp

class tst_QRegion : public QObject
{
    Q_OBJECT

public:
    tst_QRegion();

private slots:
    void boundingRect();
    void rects();
    void setRects();
    void ellipseRegion();
    void polygonRegion();
    void bitmapRegion();
    void intersected_data();
    void intersected();
    void emptyPolygonRegion_data();
    void emptyPolygonRegion();

    void intersects_region_data();
    void intersects_region();
    void intersects_rect_data();
    void intersects_rect();
    void contains_point();

    void operator_plus_data();
    void operator_plus();
    void operator_minus_data();
    void operator_minus();
};

Q_DECLARE_METATYPE(QPolygon)
Q_DECLARE_METATYPE(QVector<QRect>)
Q_DECLARE_METATYPE(QRegion)

tst_QRegion::tst_QRegion()
{
}

void tst_QRegion::boundingRect()
{
    {
	QRect rect;
	QRegion region( rect );
	QCOMPARE( region.boundingRect(), rect );
    }
    {
	QRect rect( 10, -20, 30, 40 );
	QRegion region( rect );
	QCOMPARE( region.boundingRect(), rect );
    }
    {
	QRect rect(15,25,10,10);
	QRegion region( rect );
	QCOMPARE( region.boundingRect(), rect );
    }

}

void tst_QRegion::rects()
{
    {
	QRect rect;
	QRegion region( rect );
	QVERIFY( region.isEmpty() );
	QVERIFY( region.rects().isEmpty() );
    }
    {
	QRect rect( 10, -20, 30, 40 );
	QRegion region( rect );
#if QT_VERSION < 0x040000
	QCOMPARE( region.rects().count(), (uint)1 );
#else
	QCOMPARE( region.rects().count(), 1 );
#endif

	QCOMPARE( region.rects()[0], rect );
    }
    {
	QRect r( QPoint(10, 10), QPoint(40, 40) );
	QRegion region( r );
	QVERIFY( region.contains( QPoint(10,10) ) );
	QVERIFY( region.contains( QPoint(20,40) ) );
	QVERIFY( region.contains( QPoint(40,20) ) );
	QVERIFY( !region.contains( QPoint(20,41) ) );
	QVERIFY( !region.contains( QPoint(41,20) ) );
    }
    {
	QRect r( 10, 10, 30, 30 );
	QRegion region( r );
	QVERIFY( region.contains( QPoint(10,10) ) );
	QVERIFY( region.contains( QPoint(20,39) ) );
	QVERIFY( region.contains( QPoint(39,20) ) );
	QVERIFY( !region.contains( QPoint(20,40) ) );
	QVERIFY( !region.contains( QPoint(40,20) ) );
    }
}

void tst_QRegion::setRects()
{
    {
	QRegion region;
	region.setRects( 0, 0 );
	QVERIFY( region.rects().isEmpty() );
    }
    {
	QRegion region;
	QRect rect;
	region.setRects( &rect, 1 );
	QVERIFY( !region.boundingRect().isValid() );
	QVERIFY( region.rects().isEmpty() );
    }
    {
	QRegion region;
	QRect rect( 10, -20, 30, 40 );
	region.setRects( &rect, 1 );
#if QT_VERSION < 0x040000
	QCOMPARE( region.rects().count(), (uint)1 );
#else
	QCOMPARE( region.rects().count(), 1 );
#endif
	QCOMPARE( region.rects()[0], rect );
    }
}

void tst_QRegion::ellipseRegion()
{
    QRegion region(0, 0, 100, 100, QRegion::Ellipse);

    // These should not be inside the circe
    QVERIFY(!region.contains(QPoint(13, 13)));
    QVERIFY(!region.contains(QPoint(13, 86)));
    QVERIFY(!region.contains(QPoint(86, 13)));
    QVERIFY(!region.contains(QPoint(86, 86)));

    // These should be inside
    QVERIFY(region.contains(QPoint(16, 16)));
    QVERIFY(region.contains(QPoint(16, 83)));
    QVERIFY(region.contains(QPoint(83, 16)));
    QVERIFY(region.contains(QPoint(83, 83)));

    //     ..a..
    //   ..     ..
    //  .         .
    // .           .
    // b           c
    // .           .
    //  .         .
    //   ..     ..
    //     ..d..
    QVERIFY(region.contains(QPoint(50, 0)));   // Mid-top    (a)
    QVERIFY(region.contains(QPoint(0, 50)));   // Mid-left   (b)
    QVERIFY(region.contains(QPoint(99, 50)));  // Mid-right  (c)
    QVERIFY(region.contains(QPoint(50, 99)));  // Mid-bottom (d)

    QRect bounds = region.boundingRect();
    QCOMPARE(bounds.x(), 0);
    QCOMPARE(bounds.y(), 0);
    QCOMPARE(bounds.width(), 100);
    QCOMPARE(bounds.height(), 100);
}

void tst_QRegion::polygonRegion()
{
    QPolygon pa;
    {
	QRegion region ( pa );
	QVERIFY( region.isEmpty() );
    }
    {
	pa.setPoints( 8, 10, 10, //  a____________b
			 40, 10, //  |            |
			 40, 20, //  |___      ___|
			 30, 20, //      |    |
			 30, 40, //      |    |
			 20, 40, //      |    |
			 20, 20, //      |____c
			 10, 20 );

	QRegion region ( pa );
	QVERIFY( !region.isEmpty() );

	// These should not be inside the circle
	QVERIFY( !region.contains( QPoint(  9,  9 ) ) );
	QVERIFY( !region.contains( QPoint( 30, 41 ) ) );
	QVERIFY( !region.contains( QPoint( 41, 10 ) ) );
	QVERIFY( !region.contains( QPoint( 31, 21 ) ) );

	// These should be inside
	QVERIFY( region.contains( QPoint( 10, 10 ) ) ); // Upper-left  (a)

    }
}

void tst_QRegion::emptyPolygonRegion_data()
{
    QTest::addColumn<QPolygon>("pa");
    QTest::addColumn<bool>("isEmpty");
    QTest::addColumn<int>("numRects");
    QTest::addColumn<QVector<QRect> >("rects");

    QPolygon pa;


    QTest::newRow("no points") << pa << TRUE << 0 << QVector<QRect>();
    pa = QPolygon() << QPoint(10,10);
    QTest::newRow("one point") << pa << true << 0 << QVector<QRect>();
    pa = QPolygon() << QPoint(10,10) << QPoint(10,20);
    QTest::newRow("two points, horizontal") << pa << true << 0 << QVector<QRect>();

    pa = QPolygon() << QPoint(10,10) << QPoint(20,10);
    QTest::newRow("two points, vertical") << pa << true << 0 << QVector<QRect>();

    pa = QPolygon() << QPoint(10,10) << QPoint(20,20);
    QTest::newRow("two points, diagonal") << pa << true << 0 << QVector<QRect>();

    pa = QPolygon() << QPoint(10,10) << QPoint(15,15) << QPoint(10,15) << QPoint(10, 10) ;
    QVector<QRect> v;
    v << QRect(10,11,1, 1) << QRect(10,12,2,1) << QRect(10,13,3,1) << QRect(10,14,4,1);
    QTest::newRow("triangle") << pa << false << 4 << v;

    v.clear();
    v << QRect(10,10,10,10);

    QTest::newRow("rectangle") << QPolygon(QRect(10,10,10,10))  << false << 1 << v;

}

void tst_QRegion::emptyPolygonRegion()
{
    QFETCH(QPolygon, pa);

    QRegion r(pa);
    QTEST(r.isEmpty(), "isEmpty");
    QTEST(r.rects().count(), "numRects");
    QTEST(r.rects(), "rects");
}


static const char *circle_xpm[] = {
    "20 20 2 1",
    "	c #FFFFFF",
    ".	c #000000",
    "       ......       ",
    "     ..........     ",
    "   ..............   ",
    "  ................  ",
    "  ................  ",
    " .................. ",
    " .................. ",
    "....................",
    "....................",
    "....................",
    "....................",
    "....................",
    "....................",
    " .................. ",
    " .................. ",
    "  ................  ",
    "  ................  ",
    "   ..............   ",
    "     ..........     ",
    "       ......       "
};

void tst_QRegion::bitmapRegion()
{
    QBitmap circle;
    {
	QRegion region( circle );
	QVERIFY( region.isEmpty() );
    }
    {
	circle = QPixmap( circle_xpm );
	QRegion region( circle );

	//// These should not be inside the circe
	QVERIFY( !region.contains( QPoint( 2,   2 ) ) );
	QVERIFY( !region.contains( QPoint( 2,  17 ) ) );
	QVERIFY( !region.contains( QPoint( 17,  2 ) ) );
	QVERIFY( !region.contains( QPoint( 17, 17 ) ) );

	//// These should be inside
	QVERIFY( region.contains( QPoint( 3,   3 ) ) );
	QVERIFY( region.contains( QPoint( 3,  16 ) ) );
	QVERIFY( region.contains( QPoint( 16,  3 ) ) );
	QVERIFY( region.contains( QPoint( 16, 16 ) ) );

	QVERIFY( region.contains( QPoint( 0, 10 ) ) );  // Mid-left
	QVERIFY( region.contains( QPoint( 10, 0 ) ) );  // Mid-top
#if ( QT_VERSION >= 0x030100 )
	QVERIFY( region.contains( QPoint( 19, 10 ) ) ); // Mid-right
#endif
	QVERIFY( region.contains( QPoint( 10, 19 ) ) ); // Mid-bottom
    }
}

void tst_QRegion::intersected_data()
{
    QTest::addColumn<QRegion>("r1");
    QTest::addColumn<QRegion>("r2");
    QTest::addColumn<bool>("intersects");
    // QTest::addColumn<QRegion>("intersected");

    QPolygon ps1(8);
    QPolygon ps2(8);
    ps1.putPoints(0,8, 20,20, 50,20, 50,100, 70,100, 70,20, 120,20, 120,200, 20, 200);
    ps2.putPoints(0,8, 100,150, 140,150, 140,160, 160,160, 160,150, 200,150, 200,180, 100,180);
    QTest::newRow("task30716") << QRegion(ps1) << QRegion(ps2) << TRUE;
}

void tst_QRegion::intersected()
{
    QFETCH(QRegion, r1);
    QFETCH(QRegion, r2);
    QFETCH(bool, intersects);

    QRegion interReg = r1.intersected(r2);
    QVERIFY(interReg.isEmpty() != intersects);
    // Need a way to test the intersected QRegion is right
}

void tst_QRegion::intersects_region_data()
{
    QTest::addColumn<QRegion>("r1");
    QTest::addColumn<QRegion>("r2");
    QTest::addColumn<bool>("intersects");

    QTest::newRow("rect overlap rect") << QRegion(100, 100, 200, 200)
                                       << QRegion(200, 200, 200, 200)
                                       << true;

    QTest::newRow("rect not overlap rect") << QRegion(100, 100, 200, 200)
                                           << QRegion(400, 400, 200, 200)
                                           << false;

    QTest::newRow("ellipse overlap ellipse") << QRegion(100, 100, 200, 200, QRegion::Ellipse)
                                             << QRegion(200, 200, 200, 200, QRegion::Ellipse)
                                             << true;

    QTest::newRow("ellipse not overlap ellipse") << QRegion(100, 100, 200, 200, QRegion::Ellipse)
                                                 << QRegion(400, 400, 200, 200, QRegion::Ellipse)
                                                 << false;
}

void tst_QRegion::intersects_region()
{
    QFETCH(QRegion, r1);
    QFETCH(QRegion, r2);
    QFETCH(bool, intersects);
    QCOMPARE(r1.intersects(r2), intersects);
}


void tst_QRegion::intersects_rect_data()
{
    QTest::addColumn<QRegion>("region");
    QTest::addColumn<QRect>("rect");
    QTest::addColumn<bool>("intersects");

    QTest::newRow("rect overlap rect") << QRegion(100, 100, 200, 200)
                                       << QRect(200, 200, 200, 200)
                                       << true;

    QTest::newRow("rect not overlap rect") << QRegion(100, 100, 200, 200)
                                           << QRect(400, 400, 200, 200)
                                           << false;

    QTest::newRow("ellipse overlap rect") << QRegion(100, 100, 200, 200, QRegion::Ellipse)
                                          << QRect(200, 200, 200, 200)
                                          << true;

    QTest::newRow("ellipse not overlap rect") << QRegion(100, 100, 200, 200, QRegion::Ellipse)
                                              << QRect(400, 400, 200, 200)
                                              << false;
}

void tst_QRegion::intersects_rect()
{
    QFETCH(QRegion, region);
    QFETCH(QRect, rect);
    QFETCH(bool, intersects);
    QCOMPARE(region.intersects(rect), intersects);
}

void tst_QRegion::contains_point()
{
    QCOMPARE(QRegion().contains(QPoint(1,1)),false);
    QCOMPARE(QRegion(0,0,2,2).contains(QPoint(1,1)),true);
}

void tst_QRegion::operator_plus_data()
{
    QTest::addColumn<QRegion>("dest");
    QTest::addColumn<QRegion>("add");
    QTest::addColumn<QRegion>("expected");

    QTest::newRow("empty 0") << QRegion() << QRegion() << QRegion();
    QTest::newRow("empty 1") << QRegion() << QRegion(QRect(10, 10, 10, 10))
                             << QRegion(QRect(10, 10, 10, 10));
    QTest::newRow("empty 2") << QRegion(QRect(10, 10, 10, 10)) << QRegion()
                             << QRegion(QRect(10, 10, 10, 10));

    QRegion expected;
    QVector<QRect> rects;
    rects << QRect(10, 10, 10, 10) << QRect(22, 10, 10, 10);
    expected.setRects(rects.constData(), rects.size());
    QTest::newRow("non overlapping") << QRegion(10, 10, 10, 10)
                                     << QRegion(22, 10, 10, 10)
                                     << expected;
}

void tst_QRegion::operator_plus()
{
    QFETCH(QRegion, dest);
    QFETCH(QRegion, add);
    QFETCH(QRegion, expected);

    QCOMPARE(dest + add, expected);

    dest += add;
    if (dest != expected) {
        qDebug() << "dest" << dest;
        qDebug() << "expected" << expected;
    }
    QCOMPARE(dest, expected);
}

void tst_QRegion::operator_minus_data()
{
    QTest::addColumn<QRegion>("dest");
    QTest::addColumn<QRegion>("subtract");
    QTest::addColumn<QRegion>("expected");

    QTest::newRow("empty 0") << QRegion() << QRegion() << QRegion();
    QTest::newRow("empty 1") << QRegion() << QRegion(QRect(10, 10, 10, 10))
                             << QRegion();
    QTest::newRow("empty 2") << QRegion(QRect(10, 10, 10, 10)) << QRegion()
                             << QRegion(QRect(10, 10, 10, 10));

    QRegion dest;
    QVector<QRect> rects;
    rects << QRect(10, 10, 10, 10) << QRect(22, 10, 10, 10);
    dest.setRects(rects.constData(), rects.size());
    QTest::newRow("simple 1") << dest
                              << QRegion(22, 10, 10, 10)
                              << QRegion(10, 10, 10, 10);
    QTest::newRow("simple 2") << dest
                              << QRegion(10, 10, 10, 10)
                              << QRegion(22, 10, 10, 10);
}

void tst_QRegion::operator_minus()
{
    QFETCH(QRegion, dest);
    QFETCH(QRegion, subtract);
    QFETCH(QRegion, expected);

    QCOMPARE(dest - subtract, expected);

    dest -= subtract;
    QCOMPARE(dest, expected);
}

QTEST_MAIN(tst_QRegion)
#include "tst_qregion.moc"

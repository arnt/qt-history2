#include "private/qpathclipper_p.h"
#include "paths.h"

#include <QtTest/QtTest>

#include <qpainterpath.h>
#include <qpolygon.h>
#include <qdebug.h>

class tst_QPathClipper : public QObject
{
    Q_OBJECT

public:
    tst_QPathClipper();
    virtual ~tst_QPathClipper();

public slots:
    void init();
    void cleanup();
private slots:
    void clip_data();
    void clip();
    void testIntersections();
    void testIntersections2();
    void testIntersections3();
    void testIntersections4();
    void testIntersections5();
    void testIntersections6();
    void testIntersections7();
    void testIntersections8();
    void testIntersections9();
};

Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPathClipper::Operation)

tst_QPathClipper::tst_QPathClipper()
{
}

tst_QPathClipper::~tst_QPathClipper()
{
}

void tst_QPathClipper::init()
{
    // No initialisation is required
}

void tst_QPathClipper::cleanup()
{
    // No cleanup is required.
}

static QPainterPath samplePath1()
{
    QPainterPath path;
    path.moveTo(QPointF(200, 246.64789));
    path.lineTo(QPointF(200, 206.64789));
    path.lineTo(QPointF(231.42858, 206.64789));
    path.lineTo(QPointF(231.42858, 246.64789));
    path.lineTo(QPointF(200, 246.64789));
    return path;
}

static QPainterPath samplePath2()
{
    QPainterPath path;
    path.moveTo(QPointF(200, 146.64789));
    path.lineTo(QPointF(200, 106.64789));
    path.lineTo(QPointF(231.42858, 106.64789));
    path.lineTo(QPointF(231.42858, 146.64789));
    path.lineTo(QPointF(200, 146.64789));
    return path;
}

static QPainterPath samplePath3()
{
    QPainterPath path;
    path.moveTo(QPointF(231.42858, 80.933609));
    path.lineTo(QPointF(200, 80.933609));
    path.lineTo(QPointF(200, 96.64788999999999));
    path.lineTo(QPointF(231.42858, 96.64788999999999));
    path.lineTo(QPointF(231.42858, 80.933609));
    return path;
}

static QPainterPath samplePath4()
{
    QPainterPath path;
    path.moveTo(QPointF(288.571434, 80.933609));
    path.lineTo(QPointF(431.42858, 80.933609));
    path.lineTo(QPointF(431.42858, 96.64788999999999));
    path.lineTo(QPointF(288.571434, 96.64788999999999));
    path.lineTo(QPointF(288.571434, 80.933609));
    return path;
}

static QPainterPath samplePath5()
{
    QPainterPath path;
    path.moveTo(QPointF(588.571434, 80.933609));
    path.lineTo(QPointF(682.85715, 80.933609));
    path.lineTo(QPointF(682.85715, 96.64788999999999));
    path.lineTo(QPointF(588.571434, 96.64788999999999));
    path.lineTo(QPointF(588.571434, 80.933609));
    return path;
}

static QPainterPath samplePath6()
{
    QPainterPath path;
    path.moveTo(QPointF(588.571434, 80.933609));
    path.lineTo(QPointF(200, 80.933609));
    path.lineTo(QPointF(200, 446.6479));
    path.lineTo(QPointF(682.85715, 446.6479));
    path.lineTo(QPointF(682.85715, 96.64788999999999));
    path.lineTo(QPointF(731.42858, 96.64788999999999));
    path.lineTo(QPointF(731.42858, 56.64788999999999));
    path.lineTo(QPointF(588.571434, 56.64788999999999));
    path.lineTo(QPointF(588.571434, 80.933609));
    return path;
}

static QPainterPath samplePath7()
{
    QPainterPath path;
    path.moveTo(QPointF(682.85715, 206.64789));
    path.lineTo(QPointF(682.85715, 246.64789));
    path.lineTo(QPointF(588.571434, 246.64789));
    path.lineTo(QPointF(588.571434, 206.64789));
    path.lineTo(QPointF(682.85715, 206.64789));
    return path;
}

static QPainterPath samplePath8()
{
    QPainterPath path;
    path.moveTo(QPointF(682.85715, 406.64789));
    path.lineTo(QPointF(682.85715, 446.64789));
    path.lineTo(QPointF(588.571434, 446.64789));
    path.lineTo(QPointF(588.571434, 406.64789));
    path.lineTo(QPointF(682.85715, 406.64789));
    return path;
}

static QPainterPath samplePath9()
{
    QPainterPath path;
    path.moveTo(QPointF(682.85715, 426.64789));
    path.lineTo(QPointF(682.85715, 446.6479));
    path.lineTo(QPointF(568.571434, 446.6479));
    path.lineTo(QPointF(568.571434, 426.64789));
    path.lineTo(QPointF(682.85715, 426.64789));
    return path;
}

static QPainterPath samplePath10()
{
    QPainterPath path;
    path.moveTo(QPointF(511.42858, 446.6479));
    path.lineTo(QPointF(368.571434, 446.6479));
    path.lineTo(QPointF(368.571434, 426.64789));
    path.lineTo(QPointF(511.42858, 426.64789));
    path.lineTo(QPointF(511.42858, 446.6479));
    return path;
}

static QPainterPath samplePath11()
{
    QPainterPath path;
    path.moveTo(QPointF(165.71429, 338.79076));
    path.lineTo(QPointF(227.74288, 338.79076));
    path.cubicTo(QPointF(232.95048, 338.79076),
                 QPointF(237.14288, 342.88102),
                 QPointF(237.14288, 347.96176));
    path.lineTo(QPointF(237.14288, 366.76261));
    path.cubicTo(QPointF(237.14288, 371.84335),
                 QPointF(232.95048, 375.93361),
                 QPointF(227.74288, 375.93361));
    path.lineTo(QPointF(165.7142905131896, 375.93361));
    path.lineTo(QPointF(165.71429, 338.79076));
    return path;
}
static QPainterPath samplePath12()
{
    QPainterPath path;
    path.moveTo(QPointF(333.2970790633727, 61.5348679391486));
    path.cubicTo(QPointF(339.851755668807, 65.26555884471786),
                 QPointF(346.7164458828328, 69.04482864715078),
                 QPointF(353.4159970843586, 72.56059416636147));
    path.cubicTo(QPointF(353.4166971116034, 72.56155590850551),
                 QPointF(353.4173961086004, 72.56251809989483),
                 QPointF(353.4180950127331, 72.56348028832946));
    path.cubicTo(QPointF(342.4340366381152, 76.42344228577481),
                 QPointF(317.0596805768079, 94.67086588954379),
                 QPointF(309.78055, 101.00195));
    path.cubicTo(QPointF(286.0370715501102, 121.6530659984711),
                 QPointF(272.7748256344584, 134.1525788344904),
                 QPointF(250.7436468364447, 150.4434491585085));
    path.lineTo(QPointF(247.03629, 146.56585));
    path.lineTo(QPointF(240.71086, 91.501867));
    path.cubicTo(QPointF(240.71086, 91.501867),
                 QPointF(305.6382515924416, 62.21715375368672),
                 QPointF(333.297085225735, 61.53486494396167));
    return path;
}

static QPainterPath samplePath13()
{
    QPainterPath path;
    path.moveTo(QPointF(160, 200));
    path.lineTo(QPointF(100, 200));
    path.lineTo(QPointF(100, 130));
    path.lineTo(QPointF(160, 130));
    path.lineTo(QPointF(160, 200));
    return path;
}

static QPainterPath samplePath14()
{
    QPainterPath path;
    path.moveTo(QPointF(100, 180));
    path.lineTo(QPointF(100, 80));
    path.lineTo(QPointF(120, 80));
    path.lineTo(QPointF(120, 100));
    path.lineTo(QPointF(160, 100));
    path.lineTo(QPointF(160, 180));
    path.lineTo(QPointF(100, 180));
    return path;
}

void tst_QPathClipper::clip_data()
{
    //create the testtable instance and define the elements
    QTest::addColumn<QPainterPath>("subject");
    QTest::addColumn<QPainterPath>("clip");
    QTest::addColumn<QPathClipper::Operation>("op");
    QTest::addColumn<QPainterPath>("result");

    //next we fill it with data
    QTest::newRow( "simple1" )  << Paths::frame3()
                                << Paths::frame4()
                                << QPathClipper::BoolAnd
                                << samplePath1();

    QTest::newRow( "simple2" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(0, -100)
                                << QPathClipper::BoolAnd
                                << samplePath2();

    QTest::newRow( "simple3" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(0, -150)
                                << QPathClipper::BoolAnd
                                << samplePath3();

    QTest::newRow( "simple4" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(200, -150)
                                << QPathClipper::BoolAnd
                                << samplePath4();

    QTest::newRow( "simple5" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(500, -150)
                                << QPathClipper::BoolAnd
                                << samplePath5();

    QTest::newRow( "simple6" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(500, -150)
                                << QPathClipper::BoolOr
                                << samplePath6();

    QTest::newRow( "simple7" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(500, 0)
                                << QPathClipper::BoolAnd
                                << samplePath7();

    QTest::newRow( "simple8" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(500, 200)
                                << QPathClipper::BoolAnd
                                << samplePath8();

    QTest::newRow( "simple9" )  << Paths::frame3()
                                << Paths::frame4() * QTransform().translate(480, 220)
                                << QPathClipper::BoolAnd
                                << samplePath9();

    QTest::newRow( "simple10" )  << Paths::frame3()
                                 << Paths::frame4() * QTransform().translate(280, 220)
                                 << QPathClipper::BoolAnd
                                 << samplePath10();

    QTest::newRow( "simple11" )  << Paths::frame2()*QTransform().translate(40, 235)
                                 << Paths::frame1()
                                 << QPathClipper::BoolAnd
                                 << samplePath11();

    QTest::newRow( "intersection_at_edge" )  << Paths::lips()
                                             << Paths::mailbox()*QTransform().translate(-85, 34)
                                             << QPathClipper::BoolAnd
                                             << samplePath12();

    QTest::newRow( "simple_move_to1" )  << Paths::rect4()
                                       << Paths::rect2() * QTransform().translate(-20, 50)
                                       << QPathClipper::BoolAnd
                                       << samplePath13();

    QTest::newRow( "simple_move_to2" )  << Paths::rect4()
                                        << Paths::rect2() * QTransform().translate(-20, 0)
                                        << QPathClipper::BoolAnd
                                        << samplePath14();
}

void tst_QPathClipper::clip()
{
    QFETCH( QPainterPath, subject );
    QFETCH( QPainterPath, clip );
    QFETCH( QPathClipper::Operation, op );
    QFETCH( QPainterPath,  result);
    QPathClipper clipper;
    clipper.setSubjectPath(subject);
    clipper.setClipPath(clip);
    QPainterPath x = clipper.clip(op);
    QTEST( x, "result" );
}

void tst_QPathClipper::testIntersections()
{
    QPainterPath path1;
    QPainterPath path2;

    path1.addRect(0, 0, 100, 100);
    path2.addRect(20, 20, 20, 20);
    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
    QVERIFY(path1.contains(path2));
    QVERIFY(!path2.contains(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addEllipse(0, 0, 100, 100);
    path2.addEllipse(200, 200, 100, 100);
    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
    QVERIFY(!path1.contains(path2));
    QVERIFY(!path2.contains(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addEllipse(0, 0, 100, 100);
    path2.addEllipse(50, 50, 100, 100);
    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));
    QVERIFY(!path1.contains(path2));
    QVERIFY(!path2.contains(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(100, 100, 100, 100);
    path2.addRect(50, 100, 100, 20);
    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));
    QVERIFY(!path1.contains(path2));
    QVERIFY(!path2.contains(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(100, 100, 100, 100);
    path2.addRect(110, 201, 100, 20);
    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
    QVERIFY(!path1.contains(path2));
    QVERIFY(!path2.contains(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(0, 0, 100, 100);
    path2.addRect(20, 20, 20, 20);
    path2.addRect(25, 25, 5, 5);
    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
    QVERIFY(path1.contains(path2));
    QVERIFY(!path2.contains(path1));
}

void tst_QPathClipper::testIntersections2()
{
    QPainterPath path1;
    QPainterPath path2;

    path1 = QPainterPath();
    path2 = QPainterPath();

    path1.moveTo(-8,-8);
    path1.lineTo(107,-8);
    path1.lineTo(107,107);
    path1.lineTo(-8,107);

    path2.moveTo(0,0);
    path2.lineTo(100,0);
    path2.lineTo(100,100);
    path2.lineTo(0,100);
    path2.lineTo(0,0);

    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
    QVERIFY(path1.contains(path2));
    QVERIFY(!path2.contains(path1));

    path1.closeSubpath();

    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
    QVERIFY(path1.contains(path2));
    QVERIFY(!path2.contains(path1));
}

void tst_QPathClipper::testIntersections3()
{
    QPainterPath path1 = Paths::node();
    QPainterPath path2 = Paths::interRect();

    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
}

void tst_QPathClipper::testIntersections4()
{
    QPainterPath path1;
    QPainterPath path2;

    path1.moveTo(-5, 0);
    path1.lineTo(5, 0);

    path2.moveTo(0, -5);
    path2.lineTo(0, 5);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));
}

void tst_QPathClipper::testIntersections5()
{
    QPainterPath path1;
    QPainterPath path2;

    path1.addRect(0, 0, 4, 4);
    path1.addRect(2, 1, 1, 1);
    path2.addRect(0.5, 2, 1, 1);

    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
}

void tst_QPathClipper::testIntersections6()
{
    QPainterPath path1;
    QPainterPath path2;

    path1.moveTo(QPointF(-115.567, -98.3254));
    path1.lineTo(QPointF(-45.9007, -98.3254));
    path1.lineTo(QPointF(-45.9007, -28.6588));
    path1.lineTo(QPointF(-115.567, -28.6588));

    path2.moveTo(QPointF(-110, -110));
    path2.lineTo(QPointF(110, -110));
    path2.lineTo(QPointF(110, 110));
    path2.lineTo(QPointF(-110, 110));
    path2.lineTo(QPointF(-110, -110));

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));
}


void tst_QPathClipper::testIntersections7()
{
    QPainterPath path1;
    QPainterPath path2;

    path1.addRect(0, 0, 10, 10);
    path2.addRect(5, 0, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(0, 0, 10, 10);
    path2.addRect(0, 5, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(0, 0, 10, 10);
    path2.addRect(0, 0, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    ///
    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(5, 1, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(1, 5, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(1, 1, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(5, 5, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(9, 9, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(10, 10, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(11, 11, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(1, 1, 10, 10);
    path2.addRect(12, 12, 10, 10);

    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(11, 11, 10, 10);
    path2.addRect(12, 12, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();
    path2 = QPainterPath();
    path1.addRect(11, 11, 10, 10);
    path2.addRect(10, 10, 10, 10);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));
}


void tst_QPathClipper::testIntersections8()
{
    QPainterPath path1 = Paths::node() * QTransform().translate(100, 50);
    QPainterPath path2 = Paths::node() * QTransform().translate(150, 50);;

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = Paths::node();
    path2 = Paths::node();

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = Paths::node();
    path2 = Paths::node() * QTransform().translate(0, 30);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = Paths::node();
    path2 = Paths::node() * QTransform().translate(30, 0);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = Paths::node();
    path2 = Paths::node() * QTransform().translate(30, 30);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = Paths::node();
    path2 = Paths::node() * QTransform().translate(1, 1);

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));
}


void tst_QPathClipper::testIntersections9()
{
    QPainterPath path1;
    QPainterPath path2;

    path1.addRect(QRectF(-1,143, 146, 106));
    path2.addRect(QRectF(-9,145, 150, 100));

    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();;
    path2 = QPainterPath();

    path1.addRect(QRectF(-1,191, 136, 106));
    path2.addRect(QRectF(-19,194, 150, 100));
    QVERIFY(path1.intersects(path2));
    QVERIFY(path2.intersects(path1));

    path1 = QPainterPath();;
    path2 = QPainterPath();

    path1.moveTo(-1 ,  143);
    path1.lineTo(148 ,  143);
    path1.lineTo(148 ,  250);
    path1.lineTo(-1 ,  250);

    path2.moveTo(-5 ,  146);
    path2.lineTo(145 ,  146);
    path2.lineTo(145 ,  246);
    path2.lineTo(-5 ,  246);
    path2.lineTo(-5 ,  146);

    QVERIFY(!path1.intersects(path2));
    QVERIFY(!path2.intersects(path1));
}


QTEST_APPLESS_MAIN(tst_QPathClipper)


#include "tst_qpathclipper.moc"

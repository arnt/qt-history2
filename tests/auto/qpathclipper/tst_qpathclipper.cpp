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

QTEST_APPLESS_MAIN(tst_QPathClipper)


#include "tst_qpathclipper.moc"

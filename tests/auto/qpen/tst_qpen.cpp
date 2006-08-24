/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include "qpen.h"
#include "qbrush.h"

#include <qdebug.h>

//TESTED_CLASS=
//TESTED_FILES=gui/painting/qpen.h gui/painting/qpen.cpp

class tst_QPen : public QObject
{
    Q_OBJECT

public:
    tst_QPen();

private slots:
    void getSetCheck();
    void operator_eq_eq();
    void operator_eq_eq_data();

    void stream();
    void stream_data();

    void constructor();
    void constructor_data();
};

// Testing get/set functions
void tst_QPen::getSetCheck()
{
    QPen obj1;
    // qreal QPen::miterLimit()
    // void QPen::setMiterLimit(qreal)
    obj1.setMiterLimit(0.0);
    QCOMPARE(0.0, obj1.miterLimit());
    obj1.setMiterLimit(1.1);
    QCOMPARE(1.1, obj1.miterLimit());

    // qreal QPen::widthF()
    // void QPen::setWidthF(qreal)
    obj1.setWidthF(0.0);
    QCOMPARE(0.0, obj1.widthF());
    obj1.setWidthF(1.1);
    QCOMPARE(1.1, obj1.widthF());

    // int QPen::width()
    // void QPen::setWidth(int)
    obj1.setWidth(0);
    QCOMPARE(0, obj1.width());
    obj1.setWidth(INT_MIN + 1);
    QCOMPARE(INT_MIN + 1, obj1.width());
    obj1.setWidth(INT_MAX);
    QCOMPARE(INT_MAX, obj1.width());
}

Q_DECLARE_METATYPE(QPen)
Q_DECLARE_METATYPE(QBrush)

tst_QPen::tst_QPen()

{
}

void tst_QPen::operator_eq_eq_data()
{
    QTest::addColumn<QPen>("pen1");
    QTest::addColumn<QPen>("pen2");
    QTest::addColumn<bool>("isEqual");

    QTest::newRow("differentColor") << QPen(Qt::red)
				 << QPen(Qt::blue)
				 << FALSE;
    QTest::newRow("differentWidth") << QPen(Qt::red, 2)
				 << QPen(Qt::red, 3)
				 << FALSE;
    QTest::newRow("differentPenStyle") << QPen(Qt::red, 2, Qt::DashLine)
				    << QPen(Qt::red, 2, Qt::DotLine)
				    << FALSE;
    QTest::newRow("differentCapStyle") << QPen(Qt::red, 2, Qt::DashLine, Qt::RoundCap, Qt::BevelJoin)
				    << QPen(Qt::red, 2, Qt::DashLine, Qt::SquareCap, Qt::BevelJoin)
				    << FALSE;
    QTest::newRow("differentJoinStyle") << QPen(Qt::red, 2, Qt::DashLine, Qt::RoundCap, Qt::BevelJoin)
				     << QPen(Qt::red, 2, Qt::DashLine, Qt::RoundCap, Qt::MiterJoin)
				     << FALSE;
    QTest::newRow("same") << QPen(Qt::red, 2, Qt::DashLine, Qt::RoundCap, Qt::BevelJoin)
		       << QPen(Qt::red, 2, Qt::DashLine, Qt::RoundCap, Qt::BevelJoin)
		       << TRUE;

}

void tst_QPen::operator_eq_eq()
{
    QFETCH(QPen, pen1);
    QFETCH(QPen, pen2);
    QFETCH(bool, isEqual);
    QCOMPARE(pen1 == pen2, isEqual);
}


void tst_QPen::constructor_data()
{
    QTest::addColumn<QPen>("pen");
    QTest::addColumn<QBrush>("brush");
    QTest::addColumn<double>("width");
    QTest::addColumn<int>("style");
    QTest::addColumn<int>("capStyle");
    QTest::addColumn<int>("joinStyle");

    QTest::newRow("solid_black") << QPen() << QBrush(Qt::black) << 0. << (int)Qt::SolidLine
                              << (int) Qt::SquareCap << (int)Qt::BevelJoin;
    QTest::newRow("solid_red") << QPen(Qt::red) << QBrush(Qt::red) << 0. << (int)Qt::SolidLine
                            << (int)Qt::SquareCap << (int)Qt::BevelJoin;
    QTest::newRow("full") << QPen(QBrush(QLinearGradient(0, 0, 100, 100)), 10,
                               Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin)
                       << QBrush(QLinearGradient(0, 0, 100, 100)) << 10. << (int)Qt::SolidLine
                       << (int)Qt::RoundCap << (int)Qt::MiterJoin;

}


void tst_QPen::constructor()
{
    QFETCH(QPen, pen);
    QFETCH(QBrush, brush);
    QFETCH(double, width);
    QFETCH(int, style);
    QFETCH(int, capStyle);
    QFETCH(int, joinStyle);

    QCOMPARE(pen.style(), Qt::PenStyle(style));
    QCOMPARE(pen.capStyle(), Qt::PenCapStyle(capStyle));
    QCOMPARE(pen.joinStyle(), Qt::PenJoinStyle(joinStyle));
    QCOMPARE(pen.widthF(), width);
    QCOMPARE(pen.brush(), brush);
}


void tst_QPen::stream_data()
{
    QTest::addColumn<QPen>("pen");

    QTest::newRow("solid_black") << QPen();
    QTest::newRow("solid_red") << QPen(Qt::red);
    QTest::newRow("full") << QPen(QBrush(QLinearGradient(0, 0, 100, 100)), 10, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
}


void tst_QPen::stream()
{
    QFETCH(QPen, pen);

    QByteArray bytes;

    {
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << pen;
    }

    QPen cmp;
    {
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        stream >> cmp;
    }

    QCOMPARE(pen.widthF(), cmp.widthF());
    QCOMPARE(pen.style(), cmp.style());
    QCOMPARE(pen.capStyle(), cmp.capStyle());
    QCOMPARE(pen.joinStyle(), cmp.joinStyle());
    QCOMPARE(pen.brush(), cmp.brush());

    QCOMPARE(pen, cmp);
}


QTEST_APPLESS_MAIN(tst_QPen)
#include "tst_qpen.moc"

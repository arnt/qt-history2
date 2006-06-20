/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qpoint.h>


//TESTED_CLASS=
//TESTED_FILES=qpoint.h

class tst_QPoint : public QObject
{
Q_OBJECT

public:
    tst_QPoint();
    virtual ~tst_QPoint();

private slots:
    void getSetCheck();
};

tst_QPoint::tst_QPoint()
{
}

tst_QPoint::~tst_QPoint()
{
}

// Testing get/set functions
void tst_QPoint::getSetCheck()
{
    QPoint obj1;
    // int QPoint::x()
    // void QPoint::setX(int)
    obj1.setX(0);
    QCOMPARE(0, obj1.x());
    obj1.setX(INT_MIN);
    QCOMPARE(INT_MIN, obj1.x());
    obj1.setX(INT_MAX);
    QCOMPARE(INT_MAX, obj1.x());

    // int QPoint::y()
    // void QPoint::setY(int)
    obj1.setY(0);
    QCOMPARE(0, obj1.y());
    obj1.setY(INT_MIN);
    QCOMPARE(INT_MIN, obj1.y());
    obj1.setY(INT_MAX);
    QCOMPARE(INT_MAX, obj1.y());

    QPointF obj2;
    // qreal QPointF::x()
    // void QPointF::setX(qreal)
    obj2.setX(0.0);
    QCOMPARE(0.0, obj2.x());
    obj2.setX(1.1);
    QCOMPARE(1.1, obj2.x());

    // qreal QPointF::y()
    // void QPointF::setY(qreal)
    obj2.setY(0.0);
    QCOMPARE(0.0, obj2.y());
    obj2.setY(1.1);
    QCOMPARE(1.1, obj2.y());
}

QTEST_MAIN(tst_QPoint)
#include "tst_qpoint.moc"

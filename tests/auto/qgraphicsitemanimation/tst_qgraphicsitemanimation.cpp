/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if QT_VERSION < 0x040200
QTEST_NOOP_MAIN
#else

#include <qgraphicsitemanimation.h>

#include <QtCore/qtimeline.h>
#include <QtGui/qmatrix.h>

//TESTED_CLASS=QGraphicsItemAnimation
//TESTED_FILES=gui/graphicsview/qgraphicsitemanimation.cpp

class tst_QGraphicsItemAnimation : public QObject
{
    Q_OBJECT

private slots:
    void construction();
    void linearMove();
    void linearRotation();
};

void tst_QGraphicsItemAnimation::construction()
{
    QGraphicsItemAnimation animation;
    QVERIFY(!animation.item());
    QVERIFY(!animation.timeLine());
    QCOMPARE(animation.posAt(0), QPointF());
    QCOMPARE(animation.posAt(0.5), QPointF());
    QCOMPARE(animation.posAt(1), QPointF());
    QCOMPARE(animation.matrixAt(0), QMatrix());
    QCOMPARE(animation.matrixAt(0.5), QMatrix());
    QCOMPARE(animation.matrixAt(1), QMatrix());
    QCOMPARE(animation.rotationAt(0), 0.0);
    QCOMPARE(animation.rotationAt(0.5), 0.0);
    QCOMPARE(animation.rotationAt(1), 0.0);
    QCOMPARE(animation.xTranslationAt(0), 0.0);
    QCOMPARE(animation.xTranslationAt(0.5), 0.0);
    QCOMPARE(animation.xTranslationAt(1), 0.0);
    QCOMPARE(animation.yTranslationAt(0), 0.0);
    QCOMPARE(animation.yTranslationAt(0.5), 0.0);
    QCOMPARE(animation.yTranslationAt(1), 0.0);
    QCOMPARE(animation.verticalScaleAt(0), 1.0);
    QCOMPARE(animation.horizontalScaleAt(0), 1.0);
    QCOMPARE(animation.verticalShearAt(0), 0.0);
    QCOMPARE(animation.horizontalShearAt(0), 0.0);
    animation.clear(); // don't crash
}

void tst_QGraphicsItemAnimation::linearMove()
{
    QGraphicsItemAnimation animation;

    for (int i = 0; i <= 10; ++i) {
        QCOMPARE(animation.posAt(i / 10.0).x(), qreal(0));
        QCOMPARE(animation.posAt(i / 10.0).y(), qreal(0));
    }

    animation.setPosAt(1, QPointF(10, -10));

    for (int i = 0; i <= 10; ++i) {
        QCOMPARE(animation.posAt(i / 10.0).x(), qreal(i));
        QCOMPARE(animation.posAt(i / 10.0).y(), qreal(-i));
    }

    animation.setPosAt(2, QPointF(10, -10));

    QCOMPARE(animation.posAt(11).x(), qreal(10));
}

void tst_QGraphicsItemAnimation::linearRotation()
{
    QGraphicsItemAnimation animation;
    animation.setRotationAt(1, 1);

    for (int i = 0; i <= 10; ++i)
        QCOMPARE(animation.rotationAt(i / 10.0), qreal(i / 10.0));
}

QTEST_MAIN(tst_QGraphicsItemAnimation)
#include "tst_qgraphicsitemanimation.moc"
#endif

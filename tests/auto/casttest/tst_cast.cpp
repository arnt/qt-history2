#include <QtTest/QtTest>
#include <QPointF>
#include <QPoint>
#include <QRectF>
#include <QRect>

class tst_CastTest: public QObject
{
    Q_OBJECT
private slots:
    void testCasts();
};

void tst_CastTest::testCasts()
{
    QPointF pointfs[10];
    QPoint points[10];
    QRectF rectfs[10];
    QRect rects[10];

    if (sizeof(qreal) == sizeof(double)) {
        QVERIFY(sizeof(QPointF) == 16);
        QVERIFY(sizeof(pointfs) == 160);

        QVERIFY(sizeof(QRectF) == 32);
        QVERIFY(sizeof(rectfs) == 320);
    } else {
        QVERIFY(sizeof(QPointF) == 8);
        QVERIFY(sizeof(pointfs) == 80);

        QVERIFY(sizeof(QRectF) == 16);
        QVERIFY(sizeof(rectfs) == 160);
    }

    QVERIFY(sizeof(QPoint) == 8);
    QVERIFY(sizeof(points) == 80);
    
    QVERIFY(sizeof(QRect) == 16);
    QVERIFY(sizeof(rects) == 160);
}

QTEST_MAIN(tst_CastTest)
#include "tst_cast.moc"

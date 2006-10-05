#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QVector>
#include <qdebug.h>
#include <qpolygon.h>
#include <qmatrix.h>

#include "oldtessellator.h"
#include "testtessellator.h"
#include "utils.h"
#include "simple.h"
#include "arc.h"


//TESTED_CLASS=
//TESTED_FILES=

class tst_QTessellator : public QObject
{
    Q_OBJECT

public:
    tst_QTessellator() {
    }

private slots:
    void testStandardSet();
    void testRandom();
    void testArc();
};


QPointF creatPoint()
{
    double x = int(20.0 * (rand() / (RAND_MAX + 1.0)));
    double y = int(20.0 * (rand() / (RAND_MAX + 1.0)));
    return QPointF(x, y);
}

bool test(const QPointF *pg, int pgSize, bool winding)
{
    QVector<XTrapezoid> traps;
    double area1 = 0;
    double area2 = 0;

    old_tesselate_polygon(&traps, pg, pgSize, winding);
    area1 = compute_area_for_x(traps);

    traps.clear();

    test_tesselate_polygon(&traps, pg, pgSize, winding);
    area2 = compute_area_for_x(traps);

    bool result = (area2 - area1 < 0.005);
    if (!result && area1)
        result = (qAbs(area1 - area2)/area1 < .005);

//     qDebug() << area1 << area2 << result;

    return result;
}


void simplifyTestFailure(QVector<QPointF> failure, bool winding)
{
    int i = 1;
    while (i < failure.size() - 1) {
        QVector<QPointF> t = failure;
        t.remove(i);
        if (test(t.data(), t.size(), winding)) {
            ++i;
            continue;
        }
        failure = t;
        i = 1;
    }

    for (int x = 0; x < failure.size(); ++x) {
        fprintf(stderr, "%lf,%lf, ", failure[x].x(), failure[x].y());
    }
    fprintf(stderr, "\n\n");
}

void tst_QTessellator::testStandardSet()
{
    QVector<FullData> sampleSet;
    sampleSet.append(simpleData());

    foreach(FullData data, sampleSet) {
        for (int i = 0; i < data.size(); ++i) {
            if (!test(data[i].data(), data[i].size(), false)) {
                simplifyTestFailure(data[i], false);
                QCOMPARE(true, false);
            }
            if (!test(data[i].data(), data[i].size(), true)) {
                simplifyTestFailure(data[i], true);
                QCOMPARE(true, false);
            }
        }
    }
}



void fillRandomVec(QVector<QPointF> &vec)
{
    int size = vec.size(); --size;
    for (int i = 0; i < size; ++i) {
        vec[i] = creatPoint();
    }
    vec[size] = vec[0];
}

void tst_QTessellator::testRandom()
{
    int failures = 0;
    for (int i = 5; i < 12; ++i) {
        QVector<QPointF> vec(i);
        int k = 5000;
        while (--k) {
            fillRandomVec(vec);
            if (!test(vec.data(), vec.size(), false)) {
                simplifyTestFailure(vec, false);
                ++failures;
            }
            if (!test(vec.data(), vec.size(), true)) {
                simplifyTestFailure(vec, true);
                ++failures;
            }
        }
    }
    QVERIFY(failures == 0);
}


// we need a higher threshold for failure here than in the above tests, as this basically draws
// a very thin outline, where the discretization in the new tesselator shows
bool test_arc(const QPolygonF &poly, bool winding)
{
    QVector<XTrapezoid> traps;
    double area1 = 0;
    double area2 = 0;

    old_tesselate_polygon(&traps, poly.data(), poly.size(), winding);
    area1 = compute_area_for_x(traps);

    traps.clear();

    test_tesselate_polygon(&traps, poly.data(), poly.size(), winding);
    area2 = compute_area_for_x(traps);

    bool result = (area2 - area1 < .02);
    if (!result && area1)
        result = (qAbs(area1 - area2)/area1 < .02);

    return result;
}



void tst_QTessellator::testArc()
{
    FullData arc = arcData();

    QMatrix mat;
    for (int i = 0; i < 1000; ++i) {
        mat.rotate(.01);
        mat.scale(.99, .99);
        QPolygonF poly = arc.at(0);
        QPolygonF vec = poly * mat;
        QVERIFY(test_arc(vec, true));
        QVERIFY(test_arc(vec, false));
    }
}



QTEST_MAIN(tst_QTessellator)
#include "tst_tessellator.moc"

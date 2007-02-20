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

#include "math.h"

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
    void testRects();
    void testConvexRects();
    void testConvex();
};


QPointF creatPoint()
{
    double x = int(20.0 * (rand() / (RAND_MAX + 1.0)));
    double y = int(20.0 * (rand() / (RAND_MAX + 1.0)));
    return QPointF(x, y);
}

bool test(const QPointF *pg, int pgSize, bool winding, tessellate_function tessellate = test_tesselate_polygon, qreal maxDiff = 0.005)
{
    QVector<XTrapezoid> traps;
    double area1 = 0;
    double area2 = 0;

    old_tesselate_polygon(&traps, pg, pgSize, winding);
    area1 = compute_area_for_x(traps);

    traps.clear();

    tessellate(&traps, pg, pgSize, winding);
    area2 = compute_area_for_x(traps);

    bool result = (qAbs(area2 - area1) < maxDiff);
    if (!result && area1)
        result = (qAbs(area1 - area2)/area1 < maxDiff);

    if (!result)
        qDebug() << area1 << area2;

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

static bool isConvex(const QVector<QPointF> &v)
{
    int nPoints = v.size() - 1;

    qreal lastCross = 0;
    for (int i = 0; i < nPoints; ++i) {
        QPointF a = v[i];
        QPointF b = v[(i + 1) % nPoints];

        QPointF d1 = b - a;

        for (int j = 0; j < nPoints; ++j) {
            if (j == i || j == i + 1)
                continue;

            QPointF p = v[j];
            QPointF d2 = p - a;

            qreal cross = d1.x() * d2.y() - d1.y() * d2.x();

            if (!qFuzzyCompare(cross, (qreal)0)
                && !qFuzzyCompare(cross, (qreal)0)
                && (lastCross > 0) != (cross > 0))
                return false;

            lastCross = cross;
        }
    }

    return true;
}

static void fillRectVec(QVector<QPointF> &v)
{
    int numRects = v.size() / 5;

    int first = 0;
    v[first++] = QPointF(0, 0);
    v[first++] = QPointF(10, 0);
    v[first++] = QPointF(10, 10);
    v[first++] = QPointF(0, 10);
    v[first++] = QPointF(0, 0);

    v[first++] = QPointF(0, 0);
    v[first++] = QPointF(2, 2);
    v[first++] = QPointF(4, 0);
    v[first++] = QPointF(2, -2);
    v[first++] = QPointF(0, 0);

    v[first++] = QPointF(0, 0);
    v[first++] = QPointF(4, 4);
    v[first++] = QPointF(6, 2);
    v[first++] = QPointF(2, -2);
    v[first++] = QPointF(0, 0);

    for (int i = first / 5; i < numRects; ++i) {
        QPointF a = creatPoint();
        QPointF b = creatPoint();

        QPointF delta = a - b;
        QPointF perp(delta.y(), -delta.x());

        perp *= ((int)(20.0 * rand() / (RAND_MAX + 1.0))) / 20.0;

        int j = 5 * i;
        v[j++] = a + perp;
        v[j++] = a - perp;
        v[j++] = b - perp;
        v[j++] = b + perp;
        v[j++] = a + perp;
    }
}

const int numRects = 5000;

void tst_QTessellator::testConvexRects()
{
    return;
    int failures = 0;
    QVector<QPointF> vec(numRects * 5);
    fillRectVec(vec);
    for (int rect = 0; rect < numRects; ++rect) {
        QVector<QPointF> v(5);
        for (int i = 0; i < 5; ++i)
            v[i] = vec[5 * rect + i];
        if (!test(v.data(), v.size(), false, test_tessellate_polygon_convex)) {
            simplifyTestFailure(v, false);
            ++failures;
        }
        if (!test(v.data(), v.size(), true, test_tessellate_polygon_convex)) {
            simplifyTestFailure(v, true);
            ++failures;
        }
    }
    QVERIFY(failures == 0);
}

void tst_QTessellator::testConvex()
{
    int failures = 0;
    for (int i = 4; i < 10; ++i) {
        QVector<QPointF> vec(i);
        int k = 5000;
        while (k--) {
            fillRandomVec(vec);
            if (!isConvex(vec))
                continue;
            if (!test(vec.data(), vec.size(), false, test_tessellate_polygon_convex)) {
                simplifyTestFailure(vec, false);
                ++failures;
            }
            if (!test(vec.data(), vec.size(), true, test_tessellate_polygon_convex)) {
                simplifyTestFailure(vec, true);
                ++failures;
            }
        }
    }
    QVERIFY(failures == 0);
}


void tst_QTessellator::testRects()
{
    int failures = 0;
    QVector<QPointF> vec(numRects * 5);
    fillRectVec(vec);
    for (int rect = 0; rect < numRects; ++rect) {
        QVector<QPointF> v(5);
        for (int i = 0; i < 5; ++i)
            v[i] = vec[5 * rect + i];
        if (!test(v.data(), v.size(), false, test_tessellate_polygon_rect, 0.05)) {
            simplifyTestFailure(v, false);
            ++failures;
        }
        if (!test(v.data(), v.size(), true, test_tessellate_polygon_rect, 0.05)) {
            simplifyTestFailure(v, true);
            ++failures;
        }
    }
    QVERIFY(failures == 0);
}


QTEST_MAIN(tst_QTessellator)
#include "tst_tessellator.moc"

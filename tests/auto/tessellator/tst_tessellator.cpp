#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QVector>
#include <qdebug.h>

#include "ltessellator.h"
#include "qttesselator.h"
#include "utils.h"
#include "simple.h"


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

    qt_tesselate_polygon(&traps, pg, pgSize, winding);
    area1 = compute_area_for_x(traps);

    traps.clear();

    l_tesselate_polygon(&traps, pg, pgSize, winding);
    area2 = compute_area_for_x(traps);

    bool result = (area2 - area1 < 0.005);
    if (!result && area1)
        result = (qAbs(area1 - area2)/area1 < .005);

//     qDebug() << area1 << area2 << result;

    if (!result)
        Q_ASSERT(false);
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
        int i = 5000;
        while (--i) {
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
    QCOMPARE(failures, 0);
}

QTEST_MAIN(tst_QTessellator)
#include "tst_tessellator.moc"

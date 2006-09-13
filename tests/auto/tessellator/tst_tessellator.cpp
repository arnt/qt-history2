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

bool test(const QPointF *pg, int pgSize)
{
    QVector<XTrapezoid> traps;
    double area1 = 0;
    double area2 = 0;

    qt_tesselate_polygon(&traps, pg, pgSize, true);
    area1 = compute_area_for_x(traps);

    traps.clear();

    l_tesselate_polygon(&traps, pg, pgSize, true);

    area2 = compute_area_for_x(traps);

    bool result;
    if (area1 < 1)
        result = (qAbs(area1 - area2) < 1.0);
    else
        result = (qAbs(area1 - area2)/area1 < .02);

    return result;
}


void simplifyTestFailure(QVector<QPointF> failure)
{
    int i = 1;
    while (i < failure.size() - 1) {
        QVector<QPointF> t = failure;
        t.remove(i);
        if (test(t.data(), t.size())) {
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
            if (!test(data[i].data(), data[i].size())) {
                simplifyTestFailure(data[i]);
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
    QVector<QPointF> vec(8);
    int failures = 0;
    int i = 10000;
    while (--i) {
        fillRandomVec(vec);
        if (!test(vec.data(), vec.size())) {
            simplifyTestFailure(vec);
            ++failures;
        }
    }
    if (failures > 0)
        qDebug("random test had %d failures", failures);
}

QTEST_MAIN(tst_QTessellator)
#include "tst_tessellator.moc"

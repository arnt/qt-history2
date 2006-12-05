#ifndef TESTTESSELLATOR_H
#define TESTTESSELLATOR_H

#include <QVector>
#include <QPointF>
#include "XrenderFake.h"


void test_tesselate_polygon(QVector<XTrapezoid> *traps, const QPointF *points, int nPoints,
                            bool winding);

#endif

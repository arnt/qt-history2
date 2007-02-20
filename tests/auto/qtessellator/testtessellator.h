#ifndef TESTTESSELLATOR_H
#define TESTTESSELLATOR_H

#include <QVector>
#include <QPointF>
#include "XrenderFake.h"

typedef void (*tessellate_function)(QVector<XTrapezoid> *traps, const QPointF *points, int nPoints,
                                    bool winding);

void test_tesselate_polygon(QVector<XTrapezoid> *traps, const QPointF *points, int nPoints,
                            bool winding);

void test_tessellate_polygon_convex(QVector<XTrapezoid> *traps, const QPointF *points, int nPoints,
                                    bool winding);

void test_tessellate_polygon_rect(QVector<XTrapezoid> *traps, const QPointF *points, int nPoints,
                                  bool winding);
#endif

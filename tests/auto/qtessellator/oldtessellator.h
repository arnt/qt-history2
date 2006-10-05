#ifndef NEWTESSELATOR_H
#define NEWTESSELATOR_H

#include <QVector>
#include <QPointF>
#include "XrenderFake.h"

void old_tesselate_polygon(QVector<XTrapezoid> *traps, const QPointF *pg, int pgSize,
                           bool winding);


#endif

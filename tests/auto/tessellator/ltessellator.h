#ifndef LTESSELLATOR_H
#define LTESSELLATOR_H

#include <QVector>
#include <QPointF>
#include "XrenderFake.h"

void l_tesselate_polygon(QVector<XTrapezoid> *traps, const QPointF *pg, int pgSize,
                         bool winding);

#endif

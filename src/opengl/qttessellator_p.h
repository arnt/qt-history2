#ifndef QTTESSELLATOR_H_H
#define QTTESSELLATOR_H_H

#include <QVector>
#include <QPointF>
#include <QRectF>

typedef int qt_XFixed;
typedef double	qt_XDouble;

#define qt_XDoubleToFixed(f)    ((qt_XFixed) ((f) * 65536))
#define qt_XFixedToDouble(f)    (((qt_XDouble) (f)) / 65536)

struct qt_XPointFixed {
    qt_XFixed  x, y;
};

struct qt_XLineFixed {
    qt_XPointFixed p1, p2;
};

struct qt_XTrapezoid {
    qt_XFixed     top, bottom;
    qt_XLineFixed left, right;
};

void qt_polygon_trapezoidation(QVector<qt_XTrapezoid> *traps,
                               const QPointF *pg, int pgSize,
                               bool winding, QRect *br);



#endif

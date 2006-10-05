#include "utils.h"

#include <assert.h>
#include <qglobal.h>

#include <qnum.h>

#define FloatToXFixed(i) (int)((i) * 65536)
#define IntToXFixed(i) ((i) << 16)

static double compute_x_at(XFixed y, XPointFixed p1, XPointFixed p2)
{
    double d = XFixedToDouble(p2.x - p1.x);
    return
        XFixedToDouble(p1.x) + d*XFixedToDouble(y - p1.y)/XFixedToDouble(p2.y - p1.y);
}

double compute_area(XTrapezoid *trap)
{
    double x1 = compute_x_at(trap->top, trap->left.p1, trap->left.p2);
    double x2 = compute_x_at(trap->top, trap->right.p1, trap->right.p2);
    double x3 = compute_x_at(trap->bottom, trap->left.p1, trap->left.p2);
    double x4 = compute_x_at(trap->bottom, trap->right.p1, trap->right.p2);

    double top = XFixedToDouble(trap->top);
    double bottom = XFixedToDouble(trap->bottom);
    double h = bottom - top;

    double top_base = x2 - x1;
    double bottom_base = x4 - x3;

    if ((top_base < 0 && bottom_base > 0)
        || (top_base > 0 && bottom_base < 0)) {
        double y0 = top_base*h/(top_base - bottom_base) + top;
        double area = qAbs(top_base * (y0 - top) / 2.);
        area += qAbs(bottom_base * (bottom - y0) /2.);
        return area;
    }


    return 0.5 * h * qAbs(top_base + bottom_base);
}

double compute_area_for_x(const QVector<XTrapezoid> &traps)
{
    double area = 0;

    for (int i = 0; i < traps.size(); ++i) {
        XTrapezoid trap = traps[i];
        area += compute_area(&trap);
    }
    return area;
}

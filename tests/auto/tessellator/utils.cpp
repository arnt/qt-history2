#include "utils.h"

#include <assert.h>
#include <qglobal.h>

#include <cmath>

#define FloatToXFixed(i) (int)((i) * 65536)
#define IntToXFixed(i) ((i) << 16)

static double compute_x_at(XFixed y, XPointFixed p1, XPointFixed p2)
{
    if ((p2.x - p1.x) == 0)
        return XFixedToDouble(p2.x);

    double m1 = XFixedToDouble(p2.y - p1.y)/XFixedToDouble(p2.x - p1.x);
    double b1 = XFixedToDouble(p1.y) - m1*XFixedToDouble(p1.x);

    return (XFixedToDouble(y)-b1)/m1;
}

double compute_area(XTrapezoid *trap)
{
    double x1 = compute_x_at(trap->top, trap->left.p1, trap->left.p2);
    double x2 = compute_x_at(trap->top, trap->right.p1, trap->right.p2);
    double x3 = compute_x_at(trap->bottom, trap->left.p1, trap->left.p2);
    double x4 = compute_x_at(trap->bottom, trap->right.p1, trap->right.p2);

    double top_base = qAbs(x2 - x1);
    double bottom_base = qAbs(x4 - x3);
    double h = (XFixedToDouble(trap->bottom) - XFixedToDouble(trap->top));

    double area = 0.5 * h *(top_base + bottom_base);

    if (area < 0 || std::isnan(area)) {
#if 1
        fprintf(stderr, "area is %f: (h=%f, top=%f, bottom=%f)\n",
                area, h, top_base, bottom_base);
        fprintf(stderr, "\tx1 = %f, x2 = %f, x3 = %f, x4 = %f\n",
                x1, x2, x3, x4);
#endif
        //assert(area>=0);
    }
    return area;
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

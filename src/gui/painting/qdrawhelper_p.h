#ifndef QDRAWHELPER_H
#define QDRAWHELPER_H

#include <qglobal.h>
#include <qcolor.h>


/*******************************************************************************
 * ARGB
 */
struct ARGB
{
    ARGB(uchar alpha, uchar red, uchar green, uchar blue) : b(blue), g(green), r(red), a(alpha) { }
    ARGB(uint rgba) { *((uint*)this) = rgba; }
    ARGB(const QColor &c) : b(c.blue()), g(c.green()), r(c.red()), a(c.alpha()) { }
    ARGB() : b(0), g(0), r(0), a(0) { }
    uchar b;
    uchar g;
    uchar r;
    uchar a;

    QRgb toRgba() { return qRgba(r, g, b, a); }
};


/*******************************************************************************
 * QSpan
 *
 * duplicate definition of FT_Span
 */
struct QSpan
{
    short x;
    ushort len;
    uchar coverage;
};


typedef void (*BlendColor)(ARGB *target, const QSpan *span, ARGB color);
typedef void (*BlendTransformed)(ARGB *target, const QSpan *span,
                                         qreal ix, qreal iy, qreal dx, qreal dy,
                                         ARGB *image_bits, int image_width, int image_height);

struct DrawHelper {
    BlendColor blendColor;
    BlendTransformed blendTransformed;
    BlendTransformed blendTransformedTiled;
    BlendTransformed blendTransformedBilinear;
    BlendTransformed blendTransformedBilinearTiled;
};

extern DrawHelper qDrawHelper;

void qInitAsm(DrawHelper *);

inline int qt_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }
#define qt_alpha_pixel(s, t, a, ra) { int tmp = s*a + t*ra; t = qt_div_255(tmp); }
#define qt_alpha_pixel_pm(s, t, ra) { int tmp = s + t*ra; t = qt_div_255(tmp); }

static inline void qt_blend_pixel(ARGB src, ARGB *target, int coverage)
{
    int alpha = qt_div_255(coverage * src.a);
    int rev_alpha = 255 - alpha;

    qt_alpha_pixel(src.r, target->r, alpha, rev_alpha);
    qt_alpha_pixel(src.g, target->g, alpha, rev_alpha);
    qt_alpha_pixel(src.b, target->b, alpha, rev_alpha);

    target->a = 255;
}


#endif

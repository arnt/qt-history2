#ifndef QDRAWHELPER_H
#define QDRAWHELPER_H

#include <qglobal.h>
#include <qcolor.h>


/*******************************************************************************
 * ARGB
 *
 * This does not cover the runtime byte order checks, but for QImage it should
 * work perfectly since QImage client side anyway.
 */
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
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
#else
struct ARGB
{
    ARGB(uchar alpha, uchar red, uchar green, uchar blue) : a(alpha), r(red), g(green), b(blue) { }
    ARGB(uint rgba) { *((uint*)this) = rgba; }
    ARGB(const QColor &c) : a(c.alpha()), r(c.red()), g(c.green()), b(c.blue()) { }
    ARGB() : a(0), r(0), g(0), b(0) { }
    uchar a;
    uchar r;
    uchar g;
    uchar b;
    QRgb toRgba() { return qRgba(r, g, b, a); }
};
#endif


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
    enum Layout {
        Layout_ARGB,
        Layout_RGB32,
        Layout_Count
    };
    BlendColor blendColor;
    BlendTransformed blendTransformed;
    BlendTransformed blendTransformedTiled;
    BlendTransformed blendTransformedBilinear;
    BlendTransformed blendTransformedBilinearTiled;
};

extern DrawHelper qDrawHelper[DrawHelper::Layout_Count];

void qInitDrawhelperAsm();

inline int qt_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }
#define qt_alpha_pixel(s, t, a, ra) { int tmp = s*a + t*ra; t = qt_div_255(tmp); }
#define qt_alpha_pixel_pm(s, t, ra) { int tmp = s + t*ra; t = qt_div_255(tmp); }

#define qt_blend_pixel(src, target, coverage) \
    {                                                             \
        int alpha = qt_div_255(coverage * src.a);                 \
        if (alpha == 255) {                                       \
            *target = src;                                        \
        } else {                                                  \
            int rev_alpha = 255 - alpha;                               \
            int res_alpha = alpha + qt_div_255(rev_alpha * target->a); \
                                                                       \
            target->a = res_alpha;                                     \
            if (res_alpha == 255) {                                    \
                qt_alpha_pixel(src.r, target->r, alpha, rev_alpha);    \
                qt_alpha_pixel(src.g, target->g, alpha, rev_alpha);    \
                qt_alpha_pixel(src.b, target->b, alpha, rev_alpha);    \
            } else if (res_alpha != 0) {                                \
                rev_alpha *= target->a;                                 \
                target->r = (src.r * alpha + qt_div_255(rev_alpha * target->r)) / res_alpha; \
                target->g = (src.g * alpha + qt_div_255(rev_alpha * target->g)) / res_alpha; \
                target->b = (src.b * alpha + qt_div_255(rev_alpha * target->b)) / res_alpha; \
            }                                                           \
        }                                                               \
    }

static inline void qt_blend_pixel_premul(int pr, int pg, int pb, int alpha, ARGB *target)
{
    int rev_alpha = 255 - alpha;
    int res_alpha = alpha + qt_div_255(rev_alpha * target->a);

    target->a = res_alpha;
    if (res_alpha == 255) {
        qt_alpha_pixel_pm(pr, target->r, rev_alpha);
        qt_alpha_pixel_pm(pg, target->g, rev_alpha);
        qt_alpha_pixel_pm(pb, target->b, rev_alpha);
    } else if (res_alpha != 0) {
        rev_alpha *= target->a;
        target->r = (pr + qt_div_255(rev_alpha * target->r)) / res_alpha;
        target->g = (pg + qt_div_255(rev_alpha * target->g)) / res_alpha;
        target->b = (pb + qt_div_255(rev_alpha * target->b)) / res_alpha;
    }
}

#endif

#include <private/qdrawhelper_p.h>

#if 0//def __x86__
#include "qdrawhelper_x86.cpp"
#else
void qInitDrawhelperAsm() {}
#endif

#include <math.h>
#include <private/qmath_p.h>
#define MASK(src, a) src = BYTE_MUL(src, a)

static const int fixed_scale = 1 << 16;
static const int half_point = 1 << 15;

typedef uint QT_FASTCALL (*CompositionFunction)(uint dest, uint src);

static uint QT_FASTCALL comp_func_Clear(uint, uint)
{
    return 0;
}

/*
Dca' = Sca.Da + Sca.(1 - Da)
     = Sca
Da'  = Sa.Da + Sa.(1 - Da)
     = Sa
*/
static inline uint QT_FASTCALL comp_func_Source(uint, uint src)
{
    return src;
}

static inline uint QT_FASTCALL comp_func_Destination(uint dest, uint)
{
    return dest;
}
/*
Dca' = Sca.Da + Sca.(1 - Da) + Dca.(1 - Sa)
     = Sca + Dca.(1 - Sa)
Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
     = Sa + Da - Sa.Da
*/
static inline uint QT_FASTCALL comp_func_SourceOver(uint dest, uint src)
{
    return src + BYTE_MUL(dest, 255 - qAlpha(src));
}

static inline uint QT_FASTCALL comp_func_DestinationOver(uint dest, uint src)
{
    return comp_func_SourceOver(src, dest);
}

/*
  Dca' = Sca.Da
  Da'  = Sa.Da
*/
static inline uint QT_FASTCALL comp_func_SourceIn(uint dest, uint src)
{
    int dest_a = qAlpha(dest);

    return BYTE_MUL(src, dest_a);
}

static inline uint QT_FASTCALL comp_func_DestinationIn(uint dest, uint src)
{
    return comp_func_SourceIn(src, dest);
}

/*
 Dca' = Sca.(1 - Da)
 Da'  = Sa.(1 - Da)
*/
static inline uint QT_FASTCALL comp_func_SourceOut(uint dest, uint src)
{
    int dest_ia  = (255 - qAlpha(dest));

    return BYTE_MUL(src, dest_ia);
}


static inline uint QT_FASTCALL comp_func_DestinationOut(uint dest, uint src)
{
    return comp_func_SourceOut(src, dest);
}

/*
  Dca' = Sca.Da + Dca.(1 - Sa)
  Dca' = Da.(Sca + Dc.(1 - Sa))
  Da'  = Sa.Da + Da.(1 - Sa)
       = Da
*/
static inline uint QT_FASTCALL comp_func_SourceAtop(uint dest, uint src)
{
    int dest_a = qAlpha(dest);
    int src_ia = 255 - qAlpha(src);

    return INTERPOLATE_PIXEL_255(src, dest_a, dest, src_ia);
}


static inline uint QT_FASTCALL comp_func_DestinationAtop(uint dest, uint src)
{
    return comp_func_SourceAtop(src, dest);
}

/*
  Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
  Da'  = Sa.(1 - Da) + Da.(1 - Sa)
       = Sa + Da - 2.Sa.Da
*/
static inline uint QT_FASTCALL comp_func_XOR(uint dest, uint src)
{
    // X = 0; Y = 1; Z = 1;
    int src_ia = (255 - qAlpha(src));
    int dest_ia = (255 - qAlpha(dest));

    return INTERPOLATE_PIXEL_255(src, dest_ia, dest, src_ia);
}


static CompositionFunction functionForMode(QPainter::CompositionMode mode)
{
    switch (mode) {
    case QPainter::CompositionMode_SourceOver:
        return comp_func_SourceOver;
    case QPainter::CompositionMode_DestinationOver:
        return comp_func_DestinationOver;
    case QPainter::CompositionMode_Clear:
        return comp_func_Clear;
    case QPainter::CompositionMode_Source:
        return comp_func_Source;
    case QPainter::CompositionMode_Destination:
        return comp_func_Destination;
    case QPainter::CompositionMode_SourceIn:
        return comp_func_SourceIn;
    case QPainter::CompositionMode_DestinationIn:
        return comp_func_DestinationIn;
    case QPainter::CompositionMode_SourceOut:
        return comp_func_SourceOut;
    case QPainter::CompositionMode_DestinationOut:
        return comp_func_DestinationOut;
    case QPainter::CompositionMode_SourceAtop:
        return comp_func_SourceAtop;
    case QPainter::CompositionMode_DestinationAtop:
        return comp_func_DestinationAtop;
    case QPainter::CompositionMode_Xor:
        return comp_func_XOR;
    }
    return 0;
}

static void blend_color_argb(void *t, const QSpan *span, QPainter::CompositionMode mode, const BlendColorData *data)
{
    uint *target = ((uint *)t) + span->x;
    uint color = data->color;
    if (mode == QPainter::CompositionMode_SourceOver) {
        MASK(color, span->coverage);
        int alpha = qAlpha(color);
        if (!alpha)
            return;
        if (alpha != 0xff) {
            const uint *end = target + span->len;
            while (target < end) {
                *target = comp_func_SourceOver(*target, color);
                ++target;
            }
        } else {
#if 1
            const uint *end = target + span->len;
            while (target < end) {
                *target = color;
                ++target;
            }
#else
            sse_memfill(target, color, span->len);
#endif
        }
    } else {
        CompositionFunction func = functionForMode(mode);
        const uint *end = target + span->len;

        if (span->coverage == 255) {
            while (target < end) {
                *target = func(*target, color);
                ++target;
            }
        } else {
            int icov = 255 - span->coverage;
            while (target < end) {
                uint tmp = func(*target, color);
                *target = INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov);
                ++target;
            }
        }
    }
}

static void blend_argb(void *t, const QSpan *span, const qreal dx, const qreal dy,
                       const void *ibits, const int image_width, const int image_height,
                       QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    if (y < 0 || y >= image_height)
        return;

    const uint *src = image_bits + y*image_width + x;
    const uint *end = target + span->len;
    if (x < 0) {
        src -= x;
        target -= x;
        x = 0;
    }
    if (end - target > image_width)
        end = target + image_width;

    if (mode == QPainter::CompositionMode_Source && span->coverage == 255) {
        while (target < end) {
            *target++ = *src++;
        }
        return;
    }


    CompositionFunction func = functionForMode(mode);
    if (span->coverage == 255) {
        while (target < end) {
            *target = func(*target, *src);
            ++target;
            ++src;
        }
    } else {
        int icov = 255 - span->coverage;
        while (target < end) {
            uint tmp = func(*target, *src);
            *target = INTERPOLATE_PIXEL_255(tmp, span->coverage, tmp, icov);
            ++target;
            ++src;
        }
    }
}

static void blend_tiled_argb(void *t, const QSpan *span,
                             const qreal dx, const qreal dy,
                             const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    x %= image_width;
    y %= image_height;

    if (x < 0)
        x += image_width;
    if (y < 0)
        y += image_height;

    CompositionFunction func = functionForMode(mode);
    const uint *src = image_bits + y*image_width;
    if (span->coverage == 255) {
        for (int i = x; i < x + span->len; ++i) {
            *target = func(*target, src[i%image_width]);
            ++target;
        }
    } else {
        int icov = 255 - span->coverage;
        for (int i = x; i < x + span->len; ++i) {
            uint tmp = func(*target, src[i%image_width]);
            *target = INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov);
            ++target;
        }
    }
}

static void blend_transformed_bilinear_argb(void *t, const QSpan *span,
                                            const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                            const void *ibits, const int image_width, const int image_height,
                                            QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    int x = int((ix + dx * span->x) * fixed_scale) - half_point;
    int y = int((iy + dy * span->x) * fixed_scale) - half_point;

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    CompositionFunction func = functionForMode(mode);
    int icov = 255 - span->coverage;
    const uint *end = target + span->len;
    while (target < end) {
        int x1 = (x >> 16);
        int x2 = x1 + 1;
        int y1 = (y >> 16);
        int y2 = y1 + 1;

        int distx = ((x - (x1 << 16)) >> 8);
        int disty = ((y - (y1 << 16)) >> 8);
        int idistx = 256 - distx;
        int idisty = 256 - disty;

        bool x1_out = ((x1 < 0) | (x1 >= image_width));
        bool x2_out = ((x2 < 0) | (x2 >= image_width));
        bool y1_out = ((y1 < 0) | (y1 >= image_height));
        bool y2_out = ((y2 < 0) | (y2 >= image_height));

        int y1_offset = y1 * image_width;
        int y2_offset = y1_offset + image_width;

        uint tl = (x1_out | y1_out) ? uint(0) : image_bits[y1_offset + x1];
        uint tr = (x2_out | y1_out) ? uint(0) : image_bits[y1_offset + x2];
        uint bl = (x1_out | y2_out) ? uint(0) : image_bits[y2_offset + x1];
        uint br = (x2_out | y2_out) ? uint(0) : image_bits[y2_offset + x2];

        uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
        uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
        uint res = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);

        uint tmp = func(*target, res);
        *target = icov ? INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov) : tmp;
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_bilinear_tiled_argb(void *t, const QSpan *span,
                                                  const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                                  const void *ibits, const int image_width, const int image_height,
                                                  QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    int x = int((ix + dx * span->x) * fixed_scale) - half_point;
    int y = int((iy + dy * span->x) * fixed_scale) - half_point;

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    CompositionFunction func = functionForMode(mode);
    int icov = 255 - span->coverage;
    const uint *end = target + span->len;
    while (target < end) {
        int x1 = (x >> 16);
        int x2 = (x1 + 1);
        int y1 = (y >> 16);
        int y2 = (y1 + 1);

        int distx = ((x - (x1 << 16)) >> 8);
        int disty = ((y - (y1 << 16)) >> 8);
        int idistx = 256 - distx;
        int idisty = 256 - disty;

        x1 %= image_width;
        x2 %= image_width;
        y1 %= image_height;
        y2 %= image_height;

        if (x1 < 0) x1 += image_width;
        if (x2 < 0) x2 += image_width;
        if (y1 < 0) y1 += image_height;
        if (y2 < 0) y2 += image_height;

        Q_ASSERT(x1 >= 0 && x1 < image_width);
        Q_ASSERT(x2 >= 0 && x2 < image_width);
        Q_ASSERT(y1 >= 0 && y1 < image_height);
        Q_ASSERT(y2 >= 0 && y2 < image_height);

        int y1_offset = y1 * image_width;
        int y2_offset = y2 * image_width;

        uint tl = image_bits[y1_offset + x1];
        uint tr = image_bits[y1_offset + x2];
        uint bl = image_bits[y2_offset + x1];
        uint br = image_bits[y2_offset + x2];

        uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
        uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
        uint res = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);

        uint tmp = func(*target, res);
        *target = icov ? INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov) : tmp;
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_argb(void *t, const QSpan *span,
                                   const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                   const void *ibits, const int image_width, const int image_height,
                                   QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    CompositionFunction func = functionForMode(mode);
    int icov = 255 - span->coverage;
    const uint *end = target + span->len;
    while (target < end) {
        int px = x >> 16;
        int py = y >> 16;

        bool out = (px < 0) | (px >= image_width)
                   | (py < 0) | (py >= image_height);

        int y_offset = py * image_width;
        uint pixel = out ? uint(0) : image_bits[y_offset + px];
        uint tmp = func(*target, pixel);
        *target = icov ? INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov) : tmp;
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_tiled_argb(void *t, const QSpan *span,
                                         const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                         const void *ibits, const int image_width, const int image_height,
                                         QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    CompositionFunction func = functionForMode(mode);
    int icov = 255 - span->coverage;
    const uint *end = target + span->len;
    while (target < end) {
        int px = x >> 16;
        int py = y >> 16;
        px %= image_width;
        py %= image_height;
        if (px < 0) px += image_width;
        if (py < 0) py += image_height;
        int y_offset = py * image_width;

        Q_ASSERT(px >= 0 && px < image_width);
        Q_ASSERT(py >= 0 && py < image_height);

        uint tmp = func(*target, image_bits[y_offset + px]);
        *target = icov ? INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov) : tmp;
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_linear_gradient_argb(void *t, const QSpan *span, LinearGradientData *data, qreal ybase, int,
                                       QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    qreal x1 = data->origin.x();
    qreal tt = ybase + data->xincr * (span->x - x1);

    if (mode == QPainter::CompositionMode_SourceOver && !data->alphaColor && span->coverage == 255) {
        for (int x = span->x; x<span->x + span->len; x++) {
            *target = qt_gradient_pixel(data, tt);
            ++target;
            tt += data->xincr;
        }
    } else {
        CompositionFunction func = functionForMode(mode);

        if (span->coverage == 255) {
            for (int x = span->x; x<span->x + span->len; x++) {
                *target = func(*target, qt_gradient_pixel(data, tt));
                ++target;
                tt += data->xincr;
            }
        } else {
            int icov = 255 - span->coverage;
            for (int x = span->x; x<span->x + span->len; x++) {
                uint tmp = func(*target, qt_gradient_pixel(data, tt));
                *target = INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov);
                ++target;
                tt += data->xincr;
            }
        }
    }
}

inline double determinant(double a, double b, double c)
{
    return (b * b) - (4 * a * c);
}

// function to evaluate real roots
inline double realRoots(double a, double b, double detSqrt)
{
    return (-b + detSqrt)/(2 * a);
}

static void blend_radial_gradient_argb(void *t, const QSpan *span, RadialGradientData *data,
                                       int y, QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *end = target + span->len;
    double dx = data->center.x() - data->focal.x();
    double dy = data->center.y() - data->focal.y();
    double r  = data->radius;
    double a = r*r - dx*dx - dy*dy;

    QMatrix m = data->imatrix;
    qreal ix = m.m21() * y + m.dx();
    qreal iy = m.m22() * y + m.dy();
    qreal cx = m.m11();
    qreal cy = m.m12();
    qreal rx = ix + cx * span->x - data->focal.x();
    qreal ry = iy + cy * span->x - data->focal.y();

    if (mode == QPainter::CompositionMode_SourceOver && !data->alphaColor && span->coverage == 255) {
        while (target < end) {
            double b  = 2*(rx*dx + ry*dy);
            double det = determinant(a, b , -(rx*rx + ry*ry));
            double s = realRoots(a, b, sqrt(det));

            *target = qt_gradient_pixel(data,  s);
            ++target;
            rx += cx;
            ry += cy;
        }
    } else {
        CompositionFunction func = functionForMode(mode);
        int icov = 255 - span->coverage;
        while (target < end) {
            double b  = 2*(rx*dx + ry*dy);
            double det = determinant(a, b , -(rx*rx + ry*ry));
            double s = realRoots(a, b, sqrt(det));

            uint tmp = func(*target, qt_gradient_pixel(data, s));
            *target = INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov);
            ++target;
            rx += cx;
            ry += cy;
        }
    }
}

static void blend_conical_gradient_argb(void *t, const QSpan *span, ConicalGradientData *data,
                                        int y, QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *end = target + span->len;

    QMatrix m = data->imatrix;
    qreal ix = m.m21() * y + m.dx();
    qreal iy = m.m22() * y + m.dy();
    qreal cx = m.m11();
    qreal cy = m.m12();
    qreal rx = ix + cx * span->x - data->center.x();
    qreal ry = iy + cy * span->x - data->center.y();

    int x = span->x;
    if (mode == QPainter::CompositionMode_SourceOver
        && !data->alphaColor
        && span->coverage == 255) {
        while (target < end) {
            double angle = atan2(ry, rx);
            angle += data->angle;
            *target = qt_gradient_pixel(data, angle / (2*Q_PI));
            ++target;
            ++x;
            rx += cx;
            ry += cy;
        }
    } else {
        CompositionFunction func = functionForMode(mode);
        int icov = 255 - span->coverage;
        while (target < end) {
            double angle = atan2(ry, rx);
            angle += data->angle;
            uint tmp = func(*target, qt_gradient_pixel(data, angle / (2*Q_PI)));
            *target = INTERPOLATE_PIXEL_255(tmp, span->coverage, *target, icov);
            ++target;
            ++x;
            rx += cx;
            ry += cy;
        }
    }
}

// ************************** RGB32 handling ******************************

static inline uint qt_blend_pixel_rgb32(uint dest, uint src, uint coverage)
{
    MASK(src, coverage);
    int rev_alpha = 255 - qAlpha(src);
    if (!rev_alpha)
        return src;

    return (src + BYTE_MUL(dest, rev_alpha)) | 0xff000000;
}

static void blend_color_rgb32(void *t, const QSpan *span, QPainter::CompositionMode, const BlendColorData *data)
{
    uint *target = ((uint *)t) + span->x;
    uint color = data->color;
    MASK(color, span->coverage);

    int alpha = qAlpha(color);
    if (!alpha)
        return;
    if (alpha != 0xff) {
        int rev_alpha = 255 - alpha;
        const uint *end = target + span->len;
        while (target < end) {
            uint dest = *target;
            *target = (color + BYTE_MUL(dest, rev_alpha)) | 0xff000000;
            ++target;
        }
    } else {
#if 1
        const uint *end = target + span->len;
        while (target < end) {
            *target = color;
            ++target;
        }
#else
        sse_memfill(target, color, span->len);
#endif
    }
}

static void blend_rgb32(void *t, const QSpan *span,
                        const qreal dx, const qreal dy,
                        const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode mode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    //     qDebug("x=%f,y=%f %d/%d image_height=%d", dx, dy, x, y, image_height);
    if (y < 0 || y >= image_height)
        return;

#if 0
    if (mode == QPainter::CompositionMode_Source && span->coverage == 255) {
        int span_x = span->x;
        int span_len = span->len;
        while (span_len > 0) {
            int image_x = (span_x + xoff) % image_width;
            int len = qMin(image_width - image_x, span_len);
            Q_ASSERT(image_x >= 0);
            Q_ASSERT(image_x + len <= image_width); // inclusive since it is used as upper bound.
            Q_ASSERT(span_x + len <= rb->width());
            memcpy(target, scanline + image_x, len * sizeof(uint));
            span_x += len;
            span_len -= len;
            target += len;
        }
        return;
    }
#endif

    const uint *src = image_bits + y*image_width + x;
    const uint *end = target + span->len;
    if (x < 0) {
        src -= x;
        target -= x;
        x = 0;
    }
    if (end - target > image_width)
        end = target + image_width;

    if (mode == QPainter::CompositionMode_Source && span->coverage == 255) {
        while (target < end) {
            *target++ = *src++;
        }
    } else {
        while (target < end) {
            *target = qt_blend_pixel_rgb32(*target, *src, span->coverage);
            ++target;
            ++src;
        }
    }
}

static void blend_tiled_rgb32(void *t, const QSpan *span,
                              const qreal dx, const qreal dy,
                              const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    x %= image_width;
    y %= image_height;

    if (x < 0)
        x += image_width;
    if (y < 0)
        y += image_height;

    const uint *src = image_bits + y*image_width;
    for (int i = x; i < x + span->len; ++i) {
        *target = qt_blend_pixel_rgb32(*target, src[i%image_width], span->coverage);
        ++target;
    }
}

static void blend_transformed_bilinear_rgb32(void *t, const QSpan *span,
                                             const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                             const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    int x = int((ix + dx * span->x) * fixed_scale) - half_point;
    int y = int((iy + dy * span->x) * fixed_scale) - half_point;

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const uint *end = target + span->len;
    while (target < end) {
        int x1 = (x >> 16);
        int x2 = x1 + 1;
        int y1 = (y >> 16);
        int y2 = y1 + 1;

        int distx = ((x - (x1 << 16)) >> 8);
        int disty = ((y - (y1 << 16)) >> 8);
        int idistx = 256 - distx;
        int idisty = 256 - disty;

        bool x1_out = ((x1 < 0) | (x1 >= image_width));
        bool x2_out = ((x2 < 0) | (x2 >= image_width));
        bool y1_out = ((y1 < 0) | (y1 >= image_height));
        bool y2_out = ((y2 < 0) | (y2 >= image_height));

        int y1_offset = y1 * image_width;
        int y2_offset = y1_offset + image_width;

        uint tl = (x1_out | y1_out) ? uint(0) : image_bits[y1_offset + x1];
        uint tr = (x2_out | y1_out) ? uint(0) : image_bits[y1_offset + x2];
        uint bl = (x1_out | y2_out) ? uint(0) : image_bits[y2_offset + x1];
        uint br = (x2_out | y2_out) ? uint(0) : image_bits[y2_offset + x2];

        uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
        uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
        uint res = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);

        *target = qt_blend_pixel_rgb32(*target, res, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_bilinear_tiled_rgb32(void *t, const QSpan *span,
                                                   const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                                   const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    int x = int((ix + dx * span->x) * fixed_scale) - half_point;
    int y = int((iy + dy * span->x) * fixed_scale) - half_point;

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const uint *end = target + span->len;
    while (target < end) {
        int x1 = (x >> 16);
        int x2 = (x1 + 1);
        int y1 = (y >> 16);
        int y2 = (y1 + 1);

        int distx = ((x - (x1 << 16)) >> 8);
        int disty = ((y - (y1 << 16)) >> 8);
        int idistx = 256 - distx;
        int idisty = 256 - disty;

        x1 %= image_width;
        x2 %= image_width;
        y1 %= image_height;
        y2 %= image_height;

        if (x1 < 0) x1 += image_width;
        if (x2 < 0) x2 += image_width;
        if (y1 < 0) y1 += image_height;
        if (y2 < 0) y2 += image_height;

        Q_ASSERT(x1 >= 0 && x1 < image_width);
        Q_ASSERT(x2 >= 0 && x2 < image_width);
        Q_ASSERT(y1 >= 0 && y1 < image_height);
        Q_ASSERT(y2 >= 0 && y2 < image_height);

        int y1_offset = y1 * image_width;
        int y2_offset = y2 * image_width;

        uint tl = image_bits[y1_offset + x1];
        uint tr = image_bits[y1_offset + x2];
        uint bl = image_bits[y2_offset + x1];
        uint br = image_bits[y2_offset + x2];

        uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
        uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
        uint res = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);

        *target = qt_blend_pixel_rgb32(*target, res, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_rgb32(void *t, const QSpan *span,
                                    const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                    const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const uint *end = target + span->len;
        while (target < end) {
        int px = x >> 16;
        int py = y >> 16;

        bool out = (px < 0) | (px >= image_width)
                   | (py < 0) | (py >= image_height);

        int y_offset = py * image_width;

        uint pixel = out ? uint(0) : image_bits[y_offset + px];

        *target = qt_blend_pixel_rgb32(*target, pixel, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_tiled_rgb32(void *t, const QSpan *span,
                                          const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                          const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const uint *end = target + span->len;
    while (target < end) {
        int px = x >> 16;
        int py = y >> 16;
        px %= image_width;
        py %= image_height;
        if (px < 0) px += image_width;
        if (py < 0) py += image_height;
        int y_offset = py * image_width;

        Q_ASSERT(px >= 0 && px < image_width);
        Q_ASSERT(py >= 0 && py < image_height);

        *target = qt_blend_pixel_rgb32(*target, image_bits[y_offset + px], span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_linear_gradient_rgb32(void *t, const QSpan *span, LinearGradientData *data, qreal ybase, int, QPainter::CompositionMode)
{
    uint *target = ((uint *)t) + span->x;
    qreal x1 = data->origin.x();
    qreal tt = ybase + data->xincr * (span->x - x1);

    if (!data->alphaColor && span->coverage == 255) {
        for (int x = span->x; x<span->x + span->len; x++) {
            *target = qt_gradient_pixel(data, tt);
            ++target;
            tt += data->xincr;
        }
    } else {
        for (int x = span->x; x<span->x + span->len; x++) {
            uint src = qt_gradient_pixel(data, tt);
            *target = qt_blend_pixel_rgb32(*target, src, span->coverage);
            ++target;
            tt += data->xincr;
        }
    }
}


/************************************* Mono ************************************/

static void blend_color_mono(void *t, const QSpan *span, QPainter::CompositionMode, const BlendColorData *data)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint color = data->color;

    if (color == 0xff000000) {
        for (int i = span->x; i < span->x + span->len; ++i) {
            target[i>>3] |= 0x80 >> (i & 7);
        }
    } else if (color == 0xffffffff) {
        for (int i = span->x; i < span->x + span->len; ++i) {
            target[i>>3] &= ~(0x80 >> (i & 7));
        }
    } else {
        uint g = qGray(color);
        for (int i = span->x; i < span->x + span->len; ++i) {
            if (g < qt_bayer_matrix[data->y & 15][i & 15])
                target[i >> 3] |= 0x80 >> (i & 7);
            else
                target[i >> 3] &= ~(0x80 >> (i & 7));
        }
    }
}

static void blend_mono(void *t, const QSpan *span,
                        const qreal dx, const qreal dy,
                        const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    if (y < 0 || y >= image_height)
        return;

    const uint *src = image_bits + y*image_width + x;
    for (int i = 0; i < span->len; ++i) {
        int sx = x + i;
        int dx = span->x + i;
        uint p = src[sx];
        if (qGray(p) < int(qt_bayer_matrix[y & 15][(sx) & 15]))
            target[dx >> 3] |= 0x80 >> (dx & 7);
        else
            target[dx >> 3] &= ~(0x80 >> (dx & 7));
    }
}

static void blend_tiled_mono(void *t, const QSpan *span,
                              const qreal dx, const qreal dy,
                              const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    x %= image_width;
    y %= image_height;

    if (x < 0)
        x += image_width;
    if (y < 0)
        y += image_height;

    const uint *src = image_bits + y*image_width;
    for (int i = 0; i < span->len; ++i) {
        int sx = x + i;
        int dx = span->x + i;
        uint p = src[sx%image_width];
        if (qGray(p) < int(qt_bayer_matrix[y & 15][sx & 15]))
            target[dx >> 3] |= 0x80 >> (dx & 7);
        else
            target[dx >> 3] &= ~(0x80 >> (dx & 7));
    }
}


static void blend_transformed_mono(void *t, const QSpan *span,
                                    const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                    const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
        int px = x >> 16;
        int py = y >> 16;

        bool out = (px < 0) | (px >= image_width)
                   | (py < 0) | (py >= image_height);
        if (out)
            continue;

        int y_offset = py * image_width;

        int dx = span->x + i;
        uint p = image_bits[y_offset + px];
        if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
            target[dx >> 3] |= 0x80 >> (dx & 7);
        else
            target[dx >> 3] &= ~(0x80 >> (dx & 7));

        x += fdx;
        y += fdy;
    }
}

static void blend_transformed_tiled_mono(void *t, const QSpan *span,
                                          const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                          const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
        int px = x >> 16;
        int py = y >> 16;
        px %= image_width;
        py %= image_height;
        if (px < 0) px += image_width;
        if (py < 0) py += image_height;
        int y_offset = py * image_width;

        Q_ASSERT(px >= 0 && px < image_width);
        Q_ASSERT(py >= 0 && py < image_height);

        int dx = span->x + i;
        uint p = image_bits[y_offset + px];
        if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
            target[dx >> 3] |= 0x80 >> (dx & 7);
        else
            target[dx >> 3] &= ~(0x80 >> (dx & 7));

        x += fdx;
        y += fdy;
    }
}

static void blend_linear_gradient_mono(void *t, const QSpan *span, LinearGradientData *data, qreal ybase,
                                       int y, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;

    qreal x1 = data->origin.x();
    qreal tt = ybase + data->xincr * (span->x - x1);

    for (int x = span->x; x<span->x + span->len; x++) {
        uint p = qt_gradient_pixel(data, tt);
        if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
            target[x >> 3] |= 0x80 >> (x & 7);
        else
            target[x >> 3] &= ~(0x80 >> (x & 7));
        tt += data->xincr;
    }
}

static void blend_radial_gradient_mono(void *t, const QSpan *span, RadialGradientData *data,
                                       int y, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;

    double dx = data->center.x() - data->focal.x();
    double dy = data->center.y() - data->focal.y();
    double r  = data->radius;
    double a = r*r - dx*dx - dy*dy;

    QMatrix m = data->imatrix;
    qreal ix = m.m21() * y + m.dx();
    qreal iy = m.m22() * y + m.dy();
    qreal cx = m.m11();
    qreal cy = m.m12();
    qreal rx = ix + cx * span->x - data->focal.x();
    qreal ry = iy + cy * span->x - data->focal.y();

    for (int x = span->x; x<span->x + span->len; x++) {
        double b  = 2*(rx*dx + ry*dy);
        double det = determinant(a, b , -(rx*rx + ry*ry));
        double s = realRoots(a, b, sqrt(det));

        uint p = qt_gradient_pixel(data, s);
        if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
            target[x >> 3] |= 0x80 >> (x & 7);
        else
            target[x >> 3] &= ~(0x80 >> (x & 7));
        ++x;
        rx += cx;
        ry += cy;
    }
}

static void blend_conical_gradient_mono(void *t, const QSpan *span, ConicalGradientData *data,
                                        int y, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;

    QMatrix m = data->imatrix;
    qreal ix = m.m21() * y + m.dx();
    qreal iy = m.m22() * y + m.dy();
    qreal cx = m.m11();
    qreal cy = m.m12();
    qreal rx = ix + cx * span->x - data->center.x();
    qreal ry = iy + cy * span->x - data->center.y();

    for (int x = span->x; x<span->x + span->len; x++) {
        double angle = atan2(ry, rx);
        angle += data->angle;
        uint p = qt_gradient_pixel(data, angle / 360.0);
        if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
            target[x >> 3] |= 0x80 >> (x & 7);
        else
            target[x >> 3] &= ~(0x80 >> (x & 7));
        ++x;
        rx += cx;
        ry += cy;
    }
}

// ************************** Mono LSB ********************************


static void blend_color_mono_lsb(void *t, const QSpan *span, QPainter::CompositionMode, const BlendColorData *data)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint color = data->color;

    if (color == 0xffffffff) {
        for (int i = span->x; i < span->x + span->len; ++i) {
            target[i>>3] &= ~(1 << (i & 7));
        }
    } else if (color == 0xff000000) {
        for (int i = span->x; i < span->x + span->len; ++i) {
            target[i>>3] |= 1 << (i & 7);
        }
    } else {
        uint g = qGray(color);
        for (int i = span->x; i < span->x + span->len; ++i) {
            if (g < qt_bayer_matrix[data->y & 15][i & 15])
                target[i >> 3] |= 1 << (i & 7);
            else
                target[i >> 3] &= ~(1 << (i & 7));
        }
    }
}

static void blend_mono_lsb(void *t, const QSpan *span,
                        const qreal dx, const qreal dy,
                        const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    if (y < 0 || y >= image_height)
        return;

    const uint *src = image_bits + y*image_width + x;
    for (int i = 0; i < span->len; ++i) {
        int sx = x + i;
        int dx = span->x + i;
        uint p = src[sx];
        if (qGray(p) < int(qt_bayer_matrix[y & 15][(sx) & 15]))
            target[dx >> 3] |= 1 << (dx & 7);
        else
            target[dx >> 3] &= ~(1 << (dx & 7));
    }
}

static void blend_tiled_mono_lsb(void *t, const QSpan *span,
                              const qreal dx, const qreal dy,
                              const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    x %= image_width;
    y %= image_height;

    if (x < 0)
        x += image_width;
    if (y < 0)
        y += image_height;

    const uint *src = image_bits + y*image_width;
    for (int i = 0; i < span->len; ++i) {
        int sx = x + i;
        int dx = span->x + i;
        uint p = src[sx%image_width];
        if (qGray(p) < int(qt_bayer_matrix[y & 15][sx & 15]))
            target[dx >> 3] |= 1 << (dx & 7);
        else
            target[dx >> 3] &= ~(1 << (dx & 7));
    }
}


static void blend_transformed_mono_lsb(void *t, const QSpan *span,
                                    const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                    const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
        int px = x >> 16;
        int py = y >> 16;

        bool out = (px < 0) | (px >= image_width)
                   | (py < 0) | (py >= image_height);
        if (out)
            continue;

        int y_offset = py * image_width;

        int dx = span->x + i;
        uint p = image_bits[y_offset + px];
        if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
            target[dx >> 3] |= 1 << (dx & 7);
        else
            target[dx >> 3] &= ~(1 << (dx & 7));

        x += fdx;
        y += fdy;
    }
}

static void blend_transformed_tiled_mono_lsb(void *t, const QSpan *span,
                                          const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                          const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
        int px = x >> 16;
        int py = y >> 16;
        px %= image_width;
        py %= image_height;
        if (px < 0) px += image_width;
        if (py < 0) py += image_height;
        int y_offset = py * image_width;

        Q_ASSERT(px >= 0 && px < image_width);
        Q_ASSERT(py >= 0 && py < image_height);

        int dx = span->x + i;
        uint p = image_bits[y_offset + px];
        if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
            target[dx >> 3] |= 1 << (dx & 7);
        else
            target[dx >> 3] &= ~(1 << (dx & 7));

        x += fdx;
        y += fdy;
    }
}

static void blend_linear_gradient_mono_lsb(void *t, const QSpan *span, LinearGradientData *data, qreal ybase,
                                           int y, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;

    qreal x1 = data->origin.x();
    qreal tt = ybase + data->xincr * (span->x - x1);

    for (int x = span->x; x<span->x + span->len; x++) {
        uint p = qt_gradient_pixel(data, tt);
        if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
            target[x >> 3] |= 1 << (x & 7);
        else
            target[x >> 3] &= ~(1 << (x & 7));
        tt += data->xincr;
    }
}


static void blend_radial_gradient_mono_lsb(void *t, const QSpan *span, RadialGradientData *data,
                                           int y, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;

    double dx = data->center.x() - data->focal.x();
    double dy = data->center.y() - data->focal.y();
    double r  = data->radius;
    double a = r*r - dx*dx - dy*dy;

    QMatrix m = data->imatrix;
    qreal ix = m.m21() * y + m.dx();
    qreal iy = m.m22() * y + m.dy();
    qreal cx = m.m11();
    qreal cy = m.m12();
    qreal rx = ix + cx * span->x - data->focal.x();
    qreal ry = iy + cy * span->x - data->focal.y();

    for (int x = span->x; x<span->x + span->len; x++) {
        double b  = 2*(rx*dx + ry*dy);
        double det = determinant(a, b , -(rx*rx + ry*ry));
        double s = realRoots(a, b, sqrt(det));

        uint p = qt_gradient_pixel(data, s);
        if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
            target[x >> 3] |= 1 >> (x & 7);
        else
            target[x >> 3] &= ~(1 >> (x & 7));
        ++x;
        rx += cx;
        ry += cy;
    }
}

static void blend_conical_gradient_mono_lsb(void *t, const QSpan *span, ConicalGradientData *data,
                                        int y, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uchar *target = (uchar *)t;

    QMatrix m = data->imatrix;
    qreal ix = m.m21() * y + m.dx();
    qreal iy = m.m22() * y + m.dy();
    qreal cx = m.m11();
    qreal cy = m.m12();
    qreal rx = ix + cx * span->x - data->center.x();
    qreal ry = iy + cy * span->x - data->center.y();

    for (int x = span->x; x<span->x + span->len; x++) {
        double angle = atan2(ry, rx);
        angle += data->angle;
        uint p = qt_gradient_pixel(data, angle / 360.0);
        if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
            target[x >> 3] |= 1 << (x & 7);
        else
            target[x >> 3] &= ~(1 << (x & 7));
        ++x;
        rx += cx;
        ry += cy;
    }
}

DrawHelper qDrawHelper[4] =
{
    { // Layout_ARGB
        blend_color_argb,
        blend_argb,
        blend_tiled_argb,
        blend_transformed_argb,
        blend_transformed_tiled_argb,
        blend_transformed_bilinear_argb,
        blend_transformed_bilinear_tiled_argb,
        blend_linear_gradient_argb,
        blend_radial_gradient_argb,
        blend_conical_gradient_argb
    },
    { // Layout_RGB32
        blend_color_rgb32,
        blend_rgb32,
        blend_tiled_rgb32,
        blend_transformed_rgb32,
        blend_transformed_tiled_rgb32,
        blend_transformed_bilinear_rgb32,
        blend_transformed_bilinear_tiled_rgb32,
        blend_linear_gradient_rgb32,
        blend_radial_gradient_argb,
        blend_conical_gradient_argb
    },
    { // Layout_Mono
        blend_color_mono,
        blend_mono,
        blend_tiled_mono,
        blend_transformed_mono,
        blend_transformed_tiled_mono,
        blend_transformed_mono,
        blend_transformed_tiled_mono,
        blend_linear_gradient_mono,
        blend_radial_gradient_mono,
        blend_conical_gradient_mono
    },
    { // Layout_MonoLSB
        blend_color_mono_lsb,
        blend_mono_lsb,
        blend_tiled_mono_lsb,
        blend_transformed_mono_lsb,
        blend_transformed_tiled_mono_lsb,
        blend_transformed_mono_lsb,
        blend_transformed_tiled_mono_lsb,
        blend_linear_gradient_mono_lsb,
        blend_radial_gradient_mono_lsb,
        blend_conical_gradient_mono_lsb
    }
};

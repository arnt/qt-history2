#include <private/qdrawhelper_p.h>

#if 0//def __x86__
#include "qdrawhelper_x86.cpp"
#else
void qInitDrawhelperAsm() {}
#endif

#define MASK(src, a)                            \
    src = (qt_div_255(qAlpha(src) * a) << 24)   \
        | (qt_div_255(qRed(src) * a) << 16)     \
        | (qt_div_255(qGreen(src) * a) << 8)    \
        | (qt_div_255(qBlue(src) * a))

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
    int src_ia = 255 - qAlpha(src);

    return src + ((qt_div_255(qRed(dest) * src_ia) << 16)
                  | (qt_div_255(qGreen(dest) * src_ia) << 8)
                  | (qt_div_255(qBlue(dest) * src_ia))
                  | (qt_div_255(qAlpha(dest) * src_ia)) << 24);
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

    return (qt_div_255(qAlpha(src) * dest_a) << 24)
        | (qt_div_255(qRed(src) * dest_a) << 16)
        | (qt_div_255(qGreen(src) * dest_a) << 8)
        | (qt_div_255(qBlue(src) * dest_a));
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

    return (qt_div_255(qAlpha(src) * dest_ia) << 24)
        | (qt_div_255(qRed(src) * dest_ia) << 16)
        | (qt_div_255(qGreen(src) * dest_ia) << 8)
        | qt_div_255(qBlue(src) * dest_ia);
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

    return (qt_div_255(qRed(src) * dest_a + qRed(dest) * src_ia) << 16)
        | (qt_div_255(qGreen(src) * dest_a + qGreen(dest) * src_ia) << 8)
        | qt_div_255(qBlue(src) * dest_a + qBlue(dest) * src_ia)
        | (dest_a << 24);
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
    int src_a = qAlpha(src);
    int dest_a = qAlpha(dest);
    int src_ia = (255 - src_a);
    int dest_ia = (255 - dest_a);

    return (qt_div_255(dest_ia * qRed(src) + src_ia * qRed(dest)) << 16)
        | (qt_div_255(dest_ia * qGreen(src) + src_ia * qGreen(dest)) << 8)
        | qt_div_255(dest_ia * qBlue(src) + src_ia * qBlue(dest))
        | (qt_div_255(src_a*dest_ia + dest_a*src_ia) << 24);
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

static void blend_color_argb(void *t, const QSpan *span, uint color, QPainter::CompositionMode mode)
{
    uint *target = (uint *)t;
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
    uint *target = (uint *)t;
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
    uint *target = (uint *)t;
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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    CompositionFunction func = functionForMode(mode);
    int icov = 255 - span->coverage;
    const uint *end = target + span->len;
    while (target < end) {
        int px = (x + half_point) >> 16;
        int py = (y + half_point) >> 16;

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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    CompositionFunction func = functionForMode(mode);
    int icov = 255 - span->coverage;
    const uint *end = target + span->len;
    while (target < end) {
        int px = (x + half_point) >> 16;
        int py = (y + half_point) >> 16;
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

static void blend_linear_gradient_argb(void *t, const QSpan *span, LinearGradientData *data, qreal ybase,
                                       QPainter::CompositionMode mode)
{
    uint *target = (uint *)t;
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

// ************************** RGB32 handling ******************************

static inline uint qt_blend_pixel_rgb32(uint dest, uint src, uint coverage)
{
    MASK(src, coverage);
    int rev_alpha = 255 - qAlpha(src);
    if (!rev_alpha)
        return src;

    return (src + ((qt_div_255(qRed(dest) * rev_alpha) << 16)
                   | (qt_div_255(qGreen(dest) * rev_alpha) << 8)
                   | (qt_div_255(qBlue(dest) * rev_alpha))))
        | 0xff000000;
}

static void blend_color_rgb32(void *t, const QSpan *span, uint color, QPainter::CompositionMode)
{
    uint *target = (uint *)t;
    MASK(color, span->coverage);

    int alpha = qAlpha(color);
    if (!alpha)
        return;
    if (alpha != 0xff) {
        int rev_alpha = 255 - alpha;
        const uint *end = target + span->len;
        while (target < end) {
            uint dest = *target;
            *target = (color + ((qt_div_255(qRed(dest) * rev_alpha) << 16)
                              | (qt_div_255(qGreen(dest) * rev_alpha) << 8)
                              | (qt_div_255(qBlue(dest) * rev_alpha))))
                      | 0xff000000;
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
                        const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
//     qDebug("x=%f,y=%f %d/%d image_height=%d", dx, dy, x, y, image_height);
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

    while (target < end) {
        *target = qt_blend_pixel_rgb32(*target, *src, span->coverage);
        ++target;
        ++src;
    }
}

static void blend_tiled_rgb32(void *t, const QSpan *span,
                              const qreal dx, const qreal dy,
                              const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    uint *target = (uint *)t;
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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const uint *end = target + span->len;
        while (target < end) {
        int px = (x + half_point) >> 16;
        int py = (y + half_point) >> 16;

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
    uint *target = (uint *)t;
    uint *image_bits = (uint *)ibits;
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const uint *end = target + span->len;
    while (target < end) {
        int px = (x + half_point) >> 16;
        int py = (y + half_point) >> 16;
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

static void blend_linear_gradient_rgb32(void *t, const QSpan *span, LinearGradientData *data, qreal ybase, QPainter::CompositionMode)
{
    uint *target = (uint *)t;
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

DrawHelper qDrawHelper[2] =
{
    { // Layout_ARGB
        blend_color_argb,
        blend_argb,
        blend_tiled_argb,
        blend_transformed_argb,
        blend_transformed_tiled_argb,
        blend_transformed_bilinear_argb,
        blend_transformed_bilinear_tiled_argb,
        blend_linear_gradient_argb
    },
    { // Layout_RGB32
        blend_color_rgb32,
        blend_rgb32,
        blend_tiled_rgb32,
        blend_transformed_rgb32,
        blend_transformed_tiled_rgb32,
        blend_transformed_bilinear_rgb32,
        blend_transformed_bilinear_tiled_rgb32,
        blend_linear_gradient_rgb32
    }
};

#include <private/qdrawhelper_p.h>

#if 0//def __x86__
#include "qdrawhelper_x86.cpp"
#else
void qInitDrawhelperAsm() {}
#endif


#define qt_blend_pixel_rgb32(src, target, coverage) {   \
        int alpha = qt_div_255(coverage * src.a);       \
        if (alpha == 0xff) {                            \
            *target = src;                              \
        } else {                                        \
            int rev_alpha = 255 - alpha;                \
            qt_alpha_pixel(src.r, target->r, alpha, rev_alpha); \
            qt_alpha_pixel(src.g, target->g, alpha, rev_alpha); \
            qt_alpha_pixel(src.b, target->b, alpha, rev_alpha); \
            target->a = 0xff;                                   \
        }                                                       \
    }

static void blend_color_argb(ARGB *target, const QSpan *span, ARGB color)
{
    if (!span->len)
        return;

    int alpha = qt_div_255(color.a * span->coverage);
    if (!alpha)
        return;
    if (alpha != 0xff) {
        int rev_alpha = 255 - alpha;
        int pr = alpha * color.r;
        int pg = alpha * color.g;
        int pb = alpha * color.b;

        const ARGB *end = target + span->len;
        while (target < end) {
            int res_alpha = alpha + qt_div_255(rev_alpha * target->a);
            target->a = res_alpha;
            if (res_alpha == 255) {
                qt_alpha_pixel_pm(pr, target->r, rev_alpha);
                qt_alpha_pixel_pm(pg, target->g, rev_alpha);
                qt_alpha_pixel_pm(pb, target->b, rev_alpha);
            } else if (res_alpha != 0) {
                int ra = rev_alpha * target->a;
                target->r = (pr + qt_div_255(ra * target->r)) / res_alpha;
                target->g = (pg + qt_div_255(ra * target->g)) / res_alpha;
                target->b = (pb + qt_div_255(ra * target->b)) / res_alpha;
            }
            ++target;
        }
    } else {
        const ARGB *end = target + span->len;
        while (target < end) {
            *target = color;
            ++target;
        }
    }
}

static void blend_color_rgb32(ARGB *target, const QSpan *span, ARGB color)
{
    if (!span->len)
        return;

    int alpha = qt_div_255(color.a * span->coverage);
    if (!alpha)
        return;
    if (alpha != 0xff) {
        int pr = alpha * color.r;
        int pg = alpha * color.g;
        int pb = alpha * color.b;
        int rev_alpha = 255 - alpha;

        const ARGB *end = target + span->len;
        while (target < end) {
            qt_alpha_pixel_pm(pr, target->r, rev_alpha);
            qt_alpha_pixel_pm(pg, target->g, rev_alpha);
            qt_alpha_pixel_pm(pb, target->b, rev_alpha);
            target->a = 255;
            ++target;
        }
    } else {
        const ARGB *end = target + span->len;
        while (target < end) {
            *target = color;
            ++target;
        }
    }
}

static void blend_argb(ARGB *target, const QSpan *span,
                       const qreal dx, const qreal dy,
                       const ARGB *image_bits, const int image_width, const int image_height)
{
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    if (y < 0 || y >= image_height)
        return;

    const ARGB *src = image_bits + y*image_width + x;
    const ARGB *end = target + span->len;
    if (x < 0) {
        src -= x;
        target -= x;
        x = 0;
    }
    if (end - target > image_width)
        end = target + image_width;

    while (target < end) {
        qt_blend_pixel((*src), target, span->coverage);
        ++target;
        ++src;
    }
}

static void blend_rgb32(ARGB *target, const QSpan *span,
                        const qreal dx, const qreal dy,
                        const ARGB *image_bits, const int image_width, const int image_height)
{
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
//     qDebug("x=%f,y=%f %d/%d image_height=%d", dx, dy, x, y, image_height);
    if (y < 0 || y >= image_height)
        return;

    const ARGB *src = image_bits + y*image_width + x;
    const ARGB *end = target + span->len;
    if (x < 0) {
        src -= x;
        target -= x;
        x = 0;
    }
    if (end - target > image_width)
        end = target + image_width;

    while (target < end) {
        qt_blend_pixel_rgb32((*src), target, span->coverage);
        ++target;
        ++src;
    }
}

static void blend_tiled_argb(ARGB *target, const QSpan *span,
                             const qreal dx, const qreal dy,
                             const ARGB *image_bits, const int image_width, const int image_height)
{
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    x %= image_width;
    y %= image_height;

    if (x < 0)
        x += image_width;
    if (y < 0)
        y += image_height;

    const ARGB *src = image_bits + y*image_width;
    for (int i = x; i < x + span->len; ++i) {
        qt_blend_pixel(src[i%image_width], target, span->coverage);
        ++target;
    }
}

static void blend_tiled_rgb32(ARGB *target, const QSpan *span,
                              const qreal dx, const qreal dy,
                              const ARGB *image_bits, const int image_width, const int image_height)
{
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    x %= image_width;
    y %= image_height;

    if (x < 0)
        x += image_width;
    if (y < 0)
        y += image_height;

    const ARGB *src = image_bits + y*image_width;
    for (int i = x; i < x + span->len; ++i) {
        qt_blend_pixel_rgb32(src[i%image_width], target, span->coverage);
        ++target;
    }
}

static void blend_transformed_bilinear_argb(ARGB *target, const QSpan *span,
                                            const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                            const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
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

        ARGB tl = (x1_out | y1_out) ? ARGB(0) : image_bits[y1_offset + x1];
        ARGB tr = (x2_out | y1_out) ? ARGB(0) : image_bits[y1_offset + x2];
        ARGB bl = (x1_out | y2_out) ? ARGB(0) : image_bits[y2_offset + x1];
        ARGB br = (x2_out | y2_out) ? ARGB(0) : image_bits[y2_offset + x2];

        ARGB xtop((tl.a * idistx + tr.a * distx) >> 8,
                  (tl.r * idistx + tr.r * distx) >> 8,
                  (tl.g * idistx + tr.g * distx) >> 8,
                  (tl.b * idistx + tr.b * distx) >> 8);


        ARGB xbot((bl.a * idistx + br.a * distx) >> 8,
                  (bl.r * idistx + br.r * distx) >> 8,
                  (bl.g * idistx + br.g * distx) >> 8,
                  (bl.b * idistx + br.b * distx) >> 8);


        ARGB res((xtop.a * idisty + xbot.a * disty) >> 8,
                 (xtop.r * idisty + xbot.r * disty) >> 8,
                 (xtop.g * idisty + xbot.g * disty) >> 8,
                 (xtop.b * idisty + xbot.b * disty) >> 8);

        qt_blend_pixel(res, target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_bilinear_rgb32(ARGB *target, const QSpan *span,
                                             const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                             const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
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

        ARGB tl = (x1_out | y1_out) ? ARGB(0) : image_bits[y1_offset + x1];
        ARGB tr = (x2_out | y1_out) ? ARGB(0) : image_bits[y1_offset + x2];
        ARGB bl = (x1_out | y2_out) ? ARGB(0) : image_bits[y2_offset + x1];
        ARGB br = (x2_out | y2_out) ? ARGB(0) : image_bits[y2_offset + x2];

        ARGB xtop((tl.a * idistx + tr.a * distx) >> 8,
                  (tl.r * idistx + tr.r * distx) >> 8,
                  (tl.g * idistx + tr.g * distx) >> 8,
                  (tl.b * idistx + tr.b * distx) >> 8);


        ARGB xbot((bl.a * idistx + br.a * distx) >> 8,
                  (bl.r * idistx + br.r * distx) >> 8,
                  (bl.g * idistx + br.g * distx) >> 8,
                  (bl.b * idistx + br.b * distx) >> 8);


        ARGB res((xtop.a * idisty + xbot.a * disty) >> 8,
                 (xtop.r * idisty + xbot.r * disty) >> 8,
                 (xtop.g * idisty + xbot.g * disty) >> 8,
                 (xtop.b * idisty + xbot.b * disty) >> 8);

        qt_blend_pixel_rgb32(res, target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_bilinear_tiled_argb(ARGB *target, const QSpan *span,
                                                  const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                                  const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
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

        ARGB tl = image_bits[y1_offset + x1];
        ARGB tr = image_bits[y1_offset + x2];
        ARGB bl = image_bits[y2_offset + x1];
        ARGB br = image_bits[y2_offset + x2];

        ARGB xtop((tl.a * idistx + tr.a * distx) >> 8,
                  (tl.r * idistx + tr.r * distx) >> 8,
                  (tl.g * idistx + tr.g * distx) >> 8,
                  (tl.b * idistx + tr.b * distx) >> 8);


        ARGB xbot((bl.a * idistx + br.a * distx) >> 8,
                  (bl.r * idistx + br.r * distx) >> 8,
                  (bl.g * idistx + br.g * distx) >> 8,
                  (bl.b * idistx + br.b * distx) >> 8);


        ARGB res((xtop.a * idisty + xbot.a * disty) >> 8,
                 (xtop.r * idisty + xbot.r * disty) >> 8,
                 (xtop.g * idisty + xbot.g * disty) >> 8,
                 (xtop.b * idisty + xbot.b * disty) >> 8);

        qt_blend_pixel(res, target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_bilinear_tiled_rgb32(ARGB *target, const QSpan *span,
                                                   const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                                   const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
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

        ARGB tl = image_bits[y1_offset + x1];
        ARGB tr = image_bits[y1_offset + x2];
        ARGB bl = image_bits[y2_offset + x1];
        ARGB br = image_bits[y2_offset + x2];

        ARGB xtop((tl.a * idistx + tr.a * distx) >> 8,
                  (tl.r * idistx + tr.r * distx) >> 8,
                  (tl.g * idistx + tr.g * distx) >> 8,
                  (tl.b * idistx + tr.b * distx) >> 8);


        ARGB xbot((bl.a * idistx + br.a * distx) >> 8,
                  (bl.r * idistx + br.r * distx) >> 8,
                  (bl.g * idistx + br.g * distx) >> 8,
                  (bl.b * idistx + br.b * distx) >> 8);


        ARGB res((xtop.a * idisty + xbot.a * disty) >> 8,
                 (xtop.r * idisty + xbot.r * disty) >> 8,
                 (xtop.g * idisty + xbot.g * disty) >> 8,
                 (xtop.b * idisty + xbot.b * disty) >> 8);

        qt_blend_pixel_rgb32(res, target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}


static void blend_transformed_argb(ARGB *target, const QSpan *span,
                                   const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                   const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
    while (target < end) {
        int px = (x + half_point) >> 16;
        int py = (y + half_point) >> 16;

        bool out = (px < 0) | (px >= image_width)
                   | (py < 0) | (py >= image_height);

        int y_offset = py * image_width;

        ARGB pixel = out ? ARGB(0) : image_bits[y_offset + px];

        qt_blend_pixel(pixel, target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_rgb32(ARGB *target, const QSpan *span,
                                    const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                    const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
        while (target < end) {
        int px = (x + half_point) >> 16;
        int py = (y + half_point) >> 16;

        bool out = (px < 0) | (px >= image_width)
                   | (py < 0) | (py >= image_height);

        int y_offset = py * image_width;

        ARGB pixel = out ? ARGB(0) : image_bits[y_offset + px];

        qt_blend_pixel_rgb32(pixel, target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_tiled_argb(ARGB *target, const QSpan *span,
                                         const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                         const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
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

        qt_blend_pixel(image_bits[y_offset + px], target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_tiled_rgb32(ARGB *target, const QSpan *span,
                                          const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                          const ARGB *image_bits, const int image_width, const int image_height)
{
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    const ARGB *end = target + span->len;
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

        qt_blend_pixel_rgb32(image_bits[y_offset + px], target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
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
        blend_transformed_bilinear_tiled_argb
    },
    { // Layout_RGB32
        blend_color_rgb32,
        blend_rgb32,
        blend_tiled_rgb32,
        blend_transformed_rgb32,
        blend_transformed_tiled_rgb32,
        blend_transformed_bilinear_rgb32,
        blend_transformed_bilinear_tiled_rgb32
    }
};

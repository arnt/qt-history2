#include <private/qdrawhelper_p.h>

#if 0//def __x86__
#include "qdrawhelper_x86.cpp"
#else
void qInitAsm(DrawHelper *) {}
#endif

static void blend_color(ARGB *target, const QSpan *span, ARGB color)
{
    if (!span->len)
        return;

    int alpha = qt_div_255(color.a * span->coverage);
    int pr = alpha * color.r;
    int pg = alpha * color.g;
    int pb = alpha * color.b;

    int rev_alpha = 255 - alpha;

    for (int i = span->len; i > 0 ; --i) {
        qt_blend_pixel_premul(pr, pg, pb, alpha, target, span->coverage);
        ++target;
    }
}

static void blend_transformed_bilinear(ARGB *target, const QSpan *span, qreal ix, qreal iy, qreal dx, qreal dy,
                                       ARGB *image_bits, int image_width, int image_height)
{
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
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

static void blend_transformed_bilinear_tiled(ARGB *target,
                                              const QSpan *span,
                                              qreal ix, qreal iy, qreal dx, qreal dy,
                                              ARGB *image_bits, int image_width, int image_height)
{
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
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

static void blend_transformed(ARGB *target, const QSpan *span,
                              qreal ix, qreal iy, qreal dx, qreal dy,
                              ARGB *image_bits, int image_width, int image_height)
{
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
        int px = (x + half_point) >> 16;
        int py = (y + half_point) >> 16;

        bool x_out = (px < 0) | (px >= image_width);
        bool y_out = (py < 0) | (py >= image_height);

        int y_offset = py * image_width;

        ARGB pixel = (x_out | y_out) ? ARGB(0) : image_bits[y_offset + px];

        qt_blend_pixel(pixel, target, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
}

static void blend_transformed_tiled(ARGB *target, const QSpan *span,
                                    qreal ix, qreal iy, qreal dx, qreal dy,
                                    ARGB *image_bits, int image_width, int image_height)
{
    const int fixed_scale = 1 << 16;
    const int half_point = 1 << 15;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    for (int i = 0; i < span->len; ++i) {
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


DrawHelper qDrawHelper =
{
    blend_color,
    blend_transformed,
    blend_transformed_tiled,
    blend_transformed_bilinear,
    blend_transformed_bilinear_tiled
};

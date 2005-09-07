/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
static const int buffer_size = 2048;

typedef void QT_FASTCALL (*CompositionFunction)(uint *dest, const uint *src, int length, int const_alpha);

static void QT_FASTCALL comp_func_Clear(uint *dest, const uint *, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = 0;
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], ialpha);
    }
}

/*
Dca' = Sca.Da + Sca.(1 - Da)
     = Sca
Da'  = Sa.Da + Sa.(1 - Da)
     = Sa
*/
static void QT_FASTCALL comp_func_Source(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = src[i];
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(src[i], const_alpha, dest[i], ialpha);
    }
}

/*
Dca' = Sca.Da + Sca.(1 - Da) + Dca.(1 - Sa)
     = Sca + Dca.(1 - Sa)
Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
     = Sa + Da - Sa.Da
*/
static void QT_FASTCALL comp_func_SourceOver(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = src[i] + BYTE_MUL(dest[i], 255 - qAlpha(src[i]));
    } else {
        for (int i = 0; i < length; ++i) {
            uint s = BYTE_MUL(src[i], const_alpha);
            dest[i] = s + BYTE_MUL(dest[i], 255 - qAlpha(s));
        }
    }
}

static void QT_FASTCALL comp_func_DestinationOver(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = dest[i] + BYTE_MUL(src[i], 255 - qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = dest[i] + BYTE_MUL(src[i], 255 - qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

/*
  Dca' = Sca.Da
  Da'  = Sa.Da
*/
static void QT_FASTCALL comp_func_SourceIn(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(src[i], qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(src[i], qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

static void QT_FASTCALL comp_func_DestinationIn(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], qAlpha(src[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(dest[i], qAlpha(src[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

/*
 Dca' = Sca.(1 - Da)
 Da'  = Sa.(1 - Da)
*/
static void QT_FASTCALL comp_func_SourceOut(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(src[i], 255 - qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(src[i], 255 - qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

static void QT_FASTCALL comp_func_DestinationOut(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], 255 - qAlpha(src[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(dest[i], 255 - qAlpha(src[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

/*
  Dca' = Sca.Da + Dca.(1 - Sa)
  Dca' = Da.(Sca + Dc.(1 - Sa))
  Da'  = Sa.Da + Da.(1 - Sa)
       = Da
*/
static void QT_FASTCALL comp_func_SourceAtop(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(src[i], qAlpha(dest[i]), dest[i], 255 - qAlpha(src[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = INTERPOLATE_PIXEL_255(src[i], qAlpha(dest[i]), dest[i], 255 - qAlpha(src[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

static void QT_FASTCALL comp_func_DestinationAtop(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(dest[i], qAlpha(src[i]), src[i], 255 - qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = INTERPOLATE_PIXEL_255(dest[i], qAlpha(src[i]), src[i], 255 - qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

/*
  Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
  Da'  = Sa.(1 - Da) + Da.(1 - Sa)
       = Sa + Da - 2.Sa.Da
*/
static void QT_FASTCALL comp_func_XOR(uint *dest, const uint *src, int length, int const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(src[i], 255 - qAlpha(dest[i]), dest[i], 255 - qAlpha(src[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = INTERPOLATE_PIXEL_255(src[i], 255 - qAlpha(dest[i]), dest[i], 255 - qAlpha(src[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
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
    case QPainter::CompositionMode_Destination:
        return 0;
    }
    return 0;
}

static void blend_color_argb(void *t, const QSpan *span, QPainter::CompositionMode mode, const BlendColorData *data)
{
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];

    uint *target = ((uint *)t) + span->x;
    uint color = data->color;
    int length = span->len;

    {
        // #####
        int l = qMin(length, buffer_size);
        for (int i = 0; i < l; ++i)
            buffer[i] = color;
    }
    
    while (length) {
        int l = qMin(length, buffer_size);
        func(target, buffer, l, span->coverage);
        length -= l;
        target += l;
    }
}
    
static void blend_argb(void *t, const QSpan *span, const qreal dx, const qreal dy,
                       const void *ibits, const int image_width, const int image_height,
                       QPainter::CompositionMode mode)
{
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;

    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    if (y < 0 || y >= image_height || x > image_width)
        return;
    int length = span->len;
    uint *target = ((uint *)t) + span->x;
    const uint *src = (uint *)ibits + y*image_width + x;
    if (x < 0) {
        src -= x;
        target -= x;
        length += x;
        x = 0;
    }
    if (x + length > image_width)
        length = image_width - x;

    func(target, src, length, span->coverage);
}

static void blend_tiled_argb(void *t, const QSpan *span,
                             const qreal dx, const qreal dy,
                             const void *ibits, const int image_width, const int image_height,
                             QPainter::CompositionMode mode)
{
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];
    
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

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        for (int i = 0; i < l; ++i)
            buffer[i] = src[i%image_width];
        func(target, buffer, l, span->coverage);
        length -= l;
        target += l;
    }
}

static void blend_transformed_bilinear_argb(void *t, const QSpan *span,
                                            const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                            const void *ibits, const int image_width, const int image_height,
                                            QPainter::CompositionMode mode)
{
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];
    
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    int x = int((ix + dx * span->x) * fixed_scale) - half_point;
    int y = int((iy + dy * span->x) * fixed_scale) - half_point;

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        const uint *end = buffer + l;
        uint *b = buffer;
        while (b < end) {
            int x1 = (x >> 16);
            int x2 = x1 + 1;
            int y1 = (y >> 16);
            int y2 = y1 + 1;

            int distx = ((x - (x1 << 16)) >> 8);
            int disty = ((y - (y1 << 16)) >> 8);
            int idistx = 256 - distx;
            int idisty = 256 - disty;

            bool x1_out = ((x1 < 0) || (x1 >= image_width));
            bool x2_out = ((x2 < 0) || (x2 >= image_width));
            bool y1_out = ((y1 < 0) || (y1 >= image_height));
            bool y2_out = ((y2 < 0) || (y2 >= image_height));

            int y1_offset = y1 * image_width;
            int y2_offset = y1_offset + image_width;

            uint tl = (x1_out || y1_out) ? uint(0) : image_bits[y1_offset + x1];
            uint tr = (x2_out || y1_out) ? uint(0) : image_bits[y1_offset + x2];
            uint bl = (x1_out || y2_out) ? uint(0) : image_bits[y2_offset + x1];
            uint br = (x2_out || y2_out) ? uint(0) : image_bits[y2_offset + x2];

            uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
            uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
            *b = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);
            ++b;

            x += fdx;
            y += fdy;
        }
        func(target, buffer, l, span->coverage);
        target += l;
        length -= l;
    }
}

static void blend_transformed_bilinear_tiled_argb(void *t, const QSpan *span,
                                                  const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                                  const void *ibits, const int image_width, const int image_height,
                                                  QPainter::CompositionMode mode)
{
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];

    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    int x = int((ix + dx * span->x) * fixed_scale) - half_point;
    int y = int((iy + dy * span->x) * fixed_scale) - half_point;

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        const uint *end = buffer + l;
        uint *b = buffer;
        while (b < end) {
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
            *b = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);
            ++b;
            x += fdx;
            y += fdy;
        }
        func(target, buffer, l, span->coverage);
        target += l;
        length -= l;
    }
}

static void blend_transformed_argb(void *t, const QSpan *span,
                                   const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                   const void *ibits, const int image_width, const int image_height,
                                   QPainter::CompositionMode mode)
{
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];
    
    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        const uint *end = buffer + l;
        uint *b = buffer;
        while (b < end) {
            int px = x >> 16;
            int py = y >> 16;

            bool out = (px < 0) || (px >= image_width)
                       || (py < 0) || (py >= image_height);

            int y_offset = py * image_width;
            *b = out ? uint(0) : image_bits[y_offset + px];
            x += fdx;
            y += fdy;
            ++b;
        }
        func(target, buffer, l, span->coverage);
        target += l;
        length -= l;
    }
}

static void blend_transformed_tiled_argb(void *t, const QSpan *span,
                                         const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                         const void *ibits, const int image_width, const int image_height,
                                         QPainter::CompositionMode mode)
{
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];

    uint *target = ((uint *)t) + span->x;
    uint *image_bits = (uint *)ibits;

    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        const uint *end = buffer + l;
        uint *b = buffer;
        while (b < end) {
            int px = x >> 16;
            int py = y >> 16;
            px %= image_width;
            py %= image_height;
            if (px < 0) px += image_width;
            if (py < 0) py += image_height;
            int y_offset = py * image_width;

            Q_ASSERT(px >= 0 && px < image_width);
            Q_ASSERT(py >= 0 && py < image_height);

            *b = image_bits[y_offset + px];
            x += fdx;
            y += fdy;
            ++b;
        }
        func(target, buffer, l, span->coverage);
        target += l;
        length -= l;
    }
}

static void blend_linear_gradient_argb(void *t, const QSpan *span, LinearGradientData *data, qreal ybase, int,
                                       QPainter::CompositionMode mode)
{
    if (mode == QPainter::CompositionMode_SourceOver && !data->alphaColor)
        mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];
    
    uint *target = ((uint *)t) + span->x;
    qreal x1 = data->origin.x();
    qreal tt = ybase + data->xincr * (span->x - x1);

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        for (int i = 0; i < l; ++i) {
            buffer[i] = qt_gradient_pixel(data, tt);
            tt += data->xincr;
        }
        func(target, buffer, l, span->coverage);
        target += l;
        length -= l;
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
    if (mode == QPainter::CompositionMode_SourceOver && !data->alphaColor)
        mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];

    uint *target = ((uint *)t) + span->x;
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

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        for (int i = 0; i < l; ++i) {
            double b  = 2*(rx*dx + ry*dy);
            double det = determinant(a, b , -(rx*rx + ry*ry));
            double s = realRoots(a, b, sqrt(det));

            buffer[i] = qt_gradient_pixel(data,  s);
            rx += cx;
            ry += cy;
        }
        func(target, buffer, l, span->coverage);
        target += l;
        length -= l;
    }        
}

static void blend_conical_gradient_argb(void *t, const QSpan *span, ConicalGradientData *data,
                                        int y, QPainter::CompositionMode mode)
{
//     if (mode == QPainter::CompositionMode_SourceOver && !data->alphaColor)
//         mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode(mode);
    if (!func)
        return;
    uint buffer[buffer_size];

    uint *target = ((uint *)t) + span->x;

    QMatrix m = data->imatrix;
    qreal ix = m.m21() * y + m.dx();
    qreal iy = m.m22() * y + m.dy();
    qreal cx = m.m11();
    qreal cy = m.m12();
    qreal rx = ix + cx * span->x - data->center.x();
    qreal ry = iy + cy * span->x - data->center.y();

    int length = span->len;
    while (length) {
        int l = qMin(length, buffer_size);
        for (int i = 0; i < l; ++i) {
            double angle = atan2(ry, rx);
            angle += data->angle;
            buffer[i] = qt_gradient_pixel(data, angle / (2*Q_PI));
            rx += cx;
            ry += cy;
        }
        func(target, buffer, l, span->coverage);
        target += l;
        length -= l;
    }        
}

// ************************** RGB32 handling ******************************

#if 0
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
        uint *src_end = image_bits + (image_width * image_height);
        while (target < end && src < src_end) {
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

        bool x1_out = ((x1 < 0) || (x1 >= image_width));
        bool x2_out = ((x2 < 0) || (x2 >= image_width));
        bool y1_out = ((y1 < 0) || (y1 >= image_height));
        bool y2_out = ((y2 < 0) || (y2 >= image_height));

        int y1_offset = y1 * image_width;
        int y2_offset = y1_offset + image_width;

        uint tl = (x1_out || y1_out) ? uint(0) : image_bits[y1_offset + x1];
        uint tr = (x2_out || y1_out) ? uint(0) : image_bits[y1_offset + x2];
        uint bl = (x1_out || y2_out) ? uint(0) : image_bits[y2_offset + x1];
        uint br = (x2_out || y2_out) ? uint(0) : image_bits[y2_offset + x2];

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

        bool out = (px < 0) || (px >= image_width)
                   || (py < 0) || (py >= image_height);

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
#endif

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

    const uint *src = image_bits + y*image_width;
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

        bool out = (px < 0) || (px >= image_width)
                   || (py < 0) || (py >= image_height);
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

    const uint *src = image_bits + y*image_width;
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

        bool out = (px < 0) || (px >= image_width)
                   || (py < 0) || (py >= image_height);
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



#ifdef Q_WS_QWS

// ************************** 16-bpp RGB16 handling ******************************


#include <qscreen_qws.h>

static inline ushort qt_blend_pixel_rgb16(ushort dest, uint src, uint coverage)
{
    MASK(src, coverage);
    int rev_alpha = 255 - qAlpha(src);
    if (!rev_alpha)
        return qt_convRgbTo16(src);

    return qt_convRgbTo16(src + BYTE_MUL(qt_conv16ToRgb(dest), rev_alpha));
}


static void blend_color_rgb16(void *t, const QSpan *span, QPainter::CompositionMode, const BlendColorData *data)
{
    ushort *target = ((ushort *)t) + span->x;
    uint color = data->color;
    MASK(color, span->coverage);

    int alpha = qAlpha(color);
    if (!alpha)
        return;
#if 1
    ushort pixel = qt_convRgbTo16(color);
    const ushort *end = target + span->len;
    while (target < end) {
        *target = pixel;
        ++target;
    }
#else
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
#endif
}

static void blend_rgb16(void *t, const QSpan *span,
                        const qreal dx, const qreal dy,
                        const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode mode)
{
    //src is known to be 32 bpp


    ushort *target = ((ushort *)t) + span->x;
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    //     qDebug("x=%f,y=%f %d/%d image_height=%d", dx, dy, x, y, image_height);
    if (y < 0 || y >= image_height)
        return;


    const uint *src = image_bits + y*image_width + x;
    const ushort *end = target + span->len;
    if (x < 0) {
        src -= x;
        target -= x;
        x = 0;
    }
    if (end - target > image_width)
        end = target + image_width;

    if (mode == QPainter::CompositionMode_Source && span->coverage == 255) {
        while (target < end) {
            *target++ = qt_convRgbTo16(*src++);
        }
    } else {
        while (target < end) {
            *target = qt_blend_pixel_rgb16(*target, *src, span->coverage);
            ++target;
            ++src;
        }
    }


}

static void blend_tiled_rgb16(void *t, const QSpan *span,
                              const qreal dx, const qreal dy,
                              const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    ushort *target = ((ushort *)t) + span->x;
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
        *target = qt_blend_pixel_rgb16(*target, src[i%image_width], span->coverage);
        ++target;
    }
}


#if 0
static void blend_transformed_bilinear_rgb16(void *t, const QSpan *span,
                                             const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                             const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    qDebug("not implemented blend_transformed_bilinear_rgb16");
#if 0
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

        bool x1_out = ((x1 < 0) || (x1 >= image_width));
        bool x2_out = ((x2 < 0) || (x2 >= image_width));
        bool y1_out = ((y1 < 0) || (y1 >= image_height));
        bool y2_out = ((y2 < 0) || (y2 >= image_height));

        int y1_offset = y1 * image_width;
        int y2_offset = y1_offset + image_width;

        uint tl = (x1_out || y1_out) ? uint(0) : image_bits[y1_offset + x1];
        uint tr = (x2_out || y1_out) ? uint(0) : image_bits[y1_offset + x2];
        uint bl = (x1_out || y2_out) ? uint(0) : image_bits[y2_offset + x1];
        uint br = (x2_out || y2_out) ? uint(0) : image_bits[y2_offset + x2];

        uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
        uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
        uint res = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);

        *target = qt_blend_pixel_rgb16(*target, res, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
#endif
}

static void blend_transformed_bilinear_tiled_rgb16(void *t, const QSpan *span,
                                                   const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                                   const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    qDebug("not implemented blend_transformed_bilinear_tiled_rgb16");
#if 0
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

        *target = qt_blend_pixel_rgb16(*target, res, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
#endif
}

static void blend_transformed_rgb16(void *t, const QSpan *span,
                                    const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                    const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
        qDebug("not implemented blend_transformed_rgb16");
#if 0
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

        bool out = (px < 0) || (px >= image_width)
                   || (py < 0) || (py >= image_height);

        int y_offset = py * image_width;

        uint pixel = out ? uint(0) : image_bits[y_offset + px];

        *target = qt_blend_pixel_rgb16(*target, pixel, span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
#endif
}

static void blend_transformed_tiled_rgb16(void *t, const QSpan *span,
                                          const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                          const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    qDebug("not implemented blend_transformed_tiled_rgb16");
#if 0
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

        *target = qt_blend_pixel_rgb16(*target, image_bits[y_offset + px], span->coverage);
        x += fdx;
        y += fdy;
        ++target;
    }
#endif
}

static void blend_linear_gradient_rgb16(void *t, const QSpan *span, LinearGradientData *data, qreal ybase, int, QPainter::CompositionMode)
{
    qDebug("not implemented blend_transformed_bilinear_rgb16");
#if 0
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
            *target = qt_blend_pixel_rgb16(*target, src, span->coverage);
            ++target;
            tt += data->xincr;
        }
    }
#endif
}

static void blend_radial_gradient_rgb16(void *t, const QSpan *span, RadialGradientData *data,
                                       int y, QPainter::CompositionMode mode)
{
    qDebug("blend_radial_gradient_rgb16 not implemented");
}

static void blend_conical_gradient_rgb16(void *t, const QSpan *span, ConicalGradientData *data,
                                        int y, QPainter::CompositionMode mode)
{
    qDebug("blend_conical_gradient_rgb16 not implemented");
}
#endif // 0

static inline QRgb qt_conv_4ToRgb(uchar g)
{
    g = g | g << 4;
    return qRgb(g, g, g);
}

static inline uchar qt_conv_RgbTo4(QRgb c)
{
    return qGray(c) >> 4;
}

#if 0
static inline uchar qt_blend_pixel_gray4(uchar dest, uint nibble, uint src, uint coverage)
{
    MASK(src, coverage);
    int rev_alpha = 255 - qAlpha(src);
    if (!rev_alpha)
        return qt_convRgbTo4(src);

    return qt_convRgbTo4(src + BYTE_MUL(qt_conv4ToRgb(dest), rev_alpha));
}
#endif


static void blend_color_gray4_lsb(void *t, const QSpan *span, QPainter::CompositionMode, const BlendColorData *data)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uint color = data->color;

    int x0 = span->x;
    int x1 = span->x + span->len;

    int preAdd = x0 & 1;
    int postAdd = x1 & 1;

    uchar g = qGray(color) >> 4;
    uchar gg = g | g << 4;

    uchar *target = (uchar *)t + (x0 >> 1);
    int len = (x1-postAdd) - (x0+preAdd);
    len >>= 1;
    Q_ASSERT(len >= 0);

    if (preAdd) {
        *target = (*target & 0xf0) | g;
        ++target;
    }

    while (len--)
        *target++ = gg;

    if (postAdd) {
        *target = (*target & 0x0f) | g << 4;
        ++target;
    }

    //### no alpha blending implemented
}





static void blend_tiled_gray4_lsb(void *t, const QSpan *span,
                        const qreal dx, const qreal dy,
                        const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{
    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    if (y < 0 || y >= image_height)
        return;

    const uint *src = image_bits + y*image_width;

    int x0 = span->x;
    int x1 = span->x + span->len;

    int preAdd = x0 & 1;
    int postAdd = x1 & 1;

    uchar *target = (uchar *)t + (x0 >> 1);
    int len = (x1-postAdd) - (x0+preAdd);
    len >>= 1;
    Q_ASSERT(len >= 0);

    int i = x;
    if (preAdd) {
        *target = (*target & 0xf0) | (qGray(src[i%image_width]) >> 4);
        ++target;
        ++i;
    }

    while (len--) {
        uchar gg =  qGray(src[i++%image_width]) >> 4;
        gg |= qGray(src[i++%image_width]) & 0xf0;
        *target++ = gg;
    }

    if (postAdd) {
        *target = (*target & 0x0f) | ( qGray(src[i++%image_width]) & 0xf0);
        ++target;
    }

}

static void blend_gray4_lsb(void *t, const QSpan *span,
                              const qreal dx, const qreal dy,
                              const void *ibits, const int image_width, const int image_height, QPainter::CompositionMode)
{


    if (!span->coverage)
        return;
    Q_ASSERT(span->coverage == 0xff);
    uint *image_bits = (uint *)ibits;
    // #### take care of non integer dx/dy
    int x = qRound(dx);
    int y = qRound(dy);
    if (y < 0 || y >= image_height)
        return;

    const uint *src = image_bits + y*image_width;

    int x0 = span->x;
    int x1 = span->x + span->len;

    int preAdd = x0 & 1;
    int postAdd = x1 & 1;

    uchar *target = (uchar *)t + (x0 >> 1);
    int len = (x1-postAdd) - (x0+preAdd);
    len >>= 1;
    Q_ASSERT(len >= 0);

    const uint *p = src + x;
    if (preAdd) {
        *target = (*target & 0xf0) | (qGray(*p++) >> 4);
        ++target;
    }

    while (len--) {
        uchar gg =  qGray(*p++) >> 4;
        gg |= qGray(*p++) & 0xf0;
        *target++ = gg;
    }

    if (postAdd) {
        *target = (*target & 0x0f) | (qGray(*p) & 0xf0);
        ++target;
    }

}


#endif //Q_WS_QWS



DrawHelper qDrawHelper[DrawHelper::Layout_Count] =
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
# if 0
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
#else
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
#endif
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
#ifdef Q_WS_QWS
    ,    { // Layout_RGB16
        blend_color_rgb16,
        blend_rgb16,
        blend_tiled_rgb16,
        0, //blend_transformed_rgb16,
        0, //blend_transformed_tiled_rgb16,
        0, //blend_transformed_rgb16,
        0, //blend_transformed_tiled_rgb16,
        0, //blend_linear_gradient_rgb16,
        0, //blend_radial_gradient_rgb16,
        0 //blend_conical_gradient_rgb16
    }
    ,    { // Layout_Gray4LSB
        blend_color_gray4_lsb,
        blend_gray4_lsb,
        blend_tiled_gray4_lsb,
        0, //blend_transformed_gray4_lsb,
        0, //blend_transformed_tiled_gray4_lsb,
        0, //blend_transformed_gray4_lsb,
        0, //blend_transformed_tiled_gray4_lsb,
        0, //blend_linear_gradient_gray4_lsb,
        0, //blend_radial_gradient_gray4_lsb,
        0  //blend_conical_gradient_gray4_lsb
    }
#endif
};

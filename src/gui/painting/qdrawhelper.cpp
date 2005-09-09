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
#include <private/qpaintengine_raster_p.h>
#include <math.h>
#include <private/qmath_p.h>
#define MASK(src, a) src = BYTE_MUL(src, a)

static const int fixed_scale = 1 << 16;
static const int half_point = 1 << 15;
static const int buffer_size = 2048;

static uint qt_gradient_pixel(const GradientData *data, double pos)
{
    int ipos = qRound(pos * GRADIENT_STOPTABLE_SIZE - 1);

  // calculate the actual offset.
    if (ipos < 0 || ipos >= GRADIENT_STOPTABLE_SIZE) {
        if (data->spread == QGradient::RepeatSpread) {
            ipos = ipos % GRADIENT_STOPTABLE_SIZE;
            ipos = ipos < 0 ? GRADIENT_STOPTABLE_SIZE + ipos : ipos;

        } else if (data->spread == QGradient::ReflectSpread) {
            const int limit = GRADIENT_STOPTABLE_SIZE * 2 - 1;
            ipos = ipos % limit;
            ipos = ipos < 0 ? limit + ipos : ipos;
            ipos = ipos >= GRADIENT_STOPTABLE_SIZE ? limit - ipos : ipos;

        } else {
            if (ipos < 0) ipos = 0;
            else if (ipos >= GRADIENT_STOPTABLE_SIZE) ipos = GRADIENT_STOPTABLE_SIZE-1;
        }
    }

    Q_ASSERT(ipos >= 0);
    Q_ASSERT(ipos < GRADIENT_STOPTABLE_SIZE);

    return data->colorTable[ipos];
}



static void QT_FASTCALL comp_func_solid_Clear(uint *dest, int length, const uint, uint const_alpha)
{
    if (const_alpha == 255) {
        qt_memfill_uint(dest, length, 0);
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
static void QT_FASTCALL comp_func_solid_Source(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        qt_memfill_uint(dest, length, color);
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(color, const_alpha, dest[i], ialpha);
    }
}

/*
Dca' = Sca.Da + Sca.(1 - Da) + Dca.(1 - Sa)
     = Sca + Dca.(1 - Sa)
Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
     = Sa + Da - Sa.Da
*/
static void QT_FASTCALL comp_func_solid_SourceOver(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha != 255)
        color = BYTE_MUL(color, const_alpha);
    if (qAlpha(color) == 255)
        qt_memfill_uint(dest, length, color);
    else
        for (int i = 0; i < length; ++i)
            dest[i] = color + BYTE_MUL(dest[i], 255 - qAlpha(color));
}

static void QT_FASTCALL comp_func_solid_DestinationOver(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = dest[i] + BYTE_MUL(color, 255 - qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = dest[i] + BYTE_MUL(color, 255 - qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

/*
  Dca' = Sca.Da
  Da'  = Sa.Da
*/
static void QT_FASTCALL comp_func_solid_SourceIn(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(color, qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(color, qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

static void QT_FASTCALL comp_func_solid_DestinationIn(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], qAlpha(color));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(dest[i], qAlpha(color));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

/*
 Dca' = Sca.(1 - Da)
 Da'  = Sa.(1 - Da)
*/
static void QT_FASTCALL comp_func_solid_SourceOut(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(color, 255 - qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(color, 255 - qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

static void QT_FASTCALL comp_func_solid_DestinationOut(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], 255 - qAlpha(color));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = BYTE_MUL(dest[i], 255 - qAlpha(color));
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
static void QT_FASTCALL comp_func_solid_SourceAtop(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(color, qAlpha(dest[i]), dest[i], 255 - qAlpha(color));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = INTERPOLATE_PIXEL_255(color, qAlpha(dest[i]), dest[i], 255 - qAlpha(color));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

static void QT_FASTCALL comp_func_solid_DestinationAtop(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(dest[i], qAlpha(color), color, 255 - qAlpha(dest[i]));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = INTERPOLATE_PIXEL_255(dest[i], qAlpha(color), color, 255 - qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}

/*
  Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
  Da'  = Sa.(1 - Da) + Da.(1 - Sa)
       = Sa + Da - 2.Sa.Da
*/
static void QT_FASTCALL comp_func_solid_XOR(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(color, 255 - qAlpha(dest[i]), dest[i], 255 - qAlpha(color));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = INTERPOLATE_PIXEL_255(color, 255 - qAlpha(dest[i]), dest[i], 255 - qAlpha(color));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
}


static const CompositionFunctionSolid functionForModeSolid_C[] = {
        comp_func_solid_SourceOver,
        comp_func_solid_DestinationOver,
        comp_func_solid_Clear,
        comp_func_solid_Source,
        0,
        comp_func_solid_SourceIn,
        comp_func_solid_DestinationIn,
        comp_func_solid_SourceOut,
        comp_func_solid_DestinationOut,
        comp_func_solid_SourceAtop,
        comp_func_solid_DestinationAtop,
        comp_func_solid_XOR
};

static const CompositionFunctionSolid *functionForModeSolid = functionForModeSolid_C;



static void QT_FASTCALL comp_func_Clear(uint *dest, const uint *, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        qt_memfill_uint(dest, length, 0);
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
static void QT_FASTCALL comp_func_Source(uint *dest, const uint *src, int length, uint const_alpha)
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
static void QT_FASTCALL comp_func_SourceOver(uint *dest, const uint *src, int length, uint const_alpha)
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

static void QT_FASTCALL comp_func_DestinationOver(uint *dest, const uint *src, int length, uint const_alpha)
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
static void QT_FASTCALL comp_func_SourceIn(uint *dest, const uint *src, int length, uint const_alpha)
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

static void QT_FASTCALL comp_func_DestinationIn(uint *dest, const uint *src, int length, uint const_alpha)
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
static void QT_FASTCALL comp_func_SourceOut(uint *dest, const uint *src, int length, uint const_alpha)
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

static void QT_FASTCALL comp_func_DestinationOut(uint *dest, const uint *src, int length, uint const_alpha)
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
static void QT_FASTCALL comp_func_SourceAtop(uint *dest, const uint *src, int length, uint const_alpha)
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

static void QT_FASTCALL comp_func_DestinationAtop(uint *dest, const uint *src, int length, uint const_alpha)
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
static void QT_FASTCALL comp_func_XOR(uint *dest, const uint *src, int length, uint const_alpha)
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

static const CompositionFunction functionForMode_C[] = {
        comp_func_SourceOver,
        comp_func_DestinationOver,
        comp_func_Clear,
        comp_func_Source,
        0,
        comp_func_SourceIn,
        comp_func_DestinationIn,
        comp_func_SourceOut,
        comp_func_DestinationOut,
        comp_func_SourceAtop,
        comp_func_DestinationAtop,
        comp_func_XOR
};

static const CompositionFunction *functionForMode = functionForMode_C;

static void blend_color_argb(int y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    CompositionFunctionSolid func = functionForModeSolid[data->compositionMode];
    if (!func)
        return;
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uint *target = ((uint *)t) + spans->x;
        func(target, spans->len, data->solid.color, spans->coverage);
        ++spans;
    }
}

static void blend_argb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    QPainter::CompositionMode mode = data->compositionMode;
    if (!data->texture.hasAlpha && mode == QPainter::CompositionMode_SourceOver)
        mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode[mode];
    if (!func)
        return;
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;

        // #### take care of non integer dx/dy
        int x = qRound(dx);
        int y = qRound(dy);
        if (y < 0 || y >= image_height || x > image_width)
            return;
        int length = spans->len;
        uint *target = ((uint *)t) + spans->x;
        const uint *src = (uint *)ibits + y*image_width + x;
        if (x < 0) {
            src -= x;
            target -= x;
            length += x;
            x = 0;
        }
        if (x + length > image_width)
            length = image_width - x;

        func(target, src, length, spans->coverage);
        ++spans;
    }
}

static void blend_tiled_argb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    QPainter::CompositionMode mode = data->compositionMode;
    if (!data->texture.hasAlpha && mode == QPainter::CompositionMode_SourceOver)
        mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode[mode];
    if (!func)
        return;

    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
                     
        uint buffer[buffer_size];

        uint *target = ((uint *)t) + spans->x;
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

        int length = spans->len;
        while (length) {
            int l = qMin(length, buffer_size);
            for (int i = 0; i < l; ++i)
                buffer[i] = src[i%image_width];
            func(target, buffer, l, spans->coverage);
            length -= l;
            target += l;
        }
        ++spans;
    }
}

static void blend_transformed_bilinear_argb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    CompositionFunction func = functionForMode[data->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];

    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;
    
        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)ibits;
        int x = int((ix + dx * spans->x) * fixed_scale) - half_point;
        int y = int((iy + dy * spans->x) * fixed_scale) - half_point;

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        int length = spans->len;
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
            func(target, buffer, l, spans->coverage);
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_transformed_bilinear_tiled_argb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    CompositionFunction func = functionForMode[data->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];

    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;

        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)ibits;
        int x = int((ix + dx * spans->x) * fixed_scale) - half_point;
        int y = int((iy + dy * spans->x) * fixed_scale) - half_point;

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        int length = spans->len;
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
            func(target, buffer, l, spans->coverage);
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_transformed_argb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    CompositionFunction func = functionForMode[data->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {

        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;

        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)ibits;

        int x = int((ix + dx * spans->x) * fixed_scale);
        int y = int((iy + dy * spans->x) * fixed_scale);

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        int length = spans->len;
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
            func(target, buffer, l, spans->coverage);
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_transformed_tiled_argb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    CompositionFunction func = functionForMode[data->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];

    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;

        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)ibits;

        int x = int((ix + dx * spans->x) * fixed_scale);
        int y = int((iy + dy * spans->x) * fixed_scale);

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        int length = spans->len;
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
            func(target, buffer, l, spans->coverage);
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_linear_gradient_argb(int y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    QPainter::CompositionMode mode = data->compositionMode;
    if (mode == QPainter::CompositionMode_SourceOver && !data->gradient.alphaColor)
        mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode[mode];
    if (!func)
        return;
    uint buffer[buffer_size];

    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        qreal ybase = (y - data->gradient.linear.origin.y) * data->gradient.linear.yincr;
        uint *target = ((uint *)t) + spans->x;
        qreal x1 = data->gradient.linear.origin.x;
        qreal tt = ybase + data->gradient.linear.xincr * (spans->x - x1);

        int length = spans->len;
        while (length) {
            int l = qMin(length, buffer_size);
            for (int i = 0; i < l; ++i) {
                buffer[i] = qt_gradient_pixel(&data->gradient, tt);
                tt += data->gradient.linear.xincr;
            }
            func(target, buffer, l, spans->coverage);
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static inline double determinant(double a, double b, double c)
{
    return (b * b) - (4 * a * c);
}

// function to evaluate real roots
static inline double realRoots(double a, double b, double detSqrt)
{
    return (-b + detSqrt)/(2 * a);
}

static void blend_radial_gradient_argb(int y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    QPainter::CompositionMode mode = data->compositionMode;
    if (mode == QPainter::CompositionMode_SourceOver && !data->gradient.alphaColor)
        mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode[mode];
    if (!func)
        return;
    uint buffer[buffer_size];
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {

        uint *target = ((uint *)t) + spans->x;
        double dx = data->gradient.radial.center.x - data->gradient.radial.focal.x;
        double dy = data->gradient.radial.center.y - data->gradient.radial.focal.y;
        double r  = data->gradient.radial.radius;
        double a = r*r - dx*dx - dy*dy;

        qreal ix = data->m21 * y + data->dx;
        qreal iy = data->m22 * y + data->dy;
        qreal cx = data->m11;
        qreal cy = data->m12;
        qreal rx = ix + cx * spans->x - data->gradient.radial.focal.x;
        qreal ry = iy + cy * spans->x - data->gradient.radial.focal.y;

        int length = spans->len;
        while (length) {
            int l = qMin(length, buffer_size);
            for (int i = 0; i < l; ++i) {
                double b  = 2*(rx*dx + ry*dy);
                double det = determinant(a, b , -(rx*rx + ry*ry));
                double s = realRoots(a, b, sqrt(det));

                buffer[i] = qt_gradient_pixel(&data->gradient,  s);
                rx += cx;
                ry += cy;
            }
            func(target, buffer, l, spans->coverage);
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_conical_gradient_argb(int y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    QPainter::CompositionMode mode = data->compositionMode;
    if (mode == QPainter::CompositionMode_SourceOver && !data->gradient.alphaColor)
        mode = QPainter::CompositionMode_Source;
    CompositionFunction func = functionForMode[data->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {

        uint *target = ((uint *)t) + spans->x;

        qreal ix = data->m21 * y + data->dx;
        qreal iy = data->m22 * y + data->dy;
        qreal cx = data->m11;
        qreal cy = data->m12;
        qreal rx = ix + cx * spans->x - data->gradient.radial.center.x;
        qreal ry = iy + cy * spans->x - data->gradient.radial.center.y;

        int length = spans->len;
        while (length) {
            int l = qMin(length, buffer_size);
            for (int i = 0; i < l; ++i) {
                double angle = atan2(ry, rx);
                angle += data->gradient.conical.angle;
                buffer[i] = qt_gradient_pixel(&data->gradient, angle / (2*Q_PI));
                rx += cx;
                ry += cy;
            }
            func(target, buffer, l, spans->coverage);
            target += l;
            length -= l;
        }
        ++spans;
    }
}

/************************************* Mono ************************************/

static void blend_color_mono(int y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uchar *target = (uchar *)t;
        uint color = data->solid.color;

        if (color == 0xff000000) {
            for (int i = spans->x; i < spans->x + spans->len; ++i) {
                target[i>>3] |= 0x80 >> (i & 7);
            }
        } else if (color == 0xffffffff) {
            for (int i = spans->x; i < spans->x + spans->len; ++i) {
                target[i>>3] &= ~(0x80 >> (i & 7));
            }
        } else {
            uint g = qGray(color);
            for (int i = spans->x; i < spans->x + spans->len; ++i) {
                if (g < qt_bayer_matrix[y & 15][i & 15])
                    target[i >> 3] |= 0x80 >> (i & 7);
                else
                    target[i >> 3] &= ~(0x80 >> (i & 7));
            }
        }
        ++spans;
    }
}

static void blend_mono(int _y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {

        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
        uchar *target = (uchar *)t;
        uint *image_bits = (uint *)ibits;
        // #### take care of non integer dx/dy
        int x = qRound(dx);
        int y = qRound(dy);
        if (y < 0 || y >= image_height)
            return;

        const uint *src = image_bits + y*image_width;
        for (int i = 0; i < spans->len; ++i) {
            int sx = x + i;
            int dx = spans->x + i;
            uint p = src[sx];
            if (qGray(p) < int(qt_bayer_matrix[y & 15][(sx) & 15]))
                target[dx >> 3] |= 0x80 >> (dx & 7);
            else
                target[dx >> 3] &= ~(0x80 >> (dx & 7));
        }
        ++spans;
    }
}

static void blend_tiled_mono(int _y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {

        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;
    
        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
    
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
        for (int i = 0; i < spans->len; ++i) {
            int sx = x + i;
            int dx = spans->x + i;
            uint p = src[sx%image_width];
            if (qGray(p) < int(qt_bayer_matrix[y & 15][sx & 15]))
                target[dx >> 3] |= 0x80 >> (dx & 7);
            else
                target[dx >> 3] &= ~(0x80 >> (dx & 7));
        }
        ++spans;
    }
}


static void blend_transformed_mono(int _y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);

    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;

        uchar *target = (uchar *)t;
        uint *image_bits = (uint *)ibits;

        int x = int((ix + dx * spans->x) * fixed_scale);
        int y = int((iy + dy * spans->x) * fixed_scale);

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        for (int i = 0; i < spans->len; ++i) {
            int px = x >> 16;
            int py = y >> 16;

            bool out = (px < 0) || (px >= image_width)
                       || (py < 0) || (py >= image_height);
            if (out)
                continue;

            int y_offset = py * image_width;

            int dx = spans->x + i;
            uint p = image_bits[y_offset + px];
            if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
                target[dx >> 3] |= 0x80 >> (dx & 7);
            else
                target[dx >> 3] &= ~(0x80 >> (dx & 7));

            x += fdx;
            y += fdy;
        }
        ++spans;
    }
}

static void blend_transformed_tiled_mono(int _y, int count, QT_FT_Span *spans, void *userData) 
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);

    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;

        uchar *target = (uchar *)t;
        uint *image_bits = (uint *)ibits;

        int x = int((ix + dx * spans->x) * fixed_scale);
        int y = int((iy + dy * spans->x) * fixed_scale);

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        for (int i = 0; i < spans->len; ++i) {
            int px = x >> 16;
            int py = y >> 16;
            px %= image_width;
            py %= image_height;
            if (px < 0) px += image_width;
            if (py < 0) py += image_height;
            int y_offset = py * image_width;

            Q_ASSERT(px >= 0 && px < image_width);
            Q_ASSERT(py >= 0 && py < image_height);

            int dx = spans->x + i;
            uint p = image_bits[y_offset + px];
            if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
                target[dx >> 3] |= 0x80 >> (dx & 7);
            else
                target[dx >> 3] &= ~(0x80 >> (dx & 7));

            x += fdx;
            y += fdy;
        }
        ++spans;
    }
}

static void blend_linear_gradient_mono(int y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uchar *target = (uchar *)t;

        qreal ybase = (y - data->gradient.linear.origin.y) * data->gradient.linear.yincr;
        qreal x1 = data->gradient.linear.origin.x;
        qreal tt = ybase + data->gradient.linear.xincr * (spans->x - x1);

        for (int x = spans->x; x<spans->x + spans->len; x++) {
            uint p = qt_gradient_pixel(&data->gradient, tt);
            if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
                target[x >> 3] |= 0x80 >> (x & 7);
            else
                target[x >> 3] &= ~(0x80 >> (x & 7));
            tt += data->gradient.linear.xincr;
        }
        ++spans;
    }
}

static void blend_radial_gradient_mono(int y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uchar *target = (uchar *)t;

        double dx = data->gradient.radial.center.x - data->gradient.radial.focal.x;
        double dy = data->gradient.radial.center.y - data->gradient.radial.focal.y;
        double r  = data->gradient.radial.radius;
        double a = r*r - dx*dx - dy*dy;

        qreal ix = data->m21 * y + data->dx;
        qreal iy = data->m22 * y + data->dy;
        qreal cx = data->m11;
        qreal cy = data->m12;
        qreal rx = ix + cx * spans->x - data->gradient.radial.focal.x;
        qreal ry = iy + cy * spans->x - data->gradient.radial.focal.y;

        for (int x = spans->x; x<spans->x + spans->len; x++) {
            double b  = 2*(rx*dx + ry*dy);
            double det = determinant(a, b , -(rx*rx + ry*ry));
            double s = realRoots(a, b, sqrt(det));

            uint p = qt_gradient_pixel(&data->gradient, s);
            if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
                target[x >> 3] |= 0x80 >> (x & 7);
            else
                target[x >> 3] &= ~(0x80 >> (x & 7));
            ++x;
            rx += cx;
            ry += cy;
        }
        ++spans;
    }
}

static void blend_conical_gradient_mono(int y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uchar *target = (uchar *)t;

        qreal ix = data->m21 * y + data->dx;
        qreal iy = data->m22 * y + data->dy;
        qreal cx = data->m11;
        qreal cy = data->m12;
        qreal rx = ix + cx * spans->x - data->gradient.radial.center.x;
        qreal ry = iy + cy * spans->x - data->gradient.radial.center.y;

        for (int x = spans->x; x<spans->x + spans->len; x++) {
            double angle = atan2(ry, rx);
            angle += data->gradient.conical.angle;
            uint p = qt_gradient_pixel(&data->gradient, angle / 360.0);
            if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
                target[x >> 3] |= 0x80 >> (x & 7);
            else
                target[x >> 3] &= ~(0x80 >> (x & 7));
            ++x;
            rx += cx;
            ry += cy;
        }
        ++spans;
    }
}

// ************************** Mono LSB ********************************


static void blend_color_mono_lsb(int y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uchar *target = (uchar *)t;
        uint color = data->solid.color;

        if (color == 0xffffffff) {
            for (int i = spans->x; i < spans->x + spans->len; ++i) {
                target[i>>3] &= ~(1 << (i & 7));
            }
        } else if (color == 0xff000000) {
            for (int i = spans->x; i < spans->x + spans->len; ++i) {
                target[i>>3] |= 1 << (i & 7);
            }
        } else {
            uint g = qGray(color);
            for (int i = spans->x; i < spans->x + spans->len; ++i) {
                if (g < qt_bayer_matrix[y & 15][i & 15])
                    target[i >> 3] |= 1 << (i & 7);
                else
                    target[i >> 3] &= ~(1 << (i & 7));
            }
        }
        ++spans;
    }
}

static void blend_mono_lsb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);

    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
    
        uchar *target = (uchar *)t;
        uint *image_bits = (uint *)ibits;
        // #### take care of non integer dx/dy
        int x = qRound(dx);
        int y = qRound(dy);
        if (y < 0 || y >= image_height)
            return;

        const uint *src = image_bits + y*image_width;
        for (int i = 0; i < spans->len; ++i) {
            int sx = x + i;
            int dx = spans->x + i;
            uint p = src[sx];
            if (qGray(p) < int(qt_bayer_matrix[y & 15][(sx) & 15]))
                target[dx >> 3] |= 1 << (dx & 7);
            else
                target[dx >> 3] &= ~(1 << (dx & 7));
        }
        ++spans;
    }
}

static void blend_tiled_mono_lsb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);

    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
    
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
        for (int i = 0; i < spans->len; ++i) {
            int sx = x + i;
            int dx = spans->x + i;
            uint p = src[sx%image_width];
            if (qGray(p) < int(qt_bayer_matrix[y & 15][sx & 15]))
                target[dx >> 3] |= 1 << (dx & 7);
            else
                target[dx >> 3] &= ~(1 << (dx & 7));
        }
        ++spans;
    }
}


static void blend_transformed_mono_lsb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);

    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;

        uchar *target = (uchar *)t;
        uint *image_bits = (uint *)ibits;

        int x = int((ix + dx * spans->x) * fixed_scale);
        int y = int((iy + dy * spans->x) * fixed_scale);

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        for (int i = 0; i < spans->len; ++i) {
            int px = x >> 16;
            int py = y >> 16;

            bool out = (px < 0) || (px >= image_width)
                       || (py < 0) || (py >= image_height);
            if (out)
                continue;

            int y_offset = py * image_width;

            int dx = spans->x + i;
            uint p = image_bits[y_offset + px];
            if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
                target[dx >> 3] |= 1 << (dx & 7);
            else
                target[dx >> 3] &= ~(1 << (dx & 7));

            x += fdx;
            y += fdy;
        }
        ++spans;
    }
}

static void blend_transformed_tiled_mono_lsb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);

    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;

        // Base point for the inversed transform
        qreal ix = data->m21 * _y + data->dx;
        qreal iy = data->m22 * _y + data->dy;

        // The increment pr x in the scanline
        qreal dx = data->m11;
        qreal dy = data->m12;
        const void *ibits = data->texture.imageData;

        uchar *target = (uchar *)t;
        uint *image_bits = (uint *)ibits;

        int x = int((ix + dx * spans->x) * fixed_scale);
        int y = int((iy + dy * spans->x) * fixed_scale);

        int fdx = (int)(dx * fixed_scale);
        int fdy = (int)(dy * fixed_scale);

        for (int i = 0; i < spans->len; ++i) {
            int px = x >> 16;
            int py = y >> 16;
            px %= image_width;
            py %= image_height;
            if (px < 0) px += image_width;
            if (py < 0) py += image_height;
            int y_offset = py * image_width;

            Q_ASSERT(px >= 0 && px < image_width);
            Q_ASSERT(py >= 0 && py < image_height);

            int dx = spans->x + i;
            uint p = image_bits[y_offset + px];
            if (qGray(p) < int(qt_bayer_matrix[py & 15][px & 15]))
                target[dx >> 3] |= 1 << (dx & 7);
            else
                target[dx >> 3] &= ~(1 << (dx & 7));

            x += fdx;
            y += fdy;
        }
        ++spans;
    }
}

static void blend_linear_gradient_mono_lsb(int y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uchar *target = (uchar *)t;

        qreal ybase = (y - data->gradient.linear.origin.y) * data->gradient.linear.yincr;
        qreal x1 = data->gradient.linear.origin.x;
        qreal tt = ybase + data->gradient.linear.xincr * (spans->x - x1);

        for (int x = spans->x; x<spans->x + spans->len; x++) {
            uint p = qt_gradient_pixel(&data->gradient, tt);
            if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
                target[x >> 3] |= 1 << (x & 7);
            else
                target[x >> 3] &= ~(1 << (x & 7));
            tt += data->gradient.linear.xincr;
        }
        ++spans;
    }
}


static void blend_radial_gradient_mono_lsb(int y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);

    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);
    uchar *target = (uchar *)t;

    while (count--) {
        double dx = data->gradient.radial.center.x - data->gradient.radial.focal.x;
        double dy = data->gradient.radial.center.y - data->gradient.radial.focal.y;
        double r  = data->gradient.radial.radius;
        double a = r*r - dx*dx - dy*dy;

        qreal ix = data->m21 * y + data->dx;
        qreal iy = data->m22 * y + data->dy;
        qreal cx = data->m11;
        qreal cy = data->m12;
        qreal rx = ix + cx * spans->x - data->gradient.radial.focal.x;
        qreal ry = iy + cy * spans->x - data->gradient.radial.focal.y;

        for (int x = spans->x; x<spans->x + spans->len; x++) {
            double b  = 2*(rx*dx + ry*dy);
            double det = determinant(a, b , -(rx*rx + ry*ry));
            double s = realRoots(a, b, sqrt(det));

            uint p = qt_gradient_pixel(&data->gradient, s);
            if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
                target[x >> 3] |= 1 >> (x & 7);
            else
                target[x >> 3] &= ~(1 >> (x & 7));
            ++x;
            rx += cx;
            ry += cy;
        }
        ++spans;
    }
}

static void blend_conical_gradient_mono_lsb(int y, int count, QT_FT_Span *spans, void *userData)
{
    if (!spans->coverage)
        return;
    Q_ASSERT(spans->coverage == 0xff);
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        uchar *target = (uchar *)t;

        qreal ix = data->m21 * y + data->dx;
        qreal iy = data->m22 * y + data->dy;
        qreal cx = data->m11;
        qreal cy = data->m12;
        qreal rx = ix + cx * spans->x - data->gradient.radial.center.x;
        qreal ry = iy + cy * spans->x - data->gradient.radial.center.y;

        for (int x = spans->x; x<spans->x + spans->len; x++) {
            double angle = atan2(ry, rx);
            angle += data->gradient.conical.angle;
            uint p = qt_gradient_pixel(&data->gradient, angle / 360.0);
            if (qGray(p) < int(qt_bayer_matrix[y & 15][x & 15]))
                target[x >> 3] |= 1 << (x & 7);
            else
                target[x >> 3] &= ~(1 << (x & 7));
            ++x;
            rx += cx;
            ry += cy;
        }
        ++spans;
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


static void blend_color_rgb16(int y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        ushort *target = ((ushort *)t) + spans->x;
        uint color = data->solid.color;
        MASK(color, spans->coverage);

        int alpha = qAlpha(color);
        if (!alpha)
            return;
#if 1
        ushort pixel = qt_convRgbTo16(color);
        const ushort *end = target + spans->len;
        while (target < end) {
            *target = pixel;
            ++target;
        }
#else
        if (alpha != 0xff) {
            int rev_alpha = 255 - alpha;
            const uint *end = target + spans->len;
            while (target < end) {
                uint dest = *target;
                *target = (color + BYTE_MUL(dest, rev_alpha)) | 0xff000000;
                ++target;
            }
        } else {
#if 1
            const uint *end = target + spans->len;
            while (target < end) {
                *target = color;
                ++target;
            }
#else
            sse_memfill(target, color, spans->len);
#endif
        }
#endif
        ++spans;
    }
}

static void blend_rgb16(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        //src is known to be 32 bpp

        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
    
        ushort *target = ((ushort *)t) + spans->x;
        uint *image_bits = (uint *)ibits;
        // #### take care of non integer dx/dy
        int x = qRound(dx);
        int y = qRound(dy);
        //     qDebug("x=%f,y=%f %d/%d image_height=%d", dx, dy, x, y, image_height);
        if (y < 0 || y >= image_height)
            return;


        const uint *src = image_bits + y*image_width + x;
        const ushort *end = target + spans->len;
        if (x < 0) {
            src -= x;
            target -= x;
            x = 0;
        }
        if (end - target > image_width)
            end = target + image_width;

        if (data->compositionMode == QPainter::CompositionMode_Source && spans->coverage == 255) {
            while (target < end) {
                *target++ = qt_convRgbTo16(*src++);
            }
        } else {
            while (target < end) {
                *target = qt_blend_pixel_rgb16(*target, *src, spans->coverage);
                ++target;
                ++src;
            }
        }

        ++spans;
    }
}

static void blend_tiled_rgb16(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
    
        ushort *target = ((ushort *)t) + spans->x;
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
        for (int i = x; i < x + spans->len; ++i) {
            *target = qt_blend_pixel_rgb16(*target, src[i%image_width], spans->coverage);
            ++target;
        }

        ++spans;
    }
}


static inline QRgb qt_conv_4ToRgb(uchar g)
{
    g = g | g << 4;
    return qRgb(g, g, g);
}

static inline uchar qt_conv_RgbTo4(QRgb c)
{
    return qGray(c) >> 4;
}

static void blend_color_gray4_lsb(int y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(y);

    while (count--) {
        Q_ASSERT(spans->coverage == 0xff);
        uint color = data->solid.color;

        int x0 = spans->x;
        int x1 = spans->x + spans->len;

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

        ++spans;
    }
    //### no alpha blending implemented
}


static void blend_tiled_gray4_lsb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        Q_ASSERT(spans->coverage == 0xff);

        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;
    
        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
    
        uint *image_bits = (uint *)ibits;
        // #### take care of non integer dx/dy
        int x = qRound(dx);
        int y = qRound(dy);
        if (y < 0 || y >= image_height)
            return;

        const uint *src = image_bits + y*image_width;

        int x0 = spans->x;
        int x1 = spans->x + spans->len;

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
        ++spans;
    }
}

static void blend_gray4_lsb(int _y, int count, QT_FT_Span *spans, void *userData)
{
    QSpanFillData *data = reinterpret_cast<QSpanFillData *>(userData);
    void *t = data->rasterBuffer->scanLine(_y);

    while (count--) {
        Q_ASSERT(spans->coverage == 0xff);
    
        int image_width = data->texture.width;
        int image_height = data->texture.height;
        int xoff = qRound(data->dx) % image_width;
        int yoff = qRound(data->dy) % image_height;

        if (xoff < 0)
            xoff += image_width;
        if (yoff < 0)
            yoff += image_height;

        const void *ibits = data->texture.imageData;
        const qreal dx = (xoff + spans->x)%image_width;
        const qreal dy = (_y + yoff) % image_height;
    
        uint *image_bits = (uint *)ibits;
        // #### take care of non integer dx/dy
        int x = qRound(dx);
        int y = qRound(dy);
        if (y < 0 || y >= image_height)
            return;

        const uint *src = image_bits + y*image_width;

        int x0 = spans->x;
        int x1 = spans->x + spans->len;

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
        ++spans;
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
    // ### get rid of me!
    { // Layout_RGB32
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



#ifdef QT_HAVE_SSE

enum CPUFeatures {
    None = 0,
    MMX = 0x1,
    SSE = 0x2,
    SSE2 = 0x4,
    CMOV = 0x8
};

static uint detectCPUFeatures() {
#ifdef __x86_64__
    return MMX|SSE|SSE2|CMOV;
#else
    uint result;
    /* see p. 118 of amd64 instruction set manual Vol3 */
    asm ("push %%ebx\n"
         "pushf\n"
         "pop %%eax\n"
         "mov %%eax, %%ebx\n"
         "xor $0x00200000, %%eax\n"
         "push %%eax\n"
         "popf\n"
         "pushf\n"
         "pop %%eax\n"
         "mov $0x0, %%edx\n"
         "xor %%ebx, %%eax\n"
         "jz 1f\n"

         "mov $0x00000001, %%eax\n"
         "cpuid\n"
         "1:\n"
         "pop %%ebx\n"
         "mov %%edx, %0\n"
        : "=r" (result)
        :
        : "%eax", "%ecx", "%edx"
        );

    uint features = 0;
    // result now contains the standard feature bits
    if (result & (1 << 15))
        features |= CMOV;
    if (result & (1 << 23))
        features |= MMX;
    if (result & (1 << 25))
        features |= SSE;
    if (result & (1 << 26))
        features |= SSE2;
    return features;
#endif
}


void qInitDrawhelperAsm()
{
    static uint features = 0;
    if (features)
        return;
    features = detectCPUFeatures();

    if (features & SSE) {
        qDebug("using mmx code");
        functionForMode = qt_functionForMode_SSE;
        functionForModeSolid = qt_functionForModeSolid_SSE;
    }
}

#else

void qInitDrawhelperAsm() {}

#endif // Q_HAVE_SSE

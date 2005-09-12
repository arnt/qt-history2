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
#include <mmintrin.h>
#include <xmmintrin.h>

const quint64 mmx_0x0080 = 0x0080008000800080ULL;
const quint64 mmx_0x00ff = 0x00ff00ff00ff00ffULL;


static inline __m64 spread_alpha(__m64 x)
{
    return _mm_shuffle_pi16 (x, _MM_SHUFFLE(3, 3, 3, 3));
}

static inline __m64 negate(__m64 x)
{
    return _mm_xor_si64(x, (__m64)mmx_0x00ff);
}

static inline __m64 add(__m64 a, __m64 b)
{
    return  _mm_adds_pu16 (a, b);
}

static inline __m64 byte_mul(__m64 a, __m64 b)
{
    __m64 res = _mm_mullo_pi16(a, b);
    res = add(res, (__m64)mmx_0x0080);
    res = add(res, _mm_srli_pi16 (res, 8));
    return _mm_srli_pi16(res, 8);
}


static inline __m64 interpolate_pixel_256(__m64 x, __m64 a, __m64 y, __m64 b) {
    __m64 res = add(_mm_mullo_pi16(x, a), _mm_mullo_pi16(y, b));
    return _mm_srli_pi16(res, 8);
}

static inline __m64 interpolate_pixel_255(__m64 x, __m64 a, __m64 y, __m64 b) {
    __m64 res = add(_mm_mullo_pi16(x, a), _mm_mullo_pi16(y, b));
    res = add(res, (__m64)mmx_0x0080);
    res = add(res, _mm_srli_pi16 (res, 8));
    return _mm_srli_pi16(res, 8);
}

static inline __m64 premul(__m64 x) {
    __m64 a = spread_alpha(x);
    return byte_mul(x, a);
}

static inline __m64 load(uint x)
{
    return _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), _mm_setzero_si64());
}

static inline __m64 loadAlpha(uint x)
{
    __m64 t = _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), _mm_setzero_si64());
    return _mm_shuffle_pi16 (t, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline uint store(__m64 x)
{
    return _mm_cvtsi64_si32(_mm_packs_pu16(x, _mm_setzero_si64()));
}


// solid composition methods

static void QT_FASTCALL comp_func_solid_Clear(uint *dest, int length, const uint, uint const_alpha)
{
    if (!length)
        return;

    if (const_alpha == 255) {
        if (((long)dest) & 0x7) {
            *dest = 0;
            ++dest;
            --length;
        }
        int l = length/2;
        __m64 zero = _mm_setzero_si64();
        while (l) {
            _mm_stream_pi((__m64 *)dest, zero);
            --l;
            dest += 2;
        }
        if (length & 1)
            *dest = 0;
    } else {
        __m64 ia = negate(loadAlpha(const_alpha));
        for (int i = 0; i < length; ++i) {
            dest[i] = store(byte_mul(load(dest[i]), ia));
        }
    }
    _mm_empty();
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
        __m64 a = loadAlpha(const_alpha);
        __m64 ia = negate(a);
        __m64 c = byte_mul(load(color), a);
        for (int i = 0; i < length; ++i) {
            dest[i] = store(add(c, byte_mul(load(dest[i]), ia)));
        }
        _mm_empty();
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
    if (qAlpha(color) == 255) {
        qt_memfill_uint(dest, length, color);
    } else {
        __m64 c = load(color);
        __m64 ia = negate(spread_alpha(c));
        for (int i = 0; i < length; ++i) {
            dest[i] = store(add(c, byte_mul(load(dest[i]), ia)));
        }
        _mm_empty();
    }
}

static void QT_FASTCALL comp_func_solid_DestinationOver(uint *dest, int length, uint color, uint const_alpha)
{
    __m64 c = load(color);
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            __m64 d = load(dest[i]);
            __m64 ia = negate(spread_alpha(d));
            dest[i] = store(add(d, byte_mul(c, ia)));
        }
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint tmp = dest[i] + BYTE_MUL(color, 255 - qAlpha(dest[i]));
            dest[i] = INTERPOLATE_PIXEL_255(tmp, const_alpha, dest[i], ialpha);
        }
    }
    _mm_empty();
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


const CompositionFunctionSolid qt_functionForModeSolid_SSE[] = {
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


// regular composition methods

static void QT_FASTCALL comp_func_Clear(uint *dest, const uint *, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        qt_memfill_uint(dest, length, 0);
    } else {
        __m64 ialpha = negate(spread_alpha(load(const_alpha)));
        for (int i = 0; i < length; ++i)
            dest[i] = store(byte_mul(load(dest[i]), ialpha));
    }
    _mm_empty();
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
        __m64 a = loadAlpha(const_alpha);
        __m64 ia = negate(a);
        for (int i = 0; i < length; ++i)
            dest[i] = store(interpolate_pixel_255(load(src[i]), a, load(dest[i]), ia));
    }
    _mm_empty();
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
        for (int i = 0; i < length; ++i) {
            __m64 s = load(src[i]);
            __m64 ia = negate(spread_alpha(s));
            dest[i] = store(add(s, byte_mul(load(dest[i]), ia)));
        }
    } else {
        __m64 ca = loadAlpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            __m64 s = byte_mul(load(src[i]), ca);
            __m64 ia = negate(spread_alpha(s));
            dest[i] = store(add(s, byte_mul(load(dest[i]), ia)));
        }
    }
    _mm_empty();
}

static void QT_FASTCALL comp_func_DestinationOver(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            __m64 s = load(dest[i]);
            __m64 ia = negate(spread_alpha(s));
            dest[i] = store(add(s, byte_mul(load(src[i]), ia)));
        }
    } else {
        __m64 ca = loadAlpha(const_alpha);
        __m64 ica = negate(ca);
        for (int i = 0; i < length; ++i) {
            __m64 d = load(dest[i]);
            __m64 ia = negate(spread_alpha(d));
            __m64 t = add(d, byte_mul(load(src[i]), ia));
            dest[i] = store(interpolate_pixel_255(t, ca, d, ica));
        }
    }
    _mm_empty();
}

/*
  Dca' = Sca.Da
  Da'  = Sa.Da
*/
static void QT_FASTCALL comp_func_SourceIn(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            __m64 a = spread_alpha(load(dest[i]));
            dest[i] = store(byte_mul(load(src[i]), a));
        }
        _mm_empty();
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

const CompositionFunction qt_functionForMode_SSE[] = {
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



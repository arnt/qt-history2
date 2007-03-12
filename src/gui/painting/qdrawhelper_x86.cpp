/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qdrawhelper_p.h>
#include <private/qdrawhelper_x86_p.h>
#include <private/qpaintengine_raster_p.h>

#ifdef QT_HAVE_SSE2
#  include <emmintrin.h>
#endif
#ifdef QT_HAVE_SSE
#  include <xmmintrin.h>
#endif
#ifdef QT_HAVE_3DNOW
#  include <mm3dnow.h>
#endif
#if defined(QT_HAVE_MMX) || defined(QT_HAVE_IWMMXT)
#  include <mmintrin.h>
#endif

#ifdef QT_HAVE_SSE2

void qt_memfill32_sse2(quint32 *dest, quint32 value, int count)
{
    if (count < 7) {
        switch (count) {
        case 6: *dest++ = value;
        case 5: *dest++ = value;
        case 4: *dest++ = value;
        case 3: *dest++ = value;
        case 2: *dest++ = value;
        case 1: *dest   = value;
        }
        return;
    };

    const int align = (long)(dest) & 0xf;
    switch (align) {
    case 4:  *dest++ = value; --count;
    case 8:  *dest++ = value; --count;
    case 12: *dest++ = value; --count;
    }

    int count128 = count / 4;
    __m128i *dst128 = reinterpret_cast<__m128i*>(dest);
    const __m128i value128 = _mm_set_epi32(value, value, value, value);

    int n = (count128 + 3) / 4;
    switch (count128 & 0x3) {
    case 0: do { _mm_store_si128(dst128++, value128);
    case 3:      _mm_store_si128(dst128++, value128);
    case 2:      _mm_store_si128(dst128++, value128);
    case 1:      _mm_store_si128(dst128++, value128);
    } while (--n > 0);
    }

    const int rest = count & 0x3;
    if (rest) {
        switch (rest) {
        case 3: dest[count - 3] = value;
        case 2: dest[count - 2] = value;
        case 1: dest[count - 1] = value;
        }
    }
}

void qt_memfill16_sse2(quint16 *dest, quint16 value, int count)
{
    if (count < 3) {
        switch (count) {
        case 2: *dest++ = value;
        case 1: *dest = value;
        }
        return;
    }

    const int align = (long)(dest) & 0x3;
    switch (align) {
    case 2: *dest++ = value; --count;
    }

    const quint32 value32 = (value << 16) | value;
    qt_memfill32_sse2(reinterpret_cast<quint32*>(dest), value32, count / 2);

    if (count & 0x1)
        dest[count - 1] = value;
}

void qt_bitmapblit32_sse2(QRasterBuffer *rasterBuffer, int x, int y,
                          quint32 color,
                          const uchar *src, int width, int height, int stride)
{
    quint32 *dest = reinterpret_cast<quint32*>(rasterBuffer->scanLine(y)) + x;
    const int destStride = rasterBuffer->bytesPerLine() / sizeof(quint32);

    const __m128i c128 = _mm_set1_epi32(color);
    const __m128i maskmask1 = _mm_set_epi32(0x10101010, 0x20202020,
                                            0x40404040, 0x80808080);
    const __m128i maskadd1 = _mm_set_epi32(0x70707070, 0x60606060,
                                           0x40404040, 0x00000000);

    if (width > 4) {
        const __m128i maskmask2 = _mm_set_epi32(0x01010101, 0x02020202,
                                                0x04040404, 0x08080808);
        const __m128i maskadd2 = _mm_set_epi32(0x7f7f7f7f, 0x7e7e7e7e,
                                               0x7c7c7c7c, 0x78787878);
        while (height--) {
            for (int x = 0; x < width; x += 8) {
                const quint8 s = src[x >> 3];
                if (!s)
                    continue;
                __m128i mask1 = _mm_set1_epi8(s);
                __m128i mask2 = mask1;

                mask1 = _mm_and_si128(mask1, maskmask1);
                mask1 = _mm_add_epi8(mask1, maskadd1);
                _mm_maskmoveu_si128(c128, mask1, (char*)(dest + x));
                mask2 = _mm_and_si128(mask2, maskmask2);
                mask2 = _mm_add_epi8(mask2, maskadd2);
                _mm_maskmoveu_si128(c128, mask2, (char*)(dest + x + 4));
            }
            dest += destStride;
            src += stride;
        }
    } else {
        while (height--) {
            const quint8 s = *src;
            if (s) {
                __m128i mask1 = _mm_set1_epi8(s);
                mask1 = _mm_and_si128(mask1, maskmask1);
                mask1 = _mm_add_epi8(mask1, maskadd1);
                _mm_maskmoveu_si128(c128, mask1, (char*)(dest));
            }
            dest += destStride;
            src += stride;
        }
    }
}

void qt_bitmapblit16_sse2(QRasterBuffer *rasterBuffer, int x, int y,
                          quint32 color,
                          const uchar *src, int width, int height, int stride)
{
    const quint16 c = qt_colorConvert<quint16, quint32>(color);
    quint16 *dest = reinterpret_cast<quint16*>(rasterBuffer->scanLine(y)) + x;
    const int destStride = rasterBuffer->bytesPerLine() / sizeof(quint16);

    const __m128i c128 = _mm_set1_epi16(c);
    const __m128i maskmask = _mm_set_epi16(0x0101, 0x0202, 0x0404, 0x0808,
                                           0x1010, 0x2020, 0x4040, 0x8080);
    const __m128i maskadd = _mm_set_epi16(0x7f7f, 0x7e7e, 0x7c7c, 0x7878,
                                          0x7070, 0x6060, 0x4040, 0x0000);

    while (height--) {
        for (int x = 0; x < width; x += 8) {
            const quint8 s = src[x >> 3];
            if (!s)
                continue;
            __m128i mask = _mm_set1_epi8(s);
            mask = _mm_and_si128(mask, maskmask);
            mask = _mm_add_epi8(mask, maskadd);
            _mm_maskmoveu_si128(c128, mask, (char*)(dest + x));
        }
        dest += destStride;
        src += stride;
    }
}

#endif // SSE2

#ifdef QT_HAVE_SSE

template <class MM>
inline void qt_memfill32_sse_template(quint32 *dest, quint32 value, int count)
{
    if (count < 7) {
        switch (count) {
        case 6: *dest++ = value;
        case 5: *dest++ = value;
        case 4: *dest++ = value;
        case 3: *dest++ = value;
        case 2: *dest++ = value;
        case 1: *dest   = value;
        }
        return;
    };

    __m64 *dst64 = reinterpret_cast<__m64*>(dest);
    const __m64 value64 = _mm_set_pi32(value, value);
    int count64 = count / 2;

    int n = (count64 + 3) / 4;
    switch (count64 & 0x3) {
    case 0: do { _mm_stream_pi(dst64++, value64);
    case 3:      _mm_stream_pi(dst64++, value64);
    case 2:      _mm_stream_pi(dst64++, value64);
    case 1:      _mm_stream_pi(dst64++, value64);
    } while (--n > 0);
    }

    if (count & 0x1)
        dest[count - 1] = value;

    MM::end();
}

template <class MM>
inline void qt_bitmapblit16_sse_template(QRasterBuffer *rasterBuffer,
                                         int x, int y,
                                         quint32 color,
                                         const uchar *src,
                                         int width, int height, int stride)
{
    const quint16 c = qt_colorConvert<quint16, quint32>(color);
    quint16 *dest = reinterpret_cast<quint16*>(rasterBuffer->scanLine(y)) + x;
    const int destStride = rasterBuffer->bytesPerLine() / sizeof(quint16);

    const __m64 c64 = _mm_set1_pi16(c);
    const __m64 maskmask1 = _mm_set_pi16(0x1010, 0x2020, 0x4040, 0x8080);
    const __m64 maskadd1 = _mm_set_pi16(0x7070, 0x6060, 0x4040, 0x0000);

    if (width > 4) {
        const __m64 maskmask2 = _mm_set_pi16(0x0101, 0x0202, 0x0404, 0x0808);
        const __m64 maskadd2 = _mm_set_pi16(0x7f7f, 0x7e7e, 0x7c7c, 0x7878);

        while (height--) {
            for (int x = 0; x < width; x += 8) {
                const quint8 s = src[x >> 3];
                if (!s)
                    continue;
                __m64 mask1 = _mm_set1_pi8(s);
                __m64 mask2 = mask1;
                mask1 = _m_pand(mask1, maskmask1);
                mask1 = _mm_add_pi16(mask1, maskadd1);
                _mm_maskmove_si64(c64, mask1, (char*)(dest + x));
                mask2 = _m_pand(mask2, maskmask2);
                mask2 = _mm_add_pi16(mask2, maskadd2);
                _mm_maskmove_si64(c64, mask2, (char*)(dest + x + 4));
            }
            dest += destStride;
            src += stride;
        }
    } else {
        while (height--) {
            const quint8 s = *src;
            if (s) {
                __m64 mask1 = _mm_set1_pi8(s);
                mask1 = _m_pand(mask1, maskmask1);
                mask1 = _mm_add_pi16(mask1, maskadd1);
                _mm_maskmove_si64(c64, mask1, (char*)(dest));
            }
            dest += destStride;
            src += stride;
        }
    }

    MM::end();
}

#endif // SSE

#if ((defined(QT_HAVE_MMX) || defined(QT_HAVE_SSE)) && (!defined(__APPLE__) || defined(__i386__))) || defined(QT_HAVE_IWMMXT)

#if defined(Q_OS_WIN)
#  pragma warning(disable: 4799) // No EMMS at end of function
#endif

typedef __m64 m64;

/*
  result = 0
  d = d * cia
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_Clear(uint *dest, int length, uint, uint const_alpha)
{
    if (!length)
        return;

    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, 0);
    } else {
        m64 ia = MM::negate(MM::load_alpha(const_alpha));
        for (int i = 0; i < length; ++i) {
            dest[i] = MM::store(MM::byte_mul(MM::load(dest[i]), ia));
        }
    }
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_Clear(uint *dest, const uint *, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, 0);
    } else {
        m64 ia = MM::negate(MM::load_alpha(const_alpha));
        for (int i = 0; i < length; ++i)
            dest[i] = MM::store(MM::byte_mul(MM::load(dest[i]), ia));
    }
    MM::end();
}

/*
  result = s
  dest = s * ca + d * cia
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_Source(uint *dest, int length, uint src, uint const_alpha)
{
    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, src);
    } else {
        const m64 a = MM::load_alpha(const_alpha);
        const m64 ia = MM::negate(a);
        const m64 s = MM::byte_mul(MM::load(src), a);
        for (int i = 0; i < length; ++i) {
            dest[i] = MM::store(MM::add(s, MM::byte_mul(MM::load(dest[i]), ia)));
        }
        MM::end();
    }
}

template <class MM>
static void QT_FASTCALL comp_func_Source(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        ::memcpy(dest, src, length * sizeof(uint));
    } else {
        const m64 a = MM::load_alpha(const_alpha);
        const m64 ia = MM::negate(a);
        for (int i = 0; i < length; ++i)
            dest[i] = MM::store(MM::interpolate_pixel_255(MM::load(src[i]), a,
                                                        MM::load(dest[i]), ia));
    }
    MM::end();
}

/*
  result = s + d * sia
  dest = (s + d * sia) * ca + d * cia
       = s * ca + d * (sia * ca + cia)
       = s * ca + d * (1 - sa*ca)
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_SourceOver(uint *dest, int length, uint src, uint const_alpha)
{
    if ((const_alpha & qAlpha(src)) == 255) {
        QT_MEMFILL_UINT(dest, length, src);
    } else {
        m64 s = MM::load(src);
        if (const_alpha != 255) {
            m64 ca = MM::load_alpha(const_alpha);
            s = MM::byte_mul(s, ca);
        }
        m64 a = MM::negate(MM::alpha(s));
        for (int i = 0; i < length; ++i)
            dest[i] = MM::store(MM::add(s, MM::byte_mul(MM::load(dest[i]), a)));
        MM::end();
    }
}

template <class MM>
static void QT_FASTCALL comp_func_SourceOver(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = MM::load(src[i]);
            m64 ia = MM::negate(MM::alpha(s));
            dest[i] = MM::store(MM::add(s, MM::byte_mul(MM::load(dest[i]), ia)));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = MM::byte_mul(MM::load(src[i]), ca);
            m64 ia = MM::negate(MM::alpha(s));
            dest[i] = MM::store(MM::add(s, MM::byte_mul(MM::load(dest[i]), ia)));
        }
    }
    MM::end();
}

/*
  result = d + s * dia
  dest = (d + s * dia) * ca + d * cia
       = d + s * dia * ca
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_DestinationOver(uint *dest, int length, uint src, uint const_alpha)
{
    m64 s = MM::load(src);
    if (const_alpha != 255)
        s = MM::byte_mul(s, MM::load_alpha(const_alpha));

    for (int i = 0; i < length; ++i) {
        m64 d = MM::load(dest[i]);
        m64 dia = MM::negate(MM::alpha(d));
        dest[i] = MM::store(MM::add(d, MM::byte_mul(s, dia)));
    }
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_DestinationOver(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            m64 ia = MM::negate(MM::alpha(d));
            dest[i] = MM::store(MM::add(d, MM::byte_mul(MM::load(src[i]), ia)));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            m64 dia = MM::negate(MM::alpha(d));
            dia = MM::byte_mul(dia, ca);
            dest[i] = MM::store(MM::add(d, MM::byte_mul(MM::load(src[i]), dia)));
        }
    }
    MM::end();
}

/*
  result = s * da
  dest = s * da * ca + d * cia
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_SourceIn(uint *dest, int length, uint src, uint const_alpha)
{
    if (const_alpha == 255) {
        m64 s = MM::load(src);
        for (int i = 0; i < length; ++i) {
            m64 da = MM::alpha(MM::load(dest[i]));
            dest[i] = MM::store(MM::byte_mul(s, da));
        }
    } else {
        m64 s = MM::load(src);
        m64 ca = MM::load_alpha(const_alpha);
        s = MM::byte_mul(s, ca);
        m64 cia = MM::negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::alpha(d), d, cia));
        }
    }
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_SourceIn(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 a = MM::alpha(MM::load(dest[i]));
            dest[i] = MM::store(MM::byte_mul(MM::load(src[i]), a));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        m64 cia = MM::negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            m64 da = MM::byte_mul(MM::alpha(d), ca);
            dest[i] = MM::store(MM::interpolate_pixel_255(
                                   MM::load(src[i]), da, d, cia));
        }
    }
    MM::end();
}

/*
  result = d * sa
  dest = d * sa * ca + d * cia
       = d * (sa * ca + cia)
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_DestinationIn(uint *dest, int length, uint src, uint const_alpha)
{
    m64 a = MM::alpha(MM::load(src));
    if (const_alpha != 255) {
        m64 ca = MM::load_alpha(const_alpha);
        m64 cia = MM::negate(ca);
        a = MM::byte_mul(a, ca);
        a = MM::add(a, cia);
    }
    for (int i = 0; i < length; ++i)
        dest[i] = MM::store(MM::byte_mul(MM::load(dest[i]), a));
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_DestinationIn(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 a = MM::alpha(MM::load(src[i]));
            dest[i] = MM::store(MM::byte_mul(MM::load(dest[i]), a));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        m64 cia = MM::negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            m64 a = MM::alpha(MM::load(src[i]));
            a = MM::byte_mul(a, ca);
            a = MM::add(a, cia);
            dest[i] = MM::store(MM::byte_mul(d, a));
        }
    }
    MM::end();
}

/*
  result = s * dia
  dest = s * dia * ca + d * cia
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_SourceOut(uint *dest, int length, uint src, uint const_alpha)
{
    m64 s = MM::load(src);
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 dia = MM::negate(MM::alpha(MM::load(dest[i])));
            dest[i] = MM::store(MM::byte_mul(s, dia));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        m64 cia = MM::negate(ca);
        s = MM::byte_mul(s, ca);
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::negate(MM::alpha(d)), d, cia));
        }
    }
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_SourceOut(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 ia = MM::negate(MM::alpha(MM::load(dest[i])));
            dest[i] = MM::store(MM::byte_mul(MM::load(src[i]), ia));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        m64 cia = MM::negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            m64 dia = MM::byte_mul(MM::negate(MM::alpha(d)), ca);
            dest[i] = MM::store(MM::interpolate_pixel_255(MM::load(src[i]), dia, d, cia));
        }
    }
    MM::end();
}

/*
  result = d * sia
  dest = d * sia * ca + d * cia
       = d * (sia * ca + cia)
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_DestinationOut(uint *dest, int length, uint src, uint const_alpha)
{
    m64 a = MM::negate(MM::alpha(MM::load(src)));
    if (const_alpha != 255) {
        m64 ca = MM::load_alpha(const_alpha);
        a = MM::byte_mul(a, ca);
        a = MM::add(a, MM::negate(ca));
    }
    for (int i = 0; i < length; ++i)
        dest[i] = MM::store(MM::byte_mul(MM::load(dest[i]), a));
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_DestinationOut(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 a = MM::negate(MM::alpha(MM::load(src[i])));
            dest[i] = MM::store(MM::byte_mul(MM::load(dest[i]), a));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        m64 cia = MM::negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = MM::load(dest[i]);
            m64 a = MM::negate(MM::alpha(MM::load(src[i])));
            a = MM::byte_mul(a, ca);
            a = MM::add(a, cia);
            dest[i] = MM::store(MM::byte_mul(d, a));
        }
    }
    MM::end();
}

/*
  result = s*da + d*sia
  dest = s*da*ca + d*sia*ca + d *cia
       = s*ca * da + d * (sia*ca + cia)
       = s*ca * da + d * (1 - sa*ca)
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_SourceAtop(uint *dest, int length, uint src, uint const_alpha)
{
    m64 s = MM::load(src);
    if (const_alpha != 255) {
        m64 ca = MM::load_alpha(const_alpha);
        s = MM::byte_mul(s, ca);
    }
    m64 a = MM::negate(MM::alpha(s));
    for (int i = 0; i < length; ++i) {
        m64 d = MM::load(dest[i]);
        dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::alpha(d), d, a));
    }
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_SourceAtop(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = MM::load(src[i]);
            m64 d = MM::load(dest[i]);
            dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::alpha(d), d,
                                                        MM::negate(MM::alpha(s))));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = MM::load(src[i]);
            s = MM::byte_mul(s, ca);
            m64 d = MM::load(dest[i]);
            dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::alpha(d), d,
                                                        MM::negate(MM::alpha(s))));
        }
    }
    MM::end();
}

/*
  result = d*sa + s*dia
  dest = d*sa*ca + s*dia*ca + d *cia
       = s*ca * dia + d * (sa*ca + cia)
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_DestinationAtop(uint *dest, int length, uint src, uint const_alpha)
{
    m64 s = MM::load(src);
    m64 a = MM::alpha(s);
    if (const_alpha != 255) {
        m64 ca = MM::load_alpha(const_alpha);
        s = MM::byte_mul(s, ca);
        a = MM::alpha(s);
        a = MM::add(a, MM::negate(ca));
    }
    for (int i = 0; i < length; ++i) {
        m64 d = MM::load(dest[i]);
        dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::negate(MM::alpha(d)), d, a));
    }
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_DestinationAtop(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = MM::load(src[i]);
            m64 d = MM::load(dest[i]);
            dest[i] = MM::store(MM::interpolate_pixel_255(d, MM::alpha(s), s,
                                                        MM::negate(MM::alpha(d))));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = MM::load(src[i]);
            s = MM::byte_mul(s, ca);
            m64 d = MM::load(dest[i]);
            m64 a = MM::alpha(s);
            a = MM::add(a, MM::negate(ca));
            dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::negate(MM::alpha(d)),
                                                        d, a));
        }
    }
    MM::end();
}

/*
  result = d*sia + s*dia
  dest = d*sia*ca + s*dia*ca + d *cia
       = s*ca * dia + d * (sia*ca + cia)
       = s*ca * dia + d * (1 - sa*ca)
*/
template <class MM>
static void QT_FASTCALL comp_func_solid_XOR(uint *dest, int length, uint src, uint const_alpha)
{
    m64 s = MM::load(src);
    if (const_alpha != 255) {
        m64 ca = MM::load_alpha(const_alpha);
        s = MM::byte_mul(s, ca);
    }
    m64 a = MM::negate(MM::alpha(s));
    for (int i = 0; i < length; ++i) {
        m64 d = MM::load(dest[i]);
        dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::negate(MM::alpha(d)),
                                                    d, a));
    }
    MM::end();
}

template <class MM>
static void QT_FASTCALL comp_func_XOR(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = MM::load(src[i]);
            m64 d = MM::load(dest[i]);
            dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::negate(MM::alpha(d)),
                                                        d, MM::negate(MM::alpha(s))));
        }
    } else {
        m64 ca = MM::load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = MM::load(src[i]);
            s = MM::byte_mul(s, ca);
            m64 d = MM::load(dest[i]);
            dest[i] = MM::store(MM::interpolate_pixel_255(s, MM::negate(MM::alpha(d)),
                                                        d, MM::negate(MM::alpha(s))));
        }
    }
    MM::end();
}

template <class MM>
static inline void qt_blend_color_argb_x86(int count, const QSpan *spans,
                                           void *userData,
                                           CompositionFunctionSolid *solidFunc)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->rasterBuffer->compositionMode == QPainter::CompositionMode_Source
        || (data->rasterBuffer->compositionMode == QPainter::CompositionMode_SourceOver
            && qAlpha(data->solid.color) == 255)) {
        // inline for performance
        while (count--) {
            uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
            if (spans->coverage == 255) {
                QT_MEMFILL_UINT(target, spans->len, data->solid.color);
            } else {
                // dest = s * ca + d * (1 - sa*ca) --> dest = s * ca + d * (1-ca)
                m64 ca = MM::load_alpha(spans->coverage);
                m64 s = MM::byte_mul(MM::load(data->solid.color), ca);
                m64 ica = MM::negate(ca);
                for (int i = 0; i < spans->len; ++i)
                    target[i] = MM::store(MM::add(s, MM::byte_mul(MM::load(target[i]), ica)));
            }
            ++spans;
        }
        MM::end();
        return;
    }
    CompositionFunctionSolid func = solidFunc[data->rasterBuffer->compositionMode];
    if (!func)
        return;

    while (count--) {
        uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
        func(target, spans->len, data->solid.color, spans->coverage);
        ++spans;
    }
}

#if defined(QT_HAVE_MMX) || defined(QT_HAVE_SSE)

struct MMX
{
    static inline m64 alpha(m64 x) {
        x = _mm_unpackhi_pi16(x, x);
        x = _mm_unpackhi_pi16(x, x);
        return x;
    }

    static inline m64 _negate(const m64 &x, const m64 &mmx_0x00ff) {
        return _mm_xor_si64(x, mmx_0x00ff);
    }

    static inline m64 negate(const m64 &x) {
        return _negate(x, _mm_set1_pi16(0xff));
    };

    static inline m64 add(const m64 &a, const m64 &b) {
        return  _mm_adds_pu16 (a, b);
    }

    static inline m64 _byte_mul(const m64 &a, const m64 &b,
                                const m64 &mmx_0x0080)
    {
        m64 res = _mm_mullo_pi16(a, b);
        res = _mm_adds_pu16(res, mmx_0x0080);
        res = _mm_adds_pu16(res, _mm_srli_pi16 (res, 8));
        return _mm_srli_pi16(res, 8);
    }

    static inline m64 byte_mul(const m64 &a, const m64 &b) {
        return _byte_mul(a, b, _mm_set1_pi16(0x80));
    }

    static inline m64 interpolate_pixel_256(const m64 &x, const m64 &a,
                                            const m64 &y, const m64 &b)
    {
        m64 res = _mm_adds_pu16(_mm_mullo_pi16(x, a), _mm_mullo_pi16(y, b));
        return _mm_srli_pi16(res, 8);
    }

    static inline m64 _interpolate_pixel_255(const m64 &x, const m64 &a,
                                             const m64 &y, const m64 &b,
                                             const m64 &mmx_0x0080)
    {
        m64 res = _mm_adds_pu16(_mm_mullo_pi16(x, a), _mm_mullo_pi16(y, b));
        res = _mm_adds_pu16(res, mmx_0x0080);
        res = _mm_adds_pu16(res, _mm_srli_pi16 (res, 8));
        return _mm_srli_pi16(res, 8);
    }

    static inline m64 interpolate_pixel_255(const m64 &x, const m64 &a,
                                            const m64 &y, const m64 &b)
    {
        return _interpolate_pixel_255(x, a, y, b, _mm_set1_pi16(0x80));
    }

    static inline m64 _premul(m64 x, const m64 &mmx_0x0080) {
        m64 a = alpha(x);
        return _byte_mul(x, a, mmx_0x0080);
    }

    static inline m64 premul(const m64 &x) {
        return _premul(x, _mm_set1_pi16(0x80));
    }

    static inline m64 _load(uint x, const m64 &mmx_0x0000) {
        return _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), mmx_0x0000);
    }

    static inline m64 load(uint x) {
        return _load(x, _mm_setzero_si64());
    }

    static inline m64 _load_alpha(uint x, const m64 &mmx_0x0000) {
        m64 t = _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), mmx_0x0000);
        t = _mm_unpacklo_pi16(t, t);
        t = _mm_unpacklo_pi16(t, t);
        return t;
    }

    static inline m64 load_alpha(uint x) {
        return _load_alpha(x, _mm_setzero_si64());
    }

    static inline uint _store(const m64 &x, const m64 &mmx_0x0000) {
        return _mm_cvtsi64_si32(_mm_packs_pu16(x, mmx_0x0000));
    }

    static inline uint store(const m64 &x) {
        return _store(x, _mm_setzero_si64());
    }

    static inline void end() {
        _mm_empty();
    }
};

#ifdef QT_HAVE_3DNOW
struct MMX3DNOW : public MMX
{
    static inline void end() {
        _m_femms();
    }
};
#endif // QT_HAVE_3DNOW

#endif // QT_HAVE_MMX


#ifdef QT_HAVE_SSE

#ifndef _MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) \
 (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | (fp0))

#endif

struct SSE : public MMX
{
    static inline m64 alpha(m64 x) {
        return _mm_shuffle_pi16 (x, _MM_SHUFFLE(3, 3, 3, 3));
    }

    static inline m64 _load_alpha(uint x, const m64 &mmx_0x0000) {
        m64 t = _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), mmx_0x0000);
        return _mm_shuffle_pi16 (t, _MM_SHUFFLE(0, 0, 0, 0));
    }

    static inline m64 load_alpha(uint x) {
        return _load_alpha(x, _mm_setzero_si64());
    }
};

#ifdef QT_HAVE_3DNOW
struct SSE3DNOW : public SSE
{
    static inline void end() {
        _m_femms();
    }
};
#endif // QT_HAVE_3DNOW
#endif // QT_HAVE_SSE

void QT_FASTCALL comp_func_solid_Plus(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Plus(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_Multiply(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Multiply(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_Screen(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Screen(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_Overlay(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Overlay(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_Darken(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Darken(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_Lighten(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Lighten(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_ColorDodge(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_ColorDodge(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_ColorBurn(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_ColorBurn(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_HardLight(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_HardLight(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_SoftLight(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_SoftLight(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_Difference(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Difference(uint *dest, const uint *src, int length, uint const_alpha);

void QT_FASTCALL comp_func_solid_Exclusion(uint *dest, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Exclusion(uint *dest, const uint *src, int length, uint const_alpha);


#ifdef QT_HAVE_MMX

const CompositionFunctionSolid qt_functionForModeSolid_MMX[] = {
    comp_func_solid_SourceOver<MMX>,
    comp_func_solid_DestinationOver<MMX>,
    comp_func_solid_Clear<MMX>,
    comp_func_solid_Source<MMX>,
    0,
    comp_func_solid_SourceIn<MMX>,
    comp_func_solid_DestinationIn<MMX>,
    comp_func_solid_SourceOut<MMX>,
    comp_func_solid_DestinationOut<MMX>,
    comp_func_solid_SourceAtop<MMX>,
    comp_func_solid_DestinationAtop<MMX>,
    comp_func_solid_XOR<MMX>,
    comp_func_solid_Plus,
    comp_func_solid_Multiply,
    comp_func_solid_Screen,
    comp_func_solid_Overlay,
    comp_func_solid_Darken,
    comp_func_solid_Lighten,
    comp_func_solid_ColorDodge,
    comp_func_solid_ColorBurn,
    comp_func_solid_HardLight,
    comp_func_solid_SoftLight,
    comp_func_solid_Difference,
    comp_func_solid_Exclusion
};

const CompositionFunction qt_functionForMode_MMX[] = {
    comp_func_SourceOver<MMX>,
    comp_func_DestinationOver<MMX>,
    comp_func_Clear<MMX>,
    comp_func_Source<MMX>,
    0,
    comp_func_SourceIn<MMX>,
    comp_func_DestinationIn<MMX>,
    comp_func_SourceOut<MMX>,
    comp_func_DestinationOut<MMX>,
    comp_func_SourceAtop<MMX>,
    comp_func_DestinationAtop<MMX>,
    comp_func_XOR<MMX>,
    comp_func_Plus,
    comp_func_Multiply,
    comp_func_Screen,
    comp_func_Overlay,
    comp_func_Darken,
    comp_func_Lighten,
    comp_func_ColorDodge,
    comp_func_ColorBurn,
    comp_func_HardLight,
    comp_func_SoftLight,
    comp_func_Difference,
    comp_func_Exclusion
};

void qt_blend_color_argb_mmx(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<MMX>(count, spans, userData,
                                 (CompositionFunctionSolid*)qt_functionForModeSolid_MMX);
}

#ifdef QT_HAVE_3DNOW

const CompositionFunctionSolid qt_functionForModeSolid_MMX3DNOW[] = {
    comp_func_solid_SourceOver<MMX3DNOW>,
    comp_func_solid_DestinationOver<MMX3DNOW>,
    comp_func_solid_Clear<MMX3DNOW>,
    comp_func_solid_Source<MMX3DNOW>,
    0,
    comp_func_solid_SourceIn<MMX3DNOW>,
    comp_func_solid_DestinationIn<MMX3DNOW>,
    comp_func_solid_SourceOut<MMX3DNOW>,
    comp_func_solid_DestinationOut<MMX3DNOW>,
    comp_func_solid_SourceAtop<MMX3DNOW>,
    comp_func_solid_DestinationAtop<MMX3DNOW>,
    comp_func_solid_XOR<MMX3DNOW>,
    comp_func_solid_Plus,
    comp_func_solid_Multiply,
    comp_func_solid_Screen,
    comp_func_solid_Overlay,
    comp_func_solid_Darken,
    comp_func_solid_Lighten,
    comp_func_solid_ColorDodge,
    comp_func_solid_ColorBurn,
    comp_func_solid_HardLight,
    comp_func_solid_SoftLight,
    comp_func_solid_Difference,
    comp_func_solid_Exclusion
};

const CompositionFunction qt_functionForMode_MMX3DNOW[] = {
    comp_func_SourceOver<MMX3DNOW>,
    comp_func_DestinationOver<MMX3DNOW>,
    comp_func_Clear<MMX3DNOW>,
    comp_func_Source<MMX3DNOW>,
    0,
    comp_func_SourceIn<MMX3DNOW>,
    comp_func_DestinationIn<MMX3DNOW>,
    comp_func_SourceOut<MMX3DNOW>,
    comp_func_DestinationOut<MMX3DNOW>,
    comp_func_SourceAtop<MMX3DNOW>,
    comp_func_DestinationAtop<MMX3DNOW>,
    comp_func_XOR<MMX3DNOW>,
    comp_func_Plus,
    comp_func_Multiply,
    comp_func_Screen,
    comp_func_Overlay,
    comp_func_Darken,
    comp_func_Lighten,
    comp_func_ColorDodge,
    comp_func_ColorBurn,
    comp_func_HardLight,
    comp_func_SoftLight,
    comp_func_Difference,
    comp_func_Exclusion
};

void qt_blend_color_argb_mmx3dnow(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<MMX3DNOW>(count, spans, userData,
                                      (CompositionFunctionSolid*)qt_functionForModeSolid_MMX3DNOW);
}

#endif // QT_HAVE_3DNOW
#endif // QT_HAVE_MMX

#ifdef QT_HAVE_SSE

const CompositionFunctionSolid qt_functionForModeSolid_SSE[] = {
    comp_func_solid_SourceOver<SSE>,
    comp_func_solid_DestinationOver<SSE>,
    comp_func_solid_Clear<SSE>,
    comp_func_solid_Source<SSE>,
    0,
    comp_func_solid_SourceIn<SSE>,
    comp_func_solid_DestinationIn<SSE>,
    comp_func_solid_SourceOut<SSE>,
    comp_func_solid_DestinationOut<SSE>,
    comp_func_solid_SourceAtop<SSE>,
    comp_func_solid_DestinationAtop<SSE>,
    comp_func_solid_XOR<SSE>,
    comp_func_solid_Plus,
    comp_func_solid_Multiply,
    comp_func_solid_Screen,
    comp_func_solid_Overlay,
    comp_func_solid_Darken,
    comp_func_solid_Lighten,
    comp_func_solid_ColorDodge,
    comp_func_solid_ColorBurn,
    comp_func_solid_HardLight,
    comp_func_solid_SoftLight,
    comp_func_solid_Difference,
    comp_func_solid_Exclusion
};

const CompositionFunction qt_functionForMode_SSE[] = {
    comp_func_SourceOver<SSE>,
    comp_func_DestinationOver<SSE>,
    comp_func_Clear<SSE>,
    comp_func_Source<SSE>,
    0,
    comp_func_SourceIn<SSE>,
    comp_func_DestinationIn<SSE>,
    comp_func_SourceOut<SSE>,
    comp_func_DestinationOut<SSE>,
    comp_func_SourceAtop<SSE>,
    comp_func_DestinationAtop<SSE>,
    comp_func_XOR<SSE>,
    comp_func_Plus,
    comp_func_Multiply,
    comp_func_Screen,
    comp_func_Overlay,
    comp_func_Darken,
    comp_func_Lighten,
    comp_func_ColorDodge,
    comp_func_ColorBurn,
    comp_func_HardLight,
    comp_func_SoftLight,
    comp_func_Difference,
    comp_func_Exclusion
};

void qt_blend_color_argb_sse(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<SSE>(count, spans, userData,
                                 (CompositionFunctionSolid*)qt_functionForModeSolid_SSE);
}

void qt_memfill32_sse(quint32 *dest, quint32 value, int count)
{
    return qt_memfill32_sse_template<SSE>(dest, value, count);
}

void qt_bitmapblit16_sse(QRasterBuffer *rasterBuffer, int x, int y,
                         quint32 color,
                         const uchar *src,
                         int width, int height, int stride)
{
    return qt_bitmapblit16_sse_template<SSE>(rasterBuffer, x,y, color,
                                             src, width, height, stride);
}

#ifdef QT_HAVE_3DNOW
const CompositionFunctionSolid qt_functionForModeSolid_SSE3DNOW[] = {
    comp_func_solid_SourceOver<SSE3DNOW>,
    comp_func_solid_DestinationOver<SSE3DNOW>,
    comp_func_solid_Clear<SSE3DNOW>,
    comp_func_solid_Source<SSE3DNOW>,
    0,
    comp_func_solid_SourceIn<SSE3DNOW>,
    comp_func_solid_DestinationIn<SSE3DNOW>,
    comp_func_solid_SourceOut<SSE3DNOW>,
    comp_func_solid_DestinationOut<SSE3DNOW>,
    comp_func_solid_SourceAtop<SSE3DNOW>,
    comp_func_solid_DestinationAtop<SSE3DNOW>,
    comp_func_solid_XOR<SSE3DNOW>,
    comp_func_solid_Plus,
    comp_func_solid_Multiply,
    comp_func_solid_Screen,
    comp_func_solid_Overlay,
    comp_func_solid_Darken,
    comp_func_solid_Lighten,
    comp_func_solid_ColorDodge,
    comp_func_solid_ColorBurn,
    comp_func_solid_HardLight,
    comp_func_solid_SoftLight,
    comp_func_solid_Difference,
    comp_func_solid_Exclusion
};

const CompositionFunction qt_functionForMode_SSE3DNOW[] = {
    comp_func_SourceOver<SSE3DNOW>,
    comp_func_DestinationOver<SSE3DNOW>,
    comp_func_Clear<SSE3DNOW>,
    comp_func_Source<SSE3DNOW>,
    0,
    comp_func_SourceIn<SSE3DNOW>,
    comp_func_DestinationIn<SSE3DNOW>,
    comp_func_SourceOut<SSE3DNOW>,
    comp_func_DestinationOut<SSE3DNOW>,
    comp_func_SourceAtop<SSE3DNOW>,
    comp_func_DestinationAtop<SSE3DNOW>,
    comp_func_XOR<SSE3DNOW>,
    comp_func_Plus,
    comp_func_Multiply,
    comp_func_Screen,
    comp_func_Overlay,
    comp_func_Darken,
    comp_func_Lighten,
    comp_func_ColorDodge,
    comp_func_ColorBurn,
    comp_func_HardLight,
    comp_func_SoftLight,
    comp_func_Difference,
    comp_func_Exclusion
};

void qt_blend_color_argb_sse3dnow(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<SSE3DNOW>(count, spans, userData,
                                      (CompositionFunctionSolid*)qt_functionForModeSolid_SSE3DNOW);
}

void qt_memfill32_sse3dnow(quint32 *dest, quint32 value, int count)
{
    return qt_memfill32_sse_template<SSE3DNOW>(dest, value, count);
}


void qt_bitmapblit16_sse3dnow(QRasterBuffer *rasterBuffer, int x, int y,
                              quint32 color,
                              const uchar *src,
                              int width, int height, int stride)
{
    return qt_bitmapblit16_sse_template<SSE3DNOW>(rasterBuffer, x,y, color,
                                                  src, width, height, stride);
}

#endif // QT_HAVE_3DNOW
#endif // QT_HAVE_SSE

#endif // QT_HAVE_MMX || QT_HAVE_SSE

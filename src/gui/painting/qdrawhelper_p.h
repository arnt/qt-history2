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

#ifndef QDRAWHELPER_P_H
#define QDRAWHELPER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qglobal.h"
#include "QtGui/qcolor.h"
#include "QtGui/qpainter.h"
#include "QtGui/qimage.h"
#ifndef QT_FT_BEGIN_HEADER
#define QT_FT_BEGIN_HEADER
#define QT_FT_END_HEADER
#endif
#include "private/qrasterdefs_p.h"

#ifdef Q_WS_QWS
#include "QtGui/qscreen_qws.h"
#endif

#define QT_ROTATION_CACHEDREAD 1
#define QT_ROTATION_CACHEDWRITE 2
#define QT_ROTATION_PACKING 3
#define QT_ROTATION_TILED 4

#ifndef QT_ROTATION_ALGORITHM
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#define QT_ROTATION_ALGORITHM QT_ROTATION_TILED
#else
#define QT_ROTATION_ALGORITHM QT_ROTATION_CACHEDREAD
#endif
#endif

/*******************************************************************************
 * QSpan
 *
 * duplicate definition of FT_Span
 */
typedef QT_FT_Span QSpan;

struct SolidData;
struct TextureData;
struct GradientData;
struct LinearGradientData;
struct RadialGradientData;
struct ConicalGradientData;
struct QSpanData;
class QGradient;
class QRasterBuffer;

typedef QT_FT_SpanFunc ProcessSpans;
typedef void (*BitmapBlitFunc)(QRasterBuffer *rasterBuffer,
                               int x, int y, quint32 color,
                               const uchar *bitmap,
                               int mapWidth, int mapHeight, int mapStride);
typedef void (*RectFillFunc)(QRasterBuffer *rasterBuffer,
                             int x, int y, int width, int height,
                             quint32 color);

struct DrawHelper {
    ProcessSpans blendColor;
    ProcessSpans blendGradient;
    BitmapBlitFunc bitmapBlit;
    RectFillFunc fillRect;
};

extern DrawHelper qDrawHelper[QImage::NImageFormats];
void qBlendTexture(int count, const QSpan *spans, void *userData);
#ifdef Q_WS_QWS
extern DrawHelper qDrawHelperCallback[QImage::NImageFormats];
void qBlendTextureCallback(int count, const QSpan *spans, void *userData);
#endif

typedef void QT_FASTCALL (*CompositionFunction)(uint *dest, const uint *src, int length, uint const_alpha);
typedef void QT_FASTCALL (*CompositionFunctionSolid)(uint *dest, int length, uint color, uint const_alpha);

void qInitDrawhelperAsm();

class QRasterPaintEngine;

struct SolidData
{
    uint color;
};

struct LinearGradientData
{
    struct {
        qreal x;
        qreal y;
    } origin;
    struct {
        qreal x;
        qreal y;
    } end;
};

struct RadialGradientData
{
    struct {
        qreal x;
        qreal y;
    } center;
    struct {
        qreal x;
        qreal y;
    } focal;
    qreal radius;
};

struct ConicalGradientData
{
    struct {
        qreal x;
        qreal y;
    } center;
    qreal angle;
};

struct GradientData
{
    QGradient::Spread spread;

    union {
        LinearGradientData linear;
        RadialGradientData radial;
        ConicalGradientData conical;
    };

#ifdef Q_WS_QWS
#define GRADIENT_STOPTABLE_SIZE 256
#else
#define GRADIENT_STOPTABLE_SIZE 1024
#endif

    uint* colorTable; //[GRADIENT_STOPTABLE_SIZE];

    uint alphaColor : 1;
    uint needsResolving : 1;
};

struct TextureData
{
    const uchar *imageData;
    const uchar *scanLine(int y) const { return imageData + y*bytesPerLine; }
    int width;
    int height;
    int bytesPerLine;
    QImage::Format format;
    const QVector<QRgb> *colorTable;
    bool hasAlpha;
    enum Type {
        Plain,
        Tiled
    };
    Type type;
    int const_alpha;
};

struct QSpanData
{
    QRasterBuffer *rasterBuffer;
#ifdef Q_WS_QWS
    QRasterPaintEngine *rasterEngine;
#endif
    ProcessSpans blend;
    ProcessSpans unclipped_blend;
    BitmapBlitFunc bitmapBlit;
    RectFillFunc fillRect;
    qreal m11, m12, m13, m21, m22, m23, dx, dy;   // inverse xform matrix
    enum Type {
        None,
        Solid,
        LinearGradient,
        RadialGradient,
        ConicalGradient,
        Texture
    } type : 8;
    int txop : 8;
    bool bilinear;
    QImage tempImage;
    union {
        SolidData solid;
        GradientData gradient;
        TextureData texture;
    };
    void init(QRasterBuffer *rb, QRasterPaintEngine *pe = 0);
    void setup(const QBrush &brush, int alpha);
    void setupMatrix(const QTransform &matrix, int txop, int bilinear);
    void initTexture(const QImage *image, int alpha, TextureData::Type = TextureData::Plain);
    void adjustSpanMethods();
};

template <class DST, class SRC>
static inline DST qt_colorConvert(SRC color)
{
    return color;
}

#if defined(QT_QWS_DEPTH_16) && defined(QT_QWS_DEPTH_32)
template <>
static inline quint32 qt_colorConvert(quint16 color)
{
    return qt_conv16ToRgb(color);
}
#endif

#ifdef QT_QWS_DEPTH_16
template <>
static inline quint16 qt_colorConvert(quint32 color)
{
    return qt_convRgbTo16(color);
}
#endif

#ifdef QT_QWS_DEPTH_8
template <>
static inline quint8 qt_colorConvert(quint32 color)
{
    uchar r = (qRed(color) + 0x19) / 0x33;
    uchar g = (qGreen(color) + 0x19) / 0x33;
    uchar b = (qBlue(color) + 0x19) / 0x33;

    return r*6*6 + g*6 + b;
}

template <>
static inline quint8 qt_colorConvert(quint16 color)
{
    return qt_colorConvert<quint32, quint8>(qt_conv16ToRgb(color));
}
#endif // QT_QWS_DEPTH_8

#ifdef QT_QWS_DEPTH_24

// hw: endianess??
class quint24
{
public:
    inline quint24(quint32 v)
    {
        data[0] = qBlue(v);
        data[1] = qGreen(v);
        data[2] = qRed(v);
    }

private:
    uchar data[3];
} Q_PACKED;

template <>
static inline quint24 qt_colorConvert(quint32 color)
{
    return quint24(color);
}

#endif // QT_QWS_DEPTH_24

#ifdef QT_QWS_DEPTH_18

// hw: endianess??
class quint18
{
public:
    inline quint18(quint32 v)
    {
        uchar b = qBlue(v);
        uchar g = qGreen(v);
        uchar r = qRed(v);
        uint p = (b >> 2) | ((g >> 2) << 6) | ((r >> 2) << 12);
        data[0] = qBlue(p);
        data[1] = qGreen(p);
        data[2] = qRed(p);
    }

private:
    uchar data[3];
} Q_PACKED;

template <>
static inline quint18 qt_colorConvert(quint32 color)
{
    return quint18(color);
}

#endif // QT_QWS_DEPTH_18

template <class T>
void qt_memfill(T *dest, T value, int count);

template<> inline void qt_memfill(quint32 *dest, quint32 color, int count)
{
    extern void (*qt_memfill32)(quint32 *dest, quint32 value, int count);
    qt_memfill32(dest, color, count);
}

template<> inline void qt_memfill(quint16 *dest, quint16 color, int count)
{
    extern void (*qt_memfill16)(quint16 *dest, quint16 value, int count);
    qt_memfill16(dest, color, count);
}

template <class T>
inline void qt_memfill(T *dest, T value, int count)
{
    int n = (count + 7) / 8;
    switch (count & 0x07)
    {
    case 0: do { *dest++ = value;
    case 7:      *dest++ = value;
    case 6:      *dest++ = value;
    case 5:      *dest++ = value;
    case 4:      *dest++ = value;
    case 3:      *dest++ = value;
    case 2:      *dest++ = value;
    case 1:      *dest++ = value;
    } while (--n > 0);
    }
}

template <class T>
static void qt_rectfill(T *dest, T value,
                        int x, int y, int width, int height, int stride)
{
    stride /= sizeof(T);
    dest += y * stride + x;
    for (int j = 0; j < height; ++j) {
        qt_memfill(dest, value, width);
        dest += stride;
    }
}

template <class DST, class SRC>
static inline void qt_memconvert(DST *dest, const SRC *src, int count)
{
    /* Duff's device */
    int n = (count + 7) / 8;
    switch (count & 0x07)
    {
    case 0: do { *dest++ = qt_colorConvert<DST, SRC>(*src++);
    case 7:      *dest++ = qt_colorConvert<DST, SRC>(*src++);
    case 6:      *dest++ = qt_colorConvert<DST, SRC>(*src++);
    case 5:      *dest++ = qt_colorConvert<DST, SRC>(*src++);
    case 4:      *dest++ = qt_colorConvert<DST, SRC>(*src++);
    case 3:      *dest++ = qt_colorConvert<DST, SRC>(*src++);
    case 2:      *dest++ = qt_colorConvert<DST, SRC>(*src++);
    case 1:      *dest++ = qt_colorConvert<DST, SRC>(*src++);
    } while (--n > 0);
    }
}

#define QT_MEMFILL_UINT(dest, length, color)            \
    qt_memfill<quint32>(dest, color, length);

#define QT_MEMFILL_USHORT(dest, length, color) \
    qt_memfill<quint16>(dest, color, length);

#define QT_MEMCPY_REV_UINT(dest, src, length) \
do {                                          \
    /* Duff's device */                       \
    uint *_d = (uint*)(dest) + length;         \
    const uint *_s = (uint*)(src) + length;    \
    register int n = ((length) + 7) / 8;      \
    switch ((length) & 0x07)                  \
    {                                         \
    case 0: do { *--_d = *--_s;                 \
    case 7:      *--_d = *--_s;                 \
    case 6:      *--_d = *--_s;                 \
    case 5:      *--_d = *--_s;                 \
    case 4:      *--_d = *--_s;                 \
    case 3:      *--_d = *--_s;                 \
    case 2:      *--_d = *--_s;                 \
    case 1:      *--_d = *--_s;                 \
    } while (--n > 0);                        \
    }                                         \
} while (0)

#define QT_MEMCPY_USHORT(dest, src, length) \
do {                                          \
    /* Duff's device */                       \
    ushort *_d = (ushort*)(dest);         \
    const ushort *_s = (ushort*)(src);    \
    register int n = ((length) + 7) / 8;      \
    switch ((length) & 0x07)                  \
    {                                         \
    case 0: do { *_d++ = *_s++;                 \
    case 7:      *_d++ = *_s++;                 \
    case 6:      *_d++ = *_s++;                 \
    case 5:      *_d++ = *_s++;                 \
    case 4:      *_d++ = *_s++;                 \
    case 3:      *_d++ = *_s++;                 \
    case 2:      *_d++ = *_s++;                 \
    case 1:      *_d++ = *_s++;                 \
    } while (--n > 0);                        \
    }                                         \
} while (0)

#ifdef Q_WS_QWS
template <class DST, class SRC>
void qt_memrotate90(const SRC *src, int srcWidth, int srcHeight, int srcStride,
                    DST *dst, int dstStride);
template <class DST, class SRC>
void qt_memrotate180(const SRC *src, int srcWidth, int srcHeight, int srcStride,
                     DST *dst, int dstStride);
template <class DST, class SRC>
void qt_memrotate270(const SRC *src, int srcWidth, int srcHeight, int srcStride,
                     DST *dst, int dstStride);
#endif // Q_WS_QWS

static inline int qt_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }

inline ushort qConvertRgb32To16(uint c)
{
   return (((c) >> 3) & 0x001f)
       | (((c) >> 5) & 0x07e0)
       | (((c) >> 8) & 0xf800);
}

inline QRgb qConvertRgb16To32(uint c)
{
    return 0xff000000
        | ((((c) << 3) & 0xf8) | (((c) >> 2) & 0x7))
        | ((((c) << 5) & 0xfc00) | (((c) >> 1) & 0x300))
        | ((((c) << 8) & 0xf80000) | (((c) << 3) & 0x70000));
}

#if 1
static inline uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t >>= 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x &= 0xff00ff00;
    x |= t;
    return x;
}

static inline uint INTERPOLATE_PIXEL_255(uint x, uint a, uint y, uint b) {
    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
    x &= 0xff00ff00;
    x |= t;
    return x;
}

static inline uint BYTE_MUL(uint x, uint a) {
    uint t = (x & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a;
    x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
    x &= 0xff00ff00;
    x |= t;
    return x;
}

static inline uint PREMUL(uint x) {
    uint a = x >> 24;
    uint t = (x & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff) * a;
    x = (x + ((x >> 8) & 0xff) + 0x80);
    x &= 0xff00;
    x |= t | (a << 24);
    return x;
}
#else
// possible implementation for 64 bit
static inline uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    ulong t = (((ulong(x)) | ((ulong(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t += (((ulong(y)) | ((ulong(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
    t >>= 8;
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

static inline uint INTERPOLATE_PIXEL_255(uint x, uint a, uint y, uint b) {
    ulong t = (((ulong(x)) | ((ulong(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t += (((ulong(y)) | ((ulong(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
    t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080);
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

static inline uint BYTE_MUL(uint x, uint a) {
    ulong t = (((ulong(x)) | ((ulong(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080);
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

static inline uint PREMUL(uint x) {
    uint a = x >> 24;
    ulong t = (((ulong(x)) | ((ulong(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080);
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24)) | 0xff000000;
}
#endif

#define INV_PREMUL(p)                                   \
    (qAlpha(p) == 0 ? 0 :                               \
    ((qAlpha(p) << 24)                                  \
     | (((255*qRed(p))/ qAlpha(p)) << 16)               \
     | (((255*qGreen(p)) / qAlpha(p))  << 8)            \
     | ((255*qBlue(p)) / qAlpha(p))))


const uint qt_bayer_matrix[16][16] = {
    { 0x1, 0xc0, 0x30, 0xf0, 0xc, 0xcc, 0x3c, 0xfc,
      0x3, 0xc3, 0x33, 0xf3, 0xf, 0xcf, 0x3f, 0xff},
    { 0x80, 0x40, 0xb0, 0x70, 0x8c, 0x4c, 0xbc, 0x7c,
      0x83, 0x43, 0xb3, 0x73, 0x8f, 0x4f, 0xbf, 0x7f},
    { 0x20, 0xe0, 0x10, 0xd0, 0x2c, 0xec, 0x1c, 0xdc,
      0x23, 0xe3, 0x13, 0xd3, 0x2f, 0xef, 0x1f, 0xdf},
    { 0xa0, 0x60, 0x90, 0x50, 0xac, 0x6c, 0x9c, 0x5c,
      0xa3, 0x63, 0x93, 0x53, 0xaf, 0x6f, 0x9f, 0x5f},
    { 0x8, 0xc8, 0x38, 0xf8, 0x4, 0xc4, 0x34, 0xf4,
      0xb, 0xcb, 0x3b, 0xfb, 0x7, 0xc7, 0x37, 0xf7},
    { 0x88, 0x48, 0xb8, 0x78, 0x84, 0x44, 0xb4, 0x74,
      0x8b, 0x4b, 0xbb, 0x7b, 0x87, 0x47, 0xb7, 0x77},
    { 0x28, 0xe8, 0x18, 0xd8, 0x24, 0xe4, 0x14, 0xd4,
      0x2b, 0xeb, 0x1b, 0xdb, 0x27, 0xe7, 0x17, 0xd7},
    { 0xa8, 0x68, 0x98, 0x58, 0xa4, 0x64, 0x94, 0x54,
      0xab, 0x6b, 0x9b, 0x5b, 0xa7, 0x67, 0x97, 0x57},
    { 0x2, 0xc2, 0x32, 0xf2, 0xe, 0xce, 0x3e, 0xfe,
      0x1, 0xc1, 0x31, 0xf1, 0xd, 0xcd, 0x3d, 0xfd},
    { 0x82, 0x42, 0xb2, 0x72, 0x8e, 0x4e, 0xbe, 0x7e,
      0x81, 0x41, 0xb1, 0x71, 0x8d, 0x4d, 0xbd, 0x7d},
    { 0x22, 0xe2, 0x12, 0xd2, 0x2e, 0xee, 0x1e, 0xde,
      0x21, 0xe1, 0x11, 0xd1, 0x2d, 0xed, 0x1d, 0xdd},
    { 0xa2, 0x62, 0x92, 0x52, 0xae, 0x6e, 0x9e, 0x5e,
      0xa1, 0x61, 0x91, 0x51, 0xad, 0x6d, 0x9d, 0x5d},
    { 0xa, 0xca, 0x3a, 0xfa, 0x6, 0xc6, 0x36, 0xf6,
      0x9, 0xc9, 0x39, 0xf9, 0x5, 0xc5, 0x35, 0xf5},
    { 0x8a, 0x4a, 0xba, 0x7a, 0x86, 0x46, 0xb6, 0x76,
      0x89, 0x49, 0xb9, 0x79, 0x85, 0x45, 0xb5, 0x75},
    { 0x2a, 0xea, 0x1a, 0xda, 0x26, 0xe6, 0x16, 0xd6,
      0x29, 0xe9, 0x19, 0xd9, 0x25, 0xe5, 0x15, 0xd5},
    { 0xaa, 0x6a, 0x9a, 0x5a, 0xa6, 0x66, 0x96, 0x56,
      0xa9, 0x69, 0x99, 0x59, 0xa5, 0x65, 0x95, 0x55}
};

#define ARGB_COMBINE_ALPHA(argb, alpha) \
    ((((argb >> 24) * alpha) >> 8) << 24) | (argb & 0x00ffffff)

#endif // QDRAWHELPER_P_H

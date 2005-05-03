#ifndef QDRAWHELPER_H
#define QDRAWHELPER_H

#include <qglobal.h>
#include <qcolor.h>
#include <qpainter.h>

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

struct GradientData;
struct LinearGradientData;
struct ConicalGradientData;
extern uint qt_gradient_pixel(const GradientData *data, double pos);


typedef void (*BlendColor)(void *target, const QSpan *span, uint color,
                           QPainter::CompositionMode mode);

typedef void (*Blend)(void *target, const QSpan *span,
                      const qreal dx, const qreal dy,
                      const void *image_bits, const int image_width, const int image_height,
                      QPainter::CompositionMode mode);

typedef void (*BlendTransformed)(void *target, const QSpan *span,
                                 const qreal ix, const qreal iy,
                                 const qreal dx, const qreal dy,
                                 const void *image_bits,
                                 const int image_width, const int image_height,
                                 QPainter::CompositionMode mode);

typedef void (*BlendLinearGradient)(void *target,
                                    const QSpan *span,
                                    LinearGradientData *data,
                                    qreal ybase,
                                    QPainter::CompositionMode mode);

typedef void (*BlendConicalGradient)(void *target,
                                     const QSpan *span,
                                     ConicalGradientData *data,
                                     int y,
                                     QPainter::CompositionMode mode);

struct DrawHelper {
    enum Layout {
        Layout_ARGB,
        Layout_RGB32,
        Layout_Count
    };
    BlendColor blendColor;
    Blend blend;
    Blend blendTiled;
    BlendTransformed blendTransformed;
    BlendTransformed blendTransformedTiled;
    BlendTransformed blendTransformedBilinear;
    BlendTransformed blendTransformedBilinearTiled;
    BlendLinearGradient blendLinearGradient;
    BlendConicalGradient blendConicalGradient;
};

extern DrawHelper qDrawHelper[DrawHelper::Layout_Count];

void qInitDrawhelperAsm();

class QRasterBuffer;

struct GradientData
{
    QRasterBuffer *rasterBuffer;
    QGradient::Spread spread;

    int stopCount;
    qreal *stopPoints;
    uint *stopColors;

#define GRADIENT_STOPTABLE_SIZE 1024
    uint colorTable[GRADIENT_STOPTABLE_SIZE];

    uint alphaColor : 1;

    void initColorTable();
};

struct LinearGradientData : public GradientData
{
    QPointF origin;
    QPointF end;

    void init();

    qreal xincr;
    qreal yincr;
    BlendLinearGradient blendFunc;

    QPainter::CompositionMode compositionMode;
    QMatrix brushMatrix;
};

struct RadialGradientData : public GradientData
{
    QPointF center;
    qreal radius;
    QPointF focal;
};

struct ConicalGradientData : public GradientData
{
    QPointF center;
    qreal angle;
    void init(const QPointF &center, qreal angle, const QMatrix &matrix);
    BlendConicalGradient blendFunc;
    QPainter::CompositionMode compositionMode;
};


inline int qt_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }


#if 1
inline uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t >>= 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x &= 0xff00ff00;
    x |= t;
    return x;
}

inline uint INTERPOLATE_PIXEL_255(uint x, uint a, uint y, uint b) {
    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
    x &= 0xff00ff00;
    x |= t;
    return x;
}

inline uint BYTE_MUL(uint x, uint a) {
    uint t = (x & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a;
    x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
    x &= 0xff00ff00;
    x |= t;
    return x;
}

inline uint PREMUL(uint x) {
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
inline uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    ulong t = (((ulong(x)) | ((ulong(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t += (((ulong(y)) | ((ulong(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
    t >>= 8;
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

inline uint INTERPOLATE_PIXEL_255(uint x, uint a, uint y, uint b) {
    ulong t = (((ulong(x)) | ((ulong(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t += (((ulong(y)) | ((ulong(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
    t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080);
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

inline uint BYTE_MUL(uint x, uint a) {
    ulong t = (((ulong(x)) | ((ulong(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080);
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

inline uint PREMUL(uint x) {
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

#endif

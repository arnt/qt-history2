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
extern uint qt_gradient_pixel(const GradientData *data, double pos);


typedef void (*BlendColor)(void *target, const QSpan *span, uint color, QPainter::CompositionMode mode);
typedef void (*Blend)(void *target, const QSpan *span,
                      const qreal dx, const qreal dy,
                      const void *image_bits, const int image_width, const int image_height, QPainter::CompositionMode mode);
typedef void (*BlendTransformed)(void *target, const QSpan *span,
                                 const qreal ix, const qreal iy, const qreal dx, const qreal dy,
                                 const void *image_bits, const int image_width, const int image_height, QPainter::CompositionMode mode);
typedef void (*BlendLinearGradient)(void *target, const QSpan *span, LinearGradientData *data, qreal ybase, QPainter::CompositionMode mode);

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
};


inline int qt_div_255(int x) { return (x + (x>>8) + 0x1) >> 8; }
inline int qt_div_255x255(int x) { return (x + (x>>7) + (x>>14)) >> 16; }

#define INTERPOLATE_PIXEL(p1, x1, p2,  x2)                            \
    (((qAlpha(p1) * x1 + qAlpha(p2) * x2) >> 8 << 24)           \
     | ((qRed(p1) * x1 + qRed(p2) * x2) >> 8 << 16)             \
     | ((qGreen(p1) * x1 + qGreen(p2) * x2) >> 8 << 8)          \
     | (qBlue(p1) * x1 + qBlue(p2) * x2) >> 8)

#define PREMUL(p)                                          \
    ((qAlpha(p) << 24)                                     \
     | (qt_div_255(qRed(p) * qAlpha(p)) << 16)             \
     | (qt_div_255(qGreen(p) * qAlpha(p))  << 8)           \
     | qt_div_255(qBlue(p) * qAlpha(p)))

#define INV_PREMUL(p)                                          \
    qAlpha(p) == 0 ? 0 :                                       \
    ((qAlpha(p) << 24)                                     \
     | (((255*qRed(p))/ qAlpha(p)) << 16)             \
     | (((255*qGreen(p)) / qAlpha(p))  << 8)           \
     | ((255*qBlue(p)) / qAlpha(p)))


#endif

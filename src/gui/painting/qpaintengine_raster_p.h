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
**/

#ifndef QPAINTENGINE_RASTER_P_H
#define QPAINTENGINE_RASTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qpaintengine.h>
#include <QtGui/qpainterpath.h>

#include <private/qpaintengine_p.h>
#include <private/qdatabuffer_p.h>
#include <private/qdrawhelper_p.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

class QFTOutlineMapper;
class QRasterPaintEnginePrivate;
class QRasterBuffer;

struct FillData;
struct SolidFillData;
struct TextureFillData;
struct LinearGradientData;
struct RadialGradientData;
struct ConicalGradientData;

/*******************************************************************************
 * QRasterPaintEngine
 */
class QRasterPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QRasterPaintEngine)
public:
    QRasterPaintEngine();
    ~QRasterPaintEngine();
    bool begin(QPaintDevice *device);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &origin);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);
    void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);

    void drawPath(const QPainterPath &path);

    void fillPath(const QPainterPath &path, FillData *fillData);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                    Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &sr,
                         Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    void drawTextItem(const QPointF &p, const QTextItem &ti);

    void drawRect(const QRectF &rect);
    void drawLine(const QLineF &line);

    void setFlushOnEnd(bool flush);
    void flush(QPaintDevice *device);

#ifdef Q_WS_WIN
    HDC getDC() const;
    void releaseDC(HDC hdc) const;
#endif

    Type type() const { return Raster; }

    QPoint coordinateOffset() const;
};


/*******************************************************************************
 * QRasterPaintEnginePrivate
 */
class QRasterPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QRasterPaintEngine)
public:

    enum RasterOperation {
        SourceCopy,
        SourceOverComposite
    };

    void fillForBrush(const QBrush &brush, FillData *data, const QPainterPath *path);
    void updateClip_helper(const QPainterPath &path, Qt::ClipOperation);

    QImage *colorizeBitmap(const QImage *image);

    qreal *gradientStopPoints(const QGradient *gradient);
    ARGB *gradientStopColors(const QGradient *gradient);

    ARGB mapColor(const QColor &color) const;

    QBrush brush;
    QBrush bgBrush;
    QPen pen;
    QMatrix matrix;
    QMatrix brushMatrix;
    QMatrix penMatrix;

    QPaintDevice *device;
    QFTOutlineMapper *outlineMapper;
    QRasterBuffer *rasterBuffer;
    QRasterBuffer *fontRasterBuffer;

    QDataBuffer<qreal> stopPoints;
    QDataBuffer<ARGB> stopColors;

    QPainterPath baseClip;
    QRect deviceRect;

    FillData *fillData;
    SolidFillData *solidFillData;
    TextureFillData *textureFillData;
    LinearGradientData *linearGradientData;
    RadialGradientData *radialGradientData;
    ConicalGradientData *conicalGradientData;

    QImage tempImage;

    RasterOperation rasterOperation;

    uint txop;

    uint opaqueBackground : 1;
    uint clipEnabled : 1;
    uint antialiased : 1;
    uint flushOnEnd : 1;
};

/*******************************************************************************
 * QRasterBuffer
 */
class QRasterBuffer
{
public:
#ifdef Q_WS_WIN
    QRasterBuffer() : m_hdc(0), m_bitmap(0), m_buffer(0), m_width(0), m_height(0) { init(); }

    HDC hdc() const { return m_hdc; }
#endif

#ifdef Q_WS_X11
    QRasterBuffer() : m_ximg(0), m_width(0), m_height(0), m_buffer(0) { init(); }
    XImage *m_ximg;
#endif

    void init();

    void prepare(QImage *image);
    void prepare(int w, int h);
    void prepareBuffer(int w, int h);
    void prepareClip(int w, int h);

    void resetBuffer(int val=0);
    void resetClip();

    void resizeClipSpan(int y, int size);
    void appendClipSpan(int x, int y, int len, int coverage);
    void replaceClipSpans(int y, QSpan *spans, int spanCount);
    void resetClipSpans(int y, int count);

    ARGB *scanLine(int y) { Q_ASSERT(y>=0); Q_ASSERT(y<m_height); return m_buffer + y * m_width; }

#ifndef QT_NO_DEBUG
    QImage clipImage() const;
#endif

    QSpan *clipSpans(int y) const { Q_ASSERT(y >= 0 && y <= m_height); return m_clipSpans[y]; }
    int clipSpanCount(int y) const { Q_ASSERT(y >= 0 && y <= m_height); return m_clipSpanCount[y]; }

    int width() const { return m_width; }
    int height() const { return m_height; }

    ARGB *buffer() const { return m_buffer; }

private:
#ifdef Q_WS_WIN
    HDC m_hdc;
    HBITMAP m_bitmap;
#endif

    int m_width;
    int m_height;
    ARGB *m_buffer;

    int *m_clipSpanCount;
    int *m_clipSpanCapacity;
    QSpan **m_clipSpans;
};

#endif // QPAINTENGINE_RASTER_P_H

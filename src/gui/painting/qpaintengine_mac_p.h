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

#ifndef QPAINTENGINE_MAC_P_H
#define QPAINTENGINE_MAC_P_H

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

#include "qpaintengine.h"
#ifdef Q_WS_MAC //just for now (to get the coregraphics switch) ###
#  include <private/qt_mac_p.h>
#endif
#include <private/qpaintengine_p.h>
#include <private/qpolygonclipper_p.h>

/*****************************************************************************
  QuickDraw Private data
 *****************************************************************************/
struct qt_float_point
{
    float x, y;
};

class QQuickDrawPaintEnginePrivate;
class QCoreGraphicsPaintEnginePrivate;
class QQuickDrawPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QQuickDrawPaintEngine)

public:
    QQuickDrawPaintEngine();
    ~QQuickDrawPaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);

    void drawLines(const QLineF *lines, int lineCount);
    void drawRects(const QRectF *rects, int rectCount);
    void drawPoints(const QPointF *p, int pointCount);
    void drawEllipse(const QRectF &r);

    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);

    Type type() const { return QPaintEngine::QuickDraw; }
    static void initialize();
    static void cleanup();

    void setupQDPort(bool force=false, QPoint *off=NULL, QRegion *rgn=NULL);

protected:
    QQuickDrawPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures devcaps=0);
    void setClippedRegionInternal(QRegion *);

    void setupQDFont();
    void setupQDBrush();
    void setupQDPen();

    friend class QFontEngineMac;

private:
    Q_DISABLE_COPY(QQuickDrawPaintEngine)
};

class QCoreGraphicsPaintEnginePrivate;
class QCoreGraphicsPaintEngine : public QQuickDrawPaintEngine
{
    Q_DECLARE_PRIVATE(QCoreGraphicsPaintEngine)

public:
    QCoreGraphicsPaintEngine();
    ~QCoreGraphicsPaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawLines(const QLineF *lines, int lineCount);
    void drawRects(const QRectF *rects, int rectCount);
    void drawPoints(const QPointF *p, int pointCount);
    void drawEllipse(const QRectF &r);

    void drawPath(const QPainterPath &path);

    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);

    Type type() const { return QPaintEngine::CoreGraphics; }

    CGContextRef handle() const;

    static void initialize();
    static void cleanup();

    QPainter::RenderHints supportedRenderHints() const;

protected:
    friend class QMacPrintEngine;
    friend class QMacPrintEnginePrivate;
    QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr);

private:
    Q_DISABLE_COPY(QCoreGraphicsPaintEngine)
};

class QQuickDrawPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QQuickDrawPaintEngine)
public:
    QQuickDrawPaintEnginePrivate()
        : QPaintEnginePrivate()
    {
        saved = 0;
        clip.serial = 0;
        clip.dirty = locked = unclipped = false;
        clip.pdev = clip.paintable = QRegion();
        brush_style_pix = 0;
        offx = offy = 0;
    }

    struct {
        QPen pen;
        QBrush brush;
        QRegion clip;
        struct {
            QPointF origin;
            Qt::BGMode mode;
            QBrush brush;
        } bg;
    } current;

    int offx, offy;
    QPixmap *brush_style_pix;
    uint unclipped : 1, locked : 1, has_clipping : 1;
    QMacSavedPortInfo *saved;
    QPolygonClipper<qt_float_point, qt_float_point, float> polygonClipper;

    struct {
        QRegion pdev, paintable;
        uint dirty : 1, serial : 15;
    } clip;
};

/*****************************************************************************
  Private data
 *****************************************************************************/
class QCoreGraphicsPaintEnginePrivate : public QQuickDrawPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QQuickDrawPaintEngine)
public:
    QCoreGraphicsPaintEnginePrivate()
    {
        hd = 0;
        shading = 0;
    }

    //state info (shared with QD)
    CGAffineTransform orig_xform;

    //cg structures
    CGContextRef hd;
    CGShadingRef shading;

    //internal functions
    enum { CGStroke=0x01, CGEOFill=0x02, CGFill=0x04 };
    void drawPath(uchar ops, CGMutablePathRef path = 0);
    void setClip(const QRegion *rgn=0);
    float penOffset();
    inline void setTransform(const QMatrix *matrix=0)
    {
        CGContextConcatCTM(hd, CGAffineTransformInvert(CGContextGetCTM(hd)));
        CGAffineTransform xform = orig_xform;
        if(matrix)
            xform = CGAffineTransformConcat(CGAffineTransformMake(matrix->m11(), matrix->m12(),
                                                                  matrix->m21(), matrix->m22(),
                                                                  matrix->dx(),  matrix->dy()),
                                            xform);
        CGContextConcatCTM(hd, xform);
        CGContextSetTextMatrix(hd, xform);
    }
};

#endif // QPAINTENGINE_MAC_P_H

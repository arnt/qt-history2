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

#ifndef QPAINTENGINE_MAC_H
#define QPAINTENGINE_MAC_H
#include "qpaintengine.h"
#ifdef Q_WS_MAC //just for now (to get the coregraphics switch) ###
#  include <private/qt_mac_p.h>
#endif

class QQuickDrawPaintEnginePrivate;
class QCoreGraphicsPaintEnginePrivate;
class QPainterState;

class QQuickDrawPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QQuickDrawPaintEngine)

public:
    QQuickDrawPaintEngine();
    ~QQuickDrawPaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPoint &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);

    void drawLine(const QPoint &pt1, const QPoint &pt2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawEllipse(const QRect &r);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolygon(const QPointArray &pa, PolygonDrawMode mode);

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, Qt::PixmapDrawingMode);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, Qt::PixmapDrawingMode);

    Type type() const { return QPaintEngine::QuickDraw; }
    static void initialize();
    static void cleanup();

protected:
    QQuickDrawPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures devcaps=0);
    void drawPolyInternal(const QPointArray &a, bool close=true);
    void setClippedRegionInternal(QRegion *);

    void setupQDFont();
    void setupQDBrush();
    void setupQDPen();
    void setupQDPort(bool force=false, QPoint *off=NULL, QRegion *rgn=NULL);

    friend void qt_mac_set_port(const QPainter *p);
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

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPoint &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawLine(const QPoint &pt1, const QPoint &pt2);
    void drawPath(const QPainterPath &path);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &pt);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawEllipse(const QRect &r);
    void drawPolygon(const QPointArray &pa, PolygonDrawMode mode);
    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, Qt::PixmapDrawingMode mode);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, Qt::PixmapDrawingMode mode);

    Type type() const { return QPaintEngine::CoreGraphics; }

    CGContextRef handle() const;

    static void initialize();
    static void cleanup();

    QPainter::RenderHints supportedRenderHints() const;

protected:
    friend class QMacPrintEngine;
    friend class QMacPrintEnginePrivate;
    QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr);
    void drawPolyInternal(const QPointArray &a, bool close=true);

private:
    Q_DISABLE_COPY(QCoreGraphicsPaintEngine)
};

#endif

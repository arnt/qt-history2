/****************************************************************************
**
** Definition of QQuickDrawPaintEngine/QCoreGraphicsPaintEngine class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAINTENGINE_MAC_H
#define QPAINTENGINE_MAC_H
#include "qpaintengine.h"
#include "qshared.h"
#ifdef Q_WS_MAC //just for now (to get the coregraphics switch) ###
# include "qt_mac.h"
#endif

class QQuickDrawPaintEnginePrivate;
class QPainterState;

class QQuickDrawPaintEngine : public QPaintEngine
{
public:
    QQuickDrawPaintEngine(const QPaintDevice *);
    ~QQuickDrawPaintEngine();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped = FALSE);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

    void setRasterOp(RasterOp r);

    void drawLine(const QPoint &pt1, const QPoint &pt2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &penColor);
    void drawRoundRect(const QRect &r, int xRnd, int yRnd);
    void drawEllipse(const QRect &r);
    void drawArc(const QRect &r, int a, int alen);
    void drawPie(const QRect &r, int a, int alen);
    void drawChord(const QRect &r, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr);
    void drawTextItem(const QPoint &pt, const QTextItem &ti, int textflags);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

    virtual Qt::HANDLE handle() const;

    inline Type type() const { return QPaintEngine::QuickDraw; }
    static void initialize();
    static void cleanup();

protected:
    void drawPolyInternal(const QPointArray &a, bool close=true, bool inset=true);
    void setClippedRegionInternal(QRegion *);

    void setupQDFont();
    void setupQDBrush();
    void setupQDPen();
    void setupQDPort(bool force=false, QPoint *off=NULL, QRegion *rgn=NULL);

    friend class QMacStyleQDPainter;
    friend class QFontEngineMac;

private:
    QQuickDrawPaintEnginePrivate *d;

private:
#if defined(Q_DISABLE_COPY)
    QQuickDrawPaintEngine(const QQuickDrawPaintEngine &);
    QQuickDrawPaintEngine &operator=(const QQuickDrawPaintEngine &);
#endif
};

class QCoreGraphicsPaintEnginePrivate;
class QCoreGraphicsPaintEngine : public QQuickDrawPaintEngine //for now we include QuickDraw to get things working, we *must* remove it later ### --Sam
{
public:
    QCoreGraphicsPaintEngine(const QPaintDevice *);
    ~QCoreGraphicsPaintEngine();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped = FALSE);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

    void setRasterOp(RasterOp r);

    void drawLine(const QPoint &pt1, const QPoint &pt2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &pt);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &penColor);
    void drawRoundRect(const QRect &r, int xRnd, int yRnd);
    void drawEllipse(const QRect &r);
    void drawArc(const QRect &r, int a, int alen);
    void drawPie(const QRect &r, int a, int alen);
    void drawChord(const QRect &r, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif
    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr);
    void drawTextItem(const QPoint &pt, const QTextItem &ti, int textflags);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

    virtual Qt::HANDLE handle() const;

    inline Type type() const { return QPaintEngine::CoreGraphics; }
    static void initialize();
    static void cleanup();

protected:
    void drawPolyInternal(const QPointArray &a, bool close=true);

private:
    QCoreGraphicsPaintEnginePrivate *d;

private:
#if defined(Q_DISABLE_COPY)
    QCoreGraphicsPaintEngine(const QCoreGraphicsPaintEngine &);
    QCoreGraphicsPaintEngine &operator=(const QCoreGraphicsPaintEngine &);
#endif
};

#endif

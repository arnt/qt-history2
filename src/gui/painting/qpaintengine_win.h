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

#ifndef QPAINTENGINE_WIN_H
#define QPAINTENGINE_WIN_H

#include "qpaintengine.h"

class QPaintDevice;
class QPainterPath;
class QPainterState;
class QTextEngine;
class QTextLayout;
class QWin32PaintEnginePrivate;

class QWin32PaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QWin32PaintEngine)
public:
    QWin32PaintEngine();
    ~QWin32PaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &origin);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);
    void updateClipPath(const QPainterPath &path, bool clipEnabled);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawLine(const QLineF &line);
    void drawRect(const QRectF &r);
    void drawPoint(const QPointF &p);
    void drawEllipse(const QRectF &r);
    void drawPolygon(const QPolygon &p, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr, Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPointF &p, const QTextItem &ti, int textflags);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
			 Qt::PixmapDrawingMode mode);

    void drawPath(const QPainterPath &path);

    inline Type type() const { return QPaintEngine::Windows; }

    static void initialize();
    static void cleanup();

    enum { IsActive=0x01, ExtDev=0x02, IsStartingUp=0x04, NoCache=0x08,
           VxF=0x10, WxF=0x20, ClipOn=0x40, SafePolygon=0x80, MonoDev=0x100,
           DirtyFont=0x200, DirtyPen=0x400, DirtyBrush=0x800,
           RGBColor=0x1000, FontMet=0x2000, FontInf=0x4000, CtorBegin=0x8000,
           UsePrivateCx = 0x10000, VolatileDC = 0x20000, Qt2Compat = 0x40000 };

    QPainter::RenderHints supportedRenderHints() const;

    HDC winHDC() const;

protected:
    QWin32PaintEngine(QWin32PaintEnginePrivate &dptr, PaintEngineFeatures caps);

protected:
    friend class QPainter;
};

class QGdiplusPaintEnginePrivate;
class QGdiplusPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QGdiplusPaintEngine)
public:
    QGdiplusPaintEngine();
    ~QGdiplusPaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawLine(const QLineF &line);
    void drawRect(const QRectF &r);
    void drawPoint(const QPointF &p);
    void drawEllipse(const QRectF &r);
    void drawPolygon(const QPolygon &p, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
		    Qt::PixmapDrawingMode mode);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
			 Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPointF &p, const QTextItem &ti, int textflags);


    void drawPath(const QPainterPath &p);

    Type type() const { return Gdiplus; }

    static void initialize();
    static void cleanup();
};

#endif // QPAINTENGINE_WIN_H

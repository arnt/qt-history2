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

#ifndef QPAINTENGINE_WIN_P_H
#define QPAINTENGINE_WIN_P_H

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

#include <qt_windows.h>

#include "qnamespace.h"

#include <private/qpaintengine_p.h>
#include <private/qpainter_p.h>
#include <private/qpolygonclipper_p.h>

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
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawLine(const QLineF &line);
    void drawRect(const QRectF &r);
    void drawPoint(const QPointF &p);
    void drawEllipse(const QRectF &r);
    void drawPolygon(const QPointF *, int pointCount, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr, Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPointF &p, const QTextItem &ti);

    void drawPath(const QPainterPath &path);

    inline Type type() const { return QPaintEngine::Windows; }

    static void initialize();
    static void cleanup();

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
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawLine(const QLineF &line);
    void drawRect(const QRectF &r);
    void drawPoint(const QPointF &p);
    void drawEllipse(const QRectF &r);
    void drawPolygon(const QPointF *, int pointCount, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
		    Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPointF &p, const QTextItem &ti);

    void drawPath(const QPainterPath &p);

    Type type() const { return Gdiplus; }

    static void initialize();
    static void cleanup();
};

class QPaintDevice;
class QPainterPath;
class QPainterState;
class QTextEngine;
class QTextLayout;
class QWin32PaintEnginePrivate;
class QPainterPathPrivate;

// Typedefs for GDI+
class QtGpGraphics { };
class QtGpMatrix { };
class QtGpRegion { };
class QtGpPen { };
class QtGpBrush { };
class QtGpSolidFill : public QtGpBrush { };
class QtGpPath { };
class QtGpImage { };
class QtGpBitmap : public QtGpImage { };
class QtGpFont { };

struct QtGpStartupInput { Q_UINT32 version; void *cb; BOOL b1; BOOL b2; };

struct QtGpRect
{
    QtGpRect(const QRect &r) : x(r.x()), y(r.y()), w(r.width()), h(r.height()) { }
    int x, y, w, h;
};

struct qt_float_point
{
    float x, y;
    operator POINT () const { POINT pt = { qRound(x), qRound(y) }; return pt; }
};

class QGdiplusPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QGdiplusPaintEngine)
public:
    QGdiplusPaintEnginePrivate() :
        hwnd(0),
        hdc(0),
        graphics(0),
        bitmapDevice(0),
        pen(0),
        brush(0),
        cachedSolidBrush(0),
        usesTempDC(false),
        usePen(false),
        temporaryBrush(false)
    {
    }

    QtGpPath *composeGdiplusPath(const QPainterPath &p);

    HWND hwnd;
    HDC hdc;

    QtGpGraphics *graphics;
    QtGpBitmap *bitmapDevice;
    QtGpPen *pen;
    QtGpBrush *brush;

    QtGpSolidFill *cachedSolidBrush;

    QColor penColor;
    QColor brushColor;

    QPolygonClipper<qt_float_point, qt_float_point, float> polygonClipper;

    uint usesTempDC : 1;
    uint usePen : 1;
    uint temporaryBrush : 1;
};


class QWin32PaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWin32PaintEngine)
public:

    QWin32PaintEnginePrivate() :
        hwnd(0),
        hdc(0),
        hpen(0),
        hbrush(0),
        hbrushbm(0),
        holdpal(0),
        flags(0),
        nocolBrush(false),
        pixmapBrush(false),
        usesWidgetDC(false),
        forceGdi(false),
        forceGdiplus(false),
        advancedMode(false),
        ellipseHack(false),
        penStyle(Qt::SolidLine),
        brushStyle(Qt::SolidPattern),
        pWidth(0),
        pColor(0),
        bColor(0),
        txop(QPainterPrivate::TxNone),
        gdiplusInUse(false),
        gdiplusEngine(0)
    {
    }

    HWND hwnd;
    HDC hdc;
    HPEN hpen;
    HBRUSH hbrush;
    HBITMAP hbrushbm;
    HPALETTE holdpal;
    uint flags;

    uint nocolBrush:1;
    uint pixmapBrush:1;
    uint usesWidgetDC:1;

    uint forceGdi:1;
    uint forceGdiplus:1;
    uint noNativeXform:1;
    uint advancedMode:1;        // Set if running in advanced graphics mode
    uint ellipseHack:1;         // Used to work around ellipse bug in GDI

    Qt::PenStyle penStyle;
    Qt::BrushStyle brushStyle;
    float pWidth;
    COLORREF pColor;
    COLORREF bColor;
    QBrush brush;
    QPen pen;
    Qt::BGMode bgMode;

    QMatrix matrix;
    QMatrix invMatrix;
    QPainterPrivate::TransformationCodes txop;

    QPaintEngine::PaintEngineFeatures oldFeatureSet;

    QPolygonClipper<qt_float_point, POINT, float> polygonClipper;

    /*!
     Switches the paint engine into GDI+ mode
    */
    void beginGdiplus();

    /*!
      Returns true if GDI+ is currently in use
    */
    inline bool usesGdiplus() { return gdiplusInUse && gdiplusEngine; }

    /*!
      Returns true if the engine has any property set that requires it to
      use GDI+ for rendering
    */
    inline bool requiresGdiplus() {
        return !forceGdi && forceGdiplus;
    }

    /*!
      Convenience function for checking if the engine should switch to/from
      GDI+. Returns true if GDI+ is in use after the checking is done.
    */
    inline bool tryGdiplus() {
        if (requiresGdiplus() && !usesGdiplus()) {
            beginGdiplus();
        }
        return usesGdiplus();
    }

    /*!
      Fill rect with current gradient brush, using native function call
    */
    void fillGradient(const QRect &r);

    /*!
      Fill the rect current alpha brush
    */
    void fillAlpha(const QRect &r, const QColor &c);

    /*!
      Composes the path on the current device context
    */
    void composeGdiPath(const QPainterPath &p);

    void setNativeMatrix(const QMatrix &matrix);

    uint gdiplusInUse : 1;
    QGdiplusPaintEngine *gdiplusEngine;
};

#endif // QPAINTENGINE_WIN_P_H

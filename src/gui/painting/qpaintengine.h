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

#ifndef QPAINTENGINE_H
#define QPAINTENGINE_H

#include "QtCore/qnamespace.h"
#include "QtCore/qobjectdefs.h"
#include "QtGui/qpainter.h"

class QFontEngine;
class QLineF;
class QPaintDevice;
class QPaintEnginePrivate;
class QPainterPath;
class QPainterState;
class QPointF;
class QPolygonF;
class QRectF;
struct QGlyphLayout;
class QTextItemInt;

class Q_GUI_EXPORT QTextItem {
public:
    enum RenderFlag {
        RightToLeft = 0x1,
        Overline = 0x10,
        Underline = 0x20,
        StrikeOut = 0x40,

        Dummy = 0xffffffff
    };
    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)
    qreal descent() const;
    qreal ascent() const;
    qreal width() const;

    RenderFlags renderFlags() const;
    QString text() const;
    QFont font() const;
};
Q_DECLARE_TYPEINFO(QTextItem, Q_PRIMITIVE_TYPE);

class Q_GUI_EXPORT QPaintEngine
{
    Q_DECLARE_PRIVATE(QPaintEngine)
public:
    enum PaintEngineFeature {
        CoordTransform            = 0x000001,               // Points are transformed
        PenWidthTransform         = 0x000002,               // Pen width is transformed
        PatternTransform          = 0x000004,               // Brush patterns
        PatternBrush              = 0x000008,               // Native support for pixmap and pattern brushes
        PixmapTransform           = 0x000010,               // Pixmap transforms
        LinearGradientFill   	  = 0x000020,               // Can fill gradient areas.
        LinearGradientFillPolygon = 0x000040,               // Can fill polygons with linear gradients.
        PixmapScale               = 0x000080,               // Can scale (w/o XForm) in drawPixmap
	AlphaFill                 = 0x000100,               // Can fill with alpha.
        AlphaFillPolygon          = 0x000200,               // Can fill polygons with alpha.
        AlphaStroke               = 0x000400,               // Can outline with alpha.
        AlphaPixmap               = 0x000800,               // Can draw pixmaps with alpha channels
        PainterPaths              = 0x001000,               // Can fill, outline and clip paths
        ClipTransform             = 0x002000,               // Can transform clip regions.
        LineAntialiasing          = 0x004000,               // Can antialias lines
        FillAntialiasing          = 0x008000,               // Can antialias fills
        BrushStroke               = 0x010000,              // Can render brush based pens.
        RadialGradientFill        = 0x020000,              // Can render radial gradients.
        ConicalGradientFill       = 0x040000,              // Can render conical gradients
        UsesFontEngine            = 0x10000000,           // Internal use, QWidget and QPixmap
        PaintOutsidePaintEvent    = 0x20000000,            // Engine is capable of painting outside paint events
        QwsPaintEngine    = 0x40000000            // QWS hack ##### remove before RC1
    };
    Q_DECLARE_FLAGS(PaintEngineFeatures, PaintEngineFeature)

    enum DirtyFlag {
        DirtyPen                = 0x0001,
        DirtyBrush              = 0x0002,
        DirtyFont               = 0x0004,
        DirtyBackground         = 0x0008,
        DirtyTransform          = 0x0010,
        DirtyClip               = 0x0020,
        DirtyClipPath           = 0x0040,
        DirtyHints              = 0x0080,

        AllDirty                = 0x00ff
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)

    enum PolygonDrawMode {
        OddEvenMode,
        WindingMode,
        ConvexMode,
        PolylineMode
    };

    explicit QPaintEngine(PaintEngineFeatures features=0);
    virtual ~QPaintEngine();

    bool isActive() const { return active; }
    void setActive(bool newState) { active = newState; }

    virtual bool begin(QPaintDevice *pdev) = 0;
    virtual bool end() = 0;

    virtual void updatePen(const QPen &pen) = 0;
    virtual void updateBrush(const QBrush &brush, const QPointF &origin) = 0;
    virtual void updateFont(const QFont &font) = 0;
    virtual void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush) = 0;
    virtual void updateMatrix(const QMatrix &matrix) = 0;
    virtual void updateClipRegion(const QRegion &region, Qt::ClipOperation op) = 0;
    virtual void updateRenderHints(QPainter::RenderHints hints);
    virtual void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);

    virtual void drawRects(const QRect *rects, int rectCount);
    virtual void drawRects(const QRectF *rects, int rectCount);

    virtual void drawLines(const QLine *lines, int lineCount);
    virtual void drawLines(const QLineF *lines, int lineCount);

    virtual void drawEllipse(const QRectF &r);
    virtual void drawEllipse(const QRect &r);

    virtual void drawPath(const QPainterPath &path);

    virtual void drawPoints(const QPointF *points, int pointCount);
    virtual void drawPoints(const QPoint *points, int pointCount);

    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);

    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                            Qt::PixmapDrawingMode mode = Qt::ComposePixmap) = 0;
    virtual void drawTextItem(const QPointF &p, const QTextItem &textItem);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
				 Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    virtual void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                           Qt::ImageConversionFlags flags = Qt::AutoColor);

    virtual QPainter::RenderHints supportedRenderHints() const;
    QPainter::RenderHints renderHints() const;
    void setRenderHint(QPainter::RenderHint hint, bool on);

    void setPaintDevice(QPaintDevice *device);
    QPaintDevice *paintDevice() const;

    void setSystemClip(const QRegion &baseClip);
    QRegion systemClip() const;

#ifdef Q_WS_WIN
    virtual HDC getDC() const;
    virtual void releaseDC(HDC hdc) const;
#endif

    virtual QPoint coordinateOffset() const;

    enum Type {
        X11,
        Windows,
        QuickDraw, CoreGraphics, MacPrinter,
        QWindowSystem,
        PostScript,
        OpenGL,
        Picture,
        SVG,
        Raster,

        User = 50,    // first user type id
        MaxUser = 100 // last user type id
    };
    virtual Type type() const = 0;

    inline void fix_neg_rect(int *x, int *y, int *w, int *h);
    inline bool testDirty(DirtyFlags df) { return (dirtyFlag & df) != 0; }
    inline void setDirty(DirtyFlags df) { dirtyFlag |= df; }
    inline void clearDirty(DirtyFlags df) { dirtyFlag &= ~static_cast<uint>(df); }

    bool hasFeature(PaintEngineFeatures feature) const { return (gccaps & feature) != 0; }

    QPainter *painter() const;

    inline void syncState() { updateState(state); }

protected:
    QPaintEngine(QPaintEnginePrivate &data, PaintEngineFeatures devcaps=0);

    uint dirtyFlag;
    uint active : 1;
    uint selfDestruct : 1;
    QPainterState *state;
    PaintEngineFeatures gccaps;

    QPaintEnginePrivate *d_ptr;

    inline void updateState(QPainterState *state, bool updateGC = true);

private:
    uint emulationSpecifier;

    inline QPainterState *painterState() const { return state; }
    virtual void updateInternal(QPainterState *state, bool updateGC = true);

    void setAutoDestruct(bool autoDestruct) { selfDestruct = autoDestruct; }
    bool autoDestruct() const { return selfDestruct; }

    friend class QFontEngineBox;
    friend class QFontEngineMac;
    friend class QFontEngineWin;
    friend class QFontEngineFT;
    friend class QFontEngineQPF;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;
    friend class QPSPrintEngine;
    friend class QMacPrintEngine;
    friend class QMacPrintEnginePrivate;
    friend class QPainter;
    friend class QPainterPrivate;
    friend class QWidget;
    friend class QWin32PaintEngine;
    friend class QWin32PaintEnginePrivate;
    friend class QMacCGContext;
};

//
// inline functions
//

inline void QPaintEngine::fix_neg_rect(int *x, int *y, int *w, int *h)
{
    if (*w < 0) {
        *w = -*w;
        *x -= *w - 1;
    }
    if (*h < 0) {
        *h = -*h;
        *y -= *h - 1;
    }
}

inline void QPaintEngine::updateState(QPainterState *newState, bool updateGC)
{
    if (dirtyFlag || state!=newState)
        updateInternal(newState, updateGC);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QPaintEngine::PaintEngineFeatures);
Q_DECLARE_OPERATORS_FOR_FLAGS(QPaintEngine::DirtyFlags);

#endif // QPAINTENGINE_H

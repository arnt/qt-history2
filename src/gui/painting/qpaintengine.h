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

class QTextItem {
public:
    enum RenderFlag {
        RightToLeft = 0x1,
        Overline = 0x10,
        Underline = 0x20,
        StrikeOut = 0x40,

        Dummy = 0xffffffff
    };
    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)
    RenderFlags flags;
    qreal descent;
    qreal ascent;
    qreal width;

    const QChar *chars;
    int num_chars;
    QGlyphLayout *glyphs;
    int num_glyphs;
    QFontEngine *fontEngine;
};
Q_DECLARE_TYPEINFO(QTextItem, Q_PRIMITIVE_TYPE);

class Q_GUI_EXPORT QPaintEngine
{
    Q_DECLARE_PRIVATE(QPaintEngine)
public:
    enum PaintEngineFeature {
        CoordTransform            = 0x0001,               // Points are transformed
        PenWidthTransform         = 0x0002,               // Pen width is transformed
        PatternTransform          = 0x0004,               // Brush patterns
        PatternBrush              = 0x0008,               // Native support for pixmap and pattern brushes
        PixmapTransform           = 0x0010,               // Pixmap transforms
        LinearGradients   	  = 0x0020,               // Can fill gradient areas.
        LinearGradientFillPolygon = 0x0040,               // Can fill polygons with linear gradients.
        PixmapScale               = 0x0080,               // Can scale (w/o XForm) in drawPixmap
	AlphaFill                 = 0x0100,               // Can fill with alpha.
        AlphaFillPolygon          = 0x0200,               // Can fill polygons with alpha.
        AlphaStroke               = 0x0400,               // Can outline with alpha.
        AlphaPixmap               = 0x0800,               // Can draw pixmaps with alpha channels
        PainterPaths              = 0x1000,               // Can fill, outline and clip paths
        ClipTransform             = 0x2000,               // Can transform clip regions.
        LineAntialiasing          = 0x4000,               // Can antialias lines
        FillAntialiasing          = 0x8000,               // Can antialias fills
        UsesFontEngine            = 0x10000000,           // Internal use, QWidget and QPixmap
        PaintOutsidePaintEvent    = 0x20000000            // Engine is capable of painting outside paint events
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
    void setActive(bool state) { active = state; }

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

    virtual void drawEllipse(const QRectF &r);
    virtual void drawLine(const QLineF &line);
    virtual void drawLines(const QLineF *lines, int lineCount);
    virtual void drawPath(const QPainterPath &path);
    virtual void drawPoint(const QPointF &pf);
    virtual void drawPoints(const QPointF *points, int pointCount);
    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);
    virtual void drawRect(const QRectF &rf);
    virtual void drawRects(const QRectF *rects, int rectCount);

    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                            Qt::PixmapDrawingMode mode = Qt::ComposePixmap) = 0;
    virtual void drawTextItem(const QPointF &p, const QTextItem &ti);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
				 Qt::PixmapDrawingMode mode = Qt::ComposePixmap);
    virtual void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                           Qt::ImageConversionFlags flags = Qt::AutoColor);

    virtual QPainter::RenderHints supportedRenderHints() const;
    QPainter::RenderHints renderHints() const;
    void setRenderHint(QPainter::RenderHint hint, bool on);

    void setPaintDevice(QPaintDevice *device);
    QPaintDevice *paintDevice() const;

    enum Type {
        X11,
        Windows, Gdiplus,
        QuickDraw, CoreGraphics, MacPrinter,
        QWindowSystem,
        PostScript,
        OpenGL,
        Picture,
        SVG,

        User = 50,    // first user type id
        MaxUser = 100 // last user type id
    };
    virtual Type type() const = 0;

    enum {
        IsActive     = 0x00000001,
        ExtDev       = 0x00000002,
        IsStartingUp = 0x00000004,
        NoCache      = 0x00000008,
        VxF          = 0x00000010,
        WxF          = 0x00000020,
        ClipOn       = 0x00000040,
        SafePolygon  = 0x00000080,
        MonoDev      = 0x00000100,
//        DirtyFont    = 0x00000200,
//        DirtyPen     = 0x00000400,
//        DirtyBrush   = 0x00000800,
        RGBColor     = 0x00001000,
        FontMet      = 0x00002000,
        FontInf      = 0x00004000,
        CtorBegin    = 0x00008000,
        UsePrivateCx = 0x00010000,
        VolatileDC   = 0x00020000,
        Qt2Compat    = 0x00040000
    };
    inline bool testf(uint b) const { return (flags&b)!=0; }
    inline void setf(uint b) { flags |= b; }
    inline void clearf(uint b) { flags &= (~b); }
    inline void assignf(uint b) { flags = b; }
    inline void fix_neg_rect(int *x, int *y, int *w, int *h);
    inline bool hasClipping() const { return testf(ClipOn); }

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
    uint flags;
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

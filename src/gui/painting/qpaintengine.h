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

#include "qnamespace.h"
#include "qobjectdefs.h"
#include "qpainter.h"
#include "qpointarray.h"

class QFontEngine;
class QPaintDevice;
class QPaintEnginePrivate;
class QPainterPath;
class QPainterState;
class QRectF;
class QLineF;
class QPointF;
struct QGlyphLayout;

class QTextItem {
public:
    unsigned short right_to_left : 1;
    unsigned short underline : 1;
    unsigned short overline : 1;
    unsigned short strikeout : 1;
    short descent;
    int ascent;
    int width;

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
        CoordTransform          = 0x0001,               // Points are transformed
        PenWidthTransform       = 0x0002,               // Pen width is transformed
        PatternTransform        = 0x0004,               // Brush patterns
        PixmapTransform         = 0x0008,               // Pixmap transforms
        LinearGradients   	= 0x0010,               // Can fill gradient areas.
        PixmapScale             = 0x0020,               // Can scale (w/o XForm) in drawPixmap
        DrawRects               = 0x0040,               // Can draw rectangles
	SolidAlphaFill          = 0x0080,               // Can fill with alpha.
        PainterPaths            = 0x0100,               // Can fill, outline and clip paths
        ClipTransform           = 0x0200,               // Can trasform clip regions.
        UsesFontEngine          = 0x10000000,           // Internal use, QWidget and QPixmap
        PaintOutsidePaintEvent  = 0x20000000            // Engine is capable of painting outside paint events
    };
    Q_DECLARE_FLAGS(PaintEngineFeatures, PaintEngineFeature);

    enum DirtyFlags {
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

    enum PolygonDrawMode {
        OddEvenMode,
        WindingMode,
        ConvexMode,
        UnconnectedMode
    };

    QPaintEngine(PaintEngineFeatures features=0);
    virtual ~QPaintEngine();

    bool isActive() const { return active; }
    void setActive(bool state) { active = state; }

    virtual bool begin(QPaintDevice *pdev) = 0;
    virtual bool end() = 0;

    virtual void updatePen(const QPen &pen) = 0;
    virtual void updateBrush(const QBrush &brush, const QPoint &origin) = 0;
    virtual void updateFont(const QFont &font) = 0;
    virtual void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush) = 0;
    virtual void updateXForm(const QMatrix &matrix) = 0;
    virtual void updateClipRegion(const QRegion &region, bool enabled) = 0;
    virtual void updateRenderHints(QPainter::RenderHints hints);
    virtual void updateClipPath(const QPainterPath &path, bool enabled);

    virtual void drawLine(const QPoint &p1, const QPoint &p2);
    virtual void drawRect(const QRect &r);
    virtual void drawRects(const QList<QRect> &rects);
    virtual void drawPoint(const QPoint &p) = 0;
    virtual void drawPoints(const QPointArray &pa);
    virtual void drawEllipse(const QRect &r);
    virtual void drawLineSegments(const QPointArray &);
    virtual void drawPolygon(const QPointArray &pa, PolygonDrawMode mode) = 0;

    virtual void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr,
                            Qt::PixmapDrawingMode mode = Qt::ComposePixmap) = 0;
    virtual void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    virtual void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s,
				 Qt::PixmapDrawingMode mode = Qt::ComposePixmap) = 0;

    // Float functions
    virtual void drawPath(const QPainterPath &path);
    virtual void drawLine(const QLineF &line);
    virtual void drawRect(const QRectF &rf);
    virtual void drawPoint(const QPointF &pf);

    virtual QPainter::RenderHints supportedRenderHints() const;
    QPainter::RenderHints renderHints() const;
    void setRenderHints(QPainter::RenderHints hints);
    void clearRenderHints(QPainter::RenderHints hints);

    enum Type {
        //X11
        X11,
        //Windows
        Windows, Gdiplus,
        //Mac
        QuickDraw, CoreGraphics, MacPrinter,
        //QWS
        QWindowSystem,
        // PostScript
        PostScript,
        // OpenGL
        OpenGL,
        // Picture
        Picture,
        // SVG
        SVG,

        User = 50,                                // first user type id
        MaxUser = 100                                // last user type id
    };
    virtual Type type() const = 0;

    enum { IsActive                   = 0x00000001,
           ExtDev                     = 0x00000002,
           IsStartingUp               = 0x00000004,
           NoCache                    = 0x00000008,
           VxF                         = 0x00000010,
           WxF                         = 0x00000020,
           ClipOn                 = 0x00000040,
           SafePolygon                 = 0x00000080,
           MonoDev                 = 0x00000100,
//            DirtyFont                  = 0x00000200,
//            DirtyPen                 = 0x00000400,
//            DirtyBrush                 = 0x00000800,
           RGBColor                 = 0x00001000,
           FontMet                 = 0x00002000,
           FontInf                 = 0x00004000,
           CtorBegin                 = 0x00008000,
           UsePrivateCx           = 0x00010000,
           VolatileDC                 = 0x00020000,
           Qt2Compat                 = 0x00040000 };
    inline bool testf(uint b) const { return (flags&b)!=0; }
    inline void setf(uint b) { flags |= b; }
    inline void clearf(uint b) { flags &= (uint)(~b); }
    inline void assignf(uint b) { flags = (uint) b; }
    inline void fix_neg_rect(int *x, int *y, int *w, int *h);
    inline bool hasClipping() const { return testf(ClipOn); }

    inline bool testDirty(DirtyFlags df) { return (dirtyFlag & df) != 0; }
    inline void setDirty(DirtyFlags df) { dirtyFlag |= df; }
    inline void clearDirty(DirtyFlags df) { dirtyFlag &= (uint)(~df); }

    bool hasFeature(PaintEngineFeatures feature) const { return (gccaps & feature) != 0; }

    QPainter *painter() const;

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
    inline QPainterState *painterState() const { return state; }
    virtual void updateInternal(QPainterState *state, bool updateGC = true);

    void setAutoDestruct(bool autoDestruct) { selfDestruct = autoDestruct; }
    bool autoDestruct() const { return selfDestruct; }

    friend class QFontEngineBox;
    friend class QFontEngineMac;
    friend class QFontEngineWin;
    friend class QFontEngineFT;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;
    friend class QPSPrintEngine;
    friend class QMacPrintEngine;
    friend class QMacPrintEnginePrivate;
    friend class QPainter;
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
#endif // QPAINTENGINE_H


/****************************************************************************
**
** Definition of the QPaintEngine class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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

class QPainterState;
class QPaintDevice;
struct QGlyphLayout;
class QFontEngine;
class QPaintEnginePrivate;

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

class Q_GUI_EXPORT QPaintEngine : public Qt
{
    Q_DECLARE_PRIVATE(QPaintEngine)
public:
    enum Capability {
        CoordTransform          = 0x0001,               // Points are transformed
        PenWidthTransform       = 0x0002,               // Pen width is transformed
        PatternTransform        = 0x0004,               // Brush patterns
        PixmapTransform         = 0x0008,               // Pixmap transforms
        LinearGradientSupport   = 0x0010,               // Can fill gradient areas.
        PixmapScale             = 0x0020,               // Can scale (w/o XForm) in drawPixmap
        DrawRects               = 0x0040,               // Can draw rectangles
        UsesFontEngine          = 0x10000000            // Internal use, QWidget and QPixmap
    };
    Q_DECLARE_FLAGS(GCCaps, Capability);

    enum DirtyFlags {
        DirtyPen                = 0x0001,
        DirtyBrush              = 0x0002,
        DirtyFont               = 0x0004,
        DirtyBackground         = 0x0008,
        DirtyTransform          = 0x0010,
        DirtyClip               = 0x0020,
        DirtyRasterOp           = 0x0040,

        AllDirty                = 0xffff
    };

    QPaintEngine(GCCaps devcaps=0);
    virtual ~QPaintEngine();

    bool isActive() const { return active; }
    void setActive(bool state) { active = state; }

    virtual bool begin(QPaintDevice *pdev, QPainterState *state, bool unclipped = false) = 0;
    virtual bool end() = 0;

    virtual void updatePen(QPainterState *ps) = 0;
    virtual void updateBrush(QPainterState *ps) = 0;
    virtual void updateFont(QPainterState *ps) = 0;
    virtual void updateRasterOp(QPainterState *ps) = 0;
    virtual void updateBackground(QPainterState *ps) = 0;
    virtual void updateXForm(QPainterState *ps) = 0;
    virtual void updateClipRegion(QPainterState *ps) = 0;

    virtual void drawLine(const QPoint &p1, const QPoint &p2) = 0;
    virtual void drawRect(const QRect &r) = 0;
    virtual void drawRects(const QList<QRect> &rects);
    virtual void drawPoint(const QPoint &p) = 0;
    virtual void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1) = 0;
    virtual void drawRoundRect(const QRect &r, int xRnd, int yRnd) = 0;
    virtual void drawEllipse(const QRect &r) = 0;
    virtual void drawArc(const QRect &r, int a, int alen) = 0;
    virtual void drawPie(const QRect &r, int a, int alen) = 0;
    virtual void drawChord(const QRect &r, int a, int alen) = 0;
    virtual void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1) = 0;
    virtual void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1) = 0;
    virtual void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1) = 0;
    virtual void drawConvexPolygon(const QPointArray &, int index = 0, int npoints = -1) = 0;
#ifndef QT_NO_BEZIER
    virtual void drawCubicBezier(const QPointArray &, int index = 0) = 0;
#endif

    virtual void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, bool imask) = 0;
    virtual void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    virtual void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim) = 0;

#if defined Q_WS_WIN // ### not liking this!!
    virtual HDC handle() const = 0;
#else
    virtual Qt::HANDLE handle() const = 0;
#endif

    virtual QPainter::RenderHints supportedRenderHints() const;
    virtual QPainter::RenderHints renderHints() const;
    virtual void setRenderHint(QPainter::RenderHint hint, bool enable);

    enum Type {
        //X11
        X11,
        //Windows
        Windows, Gdiplus,
        //Mac
        QuickDraw, CoreGraphics,
        //QWS
        QWindowSystem,
        //Magic wrapper type
        Wrapper,
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
    inline void setDirty(DirtyFlags df) { dirtyFlag|=df; changeFlag|=df; }
    inline void unsetDirty(DirtyFlags df) { dirtyFlag &= (uint)(~df); }

    bool hasCapability(Capability cap) const { return (gccaps & cap) != 0; }

    inline void updateState(QPainterState *state, bool updateGC = true);
    inline QPainterState *painterState() const { return state; }

protected:
    QPaintEngine(QPaintEnginePrivate &data, GCCaps devcaps=0);

    uint dirtyFlag;
    uint changeFlag;
    uint active : 1;
    uint flags;
    QPainterState *state;
    GCCaps gccaps;

    QPaintEnginePrivate *d_ptr;

    friend class QWrapperPaintEngine;
    friend class QPainter;

private:
    void updateInternal(QPainterState *state, bool updateGC = true);
};

class QWrapperPaintEngine : public QPaintEngine
{
public:
    QWrapperPaintEngine(QPaintEngine *w) : QPaintEngine(w->gccaps), wrap(w) { }

    virtual bool begin(QPaintDevice *pdev, QPainterState *state, bool unclipped) { return wrap->begin(pdev, state, unclipped); }
    virtual bool end() { return wrap->end(); }

    virtual void updatePen(QPainterState *ps);
    virtual void updateBrush(QPainterState *ps);
    virtual void updateFont(QPainterState *ps);
    virtual void updateRasterOp(QPainterState *ps);
    virtual void updateBackground(QPainterState *ps);
    virtual void updateXForm(QPainterState *ps);
    virtual void updateClipRegion(QPainterState *ps);

    virtual void drawLine(const QPoint &p1, const QPoint &ps);
    virtual void drawRect(const QRect &r);
    virtual void drawPoint(const QPoint &p);
    virtual void drawPoints(const QPointArray &pa, int index, int npoints = -1);
    virtual void drawRoundRect(const QRect &r, int xRnd, int yRnd);
    virtual void drawEllipse(const QRect &r);
    virtual void drawArc(const QRect &r, int a, int alen);
    virtual void drawPie(const QRect &r, int a, int alen);
    virtual void drawChord(const QRect &r, int a, int alen);
    virtual void drawLineSegments(const QPointArray &, int index, int nlines = -1);
    virtual void drawPolyline(const QPointArray &pa, int index, int npoints = -1);
    virtual void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1);
    virtual void drawConvexPolygon(const QPointArray &, int index, int npoints = -1);
#ifndef QT_NO_BEZIER
    virtual void drawCubicBezier(const QPointArray &, int index);
#endif

    virtual void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, bool imask);
    virtual void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    virtual void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

#if defined Q_WS_WIN // ### not liking this!!
    virtual HDC handle() const { return wrap->handle(); }
#else
    virtual Qt::HANDLE handle() const { return wrap->handle(); }
#endif

    inline QPaintEngine *wrapped() const { return wrap; }
    virtual Type type() const { return QPaintEngine::Wrapper; }

private:
    QPaintEngine *wrap;
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

Q_DECLARE_OPERATORS_FOR_FLAGS(QPaintEngine::GCCaps);
#endif // QPAINTENGINE_H


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

#ifndef QPAINTENGINE_X11_P_H
#define QPAINTENGINE_X11_P_H

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
#include "qregion.h"
#include "qpen.h"
#include "qpoint.h"
#include <private/qpaintengine_p.h>
#include <private/qpainter_p.h>
#include <private/qpolygonclipper_p.h>

class QX11PaintEnginePrivate;

typedef unsigned long Picture;
#include "qx11info_x11.h"

struct qt_XPoint {
    short x;
    short y;
};

struct qt_float_point
{
    qreal x, y;
    operator qt_XPoint() const
    {
        qt_XPoint pt = { static_cast<short>(qRound(x)),
                         static_cast<short>(qRound(y)) };
        return pt;
    }
};

class QX11PaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QX11PaintEngine)
public:
    QX11PaintEngine();
    ~QX11PaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateRenderHints(QPainter::RenderHints hints);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);

    void drawLines(const QLine *lines, int lineCount);
    inline void drawLines(const QLineF *lines, int lineCount) { QPaintEngine::drawLines(lines, lineCount); }

    void drawRects(const QRect *rects, int rectCount);
    inline void drawRects(const QRectF *rects, int rectCount) { QPaintEngine::drawRects(rects, rectCount); }

    inline void drawPoints(const QPoint *points, int pointCount) { QPaintEngine::drawPoints(points, pointCount); }
    void drawPoints(const QPointF *points, int pointCount);

    void drawEllipse(const QRect &r);
    inline void drawEllipse(const QRectF &r) { QPaintEngine::drawEllipse(r); }

    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    inline void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
        { QPaintEngine::drawPolygon(points, pointCount, mode); }

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
    void drawPath(const QPainterPath &path);
    void drawTextItem(const QPointF &p, const QTextItem &textItem);

    virtual Qt::HANDLE handle() const;
    inline Type type() const { return QPaintEngine::X11; }

    QPainter::RenderHints supportedRenderHints() const;

protected:
    QX11PaintEngine(QX11PaintEnginePrivate &dptr);

    void drawMulti(const QPointF &p, const QTextItemInt &si);
    void drawBox(const QPointF &p, const QTextItemInt &si);
    void drawXLFD(const QPointF &p, const QTextItemInt &si);
#ifndef QT_NO_FONTCONFIG
    void drawFreetype(const QPointF &p, const QTextItemInt &si);
    void core_render_glyph(QFontEngineFT *fe, int xp, int yp, uint glyph);
#endif

    friend void qt_cleanup();
    friend void qt_draw_transformed_rect(QPaintEngine *pp, int x, int y, int w, int h, bool fill);
    friend void qt_draw_background(QPaintEngine *pp, int x, int y, int w, int h);
    friend class QPixmap;
    friend class QFontEngineBox;

private:
    Q_DISABLE_COPY(QX11PaintEngine)
};

class QX11PaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QX11PaintEngine)
public:
    QX11PaintEnginePrivate()
    {
        dpy = 0;
        scrn = -1;
        hd = 0;
        picture = 0;
        bg_col = Qt::white;                             // default background color
        bg_mode = Qt::TransparentMode;                  // default background mode
        gc = gc_brush = 0;
        dpy  = 0;
        xinfo = 0;
        txop = QPainterPrivate::TxNone;
        has_clipping = false;
        render_hints = 0;
    }
    enum GCMode {
        PenGC,
        BrushGC
    };

    void init();
    void fillPolygon(const QPointF *points, int pointCount, GCMode gcMode,
                     QPaintEngine::PolygonDrawMode mode);
    void fillPath(const QPainterPath &path, GCMode gcmode, bool transform);
    void strokePolygon(const QPointF *points, int pointCount);
    void setupAdaptedOrigin(const QPoint &p);
    void resetAdaptedOrigin();

    Display *dpy;
    int scrn;
    Qt::HANDLE hd;
#if !defined (QT_NO_XRENDER)
    Qt::HANDLE picture;
#else
    Qt::HANDLE picture;
#endif
    GC gc;
    GC gc_brush;

    QColor bg_col;
    uchar bg_mode;
    QPen cpen;
    QBrush cbrush;
    QBrush bg_brush;
    QRegion crgn;
    QMatrix matrix;

    uint use_path_fallback : 1;
    uint has_clipping : 1;
    uint adapted_brush_origin : 1;
    uint adapted_pen_origin : 1;
    uint render_hints;

    const QX11Info *xinfo;
    QPointF bg_origin;
    QPainterPrivate::TransformationCodes txop;
    QPolygonClipper<qt_float_point, qt_XPoint, short> polygonClipper;
    QPolygonClipper<qt_float_point, qt_float_point, float> floatClipper;
};

#endif // QPAINTENGINE_X11_P_H

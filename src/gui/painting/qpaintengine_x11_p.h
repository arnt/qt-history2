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

typedef struct _XftDraw XftDraw;
#include "qx11info_x11.h"

struct qt_XPoint {
    short x;
    short y;
};

struct qt_float_point
{
    float x, y;
    operator qt_XPoint() const
    {
        qt_XPoint pt = { static_cast<short>(qRound(x)),
                         static_cast<short>(qRound(y)) };
        return pt;
    }
};

class QX11PaintEnginePrivate : public QPaintEnginePrivate {

public:
    QX11PaintEnginePrivate()
    {
        dpy = 0;
        scrn = -1;
        hd = 0;
        xft_hd = 0;
        bg_col = Qt::white;                             // default background color
        bg_mode = Qt::TransparentMode;                  // default background mode
        tabstops = 0;                               // default tabbing
        tabarray = 0;
        tabarraylen = 0;
        ps_stack = 0;
        wm_stack = 0;
        gc = gc_brush = 0;
        dpy  = 0;
        penRef = brushRef = 0;
        clip_serial = 0;
        xinfo = 0;
        txop = QPainterPrivate::TxNone;
    }
    Display *dpy;
    int scrn;
    Qt::HANDLE hd;
#if !defined (QT_NO_XFT)
    XftDraw *xft_hd;
#else
    Qt::HANDLE xft_hd;
#endif
    GC gc;
    GC gc_brush;

    QColor bg_col;
    uchar bg_mode;
    QPen cpen;
    QBrush cbrush;
    QBrush bg_brush;
    QRegion crgn;
    int tabstops;
    int *tabarray;
    int tabarraylen;

    void *penRef;
    void *brushRef;
    void *ps_stack;
    void *wm_stack;
    uint clip_serial;
    const QX11Info *xinfo;
    QPointF bg_origin;
    QPainterPrivate::TransformationCodes txop;
    QPolygonClipper<qt_float_point, qt_XPoint, short> polygonClipper;
    QPolygonClipper<qt_float_point, qt_float_point, float> floatClipper;
};

class QX11PaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QX11PaintEngine)

public:
    QX11PaintEngine();
    ~QX11PaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateRenderHints(QPainter::RenderHints hints);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);

    virtual void drawLine(const QLineF &line);
    virtual void drawRect(const QRectF &r);
    virtual void drawRects(const QRectF *lines, int lineCount);
    virtual void drawPoint(const QPointF &p);
    virtual void drawEllipse(const QRectF &r);
    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);

    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                            Qt::PixmapDrawingMode mode);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
				 Qt::PixmapDrawingMode mode);
    void drawTextItem(const QPointF &p, const QTextItem &ti);

    virtual Qt::HANDLE handle() const;
    inline Type type() const { return QPaintEngine::X11; }

    QPainter::RenderHints supportedRenderHints() const;

    static void initialize();
    static void cleanup();

protected:
    QX11PaintEngine(QX11PaintEnginePrivate &dptr);

    void drawMulti(const QPointF &p, const QTextItem &si);
    void drawBox(const QPointF &p, const QTextItem &si);
    void drawXLFD(const QPointF &p, const QTextItem &si);
#ifndef QT_NO_XFT
    void drawXft(const QPointF &p, const QTextItem &si);
#endif

    friend void qt_cleanup();
    friend void qt_draw_transformed_rect(QPaintEngine *pp,  int x, int y, int w,  int h, bool fill);
    friend void qt_draw_background(QPaintEngine *pp, int x, int y, int w,  int h);
    friend class QPixmap;
    friend class QFontEngineBox;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;

private:
    Q_DISABLE_COPY(QX11PaintEngine)
};

#endif // QPAINTENGINE_X11_P_H

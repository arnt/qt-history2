/****************************************************************************
**
** Definition of QWSPaintEngine class.
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

#ifndef QWSPAINTENGINE_QWS_H
#define QWSPAINTENGINE_QWS_H

#include "qpaintengine.h"
#include "qshared.h"


class QGfx;
struct QWSPaintEngineData;
class QWSPaintEnginePrivate;
class QPainterState;
class QApplicationPrivate;

class QWSPaintEngine : public QPaintEngine
{
public:
    QWSPaintEngine(QPaintEnginePrivate *dptr, const QPaintDevice *);
    ~QWSPaintEngine();

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

    void drawLine(const QPoint &p1, const QPoint &p2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor);
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
    void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

    virtual Qt::HANDLE handle() const;
    inline Type type() const { return QPaintEngine::QWindowSystem; }

    static void initialize();
    static void cleanup();


    QGfx *gfx();

protected:
    void drawPolyInternal(const QPointArray &a, bool close=TRUE);

    void copyQWSData(const QWSPaintEngine *);
    void cloneQWSData(const QWSPaintEngine *);
    //virtual void setQWSData(const QWSPaintEngineData *);
    QWSPaintEngineData* getQWSData(bool def = FALSE) const;

    friend void qt_init( QApplicationPrivate *, int );
    friend void qt_cleanup();
    friend void qt_draw_transformed_rect( QPainter *pp,  int x, int y, int w,  int h, bool fill );
    friend void qt_draw_background( QPainter *pp, int x, int y, int w,  int h );
    friend class QWidget;
    friend class QPixmap;
    friend class QFontEngineBox;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;

    Q_DECL_PRIVATE(QWSPaintEngine);

private:
//    friend class QWidget;
//    friend class QPixmap;
    friend class QFontEngine;

   //QWSPaintEngineData *qwsData;

private:
#if defined(Q_DISABLE_COPY)
    QWSPaintEngine(const QWSPaintEngine &);
    QWSPaintEngine &operator=(const QWSPaintEngine &);
#endif
};

struct QWSPaintEngineData : public QShared {
    /*Display*/ void *x_display;
    int x_screen;
    int x_depth;
    int x_cells;
    Qt::HANDLE x_colormap;
    bool x_defcolormap;
    void *x_visual;
    bool x_defvisual;
};


#endif

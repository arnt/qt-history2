/****************************************************************************
**
** Definition of QWSPaintEngine class.
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

#ifndef QWSPAINTENGINE_QWS_H
#define QWSPAINTENGINE_QWS_H

#include "qatomic.h"
#include "qpaintengine.h"

class QGfx;
struct QWSPaintEngineData;
class QWSPaintEnginePrivate;
class QPainterState;
class QApplicationPrivate;

class QWSPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QWSPaintEngine)

public:
    QWSPaintEngine(QPaintDevice *);
    ~QWSPaintEngine();

    bool begin(QPaintDevice *pdev, bool unclipped = false);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPoint &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateXForm(const QWMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);

    void drawLine(const QPoint &p1, const QPoint &p2);
    void drawRect(const QRect &r);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
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

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, bool imask);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

    virtual Qt::HANDLE handle() const;
    inline Type type() const { return QPaintEngine::QWindowSystem; }

    static void initialize();
    static void cleanup();


    QGfx *gfx();

protected:
    QWSPaintEngine(QPaintEnginePrivate &dptr, QPaintDevice *);

    void drawPolyInternal(const QPointArray &a, bool close=true);

    void copyQWSData(const QWSPaintEngine *);
    void cloneQWSData(const QWSPaintEngine *);
    //virtual void setQWSData(const QWSPaintEngineData *);
    QWSPaintEngineData* getQWSData(bool def = false) const;

    friend void qt_init(QApplicationPrivate *, int);
    friend void qt_cleanup();
    friend void qt_draw_transformed_rect(QPainter *pp,  int x, int y, int w,  int h, bool fill);
    friend void qt_draw_background(QPainter *pp, int x, int y, int w,  int h);
    friend class QWidget;
    friend class QPixmap;
    friend class QFontEngineBox;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;

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

/* I don't know where you use this, but I removed the QShared and gave you a QAtomic --tws */
struct QWSPaintEngineData {
    QAtomic ref;
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

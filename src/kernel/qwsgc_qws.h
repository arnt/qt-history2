/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWSGC_QWS_H
#define QWSGC_QWS_H

#include "qabstractgc.h"
#include "qshared.h"


class QGfx;
struct QWSGCData;
class QWSGCPrivate;
class QPainterState;
class QApplicationPrivate;

class QWSGC : public QAbstractGC
{
public:
    QWSGC(const QPaintDevice *);
    ~QWSGC();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool begin = FALSE);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

    void setRasterOp(RasterOp r);

    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int w, int h);
    void drawPoint(int x, int y);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &penColor);
    void drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd);
    void drawEllipse(int x, int y, int w, int h);
    void drawArc(int x, int y, int w, int h, int a, int alen);
    void drawPie(int x, int y, int w, int h, int a, int alen);
    void drawChord(int x, int y, int w, int h, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif

    void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh);
    void drawTextItem(int x, int y, const QTextItem &ti, int textflags);

    virtual Qt::HANDLE handle() const;


    static void initialize();
    static void cleanup();


    QGfx *gfx();
    
protected:
    void copyQWSData(const QWSGC *);
    void cloneQWSData(const QWSGC *);
    //virtual void setQWSData(const QWSGCData *);
    QWSGCData* getQWSData(bool def = FALSE) const;

    friend void qt_init( QApplicationPrivate *, int );
    friend void qt_cleanup();
    friend void qt_draw_transformed_rect( QPainter *pp,  int x, int y, int w,  int h, bool fill );
    friend void qt_draw_background( QPainter *pp, int x, int y, int w,  int h );
    friend class QWidget;
    friend class QPixmap;
    friend class QFontEngineBox;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;

private:
//    friend class QWidget;
//    friend class QPixmap;
    friend class QFontEngine;

    QWSGCPrivate *d;
    //QWSGCData *qwsData;

private:
#if defined(Q_DISABLE_COPY)
    QWSGC(const QWSGC &);
    QWSGC &operator=(const QWSGC &);
#endif
};

struct QWSGCData : public QShared {
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

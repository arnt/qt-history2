/****************************************************************************
**
** Definition of QOpenGLPaintEngine class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QOPENGLPAINTENGINE_H
#define QOPENGLPAINTENGINE_H

#include "qpaintengine.h"

class QPainterState;
class QOpenGLPaintEnginePrivate;

class QOpenGLPaintEngine : public QPaintEngine
{
public:
    QOpenGLPaintEngine(const QPaintDevice *);
    ~QOpenGLPaintEngine();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool begin = FALSE);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

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
    void drawPolyInternal(const QPointArray &pa, bool close = true);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr);
    void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

    Qt::HANDLE handle() const;
    inline Type type() const { return QPaintEngine::OpenGL; }

private:
    QOpenGLPaintEnginePrivate *d;

private:
#if defined(Q_DISABLE_COPY)
    QOpenGLPaintEngine(const QOpenGLPaintEngine &);
    QOpenGLPaintEngine &operator=(const QOpenGLPaintEngine &);
#endif
};
#endif // QOPENGLPAINTENGINE_H

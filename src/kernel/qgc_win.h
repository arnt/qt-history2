/****************************************************************************
**
** Definition of QWin32PaintEngine class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIN32PAINTENGINE_H
#define QWIN32PAINTENGINE_H

#include "qabstractgc.h"

class QWin32PaintEnginePrivate;
class QPainterState;
class QPaintDevice;

class QTextLayout;
class QTextEngine;

class QWin32PaintEngine : public QPaintEngine
{
public:
    QWin32PaintEngine(const QPaintDevice *target);
    ~QWin32PaintEngine();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped=FALSE);
    bool end();

    void updatePen(QPainterState *state);
    void updateBrush(QPainterState *state);
    void updateFont(QPainterState *state);
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
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1);
    void drawConvexPolygon(const QPointArray &, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &, int index = 0);
#endif

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr);
    void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

    HDC handle() const; // ### Still not liking this...
    inline Type type() const { return QPaintEngine::Windows; }

    static void initialize();
    static void cleanup();

    enum { IsActive=0x01, ExtDev=0x02, IsStartingUp=0x04, NoCache=0x08,
	   VxF=0x10, WxF=0x20, ClipOn=0x40, SafePolygon=0x80, MonoDev=0x100,
	   DirtyFont=0x200, DirtyPen=0x400, DirtyBrush=0x800,
	   RGBColor=0x1000, FontMet=0x2000, FontInf=0x4000, CtorBegin=0x8000,
           UsePrivateCx = 0x10000, VolatileDC = 0x20000, Qt2Compat = 0x40000 };

private:
    void drawPolyInternal( const QPointArray &a, bool close );

protected:
    friend class QPainter;

    QWin32PaintEnginePrivate *d;
};

#endif // QWIN32PAINTENGINE_H

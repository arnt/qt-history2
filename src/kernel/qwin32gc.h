#ifndef QWIN32GC_H
#define QWIN32GC_H

#include "qabstractgc.h"

class QWin32GCPrivate;
class QPainterState;
class QPaintDevice;

class QTextLayout;
class QTextEngine;

class QWin32GC : public QAbstractGC
{
public:
    QWin32GC(const QPaintDevice *target);
    ~QWin32GC();

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

    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x, int y, int w, int h);
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
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1);
    void drawConvexPolygon(const QPointArray &, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pts, int index = 0);
#endif
    void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh);
    void drawTextItem(int x, int y, const QTextItem &ti, int textflags);

    HDC handle() const; // ### Still not liking this...

    static void initialize();

    enum { IsActive=0x01, ExtDev=0x02, IsStartingUp=0x04, NoCache=0x08,
	   VxF=0x10, WxF=0x20, ClipOn=0x40, SafePolygon=0x80, MonoDev=0x100,
	   DirtyFont=0x200, DirtyPen=0x400, DirtyBrush=0x800,
	   RGBColor=0x1000, FontMet=0x2000, FontInf=0x4000, CtorBegin=0x8000,
           UsePrivateCx = 0x10000, VolatileDC = 0x20000, Qt2Compat = 0x40000 };

private:
    void drawPolyInternal( const QPointArray &a, bool close );

protected:
    friend class QPainter;

    QWin32GCPrivate *d;
};

#endif // QWIN32GC_H

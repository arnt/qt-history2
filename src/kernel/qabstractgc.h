#ifndef QABSTRACTGC_H
#define QABSTRACTGC_H

#include "qnamespace.h"
#include "qrect.h"
#include "qpointarray.h"

#include "q4painter.h"

class QPainterState;
class QPaintDevice;

struct QAbstractGCPrivate {
    QAbstractGCPrivate() : active(false), flags(0) {}
    bool active : 1;
    uint flags;
};

class QAbstractGC : public Qt
{
public:
    QAbstractGC();
    virtual ~QAbstractGC() { delete d_ptr; }

    bool isActive() const { return d_ptr->active; }
    void setActive(bool state) { d_ptr->active = state; };

    virtual bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped = FALSE) = 0;
    virtual bool end() = 0;

    virtual void updatePen(QPainterState *ps) = 0;
    virtual void updateBrush(QPainterState *ps) = 0;
    virtual void updateFont(QPainterState *ps) = 0;
    virtual void updateRasterOp(QPainterState *ps) = 0;
    virtual void updateBackground(QPainterState *ps) = 0;
    virtual void updateXForm(QPainterState *ps) = 0;
    virtual void updateClipRegion(QPainterState *ps) = 0;

    virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
    virtual void drawRect(int x, int y, int w, int h) = 0;
    virtual void drawPoint(int x, int y) = 0;
    virtual void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1) = 0;
    virtual void drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &bgColor) = 0;
    virtual void drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd) = 0;
    virtual void drawEllipse(int x, int y, int w, int h) = 0;
    virtual void drawArc(int x, int y, int w, int h, int a, int alen) = 0;
    virtual void drawPie(int x, int y, int w, int h, int a, int alen) = 0;
    virtual void drawChord(int x, int y, int w, int h, int a, int alen) = 0;
    virtual void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1) = 0;
    virtual void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1) = 0;
    virtual void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1) = 0;
    virtual void drawConvexPolygon(const QPointArray &, int index = 0, int npoints = -1) = 0;
#ifndef QT_NO_BEZIER
    virtual void drawCubicBezier(const QPointArray &, int index = 0) = 0;
#endif

    virtual void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh) = 0;
    virtual void drawTextItem(int x, int y, const QTextItem &ti, int textflags) = 0;

#if defined Q_WS_WIN // ### not liking this!!
    virtual HDC handle() const = 0;
#else
    virtual Qt::HANDLE handle() const = 0;
#endif

    enum { IsActive=0x01, ExtDev=0x02, IsStartingUp=0x04, NoCache=0x08,
	   VxF=0x10, WxF=0x20, ClipOn=0x40, SafePolygon=0x80, MonoDev=0x100,
	   DirtyFont=0x200, DirtyPen=0x400, DirtyBrush=0x800,
	   RGBColor=0x1000, FontMet=0x2000, FontInf=0x4000, CtorBegin=0x8000,
           UsePrivateCx = 0x10000, VolatileDC = 0x20000, Qt2Compat = 0x40000 };
    inline bool testf( uint b ) const { return (d_ptr->flags&b)!=0; }
    inline void setf( uint b ) { d_ptr->flags |= b; }
    inline void clearf( uint b ) { d_ptr->flags &= (uint)(~b); }
    inline void fix_neg_rect( int *x, int *y, int *w, int *h );
    inline bool hasClipping() const { return testf(ClipOn); }

    enum Capability {
	CoordTransform,		// Points are transformed
	PenWidthTransform,	// Pen width is transformed
	PatternTransform,	// Brush patterns
	PixmapTransform         //
    };

    virtual bool hasCapability(Capability) const { return 0; }

    inline void setState(QPainterState *state);

private:
    void updateInternal(QPainterState *state);

protected:
    QPainterState *state;

private:
    QAbstractGCPrivate *d_ptr;
};

//
// inline functions
//

inline void QAbstractGC::fix_neg_rect(int *x, int *y, int *w, int *h)
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

inline void QAbstractGC::setState(QPainterState *newState)
{
    if (state==newState)
	return;
    updateInternal(newState);
}


#endif // QABSTRACTGC_H

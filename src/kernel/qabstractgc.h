/****************************************************************************
**
** Definition of QPainter class.
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

#ifndef QABSTRACTGC_H
#define QABSTRACTGC_H

#include "qnamespace.h"
#include "qpointarray.h"

#include "qpainter.h"

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
    enum Capability {
	CoordTransform          = 0x0001,		// Points are transformed
	PenWidthTransform	= 0x0002, 		// Pen width is transformed
	PatternTransform        = 0x0004,		// Brush patterns
	PixmapTransform         = 0x0008                // Pixmap transforms
    };
    Q_DECLARE_FLAGS(GCCaps, Capability);

    enum DirtyFlags {
	DirtyPen              	= 0x0001,
	DirtyBrush              = 0x0002,
	DirtyFont               = 0x0004,
	DirtyBackground         = 0x0008,
 	DirtyTransform          = 0x0010,
 	DirtyClip               = 0x0020,
	DirtyRasterOp 		= 0x0040
    };

    QAbstractGC(GCCaps devcaps=0);
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
    virtual void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx, int sy, bool optim) = 0;

#if defined Q_WS_WIN // ### not liking this!!
    virtual HDC handle() const = 0;
#else
    virtual Qt::HANDLE handle() const = 0;
#endif

    enum Type {
	//X11
	X11,
	//Windows
	Windows,
	//Mac
	QuickDraw, CoreGraphics,
	//QWS
	QWindowSystem,
	//Magic wrapper type
	Wrapper,

	User = 50,				// first user type id
	MaxUser = 100				// last user type id
    };
    virtual Type type() const = 0;

    enum { IsActive           	= 0x00000001,
	   ExtDev             	= 0x00000002,
	   IsStartingUp       	= 0x00000004,
	   NoCache    	      	= 0x00000008,
	   VxF 		      	= 0x00000010,
	   WxF 			= 0x00000020,
	   ClipOn 		= 0x00000040,
	   SafePolygon 		= 0x00000080,
	   MonoDev 		= 0x00000100,
// 	   DirtyFont  		= 0x00000200,
// 	   DirtyPen 		= 0x00000400,
// 	   DirtyBrush 		= 0x00000800,
	   RGBColor 		= 0x00001000,
	   FontMet 		= 0x00002000,
	   FontInf 		= 0x00004000,
	   CtorBegin 		= 0x00008000,
           UsePrivateCx   	= 0x00010000,
	   VolatileDC 		= 0x00020000,
	   Qt2Compat 		= 0x00040000 };
    inline bool testf( uint b ) const { return (d_ptr->flags&b)!=0; }
    inline void setf( uint b ) { d_ptr->flags |= b; }
    inline void clearf( uint b ) { d_ptr->flags &= (uint)(~b); }
    inline void assignf( uint b ) { d_ptr->flags = (uint) b; }
    inline void fix_neg_rect( int *x, int *y, int *w, int *h );
    inline bool hasClipping() const { return testf(ClipOn); }

    inline bool testDirty(DirtyFlags df) { return (dirtyFlag & df) != 0; }
    inline void setDirty(DirtyFlags df) { dirtyFlag|=df; changeFlag|=df; }
    inline void unsetDirty(DirtyFlags df) { dirtyFlag &= (uint)(~df); }

    bool hasCapability(Capability cap) const { return gccaps&cap; }

    inline void updateState(QPainterState *state, bool updateGC = true);

private:
    void updateInternal(QPainterState *state, bool updateGC = true);

protected:
    uint dirtyFlag;
    uint changeFlag;
    QPainterState *state;
    GCCaps gccaps;

private:
    QAbstractGCPrivate *d_ptr;

    friend class QWrapperGC;
    friend class QPainter;
};

class QWrapperGC : public QAbstractGC
{
public:
    QWrapperGC(QAbstractGC *w) : QAbstractGC(w->gccaps), wrap(w) { }

    virtual bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped) { return wrap->begin(pdev, state, unclipped); }
    virtual bool end() { return wrap->end(); }

    virtual void updatePen(QPainterState *ps) { wrap->updatePen(ps); }
    virtual void updateBrush(QPainterState *ps) { wrap->updateBrush(ps); }
    virtual void updateFont(QPainterState *ps) { wrap->updateFont(ps); }
    virtual void updateRasterOp(QPainterState *ps) { wrap->updateRasterOp(ps); }
    virtual void updateBackground(QPainterState *ps) { wrap->updateBackground(ps); }
    virtual void updateXForm(QPainterState *ps) { wrap->updateXForm(ps); }
    virtual void updateClipRegion(QPainterState *ps) { wrap->updateClipRegion(ps); }

    virtual void drawLine(int x1, int y1, int x2, int y2) { wrap->drawLine(x1, y1, x2, y2); }
    virtual void drawRect(int x, int y, int w, int h) { wrap->drawRect(x, y, w, h); }
    virtual void drawPoint(int x, int y) { wrap->drawPoint(x, y); }
    virtual void drawPoints(const QPointArray &pa, int index, int npoints) { wrap->drawPoints(pa, index, npoints); }
    virtual void drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &bgColor) { wrap->drawWinFocusRect(x, y, w, h, xorPaint, bgColor); }
    virtual void drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd) { wrap->drawRoundRect(x, y, w, h, xRnd, yRnd); }
    virtual void drawEllipse(int x, int y, int w, int h) { wrap->drawEllipse(x, y, w, h); }
    virtual void drawArc(int x, int y, int w, int h, int a, int alen) { wrap->drawArc(x, y, w, h, a, alen); }
    virtual void drawPie(int x, int y, int w, int h, int a, int alen) { wrap->drawPie(x, y, w, h, a, alen); }
    virtual void drawChord(int x, int y, int w, int h, int a, int alen) { wrap->drawChord(x, y, w, h, a, alen); }
    virtual void drawLineSegments(const QPointArray &pa, int index, int nlines) { wrap->drawLineSegments(pa, index, nlines); }
    virtual void drawPolyline(const QPointArray &pa, int index, int npoints) { wrap->drawPolyline(pa, index, npoints); }
    virtual void drawPolygon(const QPointArray &pa, bool winding, int index, int npoints) { wrap->drawPolygon(pa, winding, index, npoints); }
    virtual void drawConvexPolygon(const QPointArray &pa, int index, int npoints) { wrap->drawConvexPolygon(pa, index, npoints); }
#ifndef QT_NO_BEZIER
    virtual void drawCubicBezier(const QPointArray &pa, int index) { wrap->drawCubicBezier(pa, index); }
#endif

    virtual void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh) { wrap->drawPixmap(x, y, pm, sx, sy, sw, sh); }
    virtual void drawTextItem(int x, int y, const QTextItem &ti, int textflags) { wrap->drawTextItem(x, y, ti, textflags); }
    virtual void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx, int sy, bool optim) { wrap->drawTiledPixmap(x, y, w, h, pixmap, sx, sy, optim); }

#if defined Q_WS_WIN // ### not liking this!!
    virtual HDC handle() const { return wrap->handle(); }
#else
    virtual Qt::HANDLE handle() const { return wrap->handle(); }
#endif

    inline QAbstractGC *wrapped() const { return wrap; }
    virtual Type type() const { return QAbstractGC::Wrapper; }

private:
    QAbstractGC *wrap;
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

inline void QAbstractGC::updateState(QPainterState *newState, bool updateGC)
{
    if (dirtyFlag || state!=newState)
	updateInternal(newState, updateGC);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractGC::Capability);
#endif // QABSTRACTGC_H

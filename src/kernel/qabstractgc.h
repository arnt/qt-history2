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

    virtual void drawLine(const QPoint &p1, const QPoint &ps) = 0;
    virtual void drawRect(const QRect &r) = 0;
    virtual void drawPoint(const QPoint &p) = 0;
    virtual void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1) = 0;
    virtual void drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor) = 0;
    virtual void drawRoundRect(const QRect &r, int xRnd, int yRnd) = 0;
    virtual void drawEllipse(const QRect &r) = 0;
    virtual void drawArc(const QRect &r, int a, int alen) = 0;
    virtual void drawPie(const QRect &r, int a, int alen) = 0;
    virtual void drawChord(const QRect &r, int a, int alen) = 0;
    virtual void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1) = 0;
    virtual void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1) = 0;
    virtual void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1) = 0;
    virtual void drawConvexPolygon(const QPointArray &, int index = 0, int npoints = -1) = 0;
#ifndef QT_NO_BEZIER
    virtual void drawCubicBezier(const QPointArray &, int index = 0) = 0;
#endif

    virtual void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr) = 0;
    virtual void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags) = 0;
    virtual void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim) = 0;

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
	// PostScript
	PostScript,

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

    virtual void updatePen(QPainterState *ps);
    virtual void updateBrush(QPainterState *ps);
    virtual void updateFont(QPainterState *ps);
    virtual void updateRasterOp(QPainterState *ps);
    virtual void updateBackground(QPainterState *ps);
    virtual void updateXForm(QPainterState *ps);
    virtual void updateClipRegion(QPainterState *ps);

    virtual void drawLine(const QPoint &p1, const QPoint &ps);
    virtual void drawRect(const QRect &r);
    virtual void drawPoint(const QPoint &p);
    virtual void drawPoints(const QPointArray &pa, int index, int npoints = -1);
    virtual void drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor);
    virtual void drawRoundRect(const QRect &r, int xRnd, int yRnd);
    virtual void drawEllipse(const QRect &r);
    virtual void drawArc(const QRect &r, int a, int alen);
    virtual void drawPie(const QRect &r, int a, int alen);
    virtual void drawChord(const QRect &r, int a, int alen);
    virtual void drawLineSegments(const QPointArray &, int index, int nlines = -1);
    virtual void drawPolyline(const QPointArray &pa, int index, int npoints = -1);
    virtual void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1);
    virtual void drawConvexPolygon(const QPointArray &, int index, int npoints = -1);
#ifndef QT_NO_BEZIER
    virtual void drawCubicBezier(const QPointArray &, int index);
#endif

    virtual void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr);
    virtual void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    virtual void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim);

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

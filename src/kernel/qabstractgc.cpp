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

#include "qabstractgc.h"
#include "qpainter_p.h"

QAbstractGC::QAbstractGC(GCCaps caps)
    : dirtyFlag(0),
      changeFlag(0),
      state(0),
      gccaps(caps)
{
    d_ptr = new QAbstractGCPrivate;
}

void QAbstractGC::updateInternal(QPainterState *s, bool updateGC)
{
    if (!s || !updateGC) {
	state = s;
	return;
    }

    // Same state, do minimal update...
    if (s==state) {
	if (testDirty(DirtyPen))
	    updatePen(s);
	if (testDirty(DirtyBrush))
	    updateBrush(s);
	if (testDirty(DirtyFont))
	    updateFont(s);
	if (testDirty(DirtyRasterOp))
	    updateRasterOp(s);
	if (testDirty(DirtyBackground))
	    updateBackground(s);
	if (testDirty(DirtyTransform))
	    updateXForm(s);
	if (testDirty(DirtyClip)) {
	    updateClipRegion(s);
	}
	// Same painter, restoring old state.
    } else if (state && s->painter == state->painter) {
	if ((changeFlag&DirtyPen)!=0)
	    updatePen(s);
	if ((changeFlag&DirtyBrush)!=0)
	    updateBrush(s);
	if ((changeFlag&DirtyFont)!=0)
	    updateFont(s);
	if ((changeFlag&DirtyRasterOp)!=0)
	    updateRasterOp(s);
	if ((changeFlag&DirtyBackground)!=0)
	    updateBackground(s);
	if ((changeFlag&DirtyTransform)!=0)
	    updateXForm(s);
	if ((changeFlag&DirtyClip)!=0 || (changeFlag&DirtyClip) != 0)
	    updateClipRegion(s);
	changeFlag = 0;
	// Different painter or state == 0 which is true for first time call
    } else {
	updatePen(s);
	updateBrush(s);
	updateFont(s);
	updateRasterOp(s);
	updateBackground(s);
	updateXForm(s);
	updateClipRegion(s);
	changeFlag = 0;
    }
    dirtyFlag = 0;
    state = s;
}



void QWrapperGC::updatePen(QPainterState *ps) { wrap->updatePen(ps); }
void QWrapperGC::updateBrush(QPainterState *ps) { wrap->updateBrush(ps); }
void QWrapperGC::updateFont(QPainterState *ps) { wrap->updateFont(ps); }
void QWrapperGC::updateRasterOp(QPainterState *ps) { wrap->updateRasterOp(ps); }
void QWrapperGC::updateBackground(QPainterState *ps) { wrap->updateBackground(ps); }
void QWrapperGC::updateXForm(QPainterState *ps) { wrap->updateXForm(ps); }
void QWrapperGC::updateClipRegion(QPainterState *ps) { wrap->updateClipRegion(ps); }


void QWrapperGC::drawLine(const QPoint &p1, const QPoint &p2) { wrap->drawLine(p1, p2); }
void QWrapperGC::drawRect(const QRect &r) { wrap->drawRect(r); }
void QWrapperGC::drawPoint(const QPoint &p) { wrap->drawPoint(p); }
void QWrapperGC::drawPoints(const QPointArray &pa, int index, int npoints)
{ wrap->drawPoints(pa, index, npoints); }
void QWrapperGC::drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor)
{ wrap->drawWinFocusRect(r, xorPaint, bgColor); }
void QWrapperGC::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{ wrap->drawRoundRect(r, xRnd, yRnd); }
void QWrapperGC::drawEllipse(const QRect &r)
{ wrap->drawEllipse(r); }
void QWrapperGC::drawArc(const QRect &r, int a, int alen)
{ wrap->drawArc(r, a, alen); }
void QWrapperGC::drawPie(const QRect &r, int a, int alen)
{ wrap->drawPie(r, a, alen); }
void QWrapperGC::drawChord(const QRect &r, int a, int alen)
{ wrap->drawChord(r, a, alen); }
void QWrapperGC::drawLineSegments(const QPointArray &a, int index, int nlines)
{ wrap->drawLineSegments(a, index, nlines); }
void QWrapperGC::drawPolyline(const QPointArray &pa, int index, int npoints)
{ wrap->drawPolyline(pa, index, npoints); }
void QWrapperGC::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
{ wrap->drawPolygon(pa, winding, index, npoints); }

void QWrapperGC::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{ wrap->drawConvexPolygon(a, index, npoints); }
#ifndef QT_NO_BEZIER
void QWrapperGC::drawCubicBezier(const QPointArray &a, int index)
{ wrap->drawCubicBezier(a, index); }
#endif

void QWrapperGC::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr)
{ wrap->drawPixmap(r, pm, sr); }
void QWrapperGC::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{ wrap->drawTextItem(p, ti, textflags); }
void QWrapperGC::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim)
{ wrap->drawTiledPixmap(r, pixmap, s, optim); }


/****************************************************************************
**
** Definition of QPaintEngine class.
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

#include "qpaintengine.h"
#include "qpainter_p.h"

QPaintEngine::QPaintEngine(GCCaps caps)
    : dirtyFlag(0),
      changeFlag(0),
      state(0),
      gccaps(caps)
{
    d_ptr = new QPaintEnginePrivate;
}

void QPaintEngine::updateInternal(QPainterState *s, bool updateGC)
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



void QWrapperPaintEngine::updatePen(QPainterState *ps) { wrap->updatePen(ps); }
void QWrapperPaintEngine::updateBrush(QPainterState *ps) { wrap->updateBrush(ps); }
void QWrapperPaintEngine::updateFont(QPainterState *ps) { wrap->updateFont(ps); }
void QWrapperPaintEngine::updateRasterOp(QPainterState *ps) { wrap->updateRasterOp(ps); }
void QWrapperPaintEngine::updateBackground(QPainterState *ps) { wrap->updateBackground(ps); }
void QWrapperPaintEngine::updateXForm(QPainterState *ps) { wrap->updateXForm(ps); }
void QWrapperPaintEngine::updateClipRegion(QPainterState *ps) { wrap->updateClipRegion(ps); }


void QWrapperPaintEngine::drawLine(const QPoint &p1, const QPoint &p2) { wrap->drawLine(p1, p2); }
void QWrapperPaintEngine::drawRect(const QRect &r) { wrap->drawRect(r); }
void QWrapperPaintEngine::drawPoint(const QPoint &p) { wrap->drawPoint(p); }
void QWrapperPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{ wrap->drawPoints(pa, index, npoints); }
void QWrapperPaintEngine::drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor)
{ wrap->drawWinFocusRect(r, xorPaint, bgColor); }
void QWrapperPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{ wrap->drawRoundRect(r, xRnd, yRnd); }
void QWrapperPaintEngine::drawEllipse(const QRect &r)
{ wrap->drawEllipse(r); }
void QWrapperPaintEngine::drawArc(const QRect &r, int a, int alen)
{ wrap->drawArc(r, a, alen); }
void QWrapperPaintEngine::drawPie(const QRect &r, int a, int alen)
{ wrap->drawPie(r, a, alen); }
void QWrapperPaintEngine::drawChord(const QRect &r, int a, int alen)
{ wrap->drawChord(r, a, alen); }
void QWrapperPaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{ wrap->drawLineSegments(a, index, nlines); }
void QWrapperPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{ wrap->drawPolyline(pa, index, npoints); }
void QWrapperPaintEngine::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
{ wrap->drawPolygon(pa, winding, index, npoints); }

void QWrapperPaintEngine::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{ wrap->drawConvexPolygon(a, index, npoints); }
#ifndef QT_NO_BEZIER
void QWrapperPaintEngine::drawCubicBezier(const QPointArray &a, int index)
{ wrap->drawCubicBezier(a, index); }
#endif

void QWrapperPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr)
{ wrap->drawPixmap(r, pm, sr); }
void QWrapperPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{ wrap->drawTextItem(p, ti, textflags); }
void QWrapperPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim)
{ wrap->drawTiledPixmap(r, pixmap, s, optim); }


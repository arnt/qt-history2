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
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpaintengine.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#ifdef Q_WS_WIN
#include "qpaintengine_win.h"
#endif
#include <private/qwidget_p.h>
#include "qpaintdevicemetrics.h"
#include <private/qtextlayout_p.h>
#include <private/qtextengine_p.h>
#include <private/qfontengine_p.h>


void qt_format_text(const QFont &font, const QRect &_r, int tf, const QString& str,
		    int len, QRect *brect, int tabstops, int* tabarray, int tabarraylen,
		    QPainter* painter);

/*!
    \class QPainter
*/


QPainter::QPainter()
{
    d = new QPainterPrivate;
    init();
}

QPainter::QPainter(const QPaintDevice *pd, bool unclipped)
{
    d = new QPainterPrivate;
    init();
    Q_ASSERT(pd != 0);
    begin(pd, unclipped);
}

QPainter::~QPainter()
{
    if (isActive())
	end();
    delete d;
}

void QPainter::init()
{
    d->state->painter = this;
}

QPaintDevice *QPainter::device() const
{
    return d->device;
}

bool QPainter::isActive() const
{
    if (d->engine) {
	return d->engine->isActive();
    }
    return false;
}

void QPainter::save()
{
    d->state = new QPainterState(d->states.back());
    d->states.push_back(d->state);
    if (isActive())
	d->engine->updateState(d->state, false);
}

void QPainter::restore()
{
    if (d->states.size()==0) {
	qWarning("QPainter::restore(), unbalanced save/restore");
	return;
    }

    QPainterState *tmp = d->state;
    d->states.pop_back();
    d->state = d->states.back();

    d->txinv = false;
    if (d->engine)
	d->engine->updateState(d->state);
    delete tmp;
}

bool QPainter::begin(const QPaintDevice *pd, bool unclipped)
{
    Q_ASSERT(pd);

    if (d->engine) {
	qWarning("QPainter::begin(): Painter is already active."
		 "\n\tYou must end() the painter before a second begin()" );
	return false;
    }

    switch (pd->devType()) {
    case QInternal::Widget:
	{
	    const QWidget *widget = static_cast<const QWidget *>(pd);
	    Q_ASSERT(widget);
	    d->state->font = widget->font();
	    d->state->pen = widget->palette().color(widget->foregroundRole());
	    d->state->bgBrush = widget->palette().brush(widget->backgroundRole());
	    const QWidget *wp = widget;
	    QPoint offset;
	    while(((QWidgetPrivate*)wp->d_ptr)->isBackgroundInherited()) {
		offset += wp->pos();
		wp = wp->parentWidget();
	    }
	    d->state->bgColor = widget->paletteBackgroundColor();
	    d->state->bgOrigin = -offset;
	    d->state->ww = d->state->vw = widget->width();
	    d->state->wh = d->state->vh = widget->height();
	    break;
	}
    case QInternal::Pixmap:
	{
	    const QPixmap *pm = static_cast<const QPixmap *>(pd);
	    Q_ASSERT(pm);
	    d->state->ww = d->state->vw = pm->width();
	    d->state->wh = d->state->vh = pm->height();
	    break;
	}
    }

    const QPaintDevice *rpd = redirected(pd, &d->redirection_offset);
    if (rpd) {
	pd = rpd;
    }

    d->device = const_cast<QPaintDevice*>(pd);
    d->engine = pd->engine();

    if (!d->engine) {
	qWarning("QPainter::begin(), paintdevice returned engine == 0, type: %d\n", pd->devType());
	return true;
    }

    if (!d->engine->begin(pd, d->state, unclipped)) {
	qWarning("QPainter::begin(), QPaintEngine::begin() returned false\n");
	return false;
    }

    if (!d->redirection_offset.isNull())
	updateXForm();

    Q_ASSERT(d->engine->isActive());
    d->engine->updateState(d->state);

    return true;
}

bool QPainter::end()
{
    if (!isActive()) {
	qWarning( "QPainter::end: Painter is not active, aborted");
	return false;
    }

    if (d->states.size()>1) {
	qWarning("QPainter::end(), painter ended with %d saved states",
		 d->states.size());
    }

    bool ended = d->engine->end();
    d->engine->updateState(0);
    if (ended)
	d->engine = 0;

    return ended;
}


QFontMetrics QPainter::fontMetrics() const
{
    // ### port properly
//     if ( pdev && pdev->devType() == QInternal::Picture )
// 	return QFontMetrics( cfont );

//     return QFontMetrics(this);
    return QFontMetrics(d->state->font);
}


QFontInfo QPainter::fontInfo() const
{
    // ### port properly
//     if ( pdev && pdev->devType() == QInternal::Picture )
// 	return QFontInfo( cfont );

//     return QFontInfo(this);
    return QFontInfo(d->state->font);
}


const QPoint &QPainter::brushOrigin() const
{
    return d->state->bgOrigin;
}

void QPainter::setBrushOrigin(int x, int y)
{
    // ### updateBrush in gc probably does not deal with offset.
    d->state->bgOrigin = QPoint(x, y);
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyBrush);
}

const QPoint &QPainter::backgroundOrigin() const
{
    // ### Port properly...
    return d->state->bgOrigin;
}


const QBrush &QPainter::background() const
{
    // ### port propertly...
    return d->state->bgBrush;
}


bool QPainter::hasClipping() const
{
    return d->state->clipEnabled;
}


void QPainter::setClipping( bool enable )
{
    Q_ASSERT(d->engine);
    d->state->clipEnabled = enable;
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyClip);
}


QRegion QPainter::clipRegion(CoordinateMode m) const
{
    if (m == CoordPainter && (d->state->VxF || d->state->WxF))
	return d->state->matrix.invert() * d->state->clipRegion;

    if (!d->redirection_offset.isNull()) {
	QRegion rgn(d->state->clipRegion);
	rgn.translate(d->redirection_offset);
	return rgn;
    }
    return d->state->clipRegion;
}

void QPainter::setClipRect( const QRect &rect, CoordinateMode mode ) // ### inline?
{
    Q_ASSERT(d->engine);
    setClipRegion(QRegion(rect), mode);
}

void QPainter::setClipRegion(const QRegion &r, CoordinateMode m)
{
    Q_ASSERT(d->engine);
    if (m == CoordPainter && (d->state->VxF || d->state->WxF)) {
	d->state->clipRegion = d->state->matrix * r;
    } else {
	d->state->clipRegion = r;
	if (!d->redirection_offset.isNull())
	    d->state->clipRegion.translate(-d->redirection_offset);
    }
    d->state->clipEnabled = true;
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyClip);
}



bool QPainter::hasViewXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->VxF;
#else
    return d->state->xlatex || d->state->xlatey;
#endif
}

bool QPainter::hasWorldXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->WxF;
#else
    return d->state->xlatex || d->state->xlatey;
#endif
}

void QPainter::setWindow(int x, int y, int w, int h)
{
    if (!isActive())
	qWarning("QPainter::setWindow: Will be reset by begin()");
    d->state->wx = x;
    d->state->wy = y;
    d->state->ww = w;
    d->state->wh = h;
    if (d->state->VxF)
	updateXForm();
    else
	setViewXForm(true);
}

QRect QPainter::window() const
{
    return QRect(d->state->wx, d->state->wy, d->state->ww, d->state->wh);
}

void QPainter::setViewport(int x, int y, int w, int h)
{
    if (!isActive())
	qWarning("QPainter::setViewport: Will be reset by begin()");
    d->state->vx = x;
    d->state->vy = y;
    d->state->vw = w;
    d->state->vh = h;
    if (d->state->VxF)
	updateXForm();
    else
	setViewXForm(true);
}

QRect QPainter::viewport() const
{
    return QRect(d->state->vx, d->state->vy, d->state->vw, d->state->vh);
}

void QPainter::setViewXForm(bool enable)
{
    if (!isActive())
	qWarning( "QPainter::setViewXForm: Will be reset by begin()" );
    if (!isActive() || enable == d->state->VxF)
	return;
    d->state->VxF = enable;
    updateXForm();
}

void QPainter::setWorldMatrix(const QWMatrix &wm, bool combine)
{
    if (!isActive())
	qWarning( "QPainter::setWorldMatrix: Will be reset by begin()" );
    if (combine)
	d->state->worldMatrix = wm * d->state->worldMatrix;			// combines
    else
	d->state->worldMatrix = wm;				// set new matrix
//     bool identity = d->state->worldMatrix.isIdentity();
//     if ( identity && pdev->devType() != QInternal::Picture )
// 	setWorldXForm( false );
//     else
    if (!d->state->WxF)
	setWorldXForm(true);
    else
	updateXForm();
}

const QWMatrix &QPainter::worldMatrix() const
{
    return d->state->worldMatrix;
}

void QPainter::setWorldXForm(bool enable)
{
    if (!isActive())
	qWarning( "QPainter::setWorldXForm: Will be reset by begin()" );
    if (!isActive() || enable == d->state->WxF)
	return;
    d->state->WxF = enable;
    updateXForm();
}

#include <private/qtextlayout_p.h>

#ifndef QT_NO_TRANSFORMATIONS
void QPainter::scale(double sx, double sy)
{
    QWMatrix m;
    m.scale( sx, sy );
    setWorldMatrix( m, true );
}

void QPainter::shear(double sh, double sv)
{
    QWMatrix m;
    m.shear( sv, sh );
    setWorldMatrix( m, true );
}

void QPainter::rotate(double a)
{
    QWMatrix m;
    m.rotate( a );
    setWorldMatrix( m, true );
}
#endif

void QPainter::resetXForm()
{
    if (!isActive())
	return;
    d->state->wx = d->state->wy = d->state->vx = d->state->vy = 0;			// default view origins
    d->state->ww = d->state->vw = d->device->metric(QPaintDeviceMetrics::PdmWidth);
    d->state->wh = d->state->vh = d->device->metric(QPaintDeviceMetrics::PdmHeight);
    d->state->worldMatrix = QWMatrix();
    setWorldXForm(false);
    setViewXForm(false);
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyTransform);
}

void QPainter::translate(double dx, double dy)
{
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix m;
    m.translate( dx, dy );
    setWorldMatrix( m, true );
#else
    xlatex += (int)dx;
    xlatey += (int)dy;
    d->state->VxF = (bool)xlatex || xlatey;
#endif
}

double QPainter::translationX() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->worldMatrix.dx();
#else
    return d->state->xlatex;
#endif
}

double QPainter::translationY() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->worldMatrix.dy();
#else
    return d->state->xlatey;
#endif
}

void QPainter::drawLine(const QPoint &p1, const QPoint &p2)
{
    if (!isActive())
	return;

    d->engine->updateState(d->state);

    if ((d->state->WxF || d->state->VxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	d->engine->drawLine(xForm(p1), xForm(p2));
	return;
    }

    d->engine->drawLine(p1, p2);
}

void QPainter::drawRect(const QRect &r)
{
    if (!isActive() || r.isEmpty())
	return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {
	    drawPolygon(QPointArray(rect));
	    return;
	}
	rect = xForm(rect);
    }

    d->engine->drawRect(rect);
}

void QPainter::drawPoint(const QPoint &p)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform))
	d->engine->drawPoint(xForm(p));

    d->engine->drawPoint(p);
}

void QPainter::drawPoints(const QPointArray &pa, int index, int npoints)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if (npoints < 0)
	npoints = pa.size() - index;
    if (index + npoints > (int)pa.size())
	npoints = pa.size() - index;
    if (!isActive() || npoints < 1 || index < 0)
	return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray a = xForm(pa, index, npoints);
	index = 0;
	npoints = a.size();
	d->engine->drawPoints(a, index, npoints);
	return;
    }

    d->engine->drawPoints(pa, index, npoints);
}


void QPainter::drawWinFocusRect(const QRect &r)
{
    if (!isActive() || r.isEmpty())
	return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {
	    QPen cpen = d->state->pen;
	    d->state->pen = QPen(black, 0, DotLine);
	    d->engine->updatePen(d->state);
	    drawPolygon(QPointArray(rect));
	    d->state->pen = cpen;
	    d->engine->updatePen(d->state);
	    return;
	}
	rect = xForm(rect);
    }

    d->engine->drawWinFocusRect(rect, true, color0);
}

void QPainter::drawWinFocusRect(const QRect &r, const QColor &bgColor)
{
    if (!isActive() || r.isEmpty())
	return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {
	    QPen cpen = d->state->pen;
	    d->state->pen = QPen(black, 0, DotLine);
	    d->engine->updatePen(d->state);
	    drawPolygon(QPointArray(rect));
	    d->state->pen = cpen;
	    d->engine->updatePen(d->state);
	    return;
	}
	rect = xForm(rect);
    }

    d->engine->drawWinFocusRect(rect, false, bgColor);
}


void QPainter::setBackgroundMode(BGMode mode)
{
    if (mode != TransparentMode && mode != OpaqueMode) {
	qWarning( "QPainter::setBackgroundMode: Invalid mode" );
	return;
    }
    d->state->bgMode = mode;
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyBackground);
}

QPainter::BGMode QPainter::backgroundMode() const
{
    return d->state->bgMode;
}


void QPainter::setPen(const QColor &color)
{
    d->state->pen = QPen(color, 0, Qt::SolidLine);
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyPen);
}

void QPainter::setPen(const QPen &pen)
{
    d->state->pen = pen;
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyPen);
}

void QPainter::setPen(PenStyle style)
{
    if (d->state->pen.style() == style)
	return;
    d->state->pen.setStyle(style);
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyPen);
}

const QPen &QPainter::pen() const
{
    return d->state->pen;
}


void QPainter::setBrush(const QBrush &brush)
{
    if (d->state->brush == brush)
	return;
    d->state->brush = brush;
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyBrush);
}


void QPainter::setBrush(BrushStyle style)
{
    // ### Missing optimization from qpainter.cpp
    d->state->brush = QBrush(Qt::black, style);
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyBrush);
}

const QBrush &QPainter::brush() const
{
    return d->state->brush;
}

void QPainter::setBackgroundColor(const QColor &color)
{
    d->state->bgColor = QBrush(color);
    if (d->engine && d->engine->isActive())
	d->engine->updateBackground(d->state);
}

const QColor &QPainter::backgroundColor() const
{
    return d->state->bgColor;
}

void QPainter::setRasterOp(RasterOp op)
{
    if ((uint)op > LastROP) {
	qWarning("QPainter::setRasterOp: Invalid ROP code");
	return;
    }
    d->state->rasterOp = op;
    if (d->engine)
	d->engine->updateRasterOp(d->state);
}

void QPainter::setFont(const QFont &font)
{
    d->state->font = font;
    if (d->engine)
	d->engine->setDirty(QPaintEngine::DirtyFont);
}

const QFont &QPainter::font() const
{
    return d->state->font;
}

void QPainter::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    if (!isActive())
	return;

    if(xRnd >= 100)                          // fix ranges
	xRnd = 99;
    if(yRnd >= 100)
	yRnd = 99;
    if(xRnd <= 0 || yRnd <= 0) {             // draw normal rectangle
	drawRect(r);
	return;
    }

    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {
	    int x = rect.x();
	    int y = rect.y();
	    int w = rect.width();
	    int h = rect.height();

	    w--; // ###?
	    h--; // ###?
	    int rxx = w*xRnd/200;
	    int ryy = h*yRnd/200;
	    // were there overflows?
	    if ( rxx < 0 )
		rxx = w/200*xRnd;
	    if ( ryy < 0 )
		ryy = h/200*yRnd;
	    int rxx2 = 2*rxx;
	    int ryy2 = 2*ryy;
	    QPointArray a[4];
	    a[0].makeArc( x, y, rxx2, ryy2, 1*16*90, 16*90, d->state->matrix );
	    a[1].makeArc( x, y+h-ryy2, rxx2, ryy2, 2*16*90, 16*90, d->state->matrix );
	    a[2].makeArc( x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*16*90, 16*90, d->state->matrix );
	    a[3].makeArc( x+w-rxx2, y, rxx2, ryy2, 0*16*90, 16*90, d->state->matrix );
	    // ### is there a better way to join QPointArrays?
	    QPointArray aa;
	    aa.resize( a[0].size() + a[1].size() + a[2].size() + a[3].size() );
	    uint j = 0;
	    for ( int k=0; k<4; k++ ) {
		for ( int i=0; i<a[k].size(); i++ ) {
		    aa.setPoint( j, a[k].point(i) );
		    j++;
		}
	    }
	    d->engine->drawPolygon(aa, false, 0, aa.size());
	    return;
	}
	rect = xForm(rect);
    }
    d->engine->drawRoundRect(rect, xRnd, yRnd);
}

void QPainter::drawEllipse(const QRect &r)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {
	    QPointArray a;
	    a.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), 0, 360*16, d->state->matrix);
	    d->engine->drawPolygon(a, false, 0, a.size());
	    return;
	}
	rect = xForm(rect);
    }

    d->engine->drawEllipse(rect);
}

void QPainter::drawArc(const QRect &r, int a, int alen)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {
	    QPointArray pa;
	    pa.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), a, alen, d->state->matrix);
	    d->engine->drawPolyline(pa, 0, pa.size());
	    return;
	}
	rect = xForm(rect);
    }
    d->engine->drawArc(rect, a, alen);
}

 void QPainter::drawPie(const QRect &r, int a, int alen)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if ( a > (360*16) ) {
	a = a % (360*16);
    } else if ( a < 0 ) {
	a = a % (360*16);
	if ( a < 0 ) a += (360*16);
    }

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {		// rotate/shear
	    // arc polyline
	    QPointArray pa;
	    pa.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), a, alen, d->state->matrix );
	    int n = pa.size();
	    QPoint p = xForm(QPoint(r.x()+r.width()/2, r.y()+r.height()/2));
	    pa.resize(n+2);
	    pa.setPoint(n, p);	// add legs
	    pa.setPoint(n+1, pa.at(0));
	    d->engine->drawPolygon(pa, false, 0, pa.size());
	    return;
	}
	rect = xForm(rect);
    }
    d->engine->drawPie(rect, a, alen);
}

void QPainter::drawChord(const QRect &r, int a, int alen)
{
    if ( !isActive() )
	return;
    d->engine->updateState(d->state);

    QRect rect = r.normalize();

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->state->txop == TxRotShear) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc(rect.x(), rect.y(), rect.width()-1, rect.height()-1, a, alen, d->state->matrix); // arc polygon
	    int n = pa.size();
	    pa.resize(n+1);
	    pa.setPoint(n, pa.at(0));		// connect endpoints
	    d->engine->drawPolygon(pa, false, 0, pa.size());
	    return;
	}
	rect = xForm(rect);
    }
    d->engine->drawChord(rect, a, alen);
}

void QPainter::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if ( nlines < 0 )
	nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > (int)a.size() )
	nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
	return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray pa = xForm(a, index, nlines*2);
	if ( pa.size() != a.size()) {
	    index  = 0;
	    nlines = pa.size()/2;
	}
	d->engine->drawLineSegments(pa, index, nlines);
	return;
    }

    d->engine->drawLineSegments(a, index, nlines);
}

void QPainter::drawPolyline(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray ar = xForm(a, index, npoints);
	d->engine->drawPolyline(ar, index, npoints);
	return;
    }

    d->engine->drawPolyline(a, index, npoints);
}

void QPainter::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray ar = xForm(a, index, npoints);
	d->engine->drawPolygon(ar, winding, index, npoints);
	return;
    }
    d->engine->drawPolygon(a, winding, index, npoints);
}

void QPainter::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray ar = xForm(a, index, npoints);
	d->engine->drawConvexPolygon(ar, index, npoints);
	return;
    }
    d->engine->drawConvexPolygon(a, index, npoints);
}

void QPainter::drawCubicBezier(const QPointArray &a, int index )
{
    if ( !isActive() )
	return;
    d->engine->updateState(d->state);

    if ( (int)a.size() - index < 4 ) {
	qWarning( "QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
		  "points" );
	return;
    }

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray pa = xForm(a);
	d->engine->drawCubicBezier(pa, index);
	return;
    }

    d->engine->drawCubicBezier(a, index);
}

void QPainter::drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh)
{
    if (!isActive() || pm.isNull())
	return;
    d->engine->updateState(d->state);

    if (sw < 0)
	sw = pm.width() - sx;
    if (sh < 0)
	sh = pm.height() - sy;

    // Sanity-check clipping
    if (sx < 0) {
	x -= sx;
	sw += sx;
	sx = 0;
    }

    if (sw + sx > pm.width())
	sw = pm.width() - sx;
    if (sy < 0) {
	y -= sy;
	sh += sy;
	sy = 0;
    }

    if (sh + sy > pm.height())
	sh = pm.height() - sy;

    if (sw <= 0 || sh <= 0)
	return;

    if ((d->state->VxF || d->state->WxF) && !d->engine->hasCapability(QPaintEngine::PixmapTransform)) {
	QWMatrix mat(d->state->matrix);
	mat = QPixmap::trueMatrix(mat, sw, sh);
	QPixmap pmx;
	if (sx == 0 && sy == 0 &&
	    sw == pm.width() && sh == pm.height()) {
	    pmx = pm;			// xform the whole pixmap
	} else {
	    pmx = QPixmap(sw, sh);		// xform subpixmap
	    bitBlt(&pmx, 0, 0, &pm, sx, sy, sw, sh);
	    if (pm.mask()) {
		QBitmap mask(sw, sh);
		bitBlt(&mask, 0, 0, pm.mask(), sx, sy, sw, sh);
		pmx.setMask(mask);
	    }
	}
	pmx = pmx.xForm(mat);
	if (pmx.isNull())			// xformed into nothing
	    return;
	if (!pmx.mask() && d->state->txop == TxRotShear) {
	    QBitmap bm_clip(sw, sh, 1);	// make full mask, xform it
	    bm_clip.fill(color1);
	    pmx.setMask(bm_clip.xForm(mat));
	}
	map(x, y, &x, &y);	// compute position of pixmap
	int dx, dy;
	mat.map( 0, 0, &dx, &dy );
	d->engine->drawPixmap(QRect(x-dx, y-dy, pmx.width(), pmx.height()), pmx,
			QRect(0, 0, pmx.width(), pmx.height()));
	return;
    }

    d->engine->drawPixmap(QRect(x, y, sw, sh), pm, QRect(sx, sy, sw, sh));
    return;
}


void QPainter::drawPixmap( const QRect &r, const QPixmap &pm )
{
    if (!isActive() || pm.isNull())
	return;
    d->engine->updateState(d->state);

    QPixmap pixmap = pm;

    if (!d->engine->hasCapability(QPaintEngine::PixmapTransform)) {
	int rw = r.width();
	int rh = r.height();
	int iw= pm.width();
	int ih = pm.height();
	if ( rw <= 0 || rh <= 0 || iw <= 0 || ih <= 0 )
	    return;
	bool scale = ( rw != iw || rh != ih );
	float scaleX = (float)rw/(float)iw;
	float scaleY = (float)rh/(float)ih;
	bool smooth = ( scaleX < 1.5 || scaleY < 1.5 );

	if ( scale ) {
#ifndef QT_NO_IMAGE_SMOOTHSCALE
# ifndef QT_NO_PIXMAP_TRANSFORMATION
	    if ( smooth )
# endif
	    {
		QImage i = pm.convertToImage();
		pixmap = QPixmap( i.smoothScale( rw, rh ) );
	    }
# ifndef QT_NO_PIXMAP_TRANSFORMATION
	    else
# endif
#endif
#ifndef QT_NO_PIXMAP_TRANSFORMATION
	    {
		pixmap = pm.xForm( QWMatrix( scaleX, 0, 0, scaleY, 0, 0 ) );
	    }
#endif
	}
    }
    // ##### probably wrong with world transform!
    d->engine->drawPixmap(r, pixmap, pixmap.rect());
}

void QPainter::drawImage(int x, int y, const QImage &,
			  int sx, int sy, int sw, int sh,
			  int conversionFlags)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}

void QPainter::drawImage(const QRect &, const QImage &)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}


void QPainter::drawText(int x, int y, const QString &str, TextDirection dir)
{
    if ( !isActive() )
        return;
    d->engine->updateState(d->state);

    if (str.isEmpty())
	return;
    int len = str.length();

    QTextLayout layout(str, d->state->pfont ? *d->state->pfont : d->state->font);
    QTextEngine *engine = layout.engine();

    engine->itemize(QTextEngine::SingleLine);

    if ( dir != Auto ) {
	int level = dir == RTL ? 1 : 0;
	for ( int i = engine->items.size()-1; i >= 0; i-- )
	    engine->items[i].analysis.bidiLevel = level;
    }

    QTextLine line = layout.createLine(0, 0, 0, INT_MAX);
    const QScriptLine &sl = engine->lines[0];

    int textFlags = 0;
    if ( d->state->font.underline() ) textFlags |= Qt::Underline;
    if ( d->state->font.overline() ) textFlags |= Qt::Overline;
    if ( d->state->font.strikeOut() ) textFlags |= Qt::StrikeOut;

    // ### call fill rect here...
#if defined(Q_WS_X11)
    extern void qt_draw_background( QPaintEngine *pe, int x, int y, int w,  int h );
    if (backgroundMode() == OpaqueMode)
 	qt_draw_background(d->engine, x, y-sl.ascent, sl.textWidth, sl.ascent+sl.descent+1);
#endif

    line.draw(this, x, y-sl.ascent);
}

void QPainter::drawText(const QRect &r, int flags, const QString &str, int len,
			 QRect *br)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if ( len < 0 )
	len = str.length();
    if ( len == 0 )				// empty string
	return;

    qt_format_text(font(), r, flags, str, len, br,
		   0, 0, 0, this);
}

void QPainter::drawTextItem(const QPoint &p, const QTextItem &ti, int textFlags)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);
    if (d->engine->hasCapability(QPaintEngine::CanRenderText)) {
	d->engine->drawTextItem(p, ti, textFlags);
	return;
    }

    if (d->engine->hasCapability(QPaintEngine::UsesFontEngine)) {
	bool useFontEngine = true;
	if (d->state->txop > TxTranslate) {
	    useFontEngine = false;
	    QFontEngine *fe = ti.fontEngine;
	    QFontEngine::FECaps fecaps = fe->capabilites();
	    if (d->state->txop == TxRotShear) {
		useFontEngine = (fecaps == QFontEngine::FullTransformations);
		if (!useFontEngine
		    && d->state->matrix.m11() == d->state->matrix.m22()
		    && d->state->matrix.m12() == -d->state->matrix.m21())
		    useFontEngine = (fecaps & QFontEngine::RotScale) == QFontEngine::RotScale;
	    } else if (d->state->txop == TxScale) {
		useFontEngine = (fecaps & QFontEngine::Scale);
	    }
	}
	if (useFontEngine) {
	    ti.fontEngine->draw(d->engine, p.x(),  p.y(), ti, textFlags);
	    return;
	}
    }

    // Fallback: rasterize into a pixmap and draw the pixmap
    // ### FIXME: this is slow

    QFontEngine *fe = ti.fontEngine;
    QPixmap pm(ti.width, ti.ascent+ti.descent);
    pm.fill(white);

    QPainter painter;
    painter.begin(&pm);
    painter.setPen(black);
    painter.drawTextItem(0, ti.ascent, ti, textFlags);
    painter.end();

    QImage img = pm;
    if (img.depth() != 32)
	img = img.convertDepth(32);
    img.setAlphaBuffer(true);
    int i = 0;
    while (i < img.height()) {
	uint *p = (uint *) img.scanLine(i);
	uint *end = p + img.width();
	while (p < end) {
	    *p = ((0xff - qGray(*p)) << 24) | (pen().color().rgb() & 0x00ffffff);
	    ++p;
	}
	++i;
    }

    pm = img;
    drawPixmap(p.x(), p.y() - ti.ascent, pm);
}

QRect QPainter::boundingRect(int x, int y, int w, int h, int flags, const QString &str, int len)
{
    QRect brect;
    if ( str.isEmpty() )
	brect.setRect( x,y, 0,0 );
    else
	drawText(QRect(x, y, w, h), flags | DontPrint, str, len, &brect);
    return brect;
}

// ################### make static or prefix with qt_

/* Internal, used by drawTiledPixmap */
void drawTile( QPaintEngine *gc, int x, int y, int w, int h,
	       const QPixmap &pixmap, int xOffset, int yOffset )
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while( yPos < y + h ) {
	drawH = pixmap.height() - yOff;    // Cropping first row
	if ( yPos + drawH > y + h )	   // Cropping last row
	    drawH = y + h - yPos;
	xPos = x;
	xOff = xOffset;
	while( xPos < x + w ) {
	    drawW = pixmap.width() - xOff; // Cropping first column
	    if ( xPos + drawW > x + w )	   // Cropping last column
		drawW = x + w - xPos;
	    gc->drawPixmap(QRect(xPos, yPos, drawW, drawH), pixmap, QRect(xOff, yOff, drawW, drawH));
	    xPos += drawW;
	    xOff = 0;
	}
	yPos += drawH;
	yOff = 0;
    }
}

void QPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx, int sy)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    int sw = pixmap.width();
    int sh = pixmap.height();
    if (!sw || !sh )
	return;
    if ( sx < 0 )
	sx = sw - -sx % sw;
    else
	sx = sx % sw;
    if ( sy < 0 )
	sy = sh - -sy % sh;
    else
	sy = sy % sh;

    if ((d->state->VxF || d->state->WxF)
	&& !d->engine->hasCapability(QPaintEngine::PixmapTransform)) {
	QPixmap pm(w, h);
	QPainter p(&pm);
	// Recursive call ok, since the pixmap is not transformed...
	p.drawTiledPixmap(0, 0, w, h, pixmap, sx, sy);
	p.end();
	drawPixmap(x, y, pm);
	return;
    }

    bool optim = (pixmap.mask() && pixmap.depth() > 1 && d->state->txop <= TxTranslate);
    d->engine->drawTiledPixmap(QRect(x, y, w, h), pixmap, QPoint(sx, sy), optim);
}

void QPainter::drawPicture(int x, int y, const QPicture &p)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    qWarning("QPainter::drawPicture() not implemetned yet...\n");
}

void QPainter::eraseRect(int x, int y, int w, int h)
{
    if (!isActive())
	return;
    d->engine->updateState(d->state);

    if (d->state->bgBrush.pixmap())
	drawTiledPixmap(QRect(x, y, w, h), *d->state->bgBrush.pixmap(), -d->state->bgOrigin);
    else
	fillRect(x, y, w, h, d->state->bgBrush);
}

void QPainter::fillRect(int x, int y, int w, int h, const QBrush &brush)
{
    QPen   oldPen   = pen();			// save pen
    QBrush oldBrush = this->brush();		// save brush
    setPen( NoPen );
    setBrush( brush );
    drawRect( x, y, w, h );			// draw filled rect
    setBrush( oldBrush );			// restore brush
    setPen( oldPen );				// restore pen
}


void QPainter::updateXForm()
{
    QWMatrix m;
    if (d->state->VxF) {
	double scaleW = (double)d->state->vw/(double)d->state->ww;
	double scaleH = (double)d->state->vh/(double)d->state->wh;
	m.setMatrix(scaleW, 0, 0, scaleH, d->state->vx - d->state->wx*scaleW, d->state->vy - d->state->wy*scaleH );
    }
    if (d->state->WxF) {
	if (d->state->VxF)
	    m = d->state->worldMatrix * m;
	else
	    m = d->state->worldMatrix;
    }
    d->state->matrix = m;


    d->txinv = false;				// no inverted matrix
    d->state->txop  = TxNone;
    if (d->state->matrix.m12()==0.0 && d->state->matrix.m21()==0.0
	&& d->state->matrix.m11() >= 0.0 && d->state->matrix.m22() >= 0.0 ) {
	if (d->state->matrix.m11()==1.0 && d->state->matrix.m22()==1.0 ) {
	    if (d->state->matrix.dx()!=0.0 || d->state->matrix.dy()!=0.0 )
		d->state->txop = TxTranslate;
	} else {
	    d->state->txop = TxScale;
	}
    } else {
	d->state->txop = TxRotShear;
    }
    if (!d->redirection_offset.isNull()) {
	d->state->txop |= TxTranslate;
	d->state->WxF = true;
	// We want to translate in dev space so we do the adding of the redirection
	// offset manually.
	d->state->matrix = QWMatrix(d->state->matrix.m11(), d->state->matrix.m12(),
			      d->state->matrix.m21(), d->state->matrix.m22(),
			      d->state->matrix.dx()-d->redirection_offset.x(),
			      d->state->matrix.dy()-d->redirection_offset.y());
    }
    d->engine->setDirty(QPaintEngine::DirtyTransform);
//     printf("VxF=%d, WxF=%d\n", d->state->VxF, d->state->WxF);
//     printf("Using matrix: %f, %f, %f, %f, %f, %f\n",
// 	   d->state->matrix.m11(),
// 	   d->state->matrix.m12(),
// 	   d->state->matrix.m21(),
// 	   d->state->matrix.m22(),
// 	   d->state->matrix.dx(),
// 	   d->state->matrix.dy() );
}

void QPainter::updateInvXForm()
{
    Q_ASSERT(d->txinv == false);
    d->txinv = true;				// creating inverted matrix
    bool invertible;
    QWMatrix m;
    if (d->state->VxF) {
	m.translate(d->state->vx, d->state->vy);
	m.scale(1.0*d->state->vw/d->state->ww, 1.0*d->state->vh/d->state->wh);
	m.translate(-d->state->wx, -d->state->wy);
    }
    if (d->state->WxF) {
	if (d->state->VxF)
	    m = d->state->worldMatrix * m;
	else
	    m = d->state->worldMatrix;
    }
    d->invMatrix = m.invert( &invertible );		// invert matrix
}


QPoint QPainter::xForm(const QPoint &p) const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->state->txop) {
    case TxNone:
	return p;
    case TxTranslate:
	return QPoint(qRound(p.x() + d->state->matrix.dx()), qRound(p.y() + d->state->matrix.dy()));
    case TxScale:
	return QPoint(qRound(d->state->matrix.m11()*p.x() + d->state->matrix.dx()),
		      qRound(d->state->matrix.m22()*p.y() + d->state->matrix.dy()));
    default:
	return QPoint(qRound(d->state->matrix.m11()*p.x() + d->state->matrix.m21()*p.y()+d->state->matrix.dx()),
		      qRound(d->state->matrix.m12()*p.x() + d->state->matrix.m22()*p.y()+d->state->matrix.dy()));
    }
#else
    return QPoint(p.x() + d->state->xlatex, p.y() + d->state->xlatey);
#endif
}

QRect QPainter::xForm(const QRect &r)	const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->state->txop) {
    case TxNone:
	return r;
    case TxTranslate: {
	QRect rect(r);
	rect.moveBy(qRound(d->state->matrix.dx()), qRound(d->state->matrix.dy()));
	return rect;
    }
    case TxScale:
	return QRect(qRound(d->state->matrix.m11()*r.x() + d->state->matrix.dx()),
		     qRound(d->state->matrix.m22()*r.y() + d->state->matrix.dy()),
		     qRound(d->state->matrix.m11()*r.width()),
		     qRound(d->state->matrix.m22()*r.height()));
    case TxRotShear:
	return d->state->matrix.mapRect(r);
    }
    return r;
#else
    return QRect( r.x()+d->state->xlatex, r.y()+d->state->xlatey, r.width(), r.height() );
#endif
}

QPointArray QPainter::xForm(const QPointArray &a) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if ( d->state->txop == TxNone )
	return a;
    return d->state->matrix * a;
#else
    QPointArray p(a);
    p.translate( d->state->xlatex, d->state->xlatey );
    return p;
#endif
}

QPointArray QPainter::xForm( const QPointArray &av, int index, int npoints ) const
{
    int lastPoint = npoints < 0 ? av.size() : index+npoints;
    QPointArray a(lastPoint-index);
    memcpy( a.data(), av.data()+index, (lastPoint-index)*sizeof( QPoint ) );
#ifndef QT_NO_TRANSFORMATIONS
    return d->state->matrix*a;
#else
    a.translate( d->state->xlatex, d->state->xlatey );
    return a;
#endif
}

QPoint QPainter::xFormDev( const QPoint &p ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if(d->state->txop == TxNone)
	return p;
    if (!d->txinv) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    return QPoint(qRound(d->invMatrix.m11()*p.x() + d->invMatrix.m21()*p.y() + d->invMatrix.dx()),
		  qRound(d->invMatrix.m12()*p.x() + d->invMatrix.m22()*p.y() + d->invMatrix.dy()));
#else
    return QPoint(p.x() - xlatex, p.y() - xlatey);
#endif
}

QRect QPainter::xFormDev( const QRect &r )  const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == TxNone)
	return r;
    if (!d->txinv) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    if (d->state->txop == TxRotShear) {			// rotation/shear
	return d->invMatrix.mapRect(r);
    } else {
	return QRect(qRound(d->invMatrix.m11()*r.x() + d->invMatrix.dx()),
		     qRound(d->invMatrix.m22()*r.y() + d->invMatrix.dy()),
		     qRound(d->invMatrix.m11()*r.width()),
		     qRound(d->invMatrix.m22()*r.height()));
    }
#else
    return QRect( r.x()-d->state->xlatex, r.y()-d->state->xlatey, r.width(), r.height() );
#endif
}

QPointArray QPainter::xFormDev( const QPointArray &a ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == TxNone)
	return a;
    if ( !d->txinv ) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    return d->invMatrix * a;
#else
    QPointArray p(a);
    p.translate( -d->state->xlatex, -d->state->xlatey );
    return p;
#endif

}

QPointArray QPainter::xFormDev( const QPointArray &ad, int index, int npoints ) const
{
    int lastPoint = npoints < 0 ? ad.size() : index+npoints;
    QPointArray a( lastPoint-index );
    memcpy( a.data(), ad.data()+index, (lastPoint-index)*sizeof( QPoint ) );
#ifndef QT_NO_TRANSFORMATIONS
    if (d->state->txop == TxNone)
	return a;
    if (!d->txinv) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    return d->invMatrix * a;
#else
    a.translate( -d->state->xlatex, -d->state->xlatey );
    return a;
#endif
}

#if defined Q_WS_WIN
HDC QPainter::handle() const
#else
Qt::HANDLE QPainter::handle() const
#endif
{
    Q_ASSERT(isActive());
    Q_ASSERT(d->engine);
    return d->engine->handle();
}

double QPainter::m11() const { return d->state->matrix.m11(); }
double QPainter::m12() const { return d->state->matrix.m12(); }
double QPainter::m21() const { return d->state->matrix.m21(); }
double QPainter::m22() const { return d->state->matrix.m22(); }
double QPainter::dx() const { return d->state->matrix.dx(); }
double QPainter::dy() const { return d->state->matrix.dy(); }
double QPainter::im11() const { return d->invMatrix.m11(); }
double QPainter::im12() const { return d->invMatrix.m12(); }
double QPainter::im21() const { return d->invMatrix.m21(); }
double QPainter::im22() const { return d->invMatrix.m22(); }
double QPainter::idx() const { return d->invMatrix.dx(); }
double QPainter::idy() const { return d->invMatrix.dy(); }

struct QPaintDeviceRedirection
{
    QPaintDeviceRedirection() : device(0), replacement(0) {}
    QPaintDeviceRedirection(const QPaintDevice *device, const QPaintDevice *replacement,
			    const QPoint& offset)
	: device(device), replacement(replacement), offset(offset) { }
    const QPaintDevice *device;
    const QPaintDevice *replacement;
    QPoint offset;
    bool operator==(const QPaintDevice *pdev) const { return device == pdev; }
    Q_DUMMY_COMPARISON_OPERATOR(QPaintDeviceRedirection)
};

static QList<QPaintDeviceRedirection> redirections;


/*!
  \internal

    Redirects all paint commands for a paint device, \a device, to
    another paint device, \a replacement. The optional point \a offset
    defines an offset within the replaced device. After painting you
    must call restoreRedirected().

    In general, you'll probably find calling QPixmap::grabWidget() or
    QPixmap::grabWindow() is an easier solution.

    \sa redirected()
*/
void QPainter::setRedirected(const QPaintDevice *device,
			      const QPaintDevice *replacement,
			      const QPoint &offset)
{
    Q_ASSERT(device != 0);
    redirections.ensure_constructed();
    redirections += QPaintDeviceRedirection(device, replacement, offset);
}


/*!\internal

  Restores the previous redirection for \a device after a call to
  setRedirected().

  \sa redirected()
 */
void QPainter::restoreRedirected(const QPaintDevice *device)
{
    Q_ASSERT(device != 0);
    redirections.ensure_constructed();
    for (int i = redirections.size()-1; i >= 0; --i)
	if (redirections.at(i) == device ) {
	    redirections.removeAt(i);
	    return;
	}
}


/*!
    \internal

    Returns the replacement for \a device. The optional out parameter
    \a offset returns return the offset within the replaced device.
*/
const QPaintDevice *QPainter::redirected(const QPaintDevice *device, QPoint *offset)
{
    Q_ASSERT(device != 0);
    redirections.ensure_constructed();
    for (int i = redirections.size()-1; i >= 0; --i)
	if (redirections.at(i) == device ) {
	    if (offset)
		*offset = redirections.at(i).offset;
	    return redirections.at(i).replacement;
	}
    return 0;
}


#define QChar_linesep QChar(0x2028U)

void qt_format_text( const QFont& font, const QRect &_r,
		     int tf, const QString& str, int len, QRect *brect,
		     int tabstops, int* tabarray, int tabarraylen,
		     QPainter* painter )
{
    // we need to copy r here to protect against the case (&r == brect).
    QRect r( _r );

    bool dontclip  = (tf & Qt::DontClip)  == Qt::DontClip;
    bool wordbreak  = (tf & Qt::WordBreak)  == Qt::WordBreak;
    bool singleline = (tf & Qt::SingleLine) == Qt::SingleLine;
    bool showprefix = (tf & Qt::ShowPrefix) == Qt::ShowPrefix;
    bool noaccel = ( tf & Qt::NoAccel ) == Qt::NoAccel;

    bool isRightToLeft = str.isRightToLeft();
    if ( ( tf & Qt::AlignHorizontal_Mask ) == Qt::AlignAuto )
	tf |= isRightToLeft ? Qt::AlignRight : Qt::AlignLeft;

    bool expandtabs = ( (tf & Qt::ExpandTabs) &&
			( ( (tf & Qt::AlignLeft) && !isRightToLeft ) ||
			  ( (tf & Qt::AlignRight) && isRightToLeft ) ) );

    if ( !painter )
	tf |= Qt::DontPrint;

    int maxUnderlines = 0;
    int numUnderlines = 0;
    int underlinePositionStack[32];
    int *underlinePositions = underlinePositionStack;

    // ### port properly
    // QFont fnt(painter ? (painter->pfont ? *painter->pfont : painter->cfont) : font);
    QFont fnt(painter ? painter->font() : font);
    QFontMetrics fm( fnt );

    QString text = str;
    // compatible behaviour to the old implementation. Replace
    // tabs by spaces
    QChar *chr = text.data();
    const QChar *end = chr + len;
    bool haveLineSep = false;
    while ( chr != end ) {
	if ( *chr == '\r' || ( singleline && *chr == '\n' ) ) {
	    *chr = ' ';
	} else if ( *chr == '\n' ) {
	    *chr = QChar_linesep;
	    haveLineSep = true;
	} else if ( *chr == '&' ) {
	    ++maxUnderlines;
	}
	++chr;
    }
    if ( !expandtabs ) {
	chr = (QChar*)text.unicode();
	while ( chr != end ) {
	    if ( *chr == '\t' )
		*chr = ' ';
	    ++chr;
	}
    } else if (!tabarraylen && !tabstops) {
	tabstops = fm.width('x')*8;
    }

    if ( noaccel || showprefix ) {
	if ( maxUnderlines > 32 )
	    underlinePositions = new int[maxUnderlines];
	QChar *cout = (QChar*)text.unicode();
	QChar *cin = cout;
	int l = len;
	while ( l ) {
	    if ( *cin == '&' ) {
		++cin;
		--l;
		if ( !l )
		    break;
		if ( *cin != '&' )
		    underlinePositions[numUnderlines++] = cout - text.unicode();
	    }
	    *cout = *cin;
	    ++cout;
	    ++cin;
	    --l;
	}
	int newlen = cout - text.unicode();
	if ( newlen != text.length())
	    text.resize( newlen );
    }

    // no need to do extra work for underlines if we don't paint
    if ( tf & Qt::DontPrint )
	numUnderlines = 0;

    underlinePositions[numUnderlines] = -1;
    int height = 0;
    int width = 0;

    QTextLayout textLayout( text, fnt );
    textLayout.engine()->underlinePositions = underlinePositions;

    if ( text.isEmpty() ) {
	height = fm.height();
	width = 0;
	tf |= QPainter::DontPrint;
    } else {
	int lineWidth = wordbreak ? qMax(0, r.width()) : INT_MAX;
	if(!wordbreak)
	    tf |= Qt::IncludeTrailingSpaces;
	textLayout.beginLayout((tf & Qt::DontPrint) ? QTextLayout::NoBidi : QTextLayout::SingleLine );

	int leading = fm.leading();
	height = -leading;

	int from = 0;
	while (from < text.length()) {
	    height += leading;
	    QTextLine l = textLayout.createLine(from, height, 0, lineWidth);
	    height += l.ascent() + l.descent();
	    from += l.length();
	    width = qMax(width, l.textWidth());
	}
    }

    int yoff = 0;
    int xoff = 0;
    if ( tf & Qt::AlignBottom )
	yoff = r.height() - height;
    else if ( tf & Qt::AlignVCenter )
	yoff = (r.height() - height)/2;
    if ( tf & Qt::AlignRight )
	xoff = r.width() - width;
    else if ( tf & Qt::AlignHCenter )
	xoff = (r.width() - width)/2;
    if ( brect ) {
	*brect = QRect( r.x() + xoff, r.y() + yoff, width, height );
    }

    if (!(tf & QPainter::DontPrint)) {
	bool restoreClipping = false;
	bool painterHasClip = false;
	QRegion painterClipRegion;
	if ( !dontclip ) {
	    QRegion reg(r);
#ifdef QT_NO_TRANSFORMATIONS
	    reg.translate( painter->d->state->xlatex, painter->d->state->xlatey );
#endif
 	    if (painter->hasClipping())
 		reg &= painter->clipRegion(QPainter::CoordPainter);

	    painterHasClip = painter->hasClipping();
	    painterClipRegion = painter->clipRegion(QPainter::CoordPainter);
	    restoreClipping = true;
	    painter->setClipRegion(reg, QPainter::CoordPainter);
	} else {
	    if ( painter->hasClipping() ){
		painterHasClip = painter->hasClipping();
		painterClipRegion = painter->clipRegion(QPainter::CoordPainter);
		restoreClipping = true;
		painter->setClipping( false );
	    }
	}

	int _tf = 0;
	if (fnt.underline()) _tf |= Qt::Underline;
	if (fnt.overline()) _tf |= Qt::Overline;
	if (fnt.strikeOut()) _tf |= Qt::StrikeOut;

	for ( int i = 0; i < textLayout.numLines(); i++ ) {
	    QTextLine line = textLayout.lineAt(i);

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
	    extern void qt_draw_background( QPaintEngine *pe, int x, int y, int w,  int h );

 	    if (painter->backgroundMode() == Qt::OpaqueMode)
 		qt_draw_background(painter->d->engine, r.x() + line.x() + xoff, r.y() + yoff + line.y(),
				   line.width(), line.ascent() + line.descent() + 1);
#endif
	    line.draw(painter, r.x() + xoff + line.x(), r.y() + yoff);
	}

	if ( restoreClipping ) {
	    painter->setClipRegion(painterClipRegion, QPainter::CoordPainter);
	    painter->setClipping( painterHasClip );
	}
    }

    if ( underlinePositions != underlinePositionStack )
	delete [] underlinePositions;
}


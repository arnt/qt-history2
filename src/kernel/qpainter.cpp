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
#include "qpaintdevicemetrics.h"
#include "qtextlayout_p.h"
#include "qtextengine_p.h"
#include "qfontengine_p.h"
#include "qwidget_p.h"

#define ds d->state
#define dengine d->engine

void qt_format_text(const QFont &font, const QRect &_r, int tf, const QString& str,
		    int len, QRect *brect, int tabstops, int* tabarray, int tabarraylen,
		    QPainter* painter);

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
    ds->painter = this;
}

QPaintDevice *QPainter::device() const
{
    return d->device;
}

bool QPainter::isActive() const
{
    if (dengine) {
	return dengine->isActive();
    }
    return false;
}

void QPainter::save()
{
    ds = new QPainterState(d->states.back());
    d->states.push_back(ds);
    if (isActive())
	dengine->updateState(ds, false);
}

void QPainter::restore()
{
    if (d->states.size()==0) {
	qWarning("QPainter::restore(), unbalanced save/restore");
	return;
    }

    QPainterState *tmp = ds;
    d->states.pop_back();
    ds = d->states.back();

    if (dengine) {
	if (dengine->changeFlag & QPaintEngine::DirtyTransform)
	    updateXForm();
	dengine->updateState(ds);
    }
    delete tmp;
}

bool QPainter::begin(const QPaintDevice *pd, bool unclipped)
{
    if (dengine) {
	qWarning("QPainter::begin(): Painter is already active."
		 "\n\tYou must end() the painter before a second begin()" );
	return false;
    }

    const QPaintDevice *rpd = redirected(pd, &d->redirection_offset);
    if (rpd) {
	pd = rpd;
    }

    d->device = const_cast<QPaintDevice*>(pd);

    Q_ASSERT(pd != 0);
    dengine = pd->engine();

    if (!dengine) {
	qWarning("QPainter::begin(), paintdevice returned gc == 0, type: %d\n", pd->devType());
	return true;
    }

    switch (pd->devType()) {
    case QInternal::Widget:
	{
	    const QWidget *widget = static_cast<const QWidget *>(pd);
	    Q_ASSERT(widget);
	    ds->font = widget->font();
	    ds->pen = widget->palette().color(widget->foregroundRole());
	    ds->bgBrush = widget->palette().brush(widget->backgroundRole());
	    const QWidget *wp = widget;
	    QPoint offset;
	    while(((QWidgetPrivate*)wp->d_ptr)->isBackgroundInherited()) {
		offset += wp->pos();
		wp = wp->parentWidget();
	    }
	    ds->bgColor = widget->paletteBackgroundColor();
	    ds->bgOrigin = -offset;
	    ds->ww = ds->vw = widget->width();
	    ds->wh = ds->vh = widget->height();
	    break;
	}
    case QInternal::Pixmap:
	{
	    const QPixmap *pm = static_cast<const QPixmap *>(pd);
	    Q_ASSERT(pm);
	    ds->ww = ds->vw = pm->width();
	    ds->wh = ds->vh = pm->height();
	    break;
	}
    }

    if (!dengine->begin(pd, ds, unclipped)) {
	qWarning("QPainter::begin(), gc::begin() returned false\n");
	return false;
    }

    if (!d->redirection_offset.isNull())
	updateXForm();

    Q_ASSERT(dengine->isActive());
    dengine->updateState(ds);

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

    bool ended = dengine->end();
    dengine->updateState(0);
    if (ended)
	dengine = 0;

    return ended;
}


QFontMetrics QPainter::fontMetrics() const
{
    // ### port properly
//     if ( pdev && pdev->devType() == QInternal::Picture )
// 	return QFontMetrics( cfont );

//     return QFontMetrics(this);
    return QFontMetrics(ds->font);
}


QFontInfo QPainter::fontInfo() const
{
    // ### port properly
//     if ( pdev && pdev->devType() == QInternal::Picture )
// 	return QFontInfo( cfont );

//     return QFontInfo(this);
    return QFontInfo(ds->font);
}


const QPoint &QPainter::brushOrigin() const
{
    return ds->bgOrigin;
}

void QPainter::setBrushOrigin(int x, int y)
{
    // ### updateBrush in gc probably does not deal with offset.
    ds->bgOrigin = QPoint(x, y);
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyBrush);
}

const QPoint &QPainter::backgroundOrigin() const
{
    // ### Port properly...
    return ds->bgOrigin;
}


const QBrush &QPainter::background() const
{
    // ### port propertly...
    return ds->bgBrush;
}


bool QPainter::hasClipping() const
{
    return ds->clipEnabled;
}


void QPainter::setClipping( bool enable )
{
    Q_ASSERT(dengine);
    ds->clipEnabled = enable;
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyClip);
}


QRegion QPainter::clipRegion(CoordinateMode m) const
{
    if (m == CoordPainter && (ds->VxF || ds->WxF))
	return ds->matrix.invert() * ds->clipRegion;

    if (!d->redirection_offset.isNull()) {
	QRegion rgn(ds->clipRegion);
	rgn.translate(d->redirection_offset);
	return rgn;
    }
    return ds->clipRegion;
}

void QPainter::setClipRect( const QRect &rect, CoordinateMode mode ) // ### inline?
{
    Q_ASSERT(dengine);
    setClipRegion(QRegion(rect), mode);
}

void QPainter::setClipRegion(const QRegion &r, CoordinateMode m)
{
    Q_ASSERT(dengine);
    if (m == CoordPainter && (ds->VxF || ds->WxF)) {
	ds->clipRegion = ds->matrix * r;
    } else {
	ds->clipRegion = r;
	if (!d->redirection_offset.isNull())
	    ds->clipRegion.translate(-d->redirection_offset);
    }
    ds->clipEnabled = true;
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyClip);
}



bool QPainter::hasViewXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return ds->VxF;
#else
    return ds->xlatex || ds->xlatey;
#endif
}

bool QPainter::hasWorldXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return ds->WxF;
#else
    return ds->xlatex || ds->xlatey;
#endif
}

void QPainter::setWindow(int x, int y, int w, int h)
{
    if (!isActive())
	qWarning("QPainter::setWindow: Will be reset by begin()");
    ds->wx = x;
    ds->wy = y;
    ds->ww = w;
    ds->wh = h;
    if (ds->VxF)
	updateXForm();
    else
	setViewXForm(true);
}

QRect QPainter::window() const
{
    return QRect(ds->wx, ds->wy, ds->ww, ds->wh);
}

void QPainter::setViewport(int x, int y, int w, int h)
{
    if (!isActive())
	qWarning("QPainter::setViewport: Will be reset by begin()");
    ds->vx = x;
    ds->vy = y;
    ds->vw = w;
    ds->vh = h;
    if (ds->VxF)
	updateXForm();
    else
	setViewXForm(true);
}

QRect QPainter::viewport() const
{
    return QRect(ds->vx, ds->vy, ds->vw, ds->vh);
}

void QPainter::setViewXForm(bool enable)
{
    if (!isActive())
	qWarning( "QPainter::setViewXForm: Will be reset by begin()" );
    if (!isActive() || enable == ds->VxF)
	return;
    ds->VxF = enable;
    updateXForm();
}

void QPainter::setWorldMatrix(const QWMatrix &wm, bool combine)
{
    if (!isActive())
	qWarning( "QPainter::setWorldMatrix: Will be reset by begin()" );
    if (combine)
	ds->worldMatrix = wm * ds->worldMatrix;			// combines
    else
	ds->worldMatrix = wm;				// set new matrix
//     bool identity = ds->worldMatrix.isIdentity();
//     if ( identity && pdev->devType() != QInternal::Picture )
// 	setWorldXForm( false );
//     else
    if (!ds->WxF)
	setWorldXForm(true);
    else
	updateXForm();
}

const QWMatrix &QPainter::worldMatrix() const
{
    return ds->worldMatrix;
}

void QPainter::setWorldXForm(bool enable)
{
    if (!isActive())
	qWarning( "QPainter::setWorldXForm: Will be reset by begin()" );
    if (!isActive() || enable == ds->WxF)
	return;
    ds->WxF = enable;
    updateXForm();
}

#include "qtextlayout_p.h"

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
    ds->wx = ds->wy = ds->vx = ds->vy = 0;			// default view origins
    ds->ww = ds->vw = d->device->metric(QPaintDeviceMetrics::PdmWidth);
    ds->wh = ds->vh = d->device->metric(QPaintDeviceMetrics::PdmHeight);
    ds->worldMatrix = QWMatrix();
    setWorldXForm(false);
    setViewXForm(false);
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyTransform);
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
    ds->VxF = (bool)xlatex || xlatey;
#endif
}

double QPainter::translationX() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return ds->worldMatrix.dx();
#else
    return ds->xlatex;
#endif
}

double QPainter::translationY() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return ds->worldMatrix.dy();
#else
    return ds->xlatey;
#endif
}

void QPainter::drawLine(const QPoint &p1, const QPoint &p2)
{
    if (!isActive())
	return;

    dengine->updateState(ds);

    if ((ds->WxF || ds->VxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	dengine->drawLine(xForm(p1), xForm(p2));
	return;
    }

    dengine->drawLine(p1, p2);
}

void QPainter::drawRect(const QRect &r)
{
    if (!isActive() || r.isEmpty())
	return;
    dengine->updateState(ds);

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {
	    drawPolygon(QPointArray(rect));
	    return;
	}
	rect = xForm(rect);
    }

    dengine->drawRect(rect);
}

void QPainter::drawPoint(const QPoint &p)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform))
	dengine->drawPoint(xForm(p));

    dengine->drawPoint(p);
}

void QPainter::drawPoints(const QPointArray &pa, int index, int npoints)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if (npoints < 0)
	npoints = pa.size() - index;
    if (index + npoints > (int)pa.size())
	npoints = pa.size() - index;
    if (!isActive() || npoints < 1 || index < 0)
	return;

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray a = xForm(pa, index, npoints);
	index = 0;
	npoints = a.size();
	dengine->drawPoints(a, index, npoints);
	return;
    }

    dengine->drawPoints(pa, index, npoints);
}


void QPainter::drawWinFocusRect(const QRect &r)
{
    if (!isActive() || r.isEmpty())
	return;
    dengine->updateState(ds);

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {
	    QPen cpen = ds->pen;
	    ds->pen = QPen(black, 0, DotLine);
	    dengine->updatePen(ds);
	    drawPolygon(QPointArray(rect));
	    ds->pen = cpen;
	    dengine->updatePen(ds);
	    return;
	}
	rect = xForm(rect);
    }

    dengine->drawWinFocusRect(rect, true, color0);
}

void QPainter::drawWinFocusRect(const QRect &r, const QColor &bgColor)
{
    if (!isActive() || r.isEmpty())
	return;
    dengine->updateState(ds);

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {
	    QPen cpen = ds->pen;
	    ds->pen = QPen(black, 0, DotLine);
	    dengine->updatePen(ds);
	    drawPolygon(QPointArray(rect));
	    ds->pen = cpen;
	    dengine->updatePen(ds);
	    return;
	}
	rect = xForm(rect);
    }

    dengine->drawWinFocusRect(rect, false, bgColor);
}


void QPainter::setBackgroundMode(BGMode mode)
{
    if (mode != TransparentMode && mode != OpaqueMode) {
	qWarning( "QPainter::setBackgroundMode: Invalid mode" );
	return;
    }
    ds->bgMode = mode;
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyBackground);
}

QPainter::BGMode QPainter::backgroundMode() const
{
    return ds->bgMode;
}


void QPainter::setPen(const QColor &color)
{
    ds->pen = QPen(color, 0, Qt::SolidLine);
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyPen);
}

void QPainter::setPen(const QPen &pen)
{
    ds->pen = pen;
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyPen);
}

void QPainter::setPen(PenStyle style)
{
    if (ds->pen.style() == style)
	return;
    ds->pen.setStyle(style);
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyPen);
}

const QPen &QPainter::pen() const
{
    return ds->pen;
}


void QPainter::setBrush(const QBrush &brush)
{
    if (ds->brush == brush)
	return;
    ds->brush = brush;
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyBrush);
}


void QPainter::setBrush(BrushStyle style)
{
    // ### Missing optimization from qpainter.cpp
    ds->brush = QBrush(Qt::black, style);
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyBrush);
}

const QBrush &QPainter::brush() const
{
    return ds->brush;
}

void QPainter::setBackgroundColor(const QColor &color)
{
    ds->bgColor = QBrush(color);
    if (dengine && dengine->isActive())
	dengine->updateBackground(ds);
}

const QColor &QPainter::backgroundColor() const
{
    return ds->bgColor;
}

void QPainter::setRasterOp(RasterOp op)
{
    if ((uint)op > LastROP) {
	qWarning("QPainter::setRasterOp: Invalid ROP code");
	return;
    }
    ds->rasterOp = op;
    if (dengine)
	dengine->updateRasterOp(ds);
}

void QPainter::setFont(const QFont &font)
{
    ds->font = font;
    if (dengine)
	dengine->setDirty(QPaintEngine::DirtyFont);
}

const QFont &QPainter::font() const
{
    return ds->font;
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

    dengine->updateState(ds);

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {
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
	    a[0].makeArc( x, y, rxx2, ryy2, 1*16*90, 16*90, ds->matrix );
	    a[1].makeArc( x, y+h-ryy2, rxx2, ryy2, 2*16*90, 16*90, ds->matrix );
	    a[2].makeArc( x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*16*90, 16*90, ds->matrix );
	    a[3].makeArc( x+w-rxx2, y, rxx2, ryy2, 0*16*90, 16*90, ds->matrix );
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
	    dengine->drawPolygon(aa, false, 0, aa.size());
	    return;
	}
	rect = xForm(rect);
    }
    dengine->drawRoundRect(rect, xRnd, yRnd);
}

void QPainter::drawEllipse(const QRect &r)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {
	    QPointArray a;
	    a.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), 0, 360*16, ds->matrix);
	    dengine->drawPolygon(a, false, 0, a.size());
	    return;
	}
	rect = xForm(rect);
    }

    dengine->drawEllipse(rect);
}

void QPainter::drawArc(const QRect &r, int a, int alen)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {
	    QPointArray pa;
	    pa.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), a, alen, ds->matrix);
	    dengine->drawPolyline(pa, 0, pa.size());
	    return;
	}
	rect = xForm(rect);
    }
    dengine->drawArc(rect, a, alen);
}

 void QPainter::drawPie(const QRect &r, int a, int alen)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if ( a > (360*16) ) {
	a = a % (360*16);
    } else if ( a < 0 ) {
	a = a % (360*16);
	if ( a < 0 ) a += (360*16);
    }

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {		// rotate/shear
	    // arc polyline
	    QPointArray pa;
	    pa.makeArc(rect.x(), rect.y(), rect.width(), rect.height(), a, alen, ds->matrix );
	    int n = pa.size();
	    QPoint p = xForm(QPoint(r.x()+r.width()/2, r.y()+r.height()/2));
	    pa.resize(n+2);
	    pa.setPoint(n, p);	// add legs
	    pa.setPoint(n+1, pa.at(0));
	    dengine->drawPolygon(pa, false, 0, pa.size());
	    return;
	}
	rect = xForm(rect);
    }
    dengine->drawPie(rect, a, alen);
}

void QPainter::drawChord(const QRect &r, int a, int alen)
{
    if ( !isActive() )
	return;
    dengine->updateState(ds);

    QRect rect = r.normalize();

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	if (d->txop == TxRotShear) {		// rotate/shear
	    QPointArray pa;
	    pa.makeArc(rect.x(), rect.y(), rect.width()-1, rect.height()-1, a, alen, ds->matrix); // arc polygon
	    int n = pa.size();
	    pa.resize(n+1);
	    pa.setPoint(n, pa.at(0));		// connect endpoints
	    dengine->drawPolygon(pa, false, 0, pa.size());
	    return;
	}
	rect = xForm(rect);
    }
    dengine->drawChord(rect, a, alen);
}

void QPainter::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if ( nlines < 0 )
	nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > (int)a.size() )
	nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
	return;

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray pa = xForm(a, index, nlines*2);
	if ( pa.size() != a.size()) {
	    index  = 0;
	    nlines = pa.size()/2;
	}
	dengine->drawLineSegments(pa, index, nlines);
	return;
    }

    dengine->drawLineSegments(a, index, nlines);
}

void QPainter::drawPolyline(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray ar = xForm(a, index, npoints);
	dengine->drawPolyline(ar, index, npoints);
	return;
    }

    dengine->drawPolyline(a, index, npoints);
}

void QPainter::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray ar = xForm(a, index, npoints);
	dengine->drawPolygon(ar, winding, index, npoints);
	return;
    }
    dengine->drawPolygon(a, winding, index, npoints);
}

void QPainter::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray ar = xForm(a, index, npoints);
	dengine->drawConvexPolygon(ar, index, npoints);
	return;
    }
    dengine->drawConvexPolygon(a, index, npoints);
}

void QPainter::drawCubicBezier(const QPointArray &a, int index )
{
    if ( !isActive() )
	return;
    dengine->updateState(ds);

    if ( (int)a.size() - index < 4 ) {
	qWarning( "QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
		  "points" );
	return;
    }

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::CoordTransform)) {
	QPointArray pa = xForm(a);
	dengine->drawCubicBezier(pa, index);
	return;
    }

    dengine->drawCubicBezier(a, index);
}

void QPainter::drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh)
{
    if (!isActive() || pm.isNull())
	return;
    dengine->updateState(ds);

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

    if ((ds->VxF || ds->WxF) && !dengine->hasCapability(QPaintEngine::PixmapTransform)) {
	QWMatrix mat(ds->matrix);
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
	if (!pmx.mask() && d->txop == TxRotShear) {
	    QBitmap bm_clip(sw, sh, 1);	// make full mask, xform it
	    bm_clip.fill(color1);
	    pmx.setMask(bm_clip.xForm(mat));
	}
	map(x, y, &x, &y);	// compute position of pixmap
	int dx, dy;
	mat.map( 0, 0, &dx, &dy );
	dengine->drawPixmap(QRect(x-dx, y-dy, pmx.width(), pmx.height()), pmx,
			QRect(0, 0, pmx.width(), pmx.height()));
	return;
    }

    dengine->drawPixmap(QRect(x, y, sw, sh), pm, QRect(sx, sy, sw, sh));
    return;
}


void QPainter::drawPixmap( const QRect &r, const QPixmap &pm )
{
    if (!isActive() || pm.isNull())
	return;
    dengine->updateState(ds);

    QPixmap pixmap = pm;

    if (!dengine->hasCapability(QPaintEngine::PixmapTransform)) {
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
    dengine->drawPixmap(r, pixmap, pixmap.rect());
}

void QPainter::drawImage(int x, int y, const QImage &,
			  int sx, int sy, int sw, int sh,
			  int conversionFlags)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}

void QPainter::drawImage(const QRect &, const QImage &)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}


void QPainter::drawText(int x, int y, const QString &str, TextDirection dir)
{
    if ( !isActive() )
        return;
    dengine->updateState(ds);

    if (str.isEmpty())
	return;
    int len = str.length();

    QTextLayout layout(str, ds->pfont ? *ds->pfont : ds->font);
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
    if ( ds->font.underline() ) textFlags |= Qt::Underline;
    if ( ds->font.overline() ) textFlags |= Qt::Overline;
    if ( ds->font.strikeOut() ) textFlags |= Qt::StrikeOut;

    // ### call fill rect here...
#if defined(Q_WS_X11)
    extern void qt_draw_background( QPainter *pp, int x, int y, int w,  int h );
    if (backgroundMode() == OpaqueMode)
 	qt_draw_background(this, x, y-sl.ascent, sl.textWidth, sl.ascent+sl.descent+1);
#endif

    line.draw(this, x, y-sl.ascent);
}

void QPainter::drawText(const QRect &r, int flags, const QString &str, int len,
			 QRect *br, QTextParag **intern)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if ( len < 0 )
	len = str.length();
    if ( len == 0 )				// empty string
	return;

    qt_format_text(font(), r, flags, str, len, br,
		   0, 0, 0, this);
}

void QPainter::drawTextItem(int x, int y, const QTextItem &ti, int textFlags)
{
    if (!isActive())
	return;
    dengine->updateState(ds);
    QTextEngine *engine = ti.engine();
    QScriptItem *si = &engine->items[ti.item()];

    engine->shape( ti.item() );
    QFontEngine *fe = engine->fontEngine(*si);
    Q_ASSERT( fe );

    x += si->x;
    y += si->y;

    QGlyphFragment gf;
    gf.analysis = si->analysis;
    gf.hasPositioning = si->hasPositioning;
    gf.ascent = si->ascent;
    gf.descent = si->descent;
    gf.width = si->width;
    gf.num_glyphs = si->num_glyphs;
    gf.glyphs = engine->glyphs( si );
    fe->draw( this, x,  y, gf, textFlags );
}

void QPainter::drawGlyphs(const QPoint& p, const QGlyphFragment &gf, int textFlags)
{
    if (!isActive())
	return;
    dengine->updateState(ds);
    gf.font->draw( this, p.x(),  p.y(), gf, textFlags );
}


QRect QPainter::boundingRect(int x, int y, int w, int h, int flags, const QString &str, int len,
			     QTextParag **internal)
{
    QRect brect;
    if ( str.isEmpty() )
	brect.setRect( x,y, 0,0 );
    else
	drawText(QRect(x, y, w, h), flags | DontPrint, str, len, &brect, internal);
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
    dengine->updateState(ds);

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

    bool optim = (pixmap.mask() && pixmap.depth() > 1 && d->txop <= TxTranslate);
    dengine->drawTiledPixmap(QRect(x, y, w, h), pixmap, QPoint(sx, sy), optim);
}

void QPainter::drawPicture(int x, int y, const QPicture &p)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    qWarning("QPainter::drawPicture() not implemetned yet...\n");
}

void QPainter::eraseRect(int x, int y, int w, int h)
{
    if (!isActive())
	return;
    dengine->updateState(ds);

    if (ds->bgBrush.pixmap())
	drawTiledPixmap(QRect(x, y, w, h), *ds->bgBrush.pixmap(), -ds->bgOrigin);
    else
	fillRect(x, y, w, h, ds->bgBrush);
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
    if (ds->VxF) {
	double scaleW = (double)ds->vw/(double)ds->ww;
	double scaleH = (double)ds->vh/(double)ds->wh;
	m.setMatrix(scaleW, 0, 0, scaleH, ds->vx - ds->wx*scaleW, ds->vy - ds->wy*scaleH );
    }
    if (ds->WxF) {
	if (ds->VxF)
	    m = ds->worldMatrix * m;
	else
	    m = ds->worldMatrix;
    }
    ds->matrix = m;


    d->txinv = false;				// no inverted matrix
    d->txop  = TxNone;
    if (ds->matrix.m12()==0.0 && ds->matrix.m21()==0.0
	&& ds->matrix.m11() >= 0.0 && ds->matrix.m22() >= 0.0 ) {
	if (ds->matrix.m11()==1.0 && ds->matrix.m22()==1.0 ) {
	    if (ds->matrix.dx()!=0.0 || ds->matrix.dy()!=0.0 )
		d->txop = TxTranslate;
	} else {
	    d->txop = TxScale;
	}
    } else {
	d->txop = TxRotShear;
    }
    if (!d->redirection_offset.isNull()) {
	d->txop |= TxTranslate;
	ds->WxF = true;
	// We want to translate in dev space so we do the adding of the redirection
	// offset manually.
	ds->matrix = QWMatrix(ds->matrix.m11(), ds->matrix.m12(),
			      ds->matrix.m21(), ds->matrix.m22(),
			      ds->matrix.dx()-d->redirection_offset.x(),
			      ds->matrix.dy()-d->redirection_offset.y());
    }
    dengine->setDirty(QPaintEngine::DirtyTransform);
//     printf("VxF=%d, WxF=%d\n", ds->VxF, ds->WxF);
//     printf("Using matrix: %f, %f, %f, %f, %f, %f\n",
// 	   ds->matrix.m11(),
// 	   ds->matrix.m12(),
// 	   ds->matrix.m21(),
// 	   ds->matrix.m22(),
// 	   ds->matrix.dx(),
// 	   ds->matrix.dy() );
}

void QPainter::updateInvXForm()
{
    Q_ASSERT(d->txinv == false);
    d->txinv = true;				// creating inverted matrix
    bool invertible;
    QWMatrix m;
    if (ds->VxF) {
	m.translate(ds->vx, ds->vy);
	m.scale(1.0*ds->vw/ds->ww, 1.0*ds->vh/ds->wh);
	m.translate(-ds->wx, -ds->wy);
    }
    if (ds->WxF) {
	if (ds->VxF)
	    m = ds->worldMatrix * m;
	else
	    m = ds->worldMatrix;
    }
    d->invMatrix = m.invert( &invertible );		// invert matrix
}


QPoint QPainter::xForm(const QPoint &p) const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->txop) {
    case TxNone:
	return p;
    case TxTranslate:
	return QPoint(qRound(p.x() + ds->matrix.dx()), qRound(p.y() + ds->matrix.dy()));
    case TxScale:
	return QPoint(qRound(ds->matrix.m11()*p.x() + ds->matrix.dx()),
		      qRound(ds->matrix.m22()*p.y() + ds->matrix.dy()));
    default:
	return QPoint(qRound(ds->matrix.m11()*p.x() + ds->matrix.m21()*p.y()+ds->matrix.dx()),
		      qRound(ds->matrix.m12()*p.x() + ds->matrix.m22()*p.y()+ds->matrix.dy()));
    }
#else
    return QPoint(p.x() + ds->xlatex, p.y() + ds->xlatey);
#endif
}

QRect QPainter::xForm(const QRect &r)	const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->txop) {
    case TxNone:
	return r;
    case TxTranslate: {
	QRect rect(r);
	rect.moveBy(qRound(ds->matrix.dx()), qRound(ds->matrix.dy()));
	return rect;
    }
    case TxScale:
	return QRect(qRound(ds->matrix.m11()*r.x() + ds->matrix.dx()),
		     qRound(ds->matrix.m22()*r.y() + ds->matrix.dy()),
		     qRound(ds->matrix.m11()*r.width()),
		     qRound(ds->matrix.m22()*r.height()));
    case TxRotShear:
	return ds->matrix.mapRect(r);
    }
    return r;
#else
    return QRect( r.x()+ds->xlatex, r.y()+ds->xlatey, r.width(), r.height() );
#endif
}

QPointArray QPainter::xForm(const QPointArray &a) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if ( d->txop == TxNone )
	return a;
    return ds->matrix * a;
#else
    QPointArray p(a);
    p.translate( ds->xlatex, ds->xlatey );
    return p;
#endif
}

QPointArray QPainter::xForm( const QPointArray &av, int index, int npoints ) const
{
    int lastPoint = npoints < 0 ? av.size() : index+npoints;
    QPointArray a(lastPoint-index);
    memcpy( a.data(), av.data()+index, (lastPoint-index)*sizeof( QPoint ) );
#ifndef QT_NO_TRANSFORMATIONS
    return ds->matrix*a;
#else
    a.translate( ds->xlatex, ds->xlatey );
    return a;
#endif
}

QPoint QPainter::xFormDev( const QPoint &p ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if(d->txop == TxNone)
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
    if (d->txop == TxNone)
	return r;
    if (!d->txinv) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    if (d->txop == TxRotShear) {			// rotation/shear
	return d->invMatrix.mapRect(r);
    } else {
	return QRect(qRound(d->invMatrix.m11()*r.x() + d->invMatrix.dx()),
		     qRound(d->invMatrix.m22()*r.y() + d->invMatrix.dy()),
		     qRound(d->invMatrix.m11()*r.width()),
		     qRound(d->invMatrix.m22()*r.height()));
    }
#else
    return QRect( r.x()-ds->xlatex, r.y()-ds->xlatey, r.width(), r.height() );
#endif
}

QPointArray QPainter::xFormDev( const QPointArray &a ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->txop == TxNone)
	return a;
    if ( !d->txinv ) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    return d->invMatrix * a;
#else
    QPointArray p(a);
    p.translate( -ds->xlatex, -ds->xlatey );
    return p;
#endif

}

QPointArray QPainter::xFormDev( const QPointArray &ad, int index, int npoints ) const
{
    int lastPoint = npoints < 0 ? ad.size() : index+npoints;
    QPointArray a( lastPoint-index );
    memcpy( a.data(), ad.data()+index, (lastPoint-index)*sizeof( QPoint ) );
#ifndef QT_NO_TRANSFORMATIONS
    if (d->txop == TxNone)
	return a;
    if (!d->txinv) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    return d->invMatrix * a;
#else
    a.translate( -ds->xlatex, -ds->xlatey );
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

double QPainter::m11() const { return ds->matrix.m11(); }
double QPainter::m12() const { return ds->matrix.m12(); }
double QPainter::m21() const { return ds->matrix.m21(); }
double QPainter::m22() const { return ds->matrix.m22(); }
double QPainter::dx() const { return ds->matrix.dx(); }
double QPainter::dy() const { return ds->matrix.dy(); }
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
	    extern void qt_draw_background( QPainter *pp, int x, int y, int w,  int h );

 	    if (painter->backgroundMode() == Qt::OpaqueMode)
 		qt_draw_background(painter, r.x() + line.x() + xoff, r.y() + yoff + line.y(),
				   line.width(), line.ascent() + line.descent() + 1);
#endif
	    line.draw(painter, r.x() + xoff + line.x(), r.y() + yoff, underlinePositions);
	}

	if ( restoreClipping ) {
	    painter->setClipRegion(painterClipRegion, QPainter::CoordPainter);
	    painter->setClipping( painterHasClip );
	}
    }

    if ( underlinePositions != underlinePositionStack )
	delete [] underlinePositions;
}


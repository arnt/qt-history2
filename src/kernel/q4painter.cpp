#ifndef Q_Q4PAINTER
#include "q4paintdevice.h"
#else
#include "qpaintdevice.h"
#endif
#include "q4painter.h"
#include "q4painter_p.h"
#include "qabstractgc.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#ifdef Q_WS_WIN
#include "qwin32gc.h"
#endif
#include "qpaintdevicemetrics.h"
#include "qtextlayout_p.h"
#include "qtextengine_p.h"
#include "qfontengine_p.h"
#include "qwidget_p.h"

#define ds d->state
#define dgc d->gc

#define qWarning qDebug
#ifndef Q_Q4PAINTER
#define QPainter QPainter
#endif

void qt_format_text(const QFont &font, const QRect &_r, int tf, const QString& str,
		    int len, QRect *brect, int tabstops, int* tabarray, int tabarraylen,
		    QTextParag **, QPainter* painter);

static inline void qt_fix_rect(QPainterState *ps, int *x, int *y, int *w, int *h)
{
    if ( *w < 0 ) {
	*w = -*w;
	*x -= *w - 1;
    }
    if ( *h < 0 ) {
	*h = -*h;
	*y -= *h - 1;
    }
    if (ps->pen.style() == Qt::NoPen) {
	w++;
	h++;
    }
}

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
}

QPaintDevice *QPainter::device() const
{
    return d->device;
}

bool QPainter::isActive() const
{
    if (dgc) {
	return dgc->isActive();
    }
    return false;
}

void QPainter::save()
{
    d->save();
    if (isActive())
	dgc->setState(ds, false);
}

void QPainter::restore()
{
    d->restore();

    updateXForm();
    if (isActive())
	dgc->setState(ds);
}

bool QPainter::begin(const QPaintDevice *pd, bool unclipped)
{
    if (dgc) {
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
    dgc = pd->gc();

    if (!dgc) {
	qWarning("QPainter::begin(), paintdevice returned gc == 0, type: %d\n", pd->devType());
	return true;
    }

    switch (pd->devType()) {
    case QInternal::Widget:
	{
#ifdef Q_Q4PAINTER
	    const QWidget *widget = static_cast<const QWidget *>(pd);
#else
	    const QWidget *widget = pd->widget();
#endif
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
#ifdef Q_Q4PAINTER
	    const QPixmap *pm = static_cast<const QPixmap *>(pd);
#else
	    const QPixmap *pm = pd->pixmap();
#endif
	    Q_ASSERT(pm);
	    ds->ww = ds->vw = pm->width();
	    ds->wh = ds->vh = pm->height();
	    break;
	}
    }

    if (!dgc->begin(pd, ds, unclipped)) {
	qWarning("QPainter::begin(), gc::begin() returned false\n");
	return false;
    }
    if (!d->redirection_offset.isNull()) {
	d->txop = TxTranslate;
	ds->WxF = true;
    }

    Q_ASSERT(dgc->isActive());
    dgc->setState(ds);

    return true;
}

bool QPainter::end()
{
    if (!isActive()) {
	qWarning( "QPainter::end: Painter is not active, aborted");
	return false;
    }

    bool ended = dgc->end();
    dgc->setState(0);
    if (ended)
	dgc = 0;

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
    if (isActive())
	dgc->updateBrush(ds);
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
    Q_ASSERT(dgc);
    ds->clipEnabled = enable;
    dgc->updateClipRegion(ds);
}


QRegion QPainter::clipRegion(CoordinateMode m) const
{
    return m == CoordDevice ? ds->clipRegion : ds->matrix.invert() * ds->clipRegion;
}

void QPainter::setClipRect( const QRect &rect, CoordinateMode mode ) // ### inline?
{
    Q_ASSERT(dgc);
    setClipRegion(QRegion(rect), mode);
}

void QPainter::setClipRegion(const QRegion &r, CoordinateMode m)
{
    Q_ASSERT(dgc);
    if (m == CoordPainter && (ds->VxF || ds->WxF))
	ds->clipRegion = ds->matrix * r;
    else
	ds->clipRegion = r;
    ds->clipEnabled = true;
    dgc->updateClipRegion(ds);
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
// 	setWorldXForm( FALSE );
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

#ifndef QT_NO_TRANSFORMATIONS
void QPainter::scale(double sx, double sy)
{
    QWMatrix m;
    m.scale( sx, sy );
    setWorldMatrix( m, TRUE );
}

void QPainter::shear(double sh, double sv)
{
    QWMatrix m;
    m.shear( sv, sh );
    setWorldMatrix( m, TRUE );
}

void QPainter::rotate(double a)
{
    QWMatrix m;
    m.rotate( a );
    setWorldMatrix( m, TRUE );
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

    if (!d->redirection_offset.isNull()) {
	d->txop = TxTranslate;
	ds->WxF = true;
    }
}

void QPainter::translate(double dx, double dy)
{
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix m;
    m.translate( dx, dy );
    setWorldMatrix( m, TRUE );
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

void QPainter::drawLine(int x1, int y1, int x2, int y2)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if (ds->WxF || ds->VxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    map(x1, y1, &x1, &y1);
	    map(x2, y2, &x2, &y2);
	}
    }

    dgc->drawLine(x1, y1, x2, y2);
}

void QPainter::drawRect(int x, int y, int w, int h)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if ((ds->VxF || ds->WxF) && !dgc->hasCapability(QAbstractGC::CoordTransform)) {
	if (d->txop < TxRotShear) {
	    map(x, y, w, h, &x, &y, &w, &h);
	} else {
	    drawPolygon(QPointArray(QRect(x, y, w, h)));
	    return;
	}
    }

    dgc->drawRect(x, y, w, h);
}

void QPainter::drawPoint(int x, int y)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    map(x, y, &x, &y);
	}
    }

    dgc->drawPoint(x, y);
}

void QPainter::drawPoints(const QPointArray &pa, int index, int npoints)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if (npoints < 0)
	npoints = pa.size() - index;
    if (index + npoints > (int)pa.size())
	npoints = pa.size() - index;
    if (!isActive() || npoints < 1 || index < 0)
	return;

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPointArray a = xForm(pa, index, npoints);
	    if (a.size() != a.size()) {
		index = 0;
		npoints = a.size();
	    }
	    dgc->drawPoints(a, index, npoints);
	    return;
	}
    }

    dgc->drawPoints(pa, index, npoints);
}


void QPainter::drawWinFocusRect(int x, int y, int w, int h)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPen cpen = ds->pen;
	    ds->pen = QPen(black, 0, DotLine);
	    dgc->updatePen(ds);
	    drawPolygon(QPointArray(QRect(x, y, w, h)));
	    ds->pen = cpen;
	    dgc->updatePen(ds);
	    return;
	}
    }

    dgc->drawWinFocusRect(x, y, w, h, true, color0);
}

void QPainter::drawWinFocusRect(int x, int y, int w, int h, const QColor &bgColor)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPen cpen = ds->pen;
	    ds->pen = QPen(black, 0, DotLine);
	    dgc->updatePen(ds);
	    drawPolygon(QPointArray(QRect(x, y, w, h)));
	    ds->pen = cpen;
	    dgc->updatePen(ds);
	    return;
	}
    }

    dgc->drawWinFocusRect(x, y, w, h, false, bgColor);
}


void QPainter::setBackgroundMode(BGMode mode)
{
    if (mode != TransparentMode && mode != OpaqueMode) {
	qWarning( "QPainter::setBackgroundMode: Invalid mode" );
	return;
    }
    ds->bgMode = mode;
    if (dgc && dgc->isActive())
	dgc->updateBackground(ds);
}

QPainter::BGMode QPainter::backgroundMode() const
{
    return ds->bgMode;
}


void QPainter::setPen(const QColor &color)
{
    ds->pen = QPen(color, 0, Qt::SolidLine);
    if (dgc && dgc->isActive())
	dgc->updatePen(ds);
}

void QPainter::setPen(const QPen &pen)
{
    ds->pen = pen;
    if (dgc && dgc->isActive())
	dgc->updatePen(ds);
}

void QPainter::setPen(PenStyle style)
{
    ds->pen.setStyle(style);
    if (dgc && dgc->isActive())
	dgc->updatePen(ds);
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
    if (dgc && dgc->isActive())
	dgc->updateBrush(ds);
}


void QPainter::setBrush(BrushStyle style)
{
    // ### Missing optimization from qpainter.cpp
    ds->brush = QBrush(Qt::black, style);
    if (dgc && dgc->isActive())
	dgc->updateBrush(ds);
}

const QBrush &QPainter::brush() const
{
    return ds->brush;
}

void QPainter::setBackgroundColor(const QColor &color)
{
    ds->bgColor = QBrush(color);
    if (dgc && dgc->isActive())
	dgc->updateBackground(ds);
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
    if (dgc && dgc->isActive())
	dgc->updateRasterOp(ds);
}

void QPainter::setFont(const QFont &font)
{
    ds->font = font;
    if (dgc && dgc->isActive())
	dgc->updateFont(ds);
}

const QFont &QPainter::font() const
{
    return ds->font;
}

void QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    if (d->txop == TxRotShear) {
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
		dgc->drawPolygon(aa, false, 0, aa.size());
		return;
	    }
	    map(x, y, w, h, &x, &y, &w, &h);
	}
    }
    dgc->drawRoundRect(x, y, w, h, xRnd, yRnd);
}

void QPainter::drawEllipse(int x, int y, int w, int h)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    if (d->txop == TxRotShear) {
		QPointArray a;
		a.makeArc(x, y, w, h, 0, 360*16, ds->matrix);
		dgc->drawPolygon(a, false, 0, a.size());
		return;
	    }
	    map(x, y, w, h, &x, &y, &w, &h);
	}
    }

    dgc->drawEllipse(x, y, w, h);
}

void QPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    if (d->txop == TxRotShear) {
		QPointArray pa;
		pa.makeArc(x, y, w, h, a, alen, ds->matrix);
		dgc->drawPolyline(pa, 0, pa.size());
		return;
	    }
	    map(x, y, w, h, &x, &y, &w, &h);
	}
    }
    dgc->drawArc(x, y, w, h, a, alen);
}

 void QPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    if (d->txop == TxRotShear) {		// rotate/shear
		QPointArray pa;
		pa.makeArc( x, y, w, h, a, alen, ds->matrix ); // arc polyline
		int n = pa.size();
		int cx, cy;
		ds->matrix.map(x+w/2, y+h/2, &cx, &cy);
		pa.resize(n+2);
		pa.setPoint(n, cx, cy);	// add legs
		pa.setPoint(n+1, pa.at(0));
		dgc->drawPolygon(pa, false, 0, pa.size());
		return;
	    }
	    map( x, y, w, h, &x, &y, &w, &h );
	}
    }
    dgc->drawPie(x, y, w, h, a, alen);
}

void QPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
    if ( !isActive() )
	return;
    dgc->setState(ds);

    qt_fix_rect(ds, &x, &y, &w, &h);

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    if (d->txop == TxRotShear) {		// rotate/shear
		QPointArray pa;
		pa.makeArc(x, y, w-1, h-1, a, alen, ds->matrix); // arc polygon
		int n = pa.size();
		pa.resize(n+1);
		pa.setPoint(n, pa.at(0));		// connect endpoints
		dgc->drawPolygon(pa, false, 0, pa.size());
		return;
	    }
	    map( x, y, w, h, &x, &y, &w, &h );
	}
    }
    dgc->drawChord(x, y, w, h, a, alen);
}

void QPainter::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if ( nlines < 0 )
	nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > (int)a.size() )
	nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
	return;

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPointArray pa = xForm(a, index, nlines*2);
	    if ( pa.size() != a.size()) {
		index  = 0;
		nlines = pa.size()/2;
	    }
	    pa.translate(-d->redirection_offset);
	    dgc->drawLineSegments(pa, index, nlines);
	    return;
	}
    }

    dgc->drawLineSegments(a, index, nlines);
}

void QPainter::drawPolyline(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPointArray ar = xForm(a, index, npoints);
	    dgc->drawPolyline(ar, index, npoints);
	    return;
	}
    }

    dgc->drawPolyline(a, index, npoints);
}

void QPainter::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPointArray ar = xForm(a, index, npoints);
	    dgc->drawPolygon(ar, winding, index, npoints);
	    return;
	}
    }
    dgc->drawPolygon(a, winding, index, npoints);
}

void QPainter::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if (npoints < 0)
	npoints = a.size() - index;
    if (index + npoints > (int)a.size())
	npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
	return;

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPointArray ar = xForm(a, index, npoints);
	    dgc->drawConvexPolygon(ar, index, npoints);
	    return;
	}
    }
    dgc->drawConvexPolygon(a, index, npoints);
}

void QPainter::drawCubicBezier(const QPointArray &a, int index )
{
    if ( !isActive() )
	return;
    dgc->setState(ds);

    if ( (int)a.size() - index < 4 ) {
	qWarning( "QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
		 "points" );
	return;
    }

    if (ds->VxF || ds->WxF) {
	if (!dgc->hasCapability(QAbstractGC::CoordTransform)) {
	    QPointArray pa = xForm(a);
	    pa.translate(-d->redirection_offset);
	    dgc->drawCubicBezier(pa, index);
	    return;
	}
    }

    dgc->drawCubicBezier(a, index);
}

void QPainter::drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh)
{
    if (!isActive() || pm.isNull())
	return;
    dgc->setState(ds);

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

    if ((ds->VxF || ds->WxF) && !dgc->hasCapability(QAbstractGC::PixmapTransform)) {
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
	dgc->drawPixmap(x-dx, y-dy, pmx, 0, 0, pmx.width(), pmx.height());
	return;
    }

    dgc->drawPixmap(x, y, pm, sx, sy, sw, sh);
    return;
}


void QPainter::drawPixmap( const QRect &r, const QPixmap &pm )
{
    if (!isActive() || pm.isNull())
	return;
    dgc->setState(ds);

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

    QPixmap pixmap = pm;

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
    drawPixmap( r.x(), r.y(), pixmap );
}

void QPainter::drawImage(int x, int y, const QImage &,
			  int sx, int sy, int sw, int sh,
			  int conversionFlags)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}

void QPainter::drawImage(const QRect &, const QImage &)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qWarning("QPainter::drawImage(), %d", __LINE__);
}


void QPainter::drawText(int x, int y, const QString &str, int pos, int len, TextDirection dir)
{
    if ( !isActive() )
        return;
    dgc->setState(ds);

    if (len < 0)
        len = str.length() - pos;
    if ( len <= 0 || pos >= (int)str.length() ) // empty string
        return;
    if ( pos + len > (int)str.length() )
        len = str.length() - pos;

    bool simple = str.isSimpleText();
    // we can't take the complete string here as we would otherwise
    // get quadratic behaviour when drawing long strings in parts.
    // we do however need some chars around the part we paint to get arabic shaping correct.
    // ### maybe possible to remove after cursor restrictions work in QRT
    int start;
    int end;
    if ( simple ) {
	start = pos;
	end = pos+len;
    } else {
	start = QMAX( 0,  pos - 8 );
	end = QMIN( (int)str.length(), pos + len + 8 );
    }
    QConstString cstr( str.unicode() + start, end - start );
    pos -= start;

//     QTextEngine engine( cstr.string(), pfont ? pfont->d : cfont.d );
//     QTextEngine engine( cstr.string(), ds->font.d );
    QTextLayout layout(cstr.string(), 0);
    QTextEngine *engine = layout.engine();

    // this is actually what beginLayout does. Inlined here, so we can
    // avoid the bidi algorithm if we don't need it.
    engine->itemize( simple ? QTextEngine::NoBidi|QTextEngine::SingleLine : QTextEngine::Full|QTextEngine::SingleLine );
    engine->currentItem = 0;
    engine->firstItemInLine = -1;

    if ( !simple ) {
	layout.setBoundary( pos );
	layout.setBoundary( pos + len );
    }

    if ( dir != Auto ) {
	int level = dir == RTL ? 1 : 0;
	for ( int i = engine->items.size(); i >= 0; i-- )
	    engine->items[i].analysis.bidiLevel = level;
    }

    // small hack to force skipping of unneeded items
    start = 0;
    while ( engine->items[start].position < pos )
	++start;
    engine->currentItem = start;
    layout.beginLine( 0xfffffff );
    end = start;
    while ( !layout.atEnd() && layout.currentItem().from() < pos + len ) {
	layout.addCurrentItem();
	end++;
    }
    QFontMetrics fm(fontMetrics());
    int ascent = fm.ascent(), descent = fm.descent();
    int left, right;
    layout.endLine( 0, 0, Qt::SingleLine|Qt::AlignLeft, &ascent, &descent, &left, &right );

    // do _not_ call endLayout() here, as it would clean up the shaped items and we would do shaping another time
    // for painting.

    int textFlags = 0;
    if ( ds->font.underline() ) textFlags |= Qt::Underline;
    if ( ds->font.overline() ) textFlags |= Qt::Overline;
    if ( ds->font.strikeOut() ) textFlags |= Qt::StrikeOut;

    // ### call fill rect here...
#if defined(Q_WS_X11)
    extern void qt_draw_background( QPainter *pp, int x, int y, int w,  int h );
    if (backgroundMode() == OpaqueMode)
 	qt_draw_background(this, x, y-ascent, right-left, ascent+descent+1);
#endif

    for ( int i = start; i < end; i++ ) {
	QTextItem ti(i, engine);
	drawTextItem( x, y - ascent, ti, textFlags );
    }
}

void QPainter::drawText(const QRect &r, int flags, const QString &str, int len,
			 QRect *br, QTextParag **intern)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    if ( len < 0 )
	len = str.length();
    if ( len == 0 )				// empty string
	return;

    qt_format_text(font(), r, flags, str, len, br,
		   0, 0, 0, intern, this);
}

void QPainter::drawTextItem(int x, int y, const QTextItem &ti, int textFlags)
{
    if (!isActive())
	return;
    dgc->setState(ds);
#ifdef Q_Q4PAINTER
    QTextEngine *engine = ti.engine();
    QScriptItem *si = &engine->items[ti.item()];

    engine->shape( ti.item() );
    QFontEngine *fe = si->fontEngine;
    Q_ASSERT( fe );

    x += si->x;
    y += si->y;

#ifdef Q_WS_WIN
    QWin32GC *wingc = (QWin32GC*)dgc;
    HDC oldDC = fe->hdc;
    fe->hdc = dgc->handle();
    SelectObject( wingc->handle(), fe->hfont );
#endif
    fe->draw( this, x,  y, engine, si, textFlags );
#endif
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


void QPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap &, int sx, int sy)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qWarning("QPainter::drawTiledPixmap:: not implemented yet\n");
}

void QPainter::drawPicture(int x, int y, const QPicture &p)
{
    if (!isActive())
	return;
    dgc->setState(ds);

    qWarning("QPainter::drawPicture() not implemetned yet...\n");
}

void QPainter::eraseRect(int x, int y, int w, int h)
{
    if (!isActive())
	return;
    dgc->setState(ds);

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


    d->txinv = FALSE;				// no inverted matrix
    d->txop  = TxNone;
    if (ds->matrix.m12()==0.0 && ds->matrix.m21()==0.0
	&& ds->matrix.m11() >= 0.0 && ds->matrix.m22() >= 0.0 ) {
	if (ds->matrix.m11()==1.0 && ds->matrix.m22()==1.0 ) {
	    if (ds->matrix.dx()!=0.0 || ds->matrix.dy()!=0.0 )
		d->txop = TxTranslate;
	} else {
	    d->txop = TxScale;
// #if defined(Q_WS_WIN)
// 	    setf(DirtyFont);
// #endif
	}
    } else {
	d->txop = TxRotShear;
// #if defined(Q_WS_WIN)
// 	setf(DirtyFont);
// #endif
    }
// ###    setBrushOrigin(bro.x(), bro.y() );

    if (!d->redirection_offset.isNull()) {
	d->txop |= TxTranslate;
	ds->WxF = true;
    }

//     printf("VxF=%d, WxF=%d\n", ds->VxF, ds->WxF);
//     printf("Using matrix: %f, %f, %f, %f, %f, %f\n",
// 	   ds->matrix.m11(),
// 	   ds->matrix.m12(),
// 	   ds->matrix.m21(),
// 	   ds->matrix.m22(),
// 	   ds->matrix.dx(),
// 	   ds->matrix.dy() );

    dgc->updateXForm(ds);
}

void QPainter::updateInvXForm()
{
    Q_ASSERT(d->txinv == FALSE);
    d->txinv = TRUE;				// creating inverted matrix
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


/*!
  \internal
  Maps a point from logical coordinates to device coordinates.
*/

void QPainter::map(int x, int y, int *rx, int *ry) const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->txop) {

    case TxNone:
	*rx = x;
	*ry = y;
	break;
    case TxTranslate:
	*rx = qRound(x + ds->matrix.dx());
	*ry = qRound(y + ds->matrix.dy());
	break;
    case TxScale:
	*rx = qRound(ds->matrix.m11()*x + ds->matrix.dx());
	*ry = qRound(ds->matrix.m22()*y + ds->matrix.dy());
	break;
    default:
	*rx = qRound(ds->matrix.m11()*x + ds->matrix.m21()*y+ds->matrix.dx());
	*ry = qRound(ds->matrix.m12()*x + ds->matrix.m22()*y+ds->matrix.dy());
	break;
    }
#else
    *rx = x + ds->xlatex;
    *ry = y + ds->xlatey;
#endif

    *rx -= d->redirection_offset.x();
    *ry -= d->redirection_offset.y();
}

/*!
  \internal
  Maps a rectangle from logical coordinates to device coordinates.
  This internal function does not handle rotation and/or shear.
*/

void QPainter::map(int x, int y, int w, int h,
		    int *rx, int *ry, int *rw, int *rh) const
{
#ifndef QT_NO_TRANSFORMATIONS
    switch (d->txop) {
    case TxNone:
	*rx = x;  *ry = y;
	*rw = w;  *rh = h;
	break;
    case TxTranslate:
	*rx = qRound(x + ds->matrix.dx());
	*ry = qRound(y + ds->matrix.dy());
	*rw = w;  *rh = h;
	break;
    case TxScale:
	*rx = qRound(ds->matrix.m11()*x + ds->matrix.dx());
	*ry = qRound(ds->matrix.m22()*y + ds->matrix.dy());
	*rw = qRound(ds->matrix.m11()*w);
	*rh = qRound(ds->matrix.m22()*h);
	break;
    default:
	qWarning("QPainter::map: Internal error");
	break;
    }
#else
    *rx = x + ds->matrix.xlatex;
    *ry = y + ds->matrix.xlatey;
    *rw = w;  *rh = h;
#endif

    *rx -= d->redirection_offset.x();
    *ry -= d->redirection_offset.y();
}


void QPainter::mapInv( int x, int y, int *rx, int *ry ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if ( !d->txinv )
	qWarning( "QPainter::mapInv: Internal error" );
    *rx = qRound( d->invMatrix.m11()*x + d->invMatrix.m21()*y + d->invMatrix.dx() );
    *ry = qRound( d->invMatrix.m12()*x + d->invMatrix.m22()*y + d->invMatrix.dy() );
#else
    *rx = x - xlatex;
    *ry = y - xlatey;
#endif
}


void QPainter::mapInv( int x, int y, int w, int h,
		       int *rx, int *ry, int *rw, int *rh ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if ( !d->txinv || d->txop == TxRotShear )
	qWarning( "QPainter::mapInv: Internal error" );
    *rx = qRound( d->invMatrix.m11()*x + d->invMatrix.dx() );
    *ry = qRound( d->invMatrix.m22()*y + d->invMatrix.dy() );
    *rw = qRound( d->invMatrix.m11()*w );
    *rh = qRound( d->invMatrix.m22()*h );
#else
    *rx = x - xlatex;
    *ry = y - xlatey;
    *rw = w;
    *rh = h;
#endif
}


QPoint QPainter::xForm( const QPoint &pt) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if ( d->txop == TxNone )
	return pt;
    int x=pt.x(), y=pt.y();
    map( x, y, &x, &y );
    return QPoint( x, y ) + d->redirection_offset;
#else
    return QPoint( pt.x()+ds->xlatex, pt.y()+ds->xlatey );
#endif
}

QRect QPainter::xForm( const QRect &rv )	const
{
#ifndef QT_NO_TRANSFORMATIONS
    if ( d->txop == TxNone )
	return rv;
    if ( d->txop == TxRotShear ) {			// rotation/shear
	return ds->matrix.mapRect( rv );
    }
    // Just translation/scale
    int x, y, w, h;
    rv.rect(&x, &y, &w, &h);
    map(x, y, w, h, &x, &y, &w, &h);
    return QRect( x + d->redirection_offset.x(), y + d->redirection_offset.y(), w, h );
#else
    return QRect( rv.x()+ds->xlatex, rv.y()+ds->xlatey, rv.width(), rv.height() );
#endif
}

QPointArray QPainter::xForm( const QPointArray &av ) const
{
    QPointArray a = av;
#ifndef QT_NO_TRANSFORMATIONS
    if ( d->txop != TxNone )
	return ds->matrix * av;
#else
    a.translate( ds->xlatex, ds->xlatey );
#endif
    return a;
}

QPointArray QPainter::xForm( const QPointArray &av, int index, int npoints ) const
{
    int lastPoint = npoints < 0 ? av.size() : index+npoints;
    QPointArray a( lastPoint-index );
    memcpy( a.data(), av.data()+index, (lastPoint-index)*sizeof( QPoint ) );
#ifndef QT_NO_TRANSFORMATIONS
    return ds->matrix*a;
#else
    a.translate( ds->xlatex, ds->xlatey );
    return a;
#endif
}

QPoint QPainter::xFormDev( const QPoint &pd ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if(d->txop == TxNone)
	return pd;
    if (!d->txinv) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
#endif
    int x=pd.x(), y=pd.y();
    mapInv( x, y, &x, &y );
    return QPoint( x, y );
}

QRect QPainter::xFormDev( const QRect &rd )  const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->txop == TxNone)
	return rd;
    if (!d->txinv) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    if (d->txop == TxRotShear) {			// rotation/shear
	return d->invMatrix.mapRect(rd);
    }
#endif
    // Just translation/scale
    int x, y, w, h;
    rd.rect( &x, &y, &w, &h );
    mapInv( x, y, w, h, &x, &y, &w, &h );
    return QRect( x, y, w, h);
}

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{
#ifndef QT_NO_TRANSFORMATIONS
    if (d->txop == TxNone)
	return ad;
    if ( !d->txinv ) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    return d->invMatrix * ad;
#else
    // ###
    return ad;
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
    // ###
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
    Q_ASSERT(d->gc);
    return d->gc->handle();
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
		     QTextParag **, QPainter* painter )
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
    // str.setLength() always does a deep copy, so the replacement
    // code below is safe.
    text.setLength( len );
    // compatible behaviour to the old implementation. Replace
    // tabs by spaces
    QChar *chr = (QChar*)text.unicode();
    const QChar *end = chr + len;
    bool haveLineSep = FALSE;
    while ( chr != end ) {
	if ( *chr == '\r' || ( singleline && *chr == '\n' ) ) {
	    *chr = ' ';
	} else if ( *chr == '\n' ) {
	    *chr = QChar_linesep;
	    haveLineSep = TRUE;
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
	    text.setLength( newlen );
    }

    // no need to do extra work for underlines if we don't paint
    if ( tf & Qt::DontPrint )
	numUnderlines = 0;

    int height = 0;
    int left = r.width();
    int right = 0;

    QTextLayout textLayout( text, fnt );
    int rb = QMAX( 0, -fm.minRightBearing() );
    int lb = QMAX( 0, -fm.minLeftBearing() );

    if ( text.isEmpty() ) {
	height = fm.height();
	left = right = 0;
	tf |= QPainter::DontPrint;
    } else {
	textLayout.beginLayout((haveLineSep || expandtabs || wordbreak) ?
			       QTextLayout::MultiLine :
			       (tf & Qt::DontPrint) ? QTextLayout::NoBidi : QTextLayout::SingleLine );

	// break underline chars into items of their own
	for( int i = 0; i < numUnderlines; i++ ) {
	    textLayout.setBoundary( underlinePositions[i] );
	    textLayout.setBoundary( underlinePositions[i]+1 );
	}

	int lineWidth = wordbreak ? QMAX(0, r.width()-rb-lb) : INT_MAX;
	if(!wordbreak)
	    tf |= Qt::IncludeTrailingSpaces;

	int add = 0;
	int leading = fm.leading();
	int asc = fm.ascent();
	int desc = fm.descent();
	height = -leading;

	//qDebug("\n\nbeginLayout: lw = %d, rectwidth=%d", lineWidth , r.width());
	while ( !textLayout.atEnd() ) {
	    height += leading;
	    textLayout.beginLine( lineWidth == INT_MAX ? lineWidth : lineWidth + add );
	    //qDebug("-----beginLine( %d )-----",  lineWidth+add );
	    bool linesep = FALSE;
	    while ( 1 ) {
		QTextItem ti = textLayout.currentItem();
 		//qDebug("item: from=%d, ch=%x", ti.from(), text.unicode()[ti.from()].unicode() );
		if ( expandtabs && ti.isTab() ) {
		    int tw = 0;
		    int x = textLayout.widthUsed();
		    if ( tabarraylen ) {
// 			qDebug("tabarraylen=%d", tabarraylen );
			int tab = 0;
			while ( tab < tabarraylen ) {
			    if ( tabarray[tab] > x ) {
				tw = tabarray[tab] - x;
				break;
			    }
			    ++tab;
			}
		    } else {
			tw = tabstops - (x % tabstops);
		    }
 		    //qDebug("tw = %d",  tw );
		    if ( tw )
			ti.setWidth( tw );
		}
		if ( ti.isObject() && text.unicode()[ti.from()] == QChar_linesep )
		    linesep = TRUE;

		if ( linesep || textLayout.addCurrentItem() != QTextLayout::Ok || textLayout.atEnd() )
		    break;
	    }

	    int ascent = asc, descent = desc, lineLeft, lineRight;
	    textLayout.setLineWidth( r.width()-rb-lb + add );
	    int state = textLayout.endLine( 0, height, tf, &ascent, &descent,
					    &lineLeft, &lineRight );

	    if ( state != QTextLayout::LineEmpty || linesep ) {
		//qDebug("finalizing line: lw=%d ascent = %d, descent=%d lineleft=%d lineright=%d", lineWidth+add, ascent, descent,lineLeft, lineRight  );
		left = QMIN( left, lineLeft );
		right = QMAX( right, lineRight );
		height += ascent + descent + 1;
		add = 0;
		if ( linesep )
		    textLayout.nextItem();
	    } else {
		add += 10;
	    }
	}
    }

    int yoff = 0;
    if ( tf & Qt::AlignBottom )
	yoff = r.height() - height;
    else if ( tf & Qt::AlignVCenter )
	yoff = (r.height() - height)/2;

    if ( brect ) {
	*brect = QRect( r.x() + left, r.y() + yoff, right-left + lb+rb, height );
	//qDebug("br = %d %d %d/%d, left=%d, right=%d", brect->x(), brect->y(), brect->width(), brect->height(), left, right);
    }

    if (!(tf & QPainter::DontPrint)) {
	bool restoreClipping = FALSE;
	bool painterHasClip = FALSE;
	QRegion painterClipRegion;
	if ( !dontclip ) {
#ifndef QT_NO_TRANSFORMATIONS
	    // ### port properly
	    QRegion reg = /* painter->xmat * */ r;
#else
	    QRegion reg = r;
	    reg.translate( painter->xlatex, painter->xlatey );
#endif
	    if ( painter->hasClipping() )
		reg &= painter->clipRegion();

	    painterHasClip = painter->hasClipping();
	    painterClipRegion = painter->clipRegion();
	    restoreClipping = TRUE;
	    painter->setClipRegion( reg );
	} else {
	    if ( painter->hasClipping() ){
		painterHasClip = painter->hasClipping();
		painterClipRegion = painter->clipRegion();
		restoreClipping = TRUE;
		painter->setClipping( FALSE );
	    }
	}

	int cUlChar = 0;
	int _tf = 0;
	if (fnt.underline()) _tf |= Qt::Underline;
	if (fnt.overline()) _tf |= Qt::Overline;
	if (fnt.strikeOut()) _tf |= Qt::StrikeOut;

	//qDebug("have %d items",textLayout.numItems());
	for ( int i = 0; i < textLayout.numItems(); i++ ) {
	    QTextItem ti = textLayout.itemAt( i );
 	    //qDebug("Item %d: from=%d,  length=%d,  space=%d x=%d", i, ti.from(),  ti.length(), ti.isSpace(), ti.x() );
	    if ( ti.isTab() || ti.isObject() )
		continue;
	    int textFlags = _tf;
	    if ( !noaccel && numUnderlines > cUlChar && ti.from() == underlinePositions[cUlChar] ) {
		textFlags |= Qt::Underline;
		cUlChar++;
	    }
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
	    extern void qt_draw_background( QPainter *pp, int x, int y, int w,  int h );

 	    if (painter->backgroundMode() == Qt::OpaqueMode)
 		qt_draw_background(painter, r.x()+lb + ti.x(), r.y() + yoff + ti.y() - ti.ascent(),
				   ti.width(), ti.ascent() + ti.descent() + 1);
#endif
	    painter->drawTextItem( r.x()+lb, r.y() + yoff, ti, textFlags );
	}

	if ( restoreClipping ) {
	    painter->setClipRegion( painterClipRegion );
	    painter->setClipping( painterHasClip );
	}
    }

    if ( underlinePositions != underlinePositionStack )
	delete [] underlinePositions;
}


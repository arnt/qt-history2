/****************************************************************************
**
** Definition of QWSPaintEngine class.
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

#include "qpaintengine_qws.h"
#include "qpainter_p.h"
#include <private/qfontengine_p.h>

#include "qgfx_qws.h"
#include "qregion.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include <private/qpaintengine_p.h>

/* paintevent magic to provide Windows semantics on Qt/E
 */
static QRegion* paintEventClipRegion = 0;
//static QRegion* paintEventSaveRegion = 0;
static QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region)
{
    if ( !paintEventClipRegion )
	paintEventClipRegion = new QRegion( region );
    else
	*paintEventClipRegion = region;
    paintEventDevice = dev;

#ifdef QWS_EXTRA_DEBUG
    qDebug( "qt_set_paintevent_clipping" );
    QMemArray<QRect> ar = region.rects();
    for ( int i=0; i<int(ar.size()); i++ ) {
	QRect r = ar[i];
        qDebug( "   r[%d]:  %d,%d %dx%d", i,
		r.x(), r.y(), r.width(), r.height() );
    }
#endif
}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
//    delete paintEventSaveRegion;
    paintEventClipRegion = 0;
//    paintEventSaveRegion = 0;
    paintEventDevice = 0;
}

static QList<QPainter*> *widgetPainterList = 0;

void qwsUpdateActivePainters()
{
    /* ##############
    if ( widgetPainterList ) {
	for (int i = 0; i < widgetPainterList->size(); ++i) {
	    QPainter *ptr = widgetPainterList->at(i);
	    ptr->save();
	    delete ptr->gfx;
	    ptr->gfx = ptr->device()->graphicsContext();
	    ptr->setf( QPainter::VolatileDC );
	    ptr->restore();
	}
    }
    */
}


void qt_draw_background( QPaintEngine *pe, int/* x*/, int /*y*/, int /*w*/,  int /*h*/ )
{
    QWSPaintEngine *p = static_cast<QWSPaintEngine *>(pe);
// // //     XSetForeground( p->d->dpy, p->d->gc, p->d->bg_brush.color().pixel(p->d->scrn) );
// // //     qt_draw_transformed_rect( pp, x, y, w, h, TRUE);
// // //     XSetForeground( p->d->dpy, p->d->gc, p->d->cpen.color().pixel(p->d->scrn) );
}
// ########


class QWSPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWSPaintEngine);
public:
    QWSPaintEnginePrivate() :gfx(0) {}
    QGfx *gfx;
};

#define d d_func()
#define q q_func()

QWSPaintEngine::QWSPaintEngine(QPaintEnginePrivate &dptr, QPaintDevice *pdev)
    : QPaintEngine(dptr, UsesFontEngine)
{
    d->pdev = pdev;
//	qDebug("QWSPaintEngine::QWSPaintEngine");
}

QWSPaintEngine::QWSPaintEngine(QPaintDevice *pdev)
    : QPaintEngine(*(new QWSPaintEnginePrivate), UsesFontEngine)
{
    d->pdev = pdev;
//	qDebug("QWSPaintEngine::QWSPaintEngine");
}

QWSPaintEngine::~QWSPaintEngine()
{
//	qDebug("QWSPaintEngine::~QWSPaintEngine");
}

QGfx *QWSPaintEngine::gfx()
{
    return d->gfx;
}


bool QWSPaintEngine::begin(QPaintDevice *pdev, QPainterState *ps, bool unclipped)
{
    if ( isActive() ) {                         // already active painting
         qWarning( "QWSC::begin: Painter is already active."
                   "\n\tYou must end() the painter before a second begin()" );
	return true;
    }
    if(pdev->devType() == QInternal::Widget &&
       !static_cast<QWidget*>(pdev)->testWState(WState_InPaintEvent)) {
	qWarning("QPainter::begin: Widget painting can only begin as a "
		 "result of a paintEvent");
//	return false;
    }

    Q_ASSERT(d->gfx == 0);

    d->pdev = pdev;
    d->gfx = pdev->graphicsContext();
///    qDebug("QWSPaintEngine::begin %p gfx %p", this, d->gfx);
    setActive(true);

    updatePen(ps);
    updateBrush(ps);
    updateClipRegion(ps);

    return true;
}
bool QWSPaintEngine::end(){
    setActive(false);
///    qDebug("QWSPaintEngine::end %p (gfx %p)", this, d->gfx);
    delete d->gfx;
    d->gfx = 0;
    return true;
}

void QWSPaintEngine::updatePen(QPainterState *ps)
{
    d->gfx->setPen(ps->pen);

//    qDebug("QWSPaintEngine::updatePen");
}
void QWSPaintEngine::updateBrush(QPainterState *ps)
{
    //qDebug("QWSPaintEngine::updateBrush");

    static uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static uchar hor_pat[] = {			// horizontal pattern
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar ver_pat[] = {			// vertical pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar cross_pat[] = {			// cross pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar bdiag_pat[] = {			// backward diagonal pattern
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    static uchar fdiag_pat[] = {			// forward diagonal pattern
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
	0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    static uchar dcross_pat[] = {			// diagonal cross pattern
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
	0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    static uchar *pat_tbl[] = {
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
	dense6_pat, dense7_pat,
	hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    if ( !d->gfx )
	return;

    uchar * pat=0;
    int bs=ps->brush.style();
    int dd=0;
    if( bs>=Dense1Pattern && bs <= DiagCrossPattern ) {
	pat=pat_tbl[ bs-Dense1Pattern ];
	if(bs<=Dense7Pattern)
	    dd=8;
	else if (bs<=CrossPattern)
	    dd=24;
	else
	    dd=16;
    }
    if ( bs == CustomPattern || pat ) {
        QPixmap *pm;
        if ( pat ) {
            QString key="$qt-brush$" + QString::number( bs );
            pm = QPixmapCache::find( key );
            bool del = FALSE;
            if ( !pm ) {                        // not already in pm dict
		pm = new QBitmap( dd, dd, pat, TRUE );
		del = !QPixmapCache::insert( key, pm );
            }
	    if ( ps->brush.d->pixmap )
		delete ps->brush.d->pixmap;
	    ps->brush.d->pixmap = new QPixmap( *pm );
	    if (del) delete pm;
	}
	pm = ps->brush.d->pixmap;
    }

    d->gfx->setBrush(ps->brush);
    d->gfx->setBrushPixmap( ps->brush.d->pixmap );
}

void QWSPaintEngine::updateFont(QPainterState *ps)
{
//    qDebug("QWSPaintEngine::updateFont");
}
void QWSPaintEngine::updateRasterOp(QPainterState *ps)
{
//    qDebug("QWSPaintEngine::updateRasterOp");
        d->gfx->setRop(ps->rasterOp);
}
void QWSPaintEngine::updateBackground(QPainterState *ps)
{
//    qDebug("QWSPaintEngine::updateBackground");
}
void QWSPaintEngine::updateXForm(QPainterState */*ps*/)
{
//    qDebug("QWSPaintEngine::updateXForm");
}
void QWSPaintEngine::updateClipRegion(QPainterState *ps)
{
//    qDebug("QWSPaintEngine::updateClipRegion");

    Q_ASSERT(isActive());

    bool painterClip = ps->clipEnabled;
    bool eventClip = paintEventDevice == d->pdev && paintEventClipRegion;
/*
  if (enable == testf(ClipOn)
  && ( paintEventDevice != device() || !enable
  || !paintEventSaveRegion || paintEventSaveRegion->isNull() ) )
  return;
*/

    if (painterClip || eventClip) {
	QRegion crgn;
	if (painterClip) {
	    crgn = ps->clipRegion;
	    if (eventClip)
		crgn = crgn.intersect(*paintEventClipRegion);
	} else {
	    crgn = *paintEventClipRegion;
	}
	//note that gfx is already translated by redirection_offset
	d->gfx->setClipRegion( crgn );
    } else {
	d->gfx->setClipping( FALSE );
    }
    if (painterClip)
	setf(ClipOn);
    else
	clearf(ClipOn);
}

void QWSPaintEngine::setRasterOp(RasterOp r)
{
    d->gfx->setRop( r );
//    qDebug("QWSPaintEngine::setRasterOp");
}

void QWSPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    if ( state->pen.style() != NoPen )
	d->gfx->drawLine( p1.x(), p1.y(), p2.x(), p2.y() );
}
void QWSPaintEngine::drawRect(const QRect &r)
{
    //############ gfx->setBrushOffset( x-bro.x(), y-bro.y() );

    int x1, y1, w, h;
    r.rect(&x1, &y1, &w, &h);

    if (state->pen.style() != NoPen) {
	if ( state->pen.width() > 1 ) {
	    QPointArray a(r, TRUE);
	    drawPolyInternal( a );
	    return;
	} else	{
	    int x2 = x1 + (w-1);
	    int y2 = y1 + (h-1);
	    d->gfx->drawLine(x1, y1, x2, y1);
	    d->gfx->drawLine(x2, y1, x2, y2);
	    d->gfx->drawLine(x1, y2, x2, y2);
	    d->gfx->drawLine(x1, y1, x1, y2);
	    x1 += 1;
	    y1 += 1;
	    w -= 2;
	    h -= 2;
	}
    }

    d->gfx->fillRect( x1, y1, w, h );
}
void QWSPaintEngine::drawPoint(const QPoint &p)
{
    d->gfx->drawPoint(p.x(), p.y());
}

void QWSPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    if ( state->pen.style() == NoPen )
	return;

    d->gfx->drawPoints( pa, index, npoints );
}

void QWSPaintEngine::drawWinFocusRect(const QRect &r, bool /*xorPaint*/, const QColor &/*bgColor*/)
{
    static char winfocus_line[] = { 1, 1 };

//    d->gfx->setBrush(QBrush());
/*#########
    if ( xorPaint ) {
        if ( QColor::numBitPlanes() <= 8 )
	    d->gfx->setPen(QPen(color1));
	else
	    d->gfx->setPen(QPen(white));
	d->gfx->setRop(XorROP);
    } else {
	if ( qGray( bgColor.rgb() ) < 128 )
	    setPen(QPen(white));
	else
	    setPen(QPen(black));
    }
*/

    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    d->gfx->setDashes(winfocus_line, 2);
    d->gfx->setDashedLines(TRUE);
    if ( state->pen.style() != NoPen ) {
	d->gfx->drawLine(x,y,x+(w-1),y);
	d->gfx->drawLine(x+(w-1),y,x+(w-1),y+(h-1));
	d->gfx->drawLine(x,y+(h-1),x+(w-1),y+(h-1));
	d->gfx->drawLine(x,y,x,y+(h-1));
	x++;
	y++;
	w -= 2;
	h -= 2;
    }
//###?????    gfx->fillRect(x,y,w,h);

    d->gfx->setDashedLines(FALSE);

//     setRasterOp( old_rop );
//     setPen( old_pen );
//     setBrush( old_brush );
}
void QWSPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    QPointArray a;

    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    w--;
    h--;
    int rxx = w*xRnd/200;
    int ryy = h*yRnd/200;
    int rxx2 = 2*rxx;
    int ryy2 = 2*ryy;
    int xx, yy;

    // ###### WWA: this should use the new makeArc (with xmat)

    a.makeEllipse( x, y, rxx2, ryy2 );
    int s = a.size()/4;
    int i = 0;
    while ( i < s ) {
	a.point( i, &xx, &yy );
	xx += w - rxx2;
	a.setPoint( i++, xx, yy );
    }
    i = 2*s;
    while ( i < 3*s ) {
	a.point( i, &xx, &yy );
	yy += h - ryy2;
	a.setPoint( i++, xx, yy );
    }
    while ( i < 4*s ) {
	a.point( i, &xx, &yy );
	xx += w - rxx2;
	yy += h - ryy2;
	a.setPoint( i++, xx, yy );
    }
    //  a = xForm(a);
    //a.translate(-redirection_offset);
    drawPolyInternal( a );
//    qDebug("QWSPaintEngine::drawRoundRect");
}


void QWSPaintEngine::drawPolyInternal( const QPointArray &a, bool close )
{
    if ( a.size() < 2 || !d->gfx )
	return;

    int x1, y1, x2, y2;				// connect last to first point
    a.point( a.size()-1, &x1, &y1 );
    a.point( 0, &x2, &y2 );
    bool do_close = close && !(x1 == x2 && y1 == y2);

    if ( close && state->brush.style() != NoBrush ) {	// draw filled polygon
	d->gfx->drawPolygon(a,FALSE,0,a.size());
	if ( state->pen.style() == NoPen ) {		// draw fake outline
	    d->gfx->drawPolyline(a,0,a.size());
	    if ( do_close )
		d->gfx->drawLine(x1,y1,x2,y2);
	}
    }
    if ( state->pen.style() != NoPen ) {		// draw outline
	d->gfx->drawPolyline(a,0,a.size());
	if ( do_close )
	    d->gfx->drawLine(x1,y1,x2,y2);
    }
}




void QWSPaintEngine::drawEllipse(const QRect &r)
{
    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    QPointArray a;
// #ifndef QT_NO_TRANSFORMATIONS
//     a.makeArc( x, y, w, h, 0, 360*16, xmat );
//     a.translate(-redirection_offset);
// #else
//     map( x, y, &x, &y );
    a.makeEllipse(x, y, w, h);
//#endif
/*###########
    QPen oldpen=pen();
    QPen tmppen=oldpen;
    tmppen.setJoinStyle(BevelJoin);
    setPen(tmppen);
*/
    drawPolyInternal( a );
}

void QWSPaintEngine::drawArc(const QRect &r, int a, int alen)
{
    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    QPointArray pa;
    pa.makeArc( x, y, w, h, a, alen );
    drawPolyInternal( pa, FALSE );
}

void QWSPaintEngine::drawPie(const QRect &r, int a, int alen)
{
    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    QPointArray pa;
// #ifndef QT_NO_TRANSFORMATIONS
//     pa.makeArc( x, y, w, h, a, alen, xmat );	// arc polyline
// #else
//     map( x, y, &x, &y );
    pa.makeArc( x, y, w, h, a, alen );		// arc polyline
//#endif
    int n = pa.size();
    int cx, cy;
// #ifndef QT_NO_TRANSFORMATIONS
//     xmat.map(x+w/2, y+h/2, &cx, &cy);
//#else
    cx = x+w/2;
    cy = y+h/2;
//#endif
    pa.resize( n+2 );
    pa.setPoint( n, cx, cy );			// add legs
    pa.setPoint( n+1, pa.at(0) );
    drawPolyInternal( pa );

}

void QWSPaintEngine::drawChord(const QRect &r, int a, int alen)
{
    int x, y, w, h;
    r.rect(&x, &y, &w, &h);

    QPointArray pa;
    pa.makeArc( x, y, w-1, h-1, a, alen );	// arc polygon
    int n = pa.size();
    pa.resize( n+1 );
    pa.setPoint( n, pa.at(0) );			// connect endpoints
    drawPolyInternal( pa );
}

void QWSPaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    for ( int i=0; i<nlines; i++ ) {
	int x1,y1,x2,y2;
	a.point( index++, &x1, &y1 );
	a.point( index++, &x2, &y2 );
	if ( state->pen.style() != NoPen )
	    d->gfx->drawLine( x1, y1, x2, y2 );
    }
}

void QWSPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    if ( state->pen.style() != NoPen )
	d->gfx->drawPolyline( pa, index, npoints );
}

void QWSPaintEngine::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
{
#if 0
#ifndef QT_NO_TRANSFORMATIONS
	bool tx = (txop != TxNone);
#else
	bool tx = xlatex || xlatey;
#endif
	if ( tx ) {
	    pa = xForm( a, index, npoints );
	    if ( pa.size() != a.size() ) {
		index   = 0;
		npoints = pa.size();
	    }
	    pa.translate(-redirection_offset);
	}

#endif
    d->gfx->drawPolygon( pa, winding, index, npoints );

}

void QWSPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
//    qDebug("QWSPaintEngine::drawConvexPolygon");
    drawPolygon(pa, false, index, npoints);
}

#ifndef QT_NO_BEZIER
void QWSPaintEngine::drawCubicBezier(const QPointArray &pa, int index){
    qDebug("QWSPaintEngine::drawCubicBezier");
}
#endif

void QWSPaintEngine::drawPixmap(const QRect &r, const QPixmap &pixmap, const QRect &sr)
    //(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh)
{
    int x,y,w,h,sx,sy,sw,sh;
    r.rect(&x, &y, &w, &h);
    sr.rect(&sx, &sy, &sw, &sh);

    if ((w != sw || h != sh) && (sx != 0) && (sy != 0))
	qDebug( "QWSPaintEngine::drawPixmap offset stretch notimplemented" );

    //bitBlt( pdev, x, y, &pixmap, sx, sy, sw, sh, CopyROP );
    d->gfx->setSource(&pixmap);
    if(pixmap.mask()) {
	QBitmap * mymask=( (QBitmap *)pixmap.mask() );
	unsigned char * thebits=mymask->scanLine(0);
	int ls=mymask->bytesPerLine();
	d->gfx->setAlphaType(QGfx::LittleEndianMask);
	d->gfx->setAlphaSource(thebits,ls);
    } else if ( pixmap.data->hasAlpha ){
	d->gfx->setAlphaType(QGfx::InlineAlpha);
    } else {
	d->gfx->setAlphaType(QGfx::IgnoreAlpha);
    }
    if (sw == w && sh == h)
	d->gfx->blt(x,y,sw,sh,sx,sy);
    else
	d->gfx->stretchBlt(x,y,w,h,sw,sh);
}

void QWSPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim)
{
    qDebug("QWSPaintEngine::drawTiledPixmap");
}

void QWSPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags){
    ti.fontEngine->draw(this, p.x(),  p.y(), ti, textflags);
}

Qt::HANDLE QWSPaintEngine::handle() const{
    qDebug("QWSPaintEngine::handle");
    return 0;
}


void QWSPaintEngine::initialize(){
	qDebug("QWSPaintEngine::initialize");
}
void QWSPaintEngine::cleanup(){
	qDebug("QWSPaintEngine::cleanup");
}



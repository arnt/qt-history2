/****************************************************************************
** $Id: //depot/qt/main/tests/qimagepaintdevice/qimagepaintdevice.cpp#1 $
**
** Implementation of QImagePaintDevice32 class
**
** Created : 991015
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
 
#include "qpaintdevicemetrics.h"
#include "qimagepaintdevice.h"
#include "qpointarray.h"
#include "qregion.h"
#include "qfont.h"
#include <stdlib.h>

QImagePaintDevice32::QImagePaintDevice32(int w, int h) :
    QPaintDevice( QInternal::System | QInternal::ExternalDevice ),
    img(w,h,32),
    rgb((QRgb**)img.jumpTable())
{
    cliprect = 0;
    renderer = 0;
}

QImagePaintDevice32::~QImagePaintDevice32()
{
}


bool QImagePaintDevice32::cmd( int c, QPainter *, QPDevCmdParam *p )
{
    if ( c ==  PdcBegin ) {
	return TRUE;
    } else if ( c == PdcEnd ) {		// end; calc checksum and close
	return TRUE;
    }

    switch ( c ) {
	case PdcDrawPoint:
	    drawPoint(*p[0].point);
	    break;
	case PdcMoveTo:
	    moveTo(*p[0].point);
	    break;
	case PdcLineTo:
	    lineTo(*p[0].point);
	    break;
	case PdcSetBrushOrigin:
	    setBrushOrigin(*p[0].point);
	    break;

	case PdcDrawLine:
	    drawLine(*p[0].point,*p[1].point);
	    break;

	case PdcDrawRect:
	    drawRect(*p[0].rect);
	    break;
	case PdcDrawEllipse:
	    drawEllipse(*p[0].rect);
	    break;

	case PdcDrawRoundRect:
	    drawRoundRect(*p[0].rect, p[1].ival, p[2].ival);
	    break;
	case PdcDrawArc:
	    drawArc(*p[0].rect, p[1].ival, p[2].ival);
	    break;
	case PdcDrawPie:
	    drawPie(*p[0].rect, p[1].ival, p[2].ival);
	    break;
	case PdcDrawChord:
	    drawChord(*p[0].rect, p[1].ival, p[2].ival);
	    break;

	case PdcDrawLineSegments:
	    drawLineSegments(*p[0].ptarr);
	    break;
	case PdcDrawPolyline:
	    drawPolyline(*p[0].ptarr);
	    break;
	case PdcDrawQuadBezier:
	    drawQuadBezier(*p[0].ptarr);
	    break;

	case PdcDrawPolygon:
	    drawPolygon(*p[0].ptarr, p[1].ival);
	    break;

	case PdcDrawText2:
	    drawText(*p[0].point, *p[1].str);
	    break;

	case PdcDrawText2Formatted:
	    drawTextFormatted(*p[0].rect, p[1].ival, *p[2].str);
	    break;

	case PdcDrawPixmap:
	    drawPixmap(*p[0].point, *p[1].pixmap);
	    break;

	case PdcDrawImage:
	    drawImage(*p[0].point, *p[1].image);
	    break;

	case PdcSave:
	    saveState();
	    break;
	case PdcRestore:
	    restoreState();
	    break;

	case PdcSetBkColor:
	    setBkColor(*p[0].color);
	    break;

	case PdcSetBkMode:
	    setBkMode(p[0].ival);
	    break;
	case PdcSetROP:
	    setROP(p[0].ival);
	    break;

	case PdcSetFont:
	    setFont(*p[0].font);
	    break;

	case PdcSetPen:
	    setPen(*p[0].pen);
	    break;

	case PdcSetBrush:
	    setBrush(*p[0].brush);
	    break;

	case PdcSetTabStops:
	    setTabStops(p[0].ival);
	    break;

	case PdcSetTabArray:
	    setTabArray(p[0].ival, p[1].ivec);
	    break;

	case PdcSetUnit:
	    setUnit(p[0].ival);
	    break;
	case PdcSetVXform:
	    setVXform(p[0].ival);
	    break;
	case PdcSetWXform:
	    setWXform(p[0].ival);
	    break;
	case PdcSetClip:
	    setClip(p[0].ival);
	    break;

	case PdcSetWindow:
	    setWindow(*p[0].rect);
	    break;
	case PdcSetViewport:
	    setViewport(*p[0].rect);
	    break;

	case PdcSetWMatrix:
	    setMatrix(*p[0].matrix, p[1].ival);
	    break;

	case PdcSetClipRegion:
	    setClipRegion(*p[0].rgn);
	    break;

#if defined(CHECK_RANGE)
	default:
	    qWarning( "QImagePaintDevice32::cmd: Command %d not recognized", c );
#endif
    }
    return TRUE;
}

void QImagePaintDevice32::drawPoint(QPoint p)
{
    if (find(p))
	rgb[p.y()][p.x()] = fg;
}

void QImagePaintDevice32::moveTo(QPoint p)
{
    cursor = p;
}

void QImagePaintDevice32::lineTo(QPoint p)
{
    drawLine(cursor,p);
    cursor = p;
}

void QImagePaintDevice32::setBrushOrigin(QPoint p)
{
    brushorg = p;
}


// inlining this doubles speed
inline bool QRect::contains( const QPoint &p, bool proper ) const
{
    if ( proper )
        return p.x() > x1 && p.x() < x2 &&
               p.y() > y1 && p.y() < y2;
    else
        return p.x() >= x1 && p.x() <= x2 &&
               p.y() >= y1 && p.y() <= y2;
}


bool QImagePaintDevice32::find(QPoint p)
{
    if ( cliprect[clipcursor].contains(p) )
	return TRUE;

    // binary search
    int a=0;
    int l=ncliprect-1;
    int h;
    int m;
    while ( l>0 ) {
	h = l/2;
	m = a + h;
	const QRect& r = cliprect[m];
	if ( r.bottom() < p.y() || r.top() <= p.y() && r.right() < p.x() ) {
	    a = m + 1;
	    l = l - h - 1;
	} else
	    l = h;
    }
    clipcursor = a;
    bool y = cliprect[clipcursor].contains(p);
    if (!y && clipcursor) clipcursor--;
    return y;
}

void QImagePaintDevice32::findOutside(int x, int y, QRect& cr)
{
    if ( cr.bottom() < y ) {
	QRect cr1 = clipcursor < (int)ncliprect-1
			? cliprect[clipcursor+1]
			: QRect(x-1000,cr.bottom(),x+1000,y+1000);
	cr.setCoords(x-1000,cr1.top(),cr1.left()-1,cr1.bottom());
    } else if ( cr.top() > y ) {
	cr.setCoords(x-1000,y-1000,x+1000,cr.top()-1);
    } else {
	QRect cr1 = clipcursor < (int)ncliprect-1
			? cliprect[clipcursor+1]
			: QRect(x+1000,cr.y(),1,1);
	if ( cr1.y() == cr.y() ) {
	    cr.setCoords( cr.right()+1, cr.y(),
			       cr1.left()-1, cr.bottom() );
	} else {
	    cr.setCoords( cr.right()+1, cr.y(),
			       cr.right()+1000, cr.bottom() );
	}
    }
}

void QImagePaintDevice32::drawLine(QPoint p0,QPoint p1)
{
    int dx = p1.x()-p0.x();
    int dy = p1.y()-p0.y();
    int xi,yi;
    if ( dx < 0 ) {
	dx = -dx;
	xi = -1;
    } else {
	xi = 1;
	if ( dx == 0 && dy == 0 ) {
	    drawPoint(p0);
	    return;
	}
    }
    if ( dy < 0 ) {
	dy = -dy;
	yi = -1;
    } else {
	yi = 1;
    }
    int x=p0.x(),y=p0.y();
    QRect cr;
    bool plot = find(p0);
    cr = cliprect[clipcursor];
    if ( plot ) {
	rgb[y][x] = fg;
    } else {
	findOutside(x,y,cr);
    }
    if ( dx >= dy ) {
	int r = dx/2;
	do {
	    x += xi;
	    r += dy;
	    if ( r >= dx ) {
		r -= dx;
		y += yi;
	    }
	    if ( !cr.contains(QPoint(x,y)) )
	    {
		plot=find(QPoint(x,y));
		if ( plot )
		    cr = cliprect[clipcursor];
		else
		    findOutside(x,y,cr);
	    }
	    if ( plot )
		rgb[y][x] = fg;
	} while (x != p1.x());
    } else {
	int r = dy/2;
	do {
	    y += yi;
	    r += dx;
	    if ( r >= dy ) {
		r -= dy;
		x += xi;
	    }
	    if ( !cr.contains(QPoint(x,y)) )
	    {
		plot=find(QPoint(x,y));
		if ( plot )
		    cr = cliprect[clipcursor];
		else
		    findOutside(x,y,cr);
	    }
	    if ( plot )
		rgb[y][x] = fg;
	} while (y != p1.y());
    }
}

void QImagePaintDevice32::drawRect(const QRect& r)
{
    int x=r.left(),y=r.top();
    QRect cr;
    bool plot = find(QPoint(x,y));
    cr = cliprect[clipcursor];
    if ( !plot )
	findOutside(x,y,cr);
    for (int j=0; j<r.height(); j++) {
	QRgb* line = rgb[y];
	x = r.left();
	if ( !cr.contains(QPoint(x,y)) )
	{
	    plot=find(QPoint(x,y));
	    if ( plot )
		cr = cliprect[clipcursor];
	    else
		findOutside(x,y,cr);
	}
	int cx1 = cr.right();
	for (int i=0; i<r.width(); i++) {
	    if ( x > cx1 )
	    {
		plot=find(QPoint(x,y));
		if ( plot )
		    cr = cliprect[clipcursor];
		else
		    findOutside(x,y,cr);
		cx1 = cr.right();
	    }
	    if ( plot )
		line[x] = fg;
	    x++;
	}
	y++;
    }
}

void QImagePaintDevice32::drawEllipse(const QRect& r)
{
    drawPie(r,0,360*64);
}

void QImagePaintDevice32::drawRoundRect(const QRect& r, int i1, int i2)
{
    // XXX
}

void QImagePaintDevice32::drawArc(const QRect& r, int i1, int i2)
{
    // XXX
}

void QImagePaintDevice32::drawPie(const QRect& r, int i1, int i2)
{
    // XXX building a polygon might be fastest for non-360
}

void QImagePaintDevice32::drawChord(const QRect& r, int i1, int i2)
{
    // XXX
}

void QImagePaintDevice32::drawLineSegments(const QPointArray& pa)
{
    int m=pa.size()-1;
    for (int i=0; i<m; i+=2)
	drawLine(pa[i],pa[i+1]);
}

void QImagePaintDevice32::drawPolyline(const QPointArray& pa)
{
    int m=pa.size()-1;
    for (int i=0; i<m; i++)
	drawLine(pa[i],pa[i+1]); // XXX beware XOR mode vertices
}

void QImagePaintDevice32::drawQuadBezier(const QPointArray& pa)
{
    // XXX
}


/* Based on:
 *
 * Concave Polygon Scan Conversion
 * by Paul Heckbert
 * from "Graphics Gems", Academic Press, 1990
 */

// XXX need to use breseham-line edges, not this double stuff...

typedef struct {		/* a polygon edge */
    double x;	/* x coordinate of edge's intersection with current scanline */
    double dx;	/* change in x with respect to y */
    int i;	/* edge number: edge i goes from pt[i] to pt[i+1] */
} Edge;

static QPointArray pt;		/* vertices */

static int nact;		/* number of active edges */
static Edge *active;		/* active edge list:edges crossing scanline y */

static void delete_edge(int i)		/* remove edge i from active list */
{
    int j;

    for (j=0; j<nact && active[j].i!=i; j++);
    if (j>=nact) return;	/* edge not in active list; happens at win->y0*/
    nact--;
    memcpy(&active[j], &active[j+1], (nact-j)*sizeof(active[0]));
}

static void insert_edge(int i, int y)		/* append edge i to end of active list */
{
    int j;
    double dx;
    QPoint p, q;

    j = i<(int)pt.size()-1 ? i+1 : 0;
    if (pt[i].y() < pt[j].y()) {p = pt[i]; q = pt[j];}
    else		   {p = pt[j]; q = pt[i];}
    /* initialize x position at intersection of edge with scanline y */
    active[nact].dx = dx = double(q.x()-p.x())/(q.y()-p.y());
    active[nact].x = dx*(y+.5-p.y())+p.x();
    active[nact].i = i;
    nact++;
}

/* comparison routines for qsort */
static int compare_ind(const void *u, const void *v)
{return pt[*((int*)u)].y() <= pt[*((int*)v)].y() ? -1 : 1;}
static int compare_active(const void *u, const void *v)
{return ((Edge*)u)->x <= ((Edge*)v)->x ? -1 : 1;}

void QImagePaintDevice32::drawPolygon(const QPointArray& pa, int winding)
{
    int k, y0, y1, y, i, j, xl, xr;
    int *ind;		/* list of vertex indices, sorted by pt[ind[j]].y */

    pt = pa;
    int n = (int)pa.size();
    if (n<=0) return;

    ind = new int[n];
    active = new Edge[n];

    /* create y-sorted array of indices ind[k] into vertex list */
    for (k=0; k<n; k++)
	ind[k] = k;
    qsort(ind, n, sizeof(ind[0]), compare_ind);	/* sort ind by pt[ind[k]].y */

    nact = 0;				/* start with empty active list */
    k = 0;				/* ind[k] is next vertex to process */
    y0 = pt[ind[0]].y();		/* ymin of polygon */
    y1 = pt[ind[n-1]].y()-1;		/* ymax of polygon */
    if ( y0 < 0 ) y0 = 0;
    if ( y1 >= img.height() ) y1 = img.height()-1;

    for (y=y0; y<=y1; y++) {		/* step through scanlines */
	/* scanline y is at y+.5 in continuous coordinates */

	/* check vertices between previous scanline and current one, if any */
	for (; k<n && pt[ind[k]].y()<=y; k++) {
	    /* to simplify, if pt.y()=y+.5, pretend it's above */
	    /* invariant: y-.5 < pt[i].y() <= y+.5 */
	    i = ind[k];	
	    /*
	     * insert or delete edges before and after vertex i (i-1 to i,
	     * and i to i+1) from active list if they cross scanline y
	     */
	    j = i>0 ? i-1 : n-1;	/* vertex previous to i */
	    if (pt[j].y() < y)	/* old edge, remove from active list */
		delete_edge(j);
	    else if (pt[j].y() > y)	/* new edge, add to active list */
		insert_edge(j, y);
	    j = i<n-1 ? i+1 : 0;	/* vertex next after i */
	    if (pt[j].y() < y)	/* old edge, remove from active list */
		delete_edge(i);
	    else if (pt[j].y() > y)	/* new edge, add to active list */
		insert_edge(i, y);
	}

	/* sort active edge list by active[j].x */
	qsort(active, nact, sizeof(active[0]), compare_active);

	/* draw horizontal segments for scanline y */
	for (j=0; j<nact; j+=2) {	/* draw horizontal segments */
	    /* span 'tween j & j+1 is inside, span tween j+1 & j+2 is outside */
	    xl = int(active[j].x+.5);		/* left end of span */
	    xr = int(active[j+1].x-.5);	/* right end of span */
	    if ( xl < 0 ) xl = 0;
	    if ( xr >= img.width() ) xr = img.width()-1;
	    if (xl<=xr) {
		drawLine(QPoint(xl,y),QPoint(xr,y));
	    }
	    active[j].x += active[j].dx;	/* increment edge coords */
	    active[j+1].x += active[j+1].dx;
	}
    }
    delete [] ind;
    delete [] active;
}

void QImagePaintDevice32::drawPixmap(QPoint p, QPixmap pm)
{
    // XXX
}

void QImagePaintDevice32::drawImage(QPoint p, const QImage& im)
{
    // XXX
}

void QImagePaintDevice32::saveState()
{
    // XXX
}

void QImagePaintDevice32::restoreState()
{
    // XXX
}

void QImagePaintDevice32::setBkColor(const QColor& c)
{
    // XXX
}

void QImagePaintDevice32::setBkMode(int i)
{
    // XXX
}

void QImagePaintDevice32::setROP(int i)
{
    // XXX
}

void QImagePaintDevice32::setPen(const QPen& p)
{
    pen = p;
    fg = p.color().rgb();
    // XXX
}

void QImagePaintDevice32::setBrush(const QBrush& br)
{
    brush = br;
    // XXX
}

void QImagePaintDevice32::setTabStops(int i)
{
    // XXX
}

void QImagePaintDevice32::setTabArray(int i, int* ivec)
{
    // XXX
}

void QImagePaintDevice32::setUnit(int i)
{
    // XXX
}

void QImagePaintDevice32::setVXform(int i)
{
    // XXX
}

void QImagePaintDevice32::setWXform(int i)
{
    // XXX
}

void QImagePaintDevice32::setClip(int i)
{
    // XXX
}

void QImagePaintDevice32::setWindow(const QRect& r)
{
    // XXX
}

void QImagePaintDevice32::setViewport(const QRect& r)
{
    // XXX
}

void QImagePaintDevice32::setMatrix(const QWMatrix& matrix, int i)
{
    // XXX
}

void QImagePaintDevice32::setClipRegion(const QRegion& rgn)
{
    QArray<QRect> a = rgn.rects();
    delete cliprect;
    cliprect = new QRect[a.size()];
    for (int i=0; i<(int)a.size(); i++)
	cliprect[i]=a[i];
    ncliprect = a.size();
    clipcursor = 0;
}


int  QImagePaintDevice32::metric( int m ) const
{
    int val;
    switch ( m ) {
	case QPaintDeviceMetrics::PdmWidth:
	    val = img.width();
	    break;
	case QPaintDeviceMetrics::PdmHeight:
	    val = img.height();
	    break;
	case QPaintDeviceMetrics::PdmWidthMM:
	    val = img.width()*1000/img.dotsPerMeterX();
	    break;
	case QPaintDeviceMetrics::PdmHeightMM:
	    val = img.height()*1000/img.dotsPerMeterY();
	    break;
	case QPaintDeviceMetrics::PdmDpiX:
	    val = int(img.dotsPerMeterX()*0.0254+0.5);
	    break;
	case QPaintDeviceMetrics::PdmDpiY:
	    val = int(img.dotsPerMeterY()*0.0254+0.5);
	    break;
	case QPaintDeviceMetrics::PdmNumColors:
	    val = img.depth() == 32 ? 256*256*256 : img.numColors();
	    break;
	case QPaintDeviceMetrics::PdmDepth:
	    val = img.depth();
	    break;
	default:
	    val = 0;
#if defined(CHECK_RANGE)
	    qWarning( "QImagePaintDevice32::metric: Invalid metric command" );
#endif
    }
    return val;
}

void QImagePaintDevice32::blit( int x, int y, const char* data, int w, int h )
{
    // (x,y) is to destination point in the image
    // data is values 0..4 indicating transparent to opaque
    // w by h is the size of data

    // Code is like drawRect()

    QRect cr;
    bool plot = find(QPoint(x,y));
    cr = cliprect[clipcursor];
    if ( !plot )
	findOutside(x,y,cr);
    for (int j=0; j<h; j++) {
	QRgb* line = rgb[y]+x;
	if ( !cr.contains(QPoint(x,y)) )
	{
	    plot=find(QPoint(x,y));
	    if ( plot )
		cr = cliprect[clipcursor];
	    else
		findOutside(x,y,cr);
	}
	int cx1 = cr.right();
	for (int i=0; i<w; i++) {
	    if ( x > cx1 )
	    {
		plot=find(QPoint(x,y));
		if ( plot )
		    cr = cliprect[clipcursor];
		else
		    findOutside(x,y,cr);
		cx1 = cr.right();
	    }
	    if ( plot ) {
		switch ( *data ) {
		  case 1:
		    *line = qRgb (
			(qRed(*line)*3 + qRed(fg))/4,
			(qGreen(*line)*3 + qGreen(fg))/4,
			(qBlue(*line)*3 + qBlue(fg))/4
		    );
		    break;
		  case 2:
		    *line = qRgb (
			(qRed(*line) + qRed(fg))/2,
			(qGreen(*line) + qGreen(fg))/2,
			(qBlue(*line) + qBlue(fg))/2
		    );
		    break;
		  case 3:
		    *line = qRgb (
			(qRed(*line) + qRed(fg)*3)/4,
			(qGreen(*line) + qGreen(fg)*3)/4,
			(qBlue(*line) + qBlue(fg)*3)/4
		    );
		    break;
		  case 4:
		    *line = fg;
		    break;
		  default: /* including case 0 */
		    ;
		}
	    }
	    line++;
	    data++;
	    x++;
	}
	y++;
	x-=w;
    }
}

void QImagePaintDevice32::blit( const QPoint& p, const QTMap* tm )
{
    blit( p.x()+tm->offset().x(), p.y()+tm->offset().y(),
	tm->data(), tm->width(), tm->height() );
}

void QImagePaintDevice32::drawText(QPoint p, const QString& s)
{
    p *= 64;
    for (int i=0; i<(int)s.length(); i++) {
	QTMap *tm = renderer->mapFor( s[i].unicode() );
	blit(p/64, tm);
	p += tm->advance();
    }
}

void QImagePaintDevice32::drawTextFormatted(const QRect& r, int i, const QString& s)
{
    // XXX use qt_format_text
}

void QImagePaintDevice32::setFont( const QFont& f )
{
    delete renderer; // XXX no caching yet
    renderer = qt_font_renderer_ttf(f);
    // XXX try other renderers...
    if ( !renderer ) {
	qFatal("No fonts available (trying to load %s)", f.family().latin1());
    }
}

QFontRenderer::~QFontRenderer()
{
}

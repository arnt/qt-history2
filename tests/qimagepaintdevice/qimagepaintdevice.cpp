/****************************************************************************
** $Id: $
**
** Implementation of QImagePaintDevice32 class
**
** Created : 991015
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qpaintdevicemetrics.h"
#include "qimagepaintdevice.h"
#include "qpainter.h"
#include "qpointarray.h"
#include "qregion.h"
#include "qfontmetrics.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86dga.h>

QImagePaintDevice32::QImagePaintDevice32(uchar* addr, int width, int height) :
    QPaintDevice( QInternal::System | QInternal::ExternalDevice )
    img( addr, width, height, 32, 0, 0, QImage::BigEndian )
{
    init();
}

QImagePaintDevice32::QImagePaintDevice32(int w, int h) :
    QPaintDevice( QInternal::System | QInternal::ExternalDevice ),
    img(w,h,32)
{
    init();
}

void QImagePaintDevice32::init()
{
    rgb = (QRgb**)img.jumpTable();
    renderer = 0;
    cliprect = cliprect1 = new QRect[1];
    cliprect[0] = img.rect();
    ncliprect = ncliprect1 = 1;
    ocliprect = 0;
    oncliprect = 0;
    clipcursor = 0;
    clipon = 0;
}

QImagePaintDevice32::~QImagePaintDevice32()
{
    delete [] cliprect;
}


bool QImagePaintDevice32::cmd( int c, QPainter *painter, QPDevCmdParam *p )
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
	case PdcDrawCubicBezier:
	    drawCubicBezier(*p[0].ptarr);
	    break;

	case PdcDrawPolygon:
	    drawPolygon(*p[0].ptarr, p[1].ival);
	    break;

	case PdcDrawText2:
	    drawText(*p[0].point, *p[1].str);
	    break;

	case PdcDrawText2Formatted: {
	    QFontMetrics fm(font);
	    qt_format_text( fm,
		p[0].rect->x(), p[0].rect->y(), p[0].rect->width(), p[0].rect->height(),
		p[1].ival, *p[2].str, p[2].str->length(), 0, 0, 0, 0, 0, painter );
	    } break;

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

#if defined(QT_CHECK_RANGE)
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
    if ( cliprect[clipcursor].contains(p) ) {
	return TRUE;
    }

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
    if (!y && clipcursor) clipcursor--; // HACK: want rect BEFORE p
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
		line[x] = br;
	    x++;
	}
	y++;
    }
}

void QImagePaintDevice32::drawEllipse(const QRect& r)
{
    drawPie(r,0,360*64);
}

void QImagePaintDevice32::drawRoundRect(const QRect&, int, int)
{
    // XXX
}

void QImagePaintDevice32::drawArc(const QRect&, int, int)
{
    // XXX
}

void QImagePaintDevice32::drawPie(const QRect&, int, int)
{
    // XXX building a polygon might be fastest for non-360
}

void QImagePaintDevice32::drawChord(const QRect&, int, int)
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

void QImagePaintDevice32::drawCubicBezier(const QPointArray &pa )
{
    drawPolyline( pa.cubicBezier() );
}

// Based on Xserver code miFillGeneralPoly...
/*
 *
 *     Written by Brian Kelleher;  Oct. 1985
 *
 *     Routine to fill a polygon.  Two fill rules are
 *     supported: frWINDING and frEVENODD.
 *
 *     See fillpoly.h for a complete description of the algorithm.
 */

/*
 *     These are the data structures needed to scan
 *     convert regions.  Two different scan conversion
 *     methods are available -- the even-odd method, and
 *     the winding number method.
 *     The even-odd rule states that a point is inside
 *     the polygon if a ray drawn from that point in any
 *     direction will pass through an odd number of
 *     path segments.
 *     By the winding number rule, a point is decided
 *     to be inside the polygon if a ray drawn from that
 *     point in any direction passes through a different
 *     number of clockwise and counter-clockwise path
 *     segments.
 *
 *     These data structures are adapted somewhat from
 *     the algorithm in (Foley/Van Dam) for scan converting
 *     polygons.
 *     The basic algorithm is to start at the top (smallest y)
 *     of the polygon, stepping down to the bottom of
 *     the polygon by incrementing the y coordinate.  We
 *     keep a list of edges which the current scanline crosses,
 *     sorted by x.  This list is called the Active Edge Table (AET)
 *     As we change the y-coordinate, we update each entry in
 *     in the active edge table to reflect the edges new xcoord.
 *     This list must be sorted at each scanline in case
 *     two edges intersect.
 *     We also keep a data structure known as the Edge Table (ET),
 *     which keeps track of all the edges which the current
 *     scanline has not yet reached.  The ET is basically a
 *     list of ScanLineList structures containing a list of
 *     edges which are entered at a given scanline.  There is one
 *     ScanLineList per scanline at which an edge is entered.
 *     When we enter a new edge, we move it from the ET to the AET.
 *
 *     From the AET, we can implement the even-odd rule as in
 *     (Foley/Van Dam).
 *     The winding number rule is a little trickier.  We also
 *     keep the EdgeTableEntries in the AET linked by the
 *     nextWETE (winding EdgeTableEntry) link.  This allows
 *     the edges to be linked just as before for updating
 *     purposes, but only uses the edges linked by the nextWETE
 *     link as edges representing spans of the polygon to
 *     drawn (as with the even-odd rule).
 */

/*
 * for the winding number rule
 */
#define CLOCKWISE          1
#define COUNTERCLOCKWISE  -1

/* $XConsortium: miscanfill.h,v 1.5 94/04/17 20:27:50 dpw Exp $ */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/


/*
 *     scanfill.h
 *
 *     Written by Brian Kelleher; Jan 1985
 *
 *     This file contains a few macros to help track
 *     the edge of a filled object.  The object is assumed
 *     to be filled in scanline order, and thus the
 *     algorithm used is an extension of Bresenham's line
 *     drawing algorithm which assumes that y is always the
 *     major axis.
 *     Since these pieces of code are the same for any filled shape,
 *     it is more convenient to gather the library in one
 *     place, but since these pieces of code are also in
 *     the inner loops of output primitives, procedure call
 *     overhead is out of the question.
 *     See the author for a derivation if needed.
 */

/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */
#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
    int dx;      /* local storage */ \
\
    /* \
     *  if the edge is horizontal, then it is ignored \
     *  and assumed not to be processed.  Otherwise, do this stuff. \
     */ \
    if ((dy) != 0) { \
        xStart = (x1); \
        dx = (x2) - xStart; \
        if (dx < 0) { \
            m = dx / (dy); \
            m1 = m - 1; \
            incr1 = -2 * dx + 2 * (dy) * m1; \
            incr2 = -2 * dx + 2 * (dy) * m; \
            d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
        } else { \
            m = dx / (dy); \
            m1 = m + 1; \
            incr1 = 2 * dx - 2 * (dy) * m1; \
            incr2 = 2 * dx - 2 * (dy) * m; \
            d = -2 * m * (dy) + 2 * dx; \
        } \
    } \
}

#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
    if (m1 > 0) { \
        if (d > 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } else {\
        if (d >= 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } \
}


/*
 *     This structure contains all of the information needed
 *     to run the bresenham algorithm.
 *     The variables may be hardcoded into the declarations
 *     instead of using this structure to make use of
 *     register declarations.
 */
typedef struct {
    int minor;         /* minor axis        */
    int d;           /* decision variable */
    int m, m1;       /* slope and slope+1 */
    int incr1, incr2; /* error increments */
} BRESINFO;


#define BRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
	BRESINITPGON(dmaj, min1, min2, bres.minor, bres.d, \
                     bres.m, bres.m1, bres.incr1, bres.incr2)

#define BRESINCRPGONSTRUCT(bres) \
        BRESINCRPGON(bres.d, bres.minor, bres.m, bres.m1, bres.incr1, bres.incr2)


typedef struct _EdgeTableEntry {
     int ymax;             /* ycoord at which we exit this edge. */
     BRESINFO bres;        /* Bresenham info to run the edge     */
     struct _EdgeTableEntry *next;       /* next in the list     */
     struct _EdgeTableEntry *back;       /* for insertion sort   */
     struct _EdgeTableEntry *nextWETE;   /* for winding num rule */
     int ClockWise;        /* flag for winding number rule       */
} EdgeTableEntry;


typedef struct _ScanLineList{
     int scanline;              /* the scanline represented */
     EdgeTableEntry *edgelist;  /* header node              */
     struct _ScanLineList *next;  /* next in the list       */
} ScanLineList;


typedef struct {
     int ymax;                 /* ymax for the polygon     */
     int ymin;                 /* ymin for the polygon     */
     ScanLineList scanlines;   /* header node              */
} EdgeTable;


/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
     ScanLineList SLLs[SLLSPERBLOCK];
     struct _ScanLineListBlock *next;
} ScanLineListBlock;

/*
 * number of points to buffer before sending them off
 * to scanlines() :  Must be an even number
 */
#define NUMPTSTOBUFFER 200

/*
 *
 *     a few macros for the inner loops of the fill code where
 *     performance considerations don't allow a procedure call.
 *
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The winding number rule is in effect, so we must notify
 *     the caller when the edge has been removed so he
 *     can reorder the Winding Active Edge Table.
 */
#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      fixWAET = 1; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres); \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}


/*
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The even-odd rule is in effect.
 */
#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres); \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}

/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#define MAXINT 0x7fffffff
#define MININT -MAXINT

/*
 *     fillUtils.c
 *
 *     Written by Brian Kelleher;  Oct. 1985
 *
 *     This module contains all of the utility functions
 *     needed to scan convert a polygon.
 *
 */
/*
 *     InsertEdgeInET
 *
 *     Insert the given edge into the edge table.
 *     First we must find the correct bucket in the
 *     Edge table, then find the right slot in the
 *     bucket.  Finally, we can insert it.
 *
 */
bool
miInsertEdgeInET(EdgeTable *ET, EdgeTableEntry *ETE,
	int scanline, ScanLineListBlock **SLLBlock, int *iSLLBlock)
{
    register EdgeTableEntry *start, *prev;
    register ScanLineList *pSLL, *pPrevSLL;
    ScanLineListBlock *tmpSLLBlock;

    /*
     * find the right bucket to put the edge into
     */
    pPrevSLL = &ET->scanlines;
    pSLL = pPrevSLL->next;
    while (pSLL && (pSLL->scanline < scanline))
    {
        pPrevSLL = pSLL;
        pSLL = pSLL->next;
    }

    /*
     * reassign pSLL (pointer to ScanLineList) if necessary
     */
    if ((!pSLL) || (pSLL->scanline > scanline))
    {
        if (*iSLLBlock > SLLSPERBLOCK-1)
        {
            tmpSLLBlock =
		  (ScanLineListBlock *)malloc(sizeof(ScanLineListBlock));
	    if (!tmpSLLBlock)
		return FALSE;
            (*SLLBlock)->next = tmpSLLBlock;
            tmpSLLBlock->next = (ScanLineListBlock *)NULL;
            *SLLBlock = tmpSLLBlock;
            *iSLLBlock = 0;
        }
        pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);

        pSLL->next = pPrevSLL->next;
        pSLL->edgelist = (EdgeTableEntry *)NULL;
        pPrevSLL->next = pSLL;
    }
    pSLL->scanline = scanline;

    /*
     * now insert the edge in the right bucket
     */
    prev = (EdgeTableEntry *)NULL;
    start = pSLL->edgelist;
    while (start && (start->bres.minor < ETE->bres.minor))
    {
        prev = start;
        start = start->next;
    }
    ETE->next = start;

    if (prev)
        prev->next = ETE;
    else
        pSLL->edgelist = ETE;
    return TRUE;
}

/*
 *     CreateEdgeTable
 *
 *     This routine creates the edge table for
 *     scan converting polygons.
 *     The Edge Table (ET) looks like:
 *
 *    EdgeTable
 *     --------
 *    |  ymax  |        ScanLineLists
 *    |scanline|-->------------>-------------->...
 *     --------   |scanline|   |scanline|
 *                |edgelist|   |edgelist|
 *                ---------    ---------
 *                    |             |
 *                    |             |
 *                    V             V
 *              list of ETEs   list of ETEs
 *
 *     where ETE is an EdgeTableEntry data structure,
 *     and there is one ScanLineList per scanline at
 *     which an edge is initially entered.
 *
 */

typedef struct {
    int x, y;
} DDXPointRec, *DDXPointPtr;

/*
 *     Clean up our act.
 */
void
miFreeStorage(ScanLineListBlock   *pSLLBlock)
{
    register ScanLineListBlock   *tmpSLLBlock;

    while (pSLLBlock)
    {
        tmpSLLBlock = pSLLBlock->next;
        free(pSLLBlock);
        pSLLBlock = tmpSLLBlock;
    }
}

bool
miCreateETandAET(int count, DDXPointPtr pts, EdgeTable *ET,
	EdgeTableEntry *AET, EdgeTableEntry *pETEs, ScanLineListBlock *pSLLBlock)
{
    register DDXPointPtr top, bottom;
    register DDXPointPtr PrevPt, CurrPt;
    int iSLLBlock = 0;

    int dy;

    if (count < 2)  return TRUE;

    /*
     *  initialize the Active Edge Table
     */
    AET->next = (EdgeTableEntry *)NULL;
    AET->back = (EdgeTableEntry *)NULL;
    AET->nextWETE = (EdgeTableEntry *)NULL;
    AET->bres.minor = MININT;

    /*
     *  initialize the Edge Table.
     */
    ET->scanlines.next = (ScanLineList *)NULL;
    ET->ymax = MININT;
    ET->ymin = MAXINT;
    pSLLBlock->next = (ScanLineListBlock *)NULL;

    PrevPt = &pts[count-1];

    /*
     *  for each vertex in the array of points.
     *  In this loop we are dealing with two vertices at
     *  a time -- these make up one edge of the polygon.
     */
    while (count--)
    {
        CurrPt = pts++;

        /*
         *  find out which point is above and which is below.
         */
        if (PrevPt->y > CurrPt->y)
        {
            bottom = PrevPt, top = CurrPt;
            pETEs->ClockWise = 0;
        }
        else
        {
            bottom = CurrPt, top = PrevPt;
            pETEs->ClockWise = 1;
        }

        /*
         * don't add horizontal edges to the Edge table.
         */
        if (bottom->y != top->y)
        {
            pETEs->ymax = bottom->y-1;  /* -1 so we don't get last scanline */

            /*
             *  initialize integer edge algorithm
             */
            dy = bottom->y - top->y;
            BRESINITPGONSTRUCT(dy, top->x, bottom->x, pETEs->bres);

            if (!miInsertEdgeInET(ET, pETEs, top->y, &pSLLBlock, &iSLLBlock))
	    {
		miFreeStorage(pSLLBlock->next);
		return FALSE;
	    }

            ET->ymax = QMAX(ET->ymax, PrevPt->y);
            ET->ymin = QMIN(ET->ymin, PrevPt->y);
            pETEs++;
        }

        PrevPt = CurrPt;
    }
    return TRUE;
}

/*
 *     loadAET
 *
 *     This routine moves EdgeTableEntries from the
 *     EdgeTable into the Active Edge Table,
 *     leaving them sorted by smaller x coordinate.
 *
 */

void
miloadAET(EdgeTableEntry *AET, EdgeTableEntry *ETEs)
{
    register EdgeTableEntry *pPrevAET;
    register EdgeTableEntry *tmp;

    pPrevAET = AET;
    AET = AET->next;
    while (ETEs)
    {
        while (AET && (AET->bres.minor < ETEs->bres.minor))
        {
            pPrevAET = AET;
            AET = AET->next;
        }
        tmp = ETEs->next;
        ETEs->next = AET;
        if (AET)
            AET->back = ETEs;
        ETEs->back = pPrevAET;
        pPrevAET->next = ETEs;
        pPrevAET = ETEs;

        ETEs = tmp;
    }
}

/*
 *     computeWAET
 *
 *     This routine links the AET by the
 *     nextWETE (winding EdgeTableEntry) link for
 *     use by the winding number rule.  The final
 *     Active Edge Table (AET) might look something
 *     like:
 *
 *     AET
 *     ----------  ---------   ---------
 *     |ymax    |  |ymax    |  |ymax    |
 *     | ...    |  |...     |  |...     |
 *     |next    |->|next    |->|next    |->...
 *     |nextWETE|  |nextWETE|  |nextWETE|
 *     ---------   ---------   ^--------
 *         |                   |       |
 *         V------------------->       V---> ...
 *
 */
void
micomputeWAET(EdgeTableEntry *AET)
{
    register EdgeTableEntry *pWETE;
    register int inside = 1;
    register int isInside = 0;

    AET->nextWETE = (EdgeTableEntry *)NULL;
    pWETE = AET;
    AET = AET->next;
    while (AET)
    {
        if (AET->ClockWise)
            isInside++;
        else
            isInside--;

        if ((!inside && !isInside) ||
            ( inside &&  isInside))
        {
            pWETE->nextWETE = AET;
            pWETE = AET;
            inside = !inside;
        }
        AET = AET->next;
    }
    pWETE->nextWETE = (EdgeTableEntry *)NULL;
}

/*
 *     InsertionSort
 *
 *     Just a simple insertion sort using
 *     pointers and back pointers to sort the Active
 *     Edge Table.
 *
 */

int
miInsertionSort(EdgeTableEntry *AET)
{
    register EdgeTableEntry *pETEchase;
    register EdgeTableEntry *pETEinsert;
    register EdgeTableEntry *pETEchaseBackTMP;
    register int changed = 0;

    AET = AET->next;
    while (AET)
    {
        pETEinsert = AET;
        pETEchase = AET;
        while (pETEchase->back->bres.minor > AET->bres.minor)
            pETEchase = pETEchase->back;

        AET = AET->next;
        if (pETEchase != pETEinsert)
        {
            pETEchaseBackTMP = pETEchase->back;
            pETEinsert->back->next = AET;
            if (AET)
                AET->back = pETEinsert->back;
            pETEinsert->next = pETEchase;
            pETEchase->back->next = pETEinsert;
            pETEchase->back = pETEinsert;
            pETEinsert->back = pETEchaseBackTMP;
            changed = 1;
        }
    }
    return(changed);
}

void QImagePaintDevice32::fillSpans(int n, QPoint* pt, int* w)
{
    int x=pt[0].x(),y=pt[0].y();
    QRect cr;
    bool plot = find(QPoint(x,y));
    cr = cliprect[clipcursor];
    if ( !plot )
	findOutside(x,y,cr);
    for (int j=0; j<n; j++) {
	x = pt[j].x();
	y = pt[j].y();
	QRgb* line = rgb[y];
	if ( !cr.contains(pt[j]) )
	{
	    plot=find(pt[j]);
	    if ( plot )
		cr = cliprect[clipcursor];
	    else
		findOutside(x,y,cr);
	}
	int cx1 = cr.right();
	for (int i=0; i<w[j]; i++) {
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
    }
}

void QImagePaintDevice32::drawPolygon(const QPointArray& pa, int winding)
{
    DDXPointPtr ptsIn = (DDXPointPtr)pa.data();
    register EdgeTableEntry *pAET;  /* the Active Edge Table   */
    register int y;                 /* the current scanline    */
    register int nPts = 0;          /* number of pts in buffer */
    register EdgeTableEntry *pWETE; /* Winding Edge Table      */
    register ScanLineList *pSLL;    /* Current ScanLineList    */
    register DDXPointPtr ptsOut;      /* ptr to output buffers   */
    int *width;
    DDXPointRec FirstPoint[NUMPTSTOBUFFER]; /* the output buffers */
    int FirstWidth[NUMPTSTOBUFFER];
    EdgeTableEntry *pPrevAET;       /* previous AET entry      */
    EdgeTable ET;                   /* Edge Table header node  */
    EdgeTableEntry AET;             /* Active ET header node   */
    EdgeTableEntry *pETEs;          /* Edge Table Entries buff */
    ScanLineListBlock SLLBlock;     /* header for ScanLineList */
    int fixWAET = 0;

    if (pa.size() < 3)
	return;

    if(!(pETEs = (EdgeTableEntry *)
        malloc(sizeof(EdgeTableEntry) * pa.size())))
	return;
    ptsOut = FirstPoint;
    width = FirstWidth;
    if (!miCreateETandAET(pa.size(), ptsIn, &ET, &AET, pETEs, &SLLBlock))
    {
	free(pETEs);
	return;
    }
    pSLL = ET.scanlines.next;

    if (winding==0)
    {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++)
        {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;

            /*
             *  for each active edge
             */
            while (pAET)
            {
                ptsOut->x = pAET->bres.minor;
		ptsOut++->y = y;
                *width++ = pAET->next->bres.minor - pAET->bres.minor;
                nPts++;

                /*
                 *  send out the buffer when its full
                 */
                if (nPts == NUMPTSTOBUFFER)
		{
		    fillSpans( nPts, (QPoint*)FirstPoint, FirstWidth );
                    ptsOut = FirstPoint;
                    width = FirstWidth;
                    nPts = 0;
                }
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y);
            }
            miInsertionSort(&AET);
        }
    }
    else      /* default to WindingNumber */
    {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++)
        {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                micomputeWAET(&AET);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;
            pWETE = pAET;

            /*
             *  for each active edge
             */
            while (pAET)
            {
                /*
                 *  if the next edge in the active edge table is
                 *  also the next edge in the winding active edge
                 *  table.
                 */
                if (pWETE == pAET)
                {
                    ptsOut->x = pAET->bres.minor;
		    ptsOut++->y = y;
                    *width++ = pAET->nextWETE->bres.minor - pAET->bres.minor;
                    nPts++;

                    /*
                     *  send out the buffer
                     */
                    if (nPts == NUMPTSTOBUFFER)
                    {
			fillSpans( nPts, (QPoint*)FirstPoint, FirstWidth );
                        ptsOut = FirstPoint;
                        width  = FirstWidth;
                        nPts = 0;
                    }

                    pWETE = pWETE->nextWETE;
                    while (pWETE != pAET)
                        EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
                    pWETE = pWETE->nextWETE;
                }
                EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
            }

            /*
             *  reevaluate the Winding active edge table if we
             *  just had to resort it or if we just exited an edge.
             */
            if (miInsertionSort(&AET) || fixWAET)
            {
                micomputeWAET(&AET);
                fixWAET = 0;
            }
        }
    }

    /*
     *     Get any spans that we missed by buffering
     */
    fillSpans( nPts, (QPoint*)FirstPoint, FirstWidth );
    free(pETEs);
    miFreeStorage(SLLBlock.next);
}
/***** END OF X11-based CODE *****/


void QImagePaintDevice32::drawPixmap(QPoint p, QPixmap pm)
{
    QImage t = pm.convertToImage();
    drawImage( p, t );
}

void QImagePaintDevice32::drawImage(QPoint p, const QImage& im)
{
    drawImage( p, im, 0, 0, im.width(), im.height(), 0 );
}

// XXX copied from qimage.cpp
static
bool
haveSamePalette(const QImage& a, const QImage& b)
{
    if (a.depth() != b.depth()) return FALSE;
    if (a.numColors() != b.numColors()) return FALSE;
    QRgb* ca = a.colorTable();
    QRgb* cb = b.colorTable();
    for (int i=a.numColors(); i--; ) {
        if (*ca++ != *cb++) return FALSE;
    }
    return TRUE;
}

void QImagePaintDevice32::drawImage(QPoint p,
                                 const QImage& src, int sx, int sy,
                                 int sw, int sh, int conversion_flags )
{
    // XXX Note: this is the QImage bitBlt() code + clipping.
    // XXX with dst==this, src=&src, dumb case fatalled. Should share.

    int dx = p.x();
    int dy = p.y();

    // Parameter correction
    if ( sw < 0 ) sw = src.width();
    if ( sh < 0 ) sh = src.height();
    if ( sx < 0 ) { dx -= sx; sw += sx; sx = 0; }
    if ( sy < 0 ) { dy -= sy; sh += sy; sy = 0; }
    if ( dx < 0 ) { sx -= dx; sw += dx; dx = 0; }
    if ( dy < 0 ) { sy -= dy; sh += dy; dy = 0; }
    if ( sx + sw > src.width() ) sw = src.width() - sx;
    if ( sy + sh > src.height() ) sh = src.height() - sy;
    if ( dx + sw > img.width() ) sw = img.width() - dx;
    if ( dy + sh > img.height() ) sh = img.height() - dy;
    if ( sw <= 0 || sh <= 0 ) return; // Nothing left to copy
    if ( (img.bits() == src.bits()) && dx==sx && dy==sy ) return; // Same pixels

    // "Easy" to copy if both same depth and one of:
    //   - 32 bit
    //   - 8 bit, identical palette
    //   - 1 bit, identical palette and byte-aligned area
    //
    if ( haveSamePalette(img,src)
	&& ( img.depth() != 1 ||
	      !( (dx&7) || (sx&7) ||
		    ((sw&7) && (sx+sw < src.width()) ||
			       (dx+sw < img.width()) ) ) ) )
    {
	// easy to copy
    } else if ( img.depth() != 32 ) {
	qFatal("%d->%d drawImage(): Not implemented yet",
	    src.depth(),img.depth());
	/* this is no good
	QImage dstconv = img.convertDepth( 32 );
	bitBlt( &dstconv, dx, dy, src, sx, sy, sw, sh,
	   (conversion_flags&~Qt::DitherMode_Mask) | Qt::AvoidDither );
	*dst = dstconv.convertDepthWithPalette( img.depth(),
	    img.colorTable(), img.numColors() );
	return;
	*/
    }

    // Now assume palette can be ignored

    if ( img.depth() != src.depth() ) {
	if ( sw == src.width() && sh == src.height() || img.depth()==32 ) {
	    QImage srcconv = src.convertDepth( img.depth(), conversion_flags );
	    drawImage( QPoint(dx,dy), srcconv, sx, sy, sw, sh, conversion_flags );
	} else {
	    QImage srcconv = src.copy( sx, sy, sw, sh ); // ie. bitBlt
	    drawImage( QPoint(dx,dy), srcconv, 0, 0, sw, sh, conversion_flags );
	}
	return;
    }

    // Now assume both are the same depth.

    // "Easy"

    switch ( img.depth() ) {
      case 1:
	{
	    qWarning("No clipping yet in 1 bit - this is supposed to be 32bpp!");
	    uchar* d = img.scanLine(dy) + dx/8;
	    uchar* s = src.scanLine(sy) + sx/8;
	    const int bw = (sw+7)/8;
	    if ( bw < 64 ) {
		// Trust ourselves
		const int dd = img.bytesPerLine() - bw;
		const int ds = src.bytesPerLine() - bw;
		while ( sh-- ) {
		    for ( int t=bw; t--; )
			*d++ = *s++;
		    d += dd;
		    s += ds;
		}
	    } else {
		// Trust libc
		const int dd = img.bytesPerLine();
		const int ds = src.bytesPerLine();
		while ( sh-- ) {
		    memcpy( d, s, bw );
		    d += dd;
		    s += ds;
		}
	    }
	}
	break;
      case 8:
	{
	    qWarning("No clipping yet in 8 bit - this is supposed to be 32bpp!");
	    uchar* d = img.scanLine(dy) + dx;
	    uchar* s = src.scanLine(sy) + sx;
	    if ( sw < 64 ) {
		// Trust ourselves
		const int dd = img.bytesPerLine() - sw;
		const int ds = src.bytesPerLine() - sw;
		while ( sh-- ) {
		    for ( int t=sw; t--; )
			*d++ = *s++;
		    d += dd;
		    s += ds;
		}
	    } else {
		// Trust libc
		const int dd = img.bytesPerLine();
		const int ds = src.bytesPerLine();
		while ( sh-- ) {
		    memcpy( d, s, sw );
		    d += dd;
		    s += ds;
		}
	    }
	}
	break;
      case 32:
	{
	    bool noalpha = !src.hasAlphaBuffer() ||
		(conversion_flags & Qt::NoAlpha);

	    QRgb* d = (QRgb*)img.scanLine(dy) + dx;
	    QRgb* s = (QRgb*)src.scanLine(sy) + sx;
	    const int dd = img.width() - sw;
	    const int ds = src.width() - sw;

	    int x = dx, y = dy, w = sw, h = sh;
	    QRect cr;
	    bool plot = find(QPoint(x,y));
	    cr = cliprect[clipcursor];
	    if ( !plot )
		findOutside(x,y,cr);
	    for (int j=0; j<h; j++) {
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
			QRgb in = *s;
			int alpha;
			if ( noalpha || (alpha = (in >> 24)) == 0xff ) {
			    *d = *s;
			} else if ( alpha == 0 ) {
			    // skip *s
			} else {
			    *d = qRgb(
				(qRed(*d)*(255-alpha) + qRed(*s)*alpha)/255,
				(qGreen(*d)*(255-alpha) + qGreen(*s)*alpha)/255,
				(qBlue(*d)*(255-alpha) + qBlue(*s)*alpha)/255
			    );
			}
		    }
		    s++;
		    d++;
		    x++;
		}
		s += ds;
		d += dd;
		y++;
		x-=w;
	    }
	}
    }
}

void QImagePaintDevice32::saveState()
{
    // XXX
}

void QImagePaintDevice32::restoreState()
{
    // XXX
}

void QImagePaintDevice32::setBkColor(const QColor&)
{
    // XXX
}

void QImagePaintDevice32::setBkMode(int)
{
    // XXX
}

void QImagePaintDevice32::setROP(int)
{
    // XXX
}

void QImagePaintDevice32::setPen(const QPen& p)
{
    pen = p;
    fg = p.color().rgb();
    // XXX line style, width
}

void QImagePaintDevice32::setBrush(const QBrush& b)
{
    brush = b;
    br = b.color().rgb();
}

void QImagePaintDevice32::setTabStops(int)
{
    // XXX
}

void QImagePaintDevice32::setTabArray(int, int*)
{
    // XXX
}

void QImagePaintDevice32::setUnit(int)
{
    // XXX
}

void QImagePaintDevice32::setVXform(int)
{
    // XXX perhaps only support translation at first,
    // XXX or move some of the QPainter code that does the work on X11.
}

void QImagePaintDevice32::setWXform(int)
{
    // see setVXform()
}

void QImagePaintDevice32::setClip(int i)
{
    // XXX see setClipRegion()
    if ( i == clipon )
	return;
    if ( i ) {
	cliprect = ocliprect;
	ncliprect = oncliprect;
    } else {
	cliprect = cliprect1;
	ncliprect = ncliprect1;
    }
    clipon = i;
    clipcursor = 0;
}

void QImagePaintDevice32::setWindow(const QRect&)
{
    // see setVXform()
}

void QImagePaintDevice32::setViewport(const QRect&)
{
    // see setVXform()
}

void QImagePaintDevice32::setMatrix(const QWMatrix&, int)
{
    // see setVXform()
}

void QImagePaintDevice32::setClipRegion(const QRegion& rgn)
{
    QMemArray<QRect> a = rgn.rects();
    delete [] ocliprect;
    cliprect = ocliprect = new QRect[a.size()];
    for (int i=0; i<(int)a.size(); i++)
	ocliprect[i]=a[i];
    ncliprect = oncliprect = a.size();
    clipcursor = 0;
    clipon = 1;
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
#if defined(QT_CHECK_RANGE)
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
    blit( p.x()+tm->offset().x(), p.y()+tm->offset().y()-tm->height(),
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

void QImagePaintDevice32::setFont( const QFont& f )
{
    font = f;
    delete renderer; // XXX no caching yet
    renderer = qt_font_renderer_ttf(f);

    // XXX try other renderers...

    if ( !renderer ) {
	qWarning("No fonts available (trying to load %s)\n"
	    "Tried TTF fonts only.", f.family().latin1());
	renderer = qt_font_renderer_ttf(QFont("TIMES",f.pointSize()));
	if ( !renderer )
	    qFatal("No TIMES last-resort font!");
    }
}

QFontRenderer::~QFontRenderer()
{
}

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdrawutl.cpp#2 $
**
** Implementation of draw utilities
**
** Author  : Haavard Nord
** Created : 950920
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdrawutl.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qdrawutl.cpp#2 $")


/*!
  Draw a horizontal (\e y1 == \e y2) or vertical (\e x1 == \e x2) shaded line.

  The \e tColor argument specifies the top (or left) color and \e bColor
  specifies the bottom (or right) color.

  The \e lw argument specifies the line width for each of the lines. It is
  not the total line width.

  The \e mw argument specifies the width of a middle line drawn in \e mColor.

  If \e tColor is brighter than \e bColor, the line appears to be
  raised from the surface.  If \e tColor is darker than \e bColor, the line
  appears to be sunken into the surface.

  \relates QPainter

  \sa drawShadeRect(), drawShadePanel(), QFrame
*/

void drawShadeLine( QPainter *p, int x1, int y1, int x2, int y2,
		    const QColorGroup &g, bool sunken,
		    int lineWidth, int midLineWidth )
{
#if defined(CHECK_RANGE)
    ASSERT( p && lineWidth >= 0 && midLineWidth >= 0 );
#endif
    int tlw = lineWidth*2 + midLineWidth;	// total line width
    QPen oldPen = p->pen();			// save pen
    if ( sunken )
	p->setPen( g.dark() );
    else
	p->setPen( g.light() );
    QPointArray a;
    int i;
    if ( y1 == y2 ) {				// horizontal line
	int y = y1 - tlw/2;
	if ( x1 > x2 ) {			// swap x1 and x2
	    int t = x1;
	    x1 = x2;
	    x2 = t;
	}
	x2--;
	for ( i=0; i<lineWidth; i++ ) {		// draw top shadow
	    a.setPoints( 3, x1+i, y+tlw-1,
			    x1+i, y+i,
			    x2,	  y+i );
	    p->drawPolyline( a );
	}
	if ( midLineWidth > 0 ) {
	    p->setPen( g.mid() );
	    for ( i=0; i<midLineWidth; i++ )	// draw lines in the middle
		p->drawLine( x1+lineWidth, y+lineWidth+i,
			     x2-lineWidth, y+lineWidth+i );
	}
	if ( sunken )
	    p->setPen( g.light() );
	else
	    p->setPen( g.dark() );
	for ( i=0; i<lineWidth; i++ ) {		// draw bottom shadow
	    a.setPoints( 3, x1+lineWidth, y+tlw-i-1,
			    x2-i,	  y+tlw-i-1,
			    x2-i, 	  y+lineWidth );
	    p->drawPolyline( a );
	}
    }
    else if ( x1 == x2 ) {			// vertical line
	int x = x1 - tlw/2;
	if ( y1 > y2 ) {			// swap y1 and y2
	    int t = y1;
	    y1 = y2;
	    y2 = t;
	}
	y2--;
	for ( i=0; i<lineWidth; i++ ) {		// draw left shadow
	    a.setPoints( 3, x+i,     y2,
			    x+i,     y1+i,
			    x+tlw-1, y1+i );
	    p->drawPolyline( a );
	}
	if ( midLineWidth > 0 ) {
	    p->setPen( g.mid() );
	    for ( i=0; i<midLineWidth; i++ )	// draw lines in the middle
		p->drawLine( x+lineWidth+i, y1+lineWidth, x+lineWidth+i, y2 );
	}
	if ( sunken )
	    p->setPen( g.light() );
	else
	    p->setPen( g.dark() );
	for ( i=0; i<lineWidth; i++ ) {		// draw right shadow
	    a.setPoints( 3, x+lineWidth,      y2-i,
			    x+tlw-i-1, y2-i,
			    x+tlw-i-1, y1+lineWidth );
	    p->drawPolyline( a );
	}
    }
    p->setPen( oldPen );
}


/*!
  Draw a shaded rectangle given by \e (x,y,w,h).

  The arguments have the same meaning as for drawShadeLine().
  \sa drawShadeLine(), drawShadePanel(), QFrame

  \relates QPainter
*/

void drawShadeRect( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool sunken,
		    int lineWidth, int midLineWidth,
		    const QBrush *fill )
{
    if ( w == 0 || h == 0 )
	return;
#if defined(CHECK_RANGE)
    ASSERT( w > 0 && h > 0 && lineWidth >= 0 && midLineWidth >= 0 );
#endif
    QPen oldPen = p->pen();			// save pen
    if ( sunken )
	p->setPen( g.dark() );
    else
	p->setPen( g.light() );
    int x1=x, y1=y, x2=x+w-1, y2=y+h-1;
    QPointArray a;
    if ( lineWidth == 1 && midLineWidth == 0 ) {// standard shade rectangle
	a.setPoints( 8, x1,y1, x2,y1, x1,y1+1, x1,y2, x1+2,y2-1,
		     x2-1,y2-1, x2-1,y1+2,  x2-1,y2-2 );
	p->drawLineSegments( a );		// draw top/left lines
	if ( sunken )
	    p->setPen( g.light() );
	else
	    p->setPen( g.dark() );	
	a.setPoints( 8, x1+1,y1+1, x2,y1+1, x1+1,y1+2, x1+1,y2-1,
		     x1+1,y2, x2,y2,  x2,y1+2, x2,y2-1 );
	p->drawLineSegments( a );		// draw bottom/right lines
    }
    else {					// more complicated
	int t = lineWidth*2+midLineWidth;
	int m = lineWidth+midLineWidth;
	int i, j=0, k=m;
	for ( i=0; i<lineWidth; i++ ) {		// draw top shadow
	    p->drawLine( x1+j, y2-j, x1+j, y1+j );
	    p->drawLine( x1+j, y1+j, x2-j, y1+j );
	    p->drawLine( x1+t, y2-k, x2-k, y2-k );
	    p->drawLine( x2-k, y2-k, x2-k, y1+k );
	    j++;
	    k++;
	}
	p->setPen( g.mid() );
	j = lineWidth*2;
	for ( i=0; i<midLineWidth; i++ ) {	// draw lines in the middle
	    p->drawRect( x1+lineWidth+i, y1+lineWidth+i, w-j, h-j );
	    j += 2;
	}
	if ( sunken )
	    p->setPen( g.light() );
	else
	    p->setPen( g.dark() );	
	j = 0;
	k = m;
	for ( i=0; i<lineWidth; i++ ) {		// draw bottom shadow
	    p->drawLine( x1+1+j,y2-j, x2-j, y2-j );
	    p->drawLine( x2-j,  y2-j, x2-j, y1+j+1 );
	    p->drawLine( x1+k,  y2-m, x1+k, y1+k );
	    p->drawLine( x1+k,  y1+k, x2-k, y1+k );
	    j++;
	    k++;
	}
    }
    if ( fill ) {
	QBrush oldBrush = p->brush();
	int tlw = lineWidth + midLineWidth;
	p->setPen( NoPen );
	p->setBrush( *fill );
	p->drawRect( x+tlw, y+tlw, w-2*tlw, h-2*tlw );
	p->setBrush( oldBrush );
    }
    p->setPen( oldPen );			// restore pen
}


/*!
  Draw a shaded panel given by \e (x,y,w,h).
  \sa drawShadeLine(), drawShadeRect(), QFrame

  \relates QPainter
*/

void drawShadePanel( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken,
		     int lineWidth, const QBrush *fill )
{
    if ( w == 0 || h == 0 )
	return;
#if defined(CHECK_RANGE)
    ASSERT( w > 0 && h > 0 && lineWidth >= 0 );
#endif
    QPen oldPen = p->pen();			// save pen
    QPointArray a( 4*lineWidth );
    if ( sunken )
	p->setPen( g.dark() );
    else
	p->setPen( g.light() );
    int x1, y1, x2, y2;
    int i;
    int n = 0;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for ( i=0; i<lineWidth; i++ ) {		// top shadow
	a.setPoint( n++, x1, y1++ );
	a.setPoint( n++, x2--, y2++ );
    }
    x2 = x1;
    y1 = y+h-2;
    for ( i=0; i<lineWidth; i++ ) {		// left shadow
	a.setPoint( n++, x1++, y1 );
	a.setPoint( n++, x2++, y2-- );
    }
    p->drawLineSegments( a );
    n = 0;
    if ( sunken )
	p->setPen( g.light() );
    else
	p->setPen( g.dark() );
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for ( i=0; i<lineWidth; i++ ) {		// bottom shadow
	a.setPoint( n++, x1++, y1-- );
	a.setPoint( n++, x2, y2-- );
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lineWidth-1;
    for ( i=0; i<lineWidth; i++ ) {		// right shadow
	a.setPoint( n++, x1--, y1++ );
	a.setPoint( n++, x2--, y2 );
    }
    p->drawLineSegments( a );
    if ( fill ) {				// fill with fill color
	QBrush oldBrush = p->brush();
	p->setPen( NoPen );
	p->setBrush( *fill );
	p->drawRect( x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2 );
	p->setBrush( oldBrush );
    }
    p->setPen( oldPen );			// restore pen
}


/*!
  \internal
  This function draws a rectangle with two pixel line width.
  It is called from drawWinButton() and drawWinPanel().
*/

static void drawWinShades( QPainter *p,
			   int x, int y, int w, int h,
			   const QColor &c1, const QColor &c2,
			   const QColor &c3, const QColor &c4,
			   const QBrush *fill )
{
    if ( w < 2 || h < 2 )			// nothing to draw
	return;
    QPen oldPen = p->pen();
    QPointArray a( 3 );
    a.setPoint( 0, x, y+h-2 );
    a.setPoint( 1, x, y );
    a.setPoint( 2, x+w-2, y );
    p->setPen( c1 );
    p->drawPolyline( a );
    a.setPoint( 0, x, y+h-1 );
    a.setPoint( 1, x+w-1, y+h-1 );
    a.setPoint( 2, x+w-1, y );
    p->setPen( c2 );
    p->drawPolyline( a );
    if ( w > 4 && h > 4 ) {
	a.setPoint( 0, x+1, y+h-3 );
	a.setPoint( 1, x+1, y+1 );
	a.setPoint( 2, x+w-3, y+1 );
	p->setPen( c3 );
	p->drawPolyline( a );
	a.setPoint( 0, x+1, y+h-2 );
	a.setPoint( 1, x+w-2, y+h-2 );
	a.setPoint( 2, x+w-2, y+1 );
	p->setPen( c4 );
	p->drawPolyline( a );
	if ( fill ) {
	    QBrush oldBrush = p->brush();
	    p->setBrush( *fill );
	    p->setPen( NoPen );
	    p->drawRect( x+2, y+2, w-4, h-4 );
	    p->setBrush( oldBrush );
	}
    }
    p->setPen( oldPen );
}


void drawWinButton( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool  sunken,
		    const QBrush *fill )
{
    if ( sunken )
	drawWinShades( p, x, y, w, h,
		       black, g.light(), g.dark(), g.background(), fill );
    else
	drawWinShades( p, x, y, w, h,
		       g.light(), black, g.background(), g.dark(), fill );
}


void drawWinPanel( QPainter *p, int x, int y, int w, int h,
		   const QColorGroup &g, bool  sunken,
		   const QBrush *fill )
{
    if ( sunken )
	drawWinShades( p, x, y, w, h,
		       g.dark(), g.light(), black, g.background(), fill );
    else
	drawWinShades( p, x, y, w, h,
		       g.background(), black, g.light(), g.dark(), fill );
}


void drawPlainRect( QPainter *p, int x, int y, int w, int h, const QColor &c,
		    int lineWidth = 1, const QBrush *fill = 0 )
{
    if ( w == 0 || h == 0 )
	return;
#if defined(CHECK_RANGE)
    ASSERT( w > 0 && h > 0 && lineWidth >= 0 );
#endif
    QPen   oldPen   = p->pen();
    QBrush oldBrush = p->brush();
    p->setPen( c );
    p->setBrush( oldBrush );
    for ( int i=0; i<lineWidth; i++ )
	p->drawRect( x+i, y+i, w-i*2, h-i*2 );
    if ( fill ) {				// fill with fill color
	p->setPen( NoPen );
	p->setBrush( *fill );
	p->drawRect( x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2 );
    }
    p->setPen( oldPen );
    p->setBrush( oldBrush );
}


// ----------------------------------------------------------------------------
// Overloaded functions
//

/*!
  \overload void drawShadeLine( QPainter *p, const QPoint &p1, const QPoint &p2, const QColorGroup &g, bool sunken, int lineWidth, int midLineWidth )
*/

void drawShadeLine( QPainter *p, const QPoint &p1, const QPoint &p2,
		    const QColorGroup &g, bool sunken,
		    int lineWidth, int midLineWidth )
{
    drawShadeLine( p, p1.x(), p1.y(), p2.x(), p2.y(), g, sunken,
		   lineWidth, midLineWidth );
}


void drawShadeRect( QPainter *p, const QRect &r,
		    const QColorGroup &g, bool sunken,
		    int lineWidth, int midLineWidth,
		    const QBrush *fill )
{
    drawShadeRect( p, r.x(), r.y(), r.width(), r.height(), g, sunken,
		   lineWidth, midLineWidth, fill );
}


void drawShadePanel( QPainter *p, const QRect &r,
		     const QColorGroup &g, bool sunken,
		     int lineWidth, const QBrush *fill )
{
    drawShadePanel( p, r.x(), r.y(), r.width(), r.height(), g, sunken,
		    lineWidth, fill );
}


void drawWinButton( QPainter *p, const QRect &r,
		    const QColorGroup &g, bool sunken,
		    const QBrush *fill )
{
    drawWinButton( p, r.x(), r.y(), r.width(), r.height(), g, sunken, fill );
}


void drawWinPanel( QPainter *p, const QRect &r,
		   const QColorGroup &g, bool sunken,
		   const QBrush *fill )
{
    drawWinPanel( p, r.x(), r.y(), r.width(), r.height(), g, sunken, fill );
}


void drawPlainRect( QPainter *p, const QRect &r, const QColor &c,
		    int lineWidth = 1, const QBrush *fill = 0 )
{
    drawPlainRect( p, r.x(), r.y(), r.width(), r.height(), c,
		   lineWidth, fill );
}

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter.cpp#1 $
**
** Implementation of QPainter class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file containts the platform independent implementation of the
** QPainter class.  Platform dependent functions are implemented in the
** qptr_xxx.C files.
*****************************************************************************/

#define QPAINTER_C
#include "qpainter.h"
#include "qpntarry.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpainter.cpp#1 $";
#endif


QPaintDevice *QPainter::pdev_ov = 0;

bool QPainter::redirect( const QPaintDevice *pd )
{
    if ( pdev_ov && pd )			// already set
	return FALSE;
    pdev_ov = (QPaintDevice *)pd;
    return TRUE;
}


void QPainter::drawShadeLine( int x1, int y1, int x2, int y2,
			      const QColor &tColor, const QColor &bColor )
{
    QPen oldPen = pen();			// save pen
    QPen newPen( tColor );
    setPen( newPen );
    int dx, dy;
    if ( y1 == y2 ) {				// horizontal line
	dx = 0;
	dy = 1;
    }
    else {					// vertical line
	dx = 1;
	dy = 0;
    }
    drawLine( x1, y1, x2, y2 );			// draw top line
    cpen.setColor( bColor );
    drawLine( x1+dx, y1+dy, x2+dx, y2+dy );	// draw bottom line
    setPen( oldPen );				// restore pen
}


void QPainter::drawShadeRect( int x, int y, int w, int h,
			      const QColor &tColor, const QColor &bColor,
			      bool fill )
{
    if ( w < 1 || h < 1 )			// no such rectangle
	return;
    QPen oldPen = pen();			// save pen
    QPen newPen( tColor );
    setPen( newPen );
    int x1=x, y1=y, x2=x+w-1, y2=y+h-1;
    QPointArray a( 8 );
    a.setPoint( 0, x1, y1 );			// top lines
    a.setPoint( 1, x2, y1 );
    a.setPoint( 2, x1, y1+1 );
    a.setPoint( 3, x1, y2 );
    a.setPoint( 4, x1+2, y2-1 );
    a.setPoint( 5, x2-1, y2-1 );
    a.setPoint( 6, x2-1, y1+2 );
    a.setPoint( 7, x2-1, y2-2 );
    drawLineSegments( a );
    cpen.setColor( bColor );
    a.setPoint( 0, x1+1, y1+1 );		// bottom lines
    a.setPoint( 1, x2, y1+1 );
    a.setPoint( 2, x1+1, y1+2 );
    a.setPoint( 3, x1+1, y2-1 );
    a.setPoint( 4, x1+1, y2 );
    a.setPoint( 5, x2, y2 );
    a.setPoint( 6, x2, y1+2 );
    a.setPoint( 7, x2, y2-1 );
    drawLineSegments( a );
    if ( fill && w > 4 && h > 4 ) {		// fill with background color
	QBrush oldBrush = brush();
	setBrush( QBrush(backgroundColor()) );
	cpen.setStyle( NoPen );
	drawRect( x+2, y+2, w-4, h-4 );
	setBrush( oldBrush );
    }
    setPen( oldPen );				// restore pen
}


void QPainter::drawShadePanel( int x, int y, int w, int h,
			       const QColor &tColor, const QColor &bColor,
			       int tWidth, int bWidth,
			       bool fill )
{
    if ( w < 1 || h < 1 )			// no such rectangle
	return;
    int sumWidth = tWidth + bWidth;
    if ( sumWidth >= w || sumWidth >= h || tWidth < 0 || bWidth < 0 )
	tWidth = bWidth = 0;			// fix bad width args
    QPen oldPen = pen();			// save pen
    QPen newPen( tColor );
    QPointArray a( 4*tWidth );
    setPen( newPen );
    int x1, y1, x2, y2;
    int i;
    int n = 0;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for ( i=0; i<tWidth; i++ ) {		// top shadow
	a.setPoint( n++, x1, y1++ );
	a.setPoint( n++, x2--, y2++ );
    }
    x2 = x1;
    y1 = y+h-2;
    for ( i=0; i<tWidth; i++ ) {		// left shadow
	a.setPoint( n++, x1++, y1 );
	a.setPoint( n++, x2++, y2-- );
    }
    drawLineSegments( a );
    a.resize( 4*bWidth );
    n = 0;
    cpen.setColor( bColor );
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for ( i=0; i<bWidth; i++ ) {		// bottom shadow
	a.setPoint( n++, x1++, y1-- );
	a.setPoint( n++, x2, y2-- );
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-bWidth-1;
    for ( i=0; i<bWidth; i++ ) {		// right shadow
	a.setPoint( n++, x1--, y1++ );
	a.setPoint( n++, x2--, y2 );
    }
    drawLineSegments( a );
    if ( fill ) {				// fill with background color
	QBrush oldBrush = brush();
	setBrush( QBrush(backgroundColor()) );
	cpen.setStyle( NoPen );
	drawRect( x+tWidth, y+tWidth, w-sumWidth, h-sumWidth );
	setBrush( oldBrush );
    }
    setPen( oldPen );				// restore pen
}


void QPainter::fillRect( int x, int y, int w, int h, const QColor &color )
{
    QPen oldPen = pen();			// save pen
    QPen newPen( black, 0, NoPen );
    setPen( newPen );
    QBrush oldBrush = brush();			// save brush
    setBrush( QBrush(color) );
    drawRect( x, y, w, h );			// draw filled rect
    setBrush( oldBrush );			// restore brush
    setPen( oldPen );				// restore pen
}


// --------------------------------------------------------------------------
// QPainter member functions (inline if DEBUG not defined)
//

QColor QPainter::backgroundColor() const
{
    return bg_col;
}

BGMode QPainter::backgroundMode() const
{
    return bg_mode;
}

void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.getX(), p.getY() );
}

void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.getX(), p.getY() );
}

void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.getX(), p.getY() );
}

void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.getX(), p1.getY(), p2.getX(), p2.getY() );
}

void QPainter::drawRect( const QRect &r )
{
    drawRect( r.left(), r.top(), r.width(), r.height() );
}

void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.left(), r.top(), r.width(), r.height(), xRnd, yRnd );
}

void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.left(), r.top(), r.width(), r.height() );
}

void QPainter::drawArc( const QRect &r, int a1, int a2 )
{
    drawArc( r.left(), r.top(), r.width(), r.height(), a1, a2 );
}

void QPainter::drawPie( const QRect &r, int a1, int a2 )
{
    drawPie( r.left(), r.top(), r.width(), r.height(), a1, a2 );
}

void QPainter::drawChord( const QRect &r, int a1, int a2 )
{
    drawChord( r.left(), r.top(), r.width(), r.height(), a1, a2 );
}

void QPainter::fillRect( const QRect &r, const QColor &c )
{
    fillRect( r.left(), r.top(), r.width(), r.height(), c );
}

void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.left(), r.top(), r.width(), r.height(), backgroundColor() );
}

void QPainter::drawShadeLine( const QPoint &p1, const QPoint &p2,
			      const QColor &tC, const QColor &bC )
{
    drawShadeLine( p1.getX(), p1.getY(), p2.getX(), p2.getY(), tC, bC );
}

void QPainter::drawShadeRect( const QRect &r,
			      const QColor &tC, const QColor &bC,
			      bool fill )
{
    drawShadeRect( r.left(), r.top(), r.width(), r.height(), tC, bC, fill );
}

void QPainter::drawShadePanel( const QRect &r,
			       const QColor &tC, const QColor &bC,
			       int tWidth, int bWidth,
			       bool fill )
{
    drawShadePanel( r.left(), r.top(), r.width(), r.height(), tC, bC,
		    tWidth, bWidth, fill );
}

void QPainter::drawText( const QPoint &p, const char *s, int len )
{
    drawText( p.getX(), p.getY(), s, len );
}

void QPainter::drawText( const QRect &r, TextAlignment ta, const char *s,
			 int len )
{
    drawText( r.left(), r.top(), r.width(), r.height(), ta, s, len );
}

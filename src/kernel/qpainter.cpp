/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter.cpp#5 $
**
** Implementation of QPainter class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file containts the platform independent implementation of the
** QPainter class.  Platform dependent functions are implemented in the
** qptr_xxx.cpp files.
*****************************************************************************/

#define QPAINTER_C
#include "qpainter.h"
#include "qpaintdc.h"
#include "qpntarry.h"
#include "qwxfmat.h"
#include "qstack.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpainter.cpp#5 $";
#endif


QPaintDevice *QPainter::pdev_ov = 0;

bool QPainter::redirect( const QPaintDevice *pd )
{
    if ( pdev_ov && pd )			// already set
	return FALSE;
    pdev_ov = (QPaintDevice *)pd;
    return TRUE;
}


void QPainter::setf( ushort b, bool v )
{
    if ( v )
	setf( b );
    else
	clearf( b );
}


struct QPState {				// painter state
    QFont	font;
    QPen	pen;
    QBrush	brush;
    QColor	bgc;
    uchar	bgm;
    uchar	pu;
    uchar	rop;
    QPoint	bro;
    QRect	sr, tr;
    QWXFMatrix	wm;
    bool	vxf;
    bool	wxf;
    QRegion	rgn;
    bool	clip;
};

declare(QStackM,QPState);
typedef QStackM(QPState) QPStateStack;

void QPainter::killPStack()
{
    delete (QPStateStack *)ps_stack;
}

void QPainter::save()				// save/push painter state
{
    if ( testf(ExtDev) ) {
	pdev->cmd( PDC_SAVE, 0 );
	return;
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 ) {
	pss = new QStackM(QPState);
	pss->setAutoDelete( TRUE );
	CHECK_PTR( pss );
	ps_stack = pss;
    }
    register QPState *ps = new QPState;
    CHECK_PTR( ps );
    ps->font  = cfont.copy();
    ps->pen   = cpen.copy();
    ps->brush = cbrush.copy();
    ps->bgc = bg_col;
    ps->bgm = bg_mode;
    ps->rop = rop;
    ps->bro = bro;
    ps->pu  = pu;
    ps->sr  = QRect( sx, sy, sw, sh );
    ps->tr  = QRect( tx, ty, tw, th );
    ps->wm  = *wxfmat;
    ps->vxf = testf(VxF);
    ps->wxf = testf(WxF);
    ps->rgn = crgn.copy();
    ps->clip= testf(ClipOn);
    pss->push( ps );
}

void QPainter::restore()			// restore/pop painter state
{
    if ( testf(ExtDev) ) {
	pdev->cmd( PDC_RESTORE, 0 );
	return;
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 || pss->isEmpty() )
	return;
    register QPState *ps = pss->top();
    if ( ps->font != cfont )
	setFont( ps->font );
    if ( ps->pen != cpen )
	setPen( ps->pen );
    if ( ps->brush != cbrush )
	setBrush( ps->brush );
    if ( ps->bgc != bg_col )
	setBackgroundColor( ps->bgc );
    if ( ps->bgm != bg_mode )
	setBackgroundMode( (BGMode)ps->bgm );
    if ( ps->rop != rop )
	setRasterOp( (RasterOp)ps->rop );
    if ( ps->pu != pu )
	pu = ps->pu;
    QRect sr( sx, sy, sw, sh );
    QRect tr( tx, ty, tw, th );
    if ( ps->sr != sr )
	setSourceView( ps->sr );
    if ( ps->tr != tr )
	setTargetView( ps->tr );
    if ( ps->wm != *wxfmat )
	setWxfMatrix( ps->wm );
    if ( ps->vxf != testf(VxF) )
	setViewXForm( ps->vxf );
    if ( ps->wxf != testf(WxF) )
	setWorldXForm( ps->wxf );
    if ( ps->rgn != crgn )
	setClipRegion( ps->rgn );
    if ( ps->clip != testf(ClipOn) )
	setClipping( ps->clip );
    pss->pop();
}


// --------------------------------------------------------------------------
// Painter functions for drawing shadow effects.
//

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
    QPointArray a;
    a.setPoints( 8, x1,y1, x2,y1, x1,y1+1, x1,y2, x1+2,y2-1, x2-1,y2-1,
		    x2-1,y1+2,  x2-1,y2-2 );
    drawLineSegments( a );			// draw top lines
    cpen.setColor( bColor );
    a.setPoints( 8, x1+1,y1+1, x2,y1+1, x1+1,y1+2, x1+1,y2-1, x1+1,y2, x2,y2,
		 x2,y1+2, x2,y2-1 );
    drawLineSegments( a );			// draw bottom lines
    if ( fill && w > 4 && h > 4 ) {		// fill with current brush
	cpen.setStyle( NoPen );
	drawRect( x+2, y+2, w-4, h-4 );
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
	cpen.setStyle( NoPen );
	drawRect( x+tWidth, y+tWidth, w-sumWidth, h-sumWidth );
    }
    setPen( oldPen );				// restore pen
}


// --------------------------------------------------------------------------
// Convenience function for filling a rectangle.
//

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

void QPainter::setBrushOrigin( const QPoint &p )
{
    setBrushOrigin( p.x(), p.y() );
}

void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.x(), p.y() );
}

void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.x(), p.y() );
}

void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.x(), p.y() );
}

void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.x(), p1.y(), p2.x(), p2.y() );
}

void QPainter::drawRect( const QRect &r )
{
    drawRect( r.x(), r.y(), r.width(), r.height() );
}

void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.x(), r.y(), r.width(), r.height(), xRnd, yRnd );
}

void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.x(), r.y(), r.width(), r.height() );
}

void QPainter::drawArc( const QRect &r, int a1, int a2 )
{
    drawArc( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

void QPainter::drawPie( const QRect &r, int a1, int a2 )
{
    drawPie( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

void QPainter::drawChord( const QRect &r, int a1, int a2 )
{
    drawChord( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

void QPainter::fillRect( const QRect &r, const QColor &c )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), c );
}

void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), backgroundColor() );
}

void QPainter::drawShadeLine( const QPoint &p1, const QPoint &p2,
			      const QColor &tC, const QColor &bC )
{
    drawShadeLine( p1.x(), p1.y(), p2.x(), p2.y(), tC, bC );
}

void QPainter::drawShadeRect( const QRect &r,
			      const QColor &tC, const QColor &bC,
			      bool fill )
{
    drawShadeRect( r.x(), r.y(), r.width(), r.height(), tC, bC, fill );
}

void QPainter::drawShadePanel( const QRect &r,
			       const QColor &tC, const QColor &bC,
			       int tWidth, int bWidth,
			       bool fill )
{
    drawShadePanel( r.x(), r.y(), r.width(), r.height(), tC, bC,
		    tWidth, bWidth, fill );
}

void QPainter::drawText( const QPoint &p, const char *s, int len )
{
    drawText( p.x(), p.y(), s, len );
}

void QPainter::drawText( const QRect &r, TextAlignment ta, const char *str,
			 int len )
{
    drawText( r.x(), r.y(), r.width(), r.height(), ta, str, len );
}


// --------------------------------------------------------------------------
// QPen stream functions
//

QDataStream &operator<<( QDataStream &s, const QPen &p )
{
    return s << (UINT8)p.style() << (UINT8)p.width() << p.color();
}

QDataStream &operator>>( QDataStream &s, QPen &p )
{
    UINT8 style, width;
    QColor color;
    s >> style;
    s >> width;
    s >> color;
    p = QPen( color, (uint)width, (PenStyle)style );
    return s;
}


// --------------------------------------------------------------------------
// QBrush stream functions
//

QDataStream &operator<<( QDataStream &s, const QBrush &b )
{
    return s << (UINT8)b.style() << b.color();
}

QDataStream &operator>>( QDataStream &s, QBrush &b )
{
    UINT8 style;
    QColor color;
    s >> style;
    s >> color;
#if defined(DEBUG)
    if ( style == CustomPattern )
	warning( "QBrush: Cannot read bitmap brush from data stream" );
#endif
    b = QBrush( color, (BrushStyle)style );
    return s;
}

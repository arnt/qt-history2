/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter.cpp#34 $
**
** Implementation of QPainter class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file containts the platform independent implementation of the
** QPainter class.  Platform dependent functions are implemented in the
** qptr_xxx.cpp files.
*****************************************************************************/

#define QPAINTER_C
#include "qpainter.h"
#include "qpaintdc.h"
#include "qbitmap.h"
#include "qstack.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpainter.cpp#34 $";
#endif


/*!
\class QPainter qpainter.h
\brief The QPainter class paints on paint devices.

The painter provides graphics rendering on any QPaintDevice object.

Graphics can be transformed using view transformation, world transformation
or a combination of these two.
View transformation is a window/viewport transformation with translation
and scaling.  World transformation is a full 2D transformation including
rotation and shearing.

The typical use of a painter is:
<ol>
<li> Call begin() to begin painting on a device.
<li> Set a pen, a brush etc.
<li> Perform the drawing.
<li> Call end() to finish painting.
</ol>

Example of use:
\code
  QWidget  widget;
  QPainter paint;
  paint.begin( &widget );
  paint.setPen( blue );
  paint.drawText( 20, 20, 100, 100, AlignCenter, "The Text" );
  paint.end();
\endcode
*/


void QPainter::setf( ushort b, bool v )		// set painter flag (internal)
{
    if ( v )
	setf( b );
    else
	clearf( b );
}


/*!
Set the number of pixels per tab stop to a fixed number.

Tab stops are used when drawing formatted text with \c ExpandTabs set.
\sa setTabArray().
*/

void QPainter::setTabStops( int ts )		// set tab stops
{
    tabstops = ts;
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[1];
	param[0].ival = ts;
	pdev->cmd( PDC_SETTABSTOPS, param );
    }
}

/*!
Set an array containing the tab stops.

The last tab stop must be 0 (terminates the array).
\sa setTabStops().
*/

void QPainter::setTabArray( int *ta )		// set tab array
{
    if ( ta != tabarray ) {
	tabarraylen = 0;
	delete tabarray;			// delete old array
	if ( ta ) {				// tabarray = copy of 'ta'
	    while ( ta[tabarraylen] )
		tabarraylen++;
	    tabarray = new int[tabarraylen];	// duplicate ta
	    memcpy( tabarray, ta, sizeof(int)*tabarraylen );
	}
	else
	    tabarray = 0;
    }
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[2];
	param[0].ival = tabarraylen;
	param[1].ivec = tabarray;
	pdev->cmd( PDC_SETTABARRAY, param );
    }
}


struct QPState {				// painter state
    QFont	 font;
    QPen	 pen;
    QBrush	 brush;
    QColor	 bgc;
    uchar	 bgm;
    uchar	 pu;
    uchar	 rop;
    QPoint	 bro;
    QRect	 wr, vr;
    Q2DMatrix	 wm;
    bool	 vxf;
    bool	 wxf;
    QRegion	 rgn;
    bool	 clip;
    int		 ts;
    int		*ta;
};

declare(QStackM,QPState);
typedef QStackM(QPState) QPStateStack;


void QPainter::killPStack()
{
    delete (QPStateStack *)ps_stack;
}

/*!
Saves the current painter state (pushes the state onto a stack).
\sa restore().
*/

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
    ps->font  = cfont;
    ps->pen   = cpen;
    ps->brush = cbrush;
    ps->bgc   = bg_col;
    ps->bgm   = bg_mode;
    ps->rop   = rop;
    ps->bro   = bro;
    ps->pu    = pu;
    ps->wr    = QRect( wx, wy, ww, wh );
    ps->vr    = QRect( vx, vy, vw, vh );
    ps->wm    = wxmat;
    ps->vxf   = testf(VxF);
    ps->wxf   = testf(WxF);
    ps->rgn   = crgn.copy();
    ps->clip  = testf(ClipOn);
    ps->ts    = tabstops;
    ps->ta    = tabarray;
    pss->push( ps );
}

/*!
Restores the current painter state (pops a saved state off the stack).
\sa save().
*/

void QPainter::restore()			// restore/pop painter state
{
    if ( testf(ExtDev) ) {
	pdev->cmd( PDC_RESTORE, 0 );
	return;
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 || pss->isEmpty() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::restore: Empty stack error" );
#endif
	return;
    }
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
    QRect wr( wx, wy, ww, wh );
    QRect vr( vx, vy, vw, vh );
    if ( ps->wr != wr )
	setWindow( ps->wr );
    if ( ps->vr != vr )
	setViewport( ps->vr );
    if ( ps->wm != wxmat )
	setWorldMatrix( ps->wm );
    if ( ps->vxf != testf(VxF) )
	setViewXForm( ps->vxf );
    if ( ps->wxf != testf(WxF) )
	setWorldXForm( ps->wxf );
    if ( ps->rgn != crgn )
	setClipRegion( ps->rgn );
    if ( ps->clip != testf(ClipOn) )
	setClipping( ps->clip );
    tabstops = ps->ts;
    tabarray = ps->ta;
    pss->pop();
}


// --------------------------------------------------------------------------
// Painter functions for drawing shadow effects.
//

/*!
Draw a horizontal (\e y1 == \e y2) or vertical (\e x1 == \e x2) shaded line.

The \e tColor argument specifies the top (or left) color and \e bColor
specifies the bottom (or right) color.

The \e lw argument specifies the line width for each of the lines. It is
not the total line width.

The \e mw argument specifies the width of a middle line drawn in \e mColor.

If \e tColor is brighter that \e bColor, the line appears to be
raised from the surface.  If \e tColor is darker that \e bColor, the line
appears to be sunken into the surface.

\sa drawShadeRect() and drawShadePanel().
*/

void QPainter::drawShadeLine( int x1, int y1, int x2, int y2,
			      const QColor &tColor, const QColor &bColor,
			      int lw, const QColor &mColor, int mlw )
{
    int tlw = lw*2 + mlw;			// total line width
    QPen oldPen = pen();			// save pen
    setPen( tColor );				// makes new cpen
    if ( y1 == y2 ) {				// horizontal line
	int y = y1 - tlw/2;
	if ( x1 > x2 ) {			// swap x1 and x2
	    int t = x1;
	    x1 = x2;
	    x2 = t;
	}
	x2--;
	QPointArray a;
	int i;
	for ( i=0; i<lw; i++ ) {		// draw top shadow
	    a.setPoints( 3, x1+i, y+tlw-1,
			    x1+i, y+i,
			    x2,	  y+i );
	    drawPolyline( a );
	}
	setPen( mColor );
	for ( i=0; i<mlw; i++ )			// draw lines in the middle
	    drawLine( x1+lw, y+lw+i, x2-lw, y+lw+i );
	setPen( bColor );
	for ( i=0; i<lw; i++ ) {		// draw bottom shadow
	    a.setPoints( 3, x1+lw, y+tlw-i-1,
			    x2-i,  y+tlw-i-1,
			    x2-i,  y+lw );
	    drawPolyline( a );
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
	QPointArray a;
	int i;
	for ( i=0; i<lw; i++ ) {		// draw top shadow
	    a.setPoints( 3, x+i,     y2,
			    x+i,     y1+i,
			    x+tlw-1, y1+i );
	    drawPolyline( a );
	}
	setPen( mColor );
	for ( i=0; i<mlw; i++ )			// draw lines in the middle
	    drawLine( x+lw+i, y1+lw, x+lw+i, y2 );
	setPen( bColor );
	for ( i=0; i<lw; i++ ) {		// draw bottom shadow
	    a.setPoints( 3, x+lw,      y2-i,
			    x+tlw-i-1, y2-i,
			    x+tlw-i-1, y1+lw );
	    drawPolyline( a );
	}
    }
    setPen( oldPen );
}


/*!
Draw a shaded rectangle given by \e (x,y,w,h).

The arguments have the same meaning as for drawShadeLine().
\sa drawShadeLine(), drawShadePanel()
*/

void QPainter::drawShadeRect( int x, int y, int w, int h,
			      const QColor &tColor, const QColor &bColor,
			      int lw, const QColor &mColor, int mlw )
{
    if ( w < 1 || h < 1 || lw < 0 || mlw < 0 )	// bad parameters
	return;
    QPen oldPen = pen();			// save pen
    setPen( tColor );				// makes new cpen
    int x1=x, y1=y, x2=x+w-1, y2=y+h-1;
    QPointArray a;
    if ( lw == 1 && mlw == 0 ) {		// standard shade rectangle
	a.setPoints( 8, x1,y1, x2,y1, x1,y1+1, x1,y2, x1+2,y2-1,
		     x2-1,y2-1, x2-1,y1+2,  x2-1,y2-2 );
	drawLineSegments( a );			// draw top lines
	setPen( bColor );
	a.setPoints( 8, x1+1,y1+1, x2,y1+1, x1+1,y1+2, x1+1,y2-1,
		     x1+1,y2, x2,y2,  x2,y1+2, x2,y2-1 );
	drawLineSegments( a );			// draw bottom lines
    }
    else {					// more complicated
	int t = lw*2+mlw;
	int m = lw+mlw;
	int i, j=0, k=m;
	for ( i=0; i<lw; i++ ) {		// draw top shadow
	    drawLine( x1+j, y2-j, x1+j, y1+j );
	    drawLine( x1+j, y1+j, x2-j, y1+j );
	    drawLine( x1+t, y2-k, x2-k, y2-k );
	    drawLine( x2-k, y2-k, x2-k, y1+k );
	    j++;
	    k++;
	}
	setPen( mColor );
	j = lw*2;
	for ( i=0; i<mlw; i++ ) {		// draw lines in the middle
	    drawRect( x1+lw+i, y1+lw+i, w-j, h-j );
	    j += 2;
	}
	setPen( bColor );
	j = 0;
	k = m;
	for ( i=0; i<lw; i++ ) {		// draw bottom shadow
	    drawLine( x1+1+j,y2-j, x2-j, y2-j );
	    drawLine( x2-j,  y2-j, x2-j, y1+j+1 );
	    drawLine( x1+k,  y2-m, x1+k, y1+k );
	    drawLine( x1+k,  y1+k, x2-k, y1+k );
	    j++;
	    k++;
	}
    }
    setPen( oldPen );				// restore pen
}


/*!
Draw a shaded panel given by \e (x,y,w,h).
*/

void QPainter::drawShadePanel( int x, int y, int w, int h,
			       const QColor &tColor, const QColor &bColor,
			       int lw, const QColor &fColor, bool fill )
{
    if ( w < 1 || h < 1 || lw <= 0 )		// bad parameters
	return;
    QPen oldPen = pen();			// save pen
    QPointArray a( 4*lw );
    setPen( tColor );
    int x1, y1, x2, y2;
    int i;
    int n = 0;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for ( i=0; i<lw; i++ ) {			// top shadow
	a.setPoint( n++, x1, y1++ );
	a.setPoint( n++, x2--, y2++ );
    }
    x2 = x1;
    y1 = y+h-2;
    for ( i=0; i<lw; i++ ) {			// left shadow
	a.setPoint( n++, x1++, y1 );
	a.setPoint( n++, x2++, y2-- );
    }
    drawLineSegments( a );
    n = 0;
    setPen( bColor );
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for ( i=0; i<lw; i++ ) {			// bottom shadow
	a.setPoint( n++, x1++, y1-- );
	a.setPoint( n++, x2, y2-- );
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lw-1;
    for ( i=0; i<lw; i++ ) {			// right shadow
	a.setPoint( n++, x1--, y1++ );
	a.setPoint( n++, x2--, y2 );
    }
    drawLineSegments( a );
    if ( fill ) {				// fill with fill color
	cpen.setStyle( NoPen );
	QBrush oldBrush = brush();
	setBrush ( fColor );
	drawRect( x+lw, y+lw, w-lw*2, h-lw*2 );
	setBrush( oldBrush );
    }
    setPen( oldPen );				// restore pen
}


// --------------------------------------------------------------------------
// Convenience function for filling a rectangle.
//

/*!
Fills the rectangle \e (x,y,w,h) with a \e color.
*/

void QPainter::fillRect( int x, int y, int w, int h, const QColor &color )
{
    QPen oldPen = pen();			// save pen
    QPen pen( black, 0, NoPen );
    setPen( pen );
    QBrush oldBrush = brush();			// save brush
    setBrush( color );
    drawRect( x, y, w, h );			// draw filled rect
    setBrush( oldBrush );			// restore brush
    setPen( oldPen );				// restore pen
}


// --------------------------------------------------------------------------
// QPainter member functions (inline if DEBUG not defined)
//

/*!
Synonymous to setBrushOrigin(x,y).
*/

void QPainter::setBrushOrigin( const QPoint &p )
{
    setBrushOrigin( p.x(), p.y() );
}

/*!
Synonymous to setWindow(x,y,w,h).
*/

void QPainter::setWindow( const QRect &r )
{
    setWindow( r.x(), r.y(), r.width(), r.height() );
}

/*!
Synonymous to setViewport(x,y,w,h).
*/

void QPainter::setViewport( const QRect &r )
{
    setViewport( r.x(), r.y(), r.width(), r.height() );
}

/*!
Clips graphics to the rectangle defined by \e x,\e y,\e w and \e h.

Clipping is automatically enabled.

\sa setClipRegion().
*/

void QPainter::setClipRect( int x, int y, int w, int h )
{
    setClipRect( QRect(x,y,w,h) );
}

/*!
Synonymous to drawPoint(x,y).
*/

void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.x(), p.y() );
}

/*!
Synonymous to moveTo(x,y).
*/

void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.x(), p.y() );
}

/*!
Synonymous to lineTo(x,y).
*/

void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.x(), p.y() );
}

/*!
Synonymous to drawLine(x1,y1,x2,y2).
*/

void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.x(), p1.y(), p2.x(), p2.y() );
}

/*!
Synonymous to drawRect(x,y,w,h).
*/

void QPainter::drawRect( const QRect &r )
{
    drawRect( r.x(), r.y(), r.width(), r.height() );
}

/*!
Synonymous to drawRoundRect(x,y,w,h,xRnd,yRnd).
*/

void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.x(), r.y(), r.width(), r.height(), xRnd, yRnd );
}

/*!
Synonymous to drawEllipse(x,y,w,h).
*/

void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.x(), r.y(), r.width(), r.height() );
}

/*!
Synonymous to drawArc(x,y,w,h,a1,a2).
*/

void QPainter::drawArc( const QRect &r, int a1, int a2 )
{
    drawArc( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

/*!
Synonymous to drawPie(x,y,w,h,a1,a2).
*/

void QPainter::drawPie( const QRect &r, int a1, int a2 )
{
    drawPie( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

/*!
Synonymous to drawChord(x,y,w,h,a1,a2).
*/

void QPainter::drawChord( const QRect &r, int a1, int a2 )
{
    drawChord( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

/*!
Synonymous to drawPixmap(x,y,pm,sx,sy,sw,sh).
*/

void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm,
			   const QRect &sr )
{
    drawPixmap( p.x(), p.y(), pm, sr.x(), sr.y(), sr.width(), sr.height() );
}

/*!
Synonymous to fillRect(x,y,w,h,c).
*/

void QPainter::fillRect( const QRect &r, const QColor &c )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), c );
}

/*!
Calls <code>fillRect( x, y, w, h, backgroundColor() )</code>.
*/

void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

/*!
Synonymous to eraseRect(x,y,w,h).
*/

void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), backgroundColor() );
}

/*!
Synonymous to drawShadeLine(x1,y1,x2,y2,...).
*/

void QPainter::drawShadeLine( const QPoint &p1, const QPoint &p2,
			      const QColor &tc, const QColor &bc,
			      int lw, const QColor &mc, int mlw )
{
    drawShadeLine( p1.x(), p1.y(), p2.x(), p2.y(), tc, bc, lw, mc, mlw );
}

/*!
Synonymous to drawShadeRect(x1,y1,x2,y2,...).
*/

void QPainter::drawShadeRect( const QRect &r,
			      const QColor &tc, const QColor &bc,
			      int lw, const QColor &mc, int mlw )
{
    drawShadeRect( r.x(), r.y(), r.width(), r.height(), tc, bc, lw, mc, mlw );
}

/*!
Synonymous to drawShadePanel(x1,y1,x2,y2,...).
*/

void QPainter::drawShadePanel( const QRect &r,
			       const QColor &tc, const QColor &bc,
			       int lw, const QColor &fc, bool fill )
{
    drawShadePanel( r.x(), r.y(), r.width(), r.height(), tc, bc, lw, fc, fill);
}

/*!
Synonymous to drawText(x,y,...).
*/

void QPainter::drawText( const QPoint &p, const char *s, int len )
{
    drawText( p.x(), p.y(), s, len );
}

/*!
Synonymous to drawText(x,y,w,h,...).
*/

void QPainter::drawText( const QRect &r, int tf, const char *str, int len,
			 QRect *br, char **i )
{
    drawText( r.x(), r.y(), r.width(), r.height(), tf, str, len, br, i );
}

/*!
Synonymous to boundingRect(x,y,w,h,...).
*/

QRect QPainter::boundingRect( const QRect &r, int tf,
			      const char *str, int len, char **i )
{
    return boundingRect( r.x(), r.y(), r.width(), r.height(), tf, str, len,
			 i );
}


// --------------------------------------------------------------------------
// QPen stream functions
//

/*!
\relates QPen
Writes a pen to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QPen &p )
{
    return s << (UINT8)p.style() << (UINT8)p.width() << p.color();
}

/*!
\relates QPen
Reads a pen from the stream.
*/

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

/*!
\relates QBrush
Writes a brush to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QBrush &b )
{
    s << (UINT8)b.style() << b.color();
    if ( b.style() == CustomPattern )
	s >> *b.bitmap();
    return s;
}

/*!
\relates QBrush
Reads a brush from the stream.
*/

QDataStream &operator>>( QDataStream &s, QBrush &b )
{
    UINT8 style;
    QColor color;
    s >> style;
    s >> color;
    if ( style == CustomPattern ) {
	QBitmap bm;
	s >> bm;
	b = QBrush( color, bm );
    }
    else
	b = QBrush( color, (BrushStyle)style );
    return s;
}

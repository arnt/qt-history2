/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.cpp#1 $
**
** Implementation of QPushButton class
**
** Author  : Haavard Nord
** Created : 940221
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qpushbt.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qpushbutton.cpp#1 $";
#endif


const int extraMacWidth = 6;			// extra size for def button
const int extraMacHeight = 6;
const int extraPMWidth = 2;
const int extraPMHeight = 2;
const int extraMotifWidth = 10;
const int extraMotifHeight = 10;


QPushButton::QPushButton( QView *parent, const char *label ) : QButton(parent)
{
    init();
    setText( label );
}

QPushButton::QPushButton( QView *parent, const QRect &r,
			  const char *label ) : QButton(parent)
{
    init();
    setText( label );
    QWidget::changeGeometry( r );
}

void QPushButton::init()
{
    initMetaObject();
    autoDefButton = defButton = lastDown = lastDef = FALSE;
    if ( guiStyle() != MacStyle )
	setBackgroundColor( lightGray );
}


void QPushButton::setAutoDefault( bool autoDef )
{
    autoDefButton = autoDef;
}

void QPushButton::setDefault( bool def )	// set default on/off
{
    if ( (defButton && def) || !(defButton || def) )
	return;					// no change
    defButton = def;
    if ( defButton )
	emit becameDefault();
    int gs = guiStyle();
    if ( gs != MacStyle && gs != MotifStyle ) {
	if ( isVisible() )
	    paintEvent( 0 );
    }
    else
	fixDefButton();
}


bool QPushButton::move( int x, int y )
{
    int wx, hx;
    extraSize( wx, hx, TRUE );
    return QWidget::move( x-wx/2, y-hx/2 );
}

bool QPushButton::resize( int w, int h )
{
    int wx, hx;
    extraSize( wx, hx, TRUE );
    return QWidget::resize( w+wx, h+hx );
}

bool QPushButton::changeGeometry( int x, int y, int w, int h )
{
    int wx, hx;
    extraSize( wx, hx, TRUE );
    return QWidget::changeGeometry( x-wx/2, y-hx/2, w+wx, h+hx );
}


bool QPushButton::extraSize( int &wx, int &hx, bool onlyWhenDefault )
{
    int gs = guiStyle();
    int w=0, h=0;
    bool hasExtra = TRUE;
    if ( gs == MacStyle ) {			// larger def mac buttons
	w = extraMacWidth;
	h = extraMacHeight;
    }
    else
    if ( gs == MotifStyle ) {			// larger def motif buttons
	w = extraMotifWidth;
	h = extraMotifHeight;
    }
    else {
	w = h = 0;
	hasExtra = FALSE;
    }
    if ( hasExtra && onlyWhenDefault && !defButton )
	w = h = 0;
    wx = w;
    hx = h;
    return hasExtra;
}

void QPushButton::fixDefButton()
{
    int wx, hx;
    if ( !extraSize( wx, hx, FALSE ) )
	return;
    if ( !defButton ) {				// not default -> shrink
	wx = -wx;
	hx = -hx;
    }
    QRect r = geometry();
    QWidget::changeGeometry( r.left()-wx/2, r.top()-hx/2,
			     r.width()+wx, r.height()+hx );
}


void QPushButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GuiStyle gs = guiStyle();
    bool updated = isDown() != lastDown || lastDef != defButton;
    int x1, y1, x2, y2;
    clientRect().coords( &x1, &y1, &x2, &y2 );
    QPen pen( black );
    QBrush brush( white, NoBrush );

#define SAVE_PUSHBUTTON_PIXMAPS
#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int w, h;
    w = x2 + 1;
    h = y2 + 1;
    pmkey.sprintf( "push_%d_%d_%d_%d_%d", gs, isDown(), defButton, w, h );
    QPixMap *pm = findPixmap( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( 0, 0, *pm );
	drawButtonFace( p );
	lastDown = isDown();
	lastDef = defButton;
	return;
    }
    bool use_pm = acceptPixmap( w, h ) && !isDown();
    QPainter  pmpaint;
    if ( use_pm ) {
	pm = new QPixMap( QSize(w, h) );	// create new pixmap
	CHECK_PTR( pm );
	savePixmap( pmkey, pm );		// save for later use
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	p->setBackgroundColor( backgroundColor() );
	p->eraseRect( 0, 0, w, h );
    }
#endif

    p->setPen( pen );
    p->setBrush( brush );
    if ( gs == MacStyle ) {			// Macintosh push button
	if ( defButton ) {
	    pen.setWidth( 3 );
	    x1++; y1++; x2--; y2--;
	    p->drawRoundRect( x1, y1, x2-x1+1, y2-y1+1, 25, 25 );
	    x1 += extraMacWidth/2;
	    y1 += extraMacHeight/2;
	    x2 -= extraMacWidth/2;
	    y2 -= extraMacHeight/2;
	    pen.setWidth( 0 );
	}
	if ( updated ) {			// fill
	    brush.setStyle( SolidBrush );
	    brush.setColor( isDown() ? black : backgroundColor() );
	}
	p->drawRoundRect( x1, y1, x2-x1+1, y2-y1+1, 20, 20 );
	if ( updated )
	    brush.setStyle( NoBrush );
    }
    else if ( gs == WindowsStyle ) {		// Windows push button
	QPointArray a(8);
	a.setPoint( 0, x1+1, y1 );
	a.setPoint( 1, x2-1, y1 );
	a.setPoint( 2, x1+1, y2 );
	a.setPoint( 3, x2-1, y2 );
	a.setPoint( 4, x1, y1+1 );
	a.setPoint( 5, x1, y2-1 );
	a.setPoint( 6, x2, y1+1 );
	a.setPoint( 7, x2, y2-1 );
	p->drawLineSegments( a );		// draw frame
	x1++; y1++;
	x2--; y2--;
	if ( defButton || (autoDefButton & isDown()) ) {
	    p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
	    x1++; y1++;
	    x2--; y2--;
	}
	if ( isDown() )
	    p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, darkGray, lightGray,
			       1, 0, updated );
	else
	    p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, white, darkGray,
			       2, 2, updated );
    }
    else if ( gs == PMStyle ) {			// PM push button
	pen.setColor( darkGray );
	if ( updated ) {			// fill
	    brush.setStyle( SolidBrush );
	    brush.setColor( backgroundColor() );
	}
	p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
	if ( updated )
	    brush.setStyle( NoBrush );
	if ( !defButton ) {
	    pen.setColor( backgroundColor() );
	    p->drawPoint( x1, y1 );
	    p->drawPoint( x1, y2 );
	    p->drawPoint( x2, y1 );
	    p->drawPoint( x2, y2 );
	    pen.setColor( darkGray );
	}
	x1++; y1++;
	x2--; y2--;
	QPointArray atop(3), abottom(3);
	atop.setPoint( 0, x1, y2-1 );
	atop.setPoint( 1, x1, y1 );
	atop.setPoint( 2, x2, y1 );
	abottom.setPoint( 0, x1, y2 );
	abottom.setPoint( 1, x2, y2 );
	abottom.setPoint( 2, x2, y1+1 );
	QColor tc, bc;
	if ( isDown() ) {
	    tc = darkGray;
	    bc = white;
	}
	else {
	    tc = white;
	    bc = darkGray;
	}
	pen.setColor( tc );
	p->drawPolyline( atop );
	pen.setColor( bc );
	p->drawPolyline( abottom );
    }
    else if ( gs == MotifStyle ) {		// Motif push button
	QColor tColor, bColor;
	if ( defButton ) {			// default Motif button
	    p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, darkGray, white );
	    x1 += extraMotifWidth/2;
	    y1 += extraMotifHeight/2;
	    x2 -= extraMotifWidth/2;
	    y2 -= extraMotifHeight/2;
	}
	if ( isDown() ) {
	    tColor = black;
	    bColor = white;
	    p->setBackgroundColor( darkGray );
	}
	else {
	    tColor = white;
	    bColor = darkGray;
	}
	p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, tColor, bColor,
			   2, 2, updated );
    }

#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( 0, 0, *pm );
    }
#endif
    drawButtonFace( p );
    lastDown = isDown();
    lastDef = defButton;
}

void QPushButton::drawButtonFace( QPainter *paint )
{
    if ( !text() )
	return;
    register QPainter *p = paint;
    QSize sz = clientSize();
    int w = sz.width();
    GuiStyle gs = guiStyle();
    QPoint pos( w/2 - strlen(text())*3, sz.height()/2 + 4 );
    int dt;
    switch ( gs ) {
	case MacStyle:
	case MotifStyle:
	    p->pen().setColor( isDown() ? white : black );
	    dt = 0;
	    break;
	case WindowsStyle:
	case PMStyle:
	    p->pen().setColor( black );
	    dt = gs == WindowsStyle ? 2 : 1;
	    break;
    }
    if ( isDown() || isOn() )			// shift text
	pos += QPoint(dt,dt);
    p->drawText( pos, text() );
}

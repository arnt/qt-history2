/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbt.cpp#15 $
**
** Implementation of QPushButton class
**
** Author  : Haavard Nord
** Created : 940221
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpushbt.h"
#include "qfontmet.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qpushbt.cpp#15 $";
#endif


const int extraMacWidth = 6;			// extra size for def button
const int extraMacHeight = 6;
const int extraPMWidth = 2;
const int extraPMHeight = 2;
const int extraMotifWidth = 10;
const int extraMotifHeight = 10;


static bool extraSize( const QPushButton *b, int &wx, int &hx,
		       bool onlyWhenDefault )
{
    if ( onlyWhenDefault && !b->isDefault() ) {
	wx = hx = 0;
	return FALSE;
    }
    switch ( b->style() ) {
	case MacStyle:				// larger def Mac buttons
	    wx = extraMacWidth;
	    hx = extraMacHeight;
	    break;
	case MotifStyle:			// larger def Motif buttons
	    wx = extraMotifWidth;
	    hx = extraMotifHeight;
	    break;
	default:
	    wx = hx = 0;
	    return FALSE;
    }
    return TRUE;
}

static void resizeDefButton( QPushButton *b )
{
    int wx, hx;
    if ( !extraSize( b, wx, hx, FALSE ) )
	return;
    if ( !b->isDefault() ) {			// not default -> shrink
	wx = -wx;
	hx = -hx;
    }
    QRect r = b->geometry();
    b->QWidget::setGeometry( r.x()-wx/2, r.y()-hx/2,
			     r.width()+wx, r.height()+hx );
}


QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

QPushButton::QPushButton( const char *label, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setLabel( label );
}

void QPushButton::init()
{
    initMetaObject();
    autoDefButton = defButton = lastDown = lastDef = FALSE;
    if ( style() != MacStyle )
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
    int gs = style();
    if ( gs != MacStyle && gs != MotifStyle ) {
	if ( isVisible() )
	    paintEvent( 0 );
    }
    else
	resizeDefButton( (QPushButton*)this );
}


void QPushButton::resizeFitLabel()
{
    QFontMetrics fm = fontMetrics();
    int w = fm.width( label() );
    int h = fm.height();
    resize( w+6, h+6 );
}


void QPushButton::move( int x, int y )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::move( x-wx/2, y-hx/2 );
}

void QPushButton::move( const QPoint &p )
{
    move( p.x(), p.y() );
}

void QPushButton::resize( int w, int h )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::resize( w+wx, h+hx );
}

void QPushButton::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

void QPushButton::setGeometry( int x, int y, int w, int h )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::setGeometry( x-wx/2, y-hx/2, w+wx, h+hx );
}

void QPushButton::setGeometry( const QRect &r )
{
    setGeometry( r.x(), r.y(), r.width(), r.height() );
}


void QPushButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle 	gs = style();
    bool 	updated = isDown() != lastDown || lastDef != defButton;
    QColor	fillcol = backgroundColor();
    int 	x1, y1, x2, y2;

    clientRect().coords( &x1, &y1, &x2, &y2 );	// get coordinates
    QPen pen( black );
    QBrush brush( fillcol, NoBrush );

#define SAVE_PUSHBUTTON_PIXMAPS
#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int w, h;
    w = x2 + 1;
    h = y2 + 1;
    pmkey.sprintf( "push_%d_%d_%d_%d_%d", gs, isDown(), defButton, w, h );
    QPixMap *pm = findPixmap( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixMap( 0, 0, *pm );
	drawButtonFace( p );
	lastDown = isDown();
	lastDef = defButton;
	return;
    }
    bool use_pm = acceptPixmap( w, h ) && !isDown();
    QPainter  pmpaint;
    if ( use_pm ) {
	pm = new QPixMap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	savePixmap( pmkey, pm );		// save for later use
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	p->setBackgroundColor( fillcol );
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
	    if ( isDown() )
		brush.setColor( black );
	}
	p->drawRoundRect( x1, y1, x2-x1+1, y2-y1+1, 20, 20 );
    }
    else if ( gs == WindowsStyle ) {		// Windows push button
	QPointArray a;
	a.setPoints( 8, x1+1,y1, x2-1,y1, x1+1,y2, x2-1,y2,
		        x1,y1+1, x1,y2-1, x2,y1+1, x2,y2-1 );
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
			       1, fillcol, updated );
	else
	    p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, white, darkGray,
			       2, fillcol, updated );
    }
    else if ( gs == PMStyle ) {			// PM push button
	pen.setColor( darkGray );
	if ( updated )				// fill
	    brush.setStyle( SolidBrush );
	p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
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
	QPointArray atop, abottom;
	atop.setPoints( 3, x1,y2-1, x1,y1, x2,y1 );
	abottom.setPoints( 3, x1,y2, x2,y2, x2,y1+1 );
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
	    fillcol = darkGray;
	}
	else {
	    tColor = white;
	    bColor = darkGray;
	}
	p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, tColor, bColor,
			   2, fillcol, updated );
    }
    if ( brush.style() != NoBrush )
	brush.setStyle( NoBrush );

#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixMap( 0, 0, *pm );
    }
#endif
    drawButtonFace( p );
    lastDown = isDown();
    lastDef = defButton;
}

void QPushButton::drawButtonFace( QPainter *paint )
{
    if ( !label() )
	return;
    register QPainter *p = paint;
    GUIStyle gs = style();
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
	    dt = gs == WindowsStyle ? 2 : 0;
	    break;
    }
    QRect r = clientRect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( isDown() || isOn() ) {			// shift text
	x += dt;
	y += dt;
    }
    p->drawText( x+2, y+2, w-4, h-4,
		 AlignCenter|AlignVCenter|SingleLine|ShowPrefix, label() );
}

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.cpp#3 $
**
** Implementation of QCheckBox class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qchkbox.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qcheckbox.cpp#3 $";
#endif


QCheckBox::QCheckBox( QView *parent, const char *label ) : QButton(parent)
{
    initMetaObject();
    setText( label );
    setOnOffButton( TRUE );
}

QCheckBox::QCheckBox( QView *parent, const QRect &r,
		      const char *label ) : QButton(parent)
{
    initMetaObject();
    setText( label );
    changeGeometry( r );
    setOnOffButton( TRUE );
}


void QCheckBox::setChecked( bool checked )
{
    if ( checked )
	switchOn();
    else
	switchOff();
}


void QCheckBox::drawButton( QPainter *paint )	// draw check box
{
    register QPainter *p = paint;
    GuiStyle gs = guiStyle();
    QSize sz = clientSize();
    int x=0, y, w, h;
    switch ( gs ) {				// calculate coords
	case MacStyle:
	case WindowsStyle:
	    w = h = 13;
	    y = sz.height()/2-w/2;
	    break;
	case PMStyle:
	    w = h = 16;
	    y = sz.height()/2-w/2;
	    break;
	case MotifStyle:
	    x = 1;
	    w = h = 10;
	    y = sz.height()/2-w/2;
	    break;
    }
#define SAVE_CHECKBOX_PIXMAPS
#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    pmkey.sprintf( "check_%d_%d_%d", gs, isDown(), isOn() );
    QPixMap *pm = findPixmap( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixMap( x, y, *pm );
	if ( text() ) {				// draw text extra
	    p->pen().setColor( foregroundColor() );
	    p->drawText( QPoint(x+w+6,sz.height()/2+4), text() );
	}
	return;
    }
    bool use_pm = acceptPixmap( w, h ) && !isDown();
    QPainter  pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixMap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	savePixmap( pmkey, pm );		// save for later use
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	wx=x;  wy=y;				// save x,y coords
	x = y = 0;
	p->setBackgroundColor( backgroundColor() );
    }
#endif

    if ( gs == MacStyle || gs == WindowsStyle ){// Mac/Windows check box
	p->eraseRect( x, y, w, h );
	p->pen().setColor( black );
	p->drawRect( x, y, w, h );
	if ( isDown() )				// extra fat rectangle
	    p->drawRect( x+1, y+1, w-2, h-2 );
	if ( isOn() ) {
	    p->drawLine( x, y, x+w-1, y+h-1 );	// draw cross
	    p->drawLine( x, y+h-1, x+w-1, y );
	}
    }
    else if ( gs == PMStyle ) {			// PM check box
	QPen pen( darkGray );
	QBrush brush( backgroundColor(), SolidBrush );
	p->setPen( pen );
	p->setBrush( brush );
	p->drawRect( x, y, w, h );
	brush.setStyle( NoBrush );
	int x1 = x+1, y1=y+1, x2=x+w-2, y2=y+h-2;
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
	pen.setColor( backgroundColor() );
	p->drawPoint( x1, y2 );
	p->drawPoint( x2, y1 );
	static QCOOT check_mark[] = {
	    3,5, 5,5,  4,6, 5,6,  5,7, 6,7,  5,8, 6,8,	6,9, 9,9,
	    6,10, 8,10,	 7,11, 8,11,  7,12, 7,12,  8,8, 9,8,  8,7, 10,7,
	    9,6, 10,6,	9,5, 11,5,  10,4, 11,4,	 10,3, 12,3,
	    11,2, 12,2,	 11,1, 13,1,  12,0, 13,0 };
	static QCOOT check_mark_pix[] = {
	    3,6, 6,6, 4,7, 7,8, 5,9, 6,11, 8,12, 9,10, 10,8, 8,6,
	    11,6, 9,4, 12,4, 10,2, 13,2 };
	if ( isOn() ) {				// draw complex check mark
	    x1 = x;
	    y1 = y;
	    if ( isDown() ) {			// shift check mark
		x1++;
		y1++;
	    }
	    QPointArray amark( check_mark,
			       sizeof(check_mark)/(sizeof(QCOOT)*2) );
	    amark.move( x1, y1 );
	    pen.setColor( black );
	    p->drawLineSegments( amark );
	    pen.setColor( darkGray );
	    for ( int i=0; i<sizeof(check_mark_pix)/sizeof(QCOOT); i+=2 )
		p->drawPoint( x1 + check_mark_pix[i],
			      y1 + check_mark_pix[i+1] );
	}
    }
    else if ( gs == MotifStyle ) {		// Motif check box
	QColor tColor, bColor;
	if ( (isUp() && !isOn()) || (isDown() && isOn()) ) {
	    tColor = white;			// button is up
	    bColor = darkGray;
	}
	else {					// button is down
	    tColor = black;
	    bColor = white;
	    p->setBackgroundColor( darkGray );
	}
	p->drawShadePanel( x, y, w, h, tColor, bColor, 2, 2, TRUE );
    }

#if defined(SAVE_CHECKBOX_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixMap( wx, wy, *pm );
	w += wx;
    }
#endif
    if ( text() ) {				// draw check box text
	p->pen().setColor( foregroundColor() );
	p->drawText( QPoint(w+x+6,sz.height()/2+4), text() );
    }
}

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobutton.cpp#1 $
**
** Implementation of QRadioButton class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qradiobt.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qradiobutton.cpp#1 $";
#endif


QRadioButton::QRadioButton( QView *parent, const char *label ) : QButton(parent)
{
    initMetaObject();
    setText( label );
    setOnOffButton( TRUE );
    noHit = FALSE;
}

QRadioButton::QRadioButton( QView *parent, const QRect &r,
		      const char *label ) : QButton(parent)
{
    initMetaObject();
    setText( label );
    changeGeometry( r );
    setOnOffButton( TRUE );
    noHit = FALSE;
}


void QRadioButton::setChecked( bool checked )
{
    if ( checked )
	switchOn();
    else
	switchOff();
}


bool QRadioButton::hitButton( const QPoint &pos ) const
{
    return noHit ? FALSE : clientRect().contains( pos );
}

void QRadioButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GuiStyle gs = guiStyle();
    QSize sz = clientSize();
    int x=0, y, w, h;
    switch ( gs ) {
	case MacStyle:
	case WindowsStyle:
	    w = h = 15;
	    break;
	case PMStyle:
	    w = h = 16;
	    break;
	case MotifStyle:
	    w = h = 13;
	    break;
    }
    y = sz.height()/2 - w/2;
#define SAVE_RADIOBUTTON_PIXMAPS
#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    pmkey.sprintf( "radio_%d_%d_%d", gs, isDown(), isOn() );
    QPixMap *pm = findPixmap( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( x, y, *pm );
	if ( text() ) {				// draw text extra
	    p->pen().setColor( foregroundColor() );
	    p->drawText( QPoint(w+6,sz.height()/2+4), text() );
	}
	return;
    }
    bool use_pm = acceptPixmap( w, h ) && !isDown();
    QPainter  pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixMap( QSize(w, h) );	// create new pixmap
	CHECK_PTR( pm );
	savePixmap( pmkey, pm );		// save for later use
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	wx=x;  wy=y;				// save x,y coords
	x = y = 0;
	p->setBackgroundColor( backgroundColor() );
    }
#endif

    if ( gs == MacStyle || gs == WindowsStyle ){// Mac/Windows radio button
	static QCOOT pts1[] = {			// normal circle
	    5,0, 7,0, 8,1, 9,1, 11,3, 11,4, 12,5, 12,7,
	    11,8, 11,9, 9,11, 8,11, 7,12, 5,12, 4,11, 3,11,
	    1,9, 1,8, 0,7, 0,5, 1,4, 1,3, 3,1, 4,1 };
	static QCOOT pts2[] =  {		// fat circle
	    5,1, 7,1, 8,2, 9,2, 10,3, 10,4, 11,5, 11,7,
	    10,8, 10,9, 9,10, 8,10, 7,11, 5,11, 4,10, 3,10,
	    2,9, 2,8, 1,7, 1,5, 2,4, 2,3, 3,2, 4,2 };
	static QCOOT pts3[] =  {		// check mark
	    5,3, 7,3, 9,5, 9,7, 7,9, 5,9, 3,7, 3,5 };
	QPointArray a( pts1, sizeof(pts1)/(sizeof(QCOOT)*2) );
	a.move( x, y );
	p->eraseRect( x, y, w, h );
	p->drawPolyline( a );
	if ( isDown() ) {			// draw fat circle
	    a.setPoints( pts2, sizeof(pts2)/(sizeof(QCOOT)*2) );
	    a.move( x, y );
	    p->drawPolyline( a );
	}
	if ( isOn() ) {				// draw check mark
	    a.setPoints( pts3, sizeof(pts3)/(sizeof(QCOOT)*2) );
	    a.move( x, y );
	    p->setBrush( QBrush(black) );
	    p->drawPolygon( a );
	}
    }
    else if ( gs == PMStyle ) {			// PM radio button
	static QCOOT pts1[] = {			// normal circle
	    5,0, 10,0, 11,1, 12,1, 13,2, 14,3, 14,4, 15,5,
	    15,10, 14,11, 14,12, 13,13, 12,14, 11,14, 10,15,
	    5,15, 4,14, 3,14, 2,13, 1,12, 1,11, 0,10, 0,5,
	    1,4, 1,3, 2,2, 3,1, 4,1 };
	static QCOOT pts2[] = {			// top left shadow
	    5,1, 10,1,	3,2, 7,2,  2,3, 5,3,  2,4, 4,4,
	    1,5, 3,5,  1,6, 1,10,  2,6, 2,7 };
	static QCOOT pts3[] = {			// bottom right, dark
	    5,14, 10,14,  7,13, 12,13,	10,12, 13,12,
	    11,11, 13,11,  12,10, 14,10,  13,8, 13,9,
	    14,5, 14,9 };
	static QCOOT pts4[] = {			// bottom right, light
	    5,14, 10,14,  9,13, 12,13,	11,12, 13,12,
	    12,11, 13,11,  13,9, 13,10,	 14,5, 14,10 };
	static QCOOT pts5[] = {			// check mark
	    6,4, 8,4, 10,6, 10,8, 8,10, 6,10, 4,8, 4,6 };
	static QCOOT pts6[] = {			// check mark extras
	    4,5, 5,4,  9,4, 10,5,  10,9, 9,10,	5,10, 4,9 };
	QPen pen( darkGray );
	p->setPen( pen );
	QPointArray a( pts1, sizeof(pts1)/(sizeof(QCOOT)*2) );
	a.move( x, y );
	p->eraseRect( x, y, w, h );
	p->drawPolyline( a );			// draw normal circle
	QColor tc, bc;
	QCOOT *bp;
	int    bl;
	if ( isDown() ) {			// pressed down
	    tc = darkGray;
	    bc = white;
	    bp = pts4;
	    bl = sizeof(pts4)/(sizeof(QCOOT)*2);
	}
	else {					// released
	    tc = white;
	    bc = darkGray;
	    bp = pts3;
	    bl = sizeof(pts3)/(sizeof(QCOOT)*2);
	}
	pen.setColor( tc );
	a.setPoints( pts2, sizeof(pts2)/(sizeof(QCOOT)*2) );
	a.move( x, y );
	p->drawLineSegments( a );		// draw top shadow
	pen.setColor( bc );
	a.setPoints( bp, bl );
	a.move( x, y );
	p->drawLineSegments( a );
	if ( isOn() ) {				// draw check mark
	    int x1=x, y1=y;
	    if ( isDown() ) {
		x1++;
		y1++;
	    }
	    QBrush brush( black );
	    p->setBrush( brush );
	    pen.setColor( black );
	    a.setPoints( pts5, sizeof(pts5)/(sizeof(QCOOT)*2) );
	    a.move( x1, y1 );
	    p->drawPolygon( a );
	    brush.setStyle( NoBrush );
	    pen.setColor( darkGray );
	    a.setPoints( pts6, sizeof(pts6)/(sizeof(QCOOT)*2) );
	    a.move( x1, y1 );
	    p->drawLineSegments( a );
	}
    }
    else if ( gs == MotifStyle ) {		// Motif radio button
	static QCOOT inner_pts[] =		// used for filling diamond
	    { 3,6, 6,2, 10,4, 6,10 };
	static QCOOT top_pts[] =		// top (^) of diamond
	    { 0,6, 6,0 , 11,5, 10,5, 6,1, 1,6, 2,6, 6,2, 9,5 };
	static QCOOT bottom_pts[] =		// bottom (V) of diamond
	    { 1,7, 6,12, 12,6, 11,6, 6,11, 2,7, 3,7, 6,10, 10,6 };
	QPen   pen( black, 0, NoPen );
	bool   showUp = (isUp() && !isOn()) || (isDown() && isOn());
	QBrush brush( showUp ? backgroundColor() : darkGray );
	QPointArray a( inner_pts, sizeof(inner_pts)/(sizeof(QCOOT)*2) );
	p->eraseRect( x, y, w, h );
	p->setPen( pen );
	p->setBrush( brush );
	a.move( x, y );
	p->drawPolygon( a );			// clear inner area
	pen.setStyle( SolidLine );
	if ( showUp )
	    pen.setColor( white );
	brush.setStyle( NoBrush );
	a.setPoints( top_pts, sizeof(top_pts)/(sizeof(QCOOT)*2)	 );
	a.move( x, y );
	p->drawPolyline( a );			// draw top part
	pen.setColor( showUp ? black : white );
	a.setPoints( bottom_pts, sizeof(bottom_pts)/(sizeof(QCOOT)*2) );
	a.move( x, y );
	p->drawPolyline( a );			// draw bottom part
    }

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( wx, wy, *pm );
	w += wx;
    }
#endif
    if ( text() ) {
	p->pen().setColor( foregroundColor() );
	p->drawText( QPoint(w+6,sz.height()/2+4), text() );
    }
}


void QRadioButton::mouseReleaseEvent( QMouseEvent *e )
{
    if ( isOn() )				// cannot switch off
	noHit = TRUE;
    QButton::mouseReleaseEvent( e );		// send to button handler
    noHit = FALSE;
}

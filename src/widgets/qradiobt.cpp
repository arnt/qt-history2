/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobt.cpp#13 $
**
** Implementation of QRadioButton class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qradiobt.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qradiobt.cpp#13 $";
#endif


static void getSizeOfBitMap( GUIStyle gs, int *w, int *h )
{
    switch ( gs ) {
	case MacStyle:
	case WindowsStyle:
	    *w = *h = 15;
	    break;
	case PMStyle:
	    *w = *h = 16;
	    break;
	case MotifStyle:
	    *w = *h = 13;
	    break;
    }
}


QRadioButton::QRadioButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setOnOffButton( TRUE );
    noHit = FALSE;
}

QRadioButton::QRadioButton( const char *label, QWidget *parent,
			    const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setLabel( label );
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


void QRadioButton::resizeFitLabel()
{
    QFontMetrics fm = fontMetrics();
    int w = fm.width( label() );
    int h = fm.height();
    int wbm, hbm;
    getSizeOfBitMap( style(), &wbm, &hbm );
    if ( h < hbm )
	h = hbm;
    resize( w+wbm+6, h );
}


bool QRadioButton::hitButton( const QPoint &pos ) const
{
    return noHit ? FALSE : rect().contains( pos );
}

void QRadioButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle gs = style();
    QSize sz = size();
    QFontMetrics fm = fontMetrics();
    int x=0, y, w, h;
    getSizeOfBitMap( gs, &w, &h );
    y = sz.height()/2 - w/2;

#define SAVE_RADIOBUTTON_PIXMAPS
#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    pmkey.sprintf( "$qt_radio_%d_%d_%d", gs, isDown(), isOn() );
    QPixMap *pm = QPixMap::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixMap( x, y, *pm );
	if ( label() ) {			// draw text extra
	    p->pen().setColor( foregroundColor() );
	    p->drawText( w+6, sz.height()/2+fm.height()/2-fm.descent(),
			 label() );
	}
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixMap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	wx=x;  wy=y;				// save x,y coords
	x = y = 0;
	p->setBackgroundColor( backgroundColor() );
    }
#endif

#define QCOOTARRLEN(x) sizeof(x)/(sizeof(QCOOT)*2)

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
	QPointArray a( QCOOTARRLEN(pts1), pts1 );
	a.move( x, y );
	p->eraseRect( x, y, w, h );
	p->drawPolyline( a );
	if ( isDown() ) {			// draw fat circle
	    a.setPoints( QCOOTARRLEN(pts2), pts2 );
	    a.move( x, y );
	    p->drawPolyline( a );
	}
	if ( isOn() ) {				// draw check mark
	    a.setPoints( QCOOTARRLEN(pts3), pts3 );
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
	QPointArray a( QCOOTARRLEN(pts1), pts1 );
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
	    bl = QCOOTARRLEN(pts4);
	}
	else {					// released
	    tc = white;
	    bc = darkGray;
	    bp = pts3;
	    bl = QCOOTARRLEN(pts3);
	}
	pen.setColor( tc );
	a.setPoints( QCOOTARRLEN(pts2), pts2 );
	a.move( x, y );
	p->drawLineSegments( a );		// draw top shadow
	pen.setColor( bc );
	a.setPoints( bl, bp );
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
	    a.setPoints( QCOOTARRLEN(pts5), pts5 );
	    a.move( x1, y1 );
	    p->drawPolygon( a );
	    brush.setStyle( NoBrush );
	    pen.setColor( darkGray );
	    a.setPoints( QCOOTARRLEN(pts6), pts6 );
	    a.move( x1, y1 );
	    p->drawLineSegments( a );
	}
    }
    else if ( gs == MotifStyle ) {		// Motif radio button
	static QCOOT inner_pts[] =		// used for filling diamond
	    { 2,6, 6,2, 10,6, 6,10 };
	static QCOOT top_pts[] =		// top (^) of diamond
	    { 0,6, 6,0 , 11,5, 10,5, 6,1, 1,6, 2,6, 6,2, 9,5 };
	static QCOOT bottom_pts[] =		// bottom (V) of diamond
	    { 1,7, 6,12, 12,6, 11,6, 6,11, 2,7, 3,7, 6,10, 10,6 };
	QPen   pen( black, 0, NoPen );
	bool   showUp = (isUp() && !isOn()) || (isDown() && isOn());
	QColor bgc = backgroundColor();
	QBrush brush( showUp ? bgc : darkGray );
	QPointArray a( QCOOTARRLEN(inner_pts), inner_pts );
	p->eraseRect( x, y, w, h );
	p->setPen( pen );
	p->setBrush( brush );
	a.move( x, y );
	p->drawPolygon( a );			// clear inner area
	pen.setStyle( SolidLine );
	if ( showUp )
	    pen.setColor( white );
	brush.setStyle( NoBrush );
	a.setPoints( QCOOTARRLEN(top_pts), top_pts );
	a.move( x, y );
	p->drawPolyline( a );			// draw top part
	pen.setColor( showUp ? black : white );
	a.setPoints( QCOOTARRLEN(bottom_pts), bottom_pts );
	a.move( x, y );
	p->drawPolyline( a );			// draw bottom part
    }

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixMap( wx, wy, *pm );
	w += wx;
	QPixMap::insert( pmkey, pm );		// save for later use
    }
#endif
    if ( label() ) {
	QFontMetrics fm = fontMetrics();
	p->pen().setColor( foregroundColor() );
	p->drawText( w+6, sz.height()/2+fm.height()/2-fm.descent(), label() );
    }
}


void QRadioButton::mouseReleaseEvent( QMouseEvent *e )
{
    if ( isOn() )				// cannot switch off
	noHit = TRUE;
    QButton::mouseReleaseEvent( e );		// send to button handler
    noHit = FALSE;
}

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qchkbox.cpp#16 $
**
** Implementation of QCheckBox class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qchkbox.h"
#include "qpainter.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qchkbox.cpp#16 $";
#endif


static void getSizeOfBitMap( GUIStyle gs, int *w, int *h )
{
    switch ( gs ) {				// calculate coords
	case MacStyle:
	case WindowsStyle:
	    *w = *h = 13;
	    break;
	case PMStyle:
	    *w = *h = 16;
	    break;
	case MotifStyle:
	    *w = *h = 10;
    }
}


QCheckBox::QCheckBox( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setOnOffButton( TRUE );
}

QCheckBox::QCheckBox( const char *label, QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setLabel( label );
    setOnOffButton( TRUE );
}


void QCheckBox::setChecked( bool checked )
{
    if ( checked )
	switchOn();
    else
	switchOff();
}


void QCheckBox::resizeFitLabel()
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


void QCheckBox::drawButton( QPainter *paint )	// draw check box
{
    register QPainter *p = paint;
    GUIStyle     gs = style();
    QColorGroup  g  = colorGroup();
    QSize 	 sz = size();
    QFontMetrics fm = fontMetrics();
    int		 x=0, y, w, h;
    int		 wmore = 0;

    getSizeOfBitMap( gs, &w, &h );
    y = sz.height()/2 - w/2;

    if ( gs == MacStyle || gs == WindowsStyle )
	wmore = 1;
    else if ( gs == MotifStyle )
	wmore = 2;

#define SAVE_CHECKBOX_PIXMAPS
#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    pmkey.sprintf( "$qt_check_%d_%d_%d_%d", gs, palette().serialNumber(),
		   isDown(), isOn() );
    QPixMap *pm = QPixMap::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixMap( x, y, *pm );
	if ( label() ) {			// draw text extra
	    p->pen().setColor( g.text() );
	    p->drawText( w+6+wmore, sz.height()/2+fm.height()/2-fm.descent(),
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
	p->setBackgroundColor( g.background() );
    }
#endif

    if ( gs == MacStyle || gs == WindowsStyle ){// Mac/Windows check box
	p->eraseRect( x, y, w, h );
	p->pen().setColor( g.foreground() );
	p->drawRect( x, y, w, h );
	if ( isDown() )				// extra fat rectangle
	    p->drawRect( x+1, y+1, w-2, h-2 );
	if ( isOn() ) {
	    p->drawLine( x, y, x+w-1, y+h-1 );	// draw cross
	    p->drawLine( x, y+h-1, x+w-1, y );
	}
    }
    else if ( gs == PMStyle ) {			// PM check box
	QPen   pen( g.dark() );
	QBrush brush( g.background() );
	p->setPen( pen );
	p->setBrush( brush );
	p->drawRect( x, y, w, h );
	brush.setStyle( NoBrush );
	int x1 = x+1, y1=y+1, x2=x+w-2, y2=y+h-2;
	QPointArray atop, abottom;
	atop.setPoints( 3, x1,y2-1, x1,y1, x2,y1 );
	abottom.setPoints( 3, x1,y2, x2,y2, x2,y1+1 );
	QColor tc, bc;
	if ( isDown() ) {
	    tc = g.dark();
	    bc = g.light();
	}
	else {
	    tc = g.light();
	    bc = g.dark();
	}
	pen.setColor( tc );
	p->drawPolyline( atop );
	pen.setColor( bc );
	p->drawPolyline( abottom );
	pen.setColor( g.background() );
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
	    QPointArray amark( sizeof(check_mark)/(sizeof(QCOOT)*2),
			       check_mark );
	    amark.move( x1, y1 );
	    pen.setColor( g.foreground() );
	    p->drawLineSegments( amark );
	    pen.setColor( g.dark() );
	    for ( int i=0; i<sizeof(check_mark_pix)/sizeof(QCOOT); i+=2 )
		p->drawPoint( x1 + check_mark_pix[i],
			      y1 + check_mark_pix[i+1] );
	}
    }
    else if ( gs == MotifStyle ) {		// Motif check box
	QColor tColor, bColor, fColor;
	bool down;
	if ( (isUp() && !isOn()) || (isDown() && isOn()) ) {
	    tColor = g.light();			// button is up
	    bColor = g.dark();
	    fColor = g.background();
	}
	else {					// button is down
	    tColor = g.dark();
	    bColor = g.light();
	    fColor = g.mid();
	}
	p->drawShadePanel( x, y, w, h, tColor, bColor, 2, fColor, TRUE );
    }

#if defined(SAVE_CHECKBOX_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixMap( wx, wy, *pm );
	w += wx;
	QPixMap::insert( pmkey, pm );		// save for later use
    }
#endif
    if ( label() ) {				// draw check box text
	p->pen().setColor( g.text() );
	p->drawText( x+w+6+wmore, sz.height()/2+fm.height()/2-fm.descent(),
		     label() );
    }
}

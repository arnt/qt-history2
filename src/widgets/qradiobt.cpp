/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobt.cpp#26 $
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
#include "qpixmap.h"
#include "qpmcache.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qradiobt.cpp#26 $";
#endif


/*!
\class QRadioButton qradiobt.h
\brief The QRadioButton widget provides a radio button with a text label.

QRadioButton and QCheckBox are both toggle buttons, that is, they can be
switched on (checked) or off (unchecked).  Unlike check boxes, radio
buttons are normally organized in groups where only one radio button can be
switched on at a time.

The QButtonGroup widget is very useful for defining groups of radio buttons.
*/


static void getSizeOfBitmap( GUIStyle gs, int *w, int *h )
{
    switch ( gs ) {
	case MacStyle:
	case Win3Style:
	    *w = *h = 15;
	    break;
	case WindowsStyle:
	    *w = *h = 12;
	    break;
	case PMStyle:
	    *w = *h = 16;
	    break;
	case MotifStyle:
	    *w = *h = 13;
	    break;
	default:
	    *w = *h = 10;
    }
}


/*!
Constructs a radio button with no text.

The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setToggleButton( TRUE );
    noHit = FALSE;
}

/*!
Constructs a radio button with a text.

The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( const char *text, QWidget *parent,
			    const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setText( text );
    setToggleButton( TRUE );
    noHit = FALSE;
}


/*!
\fn bool QRadioButton::isChecked() const
Returns TRUE if the radio button is checked, or FALSE if it is not checked.

\sa setChecked().
*/

/*!
Checks the radio button if \e checked is TRUE, or unchecks it if \e checked
is FALSE.

Calling this function does not affect other radio buttons unless a radio
button group has been defined using the QButtonGroup widget.

\sa isChecked().
*/

void QRadioButton::setChecked( bool checked )
{
    if ( checked )
	switchOn();
    else
	switchOff();
}


/*!
Adjusts the size of the radio button to fit the contents.

This function is called automatically whenever the contents change and
auto-resizing is enabled.

\sa setAutoResizing()
*/

void QRadioButton::adjustSize()
{
    QFontMetrics fm = fontMetrics();
    int w = fm.width( text() );
    int h = fm.height();
    int wbm, hbm;
    getSizeOfBitmap( style(), &wbm, &hbm );
    if ( h < hbm )
	h = hbm;
    resize( w+wbm+6, h );
}


bool QRadioButton::hitButton( const QPoint &pos ) const
{
    return noHit ? FALSE : rect().contains( pos );
}


/*!
Draws the radio button.
*/

void QRadioButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	 gs = style();
    QColorGroup  g  = colorGroup();
    QSize	 sz = size();
    QFontMetrics fm = fontMetrics();
    int		 x, y, w, h;
    int		 wless = 0;

    getSizeOfBitmap( gs, &w, &h );
    x = 0;
    y = sz.height()/2 - w/2;

    if ( gs == WindowsStyle )
	wless = -1;
    else if ( gs != PMStyle )
	wless = 1;

#define SAVE_RADIOBUTTON_PIXMAPS
#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    pmkey.sprintf( "$qt_radio_%d_%d_%d_%d", gs, palette().serialNumber(),
		   isDown(), isOn() );
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( x, y, *pm );
	if ( text() ) {				// draw text extra
	    p->pen().setColor( g.text() );
	    p->drawText( w+6-wless, sz.height()/2+fm.height()/2-fm.descent(),
			 text() );
	}
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixmap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	wx=x;  wy=y;				// save x,y coords
	x = y = 0;
	p->setBackgroundColor( g.background() );
    }
#endif

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

    if ( gs == MacStyle || gs == Win3Style ){	// Mac/Windows 3.x radio button
	static QCOORD pts1[] = {		// normal circle
	    5,0, 7,0, 8,1, 9,1, 11,3, 11,4, 12,5, 12,7,
	    11,8, 11,9, 9,11, 8,11, 7,12, 5,12, 4,11, 3,11,
	    1,9, 1,8, 0,7, 0,5, 1,4, 1,3, 3,1, 4,1 };
	static QCOORD pts2[] =  {		// fat circle
	    5,1, 7,1, 8,2, 9,2, 10,3, 10,4, 11,5, 11,7,
	    10,8, 10,9, 9,10, 8,10, 7,11, 5,11, 4,10, 3,10,
	    2,9, 2,8, 1,7, 1,5, 2,4, 2,3, 3,2, 4,2 };
	static QCOORD pts3[] =  {		// check mark
	    5,3, 7,3, 9,5, 9,7, 7,9, 5,9, 3,7, 3,5 };
	QPointArray a( QCOORDARRLEN(pts1), pts1 );
	a.move( x, y );
	p->eraseRect( x, y, w, h );
	p->setPen( g.foreground() );
	p->drawPolyline( a );
	if ( isDown() ) {			// draw fat circle
	    a.setPoints( QCOORDARRLEN(pts2), pts2 );
	    a.move( x, y );
	    p->drawPolyline( a );
	}
	if ( isOn() ) {				// draw check mark
	    a.setPoints( QCOORDARRLEN(pts3), pts3 );
	    a.move( x, y );
	    p->setBrush( g.foreground() );
	    p->drawPolygon( a );
	}
    }
    else if ( gs == WindowsStyle ) {		// Windows radio button
	static QCOORD pts1[] = {		// dark lines
	    1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
	static QCOORD pts2[] = {		// black lines
	    2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
	static QCOORD pts3[] = {		// background lines
	    2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
	static QCOORD pts4[] = {		// white lines
	    2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
	    11,4, 10,3, 10,2 };
	static QCOORD pts5[] = {		// inner fill
	    4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
	p->eraseRect( x, y, w, h );
	QPointArray a( QCOORDARRLEN(pts1), pts1 );
	a.move( x, y );
	p->setPen( g.dark() );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts2), pts2 );
	a.move( x, y );
	p->setPen( black );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts3), pts3 );
	a.move( x, y );
	p->setPen( g.background() );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts4), pts4 );
	a.move( x, y );
	p->setPen( white );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts5), pts5 );
	a.move( x, y );
	p->setPen( NoPen );
	p->setBrush( isDown() ? g.background() : g.base() );
	p->drawPolygon( a );
	if ( isOn() ) {
	    p->setBrush( g.foreground() );
	    p->drawRect( x+5, y+4, 2, 4 );
	    p->drawRect( x+4, y+5, 4, 2 );
	}
    }
    else if ( gs == PMStyle ) {			// PM radio button
	static QCOORD pts1[] = {		// normal circle
	    5,0, 10,0, 11,1, 12,1, 13,2, 14,3, 14,4, 15,5,
	    15,10, 14,11, 14,12, 13,13, 12,14, 11,14, 10,15,
	    5,15, 4,14, 3,14, 2,13, 1,12, 1,11, 0,10, 0,5,
	    1,4, 1,3, 2,2, 3,1, 4,1 };
	static QCOORD pts2[] = {		// top left shadow
	    5,1, 10,1,	3,2, 7,2,  2,3, 5,3,  2,4, 4,4,
	    1,5, 3,5,  1,6, 1,10,  2,6, 2,7 };
	static QCOORD pts3[] = {		// bottom right, dark
	    5,14, 10,14,  7,13, 12,13,	10,12, 13,12,
	    11,11, 13,11,  12,10, 14,10,  13,8, 13,9,
	    14,5, 14,9 };
	static QCOORD pts4[] = {		// bottom right, light
	    5,14, 10,14,  9,13, 12,13,	11,12, 13,12,
	    12,11, 13,11,  13,9, 13,10,	 14,5, 14,10 };
	static QCOORD pts5[] = {		// check mark
	    6,4, 8,4, 10,6, 10,8, 8,10, 6,10, 4,8, 4,6 };
	static QCOORD pts6[] = {		// check mark extras
	    4,5, 5,4,  9,4, 10,5,  10,9, 9,10,	5,10, 4,9 };
	p->setPen( g.dark() );
	QPointArray a( QCOORDARRLEN(pts1), pts1 );
	a.move( x, y );
	p->eraseRect( x, y, w, h );
	p->drawPolyline( a );			// draw normal circle
	QColor tc, bc;
	QCOORD *bp;
	int     bl;
	if ( isDown() ) {			// pressed down
	    tc = g.dark();
	    bc = g.light();
	    bp = pts4;
	    bl = QCOORDARRLEN(pts4);
	}
	else {					// released
	    tc = g.light();
	    bc = g.dark();
	    bp = pts3;
	    bl = QCOORDARRLEN(pts3);
	}
	p->pen().setColor( tc );
	a.setPoints( QCOORDARRLEN(pts2), pts2 );
	a.move( x, y );
	p->drawLineSegments( a );		// draw top shadow
	p->pen().setColor( bc );
	a.setPoints( bl, bp );
	a.move( x, y );
	p->drawLineSegments( a );
	if ( isOn() ) {				// draw check mark
	    int x1=x, y1=y;
	    if ( isDown() ) {
		x1++;
		y1++;
	    }
	    p->setBrush( g.foreground() );
	    p->pen().setColor( g.foreground() );
	    a.setPoints( QCOORDARRLEN(pts5), pts5 );
	    a.move( x1, y1 );
	    p->drawPolygon( a );
	    p->brush().setStyle( NoBrush );
	    p->pen().setColor( g.dark() );
	    a.setPoints( QCOORDARRLEN(pts6), pts6 );
	    a.move( x1, y1 );
	    p->drawLineSegments( a );
	}
    }
    else if ( gs == MotifStyle ) {		// Motif radio button
	static QCOORD inner_pts[] =		// used for filling diamond
	    { 2,6, 6,2, 10,6, 6,10 };
	static QCOORD top_pts[] =		// top (^) of diamond
	    { 0,6, 6,0 , 11,5, 10,5, 6,1, 1,6, 2,6, 6,2, 9,5 };
	static QCOORD bottom_pts[] =		// bottom (V) of diamond
	    { 1,7, 6,12, 12,6, 11,6, 6,11, 2,7, 3,7, 6,10, 10,6 };
	bool showUp = (isUp() && !isOn()) || (isDown() && isOn());
	QPointArray a( QCOORDARRLEN(inner_pts), inner_pts );
	p->eraseRect( x, y, w, h );
	p->setPen( NoPen );
	p->setBrush( showUp ? g.background() : g.mid() );
	a.move( x, y );
	p->drawPolygon( a );			// clear inner area
	p->pen().setStyle( SolidLine );
	p->pen().setColor( showUp ? g.light() : g.dark() );
	p->brush().setStyle( NoBrush );
	a.setPoints( QCOORDARRLEN(top_pts), top_pts );
	a.move( x, y );
	p->drawPolyline( a );			// draw top part
	p->pen().setColor( showUp ? g.dark() : g.light() );
	a.setPoints( QCOORDARRLEN(bottom_pts), bottom_pts );
	a.move( x, y );
	p->drawPolyline( a );			// draw bottom part
    }

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( wx, wy, *pm );
	w += wx;
	QPixmapCache::insert( pmkey, pm );	// save for later use
    }
#endif
    if ( text() ) {
	QFontMetrics fm = fontMetrics();
	p->pen().setColor( g.text() );
	p->drawText( w+6-wless, sz.height()/2+fm.height()/2-fm.descent(),
		     text() );
    }
}


void QRadioButton::mouseReleaseEvent( QMouseEvent *e )
{
    if ( isOn() )				// cannot switch off
	noHit = TRUE;
    QButton::mouseReleaseEvent( e );		// send to button handler
    noHit = FALSE;
}

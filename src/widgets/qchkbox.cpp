/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qchkbox.cpp#34 $
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
#include "qpmcache.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qchkbox.cpp#34 $";
#endif


/*----------------------------------------------------------------------------
  \class QCheckBox qchkbox.h
  \brief The QCheckBox widget provides a check box with a text label.

  \ingroup realwidgets

  QCheckBox and QRadioButton are both toggle buttons, but a check box
  represents an independent switch that can be on (checked) or off
  (unchecked).
 ----------------------------------------------------------------------------*/


static void getSizeOfBitmap( int gs, int *w, int *h )
{
    switch ( gs ) {				// calculate coords
	case MacStyle:
	case WindowsStyle:
	case Win3Style:
	    *w = *h = 13;
	    break;
	case PMStyle:
	    *w = *h = 16;
	    break;
	case MotifStyle:
	    *w = *h = 10;
	    break;
	default:
	    *w = *h = 10;
    }
}


/*----------------------------------------------------------------------------
  Constructs a check box with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QCheckBox::QCheckBox( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setToggleButton( TRUE );
}

/*----------------------------------------------------------------------------
  Constructs a check box with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QCheckBox::QCheckBox( const char *text, QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setText( text );
    setToggleButton( TRUE );
}


/*----------------------------------------------------------------------------
  \fn bool QCheckBox::isChecked() const
  Returns TRUE if the check box is checked, or FALSE if it is not checked.
  \sa setChecked()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Checks the check box if \e checked is TRUE, or unchecks it if \e checked
  is FALSE.
  \sa isChecked()
 ----------------------------------------------------------------------------*/

void QCheckBox::setChecked( bool checked )
{
    if ( checked )
	switchOn();
    else
	switchOff();
}



static int extraWidth(int gs) {
    if ( gs == MacStyle || gs == Win3Style )
	return 7;
    else if ( gs == MotifStyle )
	return 8;
    else
	return 6;
}


/*----------------------------------------------------------------------------
  Adjusts the size of the check box to fit the contents.

  This function is called automatically whenever the contents change and
  auto-resizing is enabled.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

void QCheckBox::adjustSize()
{
    QFontMetrics fm = fontMetrics();
    int w = fm.width( text() );
    int h = fm.height();
    int gs = style();
    int wbm, hbm;
    getSizeOfBitmap( style(), &wbm, &hbm );
    if ( h < hbm )
	h = hbm;
    w += wbm+extraWidth( style() );
    if ( w!=width() || h!=height() )
	resize( w, h );
    else
	repaint(TRUE);
}


/*----------------------------------------------------------------------------
  Draws the check box, but not the button label.
  \sa drawButtonLabel()
 ----------------------------------------------------------------------------*/

void QCheckBox::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	 gs = style();
    QColorGroup	 g  = colorGroup();
    int		 x, y, w, h;

    getSizeOfBitmap( gs, &w, &h );
    x = 0;
    y = height()/2 - h/2;

#define SAVE_CHECKBOX_PIXMAPS
#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    pmkey.sprintf( "$qt_check_%d_%d_%d_%d", gs, palette().serialNumber(),
		   isDown(), isOn() );
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( x, y, *pm );
	drawButtonLabel( p );
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

    if ( gs == MacStyle || gs == Win3Style ){	// Mac/Windows 3.x check box
	p->eraseRect( x, y, w, h );
	p->setPen( g.foreground() );
	p->drawRect( x, y, w, h );
	if ( isDown() )				// extra fat rectangle
	    p->drawRect( x+1, y+1, w-2, h-2 );
	if ( isOn() ) {
	    p->drawLine( x, y, x+w-1, y+h-1 );	// draw cross
	    p->drawLine( x, y+h-1, x+w-1, y );
	}
    }
    else if ( gs == WindowsStyle ) {		// Windows check box
	int x1=x, y1=y, x2=x+w-1, y2=y+h-1;
	QPointArray a;
	a.setPoints( 3, x1,y2-1, x1,y1, x2-1,y1 );
	p->setPen( g.dark() );
	p->drawPolyline( a );
	a.setPoints( 3, x1+1,y2-2, x1+1,y1+1, x2-2,y1+1 );
	p->setPen( black );
	p->drawPolyline( a );
	a.setPoints( 3, x1+1,y2-1, x2-1,y2-1, x2-1,y1+1 );
	p->setPen( g.background() );
	p->drawPolyline( a );
	a.setPoints( 3, x1,y2, x2,y2, x2,y1 );
	p->setPen( white );
	p->drawPolyline( a );
	p->fillRect( x1+2, y1+2, x2-x1-3, y2-y1-3,
		     isDown() ? g.background() : g.base() );
	if ( isOn() ) {
	    a.resize( 7*2 );
	    int i, xx, yy;
	    xx = x+3;
	    yy = y+5;
	    for ( i=0; i<3; i++ ) {
		a.setPoint( 2*i,   xx, yy );
		a.setPoint( 2*i+1, xx, yy+2 );
		xx++; yy++;
	    }
	    yy -= 2;
	    for ( i=3; i<7; i++ ) {
		a.setPoint( 2*i,   xx, yy );
		a.setPoint( 2*i+1, xx, yy+2 );
		xx++; yy--;
	    }
	    p->setPen( black );
	    p->drawLineSegments( a );
	}
    }
    else if ( gs == PMStyle ) {			// PM check box
	p->setPen( g.dark() );
	p->setBrush( g.background() );
	p->drawRect( x, y, w, h );
	p->setBrush( NoBrush );
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
	p->setPen( tc );
	p->drawPolyline( atop );
	p->setPen( bc );
	p->drawPolyline( abottom );
	p->setPen( g.background() );
	p->drawPoint( x1, y2 );
	p->drawPoint( x2, y1 );
	static QCOORD check_mark[] = {
	    3,5, 5,5,  4,6, 5,6,  5,7, 6,7,  5,8, 6,8,	6,9, 9,9,
	    6,10, 8,10,	 7,11, 8,11,  7,12, 7,12,  8,8, 9,8,  8,7, 10,7,
	    9,6, 10,6,	9,5, 11,5,  10,4, 11,4,	 10,3, 12,3,
	    11,2, 12,2,	 11,1, 13,1,  12,0, 13,0 };
	static QCOORD check_mark_pix[] = {
	    3,6, 6,6, 4,7, 7,8, 5,9, 6,11, 8,12, 9,10, 10,8, 8,6,
	    11,6, 9,4, 12,4, 10,2, 13,2 };
	if ( isOn() ) {				// draw complex check mark
	    x1 = x;
	    y1 = y;
	    if ( isDown() ) {			// shift check mark
		x1++;
		y1++;
	    }
	    QPointArray amark( sizeof(check_mark)/(sizeof(QCOORD)*2),
			       check_mark );
	    amark.move( x1, y1 );
	    p->setPen( g.foreground() );
	    p->drawLineSegments( amark );
	    p->setPen( g.dark() );
	    for ( int i=0; i<(int)(sizeof(check_mark_pix)/sizeof(QCOORD));
			   i+=2 )
		p->drawPoint( x1 + check_mark_pix[i],
			      y1 + check_mark_pix[i+1] );
	}
    }
    else if ( gs == MotifStyle ) {		// Motif check box
	QColor tColor, bColor, fColor;
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
	p->drawPixmap( wx, wy, *pm );
	w += wx;
	QPixmapCache::insert( pmkey, pm );	// save for later use
    }
#endif
    drawButtonLabel( p );
}


/*----------------------------------------------------------------------------
  Draws the check box label.
  \sa drawButton()
 ----------------------------------------------------------------------------*/

void QCheckBox::drawButtonLabel( QPainter *p )
{
    int x, y, w, h;
    int gs = style();
    getSizeOfBitmap( gs, &w, &h );
    y = 0;
    x = w + extraWidth( gs );
    w = width() - x;
    h = height();

    p->setPen( colorGroup().text() );

    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	if ( pm->depth() == 1 )
	    p->setBackgroundMode( OpaqueMode );
	y += h/2 - pm->height()/2;
	p->drawPixmap( x, y, *pm );
    }
    else if ( text() )
	p->drawText( x, y, w, h, AlignLeft|AlignVCenter|ShowPrefix, text() );
}

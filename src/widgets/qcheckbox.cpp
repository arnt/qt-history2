/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.cpp#51 $
**
** Implementation of QCheckBox class
**
** Created : 940222
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qchkbox.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qpixmap.h"
#include "qpmcache.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qcheckbox.cpp#51 $");


/*!
  \class QCheckBox qchkbox.h
  \brief The QCheckBox widget provides a check box with a text label.

  \ingroup realwidgets

  QCheckBox and QRadioButton are both toggle buttons, but a check box
  represents an independent switch that can be on (checked) or off
  (unchecked).
*/


static void getSizeOfBitmap( int gs, int *w, int *h )
{
    switch ( gs ) {				// calculate coords
	case WindowsStyle:
	    *w = *h = 13;
	    break;
	case MotifStyle:
	    *w = *h = 10;
	    break;
	default:
	    *w = *h = 10;
    }
}


/*!
  Constructs a check box with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setToggleButton( TRUE );
}

/*!
  Constructs a check box with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( const char *text, QWidget *parent, const char *name )
	: QButton( parent, name )
{
    initMetaObject();
    setText( text );
    setToggleButton( TRUE );
}


/*!
  \fn bool QCheckBox::isChecked() const
  Returns TRUE if the check box is checked, or FALSE if it is not checked.
  \sa setChecked()
*/

/*!
  \fn void QCheckBox::setChecked( bool check )
  Checks the check box if \e check is TRUE, or unchecks it if \e check
  is FALSE.
  \sa isChecked()
*/


static int extraWidth( int gs )
{
    if ( gs == MotifStyle )
	return 8;
    else
	return 6;
}


/*!
  Returns a size which fits the contents of the check box.
*/

QSize QCheckBox::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    int w = fm.width( text() );
    int h = fm.height();
    int gs = style();
    int wbm, hbm;
    getSizeOfBitmap( gs, &wbm, &hbm );
    if ( h < hbm )
	h = hbm;
    w += wbm+extraWidth( gs );

    return QSize( w, h );
}


/*!
  Draws the check box, but not the button label.
  \sa drawButtonLabel()
*/

void QCheckBox::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	 gs = style();
    QColorGroup	 g  = colorGroup();
    int		 x, y, w, h;

    getSizeOfBitmap( gs, &w, &h );
    x = gs == MotifStyle ? 1 : 0;
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

    if ( gs == WindowsStyle ) {			// Windows check box
	QColor fillColor;
	if ( isDown() )
	    fillColor = g.background();
	else
	    fillColor = g.base();
	QBrush fill( fillColor );
	qDrawWinPanel( p, x, y, w, h, g, TRUE, &fill );
	if ( isOn() ) {
	    QPointArray a( 7*2 );
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
    if ( gs == MotifStyle ) {			// Motif check box
	bool showUp = !(isDown() ^ isOn());
	QBrush fill( showUp ? g.background() : g.mid() );
	qDrawShadePanel( p, x, y, w, h, g, !showUp, 2, &fill );
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


/*!
  Draws the check box label.
  \sa drawButton()
*/

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

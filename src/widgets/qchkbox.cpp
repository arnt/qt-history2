/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qchkbox.cpp#65 $
**
** Implementation of QCheckBox class
**
** Created : 940222
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qchkbox.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qpixmap.h"
#include "qpmcache.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qchkbox.cpp#65 $");


/*!
  \class QCheckBox qchkbox.h
  \brief The QCheckBox widget provides a check box with a text label.

  \ingroup realwidgets

  QCheckBox and QRadioButton are both toggle buttons, but a check box
  represents an independent switch that can be on (checked) or off
  (unchecked).

  <img src=qchkbox-m.gif> <img src=qchkbox-w.gif>
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

    return QSize( w+4, h+4 );
}


/*!
  Draws the check box.  Calls drawButtonLabel() to draw the content.
  \sa drawButtonLabel()
*/

void QCheckBox::drawButton( QPainter *paint )
{
    QPainter	*p = paint;
    GUIStyle	 gs = style();
    QColorGroup	 g  = colorGroup();
    int		 x, y, w, h;

    getSizeOfBitmap( gs, &w, &h );
    x = gs == MotifStyle ? 1 : 0;
    y = height()/2 - h/2;

#define SAVE_CHECKBOX_PIXMAPS
#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isOn() )
	kf |= 2;
    if ( isEnabled() )
	kf |= 4;
    pmkey.sprintf( "$qt_check_%d_%d_%d", gs, palette().serialNumber(), kf );
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
	if (!QPixmapCache::insert(pmkey, pm) )	// save in cache
	    delete pm;
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
    GUIStyle gs = style();
    getSizeOfBitmap( gs, &w, &h );
    y = 0;
    x = w + extraWidth( gs );
    w = width() - x;
    h = height();

    qDrawItem( p, gs, x, y, w, h, 
	       AlignLeft|AlignVCenter|ShowPrefix,
	       colorGroup(), isEnabled(),
	       pixmap(), text() );

    if ( hasFocus() ) {
	QRect br = qItemRect( p, gs, x, y, w, h, 
			      AlignLeft|AlignVCenter|ShowPrefix,
			      isEnabled(),
			      pixmap(), text() );
	br.setLeft( br.left()-3 );
	br.setRight( br.right()+2 );
	br.setTop( br.top()-2 );
	br.setBottom( br.bottom()+2);
	br = br.intersect( QRect(0,0,width(),height()) );

	if ( gs == WindowsStyle ) {
	    p->drawWinFocusRect( br, backgroundColor() );
	} else {
	    p->setPen( black );
	    p->drawRect( br );
	}
    }
}

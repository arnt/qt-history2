/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.cpp#61 $
**
** Implementation of QPushButton class
**
** Author  : Haavard Nord
** Created : 940221
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpushbt.h"
#include "qdialog.h"
#include "qfontmet.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qpixmap.h"
#include "qpmcache.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qpushbutton.cpp#61 $")


/*----------------------------------------------------------------------------
  \class QPushButton qpushbt.h
  \brief The QPushButton widget provides a push button with a text label.

  \ingroup realwidgets

  A default push button in a dialog emits the clicked signal if the user
  presses the Enter key.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Constructs a push button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*----------------------------------------------------------------------------
  Constructs a push button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QPushButton::QPushButton( const char *text, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}

void QPushButton::init()
{
    initMetaObject();
    autoDefButton = defButton = lastDown = lastDef = FALSE;
}


/*----------------------------------------------------------------------------
  Makes the push button a toggle button if \e enable is TRUE, or a normal
  push button if \e enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A push button is initially not a toggle button.

  \sa setOn(), toggle(), toggleButton() toggled()
  ----------------------------------------------------------------------------*/

void QPushButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*----------------------------------------------------------------------------
  Switches a toggle button on if \e enable is TRUE or off if \e enable is
  FALSE.
  \sa isOn(), toggle(), toggled(), toggleButton()
 ----------------------------------------------------------------------------*/

void QPushButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*----------------------------------------------------------------------------
  Toggles the state of a toggle button.
  \sa isOn(), setOn(), toggled(), toggleButton()
 ----------------------------------------------------------------------------*/

void QPushButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( isOn() );
}


/*----------------------------------------------------------------------------
  \fn bool QPushButton::autoDefault() const
  Returns TRUE if the button is an auto-default button.

  \sa setAutoDefault()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the push buttons to an auto-default button if \e enable is TRUE,
  or to a normal button if \e enable is FALSE.

  An auto-default button becomes the default push button automatically
  when it receives the keyboard input focus.

  \sa autoDefault(), setDefault()
 ----------------------------------------------------------------------------*/

void QPushButton::setAutoDefault( bool enable )
{
    autoDefButton = enable;
}


/*----------------------------------------------------------------------------
  \fn bool QPushButton::isDefault() const
  Returns TRUE if the button is default.

  \sa setDefault()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the button to be the default button if \e enable is TRUE, or
  to be a normal button if \e enable is FALSE.

  A default push button in a \link QDialog dialog\endlink emits the
  QButton::clicked() signal if the user presses the Enter key.	Only
  one push button in the dialog can be default.

  Default push buttons are only allowed in dialogs.

  \sa default(), setAutoDefault(), QDialog
 ----------------------------------------------------------------------------*/

void QPushButton::setDefault( bool enable )
{
    if ( (defButton && enable) || !(defButton || enable) )
	return;					// no change
    QWidget *p = this;
    while ( p && p->parentWidget() )		// get the top level parent
	p = p->parentWidget();
    if ( !p->inherits("QDialog") )		// not a dialog
	return;
    defButton = enable;
    if ( defButton )
	((QDialog*)p)->setDefault( this );
    if ( isVisible() )
	repaint( FALSE );
}


/*----------------------------------------------------------------------------
  Adjusts the size of the push button to fit the contents.

  This function is called automatically whenever the contents change and
  auto-resizing is enabled.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

void QPushButton::adjustSize()
{
    int w, h;
    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w = pm->width()	 + 6;
	h = pm->height() + 6;
    }
    else {
	QFontMetrics fm = fontMetrics();
	QRect br = fm.boundingRect( text() );
	w = br.width()	+ 6;
	h = br.height() + 6;
	w += w/8 + 16;
	h += h/8 + 8;
    }
    if ( w!=width() || h!=height() )
	resize( w, h );
    else
	repaint(TRUE);
}


/*----------------------------------------------------------------------------
  Draws the push button, except its label.
  \sa drawButtonLabel()
 ----------------------------------------------------------------------------*/

void QPushButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	gs = style();
    QColorGroup g  = colorGroup();
    bool	updated = isDown() != (bool)lastDown || lastDef != defButton;
    QColor	fillcol = g.background();
    int		x1, y1, x2, y2;

    rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

#define SAVE_PUSHBUTTON_PIXMAPS
#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int w, h;
    w = x2 + 1;
    h = y2 + 1;
    pmkey.sprintf( "$qt_push_%d_%d_%d_%d_%d_%d_%d", gs,
		   palette().serialNumber(), isDown(), defButton, w, h,
		   isToggleButton() && isOn() );
    QPixmap *pm = QPixmapCache::find( pmkey );
    QPainter pmpaint;
    if ( pm ) {					// pixmap exists
	QPixmap pm_direct = *pm;
	pmpaint.begin( &pm_direct );
	pmpaint.drawPixmap( 0, 0, *pm );
	if ( text() )
	    pmpaint.setFont( font() );
	drawButtonLabel( &pmpaint );
	pmpaint.end();
	p->drawPixmap( 0, 0, pm_direct );
	lastDown = isDown();
	lastDef = defButton;
	return;
    }
    bool use_pm = TRUE;
    if ( use_pm ) {
	pm = new QPixmap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	p->setBackgroundColor( fillcol );
	p->eraseRect( 0, 0, w, h );
    }
#endif

    p->setPen( g.foreground() );
    p->setBrush( QBrush(fillcol,NoBrush) );

    if ( gs == WindowsStyle ) {		// Windows push button
	if ( isDown() ) {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
		p->setPen( g.dark() );
		p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	    }
	    else
		qDrawWinButton( p, x1, y1, w, h, g, TRUE );
	} else {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, w, h );
		x1++; y1++;
		x2--; y2--;
	    }
	    if ( isToggleButton() && isOn() ) {
		qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE );
		if ( updated ) {
		    p->setPen( NoPen );
		    p->setBrush( g.mid() );
		    p->drawRect( x1+1, y1+1, x2-x1-2, y2-y1-2 );
		    updated = FALSE;
		}
	    } else {
		qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, FALSE );
	    }
	}
	if ( updated )
	    p->fillRect( x1+1, y1+1, x2-x1-1, y2-y1-1, g.background() );
    }
    else if ( gs == MotifStyle ) {		// Motif push button
	if ( defButton ) {			// default Motif button
	    p->setPen( black );
	    p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
	    p->setPen( g.dark() );
	    p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	    x1 += 1;
	    y1 += 1;
	    x2 -= 1;
	    y2 -= 1;
	}
	QBrush fill( fillcol );
	if ( isDown() ) {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, 2,
			     updated ? &fill : 0 );
	} else if ( isToggleButton() && isOn() ) {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, 2, 0 );
	    if ( updated ) {
		p->setPen( NoPen );
		p->setBrush( g.mid() );
		p->drawRect( x1+2, y1+2, x2-x1-3, y2-y1-3 );
	    }
	} else {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, FALSE, 2,
			     updated ? &fill : 0 );
	}
    }
    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );

#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( 0, 0, *pm );
	QPixmapCache::insert( pmkey, pm );	// save for later use
    }
#endif
    drawButtonLabel( p );
    lastDown = isDown();
    lastDef = defButton;
}


/*----------------------------------------------------------------------------
  Draws the push button label.
  \sa drawButton()
 ----------------------------------------------------------------------------*/

void QPushButton::drawButtonLabel( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	gs = style();
    QColorGroup g  = colorGroup();
    int		dt = 0;
    switch ( gs ) {
	case WindowsStyle:
	    dt = 1;
	    break;
	case MotifStyle:
	    p->setPen( g.text() );
	    break;
	default:
	    ;
    }
    QRect r = rect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( isDown() || isOn() ) {			// shift pixmap/text
	x += dt;
	y += dt;
    }
    x += 2;  y += 2;  w -= 4;  h -= 4;
    if ( pixmap() ) {
	const QPixmap *pm = pixmap();
	if ( pm->width() > w || pm->height() > h )
	    p->setClipRect( x, y, w, h );
	if ( gs == WindowsStyle && !isDown() && isOn() ) {
	     if ( pm->depth() == 1 )
		 p->setBackgroundColor( g.background().light() );
	} else {
	    p->setBackgroundColor( g.background() );
	}
	x += w/2 - pm->width()/2;		// center
	y += h/2 - pm->height()/2;
	p->drawPixmap( x, y, *pm );
	p->setClipping( FALSE );
    }
    else if ( text() )
	p->drawText( x, y, w, h, AlignCenter|ShowPrefix, text() );
}

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.cpp#31 $
**
** Implementation of QButton widget class
**
** Author  : Haavard Nord
** Created : 940206
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qbutton.h"
#include "qbttngrp.h"
#include "qpixmap.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbutton.cpp#31 $";
#endif


/*----------------------------------------------------------------------------
  \class QButton qbutton.h

  \brief The QButton class is abstract the base class of button
  widgets, and it provides functionality common to buttons.

  \ingroup abstractwidgets

  The QButton class implements an abstract button, and lets subclasses specify
  how to reply to user actions and how to draw the button.

  The QButton class has three signals. The pressed() signal is emitted
  when the left mouse button is pressed while the cursor is inside the
  button. After being pressed, the button will be down until the left
  mouse button is again released, which causes a released() signal. If the
  left mouse button is released when the cursor is inside the button, the
  clicked() signal will be emitted.

  There are two types of buttons; standard buttons and toggle buttons. A
  standard button can either be pressed down or released. The QPushButton
  class is an example of a standard button. A toggle button has an
  additional flag that is toggled each time the button is clicked. The
  QRadioButton and QCheckBox classes are examples of toggle buttons.

  The button label can be a \link setText() text\endlink or a \link
  setPixmap() pixmap\endlink.  It is up to widget implementation to
  display the text or the pixmap.  All Qt buttons are capable of showing
  texts and pixmaps.

  Another convention thing about QButtons is \link setAutoResizing()
  auto-resizing\endlink.  Enabling
  auto-resizing will make a label resize itself whenever the contents
  change.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Constructs a standard button with a parent widget and a name.
 ----------------------------------------------------------------------------*/

QButton::QButton( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    bpixmap    = 0;
    toggleBt   = FALSE;			// button is not on/off
    buttonDown = FALSE;			// button is up
    buttonOn   = FALSE;			// button is off
    mlbDown    = FALSE;			// mouse left button up
    autoResize = FALSE;
    if ( parent && parent->inherits("QButtonGroup") ) {
	group = (QButtonGroup*)parent;
	group->insert( this );			// insert into button group
    }
    else
	group = 0;
}

/*----------------------------------------------------------------------------
  Destroys the button and all its child widgets.
 ----------------------------------------------------------------------------*/

QButton::~QButton()
{
    if ( group )				// remove from button group
	group->remove( this );
    delete bpixmap;
}


/*----------------------------------------------------------------------------
  \fn void QButton::pressed()
  This signal is emitted when the button is pressed down.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QButton::released()
  This signal is emitted when the button is released.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QButton::clicked()
  This signal is emitted when the button is activated (i.e. first pressed down
  and then released when the mouse cursor is inside the button).
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn bool QButton::isDown() const
  Returns TRUE of the button is pressed down, the opposite of isUp().
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QButton::isUp() const
  Returns TRUE of the button is up, the opposite of isDown().
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const char *QButton::text() const
  Returns the button text.
  \sa setText()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the button contents to \e text and redraws the contents.

  The button resizes itself if auto-resizing is enabled.

  \sa text(), setPixmap(), setAutoResize().
 ----------------------------------------------------------------------------*/

void QButton::setText( const char *text )
{
    if ( btext == text )
	return;
    btext = text;
    if ( bpixmap ) {
	delete bpixmap;
	bpixmap = 0;
    }
    if ( autoResize )
	adjustSize();
    else
	update();
}


/*----------------------------------------------------------------------------
  \fn const QPixmap *QButton::pixmap() const

  Returns the button pixmap, or 0 if the button isn't showing a pixmap.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the button pixmap to \e pixmap and redraws the contents.

  The button resizes itself if auto-resizing is enabled.

  \sa pixmap(), setText(), setAutoResize()
 ----------------------------------------------------------------------------*/

void QButton::setPixmap( const QPixmap &pixmap )
{
    int w, h;
    if ( bpixmap ) {
	w = bpixmap->width();
	h = bpixmap->height();
    }
    else {
	bpixmap = new QPixmap;
	w = h = -1;
    }
    *bpixmap = pixmap;
    if ( !btext.isNull() )
	btext.resize( 0 );
    if ( autoResize &&  (w != bpixmap->width() || h != bpixmap->height()) )
	adjustSize();
    else {
	if ( w >= 0 && w <= bpixmap->width() && h <= bpixmap->height() ) {
	    QPainter paint;
	    paint.begin( this );
	    drawButtonLabel( &paint );
	    paint.end();
	}
	else
	    update();
    }
}


/*----------------------------------------------------------------------------
  \fn bool QButton::autoResizing() const
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResizing()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
  FALSE.

  When auto-resizing is enabled, the button will resize itself whenever
  the contents change.

  \sa autoResizing(), adjustSize()
 ----------------------------------------------------------------------------*/

void QButton::setAutoResizing( bool enable )
{
    if ( (bool)autoResize != enable ) {
	autoResize = enable;
	if ( autoResize )
	    adjustSize();			// calls resize which repaints
    }
}


/*----------------------------------------------------------------------------
  \fn bool QButton::isOn() const
  Returns TRUE if this toggle button has been switched ON, or FALSE
  if it has been switched OFF.
  \sa switchOn(), switchOff(), isOn()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Swithes a toggle button ON.  This function should be called only for
  toggle buttons.
  \sa switchOff(), isOn()
 ----------------------------------------------------------------------------*/

void QButton::switchOn()			// switch button on
{
#if defined(CHECK_STATE)
    if ( !toggleBt )
	warning( "QButton::switchOn: Only on/off buttons should be switched" );
#endif
    bool lastOn = buttonOn;
    buttonOn = TRUE;
    if ( !lastOn )				// changed state
	repaint( FALSE );			// redraw
}

/*----------------------------------------------------------------------------
  Swithes a toggle button OFF.  This function should be called only for
  toggle buttons.
  \sa switchOn(), isOn()
 ----------------------------------------------------------------------------*/

void QButton::switchOff()			// switch button off
{
#if defined(CHECK_STATE)
    if ( !toggleBt )
	warning( "QButton::switchOff: Only on/off buttons should be switched");
#endif
    bool lastOn = buttonOn;
    buttonOn = FALSE;
    if ( lastOn )				// changed state
	repaint( FALSE );			// redraw
}


/*----------------------------------------------------------------------------
  \fn bool QButton::toggleButton() const
  Returns TRUE if the button is a toggle button.
  \sa setToggleButton()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Sets the button to become a toggle button if \e toggle is TRUE, or a
  standard button if \e toggle is FALSE.

  A button is initially a standard button.

  \sa toggleButton()
 ----------------------------------------------------------------------------*/

void QButton::setToggleButton( bool toggle )	// set to toggle button
{
    toggleBt = toggle;
}


/*----------------------------------------------------------------------------
  Returns TRUE if \e pos is inside the widget rectangle, or FALSE if it
  is outside.

  This virtual function is reimplemented by subclasses.
 ----------------------------------------------------------------------------*/

bool QButton::hitButton( const QPoint &pos ) const
{
    return rect().contains( pos );
}

/*----------------------------------------------------------------------------
  Draws the button.  The default implementation does nothing.

  This virtual function is reimplemented by subclasses to draw real buttons.
 ----------------------------------------------------------------------------*/

void QButton::drawButton( QPainter * )
{
    return;
}

/*----------------------------------------------------------------------------
  Draws the button text or pixmap.  The default implementation does nothing.

  This virtual function is reimplemented by subclasses to draw real buttons.
 ----------------------------------------------------------------------------*/

void QButton::drawButtonLabel( QPainter * )
{
    return;
}


/*----------------------------------------------------------------------------
  Handles mouse press events for the button. \sa mouseReleaseEvent()
 ----------------------------------------------------------------------------*/

void QButton::mousePressEvent( QMouseEvent *e ) // mouse press
{
    if ( e->button() != LeftButton || mlbDown )
	return;
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse press on button
	mlbDown = TRUE;				// left mouse button down
	buttonDown = TRUE;
	repaint( FALSE );
	emit pressed();
    }
}

/*----------------------------------------------------------------------------
  Handles mouse release events for the button. \sa mousePressEvent()
 ----------------------------------------------------------------------------*/

void QButton::mouseReleaseEvent( QMouseEvent *e)// mouse release
{
    if ( e->button() != LeftButton || !mlbDown )
	return;
    mlbDown = FALSE;				// left mouse button up
    bool hit = hitButton( e->pos() );
    buttonDown = FALSE;
    if ( hit ) {				// mouse release on button
	if ( toggleBt )
	    buttonOn = !buttonOn;
	repaint( FALSE );
	emit released();
	emit clicked();
    }
    else {
	repaint( FALSE );
	emit released();
    }
}

/*----------------------------------------------------------------------------
  Handles mouse move events for the button.
 ----------------------------------------------------------------------------*/

void QButton::mouseMoveEvent( QMouseEvent *e )	// mouse move event
{
    if ( !((e->state() & LeftButton) && mlbDown) )
	return;					// left mouse button is up
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse move in button
	if ( !buttonDown ) {
	    buttonDown = TRUE;
	    repaint( FALSE );
	    emit pressed();
	}
    }
    else {					// mouse move outside button
	if ( buttonDown ) {
	    buttonDown = FALSE;
	    repaint( FALSE );
	    emit released();
	}
    }
}

/*----------------------------------------------------------------------------
  Handles paint events for the button.

  Opens the painter on the button and calls drawButton().
 ----------------------------------------------------------------------------*/

void QButton::paintEvent( QPaintEvent * )
{
    QPainter paint;
    paint.begin( this );
    drawButton( &paint );			// ask subclass to draw button
    paint.end();
}


/*----------------------------------------------------------------------------
  Handles focus in events for the button.

  \sa focusOutEvent()
 ----------------------------------------------------------------------------*/

void QButton::focusInEvent( QFocusEvent * )
{
    repaint( FALSE );
}

/*----------------------------------------------------------------------------
  Handles focus out events for the button.
  \sa focusInEvent()
 ----------------------------------------------------------------------------*/

void QButton::focusOutEvent( QFocusEvent * )
{
    repaint( FALSE );
}

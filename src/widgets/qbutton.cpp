/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.cpp#22 $
**
** Implementation of QButton widget class
**
** Author  : Haavard Nord
** Created : 940206
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 QPixmapDict QDictM_QPixmap
#include "qbutton.h"
#include "qbttngrp.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbutton.cpp#22 $";
#endif


/*!
\class QButton qbutton.h
\brief The QButton class is the base class of button widgets, and it
provides functionality common to buttons.

The QButton class implements an abstract button, and lets subclasses specify
how to reply to user action and how to draw itself.

The QButton class has three signals. The pressed() signal is emitted
when the left mouse button is pressed when the cursor is inside the
button. After being pressed, the button will be down until the left mouse
button is again released, which causes a released() signal. If the
left mouse button is released when the cursor is inside the button, the
clicked() signal will be emitted.

There are two types of buttons; standard buttons and toggle buttons. A
standard button can either be pressed down or released. The QPushButton
class is an example of a standard button. A toggle button has an additional
flag that is toggled each time the button is clicked. The QRadioButton
and QCheckBox classes are examples of toggle buttons.

Enabling auto-resizing will make a label resize itself whenever
the contents change.
*/


/*!
Constructs a standard button with a parent widget and a name.
*/

QButton::QButton( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
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

/*!
Destroys the button and all its child widgets.
*/

QButton::~QButton()
{
    if ( group )				// remove from button group
	group->remove( this );
}


/*!
\fn void QButton::pressed()
This signal is emitted when the button is pressed down.
*/

/*!
\fn void QButton::released()
This signal is emitted when the button is released.
*/

/*!
\fn void QButton::clicked()
This signal is emitted when the button is activated (i.e. first pressed down
and then released when the mouse cursor is inside the button).
*/


/*!
\fn bool QButton::isDown() const
Returns TRUE of the button is pressed down, the opposite of isUp().
*/

/*!
\fn bool QButton::isUp() const
Returns TRUE of the button is standing up, the opposite of isDown().
*/

/*!
\fn const char *QButton::text() const
Returns the button text.
*/

/*!
Sets the button text to \e text and redraws the contents.

The button resizes itself if auto-resizing is enabled.

\sa setAutoResize().
*/

void QButton::setText( const char *text )
{						// set button label
    if ( btext == text )
	return;
    btext = text;
    if ( autoResize )
	adjustSize();
    else
	update();
}


/*!
\fn bool QButton::autoResizing() const
Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
disabled.

Auto-resizing is disabled by default.

\sa setAutoResizing().
*/


/*!
Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
FALSE.

When auto-resizing is enabled, the button will resize itself whenever
the contents change.

\sa autoResizing() and adjustSize().
*/

void QButton::setAutoResizing( bool enable )
{
    if ( autoResize != enable ) {
	autoResize = enable;
	if ( autoResize )
	    adjustSize();			// calls resize which repaints
    }
}


/*!
Virtual function that adjusts the size of the button to fit the contents.

This function is called automatically whenever the contents change and
auto-resizing is enabled.

\sa setAutoResizing()
*/

void QButton::adjustSize()
{
}


/*!
\fn bool QButton::isOn() const
Returns TRUE if this toggle button has been switched ON, or FALSE
if it has been switched OFF.

\sa switchOn(), switchOff(), isOn().
*/

/*!
Swithes a toggle button ON.  This function should be called only for
toggle buttons.

\sa switchOff(), isOn().
*/

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

/*!
Swithes a toggle button OFF.  This function should be called only for
toggle buttons.

\sa switchOn(), isOn().
*/

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


/*!
\fn bool QButton::toggleButton() const
Returns TRUE if the button is a toggle button.

\sa setToggleButton().
*/


/*!
Sets the button to become a toggle button if \e toggle is TRUE, or a
standard button if \e toggle is FALSE.

A button is initially a standard button.

\sa toggleButton().
*/

void QButton::setToggleButton( bool toggle )	// set to toggle button
{
    toggleBt = toggle;
}


/*!
Returns TRUE if \e pos is inside the widget rectangle, or FALSE if it
is outside.

This virtual function is reimplemented by subclasses.
*/

bool QButton::hitButton( const QPoint &pos ) const
{
    return rect().contains( pos );
}

/*!
Draws the button.  The default implementation does nothing.

This virtual function is reimplemented by subclasses to draw real buttons.
*/

void QButton::drawButton( QPainter * )
{
    return;
}


/*!
Internal handling of mouse press events.
*/

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

/*!
Internal handling of mouse release events.
*/

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

/*!
Internal handling of mouse move events.
*/

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

/*!
Paints the button.

Opens the painter on the button and calls drawButton().
*/

void QButton::paintEvent( QPaintEvent * )
{
    QPainter paint;
    paint.begin( this );
    drawButton( &paint );			// ask subclass to draw button
    paint.end();
}


void QButton::focusInEvent( QFocusEvent * )
{
    repaint( FALSE );
}

void QButton::focusOutEvent( QFocusEvent * )
{
    repaint( FALSE );
}

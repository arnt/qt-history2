/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.cpp#20 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qbutton.cpp#20 $";
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
Returns the button text.
*/

const char *QButton::label() const		// get button label
{
    return btext;
}

/*!
Sets the button text. If \e resize is TRUE, then the button will resize
itself automatically to the size of the label.
*/

void QButton::setLabel( const char *label, bool resize )
{						// set button label
    btext = label;
    if ( resize )
	resizeFitLabel();
    update();
}


/*!
Resizes the button to fit the label.

This virtual function is reimplemented by subclasses.
*/

void QButton::resizeFitLabel()			// do nothing
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
Draws the buttons.  The default implementation does nothing.

This virtual function is reimplemented by subclasses to draw real buttons.
*/

void QButton::drawButton( QPainter * )
{
    return;
}


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

void QButton::paintEvent( QPaintEvent * )
{
    QPainter paint;
    paint.begin( this );
    drawButton( &paint );			// ask subclass to draw button
    paint.end();
}


void QButton::focusChangeEvent( QFocusEvent * )
{
    repaint( FALSE );
}

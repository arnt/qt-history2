/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.cpp#5 $
**
** Implementation of event classes
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qevent.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qevent.cpp#5 $";
#endif


void qRemovePostedEvent( QEvent * );		// defined in qapp_xxx.cpp


void QEvent::peErrMsg()				// posted event error message
{
#if defined(CHECK_STATE)
    const char *n = 0;
    switch ( t ) {				// convert type to msg string
	case Event_Timer:
	    n = "Timer";
	    break;
	case Event_MouseButtonPress:
	    n = "MouseButtonPress";
	    break;
	case Event_MouseButtonRelease:
	    n = "MouseButtonRelease";
	    break;
	case Event_MouseButtonDblClick:
	    n = "MouseButtonDblClick";
	    break;
	case Event_MouseMove:
	    n = "MouseMove";
	    break;
	case Event_KeyPress:
	    n = "KeyPress";
	    break;
	case Event_KeyRelease:
	    n = "KeyRelease";
	    break;
	case Event_FocusIn:
	    n = "FocusIn";
	    break;
	case Event_FocusOut:
	    n = "FocusOut";
	    break;
	case Event_Enter:
	    n = "Enter";
	    break;
	case Event_Leave:
	    n = "Leave";
	    break;
	case Event_Paint:
	    n = "Paint";
	    break;
	case Event_Move:
	    n = "Move";
	    break;
	case Event_Resize:
	    n = "Resize";
	    break;
	case Event_Create:
	    n = "Create";
	    break;
	case Event_Destroy:
	    n = "Destroy";
	    break;
	case Event_Show:
	    n = "Show";
	    break;
	case Event_Hide:
	    n = "Hide";
	    break;
	case Event_Close:
	    n = "Close";
	    break;
	case Event_Quit:
	    n = "Quit";
	    break;
    }
    if ( n )
	warning( "QEvent: Posted event %s cannot be stack variable, ignored",
		 n );
    else
	warning( "QEvent: Posted event %d cannot be stack variable, ignored",
		 t );
#endif
    qRemovePostedEvent( this );
}


/*!
\class QEvent qevent.h
\brief The QEvent class is base class of all event classes. Event object
contain event parameters.

The main event loop of Qt fetches window system events from the event
queue, translates the events to Qt events and sends them to the widgets.
Most events come from the underlying window system, but it is also
possible to manually send events through the QApplication class.

Only classes that inherit QObject and reimplement the virtual QObject::event()
function can receive events.

QWidget reimplements this function to examine the event type and send the
event to the appropriate virtual event handler; QWidget::keyPressEvent(),
QWidget::mouseMoveEvent() etc.

The basic QEvent contains only the event type parameter. Subclasses of
QEvent contain parameters that descripe the particular event; QKeyEvent,
QMouseEvent etc.
*/

/*!
\fn QEvent::QEvent( int type )
Contructs a base event with a \e type. The file qevent.h has a list of
all event types.
*/

/*!
\fn int QEvent::type() const
Returns the event type.
*/


/*!
\class QTimerEvent qevent.h
\brief The QTimerEvent class contains the timer identifier parameter of a
timer event.

\sa QObject::startTimer() and QObject::killTimer().
*/

/*!
\fn QTimerEvent::QTimerEvent( int timerId )
Constructs a timer event object with a timer identifier.
*/

/*!
\fn int QTimer::timerId() const
Returns the timer identifier, which is the same identifier returned from
QObject::startTimer().
*/


/*!
\class QMouseEvent qevent.h
\brief The QMouseEvent class contains parameters that describe the mouse event.

Widgets receive mouse events by the virtual functions
QWidget::mousePressEvent(), QWidget::mouseReleaseEvent(),
QWidget::mouseDoubleClickEvent() and QWidget::mouseMoveEvent().
*/

/*!
\fn QMouseEvent::QMouseEvent( int type, const QPoint &pos, int button, int state )
Constructs a mouse event object with parameters.

Arguments:
\arg \e type parameter must be Event_MouseButtonPress,
Event_MouseButtonRelease, Event_MouseButtonDblClick or Event_MouseMove.
\arg \e pos is the mouse position.
\arg \e button is the button that caused the event.
\arg \e state is the current mouse button state.
*/

/*!
\fn QPoint &QMouseEvent::pos()
Returns the position of the mouse relative to the widget that received the
event.
*/

/*!
\fn int QMouseEvent::button() const
Returns the button that caused the event.

Possible return values are LeftButton, RightButton, MidButton and NoButton.
The button value is always NoButton when a mouse move event has been
received.
*/

/*!
\fn int QMouseEvent::state() const
Returns the current buttons state.

The return value is  LeftButton, RightButton, MidButton, ShiftButton,
ControlButton and AltButton OR'ed together.
*/


/*!
\class QKeyEvent qevent.h
\brief The QKeyEvent class contains parameters that describe the key event.

Widgets receive key event by the virtual functions
QWidget::keyPressEvent() and QWidget::keyReleaseEvent().
*/

/*!
\fn QKeyEvent::QKeyEvent( int type, int kc, char ac, int state )
Constructs a key event object with parameters.

Arguments:
\arg \e type parameter must be Event_KeyPress or Event_KeyRelease.
\arg \e kc is the key code
\arg \e ac is the ascii code
\arg \e state is the keyboard status
*/

/*!
\fn int QKeyEvent::key() const
Returns the keyboard code that was pressed or released.

The header file qkeycode.h lists the possible keybord codes.  These codes
are independent of the underlying platform.
*/

/*!
\fn char QKeyEvent::ascii() const
Returns the ASCII code of the key that was pressed or released.
*/

/*!
\fn int QKeyEvent::state() const
Returns the keyboard status flags.

The return value is ShiftButton, ControlButton and AltButton OR'ed together.
*/


/*!
\fn bool QKeyEvent::isAccepted() const
Returns TRUE if the receiver of the event recognized this event and
decided to keep it.
*/

/*!
\fn void QKeyEvent::accept()
Sets the accep flag parameter of the key event object.

Setting the accept parameter indicates that the event receiver wants
the key event. Unwanted key events are sent to the parent widget.
The accept flag is set by default.

\sa ignore().
*/

/*!
\fn void QKeyEvent::ignore()
Clears the accept flag parameter of the key event object.

Clearing the accept parameter indicates that the event receiver does
not want the key event. Unwanted key events are sent to the parent widget.
The accept flag is set by default.

\sa accept().
*/


/*!
\class QFocusEvent qevent.h
\brief The QFocusEvent class contains event parameters for widget focus events.
\todo Has to be implemented.

Widgets receive focus event by the virtual function
QWidget::focusChangeEvent().
*/

/*!
\fn QFocusEvent::QFocusEvent( int type )
Constructs a focus event object with parameters.

The \e type parameter muse be either Event_FocusIn or Event_FocusOut.
*/

/*!
\fn bool QFocusEvent::gotFocus()
Returns TRUE if the widget got the focus.
*/


/*!
\class QPaintEvent qevent.h
\brief The QPaintEvent class contains event parameters for paint events.

Paint events are sent to widgets that needs to update themselves, for instance
when a part of a widget is exposed because an overlying widget is moved away.
This event is received by the virtual function
QWidget::paintEvent().
*/

/*!
\fn QPaintEvent::QPaintEvent( const QRect &paintRect )
Constructs a paint event object with the rectangle that should be updated.
*/

/*!
\fn QRect &QPaintEvent::rect()
Returns the rectangle that should be updated.
*/


/*!
\class QMoveEvent qevent.h
\brief The QMoveEvent class contains event parameters for move events.

Move events are sent to widgets that have been moved to a new position
relative to their parent.  This event is received by the virtual function
QWidget::moveEvent().
*/

/*!
\fn QMoveEvent::QMoveEvent( const QPoint &pos )
Constructs a move event with the new widget position.
*/

/*!
\fn QPoint &QMoveEvent::pos()
Returns the new position of the widget, which is the same as
QWidget::pos().
*/


/*!
\class QResizeEvent qevent.h
\brief The QResizeEvent class contains event parameters for resize events.

Move events are sent to widgets that have been resized.
This event is received by the virtual function
QWidget::resizeEvent().
*/

/*!
\fn QResizeEvent::QResizeEvent( const QSize &size )
Constructs a resize event with the new widget size.
*/

/*!
\fn QSize &QResizeEvent::pos()
Returns the new size of the widget, which is the same as
QWidget::size().
*/


/*!
\class QCloseEvent qevent.h
\brief The QCloseEvent class contains parameters that describe the close event.

Close events are sent to widgets that the user wants to close, for instance
by choosing "close" from the window menu.
Widgets receive close event by the virtual function
QWidget::closeEvent().
*/

/*!
\fn QCloseEvent::QCloseEvent()
Constructs a close event object with the accept parameter set to TRUE.

/*!
\fn bool QCloseEvent::isAccepted() const
Returns TRUE if the receiver of the event has agreed to close the widget.
*/

/*!
\fn void QCloseEvent::accept()
Sets the accep flag parameter of the close event object.

Setting the accept parameter indicates that the event receiver agrees in
closing the widget window.
The accept flag is set by default.

\sa ignore().
*/

/*!
\fn void QCloseEvent::ignore()
Clears the accept flag parameter of the close event object.

Clearing the accept parameter indicates that the event receiver does
not want the widget window to close.
The accept flag is set by default.

\sa accept().
*/

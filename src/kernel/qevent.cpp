/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.cpp#115 $
**
** Implementation of event classes
**
** Created : 931029
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qevent.h"
#include "qcursor.h"
#include "qapplication.h"


/*!
  \class QEvent qevent.h
  \brief The QEvent class is the base class of all
  event classes. Event objects contain event parameters.

  \ingroup environment

  The main event loop of Qt (QApplication::exec()) fetches
  native window system events from the event queue, translates them
  into QEvent and sends the translated events to QObjects.

  Generally, events come from the underlying window system, but it is
  also possible to manually send events through the QApplication class
  using QApplication::sendEvent() and QApplication::postEvent().

  QObject received events by having its QObject::event() function
  called. The function can be reimplemented in subclasses to customize
  event handling and add additional event types. QWidget::event() is
  a notable example. By default, events are dispatched to event handlers
  like QObject::timerEvent() and QWidget::mouseMoveEvent().
  QObject::installEventFilter() allows an object to intercept events
  to another object.

  The basic QEvent contains only an event type parameter.  Subclasses
  of QEvent contain additional parameters that describe the particular
  event.

  \sa QObject::event() QObject::installEventFilter() QWidget::event()
  QApplication::sendEvent() QAppilcation::postEvent()
  QApplication::processEvents()
*/


/*! \enum Qt::ButtonState
  This enum type describes the state of the mouse buttons and the
  modifier buttons.  The currently defined values are:

  \value NoButton  used when the button state does not refer to any
  button (see QMouseEvent::button()).

  \value LeftButton  set if the left button is pressed, or this
  event refers to the left button.  Note that the left button may be
  the right button on left-handed mice.

  \value RightButton  the right button.

  \value MidButton  the middle button

  \value ShiftButton  a shift key on the keyboard is also pressed.

  \value ControlButton  a control key on the keyboard is also pressed.

  \value AltButton  an alt (or meta) key on the keyboard is also pressed.

  \value Keypad  a keypad button is pressed.
*/

/*! \enum QEvent::Type

  This enum type defines the valid event types in Qt.  The currently
  defined event types and the specialized classes for each type are as follow:

  \value None  not an event
  \value Timer  regular timer events, QTimerEvent
  \value MouseButtonPress  mouse press, QMouseEvent
  \value MouseButtonRelease  mouse release, QMouseEvent
  \value MouseButtonDblClick  mouse press again, QMouseEvent
  \value MouseMove  mouse move, QMouseEvent
  \value KeyPress  key press (including shift, for example), QKeyEvent
  \value KeyRelease  key release, QKeyEvent
  \value FocusIn  widget gains keyboard focus, QFocusEvent
  \value FocusOut  widget loses keyboard focus, QFocusEvent
  \value Enter  mouse enters widget's space
  \value Leave  mouse leaves widget's space
  \value Paint  screen update necessary, QPaintEvent
  \value Move  widget's position changed, QMoveEvent
  \value Resize  widget's size changed, QResizeEvent
  \value Show  widget was shown on screen, QShowEvent
  \value Hide  widget was removed from screen, QHideEvent
  \value Close  widget was closed (permanently), QCloseEvent
  \value Accel  key press in child for shortcut key handling, QKeyEvent
  \value Wheel  mouse wheel rolled, QWheelEvent
  \value AccelAvailable  internal event used by Qt on some platforms
  \value AccelOverride  key press in child, for overriding shortcut key handling, QKeyEvent
  \value WindowActivate  the window was activated
  \value WindowDeactivate  the window was deactivated
  \value CaptionChange  widget's caption changed
  \value IconChange  widget's icon changed
  \value ParentFontChange  the font of the parent widget changed
  \value ApplicationFontChange  the default application font changed
  \value ParentPaletteChange  the palette of the parent widget changed
  \value ApplicationPaletteChange  the default application palette changed
  \value Clipboard  system clipboard contents have changed
  \value SockAct  socket activated, used to implement QSocketNotifier
  \value DragEnter  drag-and-drop enters widget, QDragEnterEvent
  \value DragMove  drag-and-drop in progress, QDragMoveEvent
  \value DragLeave  drag-and-drop leaves widget, QDragLeaveEvent
  \value Drop  drag-and-drop is completed, QDropEvent
  \value DragResponse  internal event used by Qt on some platforms
  \value ChildInserted  object gets a child, QChildEvent
  \value ChildRemoved  object loses a child, QChildEvent
  \value LayoutHint  a widget child has changed layout properties
  \value ActivateControl  internal event used by Qt on some platforms
  \value DeactivateControl  internal event used by Qt on some platforms
  \value Quit  reserved
  \value Create  reserved
  \value Destroy  reserved
  \value Reparent  reserved
  \value User  user defined event
*/
/*!
  \fn QEvent::QEvent( Type type )
  Contructs an event object with a \a type.
*/

/*!
  \fn QEvent::Type QEvent::type() const
  Returns the event type.
*/

/*!
  \fn bool QEvent::spontaneous() const

  Returns TRUE if the event originated outside the application,
  i.e. it is a system event.
*/


/*!
  \class QTimerEvent qevent.h
  \brief The QTimerEvent class contains parameters that describe a
  timer event.

  Timer events are sent at regular intervals to objects that have
  started one or more timers.  Each timer has a unique identifier.
  A timer is started with  QObject::startTimer().

  The QTimer class provides a high-level programming interface that
  uses signals instead of events. It also provides one-shot timers.

  The event handler QObject::timerEvent() receives timer events.

  \sa QTimer, QObject::timerEvent(), QObject::startTimer(),
  QObject::killTimer(), QObject::killTimers()
*/

/*!
  \fn QTimerEvent::QTimerEvent( int timerId )
  Constructs a timer event object with the timer identifier set to \a timerId.
*/

/*!
  \fn int QTimerEvent::timerId() const
  Returns the unique timer identifier, which is the same identifier as
  returned from QObject::startTimer().
*/


/*!
  \class QMouseEvent qevent.h

  \brief The QMouseEvent class contains parameters that describe a mouse event.

  Mouse events occur when a mouse button is pressed or released inside a
  widget or when the mouse cursor is moved.

  Mouse move events will occur only when some mouse button is pressed
  down, unless mouse tracking has been enabled with
  QWidget::setMouseTracking().

  Qt automatically grabs the mouse when a mouse button is pressed inside a
  widget; the widget will continue to receive mouse events until the
  last mouse button is released.

  A mouse event contains a special accept flag that tells whether the
  receiver wants the event.  You should call QMouseEvent::ignore() if the
  mouse event is not handled by your widget. A mouse event is propagated up
  the parent widget chain until a widget accepts it with QMousEvent::accept()
  or an event filter consumes it.

  The functions pos(), x() and y() give the cursor position relative
  to the widget that receives the mouse event. If you move the widget
  as a result of the mouse event, use the global position returned by
  globalPos() to avoid a shaking motion.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The event handlers QWidget::mousePressEvent(), QWidget::mouseReleaseEvent(),
  QWidget::mouseDoubleClickEvent() and QWidget::mouseMoveEvent() receive
  mouse events.

  \sa QWidget::setMouseTracking(), QWidget::grabMouse(), QCursor::pos()
*/

/*!
  \fn QMouseEvent::QMouseEvent( Type type, const QPoint &pos, int button, int state )

  Constructs a mouse event object.

  The \a type parameter must be one of \c QEvent::MouseButtonPress,
  \c QEvent::MouseButtonRelease,
  \c QEvent::MouseButtonDblClick or \c QEvent::MouseMove.

  The \a pos parameter specifies the position relative to the
  receiving widget. \a button specifies the ButtonState of the button
  that caused the event, which should be 0 if \a type is \c
  MouseMove. \a state is the ButtonState at the time of the event.

  The globalPos() is initialized to QCursor::pos(), which may not be
  appropriate. Use the other constructor to specify the global position
  explicitly.
*/

QMouseEvent::QMouseEvent( Type type, const QPoint &pos, int button, int state )
    : QEvent(type), p(pos), b(button),s((ushort)state){
	g = QCursor::pos();
}


/*!
  \fn QMouseEvent::QMouseEvent( Type type, const QPoint &pos, const QPoint &globalPos,  int button, int state )

  Constructs a mouse event object.

  The \a type parameter must be \c QEvent::MouseButtonPress,
  \c QEvent::MouseButtonRelease,
  \c QEvent::MouseButtonDblClick or \c QEvent::MouseMove.

  The \a pos parameter specifies the position relative to the
  receiving widget. \a globalPos is the position in absolute
  coordinates. \a button specifies the ButtonState of the button that
  caused the event, which should be 0 if \a type is \c MouseMove.
  \a state is the ButtonState at the time of the event.
*/

/*!
  \fn const QPoint &QMouseEvent::pos() const
  Returns the position of the mouse pointer relative to the widget that
  received the event.

  If you move the widget as a result of the mouse event, use the
  global position returned by globalPos() to avoid a shaking motion.

  \sa x(), y(), globalPos()
*/

/*!
  \fn const QPoint &QMouseEvent::globalPos() const

  Returns the global position of the mouse pointer \e at \e the \e
  time of the event. This is important on asynchronous window systems
  like X11. Whenever you move your widgets around in response to mouse
  events, globalPos() can differ a lot from the current pointer
  position QCursor::pos(), and from
  QWidget::mapToGlobal( pos() ).

  \sa globalX(), globalY()
*/

/*!
  \fn int QMouseEvent::x() const
  Returns the X position of the mouse pointer, relative to the widget that
  received the event.
  \sa y(), pos()
*/

/*!
  \fn int QMouseEvent::y() const
  Returns the Y position of the mouse pointer, relative to the widget that
  received the event.
  \sa x(), pos()
*/

/*!
  \fn int QMouseEvent::globalX() const
  Returns the global X position of the mouse pointer at the time of the event.
  \sa globalY(), globalPos()
*/

/*!
  \fn int QMouseEvent::globalY() const
  Returns the global Y position of the mouse pointer at the time of the event.
  \sa globalX(), globalPos()
*/

/*!
  \fn ButtonState QMouseEvent::button() const
  Returns the button that caused the event.

  Possible return values are \c LeftButton, \c RightButton, \c MidButton and
  \c NoButton.

  Note that the returned value is always \c NoButton (0) for mouse move
  events.

  \sa state()
*/


/*!
  \fn ButtonState QMouseEvent::state() const

  Returns the button state (a combination of mouse buttons and keyboard
  modifiers), i.e., what buttons and keys were being held depressed
  immediately before the event was generated.

  Note that this means that for \c QEvent::MouseButtonPress and \c
  QEvent::MouseButtonDblClick, the flag for the button() itself will not be
  set in the state, whereas for \c QEvent::MouseButtonRelease it will.

  This value is mainly interesting for \c QEvent::MouseMove; for the
  other cases, button() is more useful.

  The returned value is \c LeftButton, \c RightButton, \c MidButton,
  \c ShiftButton, \c ControlButton and \c AltButton OR'ed together.

  \sa button() stateAfter()
*/

/*!
  \fn ButtonState QMouseEvent::stateAfter() const

  Returns the state of buttons after the event.

  \sa state()
*/
Qt::ButtonState QMouseEvent::stateAfter() const
{
    return Qt::ButtonState(state()^button());
}



/*!
  \fn bool QMouseEvent::isAccepted() const
  Returns TRUE if the receiver of the event wants to keep the key.
*/

/*!
  \fn void QMouseEvent::accept()
  Sets the accept flag of the mouse event object.

  Setting the accept parameter indicates that the receiver of the event wants
  the mouse event. Unwanted mouse events are sent to the parent widget.

  The accept flag is set by default.

  \sa ignore()
*/


/*!
  \fn void QMouseEvent::ignore()
  Clears the accept flag parameter of the mouse event object.

  Clearing the accept parameter indicates that the event receiver does
  not want the mouse event. Unwanted mouse events are sent to the parent
  widget.

  The accept flag is set by default.

  \sa accept()
*/


/*!
  \class QWheelEvent qevent.h
  \brief The QWheelEvent class contains parameters that describe a wheel event.

  Wheel events occur when a mouse wheel is turned while the widget has
  focus.  The rotation distance is provided by delta(). The functions
  pos() and globalPos() return the mouse pointer location at the
  time of the event.

  A wheel event contains a special accept flag that tells whether the
  receiver wants the event.  You should call QWheelEvent::accept() if you
  handle the wheel event; otherwise it will be sent to the parent widget.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The event handler QWidget::wheelEvent() receives wheel events.

  \sa QMouseEvent, QWidget::grabMouse()
*/

/*!
  \fn QWheelEvent::QWheelEvent( const QPoint &pos, int delta, int state, Orientation orient = Vertical );

  Constructs a wheel event object.

  The globalPos() is initialized to QCursor::pos(), which usually is
  right (but not always). Use the other constructor if you need to
  specify the global position explicitly.

  \sa pos(), delta(), state()
*/
QWheelEvent::QWheelEvent( const QPoint &pos, int delta, int state, Orientation orient )
    : QEvent(Wheel), p(pos), d(delta), s((ushort)state),
      accpt(TRUE), o(orient)
{
    g = QCursor::pos();
}

/*!
  \fn QWheelEvent::QWheelEvent( const QPoint &pos, const QPoint& globalPos, int delta, int state, Orientation orient = Vertical  )

  Constructs a wheel event object.

  \sa pos(), globalPos(), delta(), state()
*/

/*!
  \fn int QWheelEvent::delta() const

  Returns the distance that the wheel is rotated expressed in
  multiples or divisions of WHEEL_DELTA, which is currently set at 120.
  A positive value indicates that the wheel was rotated
  forward away from the user; a negative value indicates that the
  wheel was rotated backward toward the user.

  The WHEEL_DELTA constant was set to 120 by the wheel mouse vendors
  to allow building finer-resolution wheels in the future, including
  perhaps a freely rotating wheel with no notches. The expectation is
  that such a device would send more messages per rotation but with a
  smaller value in each message.
*/

/*!
  \fn const QPoint &QWheelEvent::pos() const
  Returns the position of the mouse pointer, relative to the widget that
  received the event.

  If you move your widgets around in response to mouse
  events, use globalPos() instead of this function.

  \sa x(), y(), globalPos()
*/

/*!
  \fn int QWheelEvent::x() const
  Returns the X position of the mouse pointer, relative to the widget that
  received the event.
  \sa y(), pos()
*/

/*!
  \fn int QWheelEvent::y() const
  Returns the Y position of the mouse pointer, relative to the widget that
  received the event.
  \sa x(), pos()
*/


/*!
  \fn const QPoint &QWheelEvent::globalPos() const

  Returns the global position of the mouse pointer \e at \e the \e
  time of the event. This is important on asynchronous window systems
  such as X11; whenever you move your widgets around in response to mouse
  events, globalPos() can differ a lot from the current pointer
  position QCursor::pos().

  \sa globalX(), globalY()
*/

/*!
  \fn int QWheelEvent::globalX() const
  Returns the global X position of the mouse pointer at the time of the event.
  \sa globalY(), globalPos()
*/

/*!
  \fn int QWheelEvent::globalY() const
  Returns the global Y position of the mouse pointer at the time of the event.
  \sa globalX(), globalPos()
*/


/*!
  \fn ButtonState QWheelEvent::state() const
  Returns the keyboard modifier flags of the event.

  The returned value is \c ShiftButton, \c ControlButton, and \c AltButton
  OR'ed together.
*/

/*!
  \fn bool QWheelEvent::isAccepted() const
  Returns TRUE if the receiver of the event handles the wheel event.
*/

/*!
  \fn void QWheelEvent::accept()
  Sets the accept flag of the wheel event object.

  Setting the accept parameter indicates that the receiver of the event wants
  the wheel event. Unwanted wheel events are sent to the parent widget.

  The accept flag is set by default.

  \sa ignore()
*/

/*!
  \fn void QWheelEvent::ignore()
  Clears the accept flag parameter of the wheel event object.

  Clearing the accept parameter indicates that the event receiver does
  not want the wheel event. Unwanted wheel events are sent to the parent widget.
  The accept flag is set by default.

  \sa accept()
*/


/*! \enum Qt::Modifier

  This enum type describes the keyboard modifier keys supported by Qt.
  The currently defined values are:

  \value SHIFT (0x00200000) - the shift keys provided on all normal keyboards
  \value CTRL (0x00400000) - the control keys
  \value ALT (0x00800000) - the normal alt keys, but not e.g. AltGr.
  \value UNICODE_ACCEL - the accelerator is specified as a Unicode code
  point, not a Qt Key
*/

/*!
  \class QKeyEvent qevent.h
  \brief The QKeyEvent class contains parameters that describe a key event.

  Key events occur when a key is pressed or released when a widget has
  keyboard input focus.

  A key event contains a special accept flag that tells whether the
  receiver wants the key.  You should call QKeyEvent::ignore() if the
  key press or release event is not handled by your widget. A key event is
  propagated up the parent widget chain until a widget accepts it with
  QKeyEvent::accept() or an event filter consumes it.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The event handlers QWidget::keyPressEvent() and QWidget::keyReleaseEvent()
  receive key events.

  \sa QFocusEvent, QWidget::grabKeyboard()
*/

/*!
  \fn QKeyEvent::QKeyEvent( Type type, int key, int ascii, int state,
			    const QString& text, bool autorep, ushort count )
  Constructs a key event object.

  The \a type parameter must be \c QEvent::KeyPress or \c QEvent::KeyRelease.

  If \a key is 0 the event is not a result of a known key (e.g., it
  may be the result of a compose sequence or keyboard macro).

  \a text will be returned by text().

  If \a autorep is TRUE, isAutoRepeat() will be TRUE.

  \a count is the number of single keys.

  The accept flag is set to TRUE.
*/

/*!
  \fn int QKeyEvent::key() const
  Returns the code of the key that was pressed or released.

  The header file qnamespace.h lists the possible keyboard codes.  These codes
  are independent of the underlying window system.

  Key code 0 means that the event is not a result of a known key (e.g., it
  may be the result of a compose sequence or keyboard macro).
*/

/*!
  \fn int QKeyEvent::ascii() const
  Returns the ASCII code of the key that was pressed or released.
  We recommend using text() instead.

  \sa text()
*/

/*!
  \fn QString QKeyEvent::text() const
  Returns the Unicode text that this key generated.

  \sa QWidget::setKeyCompression()
*/

/*!
  \fn ButtonState QKeyEvent::state() const
  Returns the keyboard modifier flags that existed immediately before
  the event occurred.

  The returned value is \c ShiftButton, \c ControlButton and \c AltButton
  OR'ed together.

  \sa stateAfter()
*/

/*!
  \fn ButtonState QKeyEvent::stateAfter() const

  Returns the keyboard modifier flags that existed immediately after
  the event occurred.

  \warning This function cannot be trusted.

  \sa state()
*/
//###### We must check with XGetModifierMapping
Qt::ButtonState QKeyEvent::stateAfter() const
{
    if ( key() == Key_Shift )
	return Qt::ButtonState(state()^ShiftButton);
    if ( key() == Key_Control )
	return Qt::ButtonState(state()^ControlButton);
    if ( key() == Key_Alt )
	return Qt::ButtonState(state()^AltButton);
    return state();
}

/*!
  \fn bool QKeyEvent::isAccepted() const
  Returns TRUE if the receiver of the event wants to keep the key.
*/

/*!
  \fn void QKeyEvent::accept()
  Sets the accept flag of the key event object.

  Setting the accept parameter indicates that the receiver of the event wants
  the key event. Unwanted key events are sent to the parent widget.

  The accept flag is set by default.

  \sa ignore()
*/

/*! \fn bool QKeyEvent::isAutoRepeat() const

  Returns TRUE if this event comes from an auto-repeating key and
  FALSE if it comes from an initial press.

  Note that if the event is a multiple-key compressed event that
  partly is due to auto-repeat, this function returns an indeterminate
  value.
*/

/*!
  \fn int QKeyEvent::count() const

  Returns the number of single keys for this event. If text() is not
  empty, this is simply the length of the string.

  However, Qt also compresses invisible keycodes such as BackSpace.
  For those, count() returns the number of key presses/repeats this
  event represents.

  \sa QWidget::setKeyCompression()
*/

/*!
  \fn void QKeyEvent::ignore()
  Clears the accept flag parameter of the key event object.

  Clearing the accept parameter indicates that the event receiver does
  not want the key event. Unwanted key events are sent to the parent
  widget.

  The accept flag is set by default.

  \sa accept()
*/


/*!
  \class QFocusEvent qevent.h
  \brief The QFocusEvent class contains event parameters for widget focus
  events.

  Focus events are sent to widgets when the keyboard input focus
  changes.  This happens due to a mouse action, the Tab or Backtab keys,
  the window system, a keyboard shortcut, or some other application-specific issue. The actual reason for a specific event is obtained by
  reason() in the appropriate event handler.

  The event handlers QWidget::focusInEvent() and QWidget::focusOutEvent()
  receive focus events.

  \sa QWidget::setFocus(), QWidget::setFocusPolicy()
*/

/*!
  \fn QFocusEvent::QFocusEvent( Type type )
  Constructs a focus event object.

  The \a type parameter must be either \c QEvent::FocusIn or \c QEvent::FocusOut.
*/



QFocusEvent::Reason QFocusEvent::m_reason = QFocusEvent::Other;
QFocusEvent::Reason QFocusEvent::prev_reason = QFocusEvent::Other;


/*! \enum QFocusEvent::Reason

  \value Mouse  the focus change happened because of a mouse action.
  \value Tab  the focus change happened because of a Tab press
  \value Backtab  the focus change happened because of a Backtab press
        (possibly including Shift/Control).
  \value ActiveWindow  the window system made this window (in)active.
  \value Popup  the application opened/closed a popup that grabbed/released focus.
  \value Shortcut  the focus change happened because of a keyboard shortcut.
  \value Other  any other reason, usually application-specific.

  See the keyboard focus overview for more about focus.
*/

/*!
  Returns the reason for this focus event.

  \sa setReason()
 */
QFocusEvent::Reason QFocusEvent::reason()
{
    return m_reason;
}

/*!
  Sets the reason for all future focus events to \a reason.

  \sa reason(), resetReason()
 */
void QFocusEvent::setReason( Reason reason )
{
    prev_reason = m_reason;
    m_reason = reason;
}

/*!
  Resets the reason for all future focus events to the value before
  the last setReason() call.

  \sa reason(), setReason()
 */
void QFocusEvent::resetReason()
{
    m_reason = prev_reason;
}

/*!
  \fn bool QFocusEvent::gotFocus() const
  Returns TRUE if the widget received the text input focus.
*/

/*!
  \fn bool QFocusEvent::lostFocus() const
  Returns TRUE if the widget lost the text input focus.
*/


/*!
  \class QPaintEvent qevent.h
  \brief The QPaintEvent class contains event parameters for paint events.

  Paint events are sent to widgets that need to update themselves, for
  instance when part of a widget is exposed because an overlying widget is
  moved.

  The event contains a region() that needs to be updated, and a rect()
  that is the bounding rectangle of that region. Both are provided because
  many widgets can't make much use of region(), and rect() can be much
  faster than region().boundingRect().  Painting is clipped to region()
  during processing of a paint event.

  The erased() function returns TRUE if the region() has been cleared
  to the widget's background (see QWidget::backgroundMode()), and
  FALSE if the region's contents are arbitrary.

  \sa QPainter QWidget::update() QWidget::repaint()
  QWidget::paintEvent() QWidget::backgroundMode() QRegion
*/

/*!
  \fn QPaintEvent::QPaintEvent( const QRegion &paintRegion, bool erased=TRUE )

  Constructs a paint event object with the region that should be updated.
*/

/*!
  \fn QPaintEvent::QPaintEvent( const QRect &paintRect, bool erased=TRUE )

  Constructs a paint event object with the rectangle that should be updated.
*/

/*!
  \fn const QRect &QPaintEvent::rect() const

  Returns the rectangle that should be updated.

  \sa region(), QPainter::setClipRect()
*/

/*!
  \fn const QRegion &QPaintEvent::region() const

  Returns the region that should be updated.

  \sa rect(), QPainter::setClipRegion()
*/

/*!
  \fn bool QPaintEvent::erased() const

  Returns whether the paint event region (or rectangle) has been
  erased with the widget's background.
*/

/*!
  \class QMoveEvent qevent.h
  \brief The QMoveEvent class contains event parameters for move events.

  Move events are sent to widgets that have been moved to a new position
  relative to their parent.

  The event handler QWidget::moveEvent() receives move events.

  \sa QWidget::move(), QWidget::setGeometry()
*/

/*!
  \fn QMoveEvent::QMoveEvent( const QPoint &pos, const QPoint &oldPos )
  Constructs a move event with the new and old widget positions.
*/

/*!
  \fn const QPoint &QMoveEvent::pos() const
  Returns the new position of the widget, excluding window frame for top level
  widgets.
*/

/*!
  \fn const QPoint &QMoveEvent::oldPos() const
  Returns the old position of the widget.
*/


/*!
  \class QResizeEvent qevent.h
  \brief The QResizeEvent class contains event parameters for resize events.

  Resize events are sent to widgets that have been resized.

  The event handler QWidget::resizeEvent() receives resize events.

  \sa QWidget::resize(), QWidget::setGeometry()
*/

/*!
  \fn QResizeEvent::QResizeEvent( const QSize &size, const QSize &oldSize )
  Constructs a resize event with the new and old widget sizes.
*/

/*!
  \fn const QSize &QResizeEvent::size() const
  Returns the new size of the widget, which is the same as
  QWidget::size().
*/

/*!
  \fn const QSize &QResizeEvent::oldSize() const
  Returns the old size of the widget.
*/


/*!
  \class QCloseEvent qevent.h
  \brief The QCloseEvent class contains parameters that describe a close event.

  Close events are sent to widgets that the user wants to close, usually
  by choosing "Close" from the window menu. They are also sent when you
  call QWidget::close() to close a widget programmatically.

  Close events contain a flag that indicates whether the receiver wants
  the widget to be closed or not.  When a widget accepts the close
  event, it is hidden (and destroyed if it was created with the \c
  WDestructiveClose flag). If it refuses to accept the close event
  nothing happens. (Under X11 it is possible that the window manager
  will forcibly close the window; but at the time of writing we are not
  aware of any window manager that does this.)

  The main widget of the application - QApplication::mainWidget() - is
  a special case.  When it accepts the close event, Qt leaves the main
  event loop and the application is immediately terminated (i.e., it
  returns from the call to QApplication::exec() in the main()
  function).

  The event handler QWidget::closeEvent() receives close events.  The
  default implementation of this event handler accepts the close
  event.  If you do not want your widget to be hidden, or want some
  special handing, you should reimplement the event handler.

  The <a href="simple-application.html#closeEvent">closeEvent() in the
  Application Walkthrough</a> shows a close event handler that asks
  whether to save a document before closing.

  If you want the widget to be deleted when it is closed, simply
  create it with the \c WDestructiveClose widget flag.  This is very
  useful for independent top-level windows in a multi-window
  application. 

  QObject emits the \link QObject::destroyed() destroyed()\endlink signal
  when it is deleted.  

  If the last top-level window is closed, the
  QApplication::lastWindowClosed() signal is emitted.

  The isAccepted() function returns TRUE if the event's receiver has
  agreed to close the widget; call accept() to agree to close the widget
  and call ignore() if the receiver of this event does not want the
  widget to be closed.

  \sa QWidget::close(), QWidget::hide(), QObject::destroyed(),
  QApplication::setMainWidget(), QApplication::lastWindowClosed(),
  QApplication::exec(), QApplication::quit()
*/

/*!
  \fn QCloseEvent::QCloseEvent()
  Constructs a close event object with the accept parameter flag set to FALSE.
  \sa accept()
*/

/*!
  \fn bool QCloseEvent::isAccepted() const
  Returns TRUE if the receiver of the event has agreed to close the widget.
  \sa accept(), ignore()
*/

/*!
  \fn void QCloseEvent::accept()
  Sets the accept flag of the close event object.

  Setting the accept flag indicates that the receiver of this event agrees
  to close the widget.

  The accept flag is \e not set by default.

  If you choose to accept in QWidget::closeEvent(), the widget will be
  hidden.  If the widget's WDestructiveClose flag is set, it will also
  be destroyed.

  \sa ignore(), QWidget::hide()
*/

/*!
  \fn void QCloseEvent::ignore()
  Clears the accept flag of the close event object.

  Clearing the accept flag indicates that the receiver of this event does not
  want the widget to be closed.

    The close event is constructed with the accept flag cleared.

  \sa accept()
*/

/*!
  \class QContextMenuEvent qevent.h
  \brief The QContextMenuEvent class contains parameters that describe a context menu event.

  Context events are sent to widgets when a user triggers a menu. What
  triggers this is platform dependant. On windows, for example, pressing
  the menu button or releasing the right button will cause this event to
  be sent. It is customary to use this to show a QPopupMenu when this
  event is triggered if you have a relevant context menu.

  ContextMenu events contain a special accept flag that indicates whether
  the receiver accepted the contextMenu.  If the event handler does not
  accept the event, then whatever triggered the event will be handled as
  a regular input event if possible.

  \sa QPopupMenu
*/

/*!
  \fn QContextMenuEvent::QContextMenuEvent( Reason reason, const QPoint &pos, const QPoint &globalPos, int state )

  Constructs a context event object with the accept parameter flag set
  to FALSE.

  The \a reason parameter must be \c QContextMenuEvent::Mouse
  or \c QContextMenuEvent::Keyboard.

  The \a pos parameter specifies the mouse position relative to the
  receiving widget. \a globalPos is the mouse position in absolute
  coordinates. \a state is the ButtonState at the time of the event.
*/


/*!
  \fn QContextMenuEvent::QContextMenuEvent( Reason reason, const QPoint &pos, int state )

  Constructs a context event object with the accept parameter flag set
  to FALSE.

  The \a reason parameter must be \c QContextMenuEvent::Mouse
  or \c QContextMenuEvent::Keyboard.

  The \a pos parameter specifies the mouse position relative to the
  receiving widget. \a state is the ButtonState at the time of the event.

  The globalPos() is initialized to QCursor::pos(), which may not be
  appropriate. Use the other constructor to specify the global position
  explicitly.
*/

QContextMenuEvent::QContextMenuEvent( Reason reason, const QPoint &pos, int state )
    : QEvent( ContextMenu ), p( pos ), accpt( FALSE ), reas( reason ), s((ushort)state)
{
    gp = QCursor::pos();
}

/*!
  \fn const QPoint &QContextMenuEvent::pos() const

  Returns the position of the mouse pointer relative to the widget that
  received the event.

  \sa x(), y(), globalPos()
*/

/*!
  \fn int QContextMenuEvent::x() const

  Returns the X position of the mouse pointer, relative to the widget that
  received the event.

  \sa y(), pos()
*/

/*!
  \fn int QContextMenuEvent::y() const

  Returns the Y position of the mouse pointer, relative to the widget that
  received the event.

  \sa x(), pos()
*/

/*!
  \fn const QPoint &QContextMenuEvent::globalPos() const

  Returns the global position of the mouse pointer at the time of the event.

  \sa x(), y(), pos()
*/

/*!
  \fn int QContextMenuEvent::globalX() const

  Returns the global X position of the mouse pointer at the time of the event.
  \sa globalY(), globalPos()
*/

/*!
  \fn int QContextMenuEvent::globalY() const

  Returns the global Y position of the mouse pointer at the time of the event.
  \sa globalX(), globalPos()
*/

/*!
  \fn bool QContextMenuEvent::isAccepted() const
  Returns TRUE if the receiver of the event has taken the context.
  \sa accept(), ignore()
*/

/*!
  \fn void QContextMenuEvent::accept()
  Sets the accept flag of the context event object.

  Setting the accept flag indicates that the receiver of this event has
  taken the context and whatever input command that caused this event will
  not be handled as it normally would have.

  The accept flag is not set by default.

  \sa ignore()
*/

/*!
  \fn void QContextMenuEvent::ignore()
  Clears the accept flag of the context event object.

  Clearing the accept flag indicates that the receiver of this event does not
  need to show a context menu.

  The accept flag is not set by default.

  \sa accept()
*/

/*!
  \enum QContextMenuEvent::Reason

  This enum describes the reason the ContextMenuEvent was sent.
  The values are:

   \value Mouse     The mouse caused the event to be sent. Normally this
   means the right mouse button was clicked, but this is platform specific.

   \value Keyboard  The keyboard caused this event to be sent.
   On windows this means the menu button was pressed.

   \value Other	    The event was sent by some other means (i.e. not by the
   mouse or keyboard).
*/


/*!
  \fn QContextMenuEvent::Reason QContextMenuEvent::reason() const
  Returns the reason for this context event.
*/


/*!
  \class QChildEvent qevent.h
  \brief The QChildEvent class contains event parameters for child object
  events.

  Child events are sent to objects when children are inserted or removed.

  A \c ChildRemoved event is sent immediately, but a \c ChildInserted event
  is \e posted (with QApplication::postEvent()).



  The handler for these events is QObject::childEvent().
*/

/*!
  \fn QChildEvent::QChildEvent( Type type, QObject *child )
  Constructs a child event object. The \a child is the object that is to
  be removed or inserted.

  The \a type parameter must be either \c QEvent::ChildInserted
  or \c QEvent::ChildRemoved.
*/

/*!
  \fn QObject *QChildEvent::child() const
  Returns the child widget that was inserted or removed.
*/

/*!
  \fn bool QChildEvent::inserted() const
  Returns TRUE if the widget received a new child.
*/

/*!
  \fn bool QChildEvent::removed() const
  Returns TRUE if the object lost a child.
*/




/*!
  \class QCustomEvent qevent.h
  \brief The QCustomEvent class provides support for custom events.

  QCustomEvent is a generic event class for user-defined events. User
  defined events can be sent to widgets or other QObject instances
  using QApplication::postEvent() or
  QApplication::sendEvent(). Subclasses of QWidget can easily receive
  custom events by implementing the QWidget::customEvent() event
  handler function.

  QCustomEvent objects should be created with a type id that uniquely
  identifies the event type. To avoid clashes with the Qt-defined
  events types, the value should be at least as large as the value of
  the "User" entry in the QEvent::Type enum.

  QCustomEvent contains a generic void* data member that may be used
  for transferring event-specific data to the receiver. Note that
  since events are normally delivered asynchronously, the data
  pointer, if used, must remain valid until the event has been
  received and processed.

  QCustomEvent can be used as-is for simple user-defined event types,
  but normally you will want to make a subclass of it for your event
  types. In a subclass, you can add data members that are suitable for
  your event type.

  Example:
  \code
  class ColorChangeEvent : public QCustomEvent
  {
  public:
    ColorChangeEvent( QColor color )
	: QCustomEvent( 346798 ), c( color ) {};
    QColor color() const { return c; };
  private:
    QColor c;
  };

  // To send an event of this custom event type:

  ColorChangeEvent* ce = new ColorChangeEvent( blue );
  QApplication::postEvent( receiver, ce );    // Qt will delete it when done

  // To receive an event of this custom event type:

  void MyWidget::customEvent( QCustomEvent * e )
  {
    if ( e->type() == 346798 ) {              // It must be a ColorChangeEvent
      ColorChangeEvent* ce = (ColorChangeEvent*)e;
      newColor = ce->color();
    }
  }
  \endcode

  \sa QWidget::customEvent(), QApplication::notify()
*/


/*!
  Constructs a custom event object with event type \a type. The value
  of \a type must be at least as large as QEvent::User. The data
  pointer is set to 0.
*/

QCustomEvent::QCustomEvent( int type )
    : QEvent( (QEvent::Type)type ), d( 0 )
{
#if defined(QT_CHECK_RANGE)
    if ( type < (int)QEvent::User )
	qWarning( "QCustomEvent: Illegal type id." );
#endif
}


/*!
  \fn QCustomEvent::QCustomEvent( Type type, void *data )
  Constructs a custom event object with the event type \a type and a
  pointer to \a data. (Note that any int value may safely be cast to
  QEvent::Type).
*/


/*!
  \fn void QCustomEvent::setData( void* data )

  Sets the generic data pointer to \a data.

  \sa data()
*/

/*!
  \fn void *QCustomEvent::data() const

  Returns a pointer to the generic event data.

  \sa setData()
*/



/*!
  \fn QDragMoveEvent::QDragMoveEvent( const QPoint& pos, Type type )

  Creates a QDragMoveEvent for which the mouse is at point \a pos,
  and the given event \a type.

  Note that internal state is also involved with QDragMoveEvent,
  so it is not useful to create these yourself.
*/

/*!
  \fn void   QDragMoveEvent::accept( const QRect & r )

  The same as accept(), but also notifies that future moves will
  also be acceptable if they remain within the rectangle \a r on the
  widget - this can improve performance, but may also be ignored by
  the underlying system.

  If the rectangle \link QRect::isEmpty() is empty\endlink, then drag
  move events will be sent continuously.  This is useful if the source is
  scrolling in a timer event.
*/

/*!
  \fn void   QDragMoveEvent::ignore( const QRect & r)

  The opposite of accept(const QRect&).
*/

/*!
  \fn QRect  QDragMoveEvent::answerRect() const

  Returns the rectangle for which the acceptance of
  the move event applies.
*/



/*!
  \fn const QPoint& QDropEvent::pos() const

  Returns the position where the drop was made.
*/

/*!
  \fn bool QDropEvent::isAccepted () const

  Returns TRUE if the drop target accepts the event.
*/

/*!
  \fn void QDropEvent::accept(bool y=TRUE)

  \reimp

  Call this to indicate whether the event provided data which your
  widget processed.  To get the data, use encodedData(), or
  preferably, the decode() methods of existing QDragObject subclasses,
  such as QTextDrag::decode(), or your own subclasses.

  \warning To accept or reject the drop, call acceptAction(), not this
  function.  This function indicates whether you processed the event
  at all.

  \sa acceptAction()
*/

/*!
  \fn void QDropEvent::acceptAction(bool y=TRUE)

  Call this to indicate that the action described by action() is accepted,
  not merely the default copy action.  If you call acceptAction(TRUE),
  there is no need to also call accept(TRUE).
*/

/*!
  \fn void QDragMoveEvent::accept( bool y )
  \reimp
  \internal
  Remove in 3.0
*/

/*!
  \fn void QDragMoveEvent::ignore()
  \reimp
  \internal
  Remove in 3.0
*/


/*!
  \enum QDropEvent::Action

  This type describes the action which a source requests that a target
  perform with dropped data.  The values are:

   \value Copy  the default action.  The source simply users the data
	    provided in the operation.
   \value Link  The source should somehow create a link to the location
	    specified by the data.
   \value Move  The source should somehow move the object from the location
	    specified by the data to a new location.
   \value Private  The target has special knowledge of the MIME type, which
	    the source should respond to similar to a Copy.
   \value UserAction  The source and target can co-operate using special
	    actions.  This feature is not supported in Qt at this time.

  The Link and Move actions only makes sense if the data is
  a reference, such as text/uri-list file lists (see QUriDrag).
*/

/*!
  \fn void QDropEvent::setAction( Action a )

  Sets the action.  This is used internally, you should not need to
  call this in your code - the \e source decides the action, not the
  target.
*/

/*!
  \fn Action QDropEvent::action() const

  Returns the Action which the target is requesting be performed with
  the data.  If your application understands the action and can
  process the supplied data, call acceptAction(); if your application
  can process the supplied data but can only perform the Copy action,
  call accept().
*/

/*!
  \fn void QDropEvent::ignore()

  The opposite of accept().
*/

/*! \fn bool QDropEvent::isActionAccepted () const

  Returns TRUE if the drop action was accepted by the drop site, and
  FALSE if not.
*/


/*! \fn void QDropEvent::setPoint (const QPoint & np)

  Sets the drop to happen at \a np.  You do normally not need to use
  this as it will be set internally before your widget receives the
  drop event.
*/ // ### here too - what coordinate system?


/*!
  \class QDragEnterEvent qevent.h
  \brief The QDragEnterEvent class provides an event which is sent to the widget when a drag and drop first drags onto the widget.

  This event is always immediate followed by a QDragMoveEvent, thus you need
  only respond to one or the other event.  Note that this class inherits most
  of its functionality from QDragMoveEvent, which in turn inherits most
  of its functionality from QDropEvent.

  \sa QDragLeaveEvent, QDragMoveEvent, QDropEvent
*/

/*!
  \fn QDragEnterEvent::QDragEnterEvent (const QPoint & pos)
  Constructs a QDragEnterEvent entering at the given point.
  Note that QDragEnterEvent constructed outside of the Qt internals
  will not work - they currently rely on internal state.
*/

/*!
  \class QDragLeaveEvent qevent.h
  \brief The QDragLeaveEvent class provides an event which is sent to the widget when a drag and drop leaves the widget

  This event is always preceded by a QDragEnterEvent and a series
  of QDragMoveEvent.  It is not sent if a QDropEvent is sent instead.

  \sa QDragEnterEvent, QDragMoveEvent, QDropEvent
*/

/*!
  \fn QDragLeaveEvent::QDragLeaveEvent()
  Constructs a QDragLeaveEvent. You must not create QDragLeaveEvents
  yourself, as they rely on Qt's internal state.
*/

/*!
  \class QHideEvent qevent.h
  \brief The QHideEvent class provides an event which is sent after a widget is hidden.

  This event is sent just before QWidget::hide() returns, and also when
  a top-level window has been hidden (iconified) by the user.
  
  if spontaneous() is TRUE the event originated outside the
  application - i.e., the user hid the window via the window manager
  controls, either by iconifying the window or by switching to another
  virtual desktop where the window isn't visible. The window will
  become hidden but not withdrawn.  If the window was iconified,
  QWidget::isMinimized() is TRUE.


  \sa QShowEvent
*/

/*!
  \fn QHideEvent::QHideEvent()

  Constructs a QHideEvent.
*/

/*!
  \class QShowEvent qevent.h
  \brief The QShowEvent class provides an event which is sent when a widget is shown.

  There are two kinds of show events: spontaneous show events by the
  window system and internal show events. Spontaneous show events are
  sent just after the window system shows the window, including after
  a top-level window has been shown (un-iconified) by the
  user. Internal show events are delivered just before the widget
  becomes visible.

  \sa QHideEvent
*/

/*!
  \fn QShowEvent::QShowEvent()

  Constructs a QShowEvent.
*/


/*!
  \fn QByteArray QDropEvent::data(const char* f) const

  \obsolete

  Use QDropEvent::encodedData().
*/


/*!
  Destroys the event.  If it was \link
  QApplication::postEvent() posted \endlink,
  it will be removed from the list of events to be posted.

  \internal
  It used to print an error (useful for people who posted events
  that were on the stack).
*/

QEvent::~QEvent()
{
    if (posted)
	QApplication::removePostedEvent( this );
}


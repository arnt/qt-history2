/****************************************************************************
** $Id: $
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

  \ingroup events
  \ingroup environment

  The main event loop of Qt (QApplication::exec()) fetches
  native window system events from the event queue, translates them
  into QEvents and sends the translated events to QObjects.

  Generally, events come from the underlying window system (spontaneous()
  returns TRUE) but it is also possible to manually send events through the
  QApplication class using QApplication::sendEvent() and
  QApplication::postEvent() (spontaneous() returns FALSE).

  QObjects receive events by having their QObject::event() function
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
  QApplication::sendEvent() QApplication::postEvent()
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
  \value MidButton  the middle button.
  \value ShiftButton  a Shift key on the keyboard is also pressed.
  \value ControlButton  a Ctrl key on the keyboard is also pressed.
  \value AltButton  an Alt (or Meta) key on the keyboard is also pressed.
  \value MetaButton a Meta key on the keyboard is also pressed.
  \value Keypad  a keypad button is pressed.
  \value KeyButtonMask is a mask for ShiftButton, ControlButton and
  AltButton.
  \value MouseButtonMask is a mask for LeftButton, RightButton and MidButton.
*/

/*! \enum QEvent::Type

  This enum type defines the valid event types in Qt.  The currently
  defined event types and the specialized classes for each type are:

  \value None  Not an event.
  \value Accessibility Accessibility information is requested
  \value Timer  Regular timer events, \l{QTimerEvent}.
  \value MouseButtonPress  Mouse press, \l{QMouseEvent}.
  \value MouseButtonRelease  Mouse release, \l{QMouseEvent}.
  \value MouseButtonDblClick  Mouse press again, \l{QMouseEvent}.
  \value MouseMove  Mouse move, \l{QMouseEvent}.
  \value KeyPress  Key press (including Shift, for example), \l{QKeyEvent}.
  \value KeyRelease  Key release, \l{QKeyEvent}.
  \value IMStart The start of input method composition.
  \value IMCompose Input method composition is taking place.
  \value IMEnd The end of input method composition.
  \value FocusIn  Widget gains keyboard focus, \l{QFocusEvent}.
  \value FocusOut  Widget loses keyboard focus, \l{QFocusEvent}.
  \value Enter  Mouse enters widget's boundaries.
  \value Leave  Mouse leaves widget's boundaries.
  \value Paint  Screen update necessary, \l{QPaintEvent}.
  \value Move  Widget's position changed, \l{QMoveEvent}.
  \value Resize  Widget's size changed, \l{QResizeEvent}.
  \value Show  Widget was shown on screen, \l{QShowEvent}.
  \value Hide  Widget was hidden, \l{QHideEvent}.
  \value ShowToParent A child widget has been shown.
  \value HideToParent A child widget has been hidden.
  \value Close  Widget was closed (permanently), \l{QCloseEvent}.
  \value ShowNormal Widget should be shown normally.
  \value ShowMaximized Widget should be shown maximized.
  \value ShowMinimized Widget should be shown minimized.
  \value ShowWindowRequest Widget's window should be shown. \obsolete
  \value DeferredDelete The object will be deleted after it has
  cleaned up.
  \value Accel  Key press in child for shortcut key handling, \l{QKeyEvent}.
  \value Wheel  Mouse wheel rolled, \l{QWheelEvent}.
  \value ContextMenu context popup menu, \l{QContextMenuEvent}
  \value AccelAvailable  Internal event used by Qt on some platforms.
  \value AccelOverride  Key press in child, for overriding shortcut key handling, \l{QKeyEvent}.
  \value WindowActivate  Window was activated.
  \value WindowDeactivate  Window was deactivated.
  \value CaptionChange  Widget's caption changed.
  \value IconChange  Widget's icon changed.
  \value ParentFontChange  Font of the parent widget changed.
  \value ApplicationFontChange  Default application font changed.
  \value PaletteChange  Palette of the widget changed.
  \value ParentPaletteChange  Palette of the parent widget changed.
  \value ApplicationPaletteChange  Default application palette changed.
  \value Clipboard  Clipboard contents have changed.
  \value SockAct  Socket activated, used to implement \l{QSocketNotifier}.
  \value DragEnter  A drag-and-drop enters widget, \l{QDragEnterEvent}.
  \value DragMove  A drag-and-drop is in progress, \l{QDragMoveEvent}.
  \value DragLeave  A drag-and-drop leaves widget, \l{QDragLeaveEvent}.
  \value Drop  A drag-and-drop is completed, \l{QDropEvent}.
  \value DragResponse  Internal event used by Qt on some platforms.
  \value ChildInserted  Object gets a child, \l{QChildEvent}.
  \value ChildRemoved  Object loses a child, \l{QChildEvent}.
  \value LayoutHint  Widget child has changed layout properties.
  \value ActivateControl  Internal event used by Qt on some platforms.
  \value DeactivateControl  Internal event used by Qt on some platforms.
  \value Quit  Reserved.
  \value Create  Reserved.
  \value Destroy  Reserved.
  \value Reparent  Reserved.
  \value Speech Reserved for speech input.
  \value Tablet Wacom Tablet event.
  \value User  User defined event.
  \value MaxUser  Last user event id.

  User events should have values between User and MaxUser inclusive.
*/
/*!
  \fn QEvent::QEvent( Type type )
  Contructs an event object of type \a type.
*/

/*!
  \fn QEvent::Type QEvent::type() const
  Returns the event type.
*/

/*!
  \fn bool QEvent::spontaneous() const

  Returns TRUE if the event originated outside the application,
  i.e. it is a system event; otherwise returns FALSE.
*/


/*!
  \class QTimerEvent qevent.h
  \ingroup events
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
  \ingroup events

  \brief The QMouseEvent class contains parameters that describe a mouse event.

  Mouse events occur when a mouse button is pressed or released inside a
  widget or when the mouse cursor is moved.

  Mouse move events will occur only when a mouse button is pressed
  down, unless mouse tracking has been enabled with
  QWidget::setMouseTracking().

  Qt automatically grabs the mouse when a mouse button is pressed inside a
  widget; the widget will continue to receive mouse events until the
  last mouse button is released.

  A mouse event contains a special accept flag that indicates whether the
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

  Returns the global position of the mouse pointer \e{at the time} of
  the event. This is important on asynchronous window systems like
  X11. Whenever you move your widgets around in response to mouse
  events, globalPos() may differ a lot from the current pointer
  position QCursor::pos(), and from QWidget::mapToGlobal( pos() ).

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

  Note that the returned value is always \c NoButton for mouse move
  events.

  \sa state()
*/


/*!
  \fn ButtonState QMouseEvent::state() const

  Returns the button state (a combination of mouse buttons and keyboard
  modifiers), i.e. what buttons and keys were being pressed
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
  Returns TRUE if the receiver of the event wants to keep the key;
  otherwise returns FALSE.
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
  \ingroup events
  \brief The QWheelEvent class contains parameters that describe a wheel event.

  Wheel events occur when a mouse wheel is turned while the widget has
  focus.  The rotation distance is provided by delta(). The functions
  pos() and globalPos() return the mouse pointer location at the
  time of the event.

  A wheel event contains a special accept flag that indicates whether the
  receiver wants the event.  You should call QWheelEvent::accept() if you
  handle the wheel event; otherwise it will be sent to the parent widget.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The event handler QWidget::wheelEvent() receives wheel events.

  \sa QMouseEvent, QWidget::grabMouse()
*/

/*!
    \fn Orientation QWheelEvent::orientation() const
    Returns the wheel's orientation.
*/

/*!
  \fn QWheelEvent::QWheelEvent( const QPoint &pos, int delta, int state, Orientation orient = Vertical );

  Constructs a wheel event object.

  The globalPos() is initialized to QCursor::pos(), i.e. \a pos, which
  is usually (but not always) right. Use the other constructor if you
  need to specify the global position explicitly. \a delta contains
  the rotation distance, \a state holds the keyboard modifier
  flags at the time of the event and \a orient holds the wheel's orientation.

  \sa pos(), delta(), state()
*/
#ifndef QT_NO_WHEELEVENT
QWheelEvent::QWheelEvent( const QPoint &pos, int delta, int state, Orientation orient )
    : QEvent(Wheel), p(pos), d(delta), s((ushort)state),
      accpt(FALSE), o(orient)
{
    g = QCursor::pos();
}
#endif
/*!
  \fn QWheelEvent::QWheelEvent( const QPoint &pos, const QPoint& globalPos, int delta, int state, Orientation orient = Vertical  )

  Constructs a wheel event object. The position when the event
  occurred is given in \a pos and \a globalPos. \a delta contains the
  rotation distance, \a state holds the keyboard modifier flags at the
  time of the event and \a orient holds the wheel's orientation.

  \sa pos(), globalPos(), delta(), state()
*/

/*!
  \fn int QWheelEvent::delta() const

  Returns the distance that the wheel is rotated expressed in
  multiples or divisions of WHEEL_DELTA, which is currently set at 120.
  A positive value indicates that the wheel was rotated
  forwards away from the user; a negative value indicates that the
  wheel was rotated backwards toward the user.

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

  Returns the global position of the mouse pointer \e{at the time} of
  the event. This is important on asynchronous window systems
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
  Returns TRUE if the receiver of the event handles the wheel event;
  otherwise returns FALSE.
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

  \value SHIFT the Shift keys provided on all standard keyboards.
  \value CTRL the Ctrl keys.
  \value ALT the normal Alt keys, but not e.g. AltGr.
  \value MODIFIER_MASK is a mask of Shift, Ctrl and Alt.
  \value UNICODE_ACCEL the accelerator is specified as a Unicode code
  point, not as a Qt Key.
*/

/*!
  \class QKeyEvent qevent.h
  \ingroup events
  \brief The QKeyEvent class contains describes a key event.

  Key events occur when a key is pressed or released when a widget has
  keyboard input focus.

  A key event contains a special accept flag that indicates whether the
  receiver wants the key event.  You should call QKeyEvent::ignore() if the
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
  If \a key is 0 the event is not a result of a known key (e.g. it
  may be the result of a compose sequence or keyboard macro).
  \a ascii is the ASCII code of the key that was pressed or released.
  \a state holds the keyboard modifiers.
  \a text is the Unicode text that the key generated.
  If \a autorep is TRUE, isAutoRepeat() will be TRUE.
  \a count is the number of single keys.

  The accept flag is set to TRUE.
*/

/*!
  \fn int QKeyEvent::key() const
  Returns the code of the key that was pressed or released.

  See \l Qt::Key for the list of keyboard codes.  These codes
  are independent of the underlying window system.

  Key code 0 means that the event is not a result of a known key (e.g. it
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
  Returns TRUE if the receiver of the event wants to keep the key;
  otherwise returns FALSE
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
  FALSE if it comes from an initial key press.

  Note that if the event is a multiple-key compressed event that
  is partly due to auto-repeat, this function could return either TRUE
  or FALSE indeterminately.
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
    \enum Qt::Key

    The key names used by Qt.

    \value Key_Escape
    \value Key_Tab
    \value Key_Backtab
    \value Key_Backspace
    \value Key_Return
    \value Key_Enter
    \value Key_Insert
    \value Key_Delete
    \value Key_Pause
    \value Key_Print
    \value Key_SysReq
    \value Key_Home
    \value Key_End
    \value Key_Left
    \value Key_Up
    \value Key_Right
    \value Key_Down
    \value Key_Prior
    \value Key_Next
    \value Key_Shift
    \value Key_Control
    \value Key_Meta
    \value Key_Alt
    \value Key_CapsLock
    \value Key_NumLock
    \value Key_ScrollLock
    \value Key_F1
    \value Key_F2
    \value Key_F3
    \value Key_F4
    \value Key_F5
    \value Key_F6
    \value Key_F7
    \value Key_F8
    \value Key_F9
    \value Key_F10
    \value Key_F11
    \value Key_F12
    \value Key_F13
    \value Key_F14
    \value Key_F15
    \value Key_F16
    \value Key_F17
    \value Key_F18
    \value Key_F19
    \value Key_F20
    \value Key_F21
    \value Key_F22
    \value Key_F23
    \value Key_F24
    \value Key_F25
    \value Key_F26
    \value Key_F27
    \value Key_F28
    \value Key_F29
    \value Key_F30
    \value Key_F31
    \value Key_F32
    \value Key_F33
    \value Key_F34
    \value Key_F35
    \value Key_Super_L
    \value Key_Super_R
    \value Key_Menu
    \value Key_Hyper_L
    \value Key_Hyper_R
    \value Key_Help
    \value Key_Space
    \value Key_Any
    \value Key_Exclam
    \value Key_QuoteDbl
    \value Key_NumberSign
    \value Key_Dollar
    \value Key_Percent
    \value Key_Ampersand
    \value Key_Apostrophe
    \value Key_ParenLeft
    \value Key_ParenRight
    \value Key_Asterisk
    \value Key_Plus
    \value Key_Comma
    \value Key_Minus
    \value Key_Period
    \value Key_Slash
    \value Key_0
    \value Key_1
    \value Key_2
    \value Key_3
    \value Key_4
    \value Key_5
    \value Key_6
    \value Key_7
    \value Key_8
    \value Key_9
    \value Key_Colon
    \value Key_Semicolon
    \value Key_Less
    \value Key_Equal
    \value Key_Greater
    \value Key_Question
    \value Key_At
    \value Key_A
    \value Key_B
    \value Key_C
    \value Key_D
    \value Key_E
    \value Key_F
    \value Key_G
    \value Key_H
    \value Key_I
    \value Key_J
    \value Key_K
    \value Key_L
    \value Key_M
    \value Key_N
    \value Key_O
    \value Key_P
    \value Key_Q
    \value Key_R
    \value Key_S
    \value Key_T
    \value Key_U
    \value Key_V
    \value Key_W
    \value Key_X
    \value Key_Y
    \value Key_Z
    \value Key_BracketLeft
    \value Key_Backslash
    \value Key_BracketRight
    \value Key_AsciiCircum
    \value Key_Underscore
    \value Key_QuoteLeft
    \value Key_BraceLeft
    \value Key_Bar
    \value Key_BraceRight
    \value Key_AsciiTilde

    \value Key_nobreakspace
    \value Key_exclamdown
    \value Key_cent
    \value Key_sterling
    \value Key_currency
    \value Key_yen
    \value Key_brokenbar
    \value Key_section
    \value Key_diaeresis
    \value Key_copyright
    \value Key_ordfeminine
    \value Key_guillemotleft
    \value Key_notsign
    \value Key_hyphen
    \value Key_registered
    \value Key_macron
    \value Key_degree
    \value Key_plusminus
    \value Key_twosuperior
    \value Key_threesuperior
    \value Key_acute
    \value Key_mu
    \value Key_paragraph
    \value Key_periodcentered
    \value Key_cedilla
    \value Key_onesuperior
    \value Key_masculine
    \value Key_guillemotright
    \value Key_onequarter
    \value Key_onehalf
    \value Key_threequarters
    \value Key_questiondown
    \value Key_Agrave
    \value Key_Aacute
    \value Key_Acircumflex
    \value Key_Atilde
    \value Key_Adiaeresis
    \value Key_Aring
    \value Key_AE
    \value Key_Ccedilla
    \value Key_Egrave
    \value Key_Eacute
    \value Key_Ecircumflex
    \value Key_Ediaeresis
    \value Key_Igrave
    \value Key_Iacute
    \value Key_Icircumflex
    \value Key_Idiaeresis
    \value Key_ETH
    \value Key_Ntilde
    \value Key_Ograve
    \value Key_Oacute
    \value Key_Ocircumflex
    \value Key_Otilde
    \value Key_Odiaeresis
    \value Key_multiply
    \value Key_Ooblique
    \value Key_Ugrave
    \value Key_Uacute
    \value Key_Ucircumflex
    \value Key_Udiaeresis
    \value Key_Yacute
    \value Key_THORN
    \value Key_ssharp
    \value Key_agrave
    \value Key_aacute
    \value Key_acircumflex
    \value Key_atilde
    \value Key_adiaeresis
    \value Key_aring
    \value Key_ae
    \value Key_ccedilla
    \value Key_egrave
    \value Key_eacute
    \value Key_ecircumflex
    \value Key_ediaeresis
    \value Key_igrave
    \value Key_iacute
    \value Key_icircumflex
    \value Key_idiaeresis
    \value Key_eth
    \value Key_ntilde
    \value Key_ograve
    \value Key_oacute
    \value Key_ocircumflex
    \value Key_otilde
    \value Key_odiaeresis
    \value Key_division
    \value Key_oslash
    \value Key_ugrave
    \value Key_uacute
    \value Key_ucircumflex
    \value Key_udiaeresis
    \value Key_yacute
    \value Key_thorn
    \value Key_ydiaeresis

    \value Key_unknown

*/


/*!
  \class QFocusEvent qevent.h
  \ingroup events
  \brief The QFocusEvent class contains event parameters for widget focus
  events.

  Focus events are sent to widgets when the keyboard input focus
  changes. Focus events occur due to mouse actions, keypresses (e.g. Tab
  or Backtab), the window system, popup menus, keyboard shortcuts or
  other application specific reasons. The reason for a particular focus
  event is returned by reason() in the appropriate event handler.

  The event handlers QWidget::focusInEvent() and QWidget::focusOutEvent()
  receive focus events.

  Use setReason() to set the reason for all focus events, and
  resetReason() to set the reason for all focus events to the reason in
  force before the last setReason() call.

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

    This enum specifies why the focus changed:

  \value Mouse  because of a mouse action.
  \value Tab  because of a Tab press
  \value Backtab  because of a Backtab press
        (possibly including Shift/Control, e.g. Shift+Tab).
  \value ActiveWindow  because the window system made this window (in)active.
  \value Popup  because the application opened/closed a popup that grabbed/released focus.
  \value Shortcut  because of a keyboard shortcut.
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
  Returns TRUE if the widget received the text input focus; otherwise
  returns FALSE.
*/

/*!
  \fn bool QFocusEvent::lostFocus() const
  Returns TRUE if the widget lost the text input focus; otherwise returns FALSE.
*/


/*!
  \class QPaintEvent qevent.h
  \ingroup events
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
  The region is given by \a paintRegion. If \a erased is TRUE the
  region will be cleared before repainting.
*/

/*!
  \fn QPaintEvent::QPaintEvent( const QRect &paintRect, bool erased=TRUE )

  Constructs a paint event object with the rectangle that should be updated.
  The region is given by \a paintRect. If \a erased is TRUE the
  region will be cleared before repainting.
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
  \ingroup events
  \brief The QMoveEvent class contains event parameters for move events.

  Move events are sent to widgets that have been moved to a new position
  relative to their parent.

  The event handler QWidget::moveEvent() receives move events.

  \sa QWidget::move(), QWidget::setGeometry()
*/

/*!
  \fn QMoveEvent::QMoveEvent( const QPoint &pos, const QPoint &oldPos )
  Constructs a move event with the new and old widget positions, \a
  pos and \a oldPos respectively.
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
  \ingroup events
  \brief The QResizeEvent class contains event parameters for resize events.

  Resize events are sent to widgets that have been resized.

  The event handler QWidget::resizeEvent() receives resize events.

  \sa QWidget::resize(), QWidget::setGeometry()
*/

/*!
  \fn QResizeEvent::QResizeEvent( const QSize &size, const QSize &oldSize )
  Constructs a resize event with the new and old widget sizes, \a
  size and \a oldSize respectively.
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
  \ingroup events
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
  Returns TRUE if the receiver of the event has agreed to close the
  widget; otherwise returns FALSE.
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
  \ingroup events
  \brief The QContextMenuEvent class contains parameters that describe a context menu event.

  Context events are sent to widgets when a user triggers a menu. What
  triggers this is platform dependent. On windows, for example, pressing
  the menu button or releasing the right mouse button will cause this event to
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
    : QEvent( ContextMenu ), p( pos ), accpt( FALSE ), consum(FALSE), 
    reas( reason ), s((ushort)state)
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
  \fn ButtonState QContextMenuEvent::state() const

  Returns the button state (a combination of mouse buttons and keyboard
  modifiers), i.e., what buttons and keys were being pressed
  immediately before the event was generated.

  The returned value is \c LeftButton, \c RightButton, \c MidButton,
  \c ShiftButton, \c ControlButton and \c AltButton OR'ed together.
*/

/*!
  \fn bool QContextMenuEvent::isConsumed() const
  Returns TRUE (which stops propagation of the event) if the receiver
  has blocked the event; otherwise returns FALSE.
  \sa accept(), ignore(), consume()
*/

/*!
  \fn void QContextMenuEvent::consume()
  Sets the consume flag of the context event object.

  Setting the consume flag indicates that the receiver of this event asked
  that the event not be propagated further (to parent classes).

  The consumed flag is not set by default.

  \sa ignore() accept()
*/

/*!
  \fn bool QContextMenuEvent::isAccepted() const
  Returns TRUE if the receiver has processed the event; otherwise returns
  FALSE.  

  \sa accept(), ignore(), consume()
*/

/*!
  \fn void QContextMenuEvent::accept()
  Sets the accept flag of the context event object.

  Setting the accept flag indicates that the receiver of this event has
  processed the event. Processing the event means you did something with it
  and it will be implicitly consume().

  The accept flag is not set by default.

  \sa ignore() consume()
*/

/*!
  \fn void QContextMenuEvent::ignore()
  Clears the accept flag of the context event object.

  Clearing the accept flag indicates that the receiver of this event does not
  need to show a context menu. This will implicitly remove the consumed flag 
  as well.

  The accept flag is not set by default.

  \sa accept() consume()
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
  \class QIMEvent qevent.h
  \ingroup events
  \brief The QIMEvent class provides parameters for input method events.

  Input method events are send to widgets, when an input method is
  used to enter text into a widget. Input methods are widely used to
  enter text in Asian languages.

  The events are of interest to widgets that accept keyboard input
  and want to be able to correctly handle Asian languages. Text input
  in Asian languages is usually a three step process.  When the user
  presses the first key on a keyboard an input context is
  created. This input context will contain a string with the typed
  characters. With every new key pressed, the input method will try to
  create a matching string for the text typed so far.

  While the input context is active, the user can move the cursor only
  inside the string belonging to this input context. At some point,
  when the user presses the Spacebar, they get to the second stage,
  where they can choose from a number of strings that match the text
  they have typed so far. The user can press Enter to confirm their
  choice or Escape to cancel the input; in either case the input
  context will be closed. Note that the particular key presses used
  for a given input context may differ from those we've mentioned
  here, i.e. they may not be Spacebar, Enter and Escape.

  These three stages are represented by three different types of
  events. The IMStartEvent, IMComposeEvent and IMEndEvent. When a new
  input context is created, an IMStartEvent will be sent to the
  widget and delivered to the \l QWidget::imStartEvent() function. The
  widget can then update internal data structures to reflect this.

  After this, an IMComposeEvent will be send to the widget with every
  key the user presses. It will contain the current composition string
  the widget has to show and the current cursor position within the
  composition string. This string is temporary and can change with
  every key the user types, so the widget will need to store the state
  before the composition started (the state it had when it received
  the IMStartEvent). IMComposeEvents will be delivered to the
  \l QWidget::imComposeEvent() function.

  Usually, widgets try to mark the part of the text that is part of
  the current composition in a way that is visible to the user. Mostly
  this is achieved by using e.g. dotted underline.

  After the user has selected the final string, and IMEndEvent will be
  sent to the widget. The event contains the final string the user
  selected. This string has to be accepted as the final text the user
  entered, and the intermediate composition string should be cleared.
  These events are delivered to \l QWidget::imEndEvent().

    If the user clicks another widget, taking the focus out of the
    widget where the compose is taking place the IMEndEvent will be
    sent and the string it holds will be the result of the composition
    up to that point (which could be an empty string).
*/

/*!
  \fn  QIMEvent::QIMEvent( Type type, const QString &text, int cursorPosition )

  Constructs a new QIMEvent with accept flag set to FALSE. \a type can
  be one of QEvent::IMStartEvent, QEvent::IMComposeEvents and
  QEvent::IMEndEvent. \a text contains the current compostion string
  and \a cursorPosition the current position of the cursor inside \a
  text.
*/

/*!
  \fn const QString &QIMEvent::text() const

  Returns the composition text. This is a null string for an
  IMStartEvent, and contains the final accepted string in the
  IMEndEvent.
*/

/*!
  \fn int QIMEvent::cursorPos() const

  Returns the current cursor position inside the composition string.
  Will return 0 for IMStartEvent and IMEndEvent.
*/

/*!
  \fn bool QIMEvent::isAccepted() const
  Returns TRUE if the receiver of the event processed the event;
  otherwise returns FALSE.
*/

/*!
  \fn void QIMEvent::accept()
  Sets the accept flag of the input method event object.

  Setting the accept parameter indicates that the receiver of the event processed
  the input method event.

  The accept flag is not set by default.

  \sa ignore()
*/


/*!
  \fn void QIMEvent::ignore()
  Clears the accept flag parameter of the input method event object.

  Clearing the accept parameter indicates that the event receiver does
  not want the input method event.

  The accept flag is cleared by default.

  \sa accept()
*/

/*!
  \class QTabletEvent qevent.h
  \ingroup events
  \brief The QTabletEvent class contains parameters that describe a Tablet
   event.

  Tablet Events are generated from a Wacom(c) tablet.  Most of the
  time you will want to deal with events from the tablet as if they
  were events from a mouse, for example retrieving the position with
  x(), y(), pos(), globalX(), globalY() and globalPos().  In some
  situations you may wish to retrieve the extra information provided
  by the tablet device driver, for example, you might want to adjust
  color brightness based on pressure.  QTabletEvent allows you to get
  the pressure(), the xTilt() and yTilt(), as well as the type of
  device being used with device() (see \l{TabletDevice}).

  A tablet event contains a special accept flag that indicates whether the
  receiver wants the event.  You should call QTabletEvent::accept() if you
  handle the tablet event; otherwise it will be sent to the parent widget.

  The QWidget::setEnabled() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The event handler QWidget::tabletEvent() receives tablet events.

*/

/*!
  \enum QTabletEvent::TabletDevice

  This enum defines what type of device is generating the event.

  \value NoDevice    No device, or an unknown device.
  \value Puck    A Puck (a device that is similar to a flat mouse with
  a transparent circle with cross-hairs).
  \value Stylus  A Stylus (the narrow end of the pen).
  \value Eraser  An Eraser (the broad end of the pen).
  \omit
  \value Menu  A menu button was pressed (currently unimplemented).
*/

/*!
  \fn QTabletEvent::QTabletEvent( const QPoint &pos, int device, int pressure, int xTilt, int yTilt )

  Constructs a tablet event object.

  The globalPos() is initialized to QCursor::pos(), i.e. \a pos, which is
  usually (but not always) correct.  Use the other constructor if you need
  to specify the global position explicitly.  \a device contains the
  \link TabletDevice device type\endlink, \a pressure contains the
  pressure exerted on the \a device, \a xTilt and \a yTilt contain the
  \a device's degrees of tilt from the X and Y axis respectively.

  \sa pos(), device(), pressure(), xTilt(), yTilt()
*/

/*!
  \fn QTabletEvent::QTabletEvent( const QPoint &pos, const QPoint &globalPos, int device, int pressure, int xTilt, int yTilt )

  Constructs a tablet event object.  The position when the event
  occurred is is given in \a pos and \a globalPos.  \a device contains
  the \link TabletDevice device type\endlink, \a pressure contains the
  pressure exerted on the \a device, \a xTilt and \a yTilt contain the
  \a device's degrees of tilt from the X and Y axis respectively.

  On Irix, \a globalPos will contain the high-resolution coordinates
  received from the tablet device driver, instead of from the
  windowing system.

  \sa pos(), globalPos(), device(), pressure(), xTilt(), yTilt()
*/

/*!
  \fn TabletDevices QTabletEvent::device() const
  Returns the type of device that generated the event.  Useful if you
  want one end of the pen to do something different than the other.

  \sa TabletDevice
*/

/*!
  \fn int QTabletEvent::pressure() const
  Returns the pressure that is exerted on the device.  This number is
  a value from 0 (no pressure) to 255 (maximum pressure).  The
  pressure is always scaled to be within this range no matter how many
  pressure levels the underlying hardware supports.
*/

/*!
  \fn int QTabletEvent::xTilt() const
  Returns the difference from the perpendicular in the X Axis.
  Positive values are towards the tablet's physical right. The angle
  is in the range -60 to +60 degrees.

  \sa yTilt()
*/

/*!
  \fn int QTabletEvent::yTilt() const
  Returns the difference from the perpendicular in the Y Axis.  Positive values
  are towards the bottom of the tablet. The angle is within the range
  -60 to +60 degrees.

  \sa xTilt()
*/

/*!
  \fn const QPoint &QTabletEvent::pos() const
  Returns the position of the device, relative to the widget that received
  the event.

  If you move widgets around in response to mouse events, use globalPos()
  instead of this function.

  \sa x(), y(), globalPos()
*/

/*!
  \fn int QTabletEvent::x() const
  Returns the X position of the device, relative to the widget that received
  the event.

  \sa y(), pos()
*/

/*!
  \fn int QTabletEvent::y() const
  Returns the Y position of the device, relative to the widget that received
  the event.

  \sa x(), pos()
*/

/*!
  \fn const QPoint &QTabletEvent::globalPos() const

  Returns the global position of the device \e{at the time} of the event.
  This is important on asynchronous windows systems like X11; whenever you
  move your widgets around in response to mouse events, globalPos() can differ
  significantly from the current position QCursor::pos().

  \sa globalX(), globalY()
*/

/*!
  \fn int QTabletEvent::globalX() const
  Returns the global X position of the mouse pointer at the time of the event.

  \sa globalY(), globalPos()
*/

/*!
  \fn int QTabletEvent::globalY() const
  Returns the global Y position of the mouse pointer at the time of the event.

  \sa globalX(), globalPos()
*/

/*!
  \fn bool QTabletEvent::isAccepted() const
  Returns TRUE if the receiver of the event handles the wheel event;
  otherwise returns FALSE.
*/

/*!
  \fn void QTabletEvent::accept()
  Sets the accept flag of the tablet event object.

  Setting the accept flag indicates that the receiver of the event wants
  the tablet event.  Unwanted tablet events are sent to the parent widget.

  The accept flag is set by default.

  \sa ignore()
*/

/*!
  \fn void QTabletEvent::ignore()
  Clears the accept flag parameter of the wheel event object.

  Clearing the accept flag indicates that the event receiver does not
  want the tablet event.  Unwanted tablet events are sent to the parent widget.

  The accept flag is set by default.

  \sa accept()
*/



/*!
  \class QChildEvent qevent.h
  \ingroup events
  \brief The QChildEvent class contains event parameters for child object
  events.

  Child events are sent to objects when children are inserted or removed.

  A \c ChildRemoved event is sent immediately, but a \c ChildInserted event
  is \e posted (with QApplication::postEvent()).

  Note that if a child is removed immediately after it is inserted, the 
  \c ChildInserted event may be suppressed, but the \c ChildRemoved 
  event will always be sent. In this case there will be a \c ChildRemoved
  event without a corresponding \c ChildInserted event.

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
  Returns TRUE if the widget received a new child; otherwise returns FALSE.
*/

/*!
  \fn bool QChildEvent::removed() const
  Returns TRUE if the object lost a child; otherwise returns FALSE.
*/




/*!
  \class QCustomEvent qevent.h
  \ingroup events
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
  QApplication::postEvent( receiver, ce );  // Qt will delete it when done

  // To receive an event of this custom event type:

  void MyWidget::customEvent( QCustomEvent * e )
  {
    if ( e->type() == 346798 ) {  // It must be a ColorChangeEvent
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
  and the event is of type \a type.

  Do not create a QDragMoveEvent yourself since these objects rely on Qt's
  internal state.
*/

/*!
  \fn void   QDragMoveEvent::accept( const QRect & r )

  The same as accept(), but also notifies that future moves will
  also be acceptable if they remain within the rectangle \a r on the
  widget: this can improve performance, but may also be ignored by
  the underlying system.

  If the rectangle \link QRect::isEmpty() is empty\endlink, then drag
  move events will be sent continuously.  This is useful if the source is
  scrolling in a timer event.
*/

/*!
  \fn void   QDragMoveEvent::ignore( const QRect & r)

  The opposite of accept(const QRect&), i.e. says that moves within
  rectangle \a r are not acceptable (will be ignored).
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

  Returns TRUE if the drop target accepts the event; otherwise returns FALSE.
*/

/*!
  \fn void QDropEvent::accept(bool y=TRUE)

  \reimp

  Call this function to indicate whether the event provided data which
  your widget processed.  To get the data, use encodedData(), or
  preferably, the decode() methods of existing QDragObject subclasses,
  such as QTextDrag::decode(), or your own subclasses.

  \warning To accept or reject the drop, don't call this function,
  call acceptAction() instead. This function indicates whether you
  processed the event at all.

  \sa acceptAction()
*/

/*!
  \fn void QDropEvent::acceptAction(bool y=TRUE)

  Call this to indicate that the action described by action() is
  accepted (i.e. if \a y is TRUE which is the default), not merely the
  default copy action.  If you call acceptAction(TRUE), there is no
  need to also call accept(TRUE).
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

  This enum describes the action which a source requests that a target
  perform with dropped data.

   \value Copy  The default action.  The source simply uses the data
	    provided in the operation.
   \value Link  The source should somehow create a link to the location
	    specified by the data.
   \value Move  The source should somehow move the object from the location
	    specified by the data to a new location.
   \value Private  The target has special knowledge of the MIME type, which
	    the source should respond to in a similar way to a Copy.
   \value UserAction  The source and target can co-operate using special
	    actions.  This feature is not supported in Qt at this time.

  The Link and Move actions only makes sense if the data is
  a reference, for example, text/uri-list file lists (see QUriDrag).
*/

/*!
  \fn void QDropEvent::setAction( Action a )

  Sets the action to \a a.  This is used internally, you should not need to
  call this in your code - the \e source decides the action, not the
  target.
*/

/*!
  \fn Action QDropEvent::action() const

  Returns the Action which the target is requesting to be performed with
  the data.  If your application understands the action and can
  process the supplied data, call acceptAction(); if your application
  can process the supplied data but can only perform the Copy action,
  call accept().
*/

/*!
  \fn void QDropEvent::ignore()

  The opposite of accept(), i.e. you have ignored the drop event.
*/

/*! \fn bool QDropEvent::isActionAccepted () const

  Returns TRUE if the drop action was accepted by the drop site; otherwise
  returns FALSE.
*/


/*! \fn void QDropEvent::setPoint (const QPoint & np)

  Sets the drop to happen at point \a np.  You do not normally need to use
  this as it will be set internally before your widget receives the
  drop event.
*/ // ### here too - what coordinate system?


/*!
  \class QDragEnterEvent qevent.h
  \ingroup events
  \ingroup draganddrop
  \brief The QDragEnterEvent class provides an event which is sent to the widget when a drag and drop first drags onto the widget.

  This event is always immediately followed by a QDragMoveEvent, so you only
  need to respond to one or the other event.  This class inherits most of its
  functionality from QDragMoveEvent, which in turn inherits most of its
  functionality from QDropEvent.

  \sa QDragLeaveEvent, QDragMoveEvent, QDropEvent
*/

/*!
  \fn QDragEnterEvent::QDragEnterEvent (const QPoint & pos)
  Constructs a QDragEnterEvent entering at the given point, \a pos.

  Do not create a QDragEnterEvent yourself since these objects rely on Qt's
  internal state.
*/

/*!
  \class QDragLeaveEvent qevent.h
  \ingroup events
  \ingroup draganddrop
  \brief The QDragLeaveEvent class provides an event which is sent to the widget when a drag and drop leaves the widget.

  This event is always preceded by a QDragEnterEvent and a series
  of \l{QDragMoveEvent}s.  It is not sent if a QDropEvent is sent instead.

  \sa QDragEnterEvent, QDragMoveEvent, QDropEvent
*/

/*!
  \fn QDragLeaveEvent::QDragLeaveEvent()
  Constructs a QDragLeaveEvent.

  Do not create a QDragLeaveEvent yourself since these objects rely on Qt's
  internal state.
*/

/*!
  \class QHideEvent qevent.h
  \ingroup events
  \brief The QHideEvent class provides an event which is sent after a widget is hidden.

  This event is sent just before QWidget::hide() returns, and also when
  a top-level window has been hidden (iconified) by the user.

  If spontaneous() is TRUE the event originated outside the
  application, i.e. the user hid the window using the window manager
  controls, either by iconifying the window or by switching to another
  virtual desktop where the window isn't visible. The window will
  become hidden but not withdrawn.  If the window was iconified,
  QWidget::isMinimized() returns TRUE.


  \sa QShowEvent
*/

/*!
  \fn QHideEvent::QHideEvent()

  Constructs a QHideEvent.
*/

/*!
  \class QShowEvent qevent.h
  \ingroup events
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


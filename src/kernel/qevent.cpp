/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.cpp#92 $
**
** Implementation of event classes
**
** Created : 931029
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qevent.h"
#include "qcursor.h"
#include "qapplication.h"


/*!
  \class QEvent qevent.h
  \brief The QEvent class is base class of all
  event classes. Event objects contain event parameters.

  \ingroup event
  \ingroup kernel

  The \link QApplication::exec() main event loop\endlink of Qt fetches
  native window system events from the event queue, translates them
  into QEvent and sends the translated events to QObjects.

  Generally, events come from the underlying window system, but it is
  also possible to manually send events through the QApplication class
  using QApplication::sendEvent() and QApplication::postEvent().

  QObject received events by having its QObject::event() function
  called.  The default implementation simply calls
  QObject::timerEvent() for timer events and ignores all other events.
  QWidget reimplements \link QWidget::event() event() \endlink and
  dispatches incoming events to various event handles such as
  QWidget::keyPressEvent() and QWidget::mouseMoveEvent().

  The basic QEvent only contains an event type parameter.  Subclasses
  of QEvent contain additional parameters that describe the particular
  event.

  \sa QObject::event() QObject::installEventFilter() QWidget::event()
  QApplication::sendEvent() QApplcation::postEvent()
  QApplication::processEvents()
*/

/*!
  \fn QEvent::QEvent( Type type )
  Contructs an event object with a \a type. The file qevent.h lists
  all event types.
*/

/*!
  \fn QEvent::Type QEvent::type() const
  Returns the event type.
*/


/*!
  \class QTimerEvent qevent.h
  \brief The QTimerEvent class contains parameters that describe a
  timer event.

  \ingroup event


  Timer events are sent at regular intervals to objects that have
  started one or more timers.  Each timer has a unique identifier.

  If interval is 0, then the timer event occurs
  once every time there are no more window system events to process.

  The QTimer class provides a high-level programming interface with one-shot
  timers and timer signals instead of events.

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

  \ingroup event


  Mouse events occur when a mouse button is pressed or released inside a
  widget, or when the mouse cursor is moved.

  Mouse move events will only occur when some mouse button is pressed down,
  unless \link QWidget::setMouseTracking() mouse tracking\endlink has been
  enabled.

  Qt makes an automatic mouse grab when a mouse button is pressed inside a
  widget, and the widget will continue to receive mouse events until the
  last mouse button is released.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The QCursor widget has static functions for reading and setting the
  position of the mouse cursor.

  The event handlers QWidget::mousePressEvent(), QWidget::mouseReleaseEvent(),
  QWidget::mouseDoubleClickEvent() and QWidget::mouseMoveEvent() receive
  mouse events.

  \sa QWidget::setMouseTracking(), QWidget::grabMouse()
*/

/*!
  \fn QMouseEvent::QMouseEvent( Type type, const QPoint &pos, int button, int state )

  Constructs a mouse event object.

  The type parameter must be \c QEvent::MouseButtonPress,
  \c QEvent::MouseButtonRelease,
  \c QEvent::MouseButtonDblClick or \c QEvent::MouseMove.
*/

QMouseEvent::QMouseEvent( Type type, const QPoint &pos, int button, int state )
    : QEvent(type), p(pos), b(button),s((ushort)state){
	g = QCursor::pos();
}


/*!
  \fn QMouseEvent::QMouseEvent( Type type, const QPoint &pos, const QPoint &globalPos,  int button, int state )

  Constructs a mouse event object.

  The type parameter must be \c QEvent::MouseButtonPress,
  \c QEvent::MouseButtonRelease,
  \c QEvent::MouseButtonDblClick or \c QEvent::MouseMove.
*/

/*!
  \fn const QPoint &QMouseEvent::pos() const
  Returns the position of the mouse pointer, relative to the widget that
  received the event.
  \sa x(), y()
*/

/*!
  \fn const QPoint &QMouseEvent::globalPos() const

  Returns the global position of the mouse pointer \e at \e the \e
  time of the event. This is important on asynchronous window systems
  like X11: Whenever you move your widgets around in response to mouse
  evens, globalPos() can differ a lot from the current pointer
  position QCursor::pos().

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
  Returns the global X position of the mouse pointer at the time of the event
  \sa globalY(), globalPos()
*/

/*!
  \fn int QMouseEvent::globalY() const
  Returns the global Y position of the mouse pointer at the time of the event
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
  modifiers), i.e. what buttons and keys were being held depressed
  immediately before the event was generated.

  Note that this means that for \c QEvent::MouseButtonPress and \c
  QEvent::MouseButtonDblClick, the flag for the button() itself will not be
  set in the state; while for \c QEvent::MouseButtonRelease, it will.

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
    if ( type() == QEvent::MouseButtonDblClick ) {
	return Qt::ButtonState(state()&~button());
    } else {
	return Qt::ButtonState(state()^button());
    }
}



/*!
  \class QWheelEvent qevent.h
  \brief The QWheelEvent class contains parameters that describe a wheel event.

  \ingroup event


  Wheel events occur when a mouse wheel is turned while the pointer is above the widget.

  A wheel event contains a special accept flag which tells whether the
  receiver wants the event.  You should call QWheelEvent::accept() if you
  handle the wheel event, otherwise it will be sent to the parent widget as well.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The event handlers QWidget::wheelEvent() receive wheel events.

  \sa QMouseEvent, QWidget::grabMouse()
*/

/*!
  \fn QWheelEvent::QWheelEvent( const QPoint &pos, int delta, int state )

  Constructs a wheel event object.

*/

/*!
  \fn int QWheelEvent::delta() const

  Returns the distance that the wheel is rotated, expressed in
  multiples or divisions of WHEEL_DELTA, which is set at 120
  currently.A positive value indicates that the wheel was rotated
  forward, away from the user; a negative value indicates that the
  wheel was rotated backward, toward the user.

  The WHEEL_DELTA constant was set to 120 by the wheel mouse vendors
  to allow building finer-resolution wheels in the future, including
  perhaps a freely-rotating wheel with no notches. The expectation is
  that such a device would send more messages per rotation, but with a
  smaller value in each message.
*/

/*!
  \fn const QPoint &QWheelEvent::pos() const
  Returns the position of the mouse pointer, relative to the widget that
  received the event.
  \sa x(), y()
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
  \fn ButtonState QWheelEvent::state() const
  Returns the keyboard modifier flags.

  The returned value is \c ShiftButton, \c ControlButton and \c AltButton
  OR'ed together.
*/

/*!
  \fn bool QWheelEvent::isAccepted() const
  Returns TRUE if the receiver of the event handles the wheel event
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



/*!
  \class QKeyEvent qevent.h
  \brief The QKeyEvent class contains parameters that describe a key event.

  \ingroup event


  Key events occur when a key is pressed or released when a widget has
  keyboard input focus.

  A key event contains a special accept flag which tells whether the
  receiver wants the key.  You should call QKeyEvent::ignore() if the
  key press or release event is not handled by your widget.

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

  If \a key is 0, the event is not a result of a known key (e.g. it
  may be the result of a compose sequence or keyboard macro).

  \a text will be returned by text().

  If \a autorep is TRUE then isAutoRepeat() will be TRUE.

  \a count is the number of single keys.

  The accept flag is set to TRUE.
*/

/*!
  \fn int QKeyEvent::key() const
  Returns the code of the key that was pressed or released.

  The header file qnamespace.h lists the possible keyboard codes.	 These codes
  are independent of the underlying window system.

  Key code 0 means that the event is not a result of a known key (e.g. it
  may be the result of a compose sequence or keyboard macro).
*/

/*!
  \fn int QKeyEvent::ascii() const
  Returns the ASCII code of the key that was pressed or released.
  Internationalized software should use text() instead.

  \sa text()
*/

/*!
  \fn QString QKeyEvent::text() const
  Returns the Unicode text which this key generated.

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

  \sa state()
*/
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

/*!
  \fn bool QKeyEvent::isAutoRepeat() const
  Returns TRUE if the event was generated by the auto-repeat feature
  of the system.  Some applications will choose to ignore events of
  this type.
*/

/*!
  \fn int QKeyEvent::count() const

  Returns the number of single keys for this event. If text() is not
  empty, this is the length of the string. But Qt also compresses auto
  repeated key presses for non-visible keycodes, such as BackSpace or
  Delete. It is save to ignore count() and simply react on new key
  events, but if your application cannot keep up with the system's
  keyboard repeat rate, the users might feel it is a bit
  klundgy. Smarter applications therefore check count() and contain
  fancy code to optimize repetitive keysstrokes, even if the screen
  update might temporarily fell behind.
*/

/*!
  \fn void QKeyEvent::ignore()
  Clears the accept flag parameter of the key event object.

  Clearing the accept parameter indicates that the event receiver does
  not want the key event. Unwanted key events are sent to the parent widget.

  The accept flag is set by default.

  \sa accept()
*/


/*!
  \class QFocusEvent qevent.h
  \brief The QFocusEvent class contains event parameters for widget focus
  events.

  \ingroup event


  Focus events are sent to widgets when the keyboard input focus changes.

  The event handlers QWidget::focusInEvent() and QWidget::focusOutEvent()
  receive focus events.

  \sa QWidget::setFocus(), QWidget::setFocusEnabled()
*/

/*!
  \fn QFocusEvent::QFocusEvent( Type type )
  Constructs a focus event object.

  The \a type parameter must be either \a QEvent::FocusIn or \a QEvent::FocusOut.
*/

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

  \ingroup event


  Paint events are sent to widgets that need to update themselves, for instance
  when a part of a widget is exposed because an overlying widget is moved away.

  The event handler QWidget::paintEvent() receives paint events.

  \sa QPainter, QWidget::update(), QWidget::repaint()
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

  \ingroup event


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
  Returns the new position of the widget, which is the same as
  QWidget::pos().
*/

/*!
  \fn const QPoint &QMoveEvent::oldPos() const
  Returns the old position of the widget.
*/


/*!
  \class QResizeEvent qevent.h
  \brief The QResizeEvent class contains event parameters for resize events.

  \ingroup event


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

  \ingroup event


  Close events are sent to widgets that the user wants to close, usually
  by choosing "Close" from the window menu. They are also sent when you
  call QWidget::close() to close a widget from inside the program.

  Close events contain a special accept flag which tells whether the
  receiver wants the widget to be closed.  When a widget accepts the close
  event, it is \link QWidget::hide() hidden\endlink. If it refuses to
  accept the close event, nothing happens.

  The \link QApplication::setMainWidget() main widget\endlink of the
  application is a special case. When it accepts the close event, the
  application is immediately \link QApplication::quit()
  terminated\endlink.

  The event handler QWidget::closeEvent() receives close events.

  The default implementation of this event handler accepts the close event.
  This makes Qt \link QWidget::hide() hide\endlink the widget.

  \code
    void QWidget::closeEvent( QCloseEvent *e )
    {
	e->accept();				// hides the widget
    }
  \endcode

  If you do not want your widget to be hidden, you should reimplement the
  event handler.

  \code
    void MyWidget::closeEvent( QCloseEvent *e )
    {
	e->ignore();				// does not hide the widget
    }
  \endcode

  If you want your widget to be deleted when it is closed, simply delete
  it in the close event. In this case, calling QCloseEvent::accept() or
  QCloseEvent::ignore() makes no difference.

  \warning Be careful.  The code below assumes that the widget was created
  on the heap using the \c new operator. Even when the widget has been
  created by new doing this is a tricky operation. Be sure that you cannot
  have any other pointers to the widget hanging around.

  \code
    void MyWidget::closeEvent( QCloseEvent * )
    {
	delete this;
    }
  \endcode

  QObject emits the \link QObject::destroyed() destroyed()\endlink signal
  when it is deleted.  This is a useful signal if a widget needs to know
  when another widget is deleted.

  \sa QWidget::close(), QWidget::hide(), QObject::destroyed(),
  QApplication::setMainWidget(), QApplication::quit()
*/

/*!
  \fn QCloseEvent::QCloseEvent()
  Constructs a close event object with the accept parameter flag set to FALSE.
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

  The accept flag is not set by default.

  If you choose to accept in QWidget::closeEvent(), the widget will be
  hidden.

  \sa ignore(), QWidget::hide()
*/

/*!
  \fn void QCloseEvent::ignore()
  Clears the accept flag of the close event object.

  Clearing the accept flag indicates that the receiver of this event does not
  want the widget to be hidden.

  The accept flag is not set by default.

  \sa accept()
*/


/*!
  \class QChildEvent qevent.h
  \brief The QChildEvent class contains event parameters for child object
  events.

  \ingroup event


  Child events are sent to objects when children are inserted or removed.

  The handler for these events is QObject::childEvent().
*/

/*!
  \fn QChildEvent::QChildEvent( Type type, QObject *child )
  Constructs a child event object.

  The \a type parameter must be either \a QEvent::ChildInserted
  or \a QEvent::ChildRemoved.
*/

/*!
  \fn QObject *QChildEvent::child() const
  Returns the child widget inserted or removed.
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

  \ingroup event


  QCustomEvent is a user-defined event type which contains a \c void*.

  \warning
  This event class is internally used to implement Qt enhancements.  It is
  not advisable to use QCustomEvent in normal applications, where other
  event types and the signal/slot mechanism can do the job.
*/

/*!
  \fn QCustomEvent::QCustomEvent( Type type, void *data )
  Constructs a custom event object with the event type \a type and a
  pointer to \a data.
*/

/*!
  \fn void *QCustomEvent::data() const
  Returns a pointer to the event data (specified in the constructor).

  The event data can be anything and must be cast to something useful
  based on the \link QEvent::type() event type\endlink. Again, it is not
  recommended to use custom events unless you are implementing Qt kernel
  enhancements.
*/

/*!
  \fn QDragMoveEvent::QDragMoveEvent( const QPoint& pos )

  Creates a QDragMoveEvent for which the mouse is at point \a pos.

  Note that internal state is also involved with QDragMoveEvent,
  so it is not useful to create these yourself.
*/

/*!
  \fn QDragMoveEvent::QDragMoveEvent( const QPoint& pos, Type type )

  Creates a QDragMoveEvent for which the mouse is at point \a pos,
  and the given event \a type.

  Note that internal state is also involved with QDragMoveEvent,
  so it is not useful to create these yourself.
*/

/*!
  \fn const QPoint& QDragMoveEvent::pos() const

  Returns the position of the mouse when the event occurred.
*/

/*!
  \fn bool   QDragMoveEvent::isAccepted() const

  Returns TRUE if accept() has been called.
*/

/*!
  \fn void   QDragMoveEvent::accept(bool yes=TRUE)

  Call this to indicate that the event provides data which your widget
  can process.  Use provides(), or preferably, the canDecode() methods
  of existing QDragObject subclasses, such as
  QTextDrag::canDecode(), or your own subclasses.
*/

/*!
  \fn void   QDragMoveEvent::ignore()

  The opposite of accept().
*/

/*!
  \fn void   QDragMoveEvent::accept( const QRect & r )

  The same as accept() above, but also notifies that future moves will
  also be acceptable if they remain within the rectangle \a r on the
  widget - this can improve performance, but may also be ignored by
  the underlying system.
*/

/*!
  \fn void   QDragMoveEvent::ignore( const QRect & r)

  The opposite of accept(const QRect&).
*/

/*!
  \fn QRect  QDragMoveEvent::answerRect() const { return rect; }

  Returns the rectangle for which the acceptance of
  the move event applies.
*/



/*!
  \fn QDropEvent::QDropEvent( const QPoint& pos )

  Creates a QDropEvent for which the mouse is at point \a pos.

  Note that internal state is also involved with QDropEvent,
  so it is not useful to create these yourself.
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
  \fn void QDropEvent::accept()

  Call this to indicate that the event provided data which your widget
  processed.  Use data(), or preferably, the decode() methods
  of existing QDragObject subclasses, such as
  QTextDrag::decode(), or your own subclasses.
*/

/*!
  \fn void   QDropEvent::ignore()

  The opposite of accept().
*/

/*!
  \class QDragEnterEvent qevent.h
  \brief The event sent to widgets when a drag-and-drop first drags onto it.

  This event is always immediate followed by a QDragMoveEvent, thus you need
  only respond to one or the other event.  Note that this class inherits most
  of its functionality from QDragMoveEvent.

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
  \brief The event sent to widgets when a drag-and-drop leaves it.

  This event is always preceded by a QDragEnterEvent and a series
  of QDragMoveEvent.  It is not sent if a QDropEvent is sent instead.

  \sa QDragEnterEvent, QDragMoveEvent, QDropEvent
*/

/*!
  \fn QDragLeaveEvent::QDragLeaveEvent()
  Constructs a QDragLeaveEvent.
  Note that QDragLeaveEvent constructed outside of the Qt internals
  will not work - they currently rely on internal state.
*/

/*!
  \class QHideEvent qevent.h
  \brief The event sent after a widget is hidden.

  This event is sent just before QWidget::hide() returns, and also when
  a top-level window has been hidden (iconified) by the user.

  \sa QShowEvent
*/

/*!
  \fn QHideEvent::QHideEvent(bool spontaneous)

  Constructs a QHideEvent.  \a spontaneous is TRUE if the event
  originated outside the application - ie. the user hid the window via the
  window manager controls.
*/

/*!
  \fn bool QHideEvent::spontaneous () const
  Returns TRUE if the event
  originated outside the application - ie. the user hid the window via the
  window manager controls.
*/

/*!
  \class QShowEvent qevent.h
  \brief The event sent when a widget is shown.

  There are two kind of show events: spontaneous show events by the
  window system and internal show events. Spontaneous show events are
  sent just after the window system shows the window, including after
  a top-level window has been shown (un-iconified) by the
  user. Internal show events are delivered just before the widget
  becomes visible.

  \sa QHideEvent
*/

/*!
  \fn QShowEvent::QShowEvent(bool spontaneous)

  Constructs a QShowEvent.  \a spontaneous is TRUE if the event
  originated outside the application - ie. the user revealed the window via the
  window manager controls.
*/

/*!
  \fn bool QShowEvent::spontaneous () const
  Returns TRUE if the event
  originated outside the application - ie. the user revealed the window via the
  window manager controls.
*/


/*!
  \fn QByteArray QDropEvent::data(const char* f) const

  \obsolete

  Use QDropEvent::encodedData().
*/


/*!
  Destroys the event.  Reports an error if the event is \link
  QApplication::postEvent() posted \endlink for dispatch to an object.
*/

QEvent::~QEvent()
{
    if (posted)
	QApplication::removePostedEvent( this );
}

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qevent.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qdebug.h"
#include "qmime.h"
#include "qdnd_p.h"

/*!
    \class QInputEvent qevent.h
    \ingroup events

    \brief The QInputEvent class is the base class for events that
    describe user input.
*/

/*!
    \internal
*/
QInputEvent::QInputEvent(Type type, Qt::KeyboardModifiers modifiers)
    : QEvent(type), modState(modifiers)
{}

QInputEvent::~QInputEvent()
{
}



/*!
    \class QMouseEvent qevent.h
    \ingroup events

    \brief The QMouseEvent class contains parameters that describe a mouse event.

    Mouse events occur when a mouse button is pressed or released
    inside a widget, or when the mouse cursor is moved.

    Mouse move events will occur only when a mouse button is pressed
    down, unless mouse tracking has been enabled with
    QWidget::setMouseTracking().

    Qt automatically grabs the mouse when a mouse button is pressed
    inside a widget; the widget will continue to receive mouse events
    until the last mouse button is released.

    A mouse event contains a special accept flag that indicates
    whether the receiver wants the event. You should call ignore() if
    the mouse event is not handled by your widget. A mouse event is
    propagated up the parent widget chain until a widget accepts it
    with accept(), or an event filter consumes it.

    The functions pos(), x(), and y() give the cursor position
    relative to the widget that receives the mouse event. If you
    move the widget as a result of the mouse event, use the global
    position returned by globalPos() to avoid a shaking motion.

    The QWidget::setEnabled() function can be used to enable or
    disable mouse and keyboard events for a widget.

    The event handlers QWidget::mousePressEvent(),
    QWidget::mouseReleaseEvent(), QWidget::mouseDoubleClickEvent(),
    and QWidget::mouseMoveEvent() receive mouse events.

    \sa QWidget::setMouseTracking() QWidget::grabMouse()
    QCursor::pos()
*/

/*!
    Constructs a mouse event object.

    The \a type parameter must be one of \c QEvent::MouseButtonPress,
    \c QEvent::MouseButtonRelease, \c QEvent::MouseButtonDblClick,
    or \c QEvent::MouseMove.

    The \a position is the mouse cursor's position relative to the
    receiving widget.
    The \a button that caused the event is given as a value from
    the \l Qt::ButtonState enum. If the event \a type is
    \c MouseMove, the appropriate button for this event is
    \c Qt::NoButton (0).
    The \a state is the \link Qt::ButtonState Qt::ButtonState \endlink
    at the time of the event.

    The globalPos() is initialized to QCursor::pos(), which may not
    be appropriate. Use the other constructor to specify the global
    position explicitly.
*/

QMouseEvent::QMouseEvent(Type type, const QPoint &pos, Qt::MouseButton button,
                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
    : QInputEvent(type, modifiers), p(pos), b(button), mouseState(buttons)
{
    g = QCursor::pos();
}

QMouseEvent::~QMouseEvent()
{
}

#ifdef QT_COMPAT
QMouseEvent::QMouseEvent(Type type, const QPoint &pos, Qt::ButtonState button, int state)
    : QInputEvent(type), p(pos), b((Qt::MouseButton)button)
{
    g = QCursor::pos();
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & Qt::KeyButtonMask);
}

QMouseEvent::QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos,
                         Qt::ButtonState button, int state)
    : QInputEvent(type), p(pos), g(globalPos), b((Qt::MouseButton)button)
{
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & Qt::KeyButtonMask);
}
#endif


/*!
    Constructs a mouse event object.

    The \a type parameter must be \c QEvent::MouseButtonPress,
    \c QEvent::MouseButtonRelease, \c QEvent::MouseButtonDblClick,
    or \c QEvent::MouseMove.

    The \a pos is the mouse cursor's position relative to the
    receiving widget. The cursor's position in global coordinates is
    specified by \a globalPos.  The \a button that caused the event is
    given as a value from the \l Qt::MouseButton enum. If the event \a
    type is \c MouseMove, the appropriate button for this event is \c
    Qt::NoButton (0).  \a buttons is the state of all buttons at the
    time of the event, \a modifiers the state of all keyboard
    modifiers.

*/
QMouseEvent::QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos,
                         Qt::MouseButton button, Qt::MouseButtons buttons,
                         Qt::KeyboardModifiers modifiers)
    : QInputEvent(type, modifiers), p(pos), g(globalPos), b(button), mouseState(buttons)
{}

/*!
    \fn const QPoint &QMouseEvent::pos() const

    Returns the position of the mouse cursor, relative to the widget
    that received the event.

    If you move the widget as a result of the mouse event, use the
    global position returned by globalPos() to avoid a shaking
    motion.

    \sa x() y() globalPos()
*/

/*!
    \fn const QPoint &QMouseEvent::globalPos() const

    Returns the global position of the mouse cursor \e{at the time
    of the event}. This is important on asynchronous window systems
    like X11. Whenever you move your widgets around in response to
    mouse events, globalPos() may differ a lot from the current
    pointer position QCursor::pos(), and from
    QWidget::mapToGlobal(pos()).

    \sa globalX() globalY()
*/

/*!
    \fn int QMouseEvent::x() const

    Returns the x position of the mouse cursor, relative to the
    widget that received the event.

    \sa y() pos()
*/

/*!
    \fn int QMouseEvent::y() const

    Returns the y position of the mouse cursor, relative to the
    widget that received the event.

    \sa x() pos()
*/

/*!
    \fn int QMouseEvent::globalX() const

    Returns the global x position of the mouse cursor at the time of
    the event.

    \sa globalY() globalPos()
*/

/*!
    \fn int QMouseEvent::globalY() const

    Returns the global y position of the mouse cursor at the time of
    the event.

    \sa globalX() globalPos()
*/

/*!
    \fn Qt::MouseButton QMouseEvent::button() const

    Returns the button that caused the event.

    Possible return values are \c Qt::LeftButton, \c Qt::RightButton, \c
    Qt::MidButton, and \c Qt::NoButton.

    Note that the returned value is always \c Qt::NoButton for mouse move
    events.

    \sa buttons() Qt::MouseButton
*/

/*!
    \fn Qt::MouseButton QMouseEvent::buttons() const

    Returns the button state when the event was generated. The button
    state is a combination of \c Qt::LeftButton, \c Qt::RightButton,
    \c Qt::MidButton using the OR operator. For mouse move events,
    this is all buttons that are pressed down. For mouse press and
    double click events this includes the button that caused the
    event. For mouse release events this excludes the button that
    caused the event.

    \sa button() Qt::MouseButton
*/


/*!\obsolete
    \fn Qt::ButtonState QMouseEvent::state() const

    Returns the button state immediately before the event was
    generated. The button state is a combination of mouse buttons
    (see Qt::ButtonState) and keyboard modifiers (Qt::Modifier).

    For example, for \c QEvent::MouseButtonPress and
    \c QEvent::MouseButtonDblClick event types, state() will
    \e not include the mouse button that's pressed.
    When the mouse button is released, the
    \c QEvent::MouseButtonRelease event has a state() that
    contains the button() that was initially pressed.

    This value is mainly interesting for \c QEvent::MouseMove;
    for the other cases, button() is more useful.

    The returned value is a selection of the following values,
    combined using the OR operator:
    \c Qt::LeftButton, \c Qt::RightButton, \c Qt::MidButton,
    \c Qt::ShiftButton, \c Qt::ControlButton, and \c Qt::AltButton.

    \sa button() stateAfter() Qt::ButtonState
*/


/*!
    \class QWheelEvent qevent.h
    \brief The QWheelEvent class contains parameters that describe a wheel event.

    \ingroup events

    Wheel events are sent to the widget under the mouse cursor, but
    if that widget does not handle the event they are sent to the
    focus widget. The rotation distance is provided by delta().
    The functions pos() and globalPos() return the mouse cursor's
    location at the time of the event.

    A wheel event contains a special accept flag that indicates
    whether the receiver wants the event. You should call ignore() if
    you do not handle the wheel event; this ensures that it will be
    sent to the parent widget.

    The QWidget::setEnable() function can be used to enable or disable
    mouse and keyboard events for a widget.

    The event handler QWidget::wheelEvent() receives wheel events.

    \sa QMouseEvent QWidget::grabMouse()
*/

/*!
    \fn Qt::Orientation QWheelEvent::orientation() const

    Returns the wheel's orientation.
*/

/*!
    Constructs a wheel event object.

    The position, \a pos, is the location of the mouse cursor within
    the widget. The globalPos() is initialized to QCursor::pos()
    which is usually, but not always, correct.
    Use the other constructor if you need to specify the global
    position explicitly. \a delta contains the rotation distance,
    \a modifiers holds the keyboard modifier flags at the time of the
    event, and \a orient holds the wheel's orientation.

    \sa pos() delta() state()
*/
#ifndef QT_NO_WHEELEVENT
QWheelEvent::QWheelEvent(const QPoint &pos, int delta,
                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                         Qt::Orientation orient)
    : QInputEvent(Wheel, modifiers), p(pos), d(delta), mouseState(buttons), o(orient)
{
    g = QCursor::pos();
}

QWheelEvent::~QWheelEvent()
{
}

#ifdef QT_COMPAT
QWheelEvent::QWheelEvent(const QPoint &pos, int delta, int state, Qt::Orientation orient)
    : QInputEvent(Wheel), p(pos), d(delta), o(orient)
{
    g = QCursor::pos();
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & Qt::KeyButtonMask);
}
#endif

/*!
    Constructs a wheel event object.

    The \a pos provides the location of the mouse cursor
    within the widget. The position in global coordinates is specified
    by \a globalPos. \a delta contains the rotation distance, \a modifiers
    holds the keyboard modifier flags at the time of the event, and
    \a orient holds the wheel's orientation.

    \sa pos() globalPos() delta() state()
*/
QWheelEvent::QWheelEvent(const QPoint &pos, const QPoint& globalPos, int delta,
                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                         Qt::Orientation orient)
    : QInputEvent(Wheel, modifiers), p(pos), g(globalPos), d(delta), mouseState(buttons), o(orient)
{}

#ifdef QT_COMPAT
QWheelEvent::QWheelEvent(const QPoint &pos, const QPoint& globalPos, int delta, int state,
                         Qt::Orientation orient)
    : QInputEvent(Wheel), p(pos), g(globalPos), d(delta), o(orient)
{
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & Qt::KeyButtonMask);
}
#endif
#endif // QT_NO_WHEELEVENT

/*!
    \fn int QWheelEvent::delta() const

    Returns the distance that the wheel is rotated, given in
    multiples or divisions of \c WHEEL_DELTA. A positive value
    indicates that the wheel was rotated forwards away from the
    user; a negative value indicates that the wheel was rotated
    backwards toward the user.

    The \c WHEEL_DELTA constant was defined to be 120 by the wheel
    mouse vendors to allow finer-resolution wheels to be built in
    the future, such as a freely rotating wheel with no notches.
    The expectation is that such a device would send more messages
    per rotation, but with a smaller value in each message.
*/

/*!
    \fn const QPoint &QWheelEvent::pos() const

    Returns the position of the mouse cursor relative to the widget
    that received the event.

    If you move your widgets around in response to mouse events,
    use globalPos() instead of this function.

    \sa x() y() globalPos()
*/

/*!
    \fn int QWheelEvent::x() const

    Returns the x position of the mouse cursor, relative to the
    widget that received the event.

    \sa y() pos()
*/

/*!
    \fn int QWheelEvent::y() const

    Returns the y position of the mouse cursor, relative to the
    widget that received the event.

    \sa x() pos()
*/


/*!
    \fn const QPoint &QWheelEvent::globalPos() const

    Returns the global position of the mouse pointer \e{at the time
    of the event}. This is important on asynchronous window systems
    such as X11; whenever you move your widgets around in response to
    mouse events, globalPos() can differ a lot from the current
    cursor position returned by QCursor::pos().

    \sa globalX() globalY()
*/

/*!
    \fn int QWheelEvent::globalX() const

    Returns the global x position of the mouse cursor at the time of
    the event.

    \sa globalY() globalPos()
*/

/*!
    \fn int QWheelEvent::globalY() const

    Returns the global y position of the mouse cursor at the time of
    the event.

    \sa globalX() globalPos()
*/


/*!\obsolete
    \fn Qt::ButtonState QWheelEvent::state() const

    Returns the keyboard modifier flags at the time of the event.

    The returned value is a selection of the following values,
    combined using the OR operator:
    \c Qt::ShiftButton, \c Qt::ControlButton, and \c Qt::AltButton.
*/


/*!
    \class QKeyEvent qevent.h
    \brief The QKeyEvent class contains describes a key event.

    \ingroup events

    Key events are sent to the widget with keyboard input focus
    when keys are pressed or released.

    A key event contains a special accept flag that indicates whether
    the receiver will handle the key event. You should call ignore()
    if the key press or release event is not handled by your widget.
    A key event is propagated up the parent widget chain until a
    widget accepts it with accept() or an event filter consumes it.
    Key events for multimedia keys are ignored by default. You should
    call accept() if your widget handles those events.

    The QWidget::setEnable() function can be used to enable or disable
    mouse and keyboard events for a widget.

    The event handlers QWidget::keyPressEvent() and
    QWidget::keyReleaseEvent() receive key events.

    \sa QFocusEvent, QWidget::grabKeyboard()
*/

/*!
    Constructs a key event object.

    The \a type parameter must be \c QEvent::KeyPress or \c
    QEvent::KeyRelease. If \a key is 0, the event is not a result of
    a known key; for example, it may be the result of a compose
    sequence or keyboard macro. The \a modifiers holds the keyboard
    modifiers, and the given \a text is the Unicode text that the
    key generated. If \a autorep is true, isAutoRepeat() will be
    true. \a count is the number of keys involved in the event.
*/
QKeyEvent::QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers, const QString& text,
                     bool autorep, ushort count)
    : QInputEvent(type, modifiers), txt(text), k(key), c(count), autor(autorep)
{
    if(key >= Qt::Key_Back && key <= Qt::Key_MediaLast)
        ignore();
}

QKeyEvent::~QKeyEvent()
{
}

/*!
    \fn int QKeyEvent::key() const

    Returns the code of the key that was pressed or released.

    See \l Qt::Key for the list of keyboard codes. These codes are
    independent of the underlying window system.

    A value of either 0 or Qt::Key_unknown means that the event is not
    the result of a known key; for example, it may be the result of
    a compose sequence, a keyboard macro, or due to key event
    compression.

    \sa Qt::WA_KeyCompression
*/

/*!
    \fn QString QKeyEvent::text() const

    Returns the Unicode text that this key generated. The text
    returned can be a null string (text().isNull == true) in cases
    where modifier keys, such as Shift, Control, Alt, and Meta,
    are being pressed or released. In such cases key() will contain
    a valid value.

    \sa Qt::WA_KeyCompression
*/

/*!
    Returns the keyboard modifier flags that existed immediately
    after the event occurred.

    \warning This function cannot be trusted.

    \sa QApplication::keyboardModifiers()
*/
//###### We must check with XGetModifierMapping
Qt::KeyboardModifiers QKeyEvent::modifiers() const
{
    if (key() == Qt::Key_Shift)
        return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::ShiftModifier);
    if (key() == Qt::Key_Control)
        return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::ControlModifier);
    if (key() == Qt::Key_Alt)
        return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::AltModifier);
    if (key() == Qt::Key_Meta)
        return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::MetaModifier);
    return QInputEvent::modifiers();
}

/*!
    \fn bool QKeyEvent::isAutoRepeat() const

    Returns true if this event comes from an auto-repeating key;
    returns false if it comes from an initial key press.

    Note that if the event is a multiple-key compressed event that is
    partly due to auto-repeat, this function could return either true
    or false indeterminately.
*/

/*!
    \fn int QKeyEvent::count() const

    Returns the number of keys involved in this event. If text()
    is not empty, this is simply the length of the string.

    \sa Qt::WA_KeyCompression
*/


/*!
    \class QFocusEvent qevent.h
    \brief The QFocusEvent class contains event parameters for widget focus
    events.

    \ingroup events

    Focus events are sent to widgets when the keyboard input focus
    changes. Focus events occur due to mouse actions, key presses
    (such as Tab or Backtab), the window system, popup menus,
    keyboard shortcuts, or other application-specific reasons.
    The reason for a particular focus event is returned by reason()
    in the appropriate event handler.

    The event handlers QWidget::focusInEvent() and
    QWidget::focusOutEvent() receive focus events.

    Use setReason() to set the reason for all focus events, and
    resetReason() to reset the reason for all focus events to the
    previously defined reason before the last setReason() call.

    \sa QWidget::setFocus(), QWidget::setFocusPolicy()
*/

/*!
    Constructs a focus event object.

    The \a type parameter must be either \c QEvent::FocusIn or \c
    QEvent::FocusOut.
*/
QFocusEvent::QFocusEvent(Type type)
    : QEvent(type)
{}

QFocusEvent::~QFocusEvent()
{
}


QFocusEvent::Reason QFocusEvent::m_reason = QFocusEvent::Other;
QFocusEvent::Reason QFocusEvent::prev_reason = QFocusEvent::Other;


/*!
    \enum QFocusEvent::Reason

    This enum specifies why the focus changed.

    \value Mouse         A mouse action occurred.
    \value Tab           The Tab key was pressed.
    \value Backtab       A Backtab occurred. The input for this may
                         include the Shift or Control keys;
                         e.g. Shift+Tab.
    \value ActiveWindow  The window system made this window either
                         active or inactive.
    \value Popup         The application opened/closed a popup that
                         grabbed/released the keyboard focus.
    \value Shortcut      A keyboard shortcut was input.
    \value Other         Another reason, usually application-specific.

    See the \link focus.html keyboard focus overview \endlink for more
    about the keyboard focus.
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
void QFocusEvent::setReason(Reason reason)
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

    Returns true if the widget received the text input focus;
    otherwise returns false.
*/

/*!
    \fn bool QFocusEvent::lostFocus() const

    Returns true if the widget lost the text input focus; otherwise
    returns false.
*/


/*!
    \class QPaintEvent qevent.h
    \brief The QPaintEvent class contains event parameters for paint events.

    \ingroup events

    Paint events are sent to widgets that need to update themselves,
    for instance when part of a widget is exposed because a covering
    widget was moved.

    The event contains a region() that needs to be updated, and a
    rect() that is the bounding rectangle of that region. Both are
    provided because many widgets can't make much use of region(),
    and rect() can be much faster than region().boundingRect().
    Painting is clipped to region() during the processing of a paint
    event.

    \sa QPainter QWidget::update() QWidget::repaint()
    QWidget::paintEvent() QRegion
*/

/*!
    Constructs a paint event object with the region that needs to
    be updated. The region is specified by \a paintRegion.
*/
QPaintEvent::QPaintEvent(const QRegion& paintRegion)
    : QEvent(Paint), m_rect(paintRegion.boundingRect()), m_region(paintRegion)
{}

/*!
    Constructs a paint event object with the rectangle that needs
    to be updated. The region is specified by \a paintRect.
*/
QPaintEvent::QPaintEvent(const QRect &paintRect)
    : QEvent(Paint), m_rect(paintRect),m_region(paintRect)
{}

/*!
    Constructs a paint event object with both a \a paintRegion and a
    \a paintRect, both of which represent the area of the widget that
    needs to be updated.

*/
QPaintEvent::QPaintEvent(const QRegion &paintRegion, const QRect &paintRect)
    : QEvent(Paint), m_rect(paintRect), m_region(paintRegion)
{}


QPaintEvent::~QPaintEvent()
{
}

/*!
    \fn const QRect &QPaintEvent::rect() const

    Returns the rectangle that needs to be updated.

    \sa region() QPainter::setClipRect()
*/

/*!
    \fn const QRegion &QPaintEvent::region() const

    Returns the region that needs to be updated.

    \sa rect() QPainter::setClipRegion()
*/


#ifdef Q_WS_QWS
QWSUpdateEvent::QWSUpdateEvent(const QRegion& paintRegion)
    : QPaintEvent(paintRegion)
{ t = QWSUpdate; }

QWSUpdateEvent::QWSUpdateEvent(const QRect &paintRect)
    : QPaintEvent(paintRect)
{ t = QWSUpdate; }

QWSUpdateEvent::~QWSUpdateEvent()
{
}
#endif


/*!
    \class QMoveEvent qevent.h
    \brief The QMoveEvent class contains event parameters for move events.

    \ingroup events

    Move events are sent to widgets that have been moved to a new
    position relative to their parent.

    The event handler QWidget::moveEvent() receives move events.

    \sa QWidget::move(), QWidget::setGeometry()
*/

/*!
    Constructs a move event with the new and old widget positions,
    \a pos and \a oldPos respectively.
*/
QMoveEvent::QMoveEvent(const QPoint &pos, const QPoint &oldPos)
    : QEvent(Move), p(pos), oldp(oldPos)
{}

QMoveEvent::~QMoveEvent()
{
}

/*!
    \fn const QPoint &QMoveEvent::pos() const

    Returns the new position of the widget. This excludes the window
    frame for top level widgets.
*/

/*!
    \fn const QPoint &QMoveEvent::oldPos() const

    Returns the old position of the widget.
*/


/*!
    \class QResizeEvent qevent.h
    \brief The QResizeEvent class contains event parameters for resize events.

    \ingroup events

    Resize events are sent to widgets that have been resized.

    The event handler QWidget::resizeEvent() receives resize events.

    \sa QWidget::resize() QWidget::setGeometry()
*/

/*!
    Constructs a resize event with the new and old widget sizes, \a
    size and \a oldSize respectively.
*/
QResizeEvent::QResizeEvent(const QSize &size, const QSize &oldSize)
    : QEvent(Resize), s(size), olds(oldSize)
{}

QResizeEvent::~QResizeEvent()
{
}

/*!
    \fn const QSize &QResizeEvent::size() const

    Returns the new size of the widget. This is the same as
    QWidget::size().
*/

/*!
    \fn const QSize &QResizeEvent::oldSize() const

    Returns the old size of the widget.
*/


/*!
    \class QCloseEvent qevent.h
    \brief The QCloseEvent class contains parameters that describe a close event.

    \ingroup events

    Close events are sent to widgets that the user wants to close,
    usually by choosing "Close" from the window menu, or by clicking
    the `X' title bar button. They are also sent when you call
    QWidget::close() to close a widget programmatically.

    Close events contain a flag that indicates whether the receiver
    wants the widget to be closed or not. When a widget accepts the
    close event, it is hidden (and destroyed if it was created with
    the \c Qt::WA_DeleteOnClose flag). If it refuses to accept the close
    event nothing happens. (Under X11 it is possible that the window
    manager will forcibly close the window; but at the time of writing
    we are not aware of any window manager that does this.)

    The application's main widget -- QApplication::mainWidget() --
    is a special case. When it accepts the close event, Qt leaves the
    main event loop and the application is immediately terminated
    (i.e. it returns from the call to QApplication::exec() in the
    main() function).

    The event handler QWidget::closeEvent() receives close events. The
    default implementation of this event handler accepts the close
    event. If you do not want your widget to be hidden, or want some
    special handing, you should reimplement the event handler and
    ignore() the event.

    The \link simple-application.html#closeEvent closeEvent() in the
    Application Walkthrough\endlink shows a close event handler that
    asks whether to save a document before closing.

    If you want the widget to be deleted when it is closed, create it
    with the \c Qt::WA_DeleteOnClose flag. This is very useful for
    independent top-level windows in a multi-window application.

    \l{QObject}s emits the \link QObject::destroyed()
    destroyed()\endlink signal when they are deleted.

    If the last top-level window is closed, the
    QApplication::lastWindowClosed() signal is emitted.

    The isAccepted() function returns true if the event's receiver has
    agreed to close the widget; call accept() to agree to close the
    widget and call ignore() if the receiver of this event does not
    want the widget to be closed.

    \sa QWidget::close(), QWidget::hide(), QObject::destroyed(),
    QApplication::setMainWidget(), QApplication::lastWindowClosed(),
    QApplication::exec(), QApplication::quit()
*/

/*!
    Constructs a close event object.

    \sa accept()
*/
QCloseEvent::QCloseEvent()
    : QEvent(Close)
{}

QCloseEvent::~QCloseEvent()
{
}

/*!
   \class QIconDragEvent qevent.h
   \brief The QIconDragEvent class indicates that a main icon drag has begun.

   \ingroup events

   Icon drag events are sent to widgets when the main icon of a window
   has been dragged away. On Mac OS X, this happens when the proxy
   icon of a window is dragged off the title bar.

   It is normal to begin using drag and drop in response to this
   event.

   \sa \link dnd.html Drag-and-drop documentation\endlink QMimeData QDrag
*/

/*!
    Constructs an icon drag event object with the accept flag set to
    false.

    \sa accept()
*/
QIconDragEvent::QIconDragEvent()
    : QEvent(IconDrag)
{ ignore(); }

QIconDragEvent::~QIconDragEvent()
{
}

/*!
    \fn bool QIconDragEvent::isAccepted() const

    Returns true if the receiver of the event has started a drag and
    drop operation; otherwise returns false.

    \sa accept(), ignore()
*/

/*!
    \fn void QIconDragEvent::accept()

    Sets the accept flag of the icon drag event object.

    Setting the accept flag indicates that the receiver of this event
    has started a drag and drop operation.

    By default, the accept flag is \e not set.

    \sa ignore(), QWidget::hide()
*/

/*!
    \fn void QIconDragEvent::ignore()

    Clears the accept flag of the icon drag object.

    Clearing the accept flag indicates that the receiver of this event
    has not handled the icon drag.

    By default, the accept flag is \e not set.

    \sa accept()
*/

/*!
    \class QContextMenuEvent qevent.h
    \brief The QContextMenuEvent class contains parameters that describe a context menu event.

    \ingroup events

    Context menu events are sent to widgets when a user performs
    an action associated with opening a context menu.
    The actions required to open context menus vary between platforms;
    for example, on Windows, pressing the menu button or clicking the
    right mouse button will cause this event to be sent.

    When this event occurs it is customary to show a QMenu with a
    context menu, if this is relevant to the context.

    Context menu events contain a special accept flag that indicates
    whether the receiver accepted the event. If the event handler does
    not accept the event then, if possible, whatever triggered the event will be
    handled as a regular input event.
*/

/*!
    \fn QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos)

    Constructs a context menu event object with the accept parameter
    flag set to false.

    The \a reason parameter must be \c QContextMenuEvent::Mouse or
    \c QContextMenuEvent::Keyboard.

    The \a pos parameter specifies the mouse position relative to the
    receiving widget. \a globalPos is the mouse position in absolute
    coordinates.
*/
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos)
    : QInputEvent(ContextMenu), p(pos), gp(globalPos), reas(reason)
{}

#ifdef QT_COMPAT
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos,
                                     int)
    : QInputEvent(ContextMenu), p(pos), gp(globalPos), reas(reason)
{}
#endif

QContextMenuEvent::~QContextMenuEvent()
{
}
/*!
    Constructs a context menu event object with the accept parameter
    flag set to false.

    The \a reason parameter must be \c QContextMenuEvent::Mouse or \c
    QContextMenuEvent::Keyboard.

    The \a pos parameter specifies the mouse position relative to the
    receiving widget.

    The globalPos() is initialized to QCursor::pos(), which may not be
    appropriate. Use the other constructor to specify the global
    position explicitly.
*/
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos)
    : QInputEvent(ContextMenu), p(pos), reas(reason)
{
    gp = QCursor::pos();
}

#ifdef QT_COMPAT
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, int)
    : QInputEvent(ContextMenu), p(pos), reas(reason)
{
    gp = QCursor::pos();
}

Qt::ButtonState QContextMenuEvent::state() const
{
    return Qt::ButtonState(int(QApplication::keyboardModifiers())|QApplication::mouseButtons());
}
#endif

/*!
    \fn const QPoint &QContextMenuEvent::pos() const

    Returns the position of the mouse pointer relative to the widget
    that received the event.

    \sa x(), y(), globalPos()
*/

/*!
    \fn int QContextMenuEvent::x() const

    Returns the x position of the mouse pointer, relative to the
    widget that received the event.

    \sa y(), pos()
*/

/*!
    \fn int QContextMenuEvent::y() const

    Returns the y position of the mouse pointer, relative to the
    widget that received the event.

    \sa x(), pos()
*/

/*!
    \fn const QPoint &QContextMenuEvent::globalPos() const

    Returns the global position of the mouse pointer at the time of
    the event.

    \sa x(), y(), pos()
*/

/*!
    \fn int QContextMenuEvent::globalX() const

    Returns the global x position of the mouse pointer at the time of
    the event.

    \sa globalY(), globalPos()
*/

/*!
    \fn int QContextMenuEvent::globalY() const

    Returns the global y position of the mouse pointer at the time of
    the event.

    \sa globalX(), globalPos()
*/

/*!
    \fn Qt::ButtonState QContextMenuEvent::state() const

    Returns the button state (a combination of mouse buttons
    and keyboard modifiers) immediately before the event was
    generated.

    The returned value is a selection of the following values,
    combined with the OR operator:
    \c Qt::LeftButton, \c Qt::RightButton, \c Qt::MidButton,
    \c Qt::ShiftButton, \c Qt::ControlButton, and \c Qt::AltButton.
*/

/*!
    \enum QContextMenuEvent::Reason

    This enum describes the reason why the event was sent.

    \value Mouse The mouse caused the event to be sent. Normally this
    means the right mouse button was clicked, but this is platform
    dependent.

    \value Keyboard The keyboard caused this event to be sent. On
    Windows, this means the menu button was pressed.

    \value Other The event was sent by some other means (i.e. not by
    the mouse or keyboard).
*/


/*!
    \fn QContextMenuEvent::Reason QContextMenuEvent::reason() const

    Returns the reason for this context event.
*/


/*!
    \class QInputMethodEvent qevent.h
    \brief The QInputMethodEvent class provides parameters for input method events.

    \ingroup events

    Input method events are sent to widgets when an input method is
    used to enter text into a widget. Input methods are widely used
    to enter text for languages with non-Latin alphabets.

    The events are of interest to authors of keyboard entry widgets
    who want to be able to correctly handle languages with complex
    character input. Text input in such languages is usually a three
    step process.

    \list 1
    \i <b>Starting to Compose</b><br>
    When the user presses the first key on a keyboard, an input
    context is created. This input context will contain a string
    of the typed characters.

    \i <b>Composing</b><br>
    With every new key pressed, the input method will try to create a
    matching string for the text typed so far. While the input context
    is active, the user can only move the cursor inside the string
    belonging to this input context.

    \i <b>Completing</b><br>
    At some point, the user will activate a user interface component
    (perhaps using a particular key) where they can choose from a
    number of strings matching the text they have typed so far. The
    user can either confirm their choice cancel the input; in either
    case the input context will be closed.
    \endlist

    These three stages are represented by three different types of
    events: InputMethodStart, InputMethodCompose, and InputMethodEnd. All of these events are
    delivered to the imEvent() method. When a new input context is
    created, an InputMethodStart event will be sent to the widget.  The widget
    can then update internal data structures to reflect this.

    After this, an InputMethodCompose event will be sent to the widget for
    every key the user presses. It will contain the current
    composition string the widget has to show and the current cursor
    position within the composition string. This string is temporary
    and can change with every key the user types, so the widget will
    need to store the state before the composition started (the state
    it had when it received the InputMethodStart event).

    Usually, widgets try to mark the part of the text that is part of
    the current composition in a way that is visible to the user. A
    commonly used visual cue is to use a dotted underline.

    After the user has selected the final string, an InputMethodEnd event will
    be sent to the widget. The event contains the final string the
    user selected, and could be empty if they canceled the
    composition. This string should be accepted as the final text the
    user entered, and the intermediate composition string should be
    cleared.

    If the user clicks another widget, taking the focus out of the
    widget where the composition is taking place, the InputMethodEnd event will
    be sent and the string it holds will be the result of the
    composition up to that point (which may be an empty string).

*/

/*!
    Constructs a new QInputMethodEvent with the accept flag set to false.  The
    event \a type can be QEvent::InputMethodStart, QEvent::InputMethodCompose, or
    QEvent::InputMethodEnd. The \a text contains the current compostion string,
    and \a cursorPosition is the current position of the cursor inside
    the \a text.  \a selLength characters are selected (default is 0).
*/
QInputMethodEvent::QInputMethodEvent(Type type, const QString &text, int cursorPosition, int selLength)
    : QInputEvent(type), txt(text), cpos(cursorPosition), selLen(selLength)
{}

QInputMethodEvent::~QInputMethodEvent()
{
}

/*!
    \fn const QString &QInputMethodEvent::text() const

    Returns the composition text. This is a null string for an
    InputMethodStart event, and contains the final accepted string (which may be
    empty) in the InputMethodEnd event.
*/

/*!
    \fn int QInputMethodEvent::cursorPos() const

    Returns the current cursor position inside the composition string.
    Will return -1 for InputMethodStart event and InputMethodEnd event.
*/

/*!
    \fn int QInputMethodEvent::selectionLength() const

    Returns the number of characters in the composition string,
    starting at cursorPos(), that should be marked as selected by the
    input widget receiving the event.
    Will return 0 for InputMethodStart event and InputMethodEnd event.
*/


/*!
    \class QTabletEvent qevent.h
    \brief The QTabletEvent class contains parameters that describe a Tablet event.

    \ingroup events

    Tablet Events are generated from a Wacom tablet. Most of
    the time you will want to deal with events from the tablet as if
    they were events from a mouse; for example, you would retrieve the
    cursor position with x(), y(), pos(), globalX(), globalY(), and
    globalPos(). In some situations you may wish to retrieve the extra
    information provided by the tablet device driver; for example, you
    might want to adjust color brightness based on pressure.
    QTabletEvent allows you to read the pressure(), the xTilt(), and
    yTilt(), as well as the type of device being used with device()
    (see \l{TabletDevice}).

    A tablet event contains a special accept flag that indicates
    whether the receiver wants the event. You should call
    QTabletEvent::accept() if you handle the tablet event; otherwise
    it will be sent to the parent widget.

    The QWidget::setEnabled() function can be used to enable or
    disable mouse and keyboard events for a widget.

    The event handler QWidget::tabletEvent() receives all three types of
    tablet events. Qt will first send a tabletEvent then, if it is not
    accepted, it will send a mouse event.  This allows applications that
    don't utilize tablets to use a tablet like a mouse, while also
    enabling those who want to use both tablets and mouses differently.

*/

/*!
    \enum QTabletEvent::TabletDevice

    This enum defines what type of device is generating the event.

    \value NoDevice    No device, or an unknown device.
    \value Puck    A Puck (a device that is similar to a flat mouse with
    a transparent circle with cross-hairs).
    \value Stylus  A Stylus (the narrow end of the pen).
    \value Eraser  An Eraser (the broad end of the pen).
*/

/*!
  \fn QTabletEvent::QTabletEvent(Type type, const QPoint &position,
                                  const QPoint &globalPos, int device,
                                  int pressure, int xTilt, int yTilt,
                                  Q_LONGLONG &uId)

  Construct a tablet event of the given \a type. The \a position
  indicates where the event occurred in the widget; \a globalPos is
  the corresponding position in absolute coordinates. The \a device
  contains the \link TabletDevice device type \endlink; \a pressure
  contains the pressure exerted on the \a device; \a xTilt and \a
  yTilt contain the device's degree of tilt from the X and Y axes
  respectively. The \a uId contains an event ID.

  On Irix, \a globalPos will contain the high-resolution coordinates
  received from the tablet device driver, instead of from the
  windowing system.

  \sa pos() globalPos() device() pressure() xTilt() yTilt() uniqueId()
*/

QTabletEvent::QTabletEvent(Type t, const QPoint &pos, const QPoint &globalPos, const QPoint &hiResPos,
                  int minX, int maxX, int minY, int maxY, int device,
                  int pressure, int minPressure, int maxPressure, int xTilt, int yTilt,
                  Qt::KeyboardModifiers keyState, Q_LONGLONG unique)
    : QInputEvent(t, keyState),
      mPos(pos),
      mGPos(globalPos),
      mHiResPos(hiResPos),
      mHiResMinX(minX),
      mHiResMaxX(maxX),
      mHiResMinY(minY),
      mHiResMaxY(maxY),
      mDev(device),
      mPress(pressure),
      mXT(xTilt),
      mYT(yTilt),
      mMinPressure(minPressure),
      mMaxPressure(maxPressure),
      mUnique(unique)
{
}

QTabletEvent::~QTabletEvent()
{
}

/*!
    \fn TabletDevices QTabletEvent::device() const

    Returns the type of device that generated the event.

    This is useful if you want to know which end of a pen was used
    to draw on the tablet.

    \sa TabletDevice
*/

/*!
    \fn int QTabletEvent::pressure() const

    Returns the pressure that is exerted on the device. This number
    is a value from 0 (no pressure) to 255 (maximum pressure)
    inclusive. The pressure is always scaled to be within this range
    no matter how many pressure levels the underlying hardware
    supports.
*/

/*!
    \fn int QTabletEvent::xTilt() const

    Returns the angle between the device (a pen, for example) and the
    perpendicular in the direction of the x axis.
    Positive values are towards the tablet's physical right. The angle
    is in the range -60 to +60 degrees.

    \img qtabletevent-tilt.png

    \sa yTilt()
*/

/*!
    \fn int QTabletEvent::yTilt() const

    Returns the angle between the device (a pen, for example) and the
    perpendicular in the direction of the y axis.
    Positive values are towards the bottom of the tablet. The angle is
    within the range -60 to +60 degrees.

    \sa xTilt()
*/

/*!
    \fn const QPoint &QTabletEvent::pos() const

    Returns the position of the device, relative to the widget that
    received the event.

    If you move widgets around in response to mouse events, use
    globalPos() instead of this function.

    \sa x() y() globalPos()
*/

/*!
    \fn int QTabletEvent::x() const

    Returns the x position of the device, relative to the widget that
    received the event.

    \sa y() pos()
*/

/*!
    \fn int QTabletEvent::y() const

    Returns the y position of the device, relative to the widget that
    received the event.

    \sa x() pos()
*/

/*!
    \fn const QPoint &QTabletEvent::globalPos() const

    Returns the global position of the device \e{at the time of the
    event}. This is important on asynchronous windows systems like X11;
    whenever you move your widgets around in response to mouse events,
    globalPos() can differ significantly from the current position
    QCursor::pos().

    \sa globalX() globalY()
*/

/*!
    \fn int QTabletEvent::globalX() const

    Returns the global x position of the mouse pointer at the time of
    the event.

    \sa globalY() globalPos()
*/

/*!
    \fn int QTabletEvent::globalY() const

    Returns the global y position of the mouse pointer at the time of
    the event.

    \sa globalX() globalPos()
*/

/*!
    \fn Q_LONGLONG QTabletEvent::uniqueId()

    Returns a unique ID for the current device, making it possible
    to differentiate between multiple devices being used at the same
    time on the tablet.

    Values for the same device might vary from OS to OS.

    It is possible to generate a unique ID for any Wacom device.

*/

/*!
    Creates a QDragMoveEvent of the required \a type indicating
    that the mouse is at the \a point given within a widget.

    \warning Do not create a QDragMoveEvent yourself since these
    objects rely on Qt's internal state.
*/
QDragMoveEvent::QDragMoveEvent(const QPoint& pos, QDrag::DropActions actions, const QMimeData *data, Type type)
    : QDropEvent(pos, actions, data, type), rect(pos, QSize(1, 1))
{}

QDragMoveEvent::~QDragMoveEvent()
{
}

/*!
    \fn void QDragMoveEvent::accept(bool y)

    \overload

    Calls QDropEvent::accept(\a{y})
*/

/*!
    \fn void QDragMoveEvent::accept(const QRect &rectangle)

    The same as accept(), but also notifies that future moves will
    also be acceptable if they remain within the \a rectangle
    given on the widget. This can improve performance, but may
    also be ignored by the underlying system.

    If the rectangle is \link QRect::isEmpty() empty \endlink, then
    drag move events will be sent continuously. This is useful if the
    source is scrolling in a timer event.
*/

/*!
    \fn void QDragMoveEvent::ignore()

    \overload

    Calls QDropEvent::ignore().
*/

/*!
    \fn void QDragMoveEvent::ignore(const QRect &rectangle)

    The opposite of the accept(const QRect&) function.
    Moves within the \a rectangle are not acceptable, and will be
    ignored.
*/

/*!
    \fn QRect QDragMoveEvent::answerRect() const

    Returns the rectangle in the widget where the drop will occur if accepted.
    You can use this information to restrict drops to certain places on the
    widget.
*/


/*!
    \class QDropEvent qevent.h
    \ingroup events
    \ingroup draganddrop

    \brief The QDropEvent class provides an event which is sent when a
    drag and drop action is completed.

    When a widget \l{QWidget::setAcceptDrops()}{accepts drop events}, it will
    receive this event if it has accepted the most recent QDragEnterEvent or
    QDragMoveEvent sent to it.

    The drop event contains a proposed action, available from proposedAction(), for
    the widget to either accept or ignore. If the action can be handled by the
    widget, you should call the acceptProposedAction() function. Since the
    proposed action can be a combination of \l QDrag::DropAction values, it may be
    useful to either select one of these values as a default action or ask
    the user to select their preferred action. If the required drop action is
    different to the proposed action, you can call setDropAction() instead of
    acceptProposedAction() to complete the drop operation.

    The mimeData() function provides the data dropped on the widget in a QMimeData
    object. This contains information about the MIME type of the data in addition to
    the data itself.

    \sa QMimeData QDrag \link dnd.html Drag and Drop\endlink
*/

/*!
    \fn const QDropEvent::QMimeData *mimeData() const

    Returns the data that was dropped on the widget and its associated MIME
    type information.
*/

/*!
    Constructs a drop event of a certain \a type corresponding to a
    drop at the given \a point in a widget. The drag data is stored
    in \a data.
*/ // ### pos is in which coordinate system?
QDropEvent::QDropEvent(const QPoint& pos, QDrag::DropActions actions, const QMimeData *data, Type type)
    : QEvent(type), p(pos), act(actions),
      drop_action(QDrag::CopyAction),
      mdata(data)
{
    default_action = QDragManager::self()->defaultAction(act);
    ignore();
}

QDropEvent::~QDropEvent()
{
}

/*!
  \compat
    Returns a byte array containing the drag's data, in \a format.

    data() normally needs to get the data from the drag source, which
    is potentially very slow, so it's advisable to call this function
    only if you're sure that you will need the data in that
    particular \a format.

    The resulting data will have a size of 0 if the format was not
    available.

    \sa format() QByteArray::size()
*/

QByteArray QDropEvent::encodedData(const char *format) const
{
    return mdata->data(QLatin1String(format));
}

/*!
  \compat
    Returns a string describing one of the available data types for
    this drag. Common examples are "text/plain" and "image/gif".
    If \a n is less than zero or greater than the number of available
    data types, format() returns 0.

    This function is provided mainly for debugging. Most drop targets
    will use provides().

    \sa data() provides()
*/

const char* QDropEvent::format(int n) const
{
    if (fmts.isEmpty()) {
        QStringList formats = mdata->formats();
        for (int i = 0; i < formats.size(); ++i)
            fmts.append(formats.at(i).toLatin1());
    }
    if (n < 0 || n >= fmts.size())
        return 0;
    return fmts.at(n).constData();
}

/*!
  \compat
    Returns true if this event provides format \a mimeType; otherwise
    returns false.

    \sa data()
*/

bool QDropEvent::provides(const char *mimeType) const
{
    return mdata->formats().contains(QLatin1String(mimeType));
}

/*!
    If the source of the drag operation is a widget in this
    application, this function returns that source; otherwise it
    returns 0. The source of the operation is the first parameter to
    the QDrag object used instantiate the drag.

    This is useful if your widget needs special behavior when dragging
    to itself.

    \sa QDrag::QDrag()
*/
QWidget* QDropEvent::source() const
{
    QDragManager *manager = QDragManager::self();
    return manager ? manager->source() : 0;
}


void QDropEvent::setDropAction(QDrag::DropAction action)
{
    if (!(action & act))
        action = QDrag::CopyAction;
    drop_action = action;
}

/*!
    \fn const QPoint& QDropEvent::pos() const

    Returns the position where the drop was made.
*/

/*!
    \fn bool QDropEvent::isAccepted () const

    Returns true if the drop target accepts the event; otherwise
    returns false.
*/

/*!
    \fn void QDropEvent::accept(bool y=true)

    Call this function to indicate whether the event provided data
    that your widget processed. Set \a y to true (the default) if
    your widget could process the data, otherwise set \a y to false.
    To get the data, use mimeData().

    \sa acceptAction() QMimeData
*/

/*!
    \fn void QDropEvent::acceptAction(bool y=true)

    Call this to indicate that the action described by action() is
    accepted (i.e. if \a y is true, which is the default), not merely
    the default copy action. If you call acceptAction(true), there is
    no need to also call accept(true).
*/

/*!
    \enum QDropEvent::Action
    \compat

    When a drag and drop action is completed, the target is expected
    to perform an action on the data provided by the source. This
    will be one of the following:

    \value Copy The default action. The source simply uses the data
                provided in the operation.
    \value Link The source should somehow create a link to the
                location specified by the data.
    \value Move The source should somehow move the object from the
                location specified by the data to a new location.
    \value Private  The target has special knowledge of the MIME type,
                which the source should respond to in a similar way to
                a Copy.
    \value UserAction  The source and target can co-operate using
                special actions. This feature is not currently
                supported.

    The Link and Move actions only makes sense if the data is a
    reference, for example, text/uri-list file lists (see QUriDrag).
*/

/*!
    \fn void QDropEvent::setAction(Action action)

    Sets the \a action to be performed on the data by the target.
    This is used internally, you should not need to call this in your
    code: the \e source decides the action, not the target.
*/

/*!
    \fn Action QDropEvent::action() const
    \compat

    Returns the Action that the target is expected to perform on the
    data. If your application understands the action and can
    process the supplied data, call acceptAction(); if your
    application can process the supplied data but can only perform the
    Copy action, call accept().
*/

/*!
    \fn void QDropEvent::ignore()

    The opposite of accept(); i.e. you have ignored the drop event.
*/

#ifdef QT_COMPAT
/*!
    \fn bool QDropEvent::isActionAccepted () const

    Returns true if the drop action was accepted by the drop site;
    otherwise returns false.
*/

QT_COMPAT QDropEvent::Action QDropEvent::action() const
{
    switch(drop_action) {
    case QDrag::CopyAction:
        return Copy;
    case QDrag::MoveAction:
        return Move;
    case QDrag::LinkAction:
        return Link;
    default:
        return Copy;
    }
}
#endif

/*!
    \fn void QDropEvent::setPoint (const QPoint &point)
    \compat

    Sets the drop to happen at the given \a point. You do not normally
    need to use this as it will be set internally before your widget
    receives the drop event.
*/ // ### here too - what coordinate system?


/*!
    \class QDragEnterEvent qevent.h
    \brief The QDragEnterEvent class provides an event which is sent to a widget when a drag and drop action enters it.

    \ingroup events
    \ingroup draganddrop

    This event is always immediately followed by a QDragMoveEvent, so
    you only need to respond to one or the other event. This class
    inherits most of its functionality from QDragMoveEvent, which in
    turn inherits most of its functionality from QDropEvent.

    \sa QDragLeaveEvent, QDragMoveEvent, QDropEvent
*/

/*!
    Constructs a QDragEnterEvent that represents a drag entering a
    widget at the given \a point. The drag data is passed as \a data.

    \warning Do not create a QDragEnterEvent yourself since these
    objects rely on Qt's internal state.
*/
QDragEnterEvent::QDragEnterEvent(const QPoint& point, QDrag::DropActions actions, const QMimeData *data)
    : QDragMoveEvent(point, actions, data, DragEnter)
{}

QDragEnterEvent::~QDragEnterEvent()
{
}

QDragResponseEvent::QDragResponseEvent(bool accepted)
    : QEvent(DragResponse), a(accepted)
{}

QDragResponseEvent::~QDragResponseEvent()
{
}

/*!
    \class QDragMoveEvent qevent.h
    \ingroup events
    \ingroup draganddrop
    \brief The QDragMoveEvent class provides an event which is sent while a drag and drop action is in progress.

    When a widget \link QWidget::setAcceptDrops() accepts drop
    events \endlink, it will receive this event repeatedly while the
    drag is within the widget's boundaries. The widget should examine
    the event to see what kind of data it \link QDragMoveEvent::provides()
    provides \endlink, and call the accept() function to accept the drop
    if appropriate.

    The rectangle supplied by the answerRect() function can be used to restrict
    drops to certain parts of the widget. For example, we can check whether the
    rectangle intersects with the geometry of a certain child widget and only
    call \l{QDropEvent::acceptProposedAction()}{acceptProposedAction()} if that
    is the case.

    Note that this class inherits most of its functionality from
    QDropEvent.
*/

/*!
    \class QDragLeaveEvent qevent.h
    \brief The QDragLeaveEvent class provides an event that is sent to a widget when a drag and drop action leaves it.

    \ingroup events
    \ingroup draganddrop

    This event is always preceded by a QDragEnterEvent and a series
    of \l{QDragMoveEvent}s. It is not sent if a QDropEvent is sent
    instead.

    \sa QDragEnterEvent, QDragMoveEvent, QDropEvent
*/

/*!
    Constructs a QDragLeaveEvent.

    \warning Do not create a QDragLeaveEvent yourself since these
    objects rely on Qt's internal state.
*/
QDragLeaveEvent::QDragLeaveEvent()
    : QEvent(DragLeave)
{}

QDragLeaveEvent::~QDragLeaveEvent()
{
}

QHelpEvent::QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos)
    : QEvent(type), p(pos), gp(globalPos)
{}

QHelpEvent::~QHelpEvent()
{
}

QStatusTipEvent::QStatusTipEvent(const QString &tip)
    : QEvent(StatusTip), s(tip)
{}

QStatusTipEvent::~QStatusTipEvent()
{
}

QWhatsThisClickedEvent::QWhatsThisClickedEvent(const QString &href)
    : QEvent(WhatsThisClicked), s(href)
{}

QWhatsThisClickedEvent::~QWhatsThisClickedEvent()
{
}

QActionEvent::QActionEvent(int type, QAction *action, QAction *before)
    : QEvent(static_cast<QEvent::Type>(type)), act(action), bef(before)
{}

QActionEvent::~QActionEvent()
{
}

/*!
    \class QHideEvent qevent.h
    \brief The QHideEvent class provides an event which is sent after a widget is hidden.

    \ingroup events

    This event is sent just before QWidget::hide() returns, and also
    when a top-level window has been hidden (iconified) by the user.

    If spontaneous() is true, the event originated outside the
    application. In this case, the user hid the window using the
    window manager controls, either by iconifying the window or by
    switching to another virtual desktop where the window isn't
    visible. The window will become hidden but not withdrawn. If the
    window was iconified, QWidget::isMinimized() returns true.

    \sa QShowEvent
*/

/*!
    Constructs a QHideEvent.
*/
QHideEvent::QHideEvent()
    : QEvent(Hide)
{}

QHideEvent::~QHideEvent()
{
}

/*!
    \class QShowEvent qevent.h
    \brief The QShowEvent class provides an event that is sent when a widget is shown.

    \ingroup events

    There are two kinds of show events: show events caused by the
    window system (spontaneous), and internal show events. Spontaneous
    show events are sent just after the window system shows the
    window; they are also sent when a top-level window is redisplayed
    after being iconified. Internal show events are delivered just
    before the widget becomes visible.

    \sa QHideEvent
*/

/*!
    Constructs a QShowEvent.
*/
QShowEvent::QShowEvent()
    : QEvent(Show)
{}

QShowEvent::~QShowEvent()
{
}

/*!
  \fn QByteArray QDropEvent::data(const char* f) const

  \obsolete

  The encoded data is in \a f.
  Use QDropEvent::encodedData().
*/

/*!
    \class QFileOpenEvent qevent.h
    \brief The QFileOpenEvent class provides an event that will be
    sent when there is a request to open a file.

    \ingroup events

    File open events will be sent to the QApplication instance when
    the operating system requests that a file be opened. This is a
    high level event that can be caused by different user actions
    depending on the user's desktop environment; for example, double
    clicking on an file icon in the Finder on Mac OS X.

    This event is only used to notify the application of a request.
    It may be safely ignored.
*/

/*!
    \internal

    Constructs a file open event for the given \a file.
*/
QFileOpenEvent::QFileOpenEvent(const QString &file)
    : QEvent(FileOpen), f(file)
{}

QFileOpenEvent::~QFileOpenEvent()
{
}

/*!
    \fn QString QFileOpenEvent::file() const

    Returns the file that is being opened.
*/

/*!
    \class QToolBarChangeEvent qevent.h
    \brief The QToolBarChangeEvent class provides an event that is
    sent whenever a the toolbar button is clicked on Mac OS X.

    \ingroup events

    The QToolBarChangeEvent is sent when the toolbar button is clicked. On Mac
    OS X, this is the long oblong button on the right side of the window
    titlebar. The default implementation is to toggle the appearance (hidden or
    shown) of the associated toolbars for the window.
*/

/*!
    \internal

    Construct a QToolBarChangeEvent given the current button state in \a state.
*/
QToolBarChangeEvent::QToolBarChangeEvent(bool t)
    : QEvent(ToolBarChange), tog(t)
{}

QToolBarChangeEvent::~QToolBarChangeEvent()
{
}

/*
    \fn Qt::ButtonState QToolBarChangeEvent::state() const

    Returns the keyboard modifier flags at the time of the event.

    The returned value is a selection of the following values,
    combined using the OR operator:
    \c Qt::ShiftButton, \c Qt::ControlButton, \c Qt::MetaButton, and \c Qt::AltButton.
*/

QShortcutEvent::QShortcutEvent(const QKeySequence &key, int id, bool ambiguous)
    : QEvent(Shortcut), sequence(key), ambig(ambiguous), sid(id)
{}

QShortcutEvent::~QShortcutEvent()
{
}

#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug dbg, const QEvent *e) {
#ifndef Q_NO_STREAMING_DEBUG
    // More useful event output could be added here

    if (!e)
        return dbg << "QEvent(this = 0x0)";
    const char *n;
    switch (e->type()) {
    case QEvent::Timer:
        n = "Timer";
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    {
        const QMouseEvent *me = static_cast<const QMouseEvent*>(e);
        switch(me->type()) {
        case QEvent::MouseButtonPress:
            n = "MouseButtonPress";
            break;
        case QEvent::MouseMove:
            n = "MouseMove";
            break;
        case QEvent::MouseButtonRelease:
            n = "MouseButtonRelease";
            break;
        case QEvent::MouseButtonDblClick:
        default:
            n = "MouseButtonDblClick";
            break;
        }
        dbg.nospace() << "QMouseEvent("  << n
                      << ", " << me->button()
                      << ", " << hex << (int)me->buttons()
                      << ", " << hex << (int)me->modifiers()
                      << ")";
    }
    return dbg.space();
    case QEvent::ToolTip:
        n = "ToolTip";
        break;
    case QEvent::WindowActivate:
        n = "WindowActivate";
        break;
    case QEvent::WindowDeactivate:
        n = "WindowDeactivate";
        break;
    case QEvent::ActivationChange:
        n = "ActivationChange";
        break;
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        n = "Wheel";
        break;
#endif
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::ShortcutOverride:
        {
            const QKeyEvent *ke = static_cast<const QKeyEvent*>(e);
            switch(ke->type()) {
            case QEvent::ShortcutOverride:
                n = "ShortcutOverride";
                break;
            case QEvent::KeyRelease:
                n = "KeyRelease";
                break;
            case QEvent::KeyPress:
            default:
                n = "KeyPress";
                break;
            }
            dbg.nospace() << "QKeyEvent("  << n
                          << ", " << hex << ke->key()
                        << ", " << hex << (int)ke->modifiers()
                        << ", \"" << ke->text()
                        << "\", " << ke->isAutoRepeat()
                        << ", " << ke->count()
                        << ")";
        }
        return dbg.space();
    case QEvent::FocusIn:
        n = "FocusIn";
        break;
    case QEvent::FocusOut:
        n = "FocusOut";
        break;
    case QEvent::Enter:
        n = "Enter";
        break;
    case QEvent::Leave:
        n = "Leave";
        break;
    case QEvent::Paint:
        n = "Paint";
        break;
    case QEvent::Move:
        n = "Move";
        break;
    case QEvent::Resize:
        n = "Resize";
        break;
    case QEvent::Create:
        n = "Create";
        break;
    case QEvent::Destroy:
        n = "Destroy";
        break;
    case QEvent::Close:
        n = "Close";
        break;
    case QEvent::Quit:
        n = "Quit";
        break;
    case QEvent::FileOpen:
        n = "FileOpen";
        break;
    default:
        dbg.nospace() << "QEvent(" << (const void *)e << ", type = " << e->type() << ')';
        return dbg.space();
    }

    dbg.nospace() << 'Q' << n << "Event(" << (const void *)e << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support the streaming of QDebug");
    return dbg;
    Q_UNUSED(e);
#endif
}
#endif

/*!
    \internal

    \class QClipboardEvent qevent.h
    \ingroup events

    \brief The QClipboard class contains the parameters for a clipboard event

    This class is for internal use only, and exists to aid the clipboard on various
    platforms to get all the information it needs.  Use QEvent::Clipboard instead.
*/

QClipboardEvent::QClipboardEvent(QEventPrivate *data)
    : QEvent(QEvent::Clipboard)
{ d = data; }

QClipboardEvent::~QClipboardEvent()
{
}

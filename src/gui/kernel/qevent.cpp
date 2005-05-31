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
    \class QInputEvent
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

/*!
  \internal
*/
QInputEvent::~QInputEvent()
{
}

/*!
    \fn Qt::KeyboardModifiers QInputEvent::modifiers() const

    Returns the keyboard modifier flags that existed immediately
    before the event occurred.

    \sa QApplication::keyboardModifiers()
*/

/*!
    \class QMouseEvent
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

    The state of the keyboard modifier keys can be found by calling the
    \l{QInputEvent::modifiers()}{modifiers()} function, inhertied from
    QInputEvent.

    The functions pos(), x(), and y() give the cursor position
    relative to the widget that receives the mouse event. If you
    move the widget as a result of the mouse event, use the global
    position returned by globalPos() to avoid a shaking motion.

    The QWidget::setEnabled() function can be used to enable or
    disable mouse and keyboard events for a widget.

    Reimplement the QWidget event handlers, QWidget::mousePressEvent(),
    QWidget::mouseReleaseEvent(), QWidget::mouseDoubleClickEvent(),
    and QWidget::mouseMoveEvent() to receive mouse events in your own
    widgets.

    \sa QWidget::setMouseTracking() QWidget::grabMouse()
    QCursor::pos()
*/

/*!
    Constructs a mouse event object.

    The \a type parameter must be one of QEvent::MouseButtonPress,
    QEvent::MouseButtonRelease, QEvent::MouseButtonDblClick,
    or QEvent::MouseMove.

    The \a position is the mouse cursor's position relative to the
    receiving widget.
    The \a button that caused the event is given as a value from
    the \l Qt::MouseButton enum. If the event \a type is
    \l MouseMove, the appropriate button for this event is Qt::NoButton.
    The mouse and keyboard states at the time of the event are specified by
    \a buttons and \a modifiers.

    The globalPos() is initialized to QCursor::pos(), which may not
    be appropriate. Use the other constructor to specify the global
    position explicitly.
*/

QMouseEvent::QMouseEvent(Type type, const QPoint &position, Qt::MouseButton button,
                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
    : QInputEvent(type, modifiers), p(position), b(button), mouseState(buttons)
{
    g = QCursor::pos();
}

/*!
    \internal
*/
QMouseEvent::~QMouseEvent()
{
}

#ifdef QT3_SUPPORT
/*!
    Use QMouseEvent(\a type, \a pos, \a button, \c mouseButtons, \c
    keyboardModifiers) instead, where \c mouseButton is \a state &
    Qt::MouseButtonMask and \c keyboardModifiers is \a state &
    Qt::KeyButtonMask.
*/
QMouseEvent::QMouseEvent(Type type, const QPoint &pos, Qt::ButtonState button, int state)
    : QInputEvent(type), p(pos), b((Qt::MouseButton)button)
{
    g = QCursor::pos();
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & (int)Qt::KeyButtonMask);
}

/*!
    Use QMouseEvent(\a type, \a pos, \a globalPos, \a button,
    \c mouseButtons, \c keyboardModifiers) instead, where
    \c mouseButton is \a state & Qt::MouseButtonMask and
    \c keyboardModifiers is \a state & Qt::KeyButtonMask.
*/
QMouseEvent::QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos,
                         Qt::ButtonState button, int state)
    : QInputEvent(type), p(pos), g(globalPos), b((Qt::MouseButton)button)
{
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & (int)Qt::KeyButtonMask);
}
#endif


/*!
    Constructs a mouse event object.

    The \a type parameter must be QEvent::MouseButtonPress,
    QEvent::MouseButtonRelease, QEvent::MouseButtonDblClick,
    or QEvent::MouseMove.

    The \a pos is the mouse cursor's position relative to the
    receiving widget. The cursor's position in global coordinates is
    specified by \a globalPos.  The \a button that caused the event is
    given as a value from the \l Qt::MouseButton enum. If the event \a
    type is \l MouseMove, the appropriate button for this event is
    Qt::NoButton. \a buttons is the state of all buttons at the
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

    Possible return values are Qt::LeftButton, Qt::RightButton,
    Qt::MidButton, and Qt::NoButton.

    Note that the returned value is always Qt::NoButton for mouse
    move events.

    \sa buttons() Qt::MouseButton
*/

/*!
    \fn Qt::MouseButton QMouseEvent::buttons() const

    Returns the button state when the event was generated. The button
    state is a combination of Qt::LeftButton, Qt::RightButton,
    Qt::MidButton using the OR operator. For mouse move events,
    this is all buttons that are pressed down. For mouse press and
    double click events this includes the button that caused the
    event. For mouse release events this excludes the button that
    caused the event.

    \sa button() Qt::MouseButton
*/


/*!
    \fn Qt::ButtonState QMouseEvent::state() const

    Returns the button state immediately before the event was
    generated. The button state is a combination of mouse buttons
    (see Qt::ButtonState) and keyboard modifiers (Qt::MouseButtons).

    Use buttons() and/or modifiers() instead. Be aware that buttons()
    return the state immediately \e after the event was generated.
*/

/*!
    \fn Qt::ButtonState QMouseEvent::stateAfter() const

    Returns the button state immediately after the event was
    generated. The button state is a combination of mouse buttons
    (see Qt::ButtonState) and keyboard modifiers (Qt::MouseButtons).

    Use buttons() and/or modifiers() instead.
*/


/*!
    \class QHoverEvent
    \ingroup events

    \brief The QHoverEvent class contains parameters that describe a mouse event.

    Mouse events occur when a mouse cursor is moved into, out of, or within a
    widget, and if the widget has the Qt::WA_Hover attribute.

    The function pos() gives the current cursor position, while oldPos() gives
    the old mouse position.
*/

/*!
    \fn const QPoint &QHoverEvent::pos() const

    Returns the position of the mouse cursor, relative to the widget
    that received the event.

    On QEvent::HoverLeave events, this position will always be
    QPoint(-1, -1).

    \sa oldPos()
*/

/*!
    \fn const QPoint &QHoverEvent::oldPos() const

    Returns the previous position of the mouse cursor, relative to the widget
    that received the event. If there is no previous position, oldPos() will
    return the same position as pos().

    On QEvent::HoverEnter events, this position will always be
    QPoint(-1, -1).

    \sa pos()
*/

/*!
    Constructs a hover event object.

    The \a type parameter must be QEvent::HoverEnter,
    QEvent::HoverLeave, or QEvent::HoverMove.

    The \a pos is the current mouse cursor's position relative to the
    receiving widget, while \a oldPos is the previous mouse cursor's
    position relative to the receiving widget.
*/
QHoverEvent::QHoverEvent(Type type, const QPoint &pos, const QPoint &oldPos)
    : QEvent(type), p(pos), op(oldPos)
{
}

/*!
    \internal
*/
QHoverEvent::~QHoverEvent()
{
}


/*!
    \class QWheelEvent
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

    The QWidget::setEnabled() function can be used to enable or
    disable mouse and keyboard events for a widget.

    The event handler QWidget::wheelEvent() receives wheel events.

    \sa QMouseEvent QWidget::grabMouse()
*/

/*!
    \fn Qt::MouseButtons QWheelEvent::buttons() const

    Returns the mouse state when the event occurred.
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
    position explicitly.

    The \a buttons describe the state of the mouse buttons at the time
    of the event, \a delta contains the rotation distance,
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

/*!
  \internal
*/
QWheelEvent::~QWheelEvent()
{
}

#ifdef QT3_SUPPORT
/*!
    Use one of the other constructors instead.
*/
QWheelEvent::QWheelEvent(const QPoint &pos, int delta, int state, Qt::Orientation orient)
    : QInputEvent(Wheel), p(pos), d(delta), o(orient)
{
    g = QCursor::pos();
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & (int)Qt::KeyButtonMask);
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

#ifdef QT3_SUPPORT
/*!
    Use one of the other constructors instead.
*/
QWheelEvent::QWheelEvent(const QPoint &pos, const QPoint& globalPos, int delta, int state,
                         Qt::Orientation orient)
    : QInputEvent(Wheel), p(pos), g(globalPos), d(delta), o(orient)
{
    mouseState = Qt::MouseButtons(state & Qt::MouseButtonMask);
    modState = Qt::KeyboardModifiers(state & (int) Qt::KeyButtonMask);
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


/*! \obsolete
    \fn Qt::ButtonState QWheelEvent::state() const

    Returns the keyboard modifier flags at the time of the event.

    The returned value is a selection of the following values,
    combined using the OR operator: Qt::ShiftButton,
    Qt::ControlButton, and Qt::AltButton.
*/


/*!
    \class QKeyEvent
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

    The \a type parameter must be QEvent::KeyPress, QEvent::KeyRelease,
    or QEvent::ShortcutOverride.

    If \a key is 0, the event is not a result of
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
}

/*!
  \internal
*/
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
    returned can be an empty string in cases
    where modifier keys, such as Shift, Control, Alt, and Meta,
    are being pressed or released. In such cases key() will contain
    a valid value.

    \sa Qt::WA_KeyCompression
*/

/*!
    Returns the keyboard modifier flags that existed immediately
    after the event occurred.

    \warning This function cannot always be trusted. The user can
    confuse it by pressing both \key{Shift} keys pressed
    simulatenously and releasing one of them, for example.

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

#ifdef QT3_SUPPORT
/*!
    \fn QKeyEvent::QKeyEvent(Type type, int key, int ascii,
                             int modifiers, const QString &text,
                             bool autorep, ushort count)

    Use one of the other constructors instead.
*/

/*!
    \fn int QKeyEvent::ascii() const

    Use text() instead.
*/

/*!
    \fn Qt::ButtonState QKeyEvent::state() const

    Use QInputEvent::modifiers() instead.
*/

/*!
    \fn Qt::ButtonState QKeyEvent::stateAfter() const

    Use modifiers() instead.
*/
#endif

/*!
    \class QFocusEvent
    \brief The QFocusEvent class contains event parameters for widget focus
    events.

    \ingroup events

    Focus events are sent to widgets when the keyboard input focus
    changes. Focus events occur due to mouse actions, key presses
    (such as \gui{Tab} or \gui{Backtab}), the window system, popup
    menus, keyboard shortcuts, or other application-specific reasons.
    The reason for a particular focus event is returned by reason()
    in the appropriate event handler.

    The event handlers QWidget::focusInEvent() and
    QWidget::focusOutEvent() receive focus events.

    \sa QWidget::setFocus(), QWidget::setFocusPolicy(), {Keyboard Focus}
*/

/*!
    Constructs a focus event object.

    The \a type parameter must be either QEvent::FocusIn or
    QEvent::FocusOut. The \a reason describes the cause of the change
    in focus.
*/
QFocusEvent::QFocusEvent(Type type, Qt::FocusReason reason)
    : QEvent(type), m_reason(reason)
{}

/*!
    \internal
*/
QFocusEvent::~QFocusEvent()
{
}

/*!
    Returns the reason for this focus event.
 */
Qt::FocusReason QFocusEvent::reason()
{
    return m_reason;
}

/*!
    \fn bool QFocusEvent::gotFocus() const

    Returns true if type() is QEVent::FocusIn; otherwise returns
    false.
*/

/*!
    \fn bool QFocusEvent::lostFocus() const

    Returns true if type() is QEVent::FocusOut; otherwise returns
    false.
*/

#ifdef QT3_SUPPORT
/*!
    \enum QFocusEvent::Reason
    \compat

    Use Qt::FocusReason instead.

    \value Mouse  Same as Qt::MouseFocusReason.
    \value Tab  Same as Qt::TabFocusReason.
    \value Backtab  Same as Qt::BacktabFocusReason.
    \value MenuBar  Same as Qt::MenuBarFocusReason.
    \value ActiveWindow  Same as Qt::ActiveWindowFocusReason
    \value Other  Same as Qt::OtherFocusReason
    \value Popup  Same as Qt::PopupFocusReason
    \value Shortcut  Same as Qt::ShortcutFocusReason
*/
#endif

/*!
    \class QPaintEvent
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

    \sa QPainter, QWidget::update(), QWidget::repaint(),
        QWidget::paintEvent()
*/

/*!
    \fn bool QPaintEvent::erased() const
    \compat

    Returns true if the paint event region (or rectangle) has been
    erased with the widget's background; otherwise returns false.

    Qt 4 \e always erases regions that require painting. The exception
    to this rule is if the widget sets the Qt::WA_NoBackground or
    Qt::WA_NoSystemBackground attributes. If either one of those
    attributes is set \e and the window system does not make use of
    subwidget alpha composition (currently X11 and Windows, but this
    may change), then the region is not erased.
*/

/*!
    \fn void QPaintEvent::setErased(bool b) { m_erased = b; }
    \internal
*/

/*!
    Constructs a paint event object with the region that needs to
    be updated. The region is specified by \a paintRegion.
*/
QPaintEvent::QPaintEvent(const QRegion& paintRegion)
    : QEvent(Paint), m_rect(paintRegion.boundingRect()), m_region(paintRegion), m_erased(false)
{}

/*!
    Constructs a paint event object with the rectangle that needs
    to be updated. The region is specified by \a paintRect.
*/
QPaintEvent::QPaintEvent(const QRect &paintRect)
    : QEvent(Paint), m_rect(paintRect),m_region(paintRect), m_erased(false)
{}


#ifdef QT3_SUPPORT
 /*!
    Constructs a paint event object with both a \a paintRegion and a
    \a paintRect, both of which represent the area of the widget that
    needs to be updated.

*/
QPaintEvent::QPaintEvent(const QRegion &paintRegion, const QRect &paintRect)
    : QEvent(Paint), m_rect(paintRect), m_region(paintRegion), m_erased(false)
{}
#endif

/*!
  \internal
*/
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
    \class QMoveEvent
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

/*!
  \internal
*/
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
    \class QResizeEvent
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

/*!
  \internal
*/
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
    \class QCloseEvent
    \brief The QCloseEvent class contains parameters that describe a close event.

    \ingroup events

    Close events are sent to widgets that the user wants to close,
    usually by choosing "Close" from the window menu, or by clicking
    the \gui{X} title bar button. They are also sent when you call
    QWidget::close() to close a widget programmatically.

    Close events contain a flag that indicates whether the receiver
    wants the widget to be closed or not. When a widget accepts the
    close event, it is hidden (and destroyed if it was created with
    the Qt::WA_DeleteOnClose flag). If it refuses to accept the close
    event nothing happens. (Under X11 it is possible that the window
    manager will forcibly close the window; but at the time of writing
    we are not aware of any window manager that does this.)

    The event handler QWidget::closeEvent() receives close events. The
    default implementation of this event handler accepts the close
    event. If you do not want your widget to be hidden, or want some
    special handing, you should reimplement the event handler and
    ignore() the event.

    The \l{mainwindows/application#close event handler}{closeEvent() in the
    Application example} shows a close event handler that
    asks whether to save a document before closing.

    If you want the widget to be deleted when it is closed, create it
    with the Qt::WA_DeleteOnClose flag. This is very useful for
    independent top-level windows in a multi-window application.

    \l{QObject}s emits the \l{QObject::destroyed()}{destroyed()}
    signal when they are deleted.

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

/*! \internal
*/
QCloseEvent::~QCloseEvent()
{
}

/*!
   \class QIconDragEvent
   \brief The QIconDragEvent class indicates that a main icon drag has begun.

   \ingroup events

   Icon drag events are sent to widgets when the main icon of a window
   has been dragged away. On Mac OS X, this happens when the proxy
   icon of a window is dragged off the title bar.

   It is normal to begin using drag and drop in response to this
   event.

   \sa {Drag and Drop}, QMimeData, QDrag
*/

/*!
    Constructs an icon drag event object with the accept flag set to
    false.

    \sa accept()
*/
QIconDragEvent::QIconDragEvent()
    : QEvent(IconDrag)
{ ignore(); }

/*! \internal */
QIconDragEvent::~QIconDragEvent()
{
}

/*!
    \class QContextMenuEvent
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
    Constructs a context menu event object with the accept parameter
    flag set to false.

    The \a reason parameter must be QContextMenuEvent::Mouse or
    QContextMenuEvent::Keyboard.

    The \a pos parameter specifies the mouse position relative to the
    receiving widget. \a globalPos is the mouse position in absolute
    coordinates.
*/
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos)
    : QInputEvent(ContextMenu), p(pos), gp(globalPos), reas(reason)
{}

#ifdef QT3_SUPPORT
/*!
    Constructs a context menu event with the given \a reason for the
    position specified by \a pos in widget coordinates and \a globalPos
    in global screen coordinates. \a dummy is ignored.
*/
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos,
                                     int /* dummy */)
    : QInputEvent(ContextMenu), p(pos), gp(globalPos), reas(reason)
{}
#endif

/*! \internal */
QContextMenuEvent::~QContextMenuEvent()
{
}
/*!
    Constructs a context menu event object with the accept parameter
    flag set to false.

    The \a reason parameter must be QContextMenuEvent::Mouse or
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

#ifdef QT3_SUPPORT
/*!
    Constructs a context menu event with the given \a reason for the
    position specified by \a pos in widget coordinates. \a dummy is
    ignored.
*/
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, int /* dummy */)
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
    Qt::LeftButton, Qt::RightButton, Qt::MidButton,
    Qt::ShiftButton, Qt::ControlButton, and Qt::AltButton.
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
    \class QInputMethodEvent
    \brief The QInputMethodEvent class provides parameters for input method events.

    \ingroup events

    Input method events are sent to widgets when an input method is
    used to enter text into a widget. Input methods are widely used
    to enter text for languages with non-Latin alphabets.

    The events are of interest to authors of keyboard entry widgets
    who want to be able to correctly handle languages with complex
    character input. Text input in such languages is usually a three
    step process:

    \list 1
    \o \bold{Starting to Compose}

       When the user presses the first key on a keyboard, an input
       context is created. This input context will contain a string
       of the typed characters.

    \o \bold{Composing}

       With every new key pressed, the input method will try to create a
       matching string for the text typed so far called preedit
       string. While the input context is active, the user can only move
       the cursor inside the string belonging to this input context.

    \o \bold{Completing}

       At some point, the user will activate a user interface component
       (perhaps using a particular key) where they can choose from a
       number of strings matching the text they have typed so far. The
       user can either confirm their choice cancel the input; in either
       case the input context will be closed.
    \endlist

    QInputMethodEvent models these three stages, and transfers the
    information needed to correctly render the intermediate result. A
    QInputMethodEvent has two main parameters: preeditString() and
    commitString(). The preeditString() parameter gives the currently
    active preedit string. The commitString() parameter gives a text
    that should get added to (or replace parts of) the text of the
    editor widget. It usually is a result of the input operations and
    has to be inserted to the widgets text directly before the preedit
    string.

    If the commitString() should replace parts of the of the text in
    the editor, replacementLength() will contain the number of
    characters to be replaced. replacementStart() contains the position
    at which characters are to be replaced relative from the start of
    the preedit string.

    A number of attributes control the visual appearance of the
    preedit string (the visual appearance of text outside the preedit
    string is controlled by the widget only). The AttributeType enum
    describes the different attributes that can be set.

    A class implementing QWidget::inputMethodEvent() should at least
    understand and honor the \l TextFormat and \l Cursor attributes.

    Since input methods need to be able to query certain properties
    from the widget, the widget must also implement
    QWidget::inputMethodQuery().

    When receiving an input method event, the text widget has to performs the
    following steps:

    \list 1
    \o If the widget has selected text, the selected text should get
       removed.

    \o Remove the text starting at replacementStart() with length
       replacementLength() and replace it by the commitString(). If
       replacementLength() is 0, replacementStart() gives the insertion
       position for the commitString().

       When doing replacement the area of the preedit
       string is ignored, thus a replacement starting at -1 with a length
       of 2 will remove the last character before the preedit string and
       the first character afterwards, and insert the commit string
       directly before the preedit string.

       If the widget implements undo/redo, this operation gets added to
       the undo stack.

    \o If there is no current preedit string, insert the
       preeditString() at the current cursor position; otherwise replace
       the previous preeditString with the one received from this event.

       If the widget implements undo/redo, the preeditString() should not
       influence the undo/redo stack in any way.

       The widget should examine the list of attributes to apply to the
       preedit string. It has to understand at least the TextFormat and
       Cursor attributes and render them as specified.
    \endlist

    \sa QInputContext
*/

/*!
    \enum QInputMethodEvent::AttributeType

    \value TextFormat
    A QTextCharFormat for the part of the preedit string specified by
    start and length. value contains a QVariant of type QTextFormat
    specifying rendering of this part of the preedit string. There
    should be at most one format for every part of the preedit
    string. If several are specified for any character in the string the
    behaviour is undefined. A conforming implementation has to at least
    honour the backgroundColor, textColor and fontUnderline properties
    of the format.

    \value Cursor
    If set, a cursor should be shown inside the preedit string at
    position start. If value is a QVariant of type QColor this color
    will be used for rendering the cursor, otherwise the color of the
    surrounding text will be used. There should be at most one Cursor
    attribute per event. If several are specified the behaviour is undefined.

    \value Language
    The variant contains a QLocale object specifying the language of a
    certain part of the preedit string. There should be at most one
    language set for every part of the preedit string. If several are
    specified for any character in the string the behaviour is undefined.

    \value Ruby
    The ruby text for a part of the preedit string. There should be at
    most one ruby text set for every part of the preedit string. If
    several are specified for any character in the string the behaviour
    is undefined.

    \sa Attribute
*/

/*!
    \class QInputMethodEvent::Attribute
    \brief The QInputMethodEvent::Attribute class stores an input method attribute.
*/

/*!
    \fn QInputMethodEvent::Attribute::Attribute(AttributeType type, int start, int length, QVariant value)

    Constructs an input method attribute. \a type specifies the type
    of attribute, \a start and \a length the position of the
    attribute, and \a value the value of the attribute.
*/

/*!
    Constructs an event of type QEvent::InputMethod. The
    attributes(), preeditString(), commitString(), replacementStart(),
    and replacementLength() are initialized to default values.

    \sa setCommitString()
*/
QInputMethodEvent::QInputMethodEvent()
    : QEvent(QEvent::InputMethod), replace_from(0), replace_length(0)
{
}

/*!
    Construcs an event of type QEvent::InputMethod. The
    preedit text is set to \a preeditText, the attributes to
    \a attributes.

    The commitString(), replacementStart(), and replacementLength()
    values can be set using setCommitString().

    \sa preeditString(), attributes()
*/
QInputMethodEvent::QInputMethodEvent(const QString &preeditText, const QList<Attribute> &attributes)
    : QEvent(QEvent::InputMethod), preedit(preeditText), attrs(attributes),
      replace_from(0), replace_length(0)
{
}

/*!
    Constructs a copy of \a other.
*/
QInputMethodEvent::QInputMethodEvent(const QInputMethodEvent &other)
    : QEvent(QEvent::InputMethod), preedit(other.preedit), attrs(other.attrs),
      commit(other.commit), replace_from(other.replace_from), replace_length(other.replace_length)
{
}

/*!
    Sets the commit string to \a commitString.

    The commit string is the text that should get added to (or
    replace parts of) the text of the editor widget. It usually is a
    result of the input operations and has to be inserted to the
    widgets text directly before the preedit string.

    If the commit string should replace parts of the of the text in
    the editor, \a replaceLength specifies the number of
    characters to be replaced. \a replaceFrom specifies the position
    at which characters are to be replaced relative from the start of
    the preedit string.

    \sa commitString(), replacementStart(), replacementLength()
*/
void QInputMethodEvent::setCommitString(const QString &commitString, int replaceFrom, int replaceLength)
{
    commit = commitString;
    replace_from = replaceFrom;
    replace_length = replaceLength;
}

/*!
    \fn const QList<Attribute> &QInputMethodEvent::attributes() const

    Returns the list of attributes passed to the QInputMethodEvent
    constructor. The attributes control the visual appearance of the
    preedit string (the visual appearance of text outside the preedit
    string is controlled by the widget only).

    \sa preeditString(), Attribute
*/

/*!
    \fn const QString &QInputMethodEvent::preeditString() const

    Returns the preedit text, i.e. the text before the user started
    editing it.

    \sa commitString(), attributes()
*/

/*!
    \fn const QString &QInputMethodEvent::commitString() const

    Returns the text that should get added to (or replace parts of)
    the text of the editor widget. It usually is a result of the
    input operations and has to be inserted to the widgets text
    directly before the preedit string.

    \sa setCommitString(), preeditString(), replacementStart(), replacementLength()
*/

/*!
    \fn int QInputMethodEvent::replacementStart() const

    Returns the position at which characters are to be replaced relative
    from the start of the preedit string.

    \sa replacementLength(), setCommitString()
*/

/*!
    \fn int QInputMethodEvent::replacementLength() const

    Returns the number of characters to be replaced in the preedit
    string.

    \sa replacementStart(), setCommitString()
*/

/*!
    \class QTabletEvent
    \brief The QTabletEvent class contains parameters that describe a Tablet event.

    \ingroup events

    Tablet Events are generated from a Wacom tablet. Most of the time you will
    want to deal with events from the tablet as if they were events from a
    mouse; for example, you would retrieve the cursor position with x(), y(),
    pos(), globalX(), globalY(), and globalPos(). In some situations you may
    wish to retrieve the extra information provided by the tablet device
    driver; for example, you might want to do subpixeling with higher
    resolution coordinates or you may want to adjust color brightness based on
    pressure.  QTabletEvent allows you to read the pressure(), the xTilt(), and
    yTilt(), as well as the type of device being used with device() (see
    \l{TabletDevice}). It can also give you the minimum and maximum values for
    each device's pressure and high resolution coordinates.

    A tablet event contains a special accept flag that indicates
    whether the receiver wants the event. You should call
    QTabletEvent::accept() if you handle the tablet event; otherwise
    it will be sent to the parent widget.

    The QWidget::setEnabled() function can be used to enable or
    disable mouse and keyboard events for a widget.

    The event handler QWidget::tabletEvent() receives all three types of
    tablet events. Qt will first send a tabletEvent then, if it is not
    accepted, it will send a mouse event. This allows applications that
    don't utilize tablets to use a tablet like a mouse, while also
    enabling those who want to use both tablets and mouses differently.

*/

/*!
    \enum QTabletEvent::TabletDevice

    This enum defines what type of device is generating the event.

    \value NoDevice    No device, or an unknown device.
    \value Puck    A Puck (a device that is similar to a flat mouse with
    a transparent circle with cross-hairs).
    \value Stylus  A Stylus.
    \value Airbrush An airbrush
    \value FourDMouse A 4D Mouse.
    \omitvalue XFreeEraser
*/

/*!
    \enum QTabletEvent::PointerType

    This enum defines what type of point is generating the event.

    \value UnknownPointer    An unknown device.
    \value Pen    Tip end of a stylus-like device (the narrow end of the pen).
    \value Cursor  Any puck-like device.
    \value Eraser  Eraser end of a stylus-like device (the broad end of the pen).

    \sa pointerType()
*/

/*!
  Construct a tablet event of the given \a type.

  The \a pos parameter indicates where the event occurred in the
  widget; \a globalPos is the corresponding position in absolute
  coordinates. The \a hiResGlobalPos contains a high resolution
  measurement of the position.

  \a pressure contains the pressure exerted on the \a device.

  \a pointerType describes the type of pen that is being used.

  \a xTilt and \a yTilt contain the device's degree of tilt from the
  x and y axes respectively.

  \a keyState specifies which keyboard modifiers are pressed (e.g.,
  \key{Ctrl}).

  The \a uniqueID parameter contains the unique ID for the current device.

  The \a z parameter contains the coordinate of the device on the tablet, this
  is usually given by a wheel on 4D mouse. If the device does not support a
  Z-axis, pass zero here.

  The \a tangentialPressure paramater contins the tangential pressure of an air
  brush. If the device does not support tangential pressure, pass 0 here.

  \a rotation contains the device's rotation in degrees. 4D mice support
  rotation. If the device does not support rotation, pass 0 here.

  \sa pos() globalPos() device() pressure() xTilt() yTilt() uniqueId(), rotation(), tangentialPressure(), z()
*/

QTabletEvent::QTabletEvent(Type type, const QPoint &pos, const QPoint &globalPos,
                           const QPointF &hiResGlobalPos, int device, int pointerType,
                           qreal pressure, int xTilt, int yTilt, qreal tangentialPressure,
                           qreal rotation, int z, Qt::KeyboardModifiers keyState, qint64 uniqueID)
    : QInputEvent(type, keyState),
      mPos(pos),
      mGPos(globalPos),
      mHiResGlobalPos(hiResGlobalPos),
      mDev(device),
      mPointerType(pointerType),
      mXT(xTilt),
      mYT(yTilt),
      mZ(z),
      mPress(pressure),
      mTangential(tangentialPressure),
      mRot(rotation),
      mUnique(uniqueID),
      mExtra(0)
{
}

/*!
    \internal
*/
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
    \fn PointerType QTabletEvent::pointerType() const

    Returns the type of point that generated the event.
*/

/*!
    \fn qreal QTabletEvent::tangentialPressure() const

    Returns the tangential pressure for the device.  This is typically given by a finger
    wheel on an airbrush tool.  The range is from -1.0 to 1.0. 0.0 indicates a
    neutral position.  Current airbrushes can only move in the positive
    direction from the neutrual position. If the device does not support
    tangential pressure, this value is always 0.0.

    \sa pressure()
*/

/*!
    \fn qreal QTabletEvent::rotation() const

    Returns the rotation of the current device in degress. This is usually
    given by a 4D Mouse. If the device doesn't support rotation this value is
    always 0.0.

*/

/*!
    \fn qreal QTabletEvent::pressure() const

    Returns the pressure for the device. 0.0 indicates that the stylus is not
    on the tablet, 1.0 indicates the maximum amount of pressure for the stylus.

    \sa tangentialPressure()
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
    \fn int QTabletEvent::z() const

    Returns the z position of the device. Typically this is represented by a
    wheel on a 4D Mouse. If the device does not support a Z-axis, this value is
    always zero. This is <em>not</em> the same as pressure.

    \sa pressure()
*/

/*!
    \fn const QPoint &QTabletEvent::globalPos() const

    Returns the global position of the device \e{at the time of the
    event}. This is important on asynchronous windows systems like X11;
    whenever you move your widgets around in response to mouse events,
    globalPos() can differ significantly from the current position
    QCursor::pos().

    \sa globalX() globalY() hiResGlobalPos()
*/

/*!
    \fn int QTabletEvent::globalX() const

    Returns the global x position of the mouse pointer at the time of
    the event.

    \sa globalY() globalPos() hiResGlobalX()
*/

/*!
    \fn int QTabletEvent::globalY() const

    Returns the global y position of the tablet device at the time of
    the event.

    \sa globalX() globalPos() hiResGlobalY()
*/

/*!
    \fn qint64 QTabletEvent::uniqueId() const

    Returns a unique ID for the current device, making it possible
    to differentiate between multiple devices being used at the same
    time on the tablet.

    Values for the same device may vary from OS to OS.

    It is possible to generate a unique ID for any Wacom device.
*/

/*!
    \fn const QPointF &QTabletEvent::hiResGlobalPos() const

    The high precision coordinates delivered from the tablet expressed.
    Sub pixeling information is in the fractional part of the QPointF.

    \sa globalPos() hiResGlobalX() hiResGlobalY()
*/

/*!
    \fn qreal &QTabletEvent::hiResGlobalX() const

    The high precision x position of the tablet device.
*/

/*!
    \fn qreal &QTabletEvent::hiResGlobalY() const

    The high precision y position of the tablet device.
*/

/*!
    Creates a QDragMoveEvent of the required \a type indicating
    that the mouse is at position \a pos given within a widget.

    The mouse and keyboard states are specified by \a buttons and
    \a modifiers, and the \a actions describe the types of drag
    and drop operation that are possible.
    The drag data is passed as MIME-encoded information in \a data.

    \warning Do not attempt to create a QDragMoveEvent yourself.
    These objects rely on Qt's internal state.
*/
QDragMoveEvent::QDragMoveEvent(const QPoint& pos, Qt::DropActions actions, const QMimeData *data,
                               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type)
    : QDropEvent(pos, actions, data, buttons, modifiers, type)
    , rect(pos, QSize(1, 1))
{}

/*!
    Destroys the event.
*/
QDragMoveEvent::~QDragMoveEvent()
{
}

/*!
    \fn void QDragMoveEvent::accept(bool y)

    Calls setAccepted(\a y) instead.
*/

/*!
    \fn void QDragMoveEvent::accept(const QRect &rectangle)

    The same as accept(), but also notifies that future moves will
    also be acceptable if they remain within the \a rectangle
    given on the widget. This can improve performance, but may
    also be ignored by the underlying system.

    If the rectangle is empty, drag move events will be sent
    continuously. This is useful if the source is scrolling in a
    timer event.
*/

/*!
    \fn void QDragMoveEvent::accept()

    \overload

    Calls QDropEvent::accept().
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
    \class QDropEvent
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
    proposed action can be a combination of \l Qt::DropAction values, it may be
    useful to either select one of these values as a default action or ask
    the user to select their preferred action. If the required drop action is
    different to the proposed action, you can call setDropAction() instead of
    acceptProposedAction() to complete the drop operation.

    The mimeData() function provides the data dropped on the widget in a QMimeData
    object. This contains information about the MIME type of the data in addition to
    the data itself.

    \sa QMimeData, QDrag, {Drag and Drop}
*/

/*!
    \fn const QMimeData *QDropEvent::mimeData() const

    Returns the data that was dropped on the widget and its associated MIME
    type information.
*/

/*!
    Constructs a drop event of a certain \a type corresponding to a
    drop at the point specified by \a pos in the destination widget's
    coordinate system.

    The \a actions indicate which types of drag and drop operation can
    be performed, and the drag data is stored as MIME-encoded data in \a data.

    The states of the mouse buttons and keyboard modifiers at the time of
    the drop are specified by \a buttons and \a modifiers.
*/ // ### pos is in which coordinate system?
QDropEvent::QDropEvent(const QPoint& pos, Qt::DropActions actions, const QMimeData *data,
                       Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type)
    : QEvent(type), p(pos), mouseState(buttons),
      modState(modifiers), act(actions),
      drop_action(Qt::CopyAction),
      mdata(data)
{
    default_action = QDragManager::self()->defaultAction(act, modifiers);
    ignore();
}

/*! \internal */
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


void QDropEvent::setDropAction(Qt::DropAction action)
{
    if (!(action & act))
        action = Qt::CopyAction;
    drop_action = action;
}

/*!
    \fn const QPoint& QDropEvent::pos() const

    Returns the position where the drop was made.
*/

/*!
    \fn Qt::MouseButtons QDropEvent::mouseButtons() const

    Returns the mouse buttons that are pressed..
*/

/*!
    \fn Qt::KeyboardModifiers QDropEvent::keyboardModifiers() const

    Returns the modifier keys that are pressed.
*/

/*!
    \fn void QDropEvent::accept()
    \internal
*/

/*!
    \fn void QDropEvent::accept(bool accept)

    Call setAccepted(\a accept) instead.
*/

/*!
    \fn void QDropEvent::acceptAction(bool accept = true)

    Call this to indicate that the action described by action() is
    accepted (i.e. if \a accept is true, which is the default), not merely
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
    \fn void QDropEvent::setDropAction(Qt::DropAction action)

    Sets the \a action to be performed on the data by the target.
    Use this to override the \l{proposedAction()}{proposed action}
    with one of the \l{possibleActions()}{possible actions}.

    If you set a drop action that is not one of the possible actions, the
    drag and drop operation will default to a copy operation.

    \sa dropAction()
*/

/*!
    \fn Qt::DropAction QDropEvent::dropAction() const

    Returns the action that the target is expected to perform on the
    data. If your application understands the action and can
    process the supplied data, call acceptAction(); if your
    application can process the supplied data but can only perform the
    Copy action, call accept().

    \sa setDropAction()
*/

/*!
    \fn Qt::DropActions QDropEvent::possibleActions() const

    Returns an OR-combination of possible drop actions.

    \sa dropAction()
*/

/*!
    \fn Qt::DropAction QDropEvent::proposedAction() const

    Returns the proposed drop action.

    \sa dropAction()
*/

/*!
    \fn void QDropEvent::acceptProposedAction()

    Sets the drop action to be the proposed action.

    \sa setDropAction(), proposedAction(), accept()
*/

#ifdef QT3_SUPPORT
/*!
    Use dropAction() instead.

    The table below shows the correspondance between the return type
    of action() and the return type of dropAction().

    \table
    \header \i Old enum value   \i New enum value
    \row    \i QDropEvent::Copy \i Qt::CopyAction
    \row    \i QDropEvent::Move \i Qt::MoveAction
    \row    \i QDropEvent::Link \i Qt::LinkAction
    \row    \i other            \i Qt::CopyAction
    \endtable
*/

QT3_SUPPORT QDropEvent::Action QDropEvent::action() const
{
    switch(drop_action) {
    case Qt::CopyAction:
        return Copy;
    case Qt::MoveAction:
        return Move;
    case Qt::LinkAction:
        return Link;
    default:
        return Copy;
    }
}
#endif

/*!
    \fn void QDropEvent::setPoint(const QPoint &point)
    \compat

    Sets the drop to happen at the given \a point. You do not normally
    need to use this as it will be set internally before your widget
    receives the drop event.
*/ // ### here too - what coordinate system?


/*!
    \class QDragEnterEvent
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
    widget at the given \a point with mouse and keyboard states specified by
    \a buttons and \a modifiers.

    The drag data is passed as MIME-encoded information in \a data, and the
    specified \a actions describe the possible types of drag and drop
    operation that can be performed.

    \warning Do not create a QDragEnterEvent yourself since these
    objects rely on Qt's internal state.
*/
QDragEnterEvent::QDragEnterEvent(const QPoint& point, Qt::DropActions actions, const QMimeData *data,
                                 Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
    : QDragMoveEvent(point, actions, data, buttons, modifiers, DragEnter)
{}

/*! \internal
*/
QDragEnterEvent::~QDragEnterEvent()
{
}

/*!
    Constructs a drag response event containing the \a accepted value,
    indicating whether the drag and drop operation was accepted by the
    recipient.
*/
QDragResponseEvent::QDragResponseEvent(bool accepted)
    : QEvent(DragResponse), a(accepted)
{}

/*! \internal
*/
QDragResponseEvent::~QDragResponseEvent()
{
}

/*!
    \class QDragMoveEvent
    \brief The QDragMoveEvent class provides an event which is sent while a drag and drop action is in progress.

    \ingroup events
    \ingroup draganddrop

    When a widget \l{QWidget::setAcceptDrops()}{accepts drop events},
    it will receive this event repeatedly while the drag is within
    the widget's boundaries. The widget should examine the event to
    see what kind of data it
    \l{QDragMoveEvent::provides()}{provides}, and call the accept()
    function to accept the drop if appropriate.

    The rectangle supplied by the answerRect() function can be used to restrict
    drops to certain parts of the widget. For example, we can check whether the
    rectangle intersects with the geometry of a certain child widget and only
    call \l{QDropEvent::acceptProposedAction()}{acceptProposedAction()} if that
    is the case.

    Note that this class inherits most of its functionality from
    QDropEvent.

    \sa QDragEnterEvent, QDragLeaveEvent, QDropEvent
*/

/*!
    \class QDragLeaveEvent
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

/*! \internal
*/
QDragLeaveEvent::~QDragLeaveEvent()
{
}

/*!
    \class QHelpEvent
    \brief The QHelpEvent class provides an event that is used to request helpful information
    about a particular point in a widget.

    \ingroup events
    \ingroup helpsystem

    This event can be intercepted in applications to provide tooltips
    or "What's This?" help for custom widgets. The type() can be
    either QEvent::ToolTip or QEvent::WhatsThis.

    \sa QToolTip, QWhatsThis, QStatusTipEvent, QWhatsThisClickedEvent
*/

/*!
    Constructs a help event with the given \a type corresponding to the
    widget-relative position specified by \a pos and the global position
    specified by \a globalPos.

    \a type must be either QEvent::ToolTip or QEvent::WhatsThis.

    \sa pos(), globalPos()
*/
QHelpEvent::QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos)
    : QEvent(type), p(pos), gp(globalPos)
{}

/*!
    \fn int QHelpEvent::x() const

    Same as pos().x().

    \sa y(), pos(), globalPos()
*/

/*!
    \fn int QHelpEvent::y() const

    Same as pos().y().

    \sa x(), pos(), globalPos()
*/

/*!
    \fn int QHelpEvent::globalX() const

    Same as globalPos().x().

    \sa x(), globalY(), globalPos()
*/

/*!
    \fn int QHelpEvent::globalY() const

    Same as globalPos().y().

    \sa y(), globalX(), globalPos()
*/

/*!
    \fn const QPoint &QHelpEvent::pos()  const

    Returns the mouse cursor position when the event was generated,
    relative to the widget to which the event is dispatched.

    \sa globalPos(), x(), y()
*/

/*!
    \fn const QPoint &QHelpEvent::globalPos() const

    Returns the mouse cursor position when the event was generated
    in global coordinates.

    \sa pos(), globalX(), globalY()
*/

/*! \internal
*/
QHelpEvent::~QHelpEvent()
{
}

/*!
    \class QStatusTipEvent
    \brief The QStatusTipEvent class provides an event that is used to show messages in a status bar.

    \ingroup events
    \ingroup helpsystem

    Status tips can be set on a widget using QWidget::setStatusTip().
    They are shown in the status bar when the mouse cursor enters the
    widget. Status tips can also be set on actions using
    QAction::setStatusTip(), and they are supported for the item view
    classes through Qt::StatusTipRole.

    \sa QStatusBar, QHelpEvent, QWhatsThisClickedEvent
*/

/*!
    Constructs a status tip event with text specified by \a tip.

    \sa tip()
*/
QStatusTipEvent::QStatusTipEvent(const QString &tip)
    : QEvent(StatusTip), s(tip)
{}

/*! \internal
*/
QStatusTipEvent::~QStatusTipEvent()
{
}

/*!
    \fn QString QStatusTipEvent::tip() const

    Returns the message to show in the status bar.

    \sa QStatusBar::showMessage()
*/

/*!
    \class QWhatsThisClickedEvent
    \brief The QWhatsThisClickedEvent class provides an event that
    can be used to handle hyperlinks in a "What's This?" text.

    \ingroup events
    \ingroup helpsystem

    \sa QWhatsThis, QHelpEvent, QStatusTipEvent
*/

/*!
    Constructs an event containing a URL specified by \a href when a link
    is clicked in a "What's This?" message.

    \sa href()
*/
QWhatsThisClickedEvent::QWhatsThisClickedEvent(const QString &href)
    : QEvent(WhatsThisClicked), s(href)
{}

/*! \internal
*/
QWhatsThisClickedEvent::~QWhatsThisClickedEvent()
{
}

/*!
    \fn QString QWhatsThisClickedEvent::href() const

    Returns the URL that was clicked by the user in the "What's
    This?" text.
*/

/*!
    \class QActionEvent
    \brief The QActionEvent class provides an event that is generated
    when a QAction is added, removed, or changed.

    \ingroup events

    Actions can be added to widgets using QWidget::addAction(). This
    generates an \l ActionAdded event, which you can handle to provide
    custom behavior. For example, QToolBar reimplements
    QWidget::actionEvent() to create \l{QToolButton}s for the
    actions.

    \sa QAction, QWidget::addAction(), QWidget::removeAction(), QWidget::actions()
*/

/*!
    Constructs an action event. The \a type can be \l ActionChanged,
    \l ActionAdded, or \l ActionRemoved.

    \a action is the action that is changed, added, or removed. If \a
    type is ActionAdded, the action is to be inserted before the
    action \a before. If \a before is 0, the action is appended.
*/
QActionEvent::QActionEvent(int type, QAction *action, QAction *before)
    : QEvent(static_cast<QEvent::Type>(type)), act(action), bef(before)
{}

/*! \internal
*/
QActionEvent::~QActionEvent()
{
}

/*!
    \fn QAction *QActionEvent::action() const

    Returns the action that is changed, added, or removed.

    \sa before()
*/

/*!
    \fn QAction *QActionEvent::before() const

    If type() is \l ActionAdded, returns the action that should
    appear before action(). If this function returns 0, the action
    should be appended to already existing actions on the same
    widget.

    \sa action(), QWidget::actions()
*/

/*!
    \class QHideEvent
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

/*! \internal
*/
QHideEvent::~QHideEvent()
{
}

/*!
    \class QShowEvent
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

/*! \internal
*/
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
    \class QFileOpenEvent
    \brief The QFileOpenEvent class provides an event that will be
    sent when there is a request to open a file.

    \ingroup events

    File open events will be sent to the QApplication::instance()
    when the operating system requests that a file be opened. This is
    a high-level event that can be caused by different user actions
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

/*! \internal
*/
QFileOpenEvent::~QFileOpenEvent()
{
}

/*!
    \fn QString QFileOpenEvent::file() const

    Returns the file that is being opened.
*/

/*!
    \internal
    \class QToolBarChangeEvent
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

/*! \internal
*/
QToolBarChangeEvent::~QToolBarChangeEvent()
{
}

/*!
    \fn bool QToolBarChangeEvent::toggle() const
    \internal
*/

/*
    \fn Qt::ButtonState QToolBarChangeEvent::state() const

    Returns the keyboard modifier flags at the time of the event.

    The returned value is a selection of the following values,
    combined using the OR operator:
    Qt::ShiftButton, Qt::ControlButton, Qt::MetaButton, and Qt::AltButton.
*/

QShortcutEvent::QShortcutEvent(const QKeySequence &key, int id, bool ambiguous)
    : QEvent(Shortcut), sequence(key), ambig(ambiguous), sid(id)
{}

QShortcutEvent::~QShortcutEvent()
{
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QEvent *e) {
#ifndef Q_BROKEN_DEBUG_STREAM
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
    qWarning("This compiler doesn't support streaming QEvent to QDebug");
    return dbg;
    Q_UNUSED(e);
#endif
}
#endif

/*!
    \internal

    \class QClipboardEvent
    \ingroup events

    \brief The QClipboardEvent class provides the parameters used in a clipboard event.

    This class is for internal use only, and exists to aid the clipboard on various
    platforms to get all the information it needs.  Use QEvent::Clipboard instead.
*/

QClipboardEvent::QClipboardEvent(QEventPrivate *data)
    : QEvent(QEvent::Clipboard)
{ d = data; }

QClipboardEvent::~QClipboardEvent()
{
}

/*!
    \class QShortcutEvent
    \brief The QShortcutEvent class provides an event which is generated when
    the user presses a key combination.

    \ingroup events

    Normally you don't need to use this class directly; QShortcut
    provides a higher-level interface to handle shortcut keys.

    \sa QShortcut
*/

/*!
    \fn QShortcutEvent::QShortcutEvent(const QKeySequence &key, int id, bool ambiguous = false)

    Constructs a shortcut event for the given \a key press,
    associated with the QShortcut ID \a id.

    \a ambiguous specifies whether there is more than one QShortcut
    for the same key sequence.
*/

/*!
    \fn QShortcutEvent::~QShortcutEvent()

    Destroys the event object.
*/

/*!
    \fn const QKeySequence &QShortcutEvent::key()

    Returns the key sequence that triggered the event.
*/

/*!
    \fn int QShortcutEvent::shortcutId()

    Returns the ID of the QShortcut object for which this event was
    generated.

    \sa QShortcut::id()
*/

/*!
    \fn bool QShortcutEvent::isAmbiguous()

    Returns true if the key sequence that triggered the event is
    ambiguous.

    \sa QShortcut::activatedAmbiguously()
*/

/*!
    \class QWindowStateChangeEvent
    \ingroup events

    \brief The QWindowStateChangeEvent class provides the window state before a
    window state change.
*/

/*! \fn Qt::WindowStates QWindowStateChangeEvent::oldState() const

    Returns the state of the window before the change.
*/

/*! \internal
 */
QWindowStateChangeEvent::QWindowStateChangeEvent(Qt::WindowStates s)
    : QEvent(WindowStateChange), ostate(s)
{
}

/*! \internal
*/
QWindowStateChangeEvent::~QWindowStateChangeEvent()
{
}

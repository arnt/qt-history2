/****************************************************************************
**
** Implementation of event classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
/*!
    \class QInputEvent qevent.h
    \ingroup events

    \brief The QInputEvent class is the base class for events that
    describe user input.

*/

/*!
    \fn QInputEvent::QInputEvent(Type type)

    \internal
*/

/*!
    \fn bool QInputEvent::isAccepted() const

    Returns the accept flag of the event object. It is set by default.

    \sa accept(), ignore()
*/

/*!
    \fn void QInputEvent::accept()

    Setting the accept parameter indicates that the event receiver
    wants the event. Unwanted events might be propagated to the parent
    widget.

    By default, the accept flag is set.

    \sa ignore()
*/


/*!
    \fn void QInputEvent::ignore()

    Clears the accept flag parameter of the event object.

    Clearing the accept parameter indicates that the event receiver
    does not want the event. Unwanted events might be propgated to the
    parent widget.

    By default, the accept flag is set.

    \sa accept()
*/



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
    \fn QMouseEvent::QMouseEvent(Type type, const QPoint &position, int button, int state)

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

QMouseEvent::QMouseEvent(Type type, const QPoint &pos, int button, int state)
    : QInputEvent(type), p(pos), b(button),s((ushort)state) {
        g = QCursor::pos();
}


/*!
    \fn QMouseEvent::QMouseEvent(Type type, const QPoint &position, const QPoint &globalPos, int button, int state)

    Constructs a mouse event object.

    The \a type parameter must be \c QEvent::MouseButtonPress,
    \c QEvent::MouseButtonRelease, \c QEvent::MouseButtonDblClick,
    or \c QEvent::MouseMove.

    The \a position is the mouse cursor's position relative to the
    receiving widget. The cursor's position in global coordinates
    is specified by \a globalPos.
    The \a button that caused the event is given as a value from
    the \l Qt::ButtonState enum. If the event \a type is
    \c MouseMove, the appropriate button for this event is
    \c Qt::NoButton (0).
    The \a state is the \link Qt::ButtonState Qt::ButtonState \endlink
    at the time of the event.

*/

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
    \fn Qt::ButtonState QMouseEvent::button() const

    Returns the button that caused the event.

    Possible return values are \c Qt::LeftButton, \c Qt::RightButton, \c
    Qt::MidButton, and \c Qt::NoButton.

    Note that the returned value is always \c Qt::NoButton for mouse move
    events.

    \sa state() Qt::ButtonState
*/


/*!
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
    \fn Qt::ButtonState QMouseEvent::stateAfter() const

    Returns the button state immediately after the event.

    \sa state() Qt::ButtonState
*/
Qt::ButtonState QMouseEvent::stateAfter() const
{
    return Qt::ButtonState(state()^button());
}



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
    \fn QWheelEvent::QWheelEvent(const QPoint &pos, int delta, int state, Qt::Orientation orient = Qt::Vertical);

    Constructs a wheel event object.

    The \a position provides the location of the mouse cursor within
    the widget. The globalPos() is initialized to QCursor::pos()
    which is usually, but not always, correct.
    Use the other constructor if you need to specify the global
    position explicitly. \a delta contains the rotation distance,
    \a state holds the keyboard modifier flags at the time of the
    event, and \a orient holds the wheel's orientation.

    \sa pos() delta() state()
*/
#ifndef QT_NO_WHEELEVENT
QWheelEvent::QWheelEvent(const QPoint &pos, int delta, int state, Qt::Orientation orient)
    : QInputEvent(Wheel), p(pos), d(delta), s((ushort)state), o(orient)
{
    g = QCursor::pos();
}
#endif
/*!
    \fn QWheelEvent::QWheelEvent(const QPoint &position, const QPoint &globalPos, int delta, int state, Qt::Orientation orient = Qt::Vertical )

    Constructs a wheel event object.

    The \a position provides the location of the mouse cursor
    within the widget. The position in global coordinates is specified
    by \a globalPos. \a delta contains the rotation distance, \a state
    holds the keyboard modifier flags at the time of the event, and
    \a orient holds the wheel's orientation.

    \sa pos() globalPos() delta() state()
*/

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


/*!
    \fn Qt::ButtonState QWheelEvent::state() const

    Returns the keyboard modifier flags at the time of the event.

    The returned value is a selection of the following values,
    combined using the OR operator:
    \c Qt::ShiftButton, \c Qt::ControlButton, and \c Qt::AltButton.
*/


/*!
    \enum Qt::Modifier

    This enum describes the keyboard modifier keys supported by Qt.

    \value SHIFT The Shift keys provided on all standard keyboards.
    \value META The Meta keys.
    \value CTRL The Ctrl keys.
    \value ALT The normal Alt keys, but not keys like AltGr.
    \value UNICODE_ACCEL The shortcut is specified as a Unicode code
    point, not as a Qt Key.
    \omitvalue MODIFIER_MASK
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
    \fn QKeyEvent::QKeyEvent(Type type, int key, int state, const QString& text, bool autorep, ushort count)

    Constructs a key event object.

    The \a type parameter must be \c QEvent::KeyPress or \c
    QEvent::KeyRelease. If \a key is 0, the event is not a result of
    a known key; for example, it may be the result of a compose
    sequence or keyboard macro. The \a state holds the keyboard
    modifiers, and the given \a text is the Unicode text that the
    key generated. If \a autorep is true, isAutoRepeat() will be
    true. \a count is the number of keys involved in the event.
*/

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
    \fn Qt::ButtonState QKeyEvent::state() const

    Returns the keyboard modifier flags that existed immediately
    before the event occurred.

    The returned value is a selection of the following values,
    combined using the OR operator:
    \c Qt::ShiftButton, \c Qt::ControlButton, \c Qt::AltButton, and
    \c Qt::MetaButton.

    \sa stateAfter()
*/

/*!
    \fn Qt::ButtonState QKeyEvent::stateAfter() const

    Returns the keyboard modifier flags that existed immediately
    after the event occurred.

    \warning This function cannot be trusted.

    \sa state()
*/
//###### We must check with XGetModifierMapping
Qt::ButtonState QKeyEvent::stateAfter() const
{
    if (key() == Qt::Key_Shift)
        return Qt::ButtonState(state()^Qt::ShiftButton);
    if (key() == Qt::Key_Control)
        return Qt::ButtonState(state()^Qt::ControlButton);
    if (key() == Qt::Key_Alt)
        return Qt::ButtonState(state()^Qt::AltButton);
    if (key() == Qt::Key_Meta)
        return Qt::ButtonState(state()^Qt::MetaButton);
    return state();
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
    \value Key_Clear
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
    \value Key_division
    \value Key_ydiaeresis

    Multimedia keys

    \value Key_Back
    \value Key_Forward
    \value Key_Stop
    \value Key_Refresh

    \value Key_VolumeDown
    \value Key_VolumeMute
    \value Key_VolumeUp
    \value Key_BassBoost
    \value Key_BassUp
    \value Key_BassDown
    \value Key_TrebleUp
    \value Key_TrebleDown

    \value Key_MediaPlay
    \value Key_MediaStop
    \value Key_MediaPrev
    \value Key_MediaNext
    \value Key_MediaRecord

    \value Key_HomePage
    \value Key_Favorites
    \value Key_Search
    \value Key_Standby
    \value Key_OpenUrl

    \value Key_LaunchMail
    \value Key_LaunchMedia
    \value Key_Launch0
    \value Key_Launch1
    \value Key_Launch2
    \value Key_Launch3
    \value Key_Launch4
    \value Key_Launch5
    \value Key_Launch6
    \value Key_Launch7
    \value Key_Launch8
    \value Key_Launch9
    \value Key_LaunchA
    \value Key_LaunchB
    \value Key_LaunchC
    \value Key_LaunchD
    \value Key_LaunchE
    \value Key_LaunchF

    \value Key_MediaLast

    \value Key_unknown

    \omitvalue Key_Direction_L
    \omitvalue Key_Direction_R

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
    \fn QFocusEvent::QFocusEvent(Type type)

    Constructs a focus event object.

    The \a type parameter must be either \c QEvent::FocusIn or \c
    QEvent::FocusOut.
*/



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
    \fn QPaintEvent::QPaintEvent(const QRegion &paintRegion)

    Constructs a paint event object with the region that needs to
    be updated. The region is specified by \a paintRegion.
*/

/*!
    \fn QPaintEvent::QPaintEvent(const QRect &paintRect)

    Constructs a paint event object with the rectangle that needs
    to be updated. The region is specified by \a paintRect.
*/

/*!
    \fn QPaintEvent::QPaintEvent(const QRegion &region, const QRect &rectangle)

    Constructs a paint event object with both a \a region and
    a \a rectangle, both of which represent the area of the
    widget that needs to be updated.

*/

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
    \fn QMoveEvent::QMoveEvent(const QPoint &pos, const QPoint &oldPos)

    Constructs a move event with the new and old widget positions,
    \a pos and \a oldPos respectively.
*/

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
    \fn QResizeEvent::QResizeEvent(const QSize &size, const QSize &oldSize)

    Constructs a resize event with the new and old widget sizes, \a
    size and \a oldSize respectively.
*/

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
    the \c Qt::WDestructiveClose flag). If it refuses to accept the close
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
    with the \c Qt::WDestructiveClose widget flag. This is very useful for
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
    \fn QCloseEvent::QCloseEvent()

    Constructs a close event object.

    \sa accept()
*/


/*!
   \class QIconDragEvent qevent.h
   \brief The QIconDragEvent class indicates that a main icon drag has begun.

   \ingroup events

   Icon drag events are sent to widgets when the main icon of a window
   has been dragged away. On Mac OS X, this happens when the proxy
   icon of a window is dragged off the title bar.

   It is normal to begin using drag and drop in response to this
   event.

   \sa \link dnd.html Drag-and-drop documentation\endlink QImageDrag
   QDragObject
*/

/*!
    \fn QIconDragEvent::QIconDragEvent()

    Constructs an icon drag event object with the accept flag set to
    false.

    \sa accept()
*/

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
    \fn QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos, int state)

    Constructs a context menu event object with the accept parameter
    flag set to false.

    The \a reason parameter must be \c QContextMenuEvent::Mouse or
    \c QContextMenuEvent::Keyboard.

    The \a pos parameter specifies the mouse position relative to the
    receiving widget. \a globalPos is the mouse position in absolute
    coordinates. \a state is the Qt::ButtonState at the time of the event.
*/


/*!
    \fn QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, int state)

    Constructs a context menu event object with the accept parameter
    flag set to false.

    The \a reason parameter must be \c QContextMenuEvent::Mouse or \c
    QContextMenuEvent::Keyboard.

    The \a pos parameter specifies the mouse position relative to the
    receiving widget. \a state is the Qt::ButtonState at the time of the
    event.

    The globalPos() is initialized to QCursor::pos(), which may not be
    appropriate. Use the other constructor to specify the global
    position explicitly.
*/

QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, int state)
    : QInputEvent(ContextMenu), p(pos), reas(reason), s((ushort)state)
{
    gp = QCursor::pos();
}

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
    \class QIMEvent qevent.h
    \brief The QIMEvent class provides parameters for input method events.

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
    events: IMStartEvent, IMComposeEvent, and IMEndEvent. When a
    new input context is created, an IMStartEvent will be sent to the
    widget and delivered to the \l QWidget::imStartEvent() function.
    The widget can then update internal data structures to reflect
    this.

    After this, an IMComposeEvent will be sent to the widget for
    every key the user presses. It will contain the current
    composition string the widget has to show and the current cursor
    position within the composition string. This string is temporary
    and can change with every key the user types, so the widget will
    need to store the state before the composition started (the state
    it had when it received the IMStartEvent). IMComposeEvents will be
    delivered to the \l QWidget::imComposeEvent() function.

    Usually, widgets try to mark the part of the text that is part of
    the current composition in a way that is visible to the user. A
    commonly used visual cue is to use a dotted underline.

    After the user has selected the final string, an IMEndEvent will
    be sent to the widget. The event contains the final string the
    user selected, and could be empty if they canceled the
    composition. This string should be accepted as the final text the
    user entered, and the intermediate composition string should be
    cleared. These events are delivered to \l QWidget::imEndEvent().

    If the user clicks another widget, taking the focus out of the
    widget where the composition is taking place, the IMEndEvent will
    be sent and the string it holds will be the result of the
    composition up to that point (which may be an empty string).

*/

/*!
    \fn QIMEvent::QIMEvent(Type type, const QString &text, int cursorPosition, int selLength)

    Constructs a new QIMEvent with the accept flag set to false.
    The event \a type can be QEvent::IMStartEvent,
    QEvent::IMComposeEvent, or QEvent::IMEndEvent. The \a text contains
    the current compostion string, and \a cursorPosition is the current
    position of the cursor inside the \a text.
    \a selLength characters are selected (default is 0).
*/

/*!
    \fn const QString &QIMEvent::text() const

    Returns the composition text. This is a null string for an
    IMStartEvent, and contains the final accepted string (which may be
    empty) in the IMEndEvent.
*/

/*!
    \fn int QIMEvent::cursorPos() const

    Returns the current cursor position inside the composition string.
    Will return -1 for IMStartEvent and IMEndEvent.
*/

/*!
    \fn int QIMEvent::selectionLength() const

    Returns the number of characters in the composition string,
    starting at cursorPos(), that should be marked as selected by the
    input widget receiving the event.
    Will return 0 for IMStartEvent and IMEndEvent.
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
    \omit
    \value Menu  A menu button was pressed (currently unimplemented).
*/

/*!
  \fn QTabletEvent::QTabletEvent(Type type, const QPoint &position,
                                  const QPoint &globalPos, int device,
                                  int pressure, int xTilt, int yTilt,
                                  const QPair<int,int> &uId)

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

QTabletEvent::QTabletEvent(Type t, const QPoint &pos, const QPoint &globalPos, int device,
                            int pressure, int xTilt, int yTilt,
                            const QPair<int, int> &uId)
    : QInputEvent(t),
      mPos(pos),
      mGPos(globalPos),
      mDev(device),
      mPress(pressure),
      mXT(xTilt),
      mYT(yTilt),
      mType(uId.first),
      mPhy(uId.second)
{
}

/*!
  \obsolete
  \fn QTabletEvent::QTabletEvent(const QPoint &pos, const QPoint &globalPos, int device, int pressure, int xTilt, int yTilt, const QPair<int,int> &uId)

    Constructs a tablet event object. The position when the event
    occurred is is given in \a pos and \a globalPos. \a device
    contains the \link TabletDevice device type\endlink, \a pressure
    contains the pressure exerted on the \a device, \a xTilt and \a
    yTilt contain the \a device's degrees of tilt from the x and y
    axis respectively. The \a uId contains an event id.

    On Irix, \a globalPos will contain the high-resolution coordinates
    received from the tablet device driver, instead of from the
    windowing system.

  \sa pos() globalPos() device() pressure() xTilt() yTilt() uniqueId()
*/

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

    \img tabletevent-xtilt.png

    \sa yTilt()
*/

/*!
    \fn int QTabletEvent::yTilt() const

    Returns the angle between the device (a pen, for example) and the
    perpendicular in the direction of the y axis.
    Positive values are towards the bottom of the tablet. The angle is
    within the range -60 to +60 degrees.

    \img tabletevent-ytilt.png

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
    \fn QPair<int, int> QTabletEvent::uniqueId()

    Returns a unique ID for the current device, making it possible
    to differentiate between multiple devices being used at the same
    time on the tablet. The \c first member contains a value for the
    type, the \c second member contains a physical ID obtained from
    the device. Each combination of these values is unique.

    Note that the \c first value will vary due to different driver
    implementations on each platform supported by Qt.

    It is possible to generate a unique ID for any Wacom device.

*/

/*!
    \fn QDragMoveEvent::QDragMoveEvent(const QPoint &point, Type type)

    Creates a QDragMoveEvent of the required \a type indicating
    that the mouse is at the \a point given within a widget.

    \warning Do not create a QDragMoveEvent yourself since these
    objects rely on Qt's internal state.
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
    \fn void QDragMoveEvent::ignore(const QRect &rectangle)

    The opposite of the accept(const QRect&) function.
    Moves within the \a rectangle are not acceptable, and will be
    ignored.
*/

/*!
    \fn QRect QDragMoveEvent::answerRect() const

    Returns the rectangle for which the acceptance of the move event
    applies.
*/



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
    To get the data, use encodedData(), or preferably, the decode()
    methods of existing QDragObject subclasses, such as
    QTextDrag::decode(), or your own subclasses.

    \sa acceptAction()
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

    When a drag and drop action is completed, the target is expected
    to perform an Action on the data provided by the source. This
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

/*!
    \fn bool QDropEvent::isActionAccepted () const

    Returns true if the drop action was accepted by the drop site;
    otherwise returns false.
*/


/*!
    \fn void QDropEvent::setPoint (const QPoint &point)

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
    \fn QDragEnterEvent::QDragEnterEvent (const QPoint &point)

    Constructs a QDragEnterEvent that represents a drag entering
    a widget at the given \a point.

    \warning Do not create a QDragEnterEvent yourself since these
    objects rely on Qt's internal state.
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
    \fn QDragLeaveEvent::QDragLeaveEvent()

    Constructs a QDragLeaveEvent.

    \warning Do not create a QDragLeaveEvent yourself since these
    objects rely on Qt's internal state.
*/

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
    \fn QHideEvent::QHideEvent()

    Constructs a QHideEvent.
*/

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
    \fn QShowEvent::QShowEvent()

    Constructs a QShowEvent.
*/


/*!
  \fn QByteArray QDropEvent::data(const char* f) const

  \obsolete

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
    \fn QFileOpenEvent::QFileOpenEvent(const QString &file)

    \internal

    Constructs a file open event for the given \a file.
*/

/*!
    \fn QString QFileOpenEvent::file() const

    Returns the file that is being opened.
*/


#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QEvent *e) {

    // More useful event output could be added here

    if (!e)
        return dbg << "QEvent(this = 0x0)";
    const char *n;
    switch (e->type()) {
    case QEvent::Timer:
        n = "Timer";
        break;
    case QEvent::MouseButtonPress:
        n = "MouseButtonPress";
        break;
    case QEvent::MouseButtonRelease:
        n = "MouseButtonRelease";
        break;
    case QEvent::MouseButtonDblClick:
        n = "MouseButtonDblClick";
        break;
    case QEvent::MouseMove:
        n = "MouseMove";
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
                        << ", " << ke->key()
                        << ", " << hex << ke->state()
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
}
#endif

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

#include "qcoreevent.h"
#include "qcoreapplication.h"
#include "qcoreapplication_p.h"

/*!
    \class QEvent qcoreevent.h
    \brief The QEvent class is the base class of all
    event classes. Event objects contain event parameters.

    \ingroup events
    \ingroup environment

    Qt's main event loop (QApplication::exec()) fetches native window
    system events from the event queue, translates them into QEvents,
    and sends the translated events to QObjects.

    In general, events come from the underlying window system
    (spontaneous() returns true), but it is also possible to manually
    send events using QApplication::sendEvent() and
    QApplication::postEvent() (spontaneous() returns false).

    QObjects receive events by having their QObject::event() function
    called. The function can be reimplemented in subclasses to
    customize event handling and add additional event types;
    QWidget::event() is a notable example. By default, events are
    dispatched to event handlers like QObject::timerEvent() and
    QWidget::mouseMoveEvent(). QObject::installEventFilter() allows an
    object to intercept events destined for another object.

    The basic QEvent contains only an event type parameter and an \l
    accept flag.  The accept flag set with accept(), and cleared with
    ignore(). It is set by default, but don't rely on this as
    subclasses may choose to clear it in their constructor.

    Subclasses of QEvent contain additional parameters that describe
    the particular event.

    \sa QObject::event() QObject::installEventFilter()
    QWidget::event() QApplication::sendEvent()
    QApplication::postEvent() QApplication::processEvents()
*/


/*!
    \enum QEvent::Type

    This enum type defines the valid event types in Qt. The event
    types and the specialized classes for each type are as follows:

    \value None  Not an event.

    \value Accessibility      Accessibility information is requested.
    \value AccessibilityHelp  Used to query for additional information about complex widgets.
                              \l{QAccessibleEvent}
    \value ActionChanged      An action has been changed.
    \value ActionAdded        A new action has been added.
    \value ActionRemoved      An action has been removed.
    \value ActivationChange   A widget's top-level window activation state has changed.
    \value ApplicationActivated The application has been made available to the user.
    \value ApplicationDeactivated The application has been suspended, and is
                                  unavailable to the user.
    \value ApplicationFontChange    The default application font has changed.
    \value ApplicationLayoutDirectionChange The default application layout
                                            direction has changed.
    \value ApplicationPaletteChange The default application palette has changed.
    \value ApplicationWindowIconChange The application's icon has changed.
    \value ChildAdded     An object gets a child, \l{QChildEvent}.
    \value ChildPolished  A widget child gets polished, \l{QChildEvent}.
    \value ChildRemoved   An object loses a child, \l{QChildEvent}.
    \value Clipboard      The clipboard contents have changed.
    \value Close          Widget was closed (permanently), \l{QCloseEvent}.
    \value ContextMenu    Context popup menu, \l{QContextMenuEvent}
    \value Create         Reserved.
    \value DeferredDelete    The object will be deleted after it has cleaned up.
    \value Destroy      Reserved.
    \value DragEnter    The cursor enters a widget during a drag and drop action, \l{QDragEnterEvent}.
    \value DragLeave    The cursor leaves a widget during a drag and drop action, \l{QDragLeaveEvent}.
    \value DragMove     A drag and drop action is in progress, \l{QDragMoveEvent}.
    \value Drop  A drag and drop action is completed, \l{QDropEvent}.
    \value EnabledChange Widget's enabled state has changed
    \value Enter  Mouse enters widget's boundaries.
    \value FileOpen File open request.
    \value FocusIn  Widget gains keyboard focus, \l{QFocusEvent}.
    \value FocusOut  Widget loses keyboard focus, \l{QFocusEvent}.
    \value FontChange Widget's font has changed
    \value Hide  Widget was hidden, \l{QHideEvent}.
    \value HideToParent  A child widget has been hidden.
    \value IconTextChange Widget's icon text has been changed
    \value KeyPress  Key press (including Shift, for example), \l{QKeyEvent}.
    \value KeyRelease  Key release, \l{QKeyEvent}.
    \value LanguageChange  The application translation changed, \l{QTranslator}
    \value LayoutDirectionChange  The direction of layouts changed
    \value LayoutRequest  Widget layout needs to be redone.
    \value Leave  Mouse leaves widget's boundaries.
    \value LocaleChange  The system locale changed
    \value MetaCall
    \value ModifiedChange Widgets modification state has been changed
    \value MouseButtonDblClick  Mouse press again, \l{QMouseEvent}.
    \value MouseButtonPress  Mouse press, \l{QMouseEvent}.
    \value MouseButtonRelease  Mouse release, \l{QMouseEvent}.
    \value MouseMove  Mouse move, \l{QMouseEvent}.
    \value MouseTrackingChange The mouse tracking state has changed.
    \value Move  Widget's position changed, \l{QMoveEvent}.
    \value Paint  Screen update necessary, \l{QPaintEvent}.
    \value PaletteChange  Palette of the widget changed.
    \value Polish The widget is polished.
    \value PolishRequest The widget should be polished.
    \omitvalue QWSUpdate
    \value Quit  Reserved.
    \value ParentChange  Reserved.
    \value ThreadChange Reserved.
    \value Resize  Widget's size changed, \l{QResizeEvent}.
    \value Shortcut Key press in child for shortcut key handling, \l{QKeyEvent}.
    \value ShortcutOverride  Key press in child, for overriding shortcut key handling, \l{QKeyEvent}.
    \value Show  Widget was shown on screen, \l{QShowEvent}.
    \value ShowToParent  A child widget has been shown.
    \value ShowWindowRequest  Widget's window should be shown (obsolete).
    \value SockAct  Socket activated, used to implement \l{QSocketNotifier}.
    \value Speech  Reserved for speech input.
    \value StatusTip
    \value StyleChange Widget's style has been changed
    \value TabletMove  A Wacom Tablet Move Event.
    \value TabletPress  A Wacom Tablet Press Event
    \value TabletRelease  A Wacom Tablet Release Event
    \value Timer  Regular timer events, \l{QTimerEvent}.
    \value ToolBarChange The toolbar button is toggled on Mac.
    \value ToolTip
    \value UpdateRequest The widget should be repainted.
    \value WhatsThis The widget should reveal "What's This?" help.
    \value WhatsThisClicked A link in a widget's "What's This?" help was clicked.
    \value QueryWhatsThis The widget should accept the event if it has What's This help
    \value EnterWhatsThisMode Send to toplevel widgets when the application enters What's This mode
    \value LeaveWhatsThisMode Send to toplevel widgets when the application leaves What's This mode
    \value Wheel  Mouse wheel rolled, \l{QWheelEvent}.
    \value WindowActivate  Window was activated.
    \value WindowBlocked
    \value WindowDeactivate  Window was deactivated.
    \value WindowIconChange
    \value WindowStateChange The window's state, i.e. minimized, maximized or full-screen, has changed, \l{QWindowStateChangeEvent}. See also \l{QWidget::windowState()}.
    \value WindowTitleChange
    \value WindowUnblocked
    \value ZOrderChange The widget's z-order has changed. This event is never sent to top level windows.

    \value User  User-defined event.
    \value MaxUser  Last user event id.

    \omitvalue ActivateControl
    \omitvalue DeactivateControl
    \omitvalue DragResponse
    \omitvalue EmbeddingControl
    \omitvalue HelpRequest
    \omitvalue IconDrag
    \omitvalue OkRequest
    \omitvalue Style

    User events should have values between User and MaxUser inclusive.
*/

/*!
    Contructs an event object of type \a type.
*/
QEvent::QEvent(Type type)
    : d(0), t(type), posted(false), spont(false), m_accept(true)
{}

/*!
  Destroys the event. If it was \link
  QApplication::postEvent() posted \endlink,
  it will be removed from the list of events to be posted.
*/

QEvent::~QEvent()
{
    if (posted && QCoreApplication::instance())
        QCoreApplicationPrivate::removePostedEvent(this);
}


/*!
    \property  QEvent::accepted
    the accept flag of the event object

    Setting the accept parameter indicates that the event receiver
    wants the event. Unwanted events might be propagated to the parent
    widget. By default, isAccepted() is set to true, but don't rely on
    this as subclasses may choose to clear it in their constructor.

    For convenience, the accept flag can also be set with accept(),
    and cleared with ignore().
*/

/*!
    \fn void QEvent::accept()

    Sets the accept flag of the event object, the equivalent of
    callign setAccepted(true).

    Setting the accept parameter indicates that the event receiver
    wants the event. Unwanted events might be propagated to the parent
    widget.

    \sa ignore()
*/


/*!
    \fn void QEvent::ignore()

    Clears the accept flag parameter of the event object, the
    equivalent of calling setAccepted(false).

    Clearing the accept parameter indicates that the event receiver
    does not want the event. Unwanted events might be propgated to the
    parent widget.

    \sa accept()
*/


/*!
    \fn QEvent::Type QEvent::type() const

    Returns the event type.
*/

/*!
    \fn bool QEvent::spontaneous() const

    Returns true if the event originated outside the application
    (a system event); otherwise returns false.
*/


/*!
    \class QTimerEvent qcoreevent.h
    \brief The QTimerEvent class contains parameters that describe a
    timer event.

    \ingroup events

    Timer events are sent at regular intervals to objects that have
    started one or more timers. Each timer has a unique identifier. A
    timer is started with QObject::startTimer().

    The QTimer class provides a high-level programming interface that
    uses signals instead of events. It also provides single-shot timers.

    The event handler QObject::timerEvent() receives timer events.

    \sa QTimer, QObject::timerEvent(), QObject::startTimer(),
    QObject::killTimer()
*/

/*!
    Constructs a timer event object with the timer identifier set to
    \a timerId.
*/
QTimerEvent::QTimerEvent(int timerId)
    : QEvent(Timer), id(timerId)
{}

/*! \internal
*/
QTimerEvent::~QTimerEvent()
{
}

/*!
    \fn int QTimerEvent::timerId() const

    Returns the unique timer identifier, which is the same identifier
    as returned from QObject::startTimer().
*/

/*!
    \class QChildEvent qcoreevent.h
    \brief The QChildEvent class contains event parameters for child object
    events.

    \ingroup events

    Child events are sent immediately to objects when children are
    added or removed.

    In both cases you can only rely on the child being a QObject, or
    if isWidgetType() returns true, a QWidget. This is because
    in the \c ChildAdded case the child is not yet fully constructed;
    in the \c ChildRemoved case it might have already been destructed.

    The handler for these events is QObject::childEvent().
*/

/*!
    Constructs a child event object of a particular \a type for the
    \a child.
*/
QChildEvent::QChildEvent(Type type, QObject *child)
    : QEvent(type), c(child)
{}

/*! \internal
*/
QChildEvent::~QChildEvent()
{
}

/*!
    \fn QObject *QChildEvent::child() const

    Returns the child object that was added or removed.
*/

/*!
    \fn bool QChildEvent::added() const

    Returns true if a child has been added to the object; otherwise
    returns false.
*/

/*!
    \fn bool QChildEvent::removed() const

    Returns true if a child has been removed from the object; otherwise
    returns false.
*/

/*!
    \fn bool QChildEvent::polished() const

*/

/*!
    \class QCustomEvent
    \brief The QCustomEvent class provides support for custom events.

    \compat

    QCustomEvent has a \c{void *} that can be used to store custom
    data.

    In Qt 3, QObject::customEvent() took a QCustomEvent pointer. We
    found out that this approach was unsatisfactory, because
    there was often no safe way of deleting the data held in the
    \c{void *}.

    In Qt 4, QObject::customEvent() takes a plain QEvent pointer.
    You can add custom data by subclassing.

    \sa QObject::customEvent(), QApplication::notify()
*/


/*!
    \fn QCustomEvent::QCustomEvent(int type, void *data)

    \compat

    Constructs a custom event object with the event \a type and a
    pointer to \a data. The value of \a type must be at least as
    large as QEvent::User. By default, the data pointer is set to 0.


*/
#ifdef QT3_SUPPORT
QCustomEvent::QCustomEvent(int type, void *data)
    : QEvent(static_cast<Type>(type))
{
    d = reinterpret_cast<QEventPrivate *>(data);
}

/*! \internal
*/
QCustomEvent::~QCustomEvent()
{
}
#endif
/*!
    \fn void QCustomEvent::setData(void *data)

    \compat

    Sets the generic data pointer to \a data.

    \sa data()
*/

/*!
    \fn void *QCustomEvent::data() const

    \compat

    Returns a pointer to the generic event data.

    \sa setData()
*/

/*!
    \fn bool QChildEvent::inserted() const

    \compat

    A child has been inserted if the event's type() is ChildInserted.
*/



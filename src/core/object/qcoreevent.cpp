#include "qcoreevent.h"
#include "qcoreapplication.h"

/*!
    \class QEvent qcoreevent.h
    \brief The QEvent class is the base class of all
    event classes. Event objects contain event parameters.

    \ingroup events
    \ingroup environment

    Qt's main event loop (QApplication::exec()) fetches native window
    system events from the event queue, translates them into QEvents
    and sends the translated events to QObjects.

    In general, events come from the underlying window system
    (spontaneous() returns TRUE) but it is also possible to manually
    send events using QApplication::sendEvent() and
    QApplication::postEvent() (spontaneous() returns FALSE).

    QObjects receive events by having their QObject::event() function
    called. The function can be reimplemented in subclasses to
    customize event handling and add additional event types;
    QWidget::event() is a notable example. By default, events are
    dispatched to event handlers like QObject::timerEvent() and
    QWidget::mouseMoveEvent(). QObject::installEventFilter() allows an
    object to intercept events destined for another object.

    The basic QEvent contains only an event type parameter.
    Subclasses of QEvent contain additional parameters that describe
    the particular event.

    \sa QObject::event() QObject::installEventFilter()
    QWidget::event() QApplication::sendEvent()
    QApplication::postEvent() QApplication::processEvents()
*/


/*!
    \enum Qt::ButtonState

    This enum type describes the state of the mouse and the modifier
    buttons.

    \value NoButton  used when the button state does not refer to any
    button (see QMouseEvent::button()).
    \value LeftButton  set if the left button is pressed, or if this
    event refers to the left button. (The left button may be
    the right button on left-handed mice.)
    \value RightButton  the right button.
    \value MidButton  the middle button.
    \value ShiftButton  a Shift key on the keyboard is also pressed.
    \value ControlButton  a Ctrl key on the keyboard is also pressed.
    \value AltButton  an Alt key on the keyboard is also pressed.
    \value MetaButton a Meta key on the keyboard is also pressed.
    \value Keypad  a keypad button is pressed.
    \value KeyButtonMask a mask for ShiftButton, ControlButton,
    AltButton and MetaButton.
    \value MouseButtonMask a mask for LeftButton, RightButton and MidButton.
*/

/*!
    \enum QEvent::Type

    This enum type defines the valid event types in Qt. The event
    types and the specialized classes for each type are these:

    \value None  Not an event.

    \value Accel  Key press in child for shortcut key handling, \l{QKeyEvent}.
    \value AccelAvailable internal.
    \value AccelOverride  Key press in child, for overriding shortcut key handling, \l{QKeyEvent}.
    \value Accessibility  Accessibility information is requested
    \value ActivateControl  Internal event used by Qt on some platforms.
    \value ActivationChange Widget's top level window activation state has changed
    \value ApplicationFontChange  Default application font changed.
    \value ApplicationPaletteChange  Default application palette changed.
    \value ChildAdded  Object gets a child, \l{QChildEvent}.
    \value ChildPolished  Object child gets polished, \l{QChildEvent}.
    \value ChildRemoved  Object loses a child, \l{QChildEvent}.
    \value Clipboard  Clipboard contents have changed.
    \value Close  Widget was closed (permanently), \l{QCloseEvent}.
    \value ContextMenu  Context popup menu, \l{QContextMenuEvent}
    \value Create  Reserved.
    \value DeactivateControl  Internal event used by Qt on some platforms.
    \value DeferredDelete  The object will be deleted after it has cleaned up.
    \value Destroy  Reserved.
    \value DragEnter  A drag-and-drop enters widget, \l{QDragEnterEvent}.
    \value DragLeave  A drag-and-drop leaves widget, \l{QDragLeaveEvent}.
    \value DragMove  A drag-and-drop is in progress, \l{QDragMoveEvent}.
    \value DragResponse  Internal event used by Qt on some platforms.
    \value Drop  A drag-and-drop is completed, \l{QDropEvent}.
    \value EmbeddingControl  Internal event used by Qt on some platforms.
    \value EmitSignal
    \value EnabledChange Widget's enabled state has changed
    \value Enter  Mouse enters widget's boundaries.
    \value FocusIn  Widget gains keyboard focus, \l{QFocusEvent}.
    \value FocusOut  Widget loses keyboard focus, \l{QFocusEvent}.
    \value FontChange Widget's font has changed
    \value HelpRequest  Internal event used by Qt on some platforms.
    \value Hide  Widget was hidden, \l{QHideEvent}.
    \value HideToParent  A child widget has been hidden.
    \value IMCompose  Input method composition is taking place.
    \value IMEnd  The end of input method composition.
    \value IMStart  The start of input method composition.
    \value IconDrag     Internal event used by Qt on some platforms when proxy icon is dragged.
    \value IconTextChange Widget's icon text has been changed
    \value InvokeSlot
    \value KeyPress  Key press (including Shift, for example), \l{QKeyEvent}.
    \value KeyRelease  Key release, \l{QKeyEvent}.
    \value LanguageChange  The application translation changed, \l{QTranslator}
    \value LayoutDirectionChange  The direction of layouts changed
    \value LayoutRequest  Widget layout needs to be redone.
    \value Leave  Mouse leaves widget's boundaries.
    \value LocaleChange  The system locale changed
    \value ModifiedChange Widgets modification state has been changed
    \value MouseButtonDblClick  Mouse press again, \l{QMouseEvent}.
    \value MouseButtonPress  Mouse press, \l{QMouseEvent}.
    \value MouseButtonRelease  Mouse release, \l{QMouseEvent}.
    \value MouseMove  Mouse move, \l{QMouseEvent}.
    \value Move  Widget's position changed, \l{QMoveEvent}.
    \value OkRequest  Internal event used by Qt on some platforms.
    \value Paint  Screen update necessary, \l{QPaintEvent}.
    \value PaletteChange  Palette of the widget changed.
    \value Polish The object is polished.
    \value PolishRequest The object should be polished.
    \value QWSUpdate internal.
    \value Quit  Reserved.
    \value Reparent  Reserved.
    \value Resize  Widget's size changed, \l{QResizeEvent}.
    \value Show  Widget was shown on screen, \l{QShowEvent}.
    \value ShowFullScreen  Widget should be shown full-screen (obsolete).
    \value ShowMaximized  Widget should be shown maximized (obsolete).
    \value ShowMinimized  Widget should be shown minimized (obsolete).
    \value ShowNormal  Widget should be shown normally (obsolete).
    \value ShowToParent  A child widget has been shown.
    \value ShowWindowRequest  Widget's window should be shown (obsolete).
    \value SockAct  Socket activated, used to implement \l{QSocketNotifier}.
    \value Speech  Reserved for speech input.
    \value StatusTip
    \value Style  Internal use only
    \value StyleChange Widget's style has been changed
    \value TabletMove  A Wacom Tablet Move Event.
    \value TabletPress  A Wacom Tablet Press Event
    \value TabletRelease  A Wacom Tablet Release Event
    \value Timer  Regular timer events, \l{QTimerEvent}.
    \value ToolTip
    \value UpdateRequest The widget should be repainted.
    \value WhatsThis
    \value Wheel  Mouse wheel rolled, \l{QWheelEvent}.
    \value WindowActivate  Window was activated.
    \value WindowBlocked
    \value WindowDeactivate  Window was deactivated.
    \value WindowIconChange
    \value WindowStateChange The window's state, i.e. minimized, maximized or full-screen, has changed. See \l{QWidget::windowState()}.
    \value WindowTitleChange
    \value WindowUnblocked

    \value User  User defined event.
    \value MaxUser  Last user event id.

    User events should have values between User and MaxUser inclusive.
*/
/*!
    \fn QEvent::QEvent( Type type )

    Contructs an event object of type \a type.
*/

/*!
  Destroys the event. If it was \link
  QApplication::postEvent() posted \endlink,
  it will be removed from the list of events to be posted.
*/

QEvent::~QEvent()
{
    if ( posted && QCoreApplication::self )
	QCoreApplication::removePostedEvent( this );
}

/*!
    \fn QEvent::Type QEvent::type() const

    Returns the event type.
*/

/*!
    \fn bool QEvent::spontaneous() const

    Returns TRUE if the event originated outside the application, i.e.
    it is a system event; otherwise returns FALSE.
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
    uses signals instead of events. It also provides one-shot timers.

    The event handler QObject::timerEvent() receives timer events.

    \sa QTimer, QObject::timerEvent(), QObject::startTimer(),
    QObject::killTimer(), QObject::killTimers()
*/

/*!
    \fn QTimerEvent::QTimerEvent( int timerId )

    Constructs a timer event object with the timer identifier set to
    \a timerId.
*/

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
    if isWidgetType() returns true, a QWidget (Reason: in the \c
    ChildAdded case the child is not yet fully constructed, in the \c
    ChildRemoved case it might have been destructed already ).

    The handler for these events is QObject::childEvent().
*/

/*!
    \fn QChildEvent::QChildEvent( Type type, QObject *child )

    Constructs a child event object for child \a child with type \a type.
*/

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

    Returns TRUE if the object lost a child; otherwise returns FALSE.
*/

/*!
    \fn bool QChildEvent::polished() const

*/

/*!
    \class QCustomEvent qcoreevent.h
    \brief The QCustomEvent class provides support for custom events.

    \ingroup events

    QCustomEvent is a generic event class for user-defined events.
    User defined events can be sent to widgets or other QObject
    instances using QApplication::postEvent() or
    QApplication::sendEvent(). Subclasses of QWidget can easily
    receive custom events by implementing the QWidget::customEvent()
    event handler function.

    QCustomEvent objects should be created with a type ID that
    uniquely identifies the event type. To avoid clashes with the
    Qt-defined events types, the value should be at least as large as
    the value of the "User" entry in the QEvent::Type enum.

    QCustomEvent contains a generic void* data member that may be used
    for transferring event-specific data to the receiver. Note that
    since events are normally delivered asynchronously, the data
    pointer, if used, must remain valid until the event has been
    received and processed.

    QCustomEvent can be used as-is for simple user-defined event
    types, but normally you will want to make a subclass of it for
    your event types. In a subclass, you can add data members that are
    suitable for your event type.

    Example:
    \code
    class ColorChangeEvent : public QCustomEvent
    {
    public:
	ColorChangeEvent( QColor color )
	    : QCustomEvent( 65432 ), c( color ) {}
	QColor color() const { return c; }
    private:
	QColor c;
    };

    // To send an event of this custom event type:

    ColorChangeEvent* ce = new ColorChangeEvent( blue );
    QApplication::postEvent( receiver, ce );  // Qt will delete it when done

    // To receive an event of this custom event type:

    void MyWidget::customEvent( QCustomEvent * e )
    {
	if ( e->type() == 65432 ) {  // It must be a ColorChangeEvent
	    ColorChangeEvent* ce = (ColorChangeEvent*)e;
	    newColor = ce->color();
	}
    }
    \endcode

    \sa QWidget::customEvent(), QApplication::notify()
*/


/*!
    Constructs a custom event object with event type \a type. The
    value of \a type must be at least as large as QEvent::User. The
    data pointer is set to 0.
*/

QCustomEvent::QCustomEvent( int type )
    : QEvent( (QEvent::Type)type ), d( 0 )
{
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





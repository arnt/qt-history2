#include "qkernelevent.h"
#include "qkernelapplication.h"

/*!
    \class QEvent qkernelevent.h
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
    \value Accessibility  Accessibility information is requested
    \value Timer  Regular timer events, \l{QTimerEvent}.
    \value MouseButtonPress  Mouse press, \l{QMouseEvent}.
    \value MouseButtonRelease  Mouse release, \l{QMouseEvent}.
    \value MouseButtonDblClick  Mouse press again, \l{QMouseEvent}.
    \value MouseMove  Mouse move, \l{QMouseEvent}.
    \value KeyPress  Key press (including Shift, for example), \l{QKeyEvent}.
    \value KeyRelease  Key release, \l{QKeyEvent}.
    \value IMStart  The start of input method composition.
    \value IMCompose  Input method composition is taking place.
    \value IMEnd  The end of input method composition.
    \value FocusIn  Widget gains keyboard focus, \l{QFocusEvent}.
    \value FocusOut  Widget loses keyboard focus, \l{QFocusEvent}.
    \value Enter  Mouse enters widget's boundaries.
    \value Leave  Mouse leaves widget's boundaries.
    \value Paint  Screen update necessary, \l{QPaintEvent}.
    \value UpdateRequest The widget should be repainted.
    \value Move  Widget's position changed, \l{QMoveEvent}.
    \value Resize  Widget's size changed, \l{QResizeEvent}.
    \value Show  Widget was shown on screen, \l{QShowEvent}.
    \value Hide  Widget was hidden, \l{QHideEvent}.
    \value ShowToParent  A child widget has been shown.
    \value HideToParent  A child widget has been hidden.
    \value Close  Widget was closed (permanently), \l{QCloseEvent}.
    \value ShowNormal  Widget should be shown normally.
    \value ShowMaximized  Widget should be shown maximized.
    \value ShowMinimized  Widget should be shown minimized.
    \value ShowFullScreen  Widget should be shown full-screen.
    \value ShowWindowRequest  Widget's window should be shown (obsolete).
    \value DeferredDelete  The object will be deleted after it has
    cleaned up.
    \value Accel  Key press in child for shortcut key handling, \l{QKeyEvent}.
    \value Wheel  Mouse wheel rolled, \l{QWheelEvent}.
    \value ContextMenu  Context popup menu, \l{QContextMenuEvent}
    \value AccelOverride  Key press in child, for overriding shortcut key handling, \l{QKeyEvent}.
    \value AccelAvailable internal.
    \value WindowActivate  Window was activated.
    \value WindowDeactivate  Window was deactivated.
    \value CaptionChange  Widget's caption changed.
    \value IconChange  Widget's icon changed.
    \value ApplicationFontChange  Default application font changed.
    \value PaletteChange  Palette of the widget changed.
    \value ApplicationPaletteChange  Default application palette changed.
    \value Clipboard  Clipboard contents have changed.
    \value SockAct  Socket activated, used to implement \l{QSocketNotifier}.
    \value DragEnter  A drag-and-drop enters widget, \l{QDragEnterEvent}.
    \value DragMove  A drag-and-drop is in progress, \l{QDragMoveEvent}.
    \value DragLeave  A drag-and-drop leaves widget, \l{QDragLeaveEvent}.
    \value Drop  A drag-and-drop is completed, \l{QDropEvent}.
    \value DragResponse  Internal event used by Qt on some platforms.
    \value Polish The object is polished.
    \value PolishRequest The object should be polished.
    \value ChildAdded  Object gets a child, \l{QChildEvent}.
    \value ChildPolished  Object child gets polished, \l{QChildEvent}.
    \value ChildRemoved  Object loses a child, \l{QChildEvent}.
    \value ChildInserted internal.
    \value LayoutRequest  Widget layout needs to be redone.
    \value LayoutHint  internal.
    \value ActivateControl  Internal event used by Qt on some platforms.
    \value DeactivateControl  Internal event used by Qt on some platforms.
    \value LanguageChange  The application translation changed, \l{QTranslator}
    \value LayoutDirectionChange  The direction of layouts changed
    \value LocaleChange  The system locale changed
    \value Quit  Reserved.
    \value Create  Reserved.
    \value Destroy  Reserved.
    \value Reparent  Reserved.
    \value Speech  Reserved for speech input.
    \value TabletMove  A Wacom Tablet Move Event.
    \value Style  Internal use only
    \value TabletPress  A Wacom Tablet Press Event
    \value TabletRelease  A Wacom Tablet Release Event
    \value OkRequest  Internal event used by Qt on some platforms.
    \value IconDrag     Internal event used by Qt on some platforms when proxy icon is dragged.
    \value HelpRequest  Internal event used by Qt on some platforms.
    \value FontChange Widget's font has changed
    \value EnabledChange Widget's enabled state has changed
    \value ActivationChange Widget's top level window activation state has changed
    \value StyleChange Widget's style has been changed
    \value IconTextChange Widget's icon text has been changed
    \value ModifiedChange Widgets modification state has been changed
    \value QWSUpdate internal.

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
    if ( posted && QKernelApplication::self )
	QKernelApplication::removePostedEvent( this );
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
    \class QTimerEvent qkernelevent.h
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
    \class QChildEvent qkernelevent.h
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
    \fn bool QChildEvent::inserted() const

    Returns TRUE if the widget received a new child; otherwise returns
    FALSE.
*/

/*!
    \fn bool QChildEvent::removed() const

    Returns TRUE if the object lost a child; otherwise returns FALSE.
*/

/*!
    \class QCustomEvent qkernelevent.h
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





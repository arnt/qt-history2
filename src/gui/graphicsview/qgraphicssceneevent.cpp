/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*!
    \class QGraphicsSceneEvent
    \brief The QGraphicsSceneEvent class provides a base class for all
    graphics view related events.
    \since 4.2
    \ingroup multimedia
   
    When a QGraphicsView receives mouse, keyboard, and drag and
    drop events, it translates them into a graphics scene event
    (events that inherit QGraphicsSceneEvent). It then forwards them
    to the scene it displays. The scene then propagates the events to
    the items it manages. A mouse click event will, for instance, be
    propagated to the item under the mouse cursor.
    
    The graphics scene events main purpose is to provide the
    coordinates from the original event in screen, scene and item coordinates;
    the item coordinates is set by the QGraphicsScene before beeing
    propagated to items.  The mouse events also add the possibility to
    retrieve the coordinates from the last event received by the view.

    \sa QEvent
*/

/*!
    \class QGraphicsSceneMouseEvent
    \brief QGraphicsSceneMouseEvent provides mouse events in the
    graphics view framework.
    \since 4.2
    \ingroup multimedia

    When a QGraphicsView receives a mouse event it translates it
    to a QGraphicsSceneMouseEvent. The event is then forwarded to
    the scene it visualizes.
    
    In addition to containing the scene, item, and screen
    coordinates of the event, mouse events will also contain the
    coordinates of the last event recieved by the view. These can be
    retrieved with \c lastPos(), lastScreenPos(), and lastScenePos().

    \sa QGraphicsSceneContextMenuEvent,
	QGraphicsSceneHoverEvent, QGraphicsSceneWheelEvent,
	QMouseEvent
*/

/*!
    \class QGraphicsSceneWheelEvent
    \brief QGraphicsSceneWheelEvent provides wheel events in the
    graphics view framework.
    \since 4.2
    \ingroup multimedia

    \l{QWheelEvent}{QWheelEvents} received by a QGraphicsView are
    translated into
    \l{QGraphicsViewWheelEvent}{QGraphicsSceneWheelEvents}; it
    translates the QWheelEvent::globalPos() into screen and scene
    coordinates.

    \sa QGraphicsSceneMouseEvent,
    QGraphicsSceneContextMenuEvent, QGraphicsSceneHoverEvent,
    QWheelEvent
*/

/*!
    \class QGraphicsSceneContextMenuEvent
    \brief QGraphicsSceneContextMenuEvent provides context menu
    events in the graphics view framework.
    \since 4.2
    \ingroup multimedia

    A QContextMenuEvent received by a QGraphicsView is translated
    into a QGraphicsSceneContextMenuEvent; it translates the
    QContextmenuEvent::pos() into screen and scene coordinates. The
    event is then forwarded to the scene visualized by the view.

    \sa QGraphicsSceneMouseEvent, QGraphicsSceneWheelEvent,
    QGraphicsSceneContextMenuEvent, QContextMenuEvent
*/

/*!
    \enum QGraphicsSceneContextMenuEvent::Reason

    This enum describes the reason why the context event was sent.

    \value Mouse The mouse caused the event to be sent. Normally
    this means the right mouse button was clicked, but this is
    platform dependent.

    \value Keyboard The keyboard caused this event to be sent. On
    Windows, this means the menu button was pressed.

    \value Other The event was sent by some other means (i.e. not
    by the mouse or keyboard).
*/

/*!
    \class QGraphicsSceneHoverEvent
    \brief QGraphicsSceneHoverEvent provides hover events in the
    graphics view framework.
    \since 4.2
    \ingroup multimedia

    When a QGraphicsView receives mouse events it translates them
    into graphics scene events (events that inherit
    QGraphicsSceneEvent). It then forwards the event to the scene
    it displays; the scene then propagates it to its items.

    \sa QGraphicsSceneMouseEvent, QGraphicsViewContextMenuEvent,
    QGraphicsViewWheelEvent, QHoverEvent.
*/

/*!
    \class QGraphicsSceneHelpEvent
    \brief QGraphicsSceneHelpEvent provides events when a tooltip is requested.
    \since 4.2
    \ingroup multimedia

    When a QGraphicsView receives a QEvent of type
    QEvent::ToolTip, it creates a QGraphicsSceneHelpEvent, which is
    forwarded to the scene. You can set a tooltip on a QGraphicsItem
    with \l{QGraphicsItem::}{setToolTip()}; by default QGraphicsScene
    displays the tooltip of the QGraphicsItem with the highest
    z-value, i.e, the top most item,  under the mouse position.
    
    QGraphicsView does not propagate events when
    \l{QWhatsThis}{"What's This"} and \l{QStatusTipEvent}{status tip}
    help is requested. If you need this, you can reimplement
    QGraphicsView::viewportEvent() and propagate QStatusTipEvent
    events and \l{QEvent}{QEvents} of type QEvent::WhatsThis to the
    scene.

    \sa QEvent
*/

/*!
    \class QGraphicsSceneDragDropEvent
    \brief QGraphicsSceneDragDropEvent provides events for 
    drag and drop in the graphics view framework. 
    \since 4.2
    \ingroup multimedia

    QGraphicsView indirectly inherits QWidget and the drag and
    drop functionality QWidget provides. When it receives a drag and
    drop event, it does not forward the event to the scene it displays
    directly, but translates it to a QGraphicsSceneDragDropEvent,
    which--unlike QDropEvent--contain the screen and scene coordinates
    of the event.
    
    QGraphicsSceneDragDropEvent contains the position of the mouse
    cursor in both screen, item, and scene coordinates; this can be
    retrieved with scenePos(), pos(), and screenPos().
    
    The scene sends the event to the first QGraphicsItem under the
    mouse cursor that accepts drops; a graphics item is set to accept
    drops with \l{QGraphicsItem::}{setAcceptDrops()}.
*/

#include "qgraphicssceneevent.h"

#ifndef QT_NO_GRAPHICSVIEW

#ifndef QT_NO_DEBUG
#include <QtCore/qdebug.h>
#endif
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

class QGraphicsSceneEventPrivate
{
public:
    inline QGraphicsSceneEventPrivate()
        : widget(0),
          q_ptr(0)
    { }

    inline virtual ~QGraphicsSceneEventPrivate()
    { }

    QWidget *widget;
    QGraphicsSceneEvent *q_ptr;
};

/*!
    Constructs a generic graphics scene event of the specified \a type.
    \internal
*/
QGraphicsSceneEvent::QGraphicsSceneEvent(Type type)
    : QEvent(type), d_ptr(new QGraphicsSceneEventPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
    \internal
    Constructs a generic graphics scene event.
*/
QGraphicsSceneEvent::QGraphicsSceneEvent(QGraphicsSceneEventPrivate &dd, Type type)
    : QEvent(type), d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys the event.
*/
QGraphicsSceneEvent::~QGraphicsSceneEvent()
{
    delete d_ptr;
}

/*!
    Returns the widget where the event originated. If the event did not
    originate from a widget, 0 is returned.

    \sa setWidget()
*/
QWidget *QGraphicsSceneEvent::widget() const
{
    return d_ptr->widget;
}

/*!
    \internal
    Sets the \a widget related to this event.

    \sa widget()
*/
void QGraphicsSceneEvent::setWidget(QWidget *widget)
{
    d_ptr->widget = widget;
}

class QGraphicsSceneMouseEventPrivate : public QGraphicsSceneEventPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsSceneMouseEvent)
public:
    inline QGraphicsSceneMouseEventPrivate()
        : button(Qt::NoButton),
          buttons(0), modifiers(0)
    { }

    QPointF pos;
    QPointF scenePos;
    QPoint screenPos;
    QPointF lastPos;
    QPointF lastScenePos;
    QPoint lastScreenPos;
    QMap<Qt::MouseButton, QPointF> buttonDownPos;
    QMap<Qt::MouseButton, QPointF> buttonDownScenePos;
    QMap<Qt::MouseButton, QPoint> buttonDownScreenPos;
    Qt::MouseButton button;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
};

/*!
    \internal
    Constructs a generic graphics scene mouse event of the specified \a type.
*/
QGraphicsSceneMouseEvent::QGraphicsSceneMouseEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneMouseEventPrivate, type)
{
}

/*!
    Destroys the event.
*/
QGraphicsSceneMouseEvent::~QGraphicsSceneMouseEvent()
{
}

/*!
    Returns the mouse cursor position in item coordinates.

    \sa scenePos(), screenPos(), lastPos()
*/
QPointF QGraphicsSceneMouseEvent::pos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->pos;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->pos = pos;
}

/*!
    Returns the mouse cursor position in scene coordinates.

    \sa pos(), screenPos(), lastScenePos()
*/
QPointF QGraphicsSceneMouseEvent::scenePos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->scenePos;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->scenePos = pos;
}

/*!
    Returns the mouse cursor position in screen coordinates.

    \sa pos(), scenePos(), lastScreenPos()
*/
QPoint QGraphicsSceneMouseEvent::screenPos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->screenPos;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->screenPos = pos;
}

/*!
    Returns the mouse cursor position in item coordinates where the specified
    \a button was clicked.

  \sa buttonDownScenePos(), buttonDownScreenPos(), pos()
*/
QPointF QGraphicsSceneMouseEvent::buttonDownPos(Qt::MouseButton button) const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttonDownPos.value(button);
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setButtonDownPos(Qt::MouseButton button, const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttonDownPos.insert(button, pos);
}

/*!
    Returns the mouse cursor position in scene coordinates where the specified
    \a button was clicked.

    \sa buttonDownPos(), buttonDownScreenPos(), scenePos()
*/
QPointF QGraphicsSceneMouseEvent::buttonDownScenePos(Qt::MouseButton button) const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttonDownScenePos.value(button);
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setButtonDownScenePos(Qt::MouseButton button, const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttonDownScenePos.insert(button, pos);
}

/*!
    Returns the mouse cursor position in screen coordinates where the
    specified \a button was clicked.

    \sa screenPos(), buttonDownPos(), buttonDownScenePos()
*/
QPoint QGraphicsSceneMouseEvent::buttonDownScreenPos(Qt::MouseButton button) const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttonDownScreenPos.value(button);
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setButtonDownScreenPos(Qt::MouseButton button, const QPoint &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttonDownScreenPos.insert(button, pos);
}

/*!
    Returns the last recorded mouse cursor position in item coordinates.

    \sa lastScenePos(), lastScreenPos(), pos()
*/
QPointF QGraphicsSceneMouseEvent::lastPos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->lastPos;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setLastPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->lastPos = pos;
}

/*!
    Returns the last recorded, the scene coordinates of the
    previous mouse event received by the view, that created the
    event mouse cursor position in scene coordinates.

    \sa lastPos(), lastScreenPos(), scenePos()
*/
QPointF QGraphicsSceneMouseEvent::lastScenePos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->lastScenePos;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setLastScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->lastScenePos = pos;
}

/*!
    Returns the last recorded mouse cursor position in screen 
    coordinates. The last recorded position is the position of
    the previous mouse event received by the view that created
    the event.

    \sa lastPos(), lastScenePos(), screenPos()
*/
QPoint QGraphicsSceneMouseEvent::lastScreenPos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->lastScreenPos;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setLastScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->lastScreenPos = pos;
}

/*!
    Returns the combination of mouse buttons that were pressed at the time the
    event was sent.

    \sa button(), modifiers()
*/
Qt::MouseButtons QGraphicsSceneMouseEvent::buttons() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttons;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setButtons(Qt::MouseButtons buttons)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttons = buttons;
}

/*!
    Returns the mouse button--if any--that caused the event.

    \sa buttons(), modifiers()
*/
Qt::MouseButton QGraphicsSceneMouseEvent::button() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->button;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setButton(Qt::MouseButton button)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->button = button;
}

/*!
    Returns the keyboard modifiers in use at the time the event was
    sent.

    \sa button(), buttons()
*/
Qt::KeyboardModifiers QGraphicsSceneMouseEvent::modifiers() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->modifiers;
}

/*!
    \internal
*/
void QGraphicsSceneMouseEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->modifiers = modifiers;
}

class QGraphicsSceneWheelEventPrivate : public QGraphicsSceneEventPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsSceneWheelEvent)
public:
    inline QGraphicsSceneWheelEventPrivate()
        : buttons(0), modifiers(0), delta(0)
    { }

    QPointF pos;
    QPointF scenePos;
    QPoint screenPos;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    int delta;
};

/*!
    Constructs a QGraphicsSceneWheelEvent of type \a type, which
    is always QEvent::GraphicsSceneWheel.
*/
QGraphicsSceneWheelEvent::QGraphicsSceneWheelEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneWheelEventPrivate, type)
{
}

/*!
    Destroys the QGraphicsSceneWheelEvent.
*/
QGraphicsSceneWheelEvent::~QGraphicsSceneWheelEvent()
{
}

/*!
    Returns the position of the cursor in item coordinates when the
    wheel event occurred.

    \sa setPos(), scenePos(), screenPos()
*/
QPointF QGraphicsSceneWheelEvent::pos() const
{
    Q_D(const QGraphicsSceneWheelEvent);
    return d->pos;
}

/*!
    \internal
*/
void QGraphicsSceneWheelEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneWheelEvent);
    d->pos = pos;
}

/*!
    Returns the position of the cursor in item coordinates when the wheel
    event occurred.

    \sa setScenePos(), pos(), screenPos()
*/
QPointF QGraphicsSceneWheelEvent::scenePos() const
{
    Q_D(const QGraphicsSceneWheelEvent);
    return d->scenePos;
}

/*!
    \internal
*/
void QGraphicsSceneWheelEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneWheelEvent);
    d->scenePos = pos;
}

/*!
    Returns the position of the cursor in screen coordinates when the wheel
    event occurred.

    \sa setScreenPos(), pos(), scenePos()
*/
QPoint QGraphicsSceneWheelEvent::screenPos() const
{
    Q_D(const QGraphicsSceneWheelEvent);
    return d->screenPos;
}

/*!
    \internal
*/
void QGraphicsSceneWheelEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneWheelEvent);
    d->screenPos = pos;
}

/*!
    Returns the mouse buttons that were pressed when the wheel event occurred.

    \sa setButtons(), modifiers()
*/
Qt::MouseButtons QGraphicsSceneWheelEvent::buttons() const
{
    Q_D(const QGraphicsSceneWheelEvent);
    return d->buttons;
}

/*!
    \internal
*/
void QGraphicsSceneWheelEvent::setButtons(Qt::MouseButtons buttons)
{
    Q_D(QGraphicsSceneWheelEvent);
    d->buttons = buttons;
}

/*!
    Returns the keyboard modifiers that were active when the wheel event
    occurred.

    \sa setModifiers(), buttons()
*/
Qt::KeyboardModifiers QGraphicsSceneWheelEvent::modifiers() const
{
    Q_D(const QGraphicsSceneWheelEvent);
    return d->modifiers;
}

/*!
    \internal
*/
void QGraphicsSceneWheelEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(QGraphicsSceneWheelEvent);
    d->modifiers = modifiers;
}

/*!
    Returns the distance that the wheel is rotated, in eights of a degree. A
    positive value indicates that the wheel was rotated forwards away from the
    user; a negative value indicates that the wheel was rotated backwards
    toward the user.

    Most mouse types work in steps of 15 degrees, in which case the delta
    value is a multiple of 120 (== 15 * 8).

    \sa setDelta()
*/
int QGraphicsSceneWheelEvent::delta() const
{
    Q_D(const QGraphicsSceneWheelEvent);
    return d->delta;
}

/*!
    \internal
*/
void QGraphicsSceneWheelEvent::setDelta(int delta)
{
    Q_D(QGraphicsSceneWheelEvent);
    d->delta = delta;
}

class QGraphicsSceneContextMenuEventPrivate : public QGraphicsSceneEventPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsSceneContextMenuEvent)
        public:
    inline QGraphicsSceneContextMenuEventPrivate()
        : modifiers(0), reason(QGraphicsSceneContextMenuEvent::Other)
        { }

    QPointF pos;
    QPointF scenePos;
    QPoint screenPos;
    Qt::KeyboardModifiers modifiers;
    QGraphicsSceneContextMenuEvent::Reason reason;
};

/*!
    Constructs a graphics scene context menu event of the specified \a type.
    \internal
*/
QGraphicsSceneContextMenuEvent::QGraphicsSceneContextMenuEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneContextMenuEventPrivate, type)
{
}

/*!
    Destroys the event.
*/
QGraphicsSceneContextMenuEvent::~QGraphicsSceneContextMenuEvent()
{
}

/*!
    Returns the position of the mouse cursor in item coordinates at the moment
    the the context menu was requested.

    \sa scenePos(), screenPos()
*/
QPointF QGraphicsSceneContextMenuEvent::pos() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->pos;
}

/*!
    \fn void QGraphicsSceneContextMenuEvent::setPos(const QPointF &point)
    \internal

    Sets the position associated with the context menu to the given \a point
    in item coordinates.
*/
void QGraphicsSceneContextMenuEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->pos = pos;
}

/*!
    Returns the position of the mouse cursor in scene coordinates at the moment the
    the context menu was requested.

    \sa pos(), screenPos()
*/
QPointF QGraphicsSceneContextMenuEvent::scenePos() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->scenePos;
}

/*!
    \fn void QGraphicsSceneContextMenuEvent::setScenePos(const QPointF &point)
    \internal

    Sets the position associated with the context menu to the given \a point
    in scene coordinates.
*/
void QGraphicsSceneContextMenuEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->scenePos = pos;
}

/*!
    Returns the position of the mouse cursor in screen coordinates at the moment the
    the context menu was requested.

    \sa pos(), scenePos()
*/
QPoint QGraphicsSceneContextMenuEvent::screenPos() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->screenPos;
}

/*!
    \fn void QGraphicsSceneContextMenuEvent::setScreenPos(const QPoint &point)
    \internal

    Sets the position associated with the context menu to the given \a point
    in screen coordinates.
*/
void QGraphicsSceneContextMenuEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->screenPos = pos;
}

/*!
    Returns the keyboard modifiers in use when the context menu was requested.
*/
Qt::KeyboardModifiers QGraphicsSceneContextMenuEvent::modifiers() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->modifiers;
}

/*!
    \internal

    Sets the keyboard modifiers associated with the context menu to the \a
    modifiers specified.
*/
void QGraphicsSceneContextMenuEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->modifiers = modifiers;
}

/*!
    Returns the reason for the context menu event.

    \sa setReason(), QGraphicsContextMenu::Reason
*/
QGraphicsSceneContextMenuEvent::Reason QGraphicsSceneContextMenuEvent::reason() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->reason;
}

/*!
    \internal
    Sets the reason for the context menu event to \a reason.

    \sa reason()
*/
void QGraphicsSceneContextMenuEvent::setReason(Reason reason)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->reason = reason;
}

class QGraphicsSceneHoverEventPrivate : public QGraphicsSceneEventPrivate
{
public:
    QPointF pos;
    QPointF scenePos;
    QPoint screenPos;
};

/*!
    \internal
    Constructs a graphics scene hover event of the specified \a type.
*/
QGraphicsSceneHoverEvent::QGraphicsSceneHoverEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneHoverEventPrivate, type)
{
}

/*!
    Destroys the event.
*/
QGraphicsSceneHoverEvent::~QGraphicsSceneHoverEvent()
{
}

/*!
    Returns the position of the mouse cursor in item coordinates at the moment
    the the hover event was sent.

    \sa scenePos(), screenPos()
*/
QPointF QGraphicsSceneHoverEvent::pos() const
{
    Q_D(const QGraphicsSceneHoverEvent);
    return d->pos;
}

/*!
    \fn void QGraphicsSceneHoverEvent::setPos(const QPointF &point)
    \internal

    Sets the position associated with the hover event to the given \a point in
    item coordinates.
*/
void QGraphicsSceneHoverEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneHoverEvent);
    d->pos = pos;
}

/*!
    Returns the position of the mouse cursor in scene coordinates at the
    moment the the hover event was sent.

    \sa pos(), screenPos()
*/
QPointF QGraphicsSceneHoverEvent::scenePos() const
{
    Q_D(const QGraphicsSceneHoverEvent);
    return d->scenePos;
}

/*!
    \fn void QGraphicsSceneHoverEvent::setScenePos(const QPointF &point)
    \internal

    Sets the position associated with the hover event to the given \a point in
    scene coordinates.
*/
void QGraphicsSceneHoverEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneHoverEvent);
    d->scenePos = pos;
}

/*!
    Returns the position of the mouse cursor in screen coordinates at the
    moment the the hover event was sent.

    \sa pos(), scenePos()
*/
QPoint QGraphicsSceneHoverEvent::screenPos() const
{
    Q_D(const QGraphicsSceneHoverEvent);
    return d->screenPos;
}

/*!
    \fn void QGraphicsSceneHoverEvent::setScreenPos(const QPoint &point)
    \internal

    Sets the position associated with the hover event to the given \a point in
    screen coordinates.
*/
void QGraphicsSceneHoverEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneHoverEvent);
    d->screenPos = pos;
}

class QGraphicsSceneHelpEventPrivate : public QGraphicsSceneEventPrivate
{
public:
    QPointF scenePos;
    QPoint screenPos;
};

/*!
    \internal
    Constructs a graphics scene help event of the specified \a type.
*/
QGraphicsSceneHelpEvent::QGraphicsSceneHelpEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneHelpEventPrivate, type)
{
}

/*!
    Destroys the event.
*/
QGraphicsSceneHelpEvent::~QGraphicsSceneHelpEvent()
{
}

/*!
    Returns the position of the mouse cursor in scene coordinates at the
    moment the the help event was sent.

    \sa screenPos()
*/
QPointF QGraphicsSceneHelpEvent::scenePos() const
{
    Q_D(const QGraphicsSceneHelpEvent);
    return d->scenePos;
}

/*!
    \fn void QGraphicsSceneHelpEvent::setScenePos(const QPointF &point)
    \internal

    Sets the position associated with the context menu to the given \a point
    in scene coordinates.
*/
void QGraphicsSceneHelpEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneHelpEvent);
    d->scenePos = pos;
}

/*!
    Returns the position of the mouse cursor in screen coordinates at the
    moment the the help event was sent.

  \sa scenePos()
*/
QPoint QGraphicsSceneHelpEvent::screenPos() const
{
    Q_D(const QGraphicsSceneHelpEvent);
    return d->screenPos;
}

/*!
    \fn void QGraphicsSceneHelpEvent::setScreenPos(const QPoint &point)
    \internal

    Sets the position associated with the context menu to the given \a point
    in screen coordinates.
*/
void QGraphicsSceneHelpEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneHelpEvent);
    d->screenPos = pos;
}

class QGraphicsSceneDragDropEventPrivate : public QGraphicsSceneEventPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsSceneDragDropEvent)
public:
    inline QGraphicsSceneDragDropEventPrivate()
        : source(0), mimeData(0)
    { }

    QPointF pos;
    QPointF scenePos;
    QPoint screenPos;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    Qt::DropActions possibleActions;
    Qt::DropAction proposedAction;
    Qt::DropAction dropAction;
    QWidget *source;
    const QMimeData *mimeData;
};

/*!
    Constructs a new QGraphicsSceneDragDropEvent of the
    specified \a type. The type can be either,
    QEvent::GraphicsSceneDragEnter, QEvent::GraphicsSceneDragLeave
    , QEvent::GraphicsSceneDragMove or, QEvent::GraphicsSceneDrop.

    \internal
*/
QGraphicsSceneDragDropEvent::QGraphicsSceneDragDropEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneDragDropEventPrivate, type)
{
}

/*!
    Destroys the object.
*/
QGraphicsSceneDragDropEvent::~QGraphicsSceneDragDropEvent()
{
}

/*!
    Returns the mouse position of the event relative to the
    view that sent the event.
    
    \sa QGraphicsView, screenPos(),
	scenePos()
*/
QPointF QGraphicsSceneDragDropEvent::pos() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->pos;
}

/*!
    \internal
    Sets the position of the mouse to \a pos; this should be
    relative to the widget that generated the event, which normally
    is a QGraphicsView.

    \sa pos(), setScenePos(), setScreenPos()
*/

void QGraphicsSceneDragDropEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->pos = pos;
}

/*!
    Returns the position of the mouse in scene coordinates.

    \sa pos(), screenPos()
*/
QPointF QGraphicsSceneDragDropEvent::scenePos() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->scenePos;
}

/*!
    \internal
    Sets the scene position of the mouse to \a pos.

    \sa scenePos(), setScreenPos(), setPos()
*/
void QGraphicsSceneDragDropEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->scenePos = pos;
}

/*!
    Returns the position of the mouse relative to the screen.

    \sa pos(), scenePos()
*/
QPoint QGraphicsSceneDragDropEvent::screenPos() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->screenPos;
}

/*!
    \internal
    Sets the mouse position relative to the screen to \a pos.

    \sa screenPos(), setScenePos(), setPos()
*/
void QGraphicsSceneDragDropEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->screenPos = pos;
}

/*!
    Returns a Qt::MouseButtons value indicating which buttons
    were pressed on the mouse when this mouse event was
    generated.

    \sa Qt::MouseButtons, button()
*/
Qt::MouseButtons QGraphicsSceneDragDropEvent::buttons() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->buttons;
}

/*!
    \internal
    Sets the mouse buttons that were pressed when the event was
    created to \a buttons.

    \sa Qt::MouseButtons, buttons()
*/
void QGraphicsSceneDragDropEvent::setButtons(Qt::MouseButtons buttons)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->buttons = buttons;
}

/*!
    Returns the keyboard modifiers that were pressed when the drag
    and drop event was created. 

    \sa Qt::KeyboardModifiers
*/
Qt::KeyboardModifiers QGraphicsSceneDragDropEvent::modifiers() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->modifiers;
}

/*!
    \internal
    Sets the keyboard modifiers that were pressed when the event
    was created to \a modifiers.

    \sa Qt::KeyboardModifiers, modifiers()
*/

void QGraphicsSceneDragDropEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->modifiers = modifiers;
}

/*!
    Returns the possible drop actions that the drag and
    drop can result in.

    \sa Qt::DropActions
*/

Qt::DropActions QGraphicsSceneDragDropEvent::possibleActions() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->possibleActions;
}

/*!
    \internal
    Sets the possible drop actions that the drag can
    result in to \a actions.

    \sa Qt::DropActions, possibleActions()
*/
void QGraphicsSceneDragDropEvent::setPossibleActions(Qt::DropActions actions)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->possibleActions = actions;
}

/*!
    Returns the drop action that is proposed, i.e., preferred.
    The action must be one of the possible actions as defined by
    \c possibleActions().

    \sa Qt::DropAction, possibleActions()    
*/

Qt::DropAction QGraphicsSceneDragDropEvent::proposedAction() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->proposedAction;
}

/*!
    \internal
    Sets the proposed action to \a action. The proposed action
    is a Qt::DropAction that is one of the possible actions as
    given by \c possibleActions().

    \sa proposedAction(), Qt::DropAction, possibleActions()
*/

void QGraphicsSceneDragDropEvent::setProposedAction(Qt::DropAction action)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->proposedAction = action;
}

/*!
    Sets the proposed action as accepted, i.e, the drop action
    is set to the proposed action. This is equal to:

    \code

    setDropAction(proposedAction());

    \endcode

    When using this function, one should not call \c accept().

    \sa dropAction(), setDropAction(), proposedAction()
*/

void QGraphicsSceneDragDropEvent::acceptProposedAction()
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->dropAction = d->proposedAction;
}

/*!
    Returns the action that was performed in this drag and drop.
    This should be set by the receiver of the drop and is
    returned by QDrag::start().

    \sa setDropAction(), acceptProposedAction()
*/

Qt::DropAction QGraphicsSceneDragDropEvent::dropAction() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->dropAction;
}

/*!
    This function lets the receiver of the drop set the drop
    action that was performed to \a action, which should be one
    of the
    \l{QGraphicsSceneDragDropEvent::possibleActions()}{possible
    actions}. Call \c accept() in stead of \c
    acceptProposedAction() if you use this function.

    \sa dropAction(), accept(), possibleActions()
*/
void QGraphicsSceneDragDropEvent::setDropAction(Qt::DropAction action)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->dropAction = action;
}

/*!
    This function returns the QGraphicsView that created the
    QGraphicsSceneDragDropEvent.
*/
QWidget *QGraphicsSceneDragDropEvent::source() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->source;
}

/*!
    \internal
    This function set the source widget, i.e., the widget that
    created the drop event, to \source.
*/
void QGraphicsSceneDragDropEvent::setSource(QWidget *source)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->source = source;
}

/*!
    This function returns the MIME data of the event.
*/
const QMimeData *QGraphicsSceneDragDropEvent::mimeData() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->mimeData;
}

/*!
    \internal
    This function sets the MIME data for the event.
*/
void QGraphicsSceneDragDropEvent::setMimeData(const QMimeData *data)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->mimeData = data;
}

#endif // QT_NO_GRAPHICSVIEW

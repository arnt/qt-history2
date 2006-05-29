/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
*/

/*!
    \class QGraphicsSceneMouseEvent
    \since 4.2
    \ingroup multimedia
*/

/*!
    \class QGraphicsSceneContextMenuEvent
    \since 4.2
    \ingroup multimedia
*/

/*!
    \class QGraphicsSceneHoverEvent
    \since 4.2
    \ingroup multimedia
*/

#include "qgraphicssceneevent.h"

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
    Returns the last recorded mouse cursor position in scene coordinates.

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
    Returns the last recorded mouse cursor position in screen coordinates.

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
    Returns the mouse button that was pressed at the time the event was sent.

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
    Returns the keyboard modifiers in use at the time the event was sent.

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

class QGraphicsSceneContextMenuEventPrivate : public QGraphicsSceneEventPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsSceneContextMenuEvent)
        public:
    inline QGraphicsSceneContextMenuEventPrivate()
        : modifiers(0)
        { }
    
    QPointF pos;
    QPointF scenePos;
    QPoint screenPos;
    Qt::KeyboardModifiers modifiers;
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
    Q_DECLARE_PUBLIC(QGraphicsSceneDragDropEvent);
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

QGraphicsSceneDragDropEvent::QGraphicsSceneDragDropEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneDragDropEventPrivate, type)
{
}

QGraphicsSceneDragDropEvent::~QGraphicsSceneDragDropEvent()
{
}

QPointF QGraphicsSceneDragDropEvent::pos() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->pos;
}

void QGraphicsSceneDragDropEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->pos = pos;
}

QPointF QGraphicsSceneDragDropEvent::scenePos() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->scenePos;
}

void QGraphicsSceneDragDropEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->scenePos = pos;
}

QPoint QGraphicsSceneDragDropEvent::screenPos() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->screenPos;
}

void QGraphicsSceneDragDropEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->screenPos = pos;
}

Qt::MouseButtons QGraphicsSceneDragDropEvent::buttons() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->buttons;
}

void QGraphicsSceneDragDropEvent::setButtons(Qt::MouseButtons buttons)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->buttons = buttons;
}

Qt::KeyboardModifiers QGraphicsSceneDragDropEvent::modifiers() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->modifiers;
}

void QGraphicsSceneDragDropEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->modifiers = modifiers;
}

Qt::DropActions QGraphicsSceneDragDropEvent::possibleActions() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->possibleActions;
}

void QGraphicsSceneDragDropEvent::setPossibleActions(Qt::DropActions actions)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->possibleActions = actions;
}

Qt::DropAction QGraphicsSceneDragDropEvent::proposedAction() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->proposedAction;
}

void QGraphicsSceneDragDropEvent::setProposedAction(Qt::DropAction action)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->proposedAction = action;
}

Qt::DropAction QGraphicsSceneDragDropEvent::dropAction() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->dropAction;
}

void QGraphicsSceneDragDropEvent::setDropAction(Qt::DropAction action)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->dropAction = action;
}

QWidget *QGraphicsSceneDragDropEvent::source() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->source;
}

void QGraphicsSceneDragDropEvent::setSource(QWidget *source)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->source = source;
}

const QMimeData *QGraphicsSceneDragDropEvent::mimeData() const
{
    Q_D(const QGraphicsSceneDragDropEvent);
    return d->mimeData;
}

void QGraphicsSceneDragDropEvent::setMimeData(const QMimeData *data)
{
    Q_D(QGraphicsSceneDragDropEvent);
    d->mimeData = data;
}

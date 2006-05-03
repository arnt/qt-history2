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

/*!
    \class QGraphicsSceneEvent
*/

/*!
    \class QGraphicsSceneMouseEvent
*/

/*!
    \class QGraphicsSceneContextMenuEvent
*/

/*!
    \class QGraphicsSceneHoverEvent
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

QGraphicsSceneEvent::QGraphicsSceneEvent(Type type)
    : QEvent(type), d_ptr(new QGraphicsSceneEventPrivate)
{
    d_ptr->q_ptr = this;
}

QGraphicsSceneEvent::QGraphicsSceneEvent(QGraphicsSceneEventPrivate &dd, Type type)
    : QEvent(type), d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

QGraphicsSceneEvent::~QGraphicsSceneEvent()
{
    delete d_ptr;
}

QWidget *QGraphicsSceneEvent::widget() const
{
    return d_ptr->widget;
}

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
          buttons(0),
          modifiers(0)
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

QGraphicsSceneMouseEvent::QGraphicsSceneMouseEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneMouseEventPrivate, type)
{
}

QGraphicsSceneMouseEvent::~QGraphicsSceneMouseEvent()
{
}

QPointF QGraphicsSceneMouseEvent::pos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->pos;
}

void QGraphicsSceneMouseEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->pos = pos;
}

QPointF QGraphicsSceneMouseEvent::scenePos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->scenePos;
}

void QGraphicsSceneMouseEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->scenePos = pos;
}

QPoint QGraphicsSceneMouseEvent::screenPos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->screenPos;
}

void QGraphicsSceneMouseEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->screenPos = pos;
}

QPointF QGraphicsSceneMouseEvent::buttonDownPos(Qt::MouseButton button) const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttonDownPos.value(button);
}

void QGraphicsSceneMouseEvent::setButtonDownPos(Qt::MouseButton button, const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttonDownPos.insert(button, pos);
}

QPointF QGraphicsSceneMouseEvent::buttonDownScenePos(Qt::MouseButton button) const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttonDownScenePos.value(button);
}

void QGraphicsSceneMouseEvent::setButtonDownScenePos(Qt::MouseButton button, const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttonDownScenePos.insert(button, pos);
}

QPoint QGraphicsSceneMouseEvent::buttonDownScreenPos(Qt::MouseButton button) const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttonDownScreenPos.value(button);
}

void QGraphicsSceneMouseEvent::setButtonDownScreenPos(Qt::MouseButton button, const QPoint &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttonDownScreenPos.insert(button, pos);
}

QPointF QGraphicsSceneMouseEvent::lastPos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->lastPos;
}

void QGraphicsSceneMouseEvent::setLastPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->lastPos = pos;
}

QPointF QGraphicsSceneMouseEvent::lastScenePos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->lastScenePos;
}

void QGraphicsSceneMouseEvent::setLastScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->lastScenePos = pos;
}

QPoint QGraphicsSceneMouseEvent::lastScreenPos() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->lastScreenPos;
}

void QGraphicsSceneMouseEvent::setLastScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->lastScreenPos = pos;
}

Qt::MouseButtons QGraphicsSceneMouseEvent::buttons() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->buttons;
}

void QGraphicsSceneMouseEvent::setButtons(Qt::MouseButtons buttons)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->buttons = buttons;
}

Qt::MouseButton QGraphicsSceneMouseEvent::button() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->button;
}

void QGraphicsSceneMouseEvent::setButton(Qt::MouseButton button)
{
    Q_D(QGraphicsSceneMouseEvent);
    d->button = button;
}

Qt::KeyboardModifiers QGraphicsSceneMouseEvent::modifiers() const
{
    Q_D(const QGraphicsSceneMouseEvent);
    return d->modifiers;
}

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

QGraphicsSceneContextMenuEvent::QGraphicsSceneContextMenuEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneContextMenuEventPrivate, type)
{
}

QGraphicsSceneContextMenuEvent::~QGraphicsSceneContextMenuEvent()
{
}

QPointF QGraphicsSceneContextMenuEvent::pos() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->pos;
}

void QGraphicsSceneContextMenuEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->pos = pos;
}

QPointF QGraphicsSceneContextMenuEvent::scenePos() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->scenePos;
}

void QGraphicsSceneContextMenuEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->scenePos = pos;
}

QPoint QGraphicsSceneContextMenuEvent::screenPos() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->screenPos;
}

void QGraphicsSceneContextMenuEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneContextMenuEvent);
    d->screenPos = pos;
}

Qt::KeyboardModifiers QGraphicsSceneContextMenuEvent::modifiers() const
{
    Q_D(const QGraphicsSceneContextMenuEvent);
    return d->modifiers;
}

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

QGraphicsSceneHoverEvent::QGraphicsSceneHoverEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneHoverEventPrivate, type)
{
}

QGraphicsSceneHoverEvent::~QGraphicsSceneHoverEvent()
{
}

QPointF QGraphicsSceneHoverEvent::pos() const
{
    Q_D(const QGraphicsSceneHoverEvent);
    return d->pos;
}

void QGraphicsSceneHoverEvent::setPos(const QPointF &pos)
{
    Q_D(QGraphicsSceneHoverEvent);
    d->pos = pos;
}

QPointF QGraphicsSceneHoverEvent::scenePos() const
{
    Q_D(const QGraphicsSceneHoverEvent);
    return d->scenePos;
}

void QGraphicsSceneHoverEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneHoverEvent);
    d->scenePos = pos;
}

QPoint QGraphicsSceneHoverEvent::screenPos() const
{
    Q_D(const QGraphicsSceneHoverEvent);
    return d->screenPos;
}

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

QGraphicsSceneHelpEvent::QGraphicsSceneHelpEvent(Type type)
    : QGraphicsSceneEvent(*new QGraphicsSceneHelpEventPrivate, type)
{
}

QGraphicsSceneHelpEvent::~QGraphicsSceneHelpEvent()
{
}

QPointF QGraphicsSceneHelpEvent::scenePos() const
{
    Q_D(const QGraphicsSceneHelpEvent);
    return d->scenePos;
}

void QGraphicsSceneHelpEvent::setScenePos(const QPointF &pos)
{
    Q_D(QGraphicsSceneHelpEvent);
    d->scenePos = pos;
}

QPoint QGraphicsSceneHelpEvent::screenPos() const
{
    Q_D(const QGraphicsSceneHelpEvent);
    return d->screenPos;
}

void QGraphicsSceneHelpEvent::setScreenPos(const QPoint &pos)
{
    Q_D(QGraphicsSceneHelpEvent);
    d->screenPos = pos;
}

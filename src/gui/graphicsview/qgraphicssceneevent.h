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

#ifndef QGRAPHICSSCENEEVENT_H
#define QGRAPHICSSCENEEVENT_H

#include <QtCore/qcoreevent.h>
#include <QtCore/qpoint.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QWidget;

class QGraphicsSceneEventPrivate;
class Q_GUI_EXPORT QGraphicsSceneEvent : public QEvent
{
public:
    QGraphicsSceneEvent(Type type);
    ~QGraphicsSceneEvent();

    QWidget *widget() const;
    void setWidget(QWidget *widget);

protected:
    QGraphicsSceneEvent(QGraphicsSceneEventPrivate &dd, Type type = None);
    QGraphicsSceneEventPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QGraphicsSceneEvent)
};

class QGraphicsSceneMouseEventPrivate;
class Q_GUI_EXPORT QGraphicsSceneMouseEvent : public QGraphicsSceneEvent
{
public:
    QGraphicsSceneMouseEvent(Type type = None);
    ~QGraphicsSceneMouseEvent();

    inline bool isMousePress() const
    { return type() == GraphicsSceneMousePress; }
    inline bool isMouseMove() const
    { return type() == GraphicsSceneMouseMove; }
    inline bool isMouseRelease() const
    { return type() == GraphicsSceneMouseRelease; }
    inline bool isMouseClick() const
    { return type() == GraphicsSceneMouseClick; }
    inline bool isMouseDoubleClick() const
    { return type() == GraphicsSceneMouseDoubleClick; }

    QPointF pos() const;
    void setPos(const QPointF &pos);

    QPointF scenePos() const;
    void setScenePos(const QPointF &pos);

    QPoint screenPos() const;
    void setScreenPos(const QPoint &pos);
    
    QPointF buttonDownPos(Qt::MouseButton button) const;
    void setButtonDownPos(Qt::MouseButton button, const QPointF &pos);

    QPointF buttonDownScenePos(Qt::MouseButton button) const;
    void setButtonDownScenePos(Qt::MouseButton button, const QPointF &pos);

    QPoint buttonDownScreenPos(Qt::MouseButton button) const;
    void setButtonDownScreenPos(Qt::MouseButton button, const QPoint &pos);

    QPointF lastPos() const;
    void setLastPos(const QPointF &pos);

    QPointF lastScenePos() const;
    void setLastScenePos(const QPointF &pos);

    QPoint lastScreenPos() const;
    void setLastScreenPos(const QPoint &pos);

    Qt::MouseButtons buttons() const;
    void setButtons(Qt::MouseButtons buttons);

    Qt::MouseButton button() const;
    void setButton(Qt::MouseButton button);

    Qt::KeyboardModifiers modifiers() const;
    void setModifiers(Qt::KeyboardModifiers modifiers);
    
private:
    Q_DECLARE_PRIVATE(QGraphicsSceneMouseEvent)
};

class QGraphicsSceneContextMenuEventPrivate;
class Q_GUI_EXPORT QGraphicsSceneContextMenuEvent : public QGraphicsSceneEvent
{
public:
    QGraphicsSceneContextMenuEvent(Type type = None);
    ~QGraphicsSceneContextMenuEvent();
    
    QPointF pos() const;
    void setPos(const QPointF &pos);

    QPointF scenePos() const;
    void setScenePos(const QPointF &pos);
    
    QPoint screenPos() const;
    void setScreenPos(const QPoint &pos);

    Qt::KeyboardModifiers modifiers() const;
    void setModifiers(Qt::KeyboardModifiers modifiers);
    
private:
    Q_DECLARE_PRIVATE(QGraphicsSceneContextMenuEvent)
};

class QGraphicsSceneHoverEventPrivate;
class Q_GUI_EXPORT QGraphicsSceneHoverEvent : public QGraphicsSceneEvent
{
public:
    QGraphicsSceneHoverEvent(Type type = None);
    ~QGraphicsSceneHoverEvent();

    inline bool isHoverEnter() const
    { return type() == GraphicsSceneHoverEnter; }
    inline bool isHoverMove() const
    { return type() == GraphicsSceneHoverMove; }
    inline bool isHoverLeave() const
    { return type() == GraphicsSceneHoverLeave; }

    QPointF pos() const;
    void setPos(const QPointF &pos);

    QPointF scenePos() const;
    void setScenePos(const QPointF &pos);
    
    QPoint screenPos() const;
    void setScreenPos(const QPoint &pos);

private:
    Q_DECLARE_PRIVATE(QGraphicsSceneHoverEvent)
};

class QGraphicsSceneHelpEventPrivate;
class Q_GUI_EXPORT QGraphicsSceneHelpEvent : public QGraphicsSceneEvent
{
public:
    QGraphicsSceneHelpEvent(Type type = None);
    ~QGraphicsSceneHelpEvent();

    QPointF scenePos() const;
    void setScenePos(const QPointF &pos);
    
    QPoint screenPos() const;
    void setScreenPos(const QPoint &pos);

private:
    Q_DECLARE_PRIVATE(QGraphicsSceneHelpEvent)
};

QT_END_HEADER

#endif

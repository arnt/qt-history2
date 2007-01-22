#include <QtGui>

#include "diagramtextitem.h"
#include "diagramscene.h"

DiagramTextItem::DiagramTextItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsTextItem(parent, scene)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

QVariant DiagramTextItem::itemChange(GraphicsItemChange change, 
				     const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
	emit selectedChange(this);
    return value;
}

void DiagramTextItem::focusOutEvent(QFocusEvent *event)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
    emit lostFocus(this);
    QGraphicsTextItem::focusOutEvent(event);
}

void DiagramTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    scene()->clearSelection();
    QGraphicsTextItem::mousePressEvent(event);
    setSelected(true);
}

void DiagramTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (textInteractionFlags() & Qt::TextEditable) {
	QGraphicsTextItem::mouseMoveEvent(event);
    } else
	QGraphicsItem::mouseMoveEvent(event);
}

void DiagramTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsSceneMouseEvent *mouseEvent =
	new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    mouseEvent->setAccepted(true);
    mouseEvent->setPos(event->pos());
    mouseEvent->setScenePos(event->scenePos());
    mouseEvent->setScreenPos(event->screenPos());
    mouseEvent->setButtonDownPos(Qt::LeftButton, event->buttonDownPos(Qt::LeftButton));
    mouseEvent->setButtonDownScreenPos(Qt::LeftButton, event->buttonDownScreenPos(Qt::LeftButton));
    mouseEvent->setButtonDownScenePos(Qt::LeftButton, event->buttonDownScenePos(Qt::LeftButton));
    mouseEvent->setWidget(event->widget());

    QGraphicsTextItem::mousePressEvent(mouseEvent);

    delete mouseEvent;
}

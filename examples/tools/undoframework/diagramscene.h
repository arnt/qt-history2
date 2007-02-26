#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QObject>
#include <QGraphicsScene>

class QGraphicsSceneDragDropEvent;
class DiagramItem;
class QGraphicsViewItem;

class DiagramScene : public QGraphicsScene
{
    Q_OBJECT

public:
    DiagramScene(QObject *parent = 0) 
	: QGraphicsScene(parent) {};

signals:
    void itemMoved(DiagramItem *movedItem, const QPointF &movedFromPosition);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsItem *movingItem;
    QPointF oldPos;
};

#endif

#include "graphwidget.h"
#include "edge.h"
#include "node.h"

#include <QGraphicsScene>
#include <QWheelEvent>

GraphWidget::GraphWidget()
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    scene->setSceneRect(-200, -200, 400, 400);

    Node *node1 = new Node;
    Node *node2 = new Node;
    Node *node3 = new Node;
    Node *node4 = new Node;
    Node *node5 = new Node;
    Node *node6 = new Node;
    Node *node7 = new Node;
    Node *node8 = new Node;
    Node *node9 = new Node;
    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);
    scene->addItem(node4);
    scene->addItem(node5);
    scene->addItem(node6);
    scene->addItem(node7);
    scene->addItem(node8);
    scene->addItem(node9);
    scene->addItem(new Edge(node1, node2));
    scene->addItem(new Edge(node2, node3));
    scene->addItem(new Edge(node2, node5));
    scene->addItem(new Edge(node3, node6));
    scene->addItem(new Edge(node4, node1));
    scene->addItem(new Edge(node4, node5));
    scene->addItem(new Edge(node5, node6));
    scene->addItem(new Edge(node5, node8));
    scene->addItem(new Edge(node6, node9));
    scene->addItem(new Edge(node7, node4));
    scene->addItem(new Edge(node8, node7));
    scene->addItem(new Edge(node9, node8));

    node1->setPos(-50, -50);
    node2->setPos(0, -50);
    node3->setPos(50, -50);
    node4->setPos(-50, 0);
    node5->setPos(0, 0);
    node6->setPos(50, 0);
    node7->setPos(-50, 50);
    node8->setPos(0, 50);
    node9->setPos(50, 50);
    
    startTimer(1000 / 25);
    setMinimumSize(400, 400);
    scale(0.8, 0.8);

    setSceneRect(scene->sceneRect());
}

void GraphWidget::paintBackground(QPainter *painter, const QRectF &rect)
{
    // Shadow
    painter->fillRect(rect.intersect(scene()->sceneRect().adjusted(5, 5, 5, 5))
                      .adjusted(-1, -1, 1, 1), Qt::darkGray);

    // Fill
    QLinearGradient gradient(scene()->sceneRect().topLeft(),
                             scene()->sceneRect().bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->setBrush(gradient);
    painter->drawRect(rect.intersect(scene()->sceneRect()).adjusted(-1, -1, 1, 1));

    // Text
    QFont font = painter->font();
    font.setPointSize(15);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::lightGray);
    QString message(tr("Click and drag the nodes around\nor zoom with the mouse wheel"));
    painter->drawText(scene()->sceneRect().adjusted(2, 2, 0, 0),
                      Qt::AlignTop | Qt::AlignHCenter, message);
    painter->setPen(Qt::black);
    painter->drawText(scene()->sceneRect(),
                      Qt::AlignTop | Qt::AlignHCenter, message);    
}

void GraphWidget::timerEvent(QTimerEvent *)
{
    QList<Node *> nodes;
    foreach (QGraphicsItem *item, scene()->items()) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
            nodes << node;
    }

    foreach (Node *node, nodes)
        node->calculateForces();
    foreach (Node *node, nodes)
        node->advance();
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scale(1 + event->delta() / 1200.0, 1 + event->delta() / 1200.0);
}

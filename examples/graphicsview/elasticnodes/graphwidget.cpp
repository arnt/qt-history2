#include "graphwidget.h"
#include "edge.h"
#include "node.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QWheelEvent>

class Scene : public QGraphicsScene
{
public:
    Scene(QObject *parent)
        : QGraphicsScene(parent)
    { }

protected:
    void drawBackground(QPainter *painter, const QRectF &rect)
    {
        Q_UNUSED(rect);

        // Shadow
        QRectF sceneRect = this->sceneRect();
        painter->fillRect(sceneRect.translated(5, 5), Qt::darkGray);

        // Fill
        QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, Qt::lightGray);
        painter->setBrush(gradient);
        painter->drawRect(sceneRect);

        // Text
        QFont font = painter->font();
        font.setPointSize(14);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(Qt::lightGray);
        QString message(tr("Click and drag the nodes around\nor zoom with the mouse wheel"));
        painter->drawText(sceneRect.translated(2, 2), Qt::AlignTop | Qt::AlignHCenter,
                          message);
        painter->setPen(Qt::black);
        painter->drawText(sceneRect, Qt::AlignTop | Qt::AlignHCenter,
                          message);
    }
};

GraphWidget::GraphWidget()
    : timerId(0)
{
    Scene *scene = new Scene(this);
    setScene(scene);
    scene->setSceneRect(-200, -200, 400, 400);

    Node *node1 = new Node(this);
    Node *node2 = new Node(this);
    Node *node3 = new Node(this);
    Node *node4 = new Node(this);
    Node *node5 = new Node(this);
    Node *node6 = new Node(this);
    Node *node7 = new Node(this);
    Node *node8 = new Node(this);
    Node *node9 = new Node(this);
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
    
    setMinimumSize(400, 400);
    scale(0.8, 0.8);

    itemMoved();

    setWindowTitle(tr("Elastic Nodes"));
}

void GraphWidget::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
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

    bool itemsMoved = false;
    foreach (Node *node, nodes) {
        if (node->advance())
            itemsMoved = true;
    }

    if (!itemsMoved) {
        killTimer(timerId);
        timerId = 0;
    }
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scale(1 + event->delta() / 1200.0, 1 + event->delta() / 1200.0);
}

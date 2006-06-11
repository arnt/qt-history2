#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QtGui/QGraphicsView>

class Node;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    GraphWidget();

    void itemMoved();

protected:
    void keyPressEvent(QKeyEvent *event);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    int timerId;
    Node *centerNode;
};

#endif

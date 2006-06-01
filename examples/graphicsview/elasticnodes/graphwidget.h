#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QtGui/QGraphicsView>

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    GraphWidget();

    void itemMoved();

protected:
    void timerEvent(QTimerEvent *);
    void wheelEvent(QWheelEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    int timerId;
};

#endif

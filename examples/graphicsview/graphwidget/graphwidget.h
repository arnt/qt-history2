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
    void paintBackground(QPainter *painter, const QRectF &rect);
    void timerEvent(QTimerEvent *);
    void wheelEvent(QWheelEvent *event);

private:
    int timerId;
};

#endif

#ifndef MOUSE_H
#define MOUSE_H

#include <QGraphicsItem>
#include <QObject>

class Mouse : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    Mouse();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

protected:
    void timerEvent(QTimerEvent *event);

private:
    qreal angle;
    qreal mouseEyeDirection;
    QColor color;
};

#endif

/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CAR_H
#define CAR_H

#include <QGraphicsItem>
#include <QObject>
#include <QBrush>

class Car : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    Car();
    QRectF boundingRect() const;

public Q_SLOTS:
    void accelerate();
    void decelerate();
    void turnLeft();
    void turnRight();

Q_SIGNALS:
    void crashed();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void timerEvent(QTimerEvent *event);

private:
    QBrush color;
    qreal wheelsAngle; // used when applying rotation
    qreal speed; // delta movement along the body axis
};

#endif // CAR_H

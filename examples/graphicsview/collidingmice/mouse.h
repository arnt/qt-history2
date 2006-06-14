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
    qreal speed;
    qreal mouseEyeDirection;
    QColor color;
};

#endif

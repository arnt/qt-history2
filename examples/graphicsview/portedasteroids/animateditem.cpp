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

#include "animateditem.h"

#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>

AnimatedPixmapItem::AnimatedPixmapItem(const QList<QPixmap> &animation,
                                       QGraphicsScene *scene)
    : QGraphicsItem(0, scene), currentFrame(0), vx(0), vy(0)
{
    for (int i = 0; i < animation.size(); ++i) {
        QPixmap pixmap = animation.at(i);
        Frame frame;
        frame.pixmap = pixmap;
        QPainterPath path;
        path.addRegion(pixmap.createHeuristicMask());
        frame.shape = path;
        frame.boundingRect = path.controlPointRect();
        frames << frame;
    }
}

void AnimatedPixmapItem::setFrame(int frame)
{
    if (!frames.isEmpty()) {
        prepareGeometryChange();
        currentFrame = frame % frames.size();
    }
}

void AnimatedPixmapItem::advance(int phase)
{
    if (phase == 1 && !frames.isEmpty()) {
        setFrame(currentFrame + 1);
        if (vx || vy)
            moveBy(vx, vy);
    }
}

QRectF AnimatedPixmapItem::boundingRect() const
{
    return frames.at(currentFrame).boundingRect;
}

QPainterPath AnimatedPixmapItem::shape() const
{
    return frames.at(currentFrame).shape;
}

void AnimatedPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    painter->drawPixmap(0, 0, frames.at(currentFrame).pixmap);
}

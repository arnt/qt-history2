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

AnimatedPixmapItem::AnimatedPixmapItem(const QList<QPixmap> &animation,
                                       QGraphicsScene *scene)
    : QGraphicsPixmapItem(0, scene), currentFrame(0), frames(animation), vx(0), vy(0)
{
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
}

void AnimatedPixmapItem::setFrame(int frame)
{
    if (!frames.isEmpty()) {
        currentFrame = frame % frames.size();
        setPixmap(frames.at(currentFrame));
    }
}

void AnimatedPixmapItem::advance(int phase)
{
    if (phase == 1 && !frames.isEmpty()) {
        currentFrame = (currentFrame + 1) % frames.size();
        setPixmap(frames.at(currentFrame));
        if (vx || vy)
            moveBy(vx, vy);
    }
}

QPainterPath AnimatedPixmapItem::shape() const
{
    QPainterPath path;
    path.addRect(0, 0,
                 frames.at(currentFrame).width(),
                 frames.at(currentFrame).height());
    return path;
}

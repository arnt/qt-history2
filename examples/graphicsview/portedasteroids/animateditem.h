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

#ifndef ANIMATEDPIXMAPITEM_H
#define ANIMATEDPIXMAPITEM_H

#include <QGraphicsItem>

class AnimatedPixmapItem : public QGraphicsPixmapItem
{
public:
    AnimatedPixmapItem(const QList<QPixmap> &animation, QGraphicsScene *scene = 0);

    void setFrame(int frame);
    inline int frame() const
    { return currentFrame; }
    inline int frameCount() const
    { return frames.size(); }
    inline QPixmap image(int frame) const
    { return frames.isEmpty() ? QPixmap() : frames.at(frame % frames.size()); }
    inline void setVelocity(qreal xvel, qreal yvel)
    { vx = xvel; vy = yvel; }
    inline qreal xVelocity() const
    { return vx; }
    inline qreal yVelocity() const
    { return vy; }

    QPainterPath shape() const;
    
    void advance(int phase);
    
private:
    int currentFrame;
    QList<QPixmap> frames;
    qreal vx, vy;
};

#endif

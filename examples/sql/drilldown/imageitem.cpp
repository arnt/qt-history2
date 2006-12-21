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

#include "imageitem.h"

ImageItem::ImageItem(int id, const QPixmap &pixmap, QGraphicsItem *parent,
                     QGraphicsScene *scene)
    : QGraphicsPixmapItem(pixmap, parent, scene)
{
    recordId = id;
    setAcceptsHoverEvents(true);

    timeLine.setDuration(150);
    timeLine.setFrameRange(0, 150);

    connect(&timeLine, SIGNAL(frameChanged(int)), this, SLOT(setFrame(int)));
    connect(&timeLine, SIGNAL(finished()), this, SLOT(updateZValue()));

    adjust();
}

void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    timeLine.setDirection(QTimeLine::Forward);
    if (timeLine.state() == QTimeLine::NotRunning)
        timeLine.start();

    z = 1.0;
    updateZValue();

    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void ImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    timeLine.setDirection(QTimeLine::Backward);
    if (timeLine.state() == QTimeLine::NotRunning)
        timeLine.start();

    z = 0.0;

    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

void ImageItem::setFrame(int frame)
{
    setMatrix(adjustedMatrix);
    QPointF center = boundingRect().center();

    translate(center.x(), center.y());
    scale(1 + frame / 330.0, 1 + frame / 330.0);
    translate(-center.x(), -center.y());
}

void ImageItem::updateZValue()
{
    setZValue(z);
}

void ImageItem::adjust()
{
    adjustedMatrix.reset();
    adjustedMatrix.scale(150/ boundingRect().width(),
                         120/ boundingRect().height());

    setMatrix(adjustedMatrix);
}

int ImageItem::id()
{
    return recordId;
}


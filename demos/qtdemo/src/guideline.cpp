/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "guideline.h"
#include <cmath>

GuideLine::GuideLine(const QLineF &line, Guide *follows) : Guide(follows)
{
    this->line = line;
}

GuideLine::GuideLine(const QPointF &end, Guide *follows) : Guide(follows)
{
    if (follows)
        this->line = QLineF(prevGuide->endPos(), end);
    else
        this->line = QLineF(QPointF(0, 0), end);
}

float GuideLine::length()
{
    return line.length();
}

QPointF GuideLine::startPos()
{
    return QPointF(this->line.p1().x() * scaleX, this->line.p1().y() * scaleY);
}

QPointF GuideLine::endPos()
{
    return QPointF(this->line.p2().x() * scaleX, this->line.p2().y() * scaleY);
}

void GuideLine::guide(DemoItem *item, float moveSpeed)
{
    float frame = item->guideFrame - this->startLength;
    float endX = (this->line.p1().x() + (frame * this->line.dx() / this->length())) * scaleX;
    float endY = (this->line.p1().y() + (frame * this->line.dy() / this->length())) * scaleY;
    QPointF pos(endX, endY);
    this->move(item, pos, moveSpeed);
}


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

#include <cmath>
#include "guide.h"
#include "colors.h"

Guide::Guide(Guide *follows)
{
    this->scaleX = 1.0;
    this->scaleY = 1.0;
    
    if (follows){
        while (follows->nextGuide != follows->firstGuide) // append to end
            follows = follows->nextGuide;
        
        follows->nextGuide = this;
        this->prevGuide = follows;
        this->firstGuide = follows->firstGuide;
        this->nextGuide = follows->firstGuide;
        this->startLength = int(follows->startLength + follows->length()) + 1;
    }
    else{
        this->prevGuide = this;
        this->firstGuide = this;
        this->nextGuide = this;
        this->startLength = 0;
    }
}

void Guide::setScale(float scaleX, float scaleY, bool all)
{
    this->scaleX = scaleX;
    this->scaleY = scaleY;
    
    if (all){
        Guide *next = this->nextGuide;
        while(next != this){
            next->scaleX = scaleX;
            next->scaleY = scaleY;
            next = next->nextGuide;
        }
    }
}

void Guide::setFence(const QRectF &fence, bool all)
{
    this->fence = fence;
    
    if (all){
        Guide *next = this->nextGuide;
        while(next != this){
            next->fence = fence;
            next = next->nextGuide;
        }
    }
}

Guide::~Guide()
{
    if (this != this->nextGuide && this->nextGuide != this->firstGuide)
        delete this->nextGuide;
}

float Guide::lengthAll()
{
    float len = length();
    Guide *next = this->nextGuide;
    while(next != this){
        len += next->length();
        next = next->nextGuide;
    }
    return len;
}

void Guide::move(DemoItem *item, QPointF &dest, float moveSpeed)
{    
    QLineF walkLine(item->getGuidedPos(), dest);  
    if (moveSpeed >= 0 && walkLine.length() > moveSpeed){
        // The item is too far away from it's destination point.
        // So we choose to move it towards it instead.
        float dx = walkLine.dx();
        float dy = walkLine.dy();
        
        if (qAbs(dx) > qAbs(dy)){
            // walk along x-axis
            if (dx != 0){
                float d = moveSpeed * dy / qAbs(dx);
                float s = dx > 0 ? moveSpeed : -moveSpeed;
                dest.setX(item->getGuidedPos().x() + s);
                dest.setY(item->getGuidedPos().y() + d);
            }
        }
        else{
            // walk along y-axis
            if (dy != 0){
                float d = moveSpeed * dx / qAbs(dy);
                float s = dy > 0 ? moveSpeed : -moveSpeed;
                dest.setX(item->getGuidedPos().x() + d);
                dest.setY(item->getGuidedPos().y() + s);
            }
        }
    }
  
    item->setGuidedPos(dest);
}

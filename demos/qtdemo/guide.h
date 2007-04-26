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

#ifndef GUIDE_H
#define GUIDE_H

#include "demoitem.h"

class Guide
{
public:
    Guide(Guide *follows = 0);
    virtual ~Guide();
    
    virtual void guide(DemoItem *item, float moveSpeed) = 0;
    void move(DemoItem *item, QPointF &dest, float moveSpeed);
    virtual QPointF startPos(){ return QPointF(0, 0); };
    virtual QPointF endPos(){ return QPointF(0, 0); };
    virtual float length(){ return 1; };
    float lengthAll();
    
    void setScale(float scaleX, float scaleY, bool all = true);
    void setFence(const QRectF &fence, bool all = true);
    
    int startLength;
    Guide *nextGuide;
    Guide *firstGuide;
    Guide *prevGuide;
    float scaleX;
    float scaleY;
    QRectF fence;
};

#endif // GUIDE_H


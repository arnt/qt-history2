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

#ifndef GUIDECIRCLE_H
#define GUIDECIRCLE_H

#include "guide.h"
#include "demoitem.h"

class GuideCircle : public Guide
{
public:
    enum DIRECTION {CW = 1, CCW = -1};
    
    GuideCircle(const QRectF &rect, float startAngle = 0, float span = 360, DIRECTION dir = CCW, Guide *follows = 0);
    
    void guide(DemoItem *item, float moveSpeed); // overridden
    QPointF startPos();
    QPointF endPos();
    float length();
    
private:
    float posX;
    float posY;
    float radiusX;
    float radiusY;
    float startAngleRad;
    float endAngleRad;
    float spanRad;
    float stepAngleRad;
};

#endif // GUIDECIRCLE_H


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

#ifndef GUIDELINE_H
#define GUIDELINE_H

#include "guide.h"
#include "demoitem.h"

class GuideLine : public Guide
{
public:
    GuideLine(const QLineF &line, Guide *follows = 0);
    GuideLine(const QPointF &end, Guide *follows = 0);
    
    void guide(DemoItem *item, float moveSpeed); // overridden
    QPointF startPos();
    QPointF endPos();
    float length();
    
private:
    QLineF line;
    
};

#endif // GUIDELINE_H


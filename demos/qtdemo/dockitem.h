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

#ifndef DOCK_ITEM_H
#define DOCK_ITEM_H

#include <QtGui>
#include "demoitem.h"

class DockItem : public DemoItem
{
public:
    enum ORIENTATION {UP, DOWN, LEFT, RIGHT};
    
    DockItem(ORIENTATION orien, qreal x, qreal y, qreal width, qreal length, QGraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    virtual ~DockItem();
    
    virtual QRectF boundingRect() const; // overridden
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0); // overridden
    
    qreal length;
    qreal width;
    ORIENTATION orientation;
    
private:
    void setupPixmap();
    QPixmap *pixmap;
};

#endif // DOCK_ITEM_H


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

#include "dockitem.h"
#include "colors.h"

DockItem::DockItem(ORIENTATION orien, qreal x, qreal y, qreal width, qreal length, QGraphicsScene *scene, QGraphicsItem *parent)
    : DemoItem(scene, parent)
{
    this->orientation = orien;
    this->width = width;
    this->length = length;
    this->setPos(x, y);
    this->setZValue(40);
    this->setupPixmap();
}

void DockItem::setupPixmap()
{
    this->pixmap = new QPixmap(int(this->boundingRect().width()), int(this->boundingRect().height()));
    this->pixmap->fill(QColor(0, 0, 0, 0));
    QPainter painter(this->pixmap);
    // create brush:
    QColor background = Colors::sceneBg1;
    QLinearGradient brush(0, 0, 0, this->boundingRect().height());
    brush.setSpread(QGradient::PadSpread);
    
    if (this->orientation == DOWN){
        brush.setColorAt(0.0, background);
        brush.setColorAt(0.2, background);
        background.setAlpha(0);
        brush.setColorAt(1.0, background);
    }
    else
        if (this->orientation == UP){
        brush.setColorAt(1.0, background);
        brush.setColorAt(0.8, background);
        background.setAlpha(0);
        brush.setColorAt(0.0, background);
    }
    else
        qWarning("DockItem doesn't support the orientation given!");
    
    painter.fillRect(0, 0, int(this->boundingRect().width()), int(this->boundingRect().height()), brush);
    
}

DockItem::~DockItem()
{
    delete this->pixmap;
}

QRectF DockItem::boundingRect() const
{
    if (this->orientation == UP || this->orientation == DOWN)
        return QRectF(0, 0, this->length, this->width);
    else
        return QRectF(0, 0, this->width, this->length);
}

void DockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->drawPixmap(0, 0, *this->pixmap);
}




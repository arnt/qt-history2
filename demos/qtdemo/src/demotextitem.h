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

#ifndef DEMO_TEXT_ITEM_H
#define DEMO_TEXT_ITEM_H

#include <QtGui>
#include "demoitem.h"
 
class DemoTextItem : public DemoItem
{
public:
    enum TYPE {STATIC_TEXT, DYNAMIC_TEXT};
     
    DemoTextItem(const QString &text, const QFont &font, const QColor &textColor,
        float textWidth, QGraphicsScene *scene = 0, QGraphicsItem *parent = 0, TYPE type = STATIC_TEXT, const QColor &bgColor = QColor());
    void setText(const QString &text);
    virtual QRectF boundingRect() const; // overridden

protected:
    virtual QImage *createImage(const QMatrix &matrix) const; // overridden
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = 0, QWidget *widget = 0); // overridden
    
private:
    float textWidth;
    QString text;
    QFont font;
    QColor textColor;
    QColor bgColor;
    TYPE type;
};

#endif // DEMO_TEXT_ITEM_H


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

#ifndef MENU_CONTENT_ITEM_H
#define MENU_CONTENT_ITEM_H

#include <QtGui>
#include <QtXml>
#include "demoitem.h"

class HeadingItem;
class DemoTextItem;

class MenuContentItem : public DemoItem
{
    
public:
    MenuContentItem(const QDomElement &el, QGraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    
    virtual QRectF boundingRect() const; // overridden
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = 0){}; // overridden
    void animationStopped(int id);
    void prepare();
    
private:
    QString name;
    QString readmePath;
    HeadingItem *heading;
    DemoTextItem *description1;
    DemoTextItem *description2;
    
    QString loadDescription(int startPara, int nrPara);
    QString extractTextFromParagraph(const QDomNode &parentNode);
    
    void createContent();
};

#endif // MENU_CONTENT_ITEM_H


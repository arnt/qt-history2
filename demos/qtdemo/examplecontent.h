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

#ifndef CONTENT_ITEM_H
#define CONTENT_ITEM_H

#include <QtGui>
#include <QtXml>
#include "demoitem.h"

class ExampleContent : public DemoItem
{
    
public:
    ExampleContent(const QString &name, QGraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = 0){};
    void prepare();
    
private:
    QString name;
    
    QString loadDescription();
    QString extractTextFromParagraph(const QDomNode &parentNode);	
    bool isSummary(const QString &text);	
    void createContent();
};

#endif // CONTENT_ITEM_H


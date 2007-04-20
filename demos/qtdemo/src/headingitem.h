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

#ifndef HEADING_ITEM_H
#define HEADING_ITEM_H

#include <QtGui>
#include "demoitem.h"
 
class HeadingItem : public DemoItem
{
public:
    HeadingItem(const QString &text, QGraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    void animationStarted(int id = 0);
    void animationStopped(int id = 0);

protected:
    virtual QImage *createImage(const QMatrix &matrix) const; // overridden
    
private:
    QString text;
};

#endif // HEADING_ITEM_H


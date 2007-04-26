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

#ifndef IMAGE_ITEM_H
#define IMAGE_ITEM_H

#include <QtGui>
#include "demoitem.h"

class ImageItem : public DemoItem
{
public:
    ImageItem(const QString &path, int maxWidth, int maxHeight, QGraphicsScene *scene = 0, QGraphicsItem *parent = 0,
         bool adjustSize = false, float scale = 1.0f);
    
    bool adjustSize;
    float scale;
protected:
    QImage *createImage(const QMatrix &matrix) const;

private:
    QString path;
    int maxWidth;
    int maxHeight;
};

#endif // DOCK_ITEM_H


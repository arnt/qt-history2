/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ITEMS_H
#define ITEMS_H

#include "demowidget.h"

#include <QList>

class Item;
class Items : public DemoWidget
{
public:
    Items(QWidget *parent = 0);
    ~Items();

    void resetState();
    void startAnimation() { resetState(); }
    void stopAnimation() {}
    void drawItems(const QRect &rect);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QPixmap buffer;
    QList<Item *> items;
    Item *selectedItem;
};

#endif

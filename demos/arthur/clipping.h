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

#ifndef CLIPPING_H
#define CLIPPING_H

#include "demowidget.h"

#include <qpixmap.h>

class QMouseEvent;

class Clipping : public DemoWidget
{
public:
    Clipping(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);
    void timerEvent(QTimerEvent *e);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    QList<QRectF> rects;
    QList<QPointF> rectDirection;
    int lastStep;

    QPoint pressPoint;
    QPoint currentPoint;
};

#endif

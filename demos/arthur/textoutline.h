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

#ifndef TEXTOUTLINE_H
#define TEXTOUTLINE_H

#include "demowidget.h"

#include <qpainterpath.h>

class TextOutline : public DemoWidget
{
public:
    enum DragLocation { TopLeft, TopRight, BottomLeft, BottomRight };

    TextOutline(QWidget *parent=0);

    void paintEvent(QPaintEvent *e);

    void showEvent(QShowEvent *e);
    void startAnimation();
    void stopAnimation();
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void resetState() { update(); }

private:

    void drawTarget(QPainter *p, const QPoint &pt);
    void updatePath();
    QPointF mapPoint(float x, float y, bool *ok);

    QVector<QPainterPath> basePaths;
    QVector<QPainterPath> xpaths;
    QRectF basePathBounds;
    DragLocation dragLocation;
    QPoint pul, pur, pbl, pbr;
};

#endif

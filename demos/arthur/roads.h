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

#ifndef ROADS_H
#define ROADS_H

#include "demowidget.h"

#include <qpainterpath.h>
#include <qpolygon.h>

class Roads : public DemoWidget
{
public:
    Roads(QWidget *parent=0);

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *) { stopAnimation(); }
    void mouseReleaseEvent(QMouseEvent *) { startAnimation(); }

    void hideEvent(QHideEvent *) { resetState(); }

    void resetState() { backBuffer = QPixmap(); }

private:
    QPainterPath yellowLine;
    QList<QPolygonF> carVectors;
    QPixmap pixmap;
    QPixmap backBuffer;

};

#endif

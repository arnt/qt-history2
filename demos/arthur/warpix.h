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

#ifndef WARPIX_H
#define WARPIX_H

#include "demowidget.h"

#include <qpixmap.h>
#include <qsound.h>

class QKeyEvent;

class Warpix : public DemoWidget
{
public:
    Warpix(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

    void setPixmap(const QPixmap &pix);

    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    QPixmap p1, p2, p3, p4;
    QPixmap buffer;

    double a, b, c, d;

    QPoint clickPos;

    QSound *beat;
};

#endif // WARPIX_H

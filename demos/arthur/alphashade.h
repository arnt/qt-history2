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

#ifndef ALPHASHADE_H
#define ALPHASHADE_H

#include "demowidget.h"

#include <qpixmap.h>
#include <qpointarray.h>

class AlphaShade : public DemoWidget
{
public:
    AlphaShade(QWidget *parent=0);
    void paintEvent(QPaintEvent *e);
    void drawPrimitives(QPainter *p);

private:
    int iterations;
    int spread;
    QPointArray polygon;
};

#endif // ALPHASHADE_H

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

#ifndef ROTATINGGRADIENT_H
#define ROTATINGGRADIENT_H

#include "demowidget.h"

#include <qwmatrix.h>

class RotatingGradient : public DemoWidget
{
public:
    RotatingGradient(QWidget *parent=0);
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *e);
private:
    QWMatrix matrix;
    QPixmap pixmap;
};

#endif // ROTATINGGRADIENT_H

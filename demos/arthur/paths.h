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

#ifndef PATHS_H
#define PATHS_H

#include <QPixmap>
#include "demowidget.h"

class Paths : public DemoWidget
{
public:
    Paths(QWidget *parent=0);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *) { stopAnimation(); }
    void mouseReleaseEvent(QMouseEvent *) { startAnimation(); }

private:
    QPixmap dblBuffer;
};

#endif

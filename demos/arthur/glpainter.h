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

#ifndef GLPAINTER_H
#define GLPAINTER_H

#include <qbasictimer.h>
#include "demowidget.h"

class GLWidget;

class GLPainter : public DemoWidget
{
public:
    GLPainter(QWidget *parent = 0);

    void startAnimation();
    void stopAnimation();

private:
    GLWidget *glwidget;
    int id;
    QBasicTimer animTimer;
};

#endif // GLPAINTER_H

/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TABLETAPPLICATION_H
#define TABLETAPPLICATION_H

#include <QApplication>

#include "tabletcanvas.h"

class TabletApplication : public QApplication
{
    Q_OBJECT

public:
    TabletApplication(int &argv, char **args)
    : QApplication(argv, args) {}

    bool event(QEvent *event);
    void setCanvas(TabletCanvas *canvas)
        { myCanvas = canvas; }

private:
    TabletCanvas *myCanvas;
};

#endif

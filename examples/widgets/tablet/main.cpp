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

#include <QtGui>

#include "mainwindow.h"
#include "tabletapplication.h"
#include "tabletcanvas.h"

int main(int argv, char *args[])
{
    TabletApplication app(argv, args);
    TabletCanvas *canvas = new TabletCanvas;
    app.setCanvas(canvas);

    MainWindow mainWindow(canvas);
    mainWindow.resize(500, 500);
    mainWindow.show();

    return app.exec();
}

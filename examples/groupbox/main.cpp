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

#include <QApplication>

#include "window.h"

/*
    Creates a window to show the use of group boxes in Qt 4.
*/

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window *window = new Window;
    window->show();
    app.setMainWidget(window);

    return app.exec();
}

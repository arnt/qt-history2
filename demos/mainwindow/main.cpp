/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>

#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.connect(&app, SIGNAL(lastWindowClosed()), SLOT(quit()));

    MainWindow *mainwindow = new MainWindow(0, Qt::WDestructiveClose);
    mainwindow->setWindowTitle("Qt Main Window Demo");
    mainwindow->show();

    return app.exec();
}

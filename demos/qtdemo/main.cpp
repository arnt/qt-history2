/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
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
#include "menumanager.h"
#include "colors.h"

static void artisticSleep(int sleepTime)
{
    QTime time;
    time.restart();
    while (time.elapsed() < sleepTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qtdemo);
    QApplication app(argc, argv);
    Colors::parseArgs(argc, argv);
    MainWindow mainWindow;
    MenuManager::instance()->init(&mainWindow);
    mainWindow.setFocus();

    if (Colors::fullscreen)
        mainWindow.showFullScreen();
    else {
        mainWindow.enableMask(true);
        mainWindow.show();
    }

    artisticSleep(500);
    mainWindow.start();
    return app.exec();
}

/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "browser.h"

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow mainWin;
    mainWin.setWindowTitle(QObject::tr("Qt SQL Browser"));

    Browser browser(&mainWin);
    mainWin.setCentralWidget(&browser);

    QMenu *menu = mainWin.menuBar()->addMenu(QObject::tr("&File"));
    menu->addAction(QObject::tr("Add &Connection..."), &browser, SLOT(addConnection()));
    menu->addSeparator();
    menu->addAction(QObject::tr("&Quit"), &app, SLOT(quit()));

    QObject::connect(&browser, SIGNAL(statusMessage(QString)),
                     mainWin.statusBar(), SLOT(showMessage(QString)));
    mainWin.show();

    return app.exec();
}

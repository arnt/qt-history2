/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include <qapplication.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qstatusbar.h>

#include "browser.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow mainWin;
    mainWin.setWindowTitle(QObject::tr("Qt SQL Browser"));

    Browser browser(&mainWin);
    mainWin.setCentralWidget(&browser);

    QMenu *menu = mainWin.menuBar()->addMenu(QObject::tr("&File"));
    menu->addAction(QObject::tr("Add &connection"), &browser, SLOT(addConnection()));
    menu->addSeparator();
    menu->addAction(QObject::tr("&Quit"), &app, SLOT(quit()));

    QObject::connect(&browser, SIGNAL(statusMessage(QString)),
                     mainWin.statusBar(), SLOT(showMessage(QString)));
    mainWin.show();

    return app.exec();
}


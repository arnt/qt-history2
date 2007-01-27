/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/qapplication.h>
#include <QtGui/qmainwindow.h>
#include <QtGui/qtabwidget.h>
#include <QtDBus/qdbusconnection.h>
#include "qdbusviewer.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow mw;

    QMenu *fileMenu = mw.menuBar()->addMenu(QObject::tr("&File"));
    QAction *quitAction = fileMenu->addAction(QObject::tr("&Exit"));
    QObject::connect(quitAction, SIGNAL(triggered()), &mw, SLOT(close()));
    quitAction->setShortcuts(QKeySequence::keyBindings(QKeySequence::Close));

    QTabWidget *mainWidget = new QTabWidget;
    mw.setCentralWidget(mainWidget);
    mainWidget->addTab(new QDBusViewer(QDBusConnection::sessionBus()), QObject::tr("Session Bus"));
    mainWidget->addTab(new QDBusViewer(QDBusConnection::systemBus()), QObject::tr("System Bus"));

    mw.show();

    return app.exec();
}


#include <QtGui>

#include "arrowpad.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    arrowPad = new ArrowPad;
    setCentralWidget(arrowPad);

    exitAct = new QAction(tr("E&xit"));
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);
}

/****************************************************************
**
** Implementation of MainWindow class, translation tutorial 2
**
****************************************************************/

#include "arrowpad.h"
#include "mainwindow.h"

#include <QApplication>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ArrowPad *ap = new ArrowPad(this);
    setCentralWidget(ap);

    QMenu *file = new QMenu(this);
    QAction *exitAction = file->addAction(tr("E&xit"), qApp, SLOT(quit()));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    
    menuBar()->addMenu(file)->setText(tr("&File"));
}

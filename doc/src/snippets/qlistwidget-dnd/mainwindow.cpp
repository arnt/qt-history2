#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    menuBar()->addMenu(fileMenu);

//  For convenient quoting:
QListWidget *listWidget = new QListWidget(this);
listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
listWidget->setDragEnabled(true);
listWidget->setAcceptDrops(true);
listWidget->setDropIndicatorShown(true);

    this->listWidget = listWidget;

    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    setupListItems();

    setCentralWidget(listWidget);
    setWindowTitle(tr("List Widget"));
}

void MainWindow::setupListItems()
{
    QListWidgetItem *item;
    item = new QListWidgetItem(tr("Oak"), listWidget);
    item = new QListWidgetItem(tr("Fir"), listWidget);
    item = new QListWidgetItem(tr("Pine"), listWidget);
    item = new QListWidgetItem(tr("Birch"), listWidget);
    item = new QListWidgetItem(tr("Hazel"), listWidget);
    item = new QListWidgetItem(tr("Redwood"), listWidget);
    item = new QListWidgetItem(tr("Sycamore"), listWidget);
    item = new QListWidgetItem(tr("Chestnut"), listWidget);
    item = new QListWidgetItem(tr("Mahogany"), listWidget);
}

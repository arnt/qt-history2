#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    QMenu *itemsMenu = new QMenu(tr("&Items"));

    QAction *insertAction = itemsMenu->addAction(tr("&Insert Item"));
    removeAction = itemsMenu->addAction(tr("&Remove Item"));
    removeAction->setEnabled(false);
    QAction *ascendingAction = itemsMenu->addAction(tr("Sort in &Ascending Order"));
    QAction *descendingAction = itemsMenu->addAction(tr("Sort in &Descending Order"));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(itemsMenu);

    listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(ascendingAction, SIGNAL(triggered()), this, SLOT(sortAscending()));
    connect(descendingAction, SIGNAL(triggered()), this, SLOT(sortDescending()));
    connect(insertAction, SIGNAL(triggered()), this, SLOT(insertItem()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));
    connect(listWidget,
            SIGNAL(currentChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(updateMenus(QListWidgetItem *)));

    setupListItems();

    setCentralWidget(listWidget);
    setWindowTitle(tr("List Widget"));
}

void MainWindow::setupListItems()
{
    new QListWidgetItem(tr("Oak"), listWidget);
    new QListWidgetItem(tr("Fir"), listWidget);
    new QListWidgetItem(tr("Pine"), listWidget);
    new QListWidgetItem(tr("Birch"), listWidget);
    new QListWidgetItem(tr("Hazel"), listWidget);
    new QListWidgetItem(tr("Redwood"), listWidget);
    new QListWidgetItem(tr("Sycamore"), listWidget);
    new QListWidgetItem(tr("Chestnut"), listWidget);
    new QListWidgetItem(tr("Mahogany"), listWidget);
}

void MainWindow::sortAscending()
{
    listWidget->sortItems(Qt::AscendingOrder);
}

void MainWindow::sortDescending()
{
    listWidget->sortItems(Qt::DescendingOrder);
}

void MainWindow::insertItem()
{
    QString itemText = QInputDialog::getText(tr("Insert Item"),
        tr("Input text for the new item:"));

    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText(itemText);
    int row = listWidget->row(listWidget->currentItem());
    listWidget->insertItem(row, newItem);
}

void MainWindow::removeItem()
{
    listWidget->takeItem(listWidget->row(listWidget->currentItem()));
}

void MainWindow::updateMenus(QListWidgetItem *current)
{
    removeAction->setEnabled(current != 0);
}

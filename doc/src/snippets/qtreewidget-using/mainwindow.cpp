#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    QMenu *itemsMenu = new QMenu(tr("&Items"));

    QAction *insertAction = itemsMenu->addAction(tr("&Insert item"));
    removeAction = itemsMenu->addAction(tr("&Remove item"));
    removeAction->setEnabled(false);
    itemsMenu->addSeparator();
    ascendingAction = itemsMenu->addAction(tr("Sort in &ascending order"));
    descendingAction = itemsMenu->addAction(tr("Sort in &descending order"));
    autoSortAction = itemsMenu->addAction(tr("&Automatically sort items"));
    autoSortAction->setCheckable(true);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(itemsMenu);

    treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(2);
    QStringList headers;
    headers << tr("Subject") << tr("Default");
    treeWidget->setHeaderLabels(headers);

    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(ascendingAction, SIGNAL(triggered()), this, SLOT(sortAscending()));
    connect(descendingAction, SIGNAL(triggered()), this, SLOT(sortDescending()));
    connect(autoSortAction, SIGNAL(triggered()), this, SLOT(updateSortItems()));
    connect(insertAction, SIGNAL(triggered()), this, SLOT(insertItem()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));
    connect(treeWidget,
            SIGNAL(currentChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(updateMenus(QTreeWidgetItem *)));

    setupTreeItems();

    setCentralWidget(treeWidget);
    setWindowTitle(tr("Tree Widget"));
}

void MainWindow::setupTreeItems()
{
    QTreeWidgetItem *cities = new QTreeWidgetItem(treeWidget);
    cities->setText(0, tr("Cities"));
    QTreeWidgetItem *osloItem = new QTreeWidgetItem(cities);
    osloItem->setText(0, tr("Oslo"));
    osloItem->setText(1, tr("Yes"));

    (new QTreeWidgetItem(cities))->setText(0, tr("Stockholm"));
    (new QTreeWidgetItem(cities))->setText(0, tr("Helsinki"));
    (new QTreeWidgetItem(cities))->setText(0, tr("Copenhagen"));

    QTreeWidgetItem *planets = new QTreeWidgetItem(treeWidget, cities);
    planets->setText(0, tr("Planets"));
    (new QTreeWidgetItem(planets))->setText(0, tr("Mercury"));
    (new QTreeWidgetItem(planets))->setText(0, tr("Venus"));

    QTreeWidgetItem *earthItem = new QTreeWidgetItem(planets);
    earthItem->setText(0, tr("Earth"));
    earthItem->setText(1, tr("Yes"));

    (new QTreeWidgetItem(planets))->setText(0, tr("Mars"));
    (new QTreeWidgetItem(planets))->setText(0, tr("Jupiter"));
    (new QTreeWidgetItem(planets))->setText(0, tr("Saturn"));
    (new QTreeWidgetItem(planets))->setText(0, tr("Uranus"));
    (new QTreeWidgetItem(planets))->setText(0, tr("Neptune"));
    (new QTreeWidgetItem(planets))->setText(0, tr("Pluto"));
}

void MainWindow::sortAscending()
{
    treeWidget->sortItems(0, Qt::AscendingOrder);
}

void MainWindow::sortDescending()
{
    treeWidget->sortItems(0, Qt::DescendingOrder);
}

void MainWindow::insertItem()
{
    QString itemText = QInputDialog::getText(tr("Insert item"),
        tr("Input text for the new item:"));

    if (itemText.isEmpty())
        return;

    QTreeWidgetItem *parent = treeWidget->currentItem()->parent();
    QTreeWidgetItem *newItem;
    if (parent)
        newItem = new QTreeWidgetItem(parent, treeWidget->currentItem());
    else
        newItem = new QTreeWidgetItem(treeWidget, treeWidget->currentItem());

    newItem->setText(0, itemText);
}

void MainWindow::removeItem()
{
    QTreeWidgetItem *parent = treeWidget->currentItem()->parent();
    int index;

    if (parent) {
        index = parent->indexOfChild(treeWidget->currentItem());
        parent->takeChild(index);
    } else {
        index = treeWidget->indexOfTopLevelItem(treeWidget->currentItem());
        treeWidget->takeTopLevelItem(index);
    }
}

void MainWindow::updateMenus(QTreeWidgetItem *current)
{
    removeAction->setEnabled(current != 0);
}

void MainWindow::updateSortItems()
{
    ascendingAction->setEnabled(!autoSortAction->isChecked());
    descendingAction->setEnabled(!autoSortAction->isChecked());

    treeWidget->setSortingEnabled(autoSortAction->isChecked());
}

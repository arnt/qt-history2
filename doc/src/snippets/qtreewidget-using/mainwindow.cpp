#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    QMenu *itemsMenu = new QMenu(tr("&Items"));

    insertAction = itemsMenu->addAction(tr("&Insert Item"));
    removeAction = itemsMenu->addAction(tr("&Remove Item"));
    removeAction->setEnabled(false);
    itemsMenu->addSeparator();
    ascendingAction = itemsMenu->addAction(tr("Sort in &Ascending Order"));
    descendingAction = itemsMenu->addAction(tr("Sort in &Descending Order"));
    autoSortAction = itemsMenu->addAction(tr("&Automatically Sort Items"));
    autoSortAction->setCheckable(true);
    itemsMenu->addSeparator();
    QAction *findItemsAction = itemsMenu->addAction(tr("&Find Items"));
    findItemsAction->setShortcut(tr("Ctrl+F"));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(itemsMenu);

    treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(2);
    QStringList headers;
    headers << tr("Subject") << tr("Default");
    treeWidget->setHeaderLabels(headers);

    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(ascendingAction, SIGNAL(triggered()), this, SLOT(sortAscending()));
    connect(autoSortAction, SIGNAL(triggered()), this, SLOT(updateSortItems()));
    connect(descendingAction, SIGNAL(triggered()), this, SLOT(sortDescending()));
    connect(findItemsAction, SIGNAL(triggered()), this, SLOT(findItems()));
    connect(insertAction, SIGNAL(triggered()), this, SLOT(insertItem()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));
    connect(treeWidget,
            SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(updateMenus(QTreeWidgetItem *)));

    setupTreeItems();
    updateMenus(treeWidget->currentItem());

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

void MainWindow::findItems()
{
    QString itemText = QInputDialog::getText(this, tr("Find Items"),
        tr("Text to find (including wildcards):"));

    if (itemText.isEmpty())
        return;

    QTreeWidgetItem *item;
    foreach (item, treeWidget->selectedItems())
        treeWidget->setItemSelected(item, false);

    QList<QTreeWidgetItem *> found = treeWidget->findItems(
        itemText, Qt::MatchWildcard);

    foreach (item, found) {
        treeWidget->setItemSelected(item, true);
        // Show the item->text(0) for each item.
    }
}

void MainWindow::insertItem()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();

    if (!currentItem)
        return;

    QString itemText = QInputDialog::getText(this, tr("Insert Item"),
        tr("Input text for the new item:"));

    if (itemText.isEmpty())
        return;

    QTreeWidgetItem *parent = currentItem->parent();
    QTreeWidgetItem *newItem;
    if (parent)
        newItem = new QTreeWidgetItem(parent, treeWidget->currentItem());
    else
        newItem = new QTreeWidgetItem(treeWidget, treeWidget->currentItem());

    newItem->setText(0, itemText);
}

void MainWindow::removeItem()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();

    if (!currentItem)
        return;

    QTreeWidgetItem *parent = currentItem->parent();
    int index;

    if (parent) {
        index = parent->indexOfChild(treeWidget->currentItem());
        delete parent->takeChild(index);
    } else {
        index = treeWidget->indexOfTopLevelItem(treeWidget->currentItem());
        delete treeWidget->takeTopLevelItem(index);
    }
}

void MainWindow::sortAscending()
{
    treeWidget->sortItems(0, Qt::AscendingOrder);
}

void MainWindow::sortDescending()
{
    treeWidget->sortItems(0, Qt::DescendingOrder);
}

void MainWindow::updateMenus(QTreeWidgetItem *current)
{
    insertAction->setEnabled(current != 0);
    removeAction->setEnabled(current != 0);
}

void MainWindow::updateSortItems()
{
    ascendingAction->setEnabled(!autoSortAction->isChecked());
    descendingAction->setEnabled(!autoSortAction->isChecked());

    treeWidget->setSortingEnabled(autoSortAction->isChecked());
}

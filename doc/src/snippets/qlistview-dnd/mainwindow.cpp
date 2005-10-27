#include <QtGui>

#include "mainwindow.h"
#include "model.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    menuBar()->addMenu(fileMenu);

//  For convenient quoting:
QListView *listView = new QListView(this);
listView->setSelectionMode(QAbstractItemView::SingleSelection);
listView->setDragEnabled(true);
listView->setAcceptDrops(true);
listView->setDropIndicatorShown(true);

    this->listView = listView;

    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    setupListItems();

    setCentralWidget(listView);
    setWindowTitle(tr("List View"));
}

void MainWindow::setupListItems()
{
    QStringList items;
    items << tr("Oak") << tr("Fir") << tr("Pine") << tr("Birch") << tr("Hazel")
          << tr("Redwood") << tr("Sycamore") << tr("Chestnut")
          << tr("Mahogany");

    DragDropListModel *model = new DragDropListModel(items, this);
    listView->setModel(model);
}

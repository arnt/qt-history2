#include <qapplication.h>
#include <qgenerictableview.h>
#include <qgenerictreeview.h>
#include <qlistview.h>
#include <qsplitter.h>
#include "model.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSplitter page;

    QAbstractItemModel *data = new Model(1000, 10, &page);
    QItemSelectionModel *selections = new QItemSelectionModel(data, data);

    QGenericTableView *table = new QGenericTableView(&page);
    table->setModel(data);
    table->setSelectionModel(selections);

    QGenericTreeView *tree = new QGenericTreeView(&page);
    tree->setModel(data);
    tree->setSelectionModel(selections);

    QListView *list = new QListView(&page);
    list->setModel(data);
    list->setSelectionModel(selections);
    list->setLayoutMode(QListView::Batched);
    list->setIconMode(QListView::Large);
    list->setMovement(QListView::Free);
    list->setFlow(QListView::LeftToRight);
    list->setWrapping(true);
    list->viewport()->setAcceptDrops(true);

    app.setMainWidget(&page);
    page.show();

    return app.exec();
}

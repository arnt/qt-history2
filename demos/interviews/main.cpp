#include <qapplication.h>
#include <qgenerictableview.h>
#include <qgenerictreeview.h>
#include <qgenericlistview.h>
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

    QGenericListView *list = new QGenericListView(&page);
    list->setModel(data);
    list->setSelectionModel(selections);
    list->setLayoutMode(QGenericListView::Batched);
    list->setIconMode(QGenericListView::Large);
    list->setMovement(QGenericListView::Free);
    list->setFlow(QGenericListView::LeftToRight);
    list->setWrapping(true);
    list->viewport()->setAcceptDrops(true);

    app.setMainWidget(&page);
    page.show();

    return app.exec();
}

/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  main.cpp

  A simple example that shows how selections can be used directly on a model.
  It shows the result of some selections made using a table view.
*/

#include <QApplication>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QTableView>

#include "model.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TableModel *model = new TableModel(8, 4, &app);

    QTableView *table = new QTableView(0);
    table->setModel(model);

    QItemSelectionModel *selectionModel = table->selectionModel();
    QModelIndex topLeft;
    QModelIndex bottomRight;

    topLeft = model->index(0, 0, QModelIndex(), QModelIndex::View);
    bottomRight = model->index(5, 2, QModelIndex(),
        QModelIndex::View);

    QItemSelection selection(topLeft, bottomRight, model);
    selectionModel->select(selection, QItemSelectionModel::Select);

    QItemSelection toggleSelection;

    topLeft = model->index(2, 1, QModelIndex(), QModelIndex::View);
    bottomRight = model->index(7, 3, QModelIndex(), QModelIndex::View);
    toggleSelection.select(topLeft, bottomRight, model);

    selectionModel->select(toggleSelection, QItemSelectionModel::Toggle);

    QItemSelection columnSelection;

    topLeft = model->index(0, 1, QModelIndex(), QModelIndex::View);
    bottomRight = model->index(0, 2, QModelIndex(), QModelIndex::View);

    columnSelection.select(topLeft, bottomRight, model);

    selectionModel->select(columnSelection,
        QItemSelectionModel::Select | QItemSelectionModel::Columns);

    QItemSelection rowSelection;

    topLeft = model->index(0, 0, QModelIndex(), QModelIndex::View);
    bottomRight = model->index(1, 0, QModelIndex(), QModelIndex::View);

    rowSelection.select(topLeft, bottomRight, model);

    selectionModel->select(rowSelection,
        QItemSelectionModel::Select | QItemSelectionModel::Rows);

    table->setWindowTitle("Selected items in a table model");
    table->show();
    table->resize(460, 280);
    app.setMainWidget(table);

    return app.exec();
}

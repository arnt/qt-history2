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

    TableModel *model = new TableModel(8, 8, &app);

    QTableView *table = new QTableView(0);
    table->setModel(model);

    QItemSelectionModel *selectionModel = table->selectionModel();

    QModelIndex topLeft = model->index(3, 3, QModelIndex(), QModelIndex::View);
    QModelIndex bottomRight = model->index(4, 4, QModelIndex(),
        QModelIndex::View);
    
    QItemSelection selection(topLeft, bottomRight, model);
    selectionModel->select(selection, QItemSelectionModel::Select);

    for (int row = 0; row < 8; row += 2) {
        for (int column = 0; column < 8; column += 2) {
            QModelIndex topLeft = model->index(row, column, QModelIndex(),
                QModelIndex::View);
            QModelIndex bottomRight = model->index(row+1, column+1,
                QModelIndex(), QModelIndex::View);
            QItemSelection selection(topLeft, bottomRight, model);
            if (((row % 4) ^ (column % 4)) != 0)
                selectionModel->select(selection, QItemSelectionModel::Toggle);
        }
    }

    table->setWindowTitle("Selected items in a table model");
    table->show();
    app.setMainWidget(table);

    return app.exec();
}

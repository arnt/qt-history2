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

  A simple example that shows how a single model can be shared between
  multiple views.
*/

#include <QApplication>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTableView>

#include "model.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TableModel *model = new TableModel(4, 2, &app);

    QTableView *firstTableView = new QTableView;
    QTableView *secondTableView = new QTableView;

    firstTableView->setModel(model);
    secondTableView->setModel(model);

    firstTableView->horizontalHeader()->setModel(model);

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 2; ++column) {
            QModelIndex index = model->index(row, column, QModelIndex());
            model->setData(index, QVariant(QString("(%1, %2)").arg(row).arg(column)));
        }
    }

    secondTableView->setSelectionModel(firstTableView->selectionModel());

    firstTableView->setWindowTitle("First table view");
    secondTableView->setWindowTitle("Second table view");
    firstTableView->show();
    secondTableView->show();
    return app.exec();
}

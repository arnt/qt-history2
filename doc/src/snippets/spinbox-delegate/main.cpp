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

  A simple example that shows how a view can use a custom delegate to edit
  data obtained from a model.
*/

#include <QApplication>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTableView>

#include "model.h"
#include "delegate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TableModel *model = new TableModel(4, 2, &app);

    QTableView *tableView = new QTableView(0);
    SpinBoxDelegate *delegate = new SpinBoxDelegate(0);

    tableView->setModel(model);
    tableView->setItemDelegate(delegate);

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 2; ++column) {
            QModelIndex index = model->index(row, column, QModelIndex());
            model->setData(index, QAbstractItemModel::EditRole,
                QVariant((row+1) * (column+1)));
        }
    }

    tableView->setWindowTitle("A delegate example");
    tableView->show();
    app.setMainWidget(tableView);

    return app.exec();
}

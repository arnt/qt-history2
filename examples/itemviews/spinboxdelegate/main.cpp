/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include <QStandardItemModel>
#include <QTableView>

#include "delegate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStandardItemModel *model = new QStandardItemModel(4, 2);
    QTableView *tableView = new QTableView;
    tableView->setModel(model);

    SpinBoxDelegate *delegate = new SpinBoxDelegate;
    tableView->setItemDelegate(delegate);

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 2; ++column) {
            QModelIndex index = model->index(row, column, QModelIndex());
            model->setData(index, QVariant((row+1) * (column+1)));
        }
    }

    tableView->setWindowTitle("Spin Box Delegate");
    tableView->show();
    return app.exec();
}

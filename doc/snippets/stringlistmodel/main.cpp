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

/*!
    The main function for the string list model example. This creates and
    populates a model with values from a string list then displays the
    contents of the model using a QListView widget.
*/

#include <qapplication.h>
#include <qlistview.h>

#include "model.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList numbers;
    numbers << "One" << "Two" << "Three" << "Four" << "Five";

    StringListModel *model = new StringListModel(numbers);
    QListView *view = new QListView();
    view->setWindowTitle("View onto a string list model");
    view->setModel(model);

    model->insertRows(5, QModelIndex(), 7);

    for (int row = 5; row < 12; ++row) {
        QModelIndex index = model->index(row, 0, QModelIndex(),
            QModelIndex::View);
        model->setData(index, QAbstractItemModel::EditRole,
            QString::number(row));
    }

    view->show();
    app.setMainWidget(view);

    return app.exec();
}

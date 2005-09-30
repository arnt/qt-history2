/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "treemodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QFile file(":/default.txt");
    file.open(QIODevice::ReadOnly);
    TreeModel *model = new TreeModel(file.readAll());
    file.close();

    QTreeView *unsortedView = new QTreeView;
    unsortedView->setModel(model);
    unsortedView->setWindowTitle("Unsorted Data");
    unsortedView->show();

    QSortingProxyModel *sortingModel = new QSortingProxyModel;
    sortingModel->setSourceModel(model);

    QTreeView *sortedView = new QTreeView;
    sortedView->setModel(sortingModel);
    sortedView->setWindowTitle("Sorted Data");
    sortedView->header()->setSortIndicator(1, Qt::AscendingOrder);
    sortedView->header()->setSortIndicatorShown(true);
    sortedView->header()->setClickable(true);
    sortedView->show();

    return app.exec();
}

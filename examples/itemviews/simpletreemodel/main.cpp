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

#include <QtGui>

#include "treemodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile file(":/default.txt");
    file.open(QIODevice::ReadOnly);
    TreeModel *model = new TreeModel(file.readAll());
    file.close();

    QTreeView *view = new QTreeView;
    view->setModel(model);
    view->setWindowTitle("Simple Tree Model");
    view->show();
    return app.exec();
}

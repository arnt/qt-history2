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

  A simple example of how to view a model in several views, and share a
  selection model.
*/

#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSplitter *splitter = new QSplitter;

    QDirModel *model = new QDirModel;
    QTreeView *tree = new QTreeView(splitter);
    tree->setModel(model);
    tree->setRootIndex(model->index(QDir::currentPath()));

    QListView *list = new QListView(splitter);
    list->setModel(model);
    list->setRootIndex(model->index(QDir::currentPath()));

    QItemSelectionModel *selection = new QItemSelectionModel(model);
    tree->setSelectionModel(selection);
    list->setSelectionModel(selection);

    splitter->setWindowTitle("Two views onto the same directory model");
    splitter->show();
    return app.exec();
}

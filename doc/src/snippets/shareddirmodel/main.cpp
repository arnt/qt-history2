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

    QDirModel *model = new QDirModel(QDir(), splitter);

    QTreeView *tree = new QTreeView(splitter);
    QListView *list = new QListView(splitter);

    tree->setModel(model);
    list->setModel(model);

    QItemSelectionModel *selection = new QItemSelectionModel(model);

    tree->setSelectionModel(selection);
    list->setSelectionModel(selection);

    splitter->setWindowTitle("Two views onto the same directory model");
    splitter->show();
    app.setMainWidget(splitter);

    return app.exec();
}

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

  A simple example of how to access items from an existing model.
*/

#include <qapplication.h>
#include <qdir.h>
#include <qdirmodel.h>
#include <qitemselectionmodel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpalette.h>
#include <qvariant.h>
#include <qwidget.h>

/*!
    Create a default directory model and, using the index-based interface to
    the model and some QLabel widgets, populate the window's layout with the
    names of objects in the directory.

    Note that we only want to read the filenames in the highest level of the
    directory, so we supply a default (invalid) QModelIndex to the model in
    order to indicate that we want top-level items.
*/

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *window = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(window);
    QLabel *title = new QLabel("Some items from the directory model", window);
    title->setBackgroundRole(QPalette::Base);
    title->setMargin(8);
    layout->addWidget(title);
    
    QDirModel *model = new QDirModel(QDir(), window);
    int numRows = model->rowCount(QModelIndex());

    for (int row = 0; row < numRows; ++row) {
        QModelIndex index = model->index(row, 0, QModelIndex());

        QString text = model->data(index, QAbstractItemModel::DisplayRole).toString();
        qDebug("%s", text.ascii());

        QLabel *label = new QLabel(text, window);
        layout->addWidget(label);
    }

    window->setWindowTitle("A simple model example");
    window->show();

    app.setMainWidget(window);
    return app.exec();
}

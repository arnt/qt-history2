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
  window.cpp

  A minimal subclass of QTableView with slots to allow the selection model
  to be monitored.
*/

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

#include "model.h"
#include "window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Selected items in a table model");

    model = new TableModel(8, 4, this);

    table = new QTableView(this);
    table->setModel(model);

    QMenu *actionMenu = new QMenu(tr("&Actions"), this);
    QAction *fillAction = actionMenu->addAction(tr("&Fill selection"));
    QAction *clearAction = actionMenu->addAction(tr("&Clear selection"));
    QAction *selectAllAction = actionMenu->addAction(tr("&Select all"));
    menuBar()->addMenu(actionMenu);

    connect(fillAction, SIGNAL(triggered()), this, SLOT(fillSelection()));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearSelection()));
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));

    selectionModel = table->selectionModel();

    statusBar();
    setCenterWidget(table);
}

void MainWindow::fillSelection()
{
    QModelIndexList indices = selectionModel->selectedItems();
    QModelIndex index;

    foreach(index, indices) {
        QString text = QString("(%1,%2)").arg(index.row()).arg(index.column());
        model->setData(index, QAbstractItemModel::EditRole, text);
    }
}

void MainWindow::clearSelection()
{
    QModelIndexList indices = selectionModel->selectedItems();
    QModelIndex index;

    foreach(index, indices)
        model->setData(index, QAbstractItemModel::EditRole, "");
}

void MainWindow::selectAll()
{
    QModelIndex parent = QModelIndex();
    QModelIndex topLeft = model->index(0, 0, parent, QModelIndex::View);
    QModelIndex bottomRight = model->index(model->rowCount(parent)-1,
        model->columnCount(parent)-1, parent, QModelIndex::View);

    QItemSelection selection(topLeft, bottomRight, model);
    selectionModel->select(selection, QItemSelectionModel::Select);
}

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
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTableView>
#include <QTabWidget>
#include <QTextStream>

#include "model.h"
#include "barview.h"
#include "pieview.h"
#include "window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt Example - Charts and Graphs");

    QMenu *fileMenu = new QMenu(tr("&File"), this);
    QAction *openAction = fileMenu->addAction(tr("&Open"));
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(QKeySequence(tr("Ctrl+S")));
    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    centerWidget = new QFrame(this);

    setupModel();
    setupLayout();

    menuBar()->addMenu(fileMenu);
    statusBar();
    setCentralWidget(centerWidget);
}

void MainWindow::setupModel()
{
    model = new TableModel(8, 3, this);
}

void MainWindow::setupLayout()
{
    QGridLayout *layout = new QGridLayout(centerWidget);

    pieChart = new PieView(centerWidget);
    horizontalBarChart = new BarView(centerWidget, Qt::Horizontal);
    verticalBarChart = new BarView(centerWidget, Qt::Vertical);
    table = new QTableView(centerWidget);

    pieChart->setModel(model);
    horizontalBarChart->setModel(model);
    verticalBarChart->setModel(model);
    table->setModel(model);

    layout->addWidget(table, 0, 0);
    layout->addWidget(horizontalBarChart, 0, 1);
    layout->addWidget(verticalBarChart, 1, 0);
    layout->addWidget(pieChart, 1, 1);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Choose a data file"), "", "*.cht");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);

        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            QString line;

            model->removeRows(0, QModelIndex::Null,
                model->rowCount(QModelIndex::Null)-1);
            model->insertColumns(0, QModelIndex::Null, 3);

            int row = 0;
            do {
                line = stream.readLine();
                if (!line.isEmpty()) {
                
                    model->insertRows(row, QModelIndex::Null, 1);

                    QStringList pieces = line.split(",", QString::SkipEmptyParts);
                    for (int column = 0; column < qMin(3, pieces.count()); ++column) {
                        QModelIndex index = model->index(row, column,
                            QModelIndex::Null, QModelIndex::View);
                        model->setData(index, QAbstractItemModel::EditRole,
                            pieces[column]);
                    }
                    row++;
                }
            } while (!line.isEmpty());

            file.close();
        }
    }
}

void MainWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save file as"), "", "*.cht");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        QTextStream stream(&file);

        if (file.open(QFile::WriteOnly)) {
            for (int row = 0; row < model->rowCount(QModelIndex::Null); ++row) {

                QStringList pieces;

                for (int column = 0; column < 3; ++column) {
                    QModelIndex index = model->index(row, column,
                        QModelIndex::Null, QModelIndex::View);
                    pieces.append(model->data(index,
                        QAbstractItemModel::DisplayRole).toString());
                }
                stream << pieces.join(",") << "\n";
            }
        }

        file.close();
    }
}

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
  mainwindow.cpp

  A minimal subclass of QTableView with slots to allow the selection model
  to be monitored.
*/

#include <QtGui>

#include "pieview.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    QAction *openAction = fileMenu->addAction(tr("&Open"));
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(QKeySequence(tr("Ctrl+S")));
    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    QMenu *windowsMenu = new QMenu(tr("&Windows"), this);
    pieWindowAction = windowsMenu->addAction("&Pie Chart");
    pieWindowAction->setCheckable(true);

    setupModel();
    setupViews();

    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(windowsMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowsMenu()));
    connect(pieWindowAction, SIGNAL(checked(bool)),
            pieChart, SLOT(setVisible(bool)));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(windowsMenu);
    statusBar();
    setCentralWidget(table);

    openFile(":/Charts/qtdata.cht");

    setWindowTitle(tr("Chart"));
    resize(640, 480);
}

void MainWindow::setupModel()
{
    model = new QStandardItemModel(8, 3, this);
}

void MainWindow::setupViews()
{
    table = new QTableView(this);

    pieChart = new PieView;
    pieChart->setWindowTitle(tr("Pie Chart"));

    table->setModel(model);
    pieChart->setModel(model);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
    table->setSelectionModel(selectionModel);
    pieChart->setSelectionModel(selectionModel);
}

void MainWindow::openFile(const QString &path)
{
    QString fileName;
    if (path.isNull())
        fileName = QFileDialog::getOpenFileName(this, tr("Choose a data file"),
                                                "", "*.cht");
    else
        fileName = path;

    if (!fileName.isEmpty()) {
        QFile file(fileName);

        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            QString line;

            model->removeRows(0, model->rowCount(QModelIndex()), QModelIndex());

            int row = 0;
            do {
                line = stream.readLine();
                if (!line.isEmpty()) {

                    model->insertRows(row, 1, QModelIndex());

                    QStringList pieces = line.split(",", QString::SkipEmptyParts);
                    for (int column = 0; column < qMin(3, pieces.count()); ++column) {
                        QModelIndex index = model->index(row, column,
                            QModelIndex());
                        model->setData(index, pieces[column]);
                    }
                    row++;
                }
            } while (!line.isEmpty());

            file.close();
            if (!pieChart->isExplicitlyHidden())
                pieChart->raise();
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

        if (file.open(QFile::WriteOnly | QFile::Text)) {
            for (int row = 0; row < model->rowCount(QModelIndex()); ++row) {

                QStringList pieces;

                for (int column = 0; column < 3; ++column) {
                    QModelIndex index = model->index(row, column,
                        QModelIndex());
                    pieces.append(model->data(index,
                        QAbstractItemModel::DisplayRole).toString());
                }
                stream << pieces.join(",") << "\n";
            }
        }

        file.close();
    }
}

void MainWindow::updateWindowsMenu()
{
    pieWindowAction->setChecked(!pieChart->isExplicitlyHidden());
}

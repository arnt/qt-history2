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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAbstractItemModel;
class QAbstractItemView;
class QDockWindow;
class QSplitter;
class QItemSelectionModel;
class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void openFile();
    void saveFile();
    void updateWindowsMenu();

private:
    void setupModel();
    void setupViews();

    QAbstractItemModel *model;
    QAbstractItemView *horizontalBarChart;
    QAbstractItemView *pieChart;
    QAbstractItemView *verticalBarChart;
    QAction *pieWindowAction;
    QSplitter *centerWidget;
    QItemSelectionModel *selectionModel;
    QTableView *table;
};

#endif

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

#ifndef WINDOW_H
#define WINDOW_H

#include <QAbstractItemView>
#include <QItemSelection>
#include <QMainWindow>
#include <QModelIndex>
#include <QWidget>

class QTableView;
class QFrame;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);

private slots:
    void openFile();
    void saveFile();

private:
    void setupModel();
    void setupLayout();

    QAbstractItemModel *model;
    QAbstractItemView *horizontalBarChart;
    QAbstractItemView *pieChart;
    QAbstractItemView *verticalBarChart;
    QFrame *centerWidget;
    QItemSelectionModel *selectionModel;
    QTableView *table;
};

#endif

/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindowbase.h"

QT_DECLARE_CLASS(QTextEdit)
QT_DECLARE_CLASS(QTreeWidget)
QT_DECLARE_CLASS(QTreeWidgetItem)

typedef QList<QTreeWidgetItem *> StyleItems;

class MainWindow : public QMainWindow, private Ui::MainWindowBase
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

public slots:
    void on_clearAction_triggered();
    void on_markAction_triggered();
    void on_printAction_triggered();
    void on_printPreviewAction_triggered();
    void on_unmarkAction_triggered();
    void printPage(int index, QPainter &painter, QPrinter &printer);
    void showFont(QTreeWidgetItem *item);
    void updateStyles(QTreeWidgetItem *item, int column);

private:
    QMap<QString, StyleItems> currentPageMap();
    bool setupPrinter(QPrinter &printer);
    void markUnmarkFonts(Qt::CheckState state);
    void setupFontTree();

    QList<int> sampleSizes;
    QMap<QString, StyleItems> pageMap;
    int markedCount;
};

#endif

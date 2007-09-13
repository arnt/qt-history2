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

#include <QMainWindow>
#include <QModelIndex>

QT_DECLARE_CLASS(QAbstractItemModel)
QT_DECLARE_CLASS(QComboBox)
class TreeModelCompleter;
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QLineEdit)
QT_DECLARE_CLASS(QProgressBar)
QT_DECLARE_CLASS(QCheckBox)
QT_DECLARE_CLASS(QTreeView)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void about();
    void changeCase(int);
    void changeMode(int);
    void highlight(const QModelIndex&);
    void updateContentsLabel(const QString&);

private:
    void createMenu();
    QAbstractItemModel *modelFromFile(const QString& fileName);

    QTreeView *treeView;
    QComboBox *caseCombo;
    QComboBox *modeCombo;
    QLabel *contentsLabel;
    TreeModelCompleter *completer;
    QLineEdit *lineEdit;
};

#endif // MAINWINDOW_H

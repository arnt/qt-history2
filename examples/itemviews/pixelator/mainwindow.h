/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

class ImageModel;
QT_DECLARE_CLASS(QAction)
QT_DECLARE_CLASS(QTableView)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void openImage(const QString &fileName);

public slots:
    void chooseImage();
    void printImage();
    void showAboutBox();
    void updateView();

private:
    ImageModel *model;
    QAction *printAction;
    QString currentPath;
    QTableView *view;
};

#endif

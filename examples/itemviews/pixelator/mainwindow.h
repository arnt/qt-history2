#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAbstractItemModel;
class QAction;
class QTableView;

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

private:
    QAbstractItemModel *model;
    QAction *printAction;
    QTableView *view;
};

#endif

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmotifwidget.h>


class MainWindow : public QMotifWidget
{
public:
    MainWindow();

    void showDialog();
    void showCustomDialog();
    void showQtDialog();
};

#endif // MAINWINDOW_H

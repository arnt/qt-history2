#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmotifwidget.h>


class MainWindow : public QMotifWidget
{
public:
    MainWindow();

    void showMotifDialog();
    void showQtDialog();
};

#endif // MAINWINDOW_H

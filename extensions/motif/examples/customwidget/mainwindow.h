#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>


class QMotifWidget;

class MainWindow : public QMainWindow
{
public:
    MainWindow();

private:
    QMotifWidget *customwidget;
};

#endif // MAINWINDOW_H

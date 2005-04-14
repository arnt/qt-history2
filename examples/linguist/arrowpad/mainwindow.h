#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class ArrowPad;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private:
    ArrowPad *arrowPad;
    QMenu *fileMenu;
    QAction *exitAct;
};

#endif

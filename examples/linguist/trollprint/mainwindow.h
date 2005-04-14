#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class PrintPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void about();

private:
    void createActions();
    void createMenus();

    PrintPanel *printPanel;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif

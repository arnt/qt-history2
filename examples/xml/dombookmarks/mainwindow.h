#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class XbelTree;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void open();
    void saveAs();
    void about();

private:
    void createActions();
    void createMenus();

    XbelTree *xbelTree;

    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *openAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif

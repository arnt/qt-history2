#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>

class ToolBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    ToolBar *toolbar;
    QAction *dockWindowActions;

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);

public slots:
    void actionTriggered(QAction *action);

private:
    void setupActions();
    void setupToolBar();
    void setupMenuBar();
    void setupDockWindows();
};

#endif // MAINWINDOW_H

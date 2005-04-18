#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class SettingsTree;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void openSettings();
    void openIniFile();
    void openPropertyList();
    void openRegistryPath();
    void about();

private:
    void createActions();
    void createMenus();

    SettingsTree *settingsTree;

    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *openSettingsAct;
    QAction *openIniFileAct;
    QAction *openPropertyListAct;
    QAction *openRegistryPathAct;
    QAction *synchronizeAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif
